#include "file_server_client.hpp"
#include <iostream>

namespace server {

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

FileServerClient::FileServerClient(const std::string& host, const std::string& port)
	: _host(host), _port(port), _resolver(_io_context), _stream(_io_context) {
}

std::string FileServerClient::send_request(const std::string& target, http::verb method, const std::string& body) {
	try {
		auto const results = _resolver.resolve(_host, _port);
		_stream.connect(results);

		http::request<http::string_body> req{method, target, _version};
		req.set(http::field::host, _host);
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

		if (!body.empty()) {
			req.body() = body;
			req.prepare_payload();
		}

		// too big
		// DEBUG_MSG("[FileServerClient::send_request] Sending request: " + req.body());
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
		ERROR_MSG("[FileServerClient::send_request] " + std::string(e.what()));
		return "Error" + std::string(e.what());
	}
}


// std::string FileServerClient::upload_file(const std::string& filename, const std::string& filepath) {
// 	auto start_time = std::chrono::high_resolution_clock::now();

// 	std::filesystem::path full_path = std::filesystem::path(filepath) / filename;
// 	std::ifstream file(full_path, std::ios::binary);
// 	if (!file) {
// 		ERROR_MSG("[FileServerClient::upload_file] Unable to open file: " + full_path.string());
// 		return "Error: Unable to open file: " + full_path.string();
// 	}

// 	const size_t chunk_size = 512;
// 	std::vector<char> buffer(chunk_size);
// 	std::string response;

// 	try {
// 		auto const results = _resolver.resolve(_host, _port);
// 		_stream.connect(results);

// 		size_t total_bytes_sent = 0;
// 		while (!file.eof()) {
// 			file.read(buffer.data(), chunk_size);
// 			std::streamsize bytes_read = file.gcount();

// 			if (bytes_read == 0) {
// 				break;
// 			}

// 			http::request<http::string_body> req{http::verb::post, "/upload/" + filename, _version};
// 			req.set(http::field::host, _host);
// 			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
// 			req.set(http::field::content_type, "application/octet-stream");
// 			req.body().assign(buffer.data(), bytes_read);
// 			req.prepare_payload();

// 			// too big
// 			// DEBUG_MSG("[FileServerClient::send_request] Sending request: " + req.body());

// 			http::write(_stream, req);

// 			beast::flat_buffer buffer;
// 			http::response<http::dynamic_body> res;
// 			http::read(_stream, buffer, res);

// 			response = beast::buffers_to_string(res.body().data());
// 			total_bytes_sent += bytes_read;
// 			DEBUG_MSG("[FileServerClient::upload_file] Chunk sent. Total bytes sent: " +  std::to_string(total_bytes_sent));
// 		}

// 		beast::error_code ec;
// 		_stream.socket().shutdown(tcp::socket::shutdown_both, ec);

// 		if(ec && ec != beast::errc::not_connected) {
// 			throw beast::system_error{ec};
// 		}
// 	}
// 	catch(std::exception const& e) {
// 		ERROR_MSG("[FileServerClient::upload_file] " +  std::string(e.what()));
// 		return "Error during upload: " + std::string(e.what());
// 	}
// 	auto end_time = std::chrono::high_resolution_clock::now();
// 	std::chrono::duration<double, std::milli> total_time_ms = end_time - start_time;

// 	DEBUG_MSG("[FileServerClient::upload_file] Chunk uploaded successfully. Execution time: " + std::to_string(total_time_ms.count())
// 	          + " ms. Server response: " + response);
// 	return "File uploaded successfully. Server response: " + response;
// }

bool FileServerClient::upload_chunk(const std::string& filename, const std::string& chunk_data) {
	try {
		auto const results = _resolver.resolve(_host, _port);
		_stream.connect(results);
		DEBUG_MSG("[FileServerClient::upload_chunk] Connected to host");

		http::request<http::string_body> req{http::verb::post, "/upload/" + filename, _version};
		req.set(http::field::host, _host);
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		req.set(http::field::content_type, "application/octet-stream");
		req.body() = chunk_data;
		req.prepare_payload();

		DEBUG_MSG("[FileServerClient::upload_chunk] Sending reques body: " + req.body());

		http::write(_stream, req);

		beast::flat_buffer buffer;
		http::response<http::dynamic_body> res;
		http::read(_stream, buffer, res);

		std::string response_body = beast::buffers_to_string(res.body().data());
		DEBUG_MSG("[FileServerClient::upload_chunk] Response body: " + response_body);

		beast::error_code ec;
		_stream.socket().shutdown(tcp::socket::shutdown_both, ec);

		if(ec && ec != beast::errc::not_connected) {
			FATAL_MSG("[FileServerClient::upload_chunk] Socket shutdown error: " + ec.message());
			return false;
		}

		return response_body == "File uploaded successfully";
	}
	catch(std::exception const& e) {
		ERROR_MSG("[FileServerClient::upload_chunk] Exception: " + std::string(e.what()));
		return false;
	}
}

std::vector<std::string> FileServerClient::download_file_chunks(const std::string& filename) {
	std::vector<std::string> chunks;

	try {
		auto const results = _resolver.resolve(_host, _port);
		_stream.connect(results);

		http::request<http::string_body> req{http::verb::get, "/download/" + filename, _version};
		req.set(http::field::host, _host);
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

		http::write(_stream, req);

		beast::flat_buffer buffer;
		http::response<http::dynamic_body> res;

		while(true) {
			buffer.clear();
			res.clear();

			boost::system::error_code ec;
			http::read(_stream, buffer, res, ec);

			if (ec == http::error::end_of_stream) {
				break;
			}

			if (ec) {
				throw beast::system_error{ec};
			}

			std::string chunk_data = beast::buffers_to_string(res.body().data());
			if (chunk_data.empty()) {
				break;
			}

			chunks.push_back(chunk_data);
			DEBUG_MSG("[FileServerClient::download_file_chunks] Received chunk "
			          + std::to_string(chunks.size()) + " size: "
			          + std::to_string(chunk_data.size()));
		}

		beast::error_code ec;
		_stream.socket().shutdown(tcp::socket::shutdown_both, ec);

		if(ec && ec != beast::errc::not_connected) {
			throw beast::system_error{ec};
		}
	}
	catch(std::exception const& e) {
		ERROR_MSG("[FileServerClient::download_file_chunks] " + std::string(e.what()));
	}

	return chunks;
}

std::string FileServerClient::download_file(const std::string& filename) {
	return send_request("/download/" + filename, http::verb::get);
}

std::string FileServerClient::delete_file(const std::string& filename) {
	return send_request("/delete/" + filename, http::verb::delete_);
}

}