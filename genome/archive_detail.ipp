//
// Copyright (c) 2018 Nico Bendlin <nico@nicode.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#ifndef GENOME_ARCHIVE_DETAIL_IPP
#define GENOME_ARCHIVE_DETAIL_IPP

namespace genome {
namespace detail {

//
// big_endian
//

template<typename T>
void
read_big_endian(T& value, u8 const(& octets)[sizeof(T)])
{
	value = static_cast<T>(octets[0]);
	for (unsigned i = 1; i < sizeof(T); ++i) {
		value = static_cast<T>(static_cast<T>(value << 8) | octets[i]);
	}
}

template<typename T, std::size_t N>
void
read_big_endian(T(& values)[N], u8 const(& octets)[N][sizeof(T)])
{
	for (std::size_t i = 0; i < N; ++i) {
		read_big_endian(values[i], octets[i]);
	}
}

template<typename T>
void
write_big_endian(T const& value, u8(& octets)[sizeof(T)])
{
	T n = value;
	octets[sizeof(T) - 1] = static_cast<u8>(n & 0xFF);
	for (signed i = sizeof(T) - 2; i >= 0; --i) {
		n = static_cast<T>(n >> 8);
		octets[i] = static_cast<u8>(n & 0xFF);
	}
}

template<typename T, std::size_t N>
void
write_big_endian(T const(& values)[N], u8(& octets)[N][sizeof(T)])
{
	for (std::size_t i = 0; i < N; ++i) {
		write_big_endian(values[i], octets[i]);
	}
}

//
// little_endian
//

template<typename T>
void
read_little_endian(T& value, u8 const(& octets)[sizeof(T)])
{
	value = static_cast<T>(octets[sizeof(T) - 1]);
	for (signed i = sizeof(T) - 2; i >= 0; --i) {
		value = static_cast<T>(static_cast<T>(value << 8) | octets[i]);
	}
}

template<typename T, std::size_t N>
void
read_little_endian(T (&values)[N], u8 const(& octets)[N][sizeof(T)])
{
	for (std::size_t i = 0; i < N; ++i) {
		read_little_endian(values[i], octets[i]);
	}
}

template<typename T>
void
write_little_endian(T const& value, u8(& octets)[sizeof(T)])
{
	T n = value;
	octets[0] = static_cast<u8>(n & 0xFF);
	for (unsigned i = 1; i < sizeof(T); ++i) {
		n = static_cast<T>(n >> 8);
		octets[i] = static_cast<u8>(n & 0xFF);
	}
}

template<typename T, std::size_t N>
void
write_little_endian(T const(& values)[N], u8(& octets)[N][sizeof(T)])
{
	for (std::size_t i = 0; i < N; ++i) {
		write_little_endian(values[i], octets[i]);
	}
}

} // namespace genome::detail
} // namespace genome

#endif // GENOME_ARCHIVE_DETAIL_IPP
