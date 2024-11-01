#include "file_server.hpp"
#include <cstddef>

namespace file_server {

inline const std::string FileServer::UPLOAD_ROUTE = "/upload/:filename";
inline const std::string FileServer::DOWNLOAD_ROUTE = "/download/:filename";
inline const std::string FileServer::DELETE_ROUTE = "/delete/:filename";
inline const std::string FileServer::LIST_ROUTE = "/list";

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

FileServer::FileServer(uint16_t port, unsigned int threads, const std::string& storage_dir, size_t max_file_size)
	: _http_endpoint(std::make_shared<Pistache::Http::Endpoint>(Pistache::Address("*:" + std::to_string(port))))
	, _threads(threads)
	, _storage_dir(storage_dir)
	, _max_file_size(max_file_size)
{
	std::filesystem::create_directory(_storage_dir);
	INFO_MSG("[FileServer::FileServer] File server created. Port: " + std::to_string(port) + ", with " + std::to_string(threads) + " threads");
	INFO_MSG("[FileServer::FileServer] Storage dir: " + _storage_dir + ", with max file size of: " + std::to_string(_max_file_size) + " bytes");
}

void FileServer::init() {
	auto opts = Pistache::Http::Endpoint::options()
	            .threads(_threads);
	_http_endpoint->init(opts);
	INFO_MSG("[FileServer::init] Initialization successful!");
	setup_routes();
}

void FileServer::start() {
	_http_endpoint->setHandler(_router.handler());
	_http_endpoint->serve();
	INFO_MSG("[FileServer::start] File server started");
}

void FileServer::setup_routes() {
	using namespace Pistache::Rest;

	Routes::Post(_router, UPLOAD_ROUTE, Routes::bind(&FileServer::upload_file, this));
	Routes::Get(_router, DOWNLOAD_ROUTE, Routes::bind(&FileServer::download_file, this));
	Routes::Delete(_router, DELETE_ROUTE, Routes::bind(&FileServer::delete_file, this));
	Routes::Get(_router, LIST_ROUTE, Routes::bind(&FileServer::list_files, this));

	INFO_MSG("[FileServer::setup_routes] Routes created:\n" + UPLOAD_ROUTE + "\n" + DOWNLOAD_ROUTE + "\n" + DELETE_ROUTE + "\n" + LIST_ROUTE);
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

// send incorrect amount of chunks and bytes too
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
	
	std::shared_lock<std::shared_mutex> file_lock(file_mutex); // exclusive write lock for specific file 

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

// should iterate recursively
// why do we need this one?
// this will take a lot of time, and doesn't make any sense
// maybe we can use it for file system status?
void FileServer::list_files(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
	DEBUG_MSG("[FileServer::list_files] Entering list_files function");

	auto start_time = std::chrono::high_resolution_clock::now();

	std::string file_list;
	int file_count = 0;

	for (const auto& entry : std::filesystem::directory_iterator(_storage_dir)) {
		file_list += entry.path().filename().string() + "\n";
		file_count++;
	}

	DEBUG_MSG("[FileServer::list_files] Found " + std::to_string(file_count) << " files");
	DEBUG_MSG("[FileServer::list_files] Sending 'Pistache::Http::Code::Ok' response");

	response.send(Pistache::Http::Code::Ok, file_list);
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