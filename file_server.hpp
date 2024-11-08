#pragma once

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/mime.h>
    
class FileServer {
    public:
	FileServer(
		uint16_t port = 9080,
		unsigned int threads = 16
		);
	~FileServer();
	
	void start();
	void stop();
    
public:
	static const std::string UPLOAD_ROUTE;
    
private:
	void setup_routes();
	void upload_file(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    
private:
	std::shared_ptr<Pistache::Http::Endpoint> _http_endpoint;
	std::shared_ptr<Pistache::Rest::Router> _router;
	unsigned int _thread_count;
	unsigned short _server_port;
};