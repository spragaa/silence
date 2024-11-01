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

class FileServer {
public:
	FileServer(
		uint16_t port = 55544,
		unsigned int threads = 16,
		const std::string& storage_dir = std::string(SOURCE_DIR) + "/file_server/media_file_system/",
		size_t max_file_size = 10000000
		);
	~FileServer();
	
	void init();
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
	unsigned int _thread_count;
	std::string _storage_dir;
	size_t _max_file_size; // 10mb by default -> document it
};

}