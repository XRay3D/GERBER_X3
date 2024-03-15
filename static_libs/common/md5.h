/*
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CONSTEXPR_MD5_H_
#define CONSTEXPR_MD5_H_

// https://github.com/Wodann/constexpr-md5-cpp/blob/master/include/md5.h

#include <array>
#include <climits>
#include <cstdint>
#include <cstring>

namespace md5 {

using Digest = std::array<unsigned char, 16>;

namespace details {
constexpr uint32_t CBLOCK = 64;
constexpr uint32_t LBLOCK = CBLOCK / 4;

constexpr size_t const_strlen(const char* str) {
    return (*str == 0) ? 0 : const_strlen(str + 1) + 1;
}

constexpr Digest make_digest(const std::array<uint32_t, 4>& input) noexcept {
    Digest digest{};
    for(size_t i = 0; i < input.size(); ++i) {
        digest[i * 4] = static_cast<unsigned char>((input[i]) & 0xff);
        digest[i * 4 + 1] = static_cast<unsigned char>((input[i] >> 8) & 0xff);
        digest[i * 4 + 2] = static_cast<unsigned char>((input[i] >> 16) & 0xff);
        digest[i * 4 + 3] = static_cast<unsigned char>((input[i] >> 24) & 0xff);
    }
    return digest;
}

using Fn = uint32_t (*)(uint32_t, uint32_t, uint32_t);
constexpr uint32_t f(uint32_t b, uint32_t c, uint32_t d) noexcept {
    return (b & c) | (~b & d);
}

constexpr uint32_t g(uint32_t b, uint32_t c, uint32_t d) noexcept {
    return (b & d) | (c & ~d);
}

constexpr uint32_t h(uint32_t b, uint32_t c, uint32_t d) noexcept {
    return b ^ c ^ d;
}

constexpr uint32_t i(uint32_t b, uint32_t c, uint32_t d) noexcept {
    return c ^ (b | ~d);
}

constexpr Fn F[4] = {f, g, h, i};

constexpr uint32_t G[CBLOCK] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12,
    5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2,
    0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9};

constexpr uint32_t K[CBLOCK] = {
    0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE,
    0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
    0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE,
    0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,
    0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA,
    0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
    0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED,
    0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,
    0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C,
    0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
    0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05,
    0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,
    0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039,
    0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
    0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1,
    0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391};

constexpr uint32_t S[LBLOCK] = {
    7, 12, 17, 22,
    5, 9, 14, 20,
    4, 11, 16, 23,
    6, 10, 15, 21};

constexpr char PADDING[CBLOCK] = {
    -128, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80 = -128
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

constexpr uint32_t rotate(uint32_t x, uint32_t n) noexcept {
    return (x << n) | (x >> (32 - n));
}

constexpr uint32_t t(Fn f, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac) noexcept {
    return b + rotate(a + f(b, c, d) + x + ac, s);
}

constexpr uint32_t to_uint32(const unsigned char* data) noexcept {
    return (static_cast<uint32_t>(data[3]) << 24) | (static_cast<uint32_t>(data[2]) << 16) | (static_cast<uint32_t>(data[1]) << 8) | (static_cast<uint32_t>(data[0]));
}

struct Context {
    std::array<unsigned char, CBLOCK> buffer;
    std::array<uint32_t, 4> state;
    uint32_t nl, nh;

    constexpr Context() noexcept
        : buffer()
        , state{0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476}
        , nl(0)
        , nh(0) { }

    constexpr void append(const char* data, size_t len) noexcept {
        std::array<uint32_t, LBLOCK> input{};
        auto k = (nl >> 3) & 0x3f;
        auto length = static_cast<uint32_t>(len);
        nl += length << 3;
        if(nl < length << 3)
            nh += 1;
        nh += length >> 29;
        for(auto ptr = data; ptr != data + len; ++ptr) {
            buffer[k++] = static_cast<unsigned char>(static_cast<int16_t>(*ptr) + UCHAR_MAX + 1);
            if(k == 0x40) {
                auto j = 0;
                for(auto i = 0; i < LBLOCK; ++i) {
                    input[i] = to_uint32(&buffer[j]);
                    j += 4;
                }
                transform(input);
                k = 0;
            }
        }
    }

    constexpr void transform(const std::array<uint32_t, LBLOCK>& input) noexcept {
        auto a = state[0], b = state[1], c = state[2], d = state[3];
        for(uint32_t r = 0; r < 4; ++r) {
            const auto g = G + r * LBLOCK;
            const auto s = S + r * 4;
            const auto k = K + r * LBLOCK;

            for(auto i = 0; i < input.size(); ++i) {
                const auto new_b = t(F[r], a, b, c, d, input[g[i]], s[i % 4], k[i]);
                a = d;
                d = c;
                c = b;
                b = new_b;
            }
        }
        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
    }

    constexpr Digest final() noexcept {

        std::array<uint32_t, LBLOCK> input{};
        const auto k = ((nl >> 3) & 0x3f);
        input[14] = nl;
        input[15] = nh;

        append(PADDING, k < 56 ? 56 - k : 120 - k);

        auto j = 0;
        for(auto i = 0; i < 14; ++i) {
            input[i] = to_uint32(&buffer[j]);
            j += 4;
        }
        transform(input);

        return make_digest(state);
    }
};
} // namespace details

template <size_t N>
consteval Digest compute(const char (&data)[N]) noexcept {
    details::Context c;
    // Don't hash the null-terminator
    c.append(data, N - 1);
    return c.final();
}

template <size_t N>
consteval uint32_t hash32(const char (&data)[N]) noexcept {
    return details::to_uint32(compute(data).data());
}

constexpr Digest compute(const char* s) noexcept {
    details::Context c;
    // Don't hash the null-terminator
    c.append(s, details::const_strlen(s));
    return c.final();
}
} // namespace md5

constexpr auto operator""_hash32(const char* str, size_t size) {
    return md5::details::to_uint32(md5::compute(str).data());
}

#endif
