#include <osc/osc.h>
#include <iostream>
#include <oscpp/client.hpp>
#include <cstring>
#include <iostream>
#include <string>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <limits>

#ifdef WINDOWS
    #include <winsock2.h>
    #pragma comment(lib, "WS2_32.lib")
    #include "Ws2tcpip.h"
    #include <tchar.h>
#else
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif



size_t makePacket(void* buffer, size_t size)
{
    // Construct a packet
    OSCPP::Client::Packet packet(buffer, size);
    packet
        // Open a bundle with a timetag
        .openBundle(1234ULL)
        // for efficiency this needs to be known in advance.
        .openMessage("/s_new", 1)
        // Write the arguments
        .string("hello world!")
        .closeMessage()
        .closeBundle();
    return packet.size();
}

size_t makePowderAnalytics(void* buffer, size_t size, MasterReturnParams* params, int index){
    // Construct a packet
    OSCPP::Client::Packet packet(buffer, size);
    std::stringstream addr;
    addr << "/tpt/" << index + 1;
    packet
        // Open a bundle with a timetag
        .openBundle(1234ULL)
        // for efficiency this needs to be known in advance.
        .openMessage(addr.str().c_str(), 11)
        // Write the arguments
        .int32(params->count)
        .float32(params->temp.first)
        .float32(params->vel.first)
        .int32(params->p_liquid)
        .int32(params->p_solid)
        .int32(params->p_gas)
        .int32(params->p_energy)
        .float32(params->y.first)
        .float32(params->y.second)
        .float32(params->x.first)
        .float32(params->x.second)
        
        
        
        .closeMessage()
        .closeBundle();
    return packet.size();
}

size_t makePlantPacket(void* buffer, size_t size, std::vector<int> bins, const char* addr){
    // Construct a packet
    OSCPP::Client::Packet packet(buffer, size);
    packet
        // Open a bundle with a timetag
        .openBundle(1234ULL)
        // for efficiency this needs to be known in advance.
        .openMessage(addr, 16);
    
    for (int i = 0; i < bins.size(); i++){
        packet.int32(bins[i]);
    }
        
        
    packet    
        .closeMessage()
        .closeBundle();
    return packet.size();
}


void oscTest(){
    
	std::cout << "hello world" << std::endl;
}

// Used for polyphonic handling so grains don't jump between handlers
void reorder_second_array(std::vector<int>* first_array, std::vector<int>* second_array) {
    // Create a map to store element to index mapping from the first array
    std::unordered_map<int, size_t> element_to_index;
    for (size_t index = 0; index < first_array->size(); ++index) {
        element_to_index[(*first_array)[index]] = index;
    }
    
    // Sort the second array based on the indices in the first array
    std::sort(second_array->begin(), second_array->end(), [&](int a, int b) {
        size_t index_a = element_to_index.count(a) ? element_to_index[a] : std::numeric_limits<size_t>::max();
        size_t index_b = element_to_index.count(b) ? element_to_index[b] : std::numeric_limits<size_t>::max();
        return index_a < index_b;
    });
}

 TPTOscClient::TPTOscClient() : partSorter() {
    // MAKEWORD(1,1) for Winsock 1.1, MAKEWORD(2,0) for Winsock 2.0
    #ifdef WINDOWS
	  WSADATA wsaData;
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        //exit(1);
    }
    #endif


    // Set up connection
    std::string hostname{ "127.0.0.1" };
    uint16_t port = 9000;
	
	sock = ::socket(AF_INET, SOCK_DGRAM, 0);

    destination.sin_family = AF_INET;
    destination.sin_port = htons(port);
	destination.sin_addr.s_addr = inet_addr("127.0.0.1");
}

TPTOscClient::~TPTOscClient(){
    #ifdef WINDOWS
    WSACleanup();
    ::closesocket(sock);
    #else
    close(sock);
    #endif

}

void TPTOscClient::CountParticle(Particle* p){
    partSorter.update(p->type);
}

void TPTOscClient::SortParticles(){
    auto newSorted = partSorter.getres();
    reorder_second_array(&sorted, &newSorted);
    sorted = newSorted;
    std::stringstream sortStream;
    /*for (size_t i = 0; i < sorted.size(); i++){
        sortStream << i + 1 << " : " << sorted[i] << "\n";
    }
    std::cout << sortStream.str() << std::endl;*/
    partSorter.reset();
}

void TPTOscClient::NewPlant(int y){
    plantHandler.update(y);
}

void TPTOscClient::KillPlant(int y){
    plantHandler.kill(y);
}



void TPTOscClient::ProcessParticle(Particle* p){
    int searchSize = std::min(sorted.size(), handlers.size());
    for (int i = 0; i < searchSize; i++){
        if (sorted[i] == p->type){
            handlers[i].update(p);
            break;
        }
            
    }
}

void TPTOscClient::AnalyzeAndSend(){
    MasterReturnParams params;
    for (int i = 0; i < handlers.size(); i++){
        
        handlers[i].get(&params);
        handlers[i].reset();
        const size_t packetSize = makePowderAnalytics(sendBuffer.data(), sendBuffer.size(), &params, i);
        int n_bytes = ::sendto(sock, sendBuffer.data(), packetSize, 0, reinterpret_cast<sockaddr*>(&destination), sizeof(destination));
    }

     auto [newBins, deletedBins] = plantHandler.get();
    const size_t packetSize = makePlantPacket(sendBuffer.data(), sendBuffer.size(), newBins, "/tptplantnew/");
    int n_bytes = ::sendto(sock, sendBuffer.data(), packetSize, 0, reinterpret_cast<sockaddr*>(&destination), sizeof(destination));

    const size_t packetSize2 = makePlantPacket(sendBuffer.data(), sendBuffer.size(), deletedBins, "/tptplantdel/");
    n_bytes = ::sendto(sock, sendBuffer.data(), packetSize, 0, reinterpret_cast<sockaddr*>(&destination), sizeof(destination));
     plantHandler.reset();
}
