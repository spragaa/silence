#pragma once

#include "debug.hpp"

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/mime.h>
#include <fstream>
#include <vector>
#include <filesystem>
#include <mutex>
#include <shared_mutex>

namespace file_server {

constexpr size_t CHUNK_SIZE_BYTES = 512;
constexpr uint8_t FILENAME_LEN = 16;
    
constexpr std::array<char, 62> ALPHABET = {
'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
'y', 'z'
};    
    
class FileServer {
public:
	FileServer(
		uint16_t port = 55544,
		unsigned int threads = 16,
		const std::string& storage_dir = std::string(SOURCE_DIR) + "/file_server/media_file_system/",
		size_t max_file_size = 10000000
		);
	~FileServer();
	
	void start();
	void stop();
	bool is_valid_filename(const std::string& filename) const;

public:
	static const std::string UPLOAD_ROUTE;
	static const std::string DOWNLOAD_ROUTE;
	static const std::string DELETE_ROUTE;
	static const std::string LIST_ROUTE;

private:
	void setup_routes();
	void upload_file(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
	void download_file(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
	void delete_file(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

	std::filesystem::path get_filepath_by_name(const std::string& filename) const;

private:
	std::shared_ptr<Pistache::Http::Endpoint> _http_endpoint;
	Pistache::Rest::Router _router;
	std::shared_mutex _file_system_mutex;
	std::unordered_map<std::string, std::unique_ptr<std::shared_mutex>> _file_mutexes;
	std::mutex _file_mutexes_map_mutex;
    bool _initialized = false;
    std::mutex _init_mutex;
	unsigned int _thread_count;
	unsigned short _server_port;
	std::string _storage_dir;
	size_t _max_file_size;
};

}