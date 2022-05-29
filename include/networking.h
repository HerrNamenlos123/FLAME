#pragma once

#include <cstddef>      // size_t
#include <cinttypes>    // uint8_t, ...
#include <string>       // std::string
#include <exception>    // std::exception
#include <functional>   // std::function
#include <system_error> // std::error_code
#include <optional>
#include <atomic>	
#include <mutex>		
#include <utility>		// std::pair
#include <queue>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"

/// <summary>
/// Great care was taken that the asio headers are only included in the source file. Keeping the asio headers
/// away from the networking headers reduces compile time significantly and makes it compatible with virtually everything.
/// (No issues with winsock/winsock2 conflicts). This is achieved by outsourcing the member variables to a struct pointer, 
/// wrapped in the IncompleteTypeWrapper to make it safe.
/// </summary>

static constexpr auto NETWORKING_DEFAULT_UDP_BUFFER_SIZE = 1024;
static constexpr auto NETWORKING_MAX_PACKET_COUNT = 50;

#define NETWORKING_SET_LOGLEVEL(...)	{ NETWORKING_INIT_LOGGER(); net::netlogger->set_level(__VA_ARGS__);		}

#define NETWORKING_LOG_TRACE(...)					{ NETWORKING_INIT_LOGGER(); net::netlogger->trace(__VA_ARGS__);		}
#define NETWORKING_LOG_WARN(...)					{ NETWORKING_INIT_LOGGER(); net::netlogger->warn(__VA_ARGS__);		}
#define NETWORKING_LOG_DEBUG(...)					{ NETWORKING_INIT_LOGGER(); net::netlogger->debug(__VA_ARGS__);		}
#define NETWORKING_LOG_INFO(...)					{ NETWORKING_INIT_LOGGER(); net::netlogger->info(__VA_ARGS__);		}
#define NETWORKING_LOG_ERROR(...)					{ NETWORKING_INIT_LOGGER(); net::netlogger->error(__VA_ARGS__);		}
#define NETWORKING_LOG_CRITICAL(...)				{ NETWORKING_INIT_LOGGER(); net::netlogger->critical(__VA_ARGS__);	}

void NETWORKING_INIT_LOGGER();

namespace net {

	// ==========================
	// ===      Logging       ===
	// ==========================

	extern std::shared_ptr<spdlog::logger> netlogger;





    template<typename T>
	class IncompleteTypeWrapper {
	public:
		IncompleteTypeWrapper(T* data) : data(data) {}
		~IncompleteTypeWrapper() { delete data; }
		T* operator->() { return data; }
		T* get() { return data; }

		IncompleteTypeWrapper(const IncompleteTypeWrapper&) = delete;
		IncompleteTypeWrapper& operator=(const IncompleteTypeWrapper&) = delete;

    private:
		T* data;
	};






	// ==================================
	// ===      NetLib::SendUDP       ===
	// ==================================
	//
	// This function simply creates a UDP socket, sends the message and closes it again. The principle is
	// the same as in the UDPClient class. The difference is that the UDPClient keeps the socket open while
	// the object is alive. Use this function for sending a few packets sporadically.
	//
	bool sendUDP(uint32_t ipAddress, uint16_t port, uint8_t* data, size_t length, bool broadcastPermissions = false);
	bool sendUDP(uint32_t ipAddress, uint16_t port, const char* data, bool broadcastPermissions = false);
	bool sendUDP(uint32_t ipAddress, uint16_t port, const std::string& data, bool broadcastPermissions = false);

	bool sendUDP(const std::string& ipAddress, uint16_t port, uint8_t* data, size_t length, bool broadcastPermissions = false);
	bool sendUDP(const std::string& ipAddress, uint16_t port, const char* data, bool broadcastPermissions = false);
	bool sendUDP(const std::string& ipAddress, uint16_t port, const std::string& data, bool broadcastPermissions = false);

    


	// ==================================
	// ===      UDPClient Class       ===
	// ==================================
	//
	// This class creates a UDP socket and keeps it alive for the lifetime of the object. 
	// Use this class for streaming a lot of packets to the same IP and port.
	
    struct UDPClientMembers;

	class UDPClient {
	public:
		UDPClient(const std::string& ipAddress, uint16_t port, bool broadcastPermission = false, bool suppressLogging = false);
		~UDPClient();

		size_t send(uint8_t* data, size_t length);
		size_t send(const char* data);
		size_t send(const std::string& data);

		std::string ip();

    private:
        void logPacket(uint8_t* data, size_t length, const std::string& ipAddress, uint16_t port);

        IncompleteTypeWrapper<UDPClientMembers> members;
		bool suppressLogging = false;
	};






	// =======================================
	// ===      UDPServerAsync Class       ===
	// =======================================

	struct UDPServerAsyncMembers;

	class UDPServerAsync {
	public:
		UDPServerAsync(
			std::function<void(uint8_t* packet, size_t packetSize)> callback,
			uint16_t port,
			size_t bufferSize = NETWORKING_DEFAULT_UDP_BUFFER_SIZE, 
			bool suppressLogging = false
		);

		UDPServerAsync(
			std::function<void(uint8_t* packet, size_t packetSize, const std::string& remoteHost, uint16_t remotePort)> callback,
			uint16_t port,
			size_t bufferSize = NETWORKING_DEFAULT_UDP_BUFFER_SIZE, 
			bool suppressLogging = false
		);

		~UDPServerAsync();

		std::string getLocalIP();

	private:
		void Initialize(size_t bufferSize);
		void OnReceive(const std::error_code& error, size_t bytes);
		void StartAsyncListener();
		void ListenerThread();

		void logPacket(uint8_t* data, size_t length, const std::string& ipAddress, uint16_t port);

		IncompleteTypeWrapper<UDPServerAsyncMembers> members;
		bool suppressLogging = false;

	};






	// ==================================
	// ===      UDPServer Class       ===
	// ==================================

	struct NetworkPacket {
		std::vector<uint8_t> data;
		std::string remoteIP;
		uint16_t remotePort = 0;
	};

	class UDPServer {
	public:
		UDPServer(uint16_t port, size_t bufferSize = NETWORKING_DEFAULT_UDP_BUFFER_SIZE, bool suppressLogging = false);
		~UDPServer();

		size_t available();
		std::optional<NetworkPacket> peek();
		std::optional<NetworkPacket> read();
		std::string getLocalIP();

	private:
		void OnReceive(uint8_t* packet, size_t packetSize, const std::string& remoteIP, uint16_t remotePort);

		UDPServerAsync server;
		std::mutex bufferMutex;
		std::queue<NetworkPacket> packetBuffer;
		bool suppressLogging = false;

	};






	// ==========================================
	// ===      UDPServerBlocking Class       ===
	// ==========================================

	struct UDPServerBlockingMembers;

	class UDPServerBlocking {
	public:
		UDPServerBlocking(uint16_t port, size_t bufferSize = NETWORKING_DEFAULT_UDP_BUFFER_SIZE, bool suppressLogging = false);
		~UDPServerBlocking();

		std::optional<std::vector<uint8_t>> read();

	private:
		void logPacket(uint8_t* data, size_t length, const std::string& ipAddress, uint16_t port);

		IncompleteTypeWrapper<UDPServerBlockingMembers> members;
		bool suppressLogging = false;

	};





	// ===============================================
	// ===      Network interfaces utilities       ===
	// ===============================================

	enum InterfaceState {
		NONE = 0,
		PRIMARY_IP = 1,
		DYNAMIC_IP = 2,
		DISCONNECTED_INTERFACE = 4,
		BEING_DELETED = 8,
		TRANSIENT = 16
	};

	struct NetworkInterface {
		size_t index;           // Windows only
		std::string address;
		std::string name;       // Non-windows only
		std::string subnet;     // Windows only
		std::string broadcast;  // Windows only
		size_t reassemblySize;  // Windows only
		int state;              // Windows only
	};

	std::vector<NetworkInterface> getNetworkInterfaces();

	std::string createBroadcastAddress(const NetworkInterface& ifc);

	std::string ipToString(uint32_t ip);
	uint32_t ipToBytes(const std::string& ip);

}






























/*

#pragma once

#include "Battery/pch.h"
#include "Battery/Core/Config.h"
#include "Battery/Utils/TypeUtils.h"

struct ALLEGRO_FILE;

namespace Battery {





	










	// =================================================
	// ===      File download / HTTP Utilities       ===
	// =================================================

	struct HttpResponse {
		std::string body;
		size_t status;
		std::string reason;

		HttpResponse(const std::string& body, size_t status, std::string reason) :
			body(body), status(status), reason(reason) {}
	};

	/// <summary>
	/// Splits an url into server hostname and server path. E.g: "https://www.google.at/my/page.html"
	///  -> "https://www.google.at" and "/my/page.html"
	/// </summary>
	std::pair<std::string, std::string> SplitUrl(const std::string& url);

	/// <summary>
	/// Do a HTTP GET request, redirects are automatically followed by default. Return value
	/// is empty when the server can't be reached, otherwise the body and HTTP code can be retrieved.
	/// </summary>
	std::optional<HttpResponse> GetHttpRequest(const std::string& url, bool followRedirect = true);

	/// <summary>
	/// Advanced HTTP GET request, use this only if you know what you're doing.
	/// </summary>
	std::optional<HttpResponse> GetHttpRequestChunked(
		const std::string& url,
		std::function<void()> onClearDataCallback,
		std::function<bool(const char*, size_t)> onReceiveCallback,
		std::optional<std::function<bool(uint64_t, uint64_t)>> onProgressCallback = std::nullopt,
		bool followRedirect = true);

	/// <summary>
	/// Download an online resource and return the buffer.
	/// OnProgress callback is optional and just for monitoring progress. Return false
	/// to cancel the download.
	/// </summary>
	std::string DownloadUrlToBuffer(
		const std::string& url, std::optional<std::function<bool(uint64_t, uint64_t)>> onProgressCallback = std::nullopt,
		bool followRedirect = true);

	/// <summary>
	/// Download an online resource and write it to disk under the given filename.
	/// OnProgress callback is optional and just for monitoring progress. Return false
	/// to cancel the download.
	/// </summary>
	bool DownloadUrlToFile(
		const std::string& url, const std::string& targetFile, bool binary = false,
		std::optional<std::function<bool(uint64_t, uint64_t)>> onProgressCallback = std::nullopt,
		bool followRedirect = true);
}*/