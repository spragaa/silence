#include "file_server.hpp"

const std::string FileServer::UPLOAD_ROUTE = "/upload/:filename";
const std::string FileServer::DOWNLOAD_ROUTE = "/download/:filename";
const std::string FileServer::DELETE_ROUTE = "/delete/:filename";
const std::string FileServer::LIST_ROUTE = "/list";

constexpr size_t CHUNK_SIZE_BYTES = 512;

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

void FileServer::upload_file(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
	auto filename = request.param(":filename").as<std::string>();
	auto filepath = fs::path(_storage_dir) / filename;

	const std::string& body = request.body();
	size_t total_bytes = body.size();
	DEBUG_MSG("[FileServer::upload_file] Upload file called, filepath:" + filepath.string() + ", body size: " + std::to_string(total_bytes) + " bytes");

	if (total_bytes >= _max_file_size) {
		response.send(Pistache::Http::Code::Bad_Request, "File size exceeds max file size limit");
		WARN_MSG("[FileServer::upload_file] File size exceeds max file size limit");
		return;
	}

	std::ofstream file(filepath, std::ios::binary);
	if (!file) {
		response.send(Pistache::Http::Code::Internal_Server_Error, "Failed to create file");
		WARN_MSG("[FileServer::upload_file] Failed to create file: " + filepath.string());
		return;
	}

	file.write(body.c_str(), total_bytes);
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
void FileServer::list_files(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    std::cout << "[DEBUG] Entering list_files function" << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::string file_list;
    int file_count = 0;
    
    for (const auto& entry : fs::directory_iterator(_storage_dir)) {
        file_list += entry.path().filename().string() + "\n";
        file_count++;
    }
    
    std::cout << "[DEBUG] Found " << file_count << " files" << std::endl;
    
    std::cout << "[DEBUG] Sending response" << std::endl;
    response.send(Pistache::Http::Code::Ok, file_list);
}