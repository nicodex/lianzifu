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
#ifndef GENOME_STRING_HPP
#define GENOME_STRING_HPP

#include <genome/genome.hpp>
#include <genome/archive.hpp>
#include <string>
#include <exception>

namespace genome {

//
// ASCII (octets)
//

typedef u8 byte_char;
typedef std::basic_string<byte_char> byte_string;

// u16-prefixed string (value.size() > 65535: archive.bad())
iarchive& operator>>(iarchive& archive, byte_string& value);
oarchive& operator<<(oarchive& archive, byte_string const& value);
// Note that this function modifies the archive's position indicator for non-empty references.
iarchive& read_ref_string(iarchive& archive, archive::streamref const& ref, byte_string& str);
// The string data is always null-terminated, but you can add additional null characters to align ref.size-based offset calculations.
oarchive& write_ref_string(oarchive& archive, byte_string const& str, archive::streamref& ref, bool force = false, archive::streamsize align = 0);

namespace byte_code {

	GENOME_CONSTEXPR_CONST byte_char null_char       = 0x00;  // '\0'
	GENOME_CONSTEXPR_CONST byte_char line_feed       = 0x0A;  // '\n'
	GENOME_CONSTEXPR_CONST byte_char carriage_return = 0x0D;  // '\r'
	GENOME_CONSTEXPR_CONST byte_char full_stop       = 0x2E;  // '.'
	GENOME_CONSTEXPR_CONST byte_char percent_sign    = 0x25;  // '%'
	GENOME_CONSTEXPR_CONST byte_char digit_first     = 0x30;  // '0'
	GENOME_CONSTEXPR_CONST byte_char digit_last      = 0x39;  // '9'
	GENOME_CONSTEXPR_CONST byte_char colon           = 0x3A;  // ':'
	GENOME_CONSTEXPR_CONST byte_char question_mark   = 0x3F;  // '?'
	GENOME_CONSTEXPR_CONST byte_char upper_first     = 0x41;  // 'A'
	GENOME_CONSTEXPR_CONST byte_char upper_last      = 0x5A;  // 'Z'
	GENOME_CONSTEXPR_CONST byte_char reverse_solidus = 0x5C;  // '\\'
	GENOME_CONSTEXPR_CONST byte_char lower_first     = 0x61;  // 'a'
	GENOME_CONSTEXPR_CONST byte_char hex_first       = 0x61;  // 'a'
	GENOME_CONSTEXPR_CONST byte_char hex_last        = 0x66;  // 'f'
	GENOME_CONSTEXPR_CONST byte_char lower_last      = 0x7A;  // 'z'
	GENOME_CONSTEXPR_CONST byte_char vertical_line   = 0x7C;  // '|'

	GENOME_CONSTEXPR_INLINE
	byte_char
	to_lower(byte_char c)
	{
		return (
			((upper_first <= c) && (c <= upper_last))
			? static_cast<byte_char>(c + (lower_first - upper_first))
			: c
		);
	}

	GENOME_CONSTEXPR_INLINE
	byte_char
	to_upper(byte_char c)
	{
		return (
			((lower_first <= c) && (c <= lower_last))
			? static_cast<byte_char>(c - (lower_first - upper_first))
			: c
		);
	}

} // namespace genome::byte_code

//
// Identifier traits (MSVC10 "C" locale lowercase)
//

struct name_traits : public byte_string::traits_type {
	static GENOME_CONSTEXPR_INLINE
	char_type
	canonicalize(char_type c)
	{
		return (byte_code::to_lower(c));
	}
	static GENOME_CONSTEXPR_INLINE
	bool
	eq(char_type const& c1, char_type const& c2)
	{
		return (canonicalize(c1) == canonicalize(c2));
	}
	static GENOME_CONSTEXPR_INLINE
	bool
	lt(char_type const& c1, char_type const& c2)
	{
		return (canonicalize(c1) < canonicalize(c2));
	}
	static inline
	int
	compare(char_type const* s1, char_type const* s2, std::size_t n)
	{
		for (std::size_t i = 0; i < n; ++i) {
			if (lt(s1[i], s2[i])) {
				return (-1);
			} else if (lt(s2[i], s1[i])) {
				return (1);
			}
		}
		return (0);
	}
	static inline
	char_type const*
	find(char_type const* s, std::size_t n, char_type const& c)
	{
		for (std::size_t i = 0; i < n; ++i) {
			if (eq(s[i], c)) {
				return (s + i);
			}
		}
		return (0);
	}
};

//
// Unicode (UTF-16)
//

typedef u16 wide_char;
typedef std::basic_string<wide_char> wide_string;

//
// Utilities
//

template<typename StringT>
bool string_startswith(StringT const& str, StringT const& prefix) GENOME_NOEXCEPT_NOTHROW;

//
// Canonically equivalent conversion (default char '?')
//

template<typename T, typename U>
struct string_converter {
	bool operator()(T const& from, U& to) const;
};
template<typename T>
struct string_converter<T, T> {
	bool operator()(T const& from, T& to) const;
};

template<typename T, typename U>
bool string_convert(T const& from, U& to);

template<>
struct string_converter<bool, std::string> {
	bool operator()(bool const& from, std::string& to) const;
};
template<>
struct string_converter<bool, std::wstring> {
	bool operator()(bool const& from, std::wstring& to) const;
};
template<>
struct string_converter<i8, std::string> {
	bool operator()(i8 const& from, std::string& to) const;
};
template<>
struct string_converter<i8, std::wstring> {
	bool operator()(i8 const& from, std::wstring& to) const;
};
template<>
struct string_converter<i16, std::string> {
	bool operator()(i16 const& from, std::string& to) const;
};
template<>
struct string_converter<i16, std::wstring> {
	bool operator()(i16 const& from, std::wstring& to) const;
};
template<>
struct string_converter<i32, std::string> {
	bool operator()(i32 const& from, std::string& to) const;
};
template<>
struct string_converter<i32, std::wstring> {
	bool operator()(i32 const& from, std::wstring& to) const;
};
template<>
struct string_converter<i64, std::string> {
	bool operator()(i64 const& from, std::string& to) const;
};
template<>
struct string_converter<i64, std::wstring> {
	bool operator()(i64 const& from, std::wstring& to) const;
};
template<>
struct string_converter<u8, std::string> {
	bool operator()(u8 const& from, std::string& to) const;
};
template<>
struct string_converter<u8, std::wstring> {
	bool operator()(u8 const& from, std::wstring& to) const;
};
template<>
struct string_converter<u16, std::string> {
	bool operator()(u16 const& from, std::string& to) const;
};
template<>
struct string_converter<u16, std::wstring> {
	bool operator()(u16 const& from, std::wstring& to) const;
};
template<>
struct string_converter<u32, std::string> {
	bool operator()(u32 const& from, std::string& to) const;
};
template<>
struct string_converter<u32, std::wstring> {
	bool operator()(u32 const& from, std::wstring& to) const;
};
template<>
struct string_converter<u64, std::string> {
	bool operator()(u64 const& from, std::string& to) const;
};
template<>
struct string_converter<u64, std::wstring> {
	bool operator()(u64 const& from, std::wstring& to) const;
};

template<>
struct string_converter<byte_string, wide_string> {
	bool operator()(byte_string const& from, wide_string& to) const;
};
template<>
struct string_converter<byte_string, std::wstring> {
	bool operator()(byte_string const& from, std::wstring& to) const;
};
template<>
struct string_converter<wide_string, byte_string> {
	bool operator()(wide_string const& from, byte_string& to) const;
};
template<>
struct string_converter<wide_string, std::wstring> {
	bool operator()(wide_string const& from, std::wstring& to) const;
};
template<>
struct string_converter<std::wstring, byte_string> {
	bool operator()(std::wstring const& from, byte_string& to) const;
};
template<>
struct string_converter<std::wstring, wide_string> {
	bool operator()(std::wstring const& from, wide_string& to) const;
};
template<>
struct string_converter<std::wstring, std::string> {
	bool operator()(std::wstring const& from, std::string& to) const;
};
template<>
struct string_converter<std::string, std::wstring> {
	bool operator()(std::string const& from, std::wstring& to) const;
};
template<>
struct string_converter<byte_string, std::string> {
	bool operator()(byte_string const& from, std::string& to) const;
};
template<>
struct string_converter<std::string, byte_string> {
	bool operator()(std::string const& from, byte_string& to) const;
};
template<>
struct string_converter<wide_string, std::string> {
	bool operator()(wide_string const& from, std::string& to) const;
};
template<>
struct string_converter<std::string, wide_string> {
	bool operator()(std::string const& from, wide_string& to) const;
};

//
// Conversion cast (throws std::range_error if not canonically equivalent)
//

template<typename T, typename U>
struct string_caster {
	U operator()(T const& value) const;
};
template<typename T>
struct string_caster<T, T> {
	T operator()(T const& value) const;
};

template<typename T, typename U>
U string_cast(T const& value);

template<typename T>
std::string to_string(T const& value);
template<typename T>
std::wstring to_wstring(T const& value);
template<typename T>
byte_string to_byte_string(T const& value);
template<typename T>
wide_string to_wide_string(T const& value);

//
// Misc
//

std::string
get_exception_name(std::exception const& e);

} // namespace genome

#include <genome/string.ipp>

#endif // GENOME_STRING_HPP
