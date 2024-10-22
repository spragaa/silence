#include "file_server.hpp"

const std::string FileServer::UPLOAD_ROUTE = "/upload/:filename";
const std::string FileServer::DOWNLOAD_ROUTE = "/download/:filename";
const std::string FileServer::DELETE_ROUTE = "/delete/:filename";
const std::string FileServer::LIST_ROUTE = "/list";

constexpr size_t CHUNK_SIZE_BYTES = 512;
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
	fs::create_directory(_storage_dir);
	INFO_MSG("[FileServer::FileServer] File server created. Port: " + std::to_string(port) + ", with " + std::to_string(threads) + " threads");
	INFO_MSG("[FileServer::FileServer] Storage dir: " + _storage_dir + ", with max file size of: " + std::to_string(_max_file_size) + " bytes");
}

void FileServer::init() {
	auto opts = Pistache::Http::Endpoint::options()
	            .threads(_threads);
	_http_endpoint->init(opts);
	INFO_MSG("[FileServer::init] Initialization successful!");
	setup_routes();
	// generate_folder_structure();
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

// from one side it is ok that server initialization takes a while,
// but on other hand, we can create relative folders once we actually need them
void FileServer::generate_folder_structure() {
    fs::path root_dir(_storage_dir);
    
    for (const char c1 : ALPHABET) {
        for (const char c2 : ALPHABET) {
            fs::path dir_path = root_dir / (std::string(1, c1) + std::string(1, c2));
            fs::create_directories(dir_path);

            for (const char sub_c1 : ALPHABET) {
                for (const char sub_c2 : ALPHABET) {
                    fs::path sub_dir_path = dir_path / (std::string(1, sub_c1) + std::string(1, sub_c2));
                    fs::create_directories(sub_dir_path);
                }
            }
        }    
    }
}

void FileServer::upload_file(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
	auto filename = request.param(":filename").as<std::string>();
	auto filepath = fs::path(_storage_dir) / filename;
	
	const std::string& body = request.body();
	
	size_t raw_data_bytes = body.size();
	if (raw_data_bytes > CHUNK_SIZE_BYTES) {
		response.send(Pistache::Http::Code::Bad_Request, "Received raw data size is bigger then acceptable chunk size!");
		WARN_MSG("[FileServer::upload_file] Received raw data size is bigger then acceptable chunk size!");
		// remove file add message about fail
		return;
	}
	
	size_t file_size = fs::file_size(filepath); 
	if (file_size > _max_file_size - raw_data_bytes) {
	    WARN_MSG("[FileServer::upload_file] Size of " + filepath.string() + " is: " + std::to_string(file_size) + ", thats more then system limit, removing this file");
		fs::remove(filepath);
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
		fs::remove(filepath);
		response.send(Pistache::Http::Code::Internal_Server_Error, "Failed to write file");
		WARN_MSG("[FileServer::upload_file] Failed to write file: " + filepath.string());
	}
}

void FileServer::download_file(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
	auto filename = request.param(":filename").as<std::string>();
	auto filepath = fs::path(_storage_dir) / filename;
	DEBUG_MSG("[FileServer::download_file] Download file called, filepath:" + filepath.string());

	if (!fs::exists(filepath)) {
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
	auto filepath = fs::path(_storage_dir) / filename;
	DEBUG_MSG("[FileServer::delete_file] Delete file called, filepath:" + filepath.string());

	if (!fs::exists(filepath)) {
		response.send(Pistache::Http::Code::Not_Found, "File not found");
		WARN_MSG("[FileServer::delete_file] File " + filepath.string() + " not found");
		return;
	}

	if (fs::remove(filepath)) {
		response.send(Pistache::Http::Code::Ok, "File deleted successfully");
		DEBUG_MSG("[FileServer::delete_file] File " + filepath.string() + " deleted successfully");
	} else {
		response.send(Pistache::Http::Code::Internal_Server_Error, "Failed to delete file");
		WARN_MSG("[FileServer::delete_file] Failed to delete file: " + filepath.string());
		
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
    
    for (const auto& entry : fs::directory_iterator(_storage_dir)) {
        file_list += entry.path().filename().string() + "\n";
        file_count++;
    }
    
    DEBUG_MSG("[FileServer::list_files] Found " + std::to_string(file_count) << " files");
    DEBUG_MSG("[FileServer::list_files] Sending 'Pistache::Http::Code::Ok' response");
    
    response.send(Pistache::Http::Code::Ok, file_list);
}