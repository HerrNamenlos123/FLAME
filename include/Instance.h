#pragma once

#include <string>
#include <mutex>
#include <map>
#include <sstream>

static std::string toString(uint32_t ip) {
	std::stringstream ipStr;
	ipStr << (ip & 0xff) << ".";
	ipStr << ((ip >> 8) & 0xff) << ".";
	ipStr << ((ip >> 16) & 0xff) << ".";
	ipStr << ((ip >> 24) & 0xff);
	return ipStr.str();
}

namespace FLAME {

	class Instance {
	public:
		Instance() {}

		void setEndpoint(uint8_t endpointID, uint32_t data) {
			std::scoped_lock<std::mutex> lock(access);
			this->data[endpointID] = data;
		}

		uint32_t getEndpoint(uint8_t endpointID) {
			std::scoped_lock<std::mutex> lock(access);
			return this->data[endpointID];
		}

		void setClientIP(uint32_t ipAddress) {
			std::scoped_lock<std::mutex> lock(access);
			clientIP = toString(ipAddress);
		}

		std::string getClientIP() {
			std::scoped_lock<std::mutex> lock(access);
			return clientIP;
		}

	private:
		std::map<int, uint32_t> data;

		std::mutex access;
		std::string clientIP = "";
	};

}
