
#include <string>
#include <iostream>
#include <chrono>
#include <sstream>

#include "Log.h"
#include "Flame.h"
#include "FLAME_Protocol.h"
#include "NetLib.h"

FLAME::Instance flameInstance;

uint32_t ipAdress = 0;
bool isvalid = false;

std::string ipToStr2(uint32_t ip)
{
	std::stringstream ipStr;
	ipStr << (ip & 0xff) << ".";
	ipStr << ((ip >> 8) & 0xff) << ".";
	ipStr << ((ip >> 16) & 0xff) << ".";
	ipStr << ((ip >> 24) & 0xff);
	return ipStr.str();
}
uint64_t getMicros() {
	using namespace std::chrono;
	return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

void packetReceived(uint8_t* packet, size_t packetSize) {
	
	if (packetSize == 0) 
		return;

	if (packetSize == FLAME_PROTOCOL_DISCOVERY_RESPONSE_LENGTH) {

		FLAME_Protocol::DiscoveryResponse dr;
		if (FLAME_Protocol::parsePacket(&dr, packet)) {
			ipAdress = dr.ipAddress;
			isvalid = true;
			LOG_WARN("IP = {}", dr.ipAddress);

		}
		else {
			LOG_ERROR("Packet invalid");
		}

	}

	if (packetSize == FLAME_PROTOCOL_REVIEW_PACKET_LENGTH) {

		FLAME_Protocol::ReviewPacket rp;

		if (parsePacket(&rp, packet)) {
			LOG_WARN("id = {}", rp.id);
			LOG_WARN("Data = {}", rp.data);

		}
		else {
			LOG_ERROR("Packet invalid");
		}
	}
	

}

void FlameTest() {

	auto interfaces = NetLib::GetNetworkInterfaces();

	//Send Discovery Packet
	uint8_t discoveryBuffer[FLAME_PROTOCOL_DISCOVERY_PACKET_LENGTH];
	FLAME_Protocol::generatePacket(discoveryBuffer);

	for (auto& ifc : interfaces) {
		LOG_WARN("IP: {}, Broadcast address: {}", ifc.address, NetLib::CreateBroadcastAddress(ifc));

		NetLib::SendUDP(NetLib::CreateBroadcastAddress(ifc), 22500, discoveryBuffer, FLAME_PROTOCOL_DISCOVERY_PACKET_LENGTH);
	}

	NetLib::SetLogLevel(NetLib::LOG_LEVEL_INFO);

	//Create Controll Packet
	FLAME_Protocol::ControlPacket cp;
	cp.axis1 = 10;
	cp.axis2 = 20;
	cp.axis3 = 30;
	cp.axis4 = 40;
	cp.id = 1;
	cp.additional = 22;

	uint8_t buffer[23];

	generatePacket( &cp, buffer );


	if (FLAME_Protocol::parsePacket(&cp, buffer)) {
		LOG_WARN("Axis1 = {}", cp.axis1);
		LOG_WARN("Axis2 = {}", cp.axis2);
		LOG_WARN("Axis3 = {}", cp.axis3);
		LOG_WARN("Axis4 = {}", cp.axis4);
		LOG_WARN("id = {}", cp.id);
		LOG_WARN("Additional = {}", cp.additional);

	}
	else {
		std::cout << "Wrong Packet" << std::endl;
	}
	NetLib::UDPServer us(packetReceived, 22500);
	while (!isvalid) {

	}
	NetLib::UDPClient uc(ipToStr2(ipAdress), 22500);
	LOG_WARN("{}", ipToStr2(ipAdress));
	

	uint64_t old = getMicros();
	uint64_t interval = 1000000;
	while (true) {
		if (getMicros() >= old + interval) {
			old = getMicros();

			uc.send(buffer, 23);
			uc.send(buffer, 23);
			uc.send(buffer, 23);
			uc.send(buffer, 23);
			uc.send(buffer, 23);
		}
	}

	

}

