#include "file_server.hpp"

void HelloHandler::onRequest(const Http::Request& /*request*/, Http::ResponseWriter response)
{
    response.send(Pistache::Http::Code::Ok, "Hello World\n");
}