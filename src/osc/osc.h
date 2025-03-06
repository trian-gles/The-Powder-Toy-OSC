#pragma once
#include <array>
#include "handler.h"

#ifdef WINDOWS
    #include <winsock2.h>
    #pragma comment(lib, "WS2_32.lib")
		#undef GetUserNameW
    #include "Ws2tcpip.h"
#else
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif


#include "simulation/Particle.h"

#define kMaxPacketSize 8192
#define HANDLERS 6

void oscTest();


class TPTOscClient
{
public:
	TPTOscClient();
	~TPTOscClient();
	void AnalyzeAndSend();
	void ProcessParticle(Particle* p);
	void CountParticle(Particle* p);
	void SortParticles();
	
private:
	int sock;
	sockaddr_in destination;
	std::array<char, kMaxPacketSize> sendBuffer;
	std::array<MasterHandler, HANDLERS> handlers; 
	ParticleIdCountSorter partSorter;
	std::vector<int> sorted;
};
