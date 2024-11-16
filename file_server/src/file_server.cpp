#include "file_server.hpp"

namespace file_server {

inline const std::string FileServer::UPLOAD_ROUTE = "/upload/:filename";
inline const std::string FileServer::DOWNLOAD_ROUTE = "/download/:filename";
inline const std::string FileServer::DELETE_ROUTE = "/delete/:filename";

FileServer::FileServer(uint16_t port, unsigned int thread_count, const std::string& storage_dir, size_t max_file_size)
	: _thread_count(thread_count)
	, _server_port(port)
	, _storage_dir(storage_dir)
	, _max_file_size(max_file_size)
{
	std::filesystem::create_directory(_storage_dir);
	INFO_MSG("[FileServer::FileServer] File server created. Port: " + std::to_string(port) + ", with " + std::to_string(thread_count) + " threads");
	INFO_MSG("[FileServer::FileServer] Storage dir: " + _storage_dir + ", with max file size of: " + std::to_string(_max_file_size) + " bytes");
}

FileServer::~FileServer() {
	stop();
}

void FileServer::start() {
	INFO_MSG("[FileServer::start] Trying to start the file server");
	std::lock_guard<std::mutex> lock(_init_mutex);
	if (_running) {
		INFO_MSG("[FileServer::start] Server already initialized, skipping this action")
		return;
	}

	try {
		auto opts = Pistache::Http::Endpoint::options()
		            .threads(_thread_count)
		            .flags(Pistache::Tcp::Options::ReuseAddr);

		_http_endpoint = std::make_unique<Pistache::Http::Endpoint>(
			Pistache::Address("*:" + std::to_string(_server_port))
			);
		_http_endpoint->init(opts);
		setup_routes();
		_running = true;
	} catch (const std::exception& e) {
		FATAL_MSG("[FileServer::start] Failed to initialize server: " + std::string(e.what()));
		throw std::runtime_error("Failed to initialize server: " + std::string(e.what()));
	} catch(...) {
		FATAL_MSG("[FileServer::start] Unknown error occured, shutting down");
		throw std::runtime_error("Unknown error occured, shutting down");
	}

	INFO_MSG("[FileServer::start] Initialization successful!");

	_http_endpoint->setHandler(_router->handler());
	_http_endpoint->serve();
	INFO_MSG("[FileServer::start] File server started");
}

void FileServer::stop() {
	std::lock_guard<std::mutex> lock(_init_mutex);
	if (_http_endpoint) {
		_http_endpoint->shutdown();

		if (!_router) {
			_router = std::make_shared<Pistache::Rest::Router>();
		}

		{
			std::lock_guard<std::mutex> lock(_file_mutexes_map_mutex);
			_file_mutexes.clear();
		}
		_running = false;
		_http_endpoint.reset();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

void FileServer::setup_routes() {
	using namespace Pistache::Rest;

	DEBUG_MSG("[FileServer::setup_routes] Setting up routes...");

	if (!_router) {
		_router = std::make_shared<Pistache::Rest::Router>();
	}

	Routes::Post(*_router, UPLOAD_ROUTE, Routes::bind(&FileServer::upload_file, this));
	Routes::Get(*_router, DOWNLOAD_ROUTE, Routes::bind(&FileServer::download_file, this));
	Routes::Delete(*_router, DELETE_ROUTE, Routes::bind(&FileServer::delete_file, this));

	INFO_MSG("[FileServer::setup_routes] Routes created:\n" + UPLOAD_ROUTE + "\n" + DOWNLOAD_ROUTE + "\n" + DELETE_ROUTE);
}

std::filesystem::path FileServer::get_filepath_by_name(const std::string& filename) const {
	std::string lvl1_dir = filename.substr(0, 2);
	std::string lvl2_dir = filename.substr(2, 2);

	std::filesystem::path directories_path = std::filesystem::path(_storage_dir) / lvl1_dir / lvl2_dir;
	std::filesystem::create_directories(directories_path);
	return directories_path / filename;
}

void FileServer::upload_file(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
	auto filename = request.param(":filename").as<std::string>();

	if (!is_valid_filename(filename)) {
		WARN_MSG("[FileServer::upload_file] File " + filename + " contains unsupported symbols or its size is incorrect, cannot upload it to file_server");
		response.send(Pistache::Http::Code::Bad_Request, "File " + filename + " contains unsupported symbols or its size is incorrect, cannot upload it to file_server");
		return;
	}

	std::unique_lock<std::shared_mutex> fs_lock(_file_system_mutex); // exclusive write lock for file system acces
	std::filesystem::path filepath = get_filepath_by_name(filename);
	// get or create mutex for specific file
	std::shared_mutex& file_mutex = [&]() -> std::shared_mutex& {
										std::lock_guard<std::mutex> map_lock(_file_mutexes_map_mutex);
										auto& mutex_ptr = _file_mutexes[filepath.string()];
										if (!mutex_ptr) {
											mutex_ptr = std::make_unique<std::shared_mutex>();
										}

										return *mutex_ptr;
									}();

	std::unique_lock<std::shared_mutex> file_lock(file_mutex); // exclusive write lock for specific file

	DEBUG_MSG("[FileServer::upload_file] Filepath is " + filepath.string());

	const std::string& body = request.body();

	size_t raw_data_bytes = body.size();
	if (raw_data_bytes > CHUNK_SIZE_BYTES) {
		response.send(Pistache::Http::Code::Bad_Request, "Received raw data size is bigger than acceptable chunk size!");
		WARN_MSG("[FileServer::upload_file] Received raw data size is bigger than acceptable chunk size. Removing it!");
		std::filesystem::remove(filepath);
		return;
	}

	size_t file_size = std::filesystem::exists(filepath) ? std::filesystem::file_size(filepath) : 0;
	if (file_size > _max_file_size - raw_data_bytes) {
		response.send(Pistache::Http::Code::Bad_Request, "Size of " + filepath.string() + " is: " + std::to_string(file_size) + ", that's more than system limit, removing it");
		WARN_MSG("[FileServer::upload_file] Size of " + filepath.string() + " is: " + std::to_string(file_size) + ", that's more than system limit, removing it");
		std::filesystem::remove(filepath);
		return;
	}

	DEBUG_MSG("[FileServer::upload_file] Upload file called, filepath:" + filepath.string() + ", body size: " + std::to_string(raw_data_bytes) + " bytes");

	std::ofstream file(filepath, std::ios::binary | std::ios::app);
	if (!file) {
		response.send(Pistache::Http::Code::Internal_Server_Error, "Failed to create file");
		WARN_MSG("[FileServer::upload_file] Failed to create file: " + filepath.string());
		return;
	}

	file.write(body.c_str(), raw_data_bytes);
	file.close();

	if (file.good()) {
		response.send(Pistache::Http::Code::Ok, "File uploaded successfully");
		INFO_MSG("[FileServer::upload_file] File " + filepath.string() +  " uploaded successfully");
	} else {
		std::filesystem::remove(filepath);
		response.send(Pistache::Http::Code::Internal_Server_Error, "Failed to write file");
		WARN_MSG("[FileServer::upload_file] Failed to write file: " + filepath.string());
	}
}

// sends incorrect amount of chunks and bytes
void FileServer::download_file(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
	auto filename = request.param(":filename").as<std::string>();

	std::shared_lock<std::shared_mutex> fs_lock(_file_system_mutex);
	auto filepath = get_filepath_by_name(filename);

	// get or create mutex for specific file
	std::shared_mutex& file_mutex = [&]() -> std::shared_mutex& {
										std::lock_guard<std::mutex> map_lock(_file_mutexes_map_mutex);
										auto it = _file_mutexes.find(filepath.string());
										if (it == _file_mutexes.end()) {
											_file_mutexes[filepath.string()] = std::make_unique<std::shared_mutex>();
										}

										return *_file_mutexes[filepath.string()];
									}();

	std::shared_lock<std::shared_mutex> file_lock(file_mutex); // shared read lock for specific file

	DEBUG_MSG("[FileServer::download_file] Download file called, filepath:" + filepath.string());

	if (!std::filesystem::exists(filepath)) {
		response.send(Pistache::Http::Code::Not_Found, "File not found");
		WARN_MSG("[FileServer::download_file] File " + filepath.string() +  " not found");
		return;
	}

	std::ifstream file(filepath, std::ios::binary);
	if (!file) {
		response.send(Pistache::Http::Code::Internal_Server_Error, "Failed to open file");
		WARN_MSG("[FileServer::download_file] Failed to open file: " + filepath.string());
		return;
	}

	response.setMime(Pistache::Http::Mime::MediaType::fromString("application/octet-stream"));
	std::vector<char> buffer(CHUNK_SIZE_BYTES);

	while (file) {
		file.read(buffer.data(), CHUNK_SIZE_BYTES);
		auto bytes_read = file.gcount();
		response.send(Pistache::Http::Code::Ok, std::string(buffer.data(), bytes_read));
	}
}

void FileServer::delete_file(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
	auto filename = request.param(":filename").as<std::string>();

	std::unique_lock<std::shared_mutex> fs_lock(_file_system_mutex);
	auto filepath = std::filesystem::path(_storage_dir) / filename;

	try {
		std::shared_mutex& file_mutex = [&]() -> std::shared_mutex& {
											std::lock_guard<std::mutex> map_lock(_file_mutexes_map_mutex);
											auto it = _file_mutexes.find(filepath.string());
											if (it == _file_mutexes.end()) {
												throw std::runtime_error("File not found");
											}

											return *(it->second);
										}();

		DEBUG_MSG("[FileServer::delete_file] Delete file called, filepath:" + filepath.string());

		std::unique_lock<std::shared_mutex> file_lock(file_mutex);

		if (!std::filesystem::exists(filepath)) {
			response.send(Pistache::Http::Code::Not_Found, "File not found");
			WARN_MSG("[FileServer::delete_file] File " + filepath.string() + " not found");
			return;
		}

		if (std::filesystem::remove(filepath)) {
			std::lock_guard<std::mutex> map_lock(_file_mutexes_map_mutex);
			_file_mutexes.erase(filepath.string());
			response.send(Pistache::Http::Code::Ok, "File deleted successfully");
			DEBUG_MSG("[FileServer::delete_file] File " + filepath.string() + " deleted successfully");
		} else {
			response.send(Pistache::Http::Code::Internal_Server_Error, "Failed to delete file");
			WARN_MSG("[FileServer::delete_file] Failed to delete file: " + filepath.string());
		}
	} catch (const std::runtime_error& re) {
		WARN_MSG("[FileServer::delete_file] File not found");
		response.send(Pistache::Http::Code::Not_Found, "File not found");
		WARN_MSG("[FileServer::delete_file] File " + filepath.string() + " not found");
		return;
	}
}

bool FileServer::is_valid_filename(const std::string& filename) const {
	std::filesystem::path file_path(filename);
	std::string str = file_path.stem().string();

	if (str.size() != FILENAME_LEN) {
		return false;
	}

	for (const char c1 : str) {
		bool is_valid = false;
		for (const char c2 : ALPHABET) {
			if (c1 == c2) {
				is_valid = true;
				break;
			}
		}

		if (!is_valid) {
			return false;
		}
	}
	return true;
}

}