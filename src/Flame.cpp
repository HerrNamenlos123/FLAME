#include "FLAME_Protocol.h"
#include<string>
#include<iostream>
using namespace FLAME_Protocol;

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

void FlameTest() {
	PacketData pd;
	pd.axis1 = 10;
	pd.axis2 = 20;
	pd.axis3 = 30;
	pd.axis4 = 40;
	pd.id = 1;
	pd.additional = 22;

	Packet packet; 
	generatePacket(&packet, &pd);

	logPacket(packet.data, 23, "localhost", 0);

	PacketData pd1;
	if (parsePacket(&packet, &pd1)) {
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


}