#include "file_server.hpp"

inline const std::string FileServer::UPLOAD_ROUTE = "/upload/:filename";

FileServer::FileServer(uint16_t port, unsigned int thread_count)
	: _thread_count(thread_count)
	, _server_port(port)
{
}

FileServer::~FileServer() {
    stop();
}

void FileServer::start() {
    stop();

    Pistache::Address addr("localhost", Pistache::Port(_server_port));
    auto opts = Pistache::Http::Endpoint::options()
        .threads(_thread_count)
        .flags(Pistache::Tcp::Options::CloseOnExec);
            
    _http_endpoint = std::make_shared<Pistache::Http::Endpoint>(addr);
    _http_endpoint->init(opts);
    setup_routes();
    _http_endpoint->setHandler(_router.handler());
	_http_endpoint->serve();
	std::cout << "started" << std::endl;
}

void FileServer::stop() {
    if (_http_endpoint) {
        _http_endpoint->shutdown();
        _router = Pistache::Rest::Router();
        _http_endpoint.reset();
        std::cout << "stopped" << std::endl;
    }
}

void FileServer::setup_routes() {
	using namespace Pistache::Rest;

	_router = Pistache::Rest::Router();
	Routes::Post(_router, UPLOAD_ROUTE, Routes::bind(&FileServer::upload_file, this));
}

void FileServer::upload_file(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
}