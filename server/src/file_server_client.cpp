#include "file_server_client.hpp"
#include <iostream>

FileServerClient::FileServerClient(const std::string& host, const std::string& port)
    : _port(host), port_(port), _resolver(_io_context), _stream(_io_context) {}

std::string FileServerClient::send_request(const std::string& target, http::verb method, const std::string& body) {
    try {
        auto const results = _resolver.resolve(_port, port_);
        _stream.connect(results);

        http::request<http::string_body> req{method, target, _version};
        req.set(http::field::host, _port);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        if (!body.empty()) {
            req.body() = body;
            req.prepare_payload();
        }

        std::cout << "Sending request: " << req << std::endl;
        http::write(_stream, req);

        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(_stream, buffer, res);

        std::string response_body = beast::buffers_to_string(res.body().data());

        beast::error_code ec;
        _stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        if(ec && ec != beast::errc::not_connected) {
            throw beast::system_error{ec};
        }

        return response_body;
    }
    catch(std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return "";
    }
}

std::string FileServerClient::list_files() {
    return send_request("/list", http::verb::get);
}

std::string FileServerClient::upload_file(const std::string& filename, const std::string& content) {
    return send_request("/upload/" + filename, http::verb::post, content);
}

std::string FileServerClient::download_file(const std::string& filename) {
    return send_request("/download/" + filename, http::verb::get);
}

std::string FileServerClient::delete_file(const std::string& filename) {
    return send_request("/delete/" + filename, http::verb::delete_);
}