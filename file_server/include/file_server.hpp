#pragma once

#include "debug.hpp"

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/mime.h>
#include <fstream>
#include <vector>
#include <filesystem>

using namespace Pistache;
namespace fs = std::filesystem;

class FileServer {
public:
	FileServer(
		uint16_t port = 55544,
		unsigned int threads = 16,
		const std::string& storage_dir = std::string(SOURCE_DIR) + "/file_server/media_file_system/",
		size_t max_file_size = 10000000
		);

	void init();
	void start();

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
	void list_files(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

	void generate_folder_structure();
	bool is_valid_filename(const std::string& filename) const;
	fs::path get_filepath_by_name(const std::string& filename) const;

private:
	std::shared_ptr<Pistache::Http::Endpoint> _http_endpoint;
	Pistache::Rest::Router _router;
	unsigned int _threads;
	std::string _storage_dir;
	size_t _max_file_size; // 10mb by default -> document it
};