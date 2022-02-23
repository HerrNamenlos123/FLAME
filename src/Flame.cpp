#include <string>
#include <iostream>
#include <chrono>

#include "Log.h"
#include "Flame.h"
#include "FLAME_Protocol.h"
#include "NetLib.h"

FLAME::Instance instance;

uint32_t getMicros() {
	using namespace std::chrono;
	return (uint32_t)duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

uint32_t getLocalIP() {
	return 0;
}

void writeUDP(uint32_t targetIP, uint16_t targetPort, uint8_t* data, uint8_t length) {
	NetLib::SendUDP(toString(targetIP), targetPort, data, length);
}

void packetReceived(uint8_t* packet, size_t packetSize) {

	if (packetSize == 0)
		return;

	if (packetSize == FLAME_PROTOCOL_DISCOVERY_RESPONSE_LENGTH) {
		FLAME_Protocol::DiscoveryResponse response;
		if (FLAME_Protocol::parsePacket(&response, packet)) {
			instance.setClientIP(response.ipAddress);
			//LOG_INFO("Received Discovery Packet from {}", instance.getClientIP());
		}
		else {
			LOG_ERROR("Packet invalid");
		}

	}

	if (packetSize == FLAME_PROTOCOL_REVIEW_PACKET_LENGTH) {
		FLAME_Protocol::ReviewPacket review;
		if (parsePacket(&review, packet)) {
			instance.setEndpoint(review.id, review.data);
			//LOG_DEBUG("Received data packet: endpoint #{} = {}", review.id, review.data);
			static uint32_t da = 0;
			da++;
			if (da != review.data) {
				LOG_ERROR("ERROR: Data invalid! da={}, received={}", da, review.data);
				da = review.data;
			}
			static uint32_t lastTime = 0;
			LOG_DEBUG("Took {} ms", getMicros() - lastTime);
			lastTime = getMicros();
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

	auto interfaces = NetLib::GetNetworkInterfaces();
	for (auto& interface : interfaces) {
		if (interface.address.length() > 0) {
			if (SplitString(interface.address, '.')[0] != "127") {
				//LOG_WARN("IP: {}, Broadcast address: {}", interface.address, NetLib::CreateBroadcastAddress(interface));
				NetLib::SendUDP(NetLib::CreateBroadcastAddress(interface), 22500, discoveryBuffer, FLAME_PROTOCOL_DISCOVERY_PACKET_LENGTH, true);
			}
		}
	}
}

std::vector<uint8_t> GenerateControlPacket(float axis1, float axis2, float axis3, float axis4, uint8_t endpointID, uint32_t additional) {
	std::vector<uint8_t> buffer(FLAME_PROTOCOL_CONTROL_PACKET_LENGTH, 0);
	FLAME_Protocol::ControlPacket cp;
	cp.axis1 = axis1;
	cp.axis2 = axis2;
	cp.axis3 = axis3;
	cp.axis4 = axis4;
	cp.id = endpointID;
	cp.additional = additional;
	generatePacket(&cp, &buffer[0]);
	return buffer;
}

void FlameTest() {

	NetLib::SetLogLevel(NetLib::LOG_LEVEL_WARN);
	NetLib::UDPServerAsync udpListener(packetReceived, FLAME_PROTOCOL_UDP_TARGET_PORT);
	std::unique_ptr<NetLib::UDPClient> udpSender;
	std::string currentIP = "";

	sendDiscoveryPackets();

	while (instance.getClientIP().empty()) {}

	LOG_INFO("FLAME client discovered at {}", instance.getClientIP());

	uint64_t old = getMicros();
	uint64_t interval = 500;
	while (true) {
		if (getMicros() >= old + interval) {
			old = getMicros();

			if (instance.getClientIP() != currentIP) {
				udpSender.reset();
				currentIP = instance.getClientIP();
				udpSender = std::make_unique<NetLib::UDPClient>(currentIP, FLAME_PROTOCOL_UDP_TARGET_PORT);
			}

			static uint32_t data = 0;
			data++;
			auto packet = GenerateControlPacket(1, 2, 3, 4, 27, data);

			if (udpSender) {
				udpSender->send(&packet[0], packet.size());
				LOG_TRACE("Sending packet: {}", data);
			}
		}
	}



}