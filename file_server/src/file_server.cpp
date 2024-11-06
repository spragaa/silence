#include "file_server.hpp"

namespace file_server {

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
    auto opts = Pistache::Http::Endpoint::options()
        .threads(_thread_count)
        .flags(Pistache::Tcp::Options::ReuseAddr)
        .flags(Pistache::Tcp::Options::ReusePort)
        .flags(Pistache::Tcp::Options::CloseOnExec);
            
    _http_endpoint = std::make_shared<Pistache::Http::Endpoint>(
        Pistache::Address("*:" + std::to_string(_server_port))
    );
    _http_endpoint->init(opts);
    setup_routes();
	_http_endpoint->setHandler(_router.handler());
	_http_endpoint->serve();
}

void FileServer::stop() {
    if (_http_endpoint) {
        _http_endpoint->shutdown();
        _router = Pistache::Rest::Router();
        _http_endpoint.reset();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void FileServer::setup_routes() {
	using namespace Pistache::Rest;

	_router = Pistache::Rest::Router();
	Routes::Post(_router, UPLOAD_ROUTE, Routes::bind(&FileServer::upload_file, this));
}

void FileServer::upload_file(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
}

}