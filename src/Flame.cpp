#include <string>
#include <iostream>
#include <chrono>

#include "Log.h"
#include "Flame.h"
#include "FLAME_Protocol.h"
#include "networking.h"

FLAME_Protocol::FLAME_Instance protocolInstance;
FLAME::Instance flameInstance;

uint32_t getMicros() {
	using namespace std::chrono;
	return (uint32_t)duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

uint32_t getLocalIP() {
	return 0;
}

void writeUDP(uint32_t targetIP, uint16_t targetPort, uint8_t* data, uint8_t length) {
	net::sendUDP(toString(targetIP), targetPort, data, length);
}

void packetReceived(uint8_t* packet, size_t packetSize) {

	if (packetSize == 0)
		return;

	if (packetSize == FLAME_PROTOCOL_DISCOVERY_RESPONSE_LENGTH) {
		FLAME_Protocol::DiscoveryResponse response;
		if (FLAME_Protocol::parsePacket(&response, packet)) {
			flameInstance.setClientIP(response.ipAddress);
			//LOG_INFO("Received Discovery Packet from {}", instance.getClientIP());
		}
		else {
			LOG_ERROR("Packet invalid");
		}

	}

	if (packetSize == FLAME_PROTOCOL_REVIEW_PACKET_LENGTH) {
		FLAME_Protocol::ReviewPacket review;
		if (parsePacket(&review, packet)) {
			flameInstance.setEndpoint(review.id, review.data);

        	if (review.id >= 0 && review.id < sizeof(protocolInstance.registers) / sizeof(protocolInstance.registers[0])) {
        	    protocolInstance.registers[review.id] = review.data;
        	}
		}
		else {
			LOG_ERROR("Packet invalid");
		}
	}
}

std::vector<std::string> SplitString(std::string str, char delimeter) {
    std::string::size_type b = 0;
    std::vector<std::string> result;
    while ((b = str.find_first_not_of(delimeter, b)) != std::string::npos) {
        auto e = str.find_first_of(delimeter, b);
        result.push_back(str.substr(b, e - b));
        b = e;
    }
    return result;
}

void sendDiscoveryPackets() {
	uint8_t discoveryBuffer[FLAME_PROTOCOL_DISCOVERY_PACKET_LENGTH];
	FLAME_Protocol::generatePacket(discoveryBuffer);

	auto interfaces = net::getNetworkInterfaces();
	for (auto& interface : interfaces) {
		if (interface.address.length() > 0) {
			if (SplitString(interface.address, '.')[0] != "127") {
				net::sendUDP(net::createBroadcastAddress(interface), 22500, discoveryBuffer, FLAME_PROTOCOL_DISCOVERY_PACKET_LENGTH, true);
			}
		}
	}
}

std::vector<uint8_t> GenerateControlPacket() {
	std::vector<uint8_t> buffer(FLAME_PROTOCOL_CONTROL_PACKET_LENGTH, 0);
	static uint8_t endpointID = 4; 
	FLAME_Protocol::ControlPacket cp;
	cp.axis1 = protocolInstance.desiredAxis1;
	cp.axis2 = protocolInstance.desiredAxis2;
	cp.axis3 = protocolInstance.desiredAxis3;
	cp.axis4 = protocolInstance.desiredAxis4;
	cp.id = endpointID;
	cp.additional = protocolInstance.registers[endpointID];
	endpointID++;
	if (endpointID >= sizeof(protocolInstance.registers) / sizeof(protocolInstance.registers[0])) {
        endpointID = 4;
    }
	generatePacket(&cp, &buffer[0]);
	return buffer;
}

void updateSystem() {

	protocolInstance.safetyMode = false;
	protocolInstance.desiredAxis1 = sin(getMicros() / 1000000.f) * 2;
	LOG_INFO("Position: {}", (float)protocolInstance.desiredAxis1);

}

void FlameTest() {

	LOG_INFO("Constructing FLAME backend");
	NETWORKING_SET_LOGLEVEL(spdlog::level::warn);
	net::UDPServerAsync udpListener(packetReceived, FLAME_PROTOCOL_UDP_TARGET_PORT);
	std::unique_ptr<net::UDPClient> udpSender;
	std::string currentIP = "";

	LOG_INFO("Sending discovery packets");
	sendDiscoveryPackets();

	LOG_INFO("Waiting for responses");
	while (flameInstance.getClientIP().empty()) {}

	LOG_INFO("FLAME client discovered at {}", flameInstance.getClientIP());

	uint64_t old = getMicros();
	uint64_t interval = 10000;
	while (true) {
		if (getMicros() >= old + interval) {
			old = getMicros();

			if (flameInstance.getClientIP() != currentIP) {
				udpSender.reset();
				currentIP = flameInstance.getClientIP();
				udpSender = std::make_unique<net::UDPClient>(currentIP, FLAME_PROTOCOL_UDP_TARGET_PORT);
			}

			updateSystem();
			if (udpSender) {
				auto packet = GenerateControlPacket();
				udpSender->send(&packet[0], packet.size());
			}
		}
	}



}