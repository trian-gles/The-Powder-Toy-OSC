#pragma once
#include <winsock2.h>
#pragma comment(lib, "WS2_32.lib")
#undef GetUserNameW
#include <array>
#include "handler.h"
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
	SOCKET sock;
	sockaddr_in destination;
	std::array<char, kMaxPacketSize> sendBuffer;
	std::array<MasterHandler, HANDLERS> handlers; 
	ParticleIdCountSorter partSorter;
	std::vector<int> sorted;
};