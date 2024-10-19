#include <pistache/endpoint.h>

using namespace Pistache;

class HelloHandler : public Http::Handler
{
public:
    HTTP_PROTOTYPE(HelloHandler)

    void onRequest(const Http::Request& /*request*/, Http::ResponseWriter response) override;
};