#include "sha256.hpp"
#include <cstring>
#include <sstream>
#include <iomanip>

namespace crypto {

SHA256::SHA256() 
    : _state{ 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
              0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 }
    , _total_length(0) {}

void SHA256::update(const std::string& data) {
    const uint8_t* input = reinterpret_cast<const uint8_t*>(data.c_str());
    size_t length = data.length();

    _total_length += length * 8;

    for (size_t i = 0; i < length; ++i) {
        _buffer.push_back(input[i]);
        if (_buffer.size() == 64) {
            transform(_buffer.data());
            _buffer.clear();
        }
    }
}

void SHA256::transform(const uint8_t* data) {
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t temp1, temp2;

    for (int i = 0; i < 16; i++) {
        w[i] = (data[i * 4] << 24) | (data[i * 4 + 1] << 16) |
               (data[i * 4 + 2] << 8) | (data[i * 4 + 3]);
    }

    for (int i = 16; i < 64; i++) {
        w[i] = sig1(w[i - 2]) + w[i - 7] + sig0(w[i - 15]) + w[i - 16];
    }

    a = _state[0];
    b = _state[1];
    c = _state[2];
    d = _state[3];
    e = _state[4];
    f = _state[5];
    g = _state[6];
    h = _state[7];

    for (int i = 0; i < 64; i++) {
        temp1 = h + ep1(e) + ch(e, f, g) + K[i] + w[i];
        temp2 = ep0(a) + maj(a, b, c);

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    _state[0] += a;
    _state[1] += b;
    _state[2] += c;
    _state[3] += d;
    _state[4] += e;
    _state[5] += f;
    _state[6] += g;
    _state[7] += h;
}

void SHA256::pad() {
    uint64_t i = _buffer.size();
    uint64_t final_length = _total_length;

    _buffer.push_back(0x80);
    while (_buffer.size() < 56) {
        _buffer.push_back(0x00);
    }

    for (int i = 7; i >= 0; i--) {
        _buffer.push_back(static_cast<uint8_t>((final_length >> (i * 8)) & 0xFF));
    }

    transform(_buffer.data());
}

std::string SHA256::digest() {
    pad();
    std::stringstream ss;

    for (int i = 0; i < 8; i++) {
        ss << std::hex << std::setw(8) << std::setfill('0') << _state[i];
    }

    _state = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
              0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    _buffer.clear();
    _total_length = 0;

    return ss.str();
}

} // namespace crypto
