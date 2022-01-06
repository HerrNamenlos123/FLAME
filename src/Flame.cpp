
#include <string>
#include <iostream>
#include <chrono>

#include "Log.h"
#include "Flame.h"
#include "FLAME_Protocol.h"
#include "NetLib.h"

FLAME::Instance flameInstance;

uint64_t getMicros() {
	using namespace std::chrono;
	return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

void packetReceived(uint8_t* packet, size_t packetSize) {
	//printf("Recived packet%s", packet);

	FLAME_Protocol::Packet pk;
	if (packetSize == FLAME_PROTOCOL_STANDARD_PACKET_LENGTH) {
		memcpy(pk.data, packet, packetSize);
		FLAME_Protocol::PacketData pd1;

		if (parsePacket(&pk, &pd1)) {
			LOG_WARN("Axis1 = {}", pd1.axis1);
			LOG_WARN("Axis2 = {}", pd1.axis2);
			LOG_WARN("Axis3 = {}", pd1.axis3);
			LOG_WARN("Axis4 = {}", pd1.axis4);
			LOG_WARN("id = {}", pd1.id);
			LOG_WARN("Additional = {}", pd1.additional);

		}
		else {
			LOG_ERROR("Packet invalid");
		}
	}
	

}

void FlameTest() {

	auto interfaces = NetLib::GetNetworkInterfaces();

	NetLib::SetLogLevel(NetLib::LOG_LEVEL_INFO);

	FLAME_Protocol::PacketData pd;
	pd.axis1 = 10;
	pd.axis2 = 20;
	pd.axis3 = 30;
	pd.axis4 = 40;
	pd.id = 1;
	pd.additional = 22;

	FLAME_Protocol::Packet packet;
	generatePacket(&packet, &pd);

	
	FLAME_Protocol::PacketData pd1;
	if (FLAME_Protocol::parsePacket(&packet, &pd1)) {
		if (pd.axis1 == pd1.axis1) {
			std::cout << "Axis1 are the same" << std::endl;
		}
		if (pd.axis2 == pd1.axis2) {
			std::cout << "Axis2 are the same" << std::endl;
		}
		if (pd.axis3 == pd1.axis3) {
			std::cout << "Axis3 are the same" << std::endl;
		}
		if (pd.axis4 == pd1.axis4) {
			std::cout << "Axis4 are the same" << std::endl;
		}
		if (pd.id == pd1.id) {
			std::cout << "ID are the same" << std::endl;
		}
		if (pd.additional == pd1.additional) {
			std::cout << "Additionals are the same" << std::endl;
		}

	}
	else {
		std::cout << "Wrong Packet" << std::endl;
	}

	NetLib::UDPClient uc("10.20.85.175", 22500);
	//NetLib::UDPClient uc("10.20.255.255", 22500);
	
	
	

	NetLib::UDPServer us(packetReceived, 22500);

	uint64_t old = getMicros();
	uint64_t interval = 1000000;
	while (true) {
		if (getMicros() >= old + interval) {
			old = getMicros();

			uc.send(packet.data, 23);
			uc.send(packet.data, 23);
			uc.send(packet.data, 23);
			uc.send(packet.data, 23);
			uc.send(packet.data, 23);
		}
	}

	

}

