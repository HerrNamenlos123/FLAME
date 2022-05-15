#include <string>
#include <iostream>
#include <chrono>
#include <thread>

#include "Log.h"
#include "Flame.h"
#include "FLAME_Protocol.h"
#include "networking.h"

#include "FLAME.pb.h"

net::UDPServer udpListener(FLAME_PROTOCOL_UDP_TARGET_PORT);
std::unique_ptr<net::UDPClient> udpSender;
uint32_t oldTime = getMicros();
uint32_t interval = 5000;

auto& tx = FLAME_Protocol::toMCU;
auto& rx = FLAME_Protocol::toPC;

uint32_t getMicros() {
	using namespace std::chrono;
	return (uint32_t)duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

uint32_t getLocalIP() {
	return 0;
}

void writeUDP(uint32_t targetIP, uint16_t targetPort, uint8_t* data, uint8_t length) {
	net::sendUDP(net::ipToString(targetIP), targetPort, data, length);
}

void writeUDPBroadcast(uint32_t targetIP, uint16_t targetPort, uint8_t* data, uint8_t length) {
	net::sendUDP(net::ipToString(targetIP), targetPort, data, length, true);
}

void packetReceived(const net::NetworkPacket& packet) {
	FLAME_Protocol::packetReceived(&packet.data[0], packet.data.size(), net::ipToBytes(packet.remoteIP), packet.remotePort);
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

	auto interfaces = net::getNetworkInterfaces();
	for (auto& interface : interfaces) {
		if (interface.address.length() > 0) {
			if (SplitString(interface.address, '.')[0] != "127") {
				FLAME_Protocol::sendDiscoveryPacket(net::ipToBytes(net::createBroadcastAddress(interface)));
			}
		}
	}
}

#define RAD_TO_DEG 57.295779513082320876798154814105
#define degrees(rad) ((rad)*RAD_TO_DEG)

size_t cnt = 0;

void updateValues() {
	tx.clearSafetyMode = true;
	float speed = 0.1;
	if ((cnt / 1000) % 2 == 0) {
		tx.desiredAxis2 = degrees(asin(22.f / 57.f));
	}
	else {
		tx.desiredAxis2 = degrees(asin(22.f / 57.f)) + 5;
	}
	LOG_WARN("Pos: {}", tx.desiredAxis2);
	//tx.desiredAxis2 = sin(getMicros() / 600000.0) * 30 + 50;
	cnt++;
}

void update() {

	auto packet = udpListener.read();
	if (packet.has_value()) {
		packetReceived(packet.value());
	}

	if (FLAME_Protocol::mcu_ip == 0) {
		return;
	}

	if (getMicros() >= oldTime + interval) {
		oldTime = getMicros();

		if (udpSender) {
			if (net::ipToBytes(udpSender->ip()) != FLAME_Protocol::mcu_ip) {
				udpSender.reset();
			}
		}
		else {
			udpSender = std::make_unique<net::UDPClient>(net::ipToString(FLAME_Protocol::mcu_ip), FLAME_PROTOCOL_UDP_TARGET_PORT);
			LOG_INFO("Discovered client at {}", net::ipToString(FLAME_Protocol::mcu_ip));
		}

		updateValues();

		if (udpSender) {
			FLAME_Protocol::sendControlPacket();
			LOG_WARN("Sending");
		}
	}

}

void FlameTest() {

	LOG_INFO("Constructing FLAME backend");
	NETWORKING_SET_LOGLEVEL(spdlog::level::warn);

	LOG_INFO("Sending discovery packets");
	sendDiscoveryPackets();

	FLAME_Protocol::mcu_ip = net::ipToBytes("10.0.0.50");

	while (true) {
		update();
		std::this_thread::sleep_for(std::chrono::microseconds(interval / 10));
	}

}
