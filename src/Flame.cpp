
#include <string>
#include <iostream>
#include <chrono>
#include <sstream>

#include "Log.h"
#include "Flame.h"
#include "FLAME_Protocol.h"
#include "NetLib.h"

FLAME_Protocol::FLAME_Instance flameInstance;

NetLib::UDPServer server(flameInstance.receiverPort);

uint32_t getMicros() {
	using namespace std::chrono;
	return (uint32_t)duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

uint32_t getLocalIP() {
    //return NetLib::ipToBytes(server.GetLocalIP());
    return NetLib::ipToBytes("10.20.85.208");
}

void writeUDP(uint32_t targetIP, uint16_t targetPort, uint8_t* data, uint8_t length) {
    NetLib::SendUDP(targetIP, targetPort, data, length);
}

void updateUDP() {

    // Receive the packet
    auto packet = server.ReceivePacket();
    if (packet) {
        //FLAME_Protocol::PacketReceived(&flameInstance, &packet->data[0], (uint8_t)packet->data.size(), NetLib::ipToBytes(packet->remoteIP));
        FLAME_Protocol::PacketReceived(&flameInstance, &packet->data[0], (uint8_t)packet->data.size(), NetLib::ipToBytes("176.85.20.10"));
    }

    // Send the Review packet
    FLAME_Protocol::UpdateReviewStream(&flameInstance);
}

void FlameTest() {

	while (true) {
        updateUDP();
	}

}

