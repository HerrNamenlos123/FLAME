
#include "networking.h"

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif

#ifdef _WIN32
	#define _WIN32_WINNT _WIN32_WINNT_WIN10		// This sets the asio winsock library to Windows 10
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
#else   // ELSE _WIN32
	#include <netinet/in.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <ifaddrs.h>
	#include <arpa/inet.h>
#endif  // END _WIN32

#include <asio.hpp>
using asio::ip::udp;

#ifdef _WIN32
	#include <iphlpapi.h>
	#pragma comment(lib, "iphlpapi.lib")
#endif

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
using namespace std::placeholders;



static std::vector<std::string> SplitString(std::string str, char delimeter) {
	std::string::size_type b = 0;
	std::vector<std::string> result;

	while ((b = str.find_first_not_of(delimeter, b)) != std::string::npos) {
		auto e = str.find_first_of(delimeter, b);
		result.push_back(str.substr(b, e - b));
		b = e;
	}

	return result;
}

static std::string JoinStrings(std::vector<std::string> strings, std::string spacer) {
	std::string str = "";

	for (size_t i = 0; i < strings.size(); i++) {
		str += strings[i];

		if (i < strings.size() - 1 && spacer != "") {
			str += spacer;
		}
	}

	return str;
}







void NETWORKING_INIT_LOGGER() {
	if (!net::netlogger) {
		spdlog::set_pattern("%^[%T] %n: %v%$");
		net::netlogger = spdlog::stdout_color_mt("networking");
		net::netlogger->set_level(spdlog::level::info);
	}
}

namespace net {


	// ==========================
	// ===      Logging       ===
	// ==========================

	std::shared_ptr<spdlog::logger> netlogger;



	// ==========================
	// ===      SendUDP       ===
	// ==========================

	bool sendUDP(const asio::ip::address& ipAddress, uint16_t port, uint8_t* data, size_t length, bool broadcastPermissions) {
		try {
			NETWORKING_LOG_DEBUG("[SendUDP()]: Connecting to {}:{}", ipAddress.to_string(), port);

			// Create the socket
			asio::io_service ioService;
			udp::socket socket(ioService);
			udp::endpoint remote_endpoint(udp::endpoint(ipAddress, port));
			socket.open(udp::v4());

			if (broadcastPermissions) {
				NETWORKING_LOG_INFO("[SendUDP]: Connected with broadcast permissions");
				socket.set_option(asio::ip::udp::socket::reuse_address(true));
        		socket.set_option(asio::socket_base::broadcast(true));
			}

			// Send the data
			socket.send_to(asio::buffer(data, length), remote_endpoint);

			std::string str = "";
			for (size_t i = 0; i < length; i++) {
				str += std::to_string(data[i]);
				str += ", ";
			}
			str.pop_back();
			str.pop_back();
			NETWORKING_LOG_INFO("[SendUDP()]: Packet sent to {}:{}", ipAddress.to_string(), port);
			NETWORKING_LOG_TRACE("[SendUDP()]: Packet sent to {}:{} -> [{}] -> \"{}\"", ipAddress.to_string(), port, str, std::string((const char*)data, length));

			// Close the socket
			socket.close();
			NETWORKING_LOG_DEBUG("[SendUDP()]: Operation successful");

			return true;
		}
		catch (std::exception& e) {
			NETWORKING_LOG_WARN("[SendUDP()]: ASIO Exception: {}", e.what());
		}

		return false;
	}

	bool sendUDP(uint32_t ipAddress, uint16_t port, uint8_t* data, size_t length, bool broadcastPermissions) {
		return sendUDP(asio::ip::address_v4(ipAddress), port, data, length, broadcastPermissions);
	}

	bool sendUDP(uint32_t ipAddress, uint16_t port, const char* data, bool broadcastPermissions) {
		return sendUDP(asio::ip::address_v4(ipAddress), port, (uint8_t*)data, strlen(data), broadcastPermissions);
	}

	bool sendUDP(uint32_t ipAddress, uint16_t port, const std::string& data, bool broadcastPermissions) {
		return sendUDP(asio::ip::address_v4(ipAddress), port, (uint8_t*)data.c_str(), data.length(), broadcastPermissions);
	}

	bool sendUDP(const std::string& ipAddress, uint16_t port, uint8_t* data, size_t length, bool broadcastPermissions) {
		return sendUDP(asio::ip::address::from_string(ipAddress), port, data, length, broadcastPermissions);
	}

	bool sendUDP(const std::string& ipAddress, uint16_t port, const char* data, bool broadcastPermissions) {
		return sendUDP(asio::ip::address::from_string(ipAddress), port, (uint8_t*)data, strlen(data), broadcastPermissions);
	}

	bool sendUDP(const std::string& ipAddress, uint16_t port, const std::string& data, bool broadcastPermissions) {
		return sendUDP(asio::ip::address::from_string(ipAddress), port, (uint8_t*)data.c_str(), data.length(), broadcastPermissions);
	}






	// ==================================
	// ===      UDPClient Class       ===
	// ==================================

	struct UDPClientMembers {
		asio::io_service ioService;
		udp::socket socket;
		udp::endpoint remote_endpoint;

		UDPClientMembers() : socket(ioService) {}
		~UDPClientMembers() = default;
	};

	UDPClient::UDPClient(const std::string& ipAddress, uint16_t port, bool broadcastPermission, bool suppressLogging) : members(new UDPClientMembers()) {
		this->suppressLogging = suppressLogging;
		try {
			members->remote_endpoint = udp::endpoint(asio::ip::address::from_string(ipAddress), port);
			members->socket.open(udp::v4());

			if (broadcastPermission) {
				if (!suppressLogging) NETWORKING_LOG_INFO("[UDPClient]: Constructing instance with broadcast permissions");
				members->socket.set_option(asio::ip::udp::socket::reuse_address(true));
        		members->socket.set_option(asio::socket_base::broadcast(true));
			}
			if (!suppressLogging) 
				NETWORKING_LOG_DEBUG("[UDPClient]: Instance constructed, pointing to {}:{}", ipAddress, port);
		}
		catch (std::exception& e) {
			throw std::runtime_error(std::string("ASIO Exception: ") + e.what());
		}
	}

	UDPClient::~UDPClient() {
		members->socket.close();
		if (!suppressLogging) NETWORKING_LOG_DEBUG("[UDPClient]: Instance destructed");
	}

	size_t UDPClient::send(uint8_t* data, size_t length) {

		try {
			size_t bytes = members->socket.send_to(asio::buffer(data, length), members->remote_endpoint);

#ifndef DEPLOY
			logPacket(data, length, members->remote_endpoint.address().to_string().c_str(), members->remote_endpoint.port());
#endif

			return bytes;
		}
		catch (std::exception& e) {
			throw std::runtime_error(std::string("ASIO Exception: ") + e.what());
		}
	}

	size_t UDPClient::send(const char* data) {
		return send((uint8_t*)data, strlen(data));
	}

	size_t UDPClient::send(const std::string& data) {
		return send((uint8_t*)data.c_str(), data.length());
	}

    void UDPClient::logPacket(uint8_t* data, size_t length, const std::string& ipAddress, uint16_t port) {
        std::string str = "";
        for (size_t i = 0; i < length; i++) {
            str += std::to_string(data[i]);
            str += ", ";
        }
        str.pop_back();
        str.pop_back();
		if (!suppressLogging) NETWORKING_LOG_INFO("[UDPClient]: Packet sent to {}:{}", ipAddress, port);
		if (!suppressLogging) 
			NETWORKING_LOG_TRACE("[UDPClient]: Packet sent to {}:{} -> [{}] -> \"{}\"", 
				ipAddress, port, str, std::string((const char*)data, length));
    }







	// =======================================
	// ===      UDPServerAsync Class       ===
	// =======================================

	struct UDPServerAsyncMembers {

		asio::io_service ioService;
		udp::socket socket;
		udp::endpoint remoteEndpoint;

		bool terminate = false;
		std::thread listenerThread;
		std::function<void(uint8_t* packet, size_t packetSize)> callback;
		std::function<void(uint8_t* packet, size_t packetSize, const std::string& remoteHost, uint16_t remotePort)> callbackWithHost;

		std::vector<uint8_t> buffer;
		size_t bufferSize = 0;

		UDPServerAsyncMembers(const udp::endpoint& endpoint) : socket(ioService, endpoint) {}
		~UDPServerAsyncMembers() = default;
	};

	UDPServerAsync::UDPServerAsync(std::function<void(uint8_t* packet, size_t packetSize)> callback, 
		uint16_t port, size_t bufferSize, bool suppressLogging)
		: members(new UDPServerAsyncMembers(udp::endpoint(udp::v4(), port)))
	{
		this->suppressLogging = suppressLogging;
		members->callback = callback;
		Initialize(bufferSize);
	}

	UDPServerAsync::UDPServerAsync(
		std::function<void(uint8_t* packet, size_t packetSize, const std::string& remoteHost, uint16_t remotePort)> callback, 
		uint16_t port, size_t bufferSize, bool suppressLogging)
		: members(new UDPServerAsyncMembers(udp::endpoint(udp::v4(), port)))
	{
		this->suppressLogging = suppressLogging;
		members->callbackWithHost = callback;
		Initialize(bufferSize);
	}

	UDPServerAsync::~UDPServerAsync() {
		if (!suppressLogging) NETWORKING_LOG_DEBUG("[UDPServerAsync]: Terminating UDP listener");

		// Set the terminate flag and wait until the listener thread returns
		members->terminate = true;
		members->socket.close();
		members->listenerThread.join();

		if (!suppressLogging) NETWORKING_LOG_DEBUG("[UDPServerAsync]: Instance destructed");
	}

	std::string UDPServerAsync::getLocalIP() {
		return members->socket.local_endpoint().address().to_string();
	}

	void UDPServerAsync::Initialize(size_t bufferSize) {
		try {
			if (!suppressLogging) NETWORKING_LOG_DEBUG("[UDPServerAsync]: Creating UDP listener ...");

			// Initialize the buffer
			members->bufferSize = bufferSize;
			members->buffer.clear();
			members->buffer.reserve(bufferSize);
			for (size_t i = 0; i < bufferSize; i++) {
				members->buffer.push_back(0);
			}
			memset(&members->buffer[0], 0, bufferSize);

			// Start the listener thread
			members->listenerThread = std::thread(std::bind(&UDPServerAsync::ListenerThread, this));

			if (!suppressLogging) NETWORKING_LOG_DEBUG("[UDPServerAsync]: Instance constructed");
		}
		catch (std::exception& e) {
			throw std::runtime_error(std::string("ASIO Exception: ") + e.what());
		}
	}

	void UDPServerAsync::OnReceive(const std::error_code& error, size_t bytes) {
		if (!error) {

			if (!suppressLogging) NETWORKING_LOG_DEBUG("[UDPServerAsync]: Packet received, calling client callback");
			std::string remoteHost = members->remoteEndpoint.address().to_string();

#ifndef DEPLOY
			logPacket(&members->buffer[0], bytes, remoteHost.c_str(), members->remoteEndpoint.port());
#endif

			if (members->callback) {
				members->callback(&members->buffer[0], bytes);
			}
			if (members->callbackWithHost) {
				members->callbackWithHost(&members->buffer[0], bytes, remoteHost, members->remoteEndpoint.port());
			}

		}
		else {

			if (members->terminate)		// Errors are ignored if thread is being terminated
				return;

			if (!suppressLogging) 
				NETWORKING_LOG_WARN("[UDPServerAsync]: Error " + std::to_string(error.value()) + ": " + error.message());
		}

		// Start listening for the next packet
		StartAsyncListener();
	}

	void UDPServerAsync::StartAsyncListener() {
		try {
			members->socket.async_receive_from(asio::buffer(&members->buffer[0], members->bufferSize), members->remoteEndpoint,
				std::bind(&UDPServerAsync::OnReceive, this, _1, _2));
			if (!suppressLogging) NETWORKING_LOG_DEBUG("[UDPServerAsync]: Async listener started");
		}
		catch (std::exception& e) {
			throw std::runtime_error(std::string("ASIO Exception: ") + e.what());
		}
	}

	void UDPServerAsync::ListenerThread() {

		if (!suppressLogging) NETWORKING_LOG_DEBUG("[UDPServerAsync]: Listener thread started");

		try {

			// Start listener once
			StartAsyncListener();

			// Main loop in the listener thread
			while (!members->terminate) {
				members->ioService.run_one();
			}

		}
		catch (std::exception& e) {
			if (!suppressLogging) 
				NETWORKING_LOG_CRITICAL(std::string("ASIO UDP Exception from listener thread: ") + e.what());
		}
		catch (...) {
			if (!suppressLogging) NETWORKING_LOG_CRITICAL("[UDPServerAsync]: Unknown exception from listener thread!");
		}

		if (!suppressLogging) NETWORKING_LOG_DEBUG("[UDPServerAsync]: Listener thread terminated");
	}

	void UDPServerAsync::logPacket(uint8_t* data, size_t length, const std::string& ipAddress, uint16_t port) {
		std::string str = "";
		for (size_t i = 0; i < length; i++) {
			str += std::to_string(data[i]);
			str += ", ";
		}
		str.pop_back();
		str.pop_back();
		if (!suppressLogging) NETWORKING_LOG_INFO("[UDPServerAsync]: Packet received from {}:{}", ipAddress, port);
		if (!suppressLogging) 
			NETWORKING_LOG_TRACE("[UDPServerAsync]: Packet received from {}:{} -> [{}] -> \"{}\"", 
				ipAddress, port, str, std::string((const char*)data, length));
	}








	// ==================================
	// ===      UDPServer Class       ===
	// ==================================

	UDPServer::UDPServer(uint16_t port, size_t bufferSize, bool suppressLogging)
		: server(std::function<void(uint8_t* packet, size_t packetSize, const std::string& remoteIP, uint16_t remotePort)>(
			std::bind(&UDPServer::OnReceive, this, _1, _2, _3, _4)), port, bufferSize, suppressLogging)
	{
		this->suppressLogging = suppressLogging;
		if (!suppressLogging) NETWORKING_LOG_DEBUG("[UDPServer]: Instance constructed");
	}

	UDPServer::~UDPServer() {
		if (!suppressLogging) NETWORKING_LOG_DEBUG("[UDPServer]: Instance destructed");
	}

	size_t UDPServer::available() {
		std::lock_guard<std::mutex> guard(bufferMutex);
		return packetBuffer.size();
	}

	std::optional<NetworkPacket> UDPServer::peek() {
		std::lock_guard<std::mutex> guard(bufferMutex);

		if (packetBuffer.empty())
			return std::nullopt;

		return std::make_optional(packetBuffer.front());
	}

	std::optional<NetworkPacket> UDPServer::read() {
		std::lock_guard<std::mutex> guard(bufferMutex);

		if (packetBuffer.empty())
			return std::nullopt;

		auto first = packetBuffer.front();
		packetBuffer.pop();
		return std::make_optional(first);
	}

	std::string UDPServer::getLocalIP() {
		return server.getLocalIP();
	}

	void UDPServer::OnReceive(uint8_t* packet, size_t packetSize, const std::string& remoteIP, uint16_t remotePort) {
		std::lock_guard<std::mutex> guard(bufferMutex);

		if (packetBuffer.size() >= NETWORKING_MAX_PACKET_COUNT)
			return;

		NetworkPacket p;
		p.data.reserve(packetSize);
		for (size_t i = 0; i < packetSize; i++) {
			p.data.push_back(packet[i]);
		}
		p.remoteIP = remoteIP;
		p.remotePort = remotePort;

		packetBuffer.push(std::move(p));
	}








	// ==========================================
	// ===      UDPServerBlocking Class       ===
	// ==========================================

	struct UDPServerBlockingMembers {

		asio::io_service ioService;
		udp::socket socket;

		std::vector<uint8_t> buffer;

		UDPServerBlockingMembers(const udp::endpoint& endpoint) : socket(ioService, endpoint) {}
		~UDPServerBlockingMembers() = default;
	};

	UDPServerBlocking::UDPServerBlocking(uint16_t port, size_t bufferSize, bool suppressLogging)
		: members(new UDPServerBlockingMembers(udp::endpoint(udp::v4(), port)))
	{
		this->suppressLogging = suppressLogging;
		try {
			if (!suppressLogging) NETWORKING_LOG_DEBUG("[UDPServerBlocking]: Creating UDP listener ...");

			// Initialize the buffer
			members->buffer.clear();
			members->buffer.reserve(bufferSize);
			for (size_t i = 0; i < bufferSize; i++) {
				members->buffer.push_back(0);
			}

			if (!suppressLogging) NETWORKING_LOG_DEBUG("[UDPServerBlocking]: Instance constructed");
		}
		catch (std::exception& e) {
			throw std::runtime_error(std::string("ASIO Exception: ") + e.what());
		}
	}

	UDPServerBlocking::~UDPServerBlocking() {

		members->socket.close();

		if (!suppressLogging) NETWORKING_LOG_DEBUG("[UDPServerBlocking]: Instance destructed");
	}

	std::optional<std::vector<uint8_t>> UDPServerBlocking::read() {

		udp::endpoint remote_endpoint;
		std::error_code error;
		members->socket.receive_from(asio::buffer(members->buffer), remote_endpoint, 0, error);

		logPacket(
			&members->buffer[0],
			members->buffer.size(),
			members->socket.remote_endpoint().address().to_string(),
			members->socket.remote_endpoint().port()
		);

		if (error && error != asio::error::message_size) {
			return std::nullopt;
		}

		return std::make_optional(members->buffer);
	}

	void UDPServerBlocking::logPacket(uint8_t* data, size_t length, const std::string& ipAddress, uint16_t port) {
		std::string str = "";
		for (size_t i = 0; i < length; i++) {
			str += std::to_string(data[i]);
			str += ", ";
		}
		str.pop_back();
		str.pop_back();
		if (!suppressLogging) NETWORKING_LOG_INFO("[UDPServerBlocking]: Packet received from {}:{}", ipAddress, port);
		if (!suppressLogging)
			NETWORKING_LOG_TRACE("[UDPServerBlocking]: Packet received from {}:{} -> [{}] -> \"{}\"", 
				ipAddress, port, str, std::string((const char*)data, length));
	}




	// ===============================================
	// ===      Network interfaces utilities       ===
	// ===============================================

	std::vector<NetworkInterface> getNetworkInterfaces() {
		std::vector<NetworkInterface> interfaces;

#ifdef _WIN32
		// Get the required buffer size
		DWORD bufferSize = 0;
		if (GetIpAddrTable(nullptr, &bufferSize, 0) != ERROR_INSUFFICIENT_BUFFER) {
			return std::vector<NetworkInterface>();
		}

		// Setup the address table
		std::vector<MIB_IPADDRTABLE> table;
		table.reserve(bufferSize / sizeof(MIB_IPADDRTABLE));
		for (size_t i = 0; i < table.capacity(); i++) {
			table.push_back(MIB_IPADDRTABLE());
		}

		// Retrieve the Win32 address table
		unsigned long size = (unsigned long)(table.size() * sizeof(MIB_IPADDRTABLE));
		DWORD ret = GetIpAddrTable(&table[0], &size, 0);
		if (ret != 0) {
			return std::vector<NetworkInterface>();
		}

		// Construct the address table
		for (size_t i = 0; i < (size_t)table[0].dwNumEntries; i++) {

			IN_ADDR IPAddr;
			NetworkInterface ifc;

			ifc.index = table[0].table[i].dwIndex;

			IPAddr.S_un.S_addr = (u_long)table[0].table[i].dwAddr;
			ifc.address = inet_ntoa(IPAddr);

			IPAddr.S_un.S_addr = (u_long)table[0].table[i].dwMask;
			ifc.subnet = inet_ntoa(IPAddr);

			IPAddr.S_un.S_addr = (u_long)table[0].table[i].dwBCastAddr;
			ifc.broadcast = inet_ntoa(IPAddr);

			ifc.reassemblySize = table[0].table[i].dwReasmSize;

			ifc.state = NONE;
			if (table[0].table[i].wType & MIB_IPADDR_PRIMARY)
				ifc.state |= InterfaceState::PRIMARY_IP;
			if (table[0].table[i].wType & MIB_IPADDR_DYNAMIC)
				ifc.state |= InterfaceState::DYNAMIC_IP;
			if (table[0].table[i].wType & MIB_IPADDR_DISCONNECTED)
				ifc.state |= InterfaceState::DISCONNECTED_INTERFACE;
			if (table[0].table[i].wType & MIB_IPADDR_DELETED)
				ifc.state |= InterfaceState::BEING_DELETED;
			if (table[0].table[i].wType & MIB_IPADDR_TRANSIENT)
				ifc.state |= InterfaceState::TRANSIENT;

			interfaces.push_back(ifc);
		}

#else   // ELSE _WIN32

		struct ifaddrs* ifaddr;     // Get all network interfaces
		if (getifaddrs(&ifaddr) == -1)
			return interfaces;

		// Loop through all network interfaces
		for (struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {

			if (ifa->ifa_addr == NULL)
				continue;

			if (ifa->ifa_addr->sa_family == AF_INET) {  // Only accept IPv4 addresses

				std::string address(INET_ADDRSTRLEN, '\0');
				std::string subnet(INET_ADDRSTRLEN, '\0');

				void* tmpAddrPtr = &((struct sockaddr_in*)(ifa->ifa_addr))->sin_addr;
				inet_ntop(AF_INET, tmpAddrPtr, &address[0], INET_ADDRSTRLEN);
				tmpAddrPtr = &((struct sockaddr_in*)(ifa->ifa_netmask))->sin_addr;
				inet_ntop(AF_INET, tmpAddrPtr, &subnet[0], INET_ADDRSTRLEN);

				NetworkInterface interface;
				interface.index = 0;
				interface.address = std::string(address);
				interface.name = std::string(ifa->ifa_name);
				interface.subnet = std::string(subnet);
				interface.reassemblySize = 0;
				interface.state = InterfaceState::NONE;

				interfaces.push_back(interface);
			}
		}

		freeifaddrs(ifaddr);

#endif  // END _WIN32

		return interfaces;
	}

	std::string createBroadcastAddress(const NetworkInterface& ifc) {
		auto ipParts = SplitString(ifc.address, '.');
		auto subParts = SplitString(ifc.subnet, '.');

		if (subParts.size() != 4 || ipParts.size() != 4) {  // Weird
			return "";
		}

		for (int i = 0; i < 4; i++) {
			if (subParts[i] != "255") {
				ipParts[i] = "255";
			}
		}

		return JoinStrings(ipParts, ".");
	}

	uint32_t ipToBytes(const std::string& ip) {
		return inet_addr(ip.c_str());
	}

}
