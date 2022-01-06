
#include<string>
#include<iostream>

#include "FLAME_Protocol.h"
#include "NetLib.h"

#include "spdlog/spdlog.h"

void logPacket(uint8_t* data, size_t length, const char* ipAddress, uint16_t port) {
	std::string str = "";
	for (size_t i = 0; i < length; i++) {
		str += std::to_string(data[i]);
		str += ", ";
	}
	str.pop_back();
	str.pop_back();
	printf("UDP packet sent to %s:%d -> [%s] -> \"%s\"", ipAddress, port, str.c_str(), data);
}

void help(uint8_t* packet, size_t packetSize) {
	//printf("Recived packet%s", packet);

	FLAME_Protocol::Packet pk;
	if (packetSize == FLAME_PROTOCOL_STANDARD_PACKET_LENGTH) {
		memcpy(pk.data, packet, packetSize);
		FLAME_Protocol::PacketData pd1;

		if (parsePacket(&pk, &pd1)) {
			std::cout << pd1.axis1 << " = Axis1" << std::endl;
			std::cout << pd1.axis2 << " = Axis2" << std::endl;
			std::cout << pd1.axis3 << " = Axis3" << std::endl;
			std::cout << pd1.axis4 << " = Axis4" << std::endl;
			std::cout << (int)pd1.id << " = ID" << std::endl;
			std::cout << pd1.additional << " = Addionial" << std::endl;

		}
		else {
			std::cout << "Wrong Packet" << std::endl;
		}
	}
	

}

void FlameTest() {

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
	
	
	

	NetLib::UDPServer us(help, 22500);

	for (size_t i = 0; i < 1000000000; i++)
	{
		for (size_t j = 0; j < 1000000000; j++)
		{

		}
		uc.send(packet.data, 23);
		uc.send(packet.data, 23);
		uc.send(packet.data, 23);
		uc.send(packet.data, 23);
		uc.send(packet.data, 23);

	}

	

}

