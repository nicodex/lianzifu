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
#include <genome/string.hpp>
#include <genome/locale.hpp>
#include <cassert>
#include <climits>
#include <cstdarg>
#include <cstdio>
#ifdef __GLIBCXX__
# include <cxxabi.h>
#endif
#include <algorithm>
#include <typeinfo>

namespace genome {

iarchive&
operator>>(iarchive& archive, byte_string& value)
{
	u16 size;
	archive >> size;
	value.resize(size);
	if (size) {
		if (!archive.read(&value[0], size)) {
			value.resize(static_cast<byte_string::size_type>(archive.gcount()));
		}
	}
	return (archive);
}

oarchive&
operator<<(oarchive& archive, byte_string const& value)
{
	byte_string::size_type length = value.length();
	if (length > u16(-1)) {
		archive.setstate(std::ios_base::failbit);
	} else {
		u16 size = static_cast<u16>(length);
		archive << size;
		if (size) {
			archive.write(value.c_str(), size);
		}
	}
	return (archive);
}

iarchive&
read_ref_string(iarchive& archive, archive::streamref const& ref, byte_string& str)
{
	str.clear();
	if (!ref.empty() && archive.seekg(ref.pos)) {
		str.resize(static_cast<byte_string::size_type>(ref.size));
		if (!archive.read(&str[0], ref.size)) {
			str.resize(static_cast<byte_string::size_type>(archive.gcount()));
		}
		str.resize(byte_string::traits_type::length(str.c_str()));
	}
	return (archive);
}

oarchive&
write_ref_string(oarchive& archive, byte_string const& str, archive::streamref& ref, bool force, archive::streamsize align)
{
	ref.clear();
	if (archive && (force || !str.empty())) {
		byte_string::const_pointer const ptr = str.c_str();
		byte_string::size_type const len = byte_string::traits_type::length(ptr);
		if (force || (len > 0)) {
			ref.size = static_cast<archive::streamsize>(len + 1);
			ref.pos = archive.tellp();
			if (archive.write(ptr, ref.size)) {
				if (align > 1) {
					archive::streamsize const rem = ref.size % align;
					if (rem > 0) {
						align -= rem;
						do {
							if (archive << byte_code::null_char) {
								++ref.size;
							} else {
								break;
							}
						} while (--align);
					}
				}
			} else {
				archive::streampos pos = archive.tellp();
				if (ref.pos < pos) {
					ref.size = pos - ref.pos;
				}
			}
		}
	}
	return (archive);
}

//
// Canonically equivalent conversion (default char '?')
//

bool
string_converter<bool, std::string>::operator()(bool const& from, std::string& to) const
{
	typedef std::numpunct<std::string::value_type> facet_type;
	facet_type const& facet = std::use_facet<facet_type>(std::locale());
	return (to.assign(from ? facet.truename() : facet.falsename()), true);
}

bool
string_converter<bool, std::wstring>::operator()(bool const& from, std::wstring& to) const
{
	typedef std::numpunct<std::wstring::value_type> facet_type;
	facet_type const& facet = std::use_facet<facet_type>(std::locale());
	return (to.assign(from ? facet.truename() : facet.falsename()), true);
}

bool
string_converter<i8, std::string>::operator()(i8 const& from, std::string& to) const
{
	std::string::value_type str[8];
	if (0 >= std::sprintf(str, "%" PRId8, from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}

bool
string_converter<i8, std::wstring>::operator()(i8 const& from, std::wstring& to) const
{
	std::wstring::value_type str[8];
	if (0 >= swprintf(str, sizeof(str) / sizeof(str[0]), L"%" GENOME_CONCAT(L, PRId8), from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}

bool
string_converter<i16, std::string>::operator()(i16 const& from, std::string& to) const
{
	std::string::value_type str[8];
	if (0 >= std::sprintf(str, "%" PRId16, from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}

bool
string_converter<i16, std::wstring>::operator()(i16 const& from, std::wstring& to) const
{
	std::wstring::value_type str[8];
	if (0 >= swprintf(str, sizeof(str) / sizeof(str[0]), L"%" GENOME_CONCAT(L, PRId16), from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}

bool
string_converter<i32, std::string>::operator()(i32 const& from, std::string& to) const
{
	std::string::value_type str[16];
	if (0 >= std::sprintf(str, "%" PRId32, from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}

bool
string_converter<i32, std::wstring>::operator()(i32 const& from, std::wstring& to) const
{
	std::wstring::value_type str[16];
	if (0 >= swprintf(str, sizeof(str) / sizeof(str[0]), L"%" GENOME_CONCAT(L, PRId32), from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}

#if defined(__MINGW32__) && defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)))
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wformat"
#endif

bool
string_converter<i64, std::string>::operator()(i64 const& from, std::string& to) const
{
	std::string::value_type str[32];
	if (0 >= std::sprintf(str, "%" PRId64, from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}

bool
string_converter<i64, std::wstring>::operator()(i64 const& from, std::wstring& to) const
{
	std::wstring::value_type str[32];
	if (0 >= swprintf(str, sizeof(str) / sizeof(str[0]), L"%" GENOME_CONCAT(L, PRId64), from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}

#if defined(__MINGW32__) && defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)))
# pragma GCC diagnostic pop
#endif

bool
string_converter<u8, std::string>::operator()(u8 const& from, std::string& to) const
{
	std::string::value_type str[8];
	if (0 >= std::sprintf(str, "%" PRIu8, from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}

bool
string_converter<u8, std::wstring>::operator()(u8 const& from, std::wstring& to) const
{
	std::wstring::value_type str[8];
	if (0 >= swprintf(str, sizeof(str) / sizeof(str[0]), L"%" GENOME_CONCAT(L, PRIu8), from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}

bool
string_converter<u16, std::string>::operator()(u16 const& from, std::string& to) const
{
	std::string::value_type str[8];
	if (0 >= std::sprintf(str, "%" PRIu16, from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}

bool
string_converter<u16, std::wstring>::operator()(u16 const& from, std::wstring& to) const
{
	std::wstring::value_type str[8];
	if (0 >= swprintf(str, sizeof(str) / sizeof(str[0]), L"%" GENOME_CONCAT(L, PRIu16), from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}

bool
string_converter<u32, std::string>::operator()(u32 const& from, std::string& to) const
{
	std::string::value_type str[16];
	if (0 >= std::sprintf(str, "%" PRIu32, from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}

bool
string_converter<u32, std::wstring>::operator()(u32 const& from, std::wstring& to) const
{
	std::wstring::value_type str[16];
	if (0 >= swprintf(str, sizeof(str) / sizeof(str[0]), L"%" GENOME_CONCAT(L, PRIu32), from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}


#if defined(__MINGW32__) && defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)))
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wformat"
#endif

bool
string_converter<u64, std::string>::operator()(u64 const& from, std::string& to) const
{
	std::string::value_type str[32];
	if (0 >= std::sprintf(str, "%" PRIu64, from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}

bool
string_converter<u64, std::wstring>::operator()(u64 const& from, std::wstring& to) const
{
	std::wstring::value_type str[32];
	if (0 >= swprintf(str, sizeof(str) / sizeof(str[0]), L"%" GENOME_CONCAT(L, PRIu64), from)) {
		return (to.clear(), false);
	}
	return (to.assign(str), true);
}

#if defined(__MINGW32__) && defined(__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)))
# pragma GCC diagnostic pop
#endif

bool
string_converter<byte_string, wide_string>::operator()(byte_string const& from, wide_string& to) const
{
	// Windows-1252 is a SBCS and cp1252_from_bytes() maps all characters
	std::size_t const size = from.length();
	to.resize(size);
	if (size > 0) {
		byte_string::const_pointer const from_begin = from.c_str();
		byte_string::const_pointer const from_end = from_begin + from.length();
		byte_string::const_pointer from_next;
		wide_string::pointer const to_begin = &to[0];
		wide_string::pointer const to_end = to_begin + size;
		wide_string::pointer to_next;
		detail::cp1252_from_bytes<wide_string::value_type, byte_string::value_type>(
			from_begin, from_end, from_next, to_begin, to_end, to_next
		);
	}
	return (true);
}

bool
string_converter<byte_string, std::wstring>::operator()(byte_string const& from, std::wstring& to) const
{
	// Windows-1252 is a SBCS and cp1252_from_bytes() maps all characters
	std::size_t const size = from.length();
	to.resize(size);
	if (size > 0) {
		byte_string::const_pointer const from_begin = from.c_str();
		byte_string::const_pointer const from_end = from_begin + from.length();
		byte_string::const_pointer from_next;
		std::wstring::pointer const to_begin = &to[0];
		std::wstring::pointer const to_end = to_begin + size;
		std::wstring::pointer to_next;
		detail::cp1252_from_bytes<std::wstring::value_type, byte_string::value_type>(
			from_begin, from_end, from_next, to_begin, to_end, to_next
		);
	}
	return (true);
}

bool
string_converter<wide_string, byte_string>::operator()(wide_string const& from, byte_string& to) const
{
	// Windows-1252 is a SBCS and cp1252_to_bytes() doesn't support decompositions
	bool converted = true;
	std::size_t const size = from.size();
	to.resize(size);
	for (std::size_t i = 0; i < size; ++i) {
		wide_string::const_pointer const from_ptr = &from[i];
		wide_string::const_pointer from_next;
		byte_string::pointer const to_ptr = &to[i];
		byte_string::pointer to_next;
		std::codecvt_base::result const result = detail::cp1252_to_bytes<wide_string::value_type, byte_string::value_type>(
			from_ptr, from_ptr + 1, from_next, to_ptr, to_ptr + 1, to_next
		);
		if (result != std::codecvt_base::ok) {
			*to_ptr = byte_code::question_mark;
			converted = false;
		}
	}
	return (converted);
}

bool
string_converter<wide_string, std::wstring>::operator()(wide_string const& from, std::wstring& to) const
{
	to.clear();
	if (from.empty()) {
		return (true);
	}
	to.reserve(from.size());
	bool converted = true;
	unsigned short high_surrogate = 0;
	for (wide_string::const_iterator i = from.begin(); i != from.end(); ++i) {
		unsigned short const code = static_cast<unsigned short>(*i);
		if ((0xD800U <= code) && (code <= (0xD800U | 0x03FFU))) {
			if (high_surrogate) {
				// unpaired high (trailing) surrogate
				to.push_back(0x003F);  // QUESTION MARK
				converted = false;
			}
			high_surrogate = code;
		} else if ((0xDC00U <= code) && (code <= (0xDC00U | 0x03FFU))) {
			if (!high_surrogate) {
				// unpaired low (trailing) surrogate
				to.push_back(0x003F);  // QUESTION MARK
				converted = false;
			} else {
				if (target_wchar_16bit) {
					// target has 16-bit wchar_t, expecting UTF-16 support
					to.push_back(static_cast<wide_char>(high_surrogate));
					to.push_back(static_cast<wide_char>(code));
				} else if (target_wchar_17planes) {
					unsigned long wcode = static_cast<unsigned long>(high_surrogate - 0xD800U) << 10;
					wcode |= static_cast<unsigned long>(code - 0xDC00U);
					wcode += 0x00010000UL;
					to.push_back(static_cast<wchar_t>(wcode));
				} else {
					to.push_back(0x003F);  // QUESTION MARK
					converted = false;
				}
				high_surrogate = 0;
			}
		} else {
			to.push_back(static_cast<wchar_t>(code));
		}
	}
	if (high_surrogate) {
		// unpaired high (trailing) surrogate at the end
		to.push_back(0x003F);  // QUESTION MARK
		converted = false;
	}
	return (converted);
}

bool
string_converter<std::wstring, byte_string>::operator()(std::wstring const& from, byte_string& to) const
{
	// Windows-1252 is a SBCS and cp1252_to_bytes() doesn't support decompositions
	bool converted = true;
	std::size_t const size = from.length();
	to.resize(size);
	for (std::size_t i = 0; i < size ; ++i) {
		std::wstring::const_pointer const from_ptr = &from[i];
		std::wstring::const_pointer from_next;
		byte_string::pointer const to_ptr = &to[i];
		byte_string::pointer to_next;
		std::codecvt_base::result const result = detail::cp1252_to_bytes<std::wstring::value_type, byte_string::value_type>(
			from_ptr, from_ptr + 1, from_next, to_ptr, to_ptr + 1, to_next
		);
		if (result != std::codecvt_base::ok) {
			*to_ptr = byte_code::question_mark;
			converted = false;
		}
	}
	return (converted);
}

bool
string_converter<std::wstring, wide_string>::operator()(std::wstring const& from, wide_string& to) const
{
	to.clear();
	if (from.empty()) {
		return (true);
	}
	to.reserve(from.size());
	bool converted = true;
	unsigned long high_surrogate = 0;
	for (std::wstring::const_iterator i = from.begin(); i != from.end(); ++i) {
		unsigned long code = static_cast<unsigned long>(*i);
		if ((0xD800 <= code) && (code <= (0xD800 | 0x03FF))) {
			if (high_surrogate) {
				// unpaired high (trailing) surrogate
				to.push_back(0x003F);  // QUESTION MARK
				converted = false;
			}
			high_surrogate = code;
		} else if ((0xDC00 <= code) && (code <= (0xDC00 | 0x03FF))) {
			if (!high_surrogate) {
				// unpaired low (trailing) surrogate
				to.push_back(0x003F);  // QUESTION MARK
				converted = false;
			} else {
				to.push_back(static_cast<wide_char>(high_surrogate));
				to.push_back(static_cast<wide_char>(code));
				high_surrogate = 0;
			}
		} else if (!target_wchar_16bit && (code >= 0x00010000UL)) {
			if (code > 0x0010FFFFUL) {
				// invalid code point (cannot be encoded with UTF-16)
				to.push_back(0x003F);  // QUESTION MARK
				converted = false;
			} else {
				code -= 0x00010000UL;
				to.push_back(static_cast<wide_char>(0xD800U | (code >> 10)));
				to.push_back(static_cast<wide_char>(0xDC00U | (code & 0x3FF)));
			}
		} else {
			to.push_back(static_cast<wide_char>(code));
		}
	}
	if (high_surrogate) {
		// unpaired high (trailing) surrogate at the end
		to.push_back(0x003F);  // QUESTION MARK
		converted = false;
	}
	return (converted);
}

bool
string_converter<std::wstring, std::string>::operator()(std::wstring const& from, std::string& to) const
{
	// Single codes are converted to be able to replace invalid codes with question
	// marks and to detect if the multibyte character is canonically equivalent.
	// This also implies that surrogates (!target_wchar_17planes, e.g. Windows)
	// and decomposed characters are not supported by this function.
	to.clear();
	if (from.empty()) {
		return (true);
	}
	to.reserve(from.size());
	bool converted = true;
	char mbs[MB_LEN_MAX + 1];
	std::mbstate_t state = std::mbstate_t();
	for (std::wstring::const_iterator i = from.begin(); i != from.end(); ++i) {
		std::wstring::const_reference wc = *i;
		std::mbstate_t nstate = state;
		std::size_t const size = std::wcrtomb(mbs, wc, &nstate);
		if (size <= MB_LEN_MAX) {
			to.append(mbs, size);
			if (converted) {
				// verify that the MBC is canonically equivalent
				wchar_t rwc;
				std::mbstate_t rstate = state;
				std::size_t const rsize = std::mbrtowc(&rwc, mbs, size, &rstate);
				if ((size != rsize) || (wc != rwc)) {
					converted = false;
				}
			}
			state = nstate;
		} else {
			// reset shift state on error and append a QUESTION MARK (U+003F)
			std::size_t const shift = std::wcrtomb(mbs, L'\0', &state);
			if ((1 < shift) && (shift <= MB_LEN_MAX)) {
				to.append(mbs, shift - 1);
			}
			std::size_t const length = std::wcrtomb(mbs, 0x003F, &state);
			if ((0 < length) && (length <= MB_LEN_MAX)) {
				to.append(mbs, length);
			}
			converted = false;
		}
	}
	std::size_t const shift = std::wcrtomb(mbs, L'\0', &state);
	if ((1 < shift) && (shift <= MB_LEN_MAX)) {
		to.append(mbs, shift - 1);
	}
	return (converted);
}

bool
string_converter<std::string, std::wstring>::operator()(std::string const& from, std::wstring& to) const
{
	to.clear();
	if (from.empty()) {
		return (true);
	}
	to.reserve(from.size());
	bool converted = true;
	std::string::const_pointer const begin = from.c_str();
	std::string::const_pointer const end = begin + from.length();
	std::mbstate_t state = std::mbstate_t();
	for (std::string::const_pointer mbs = begin; mbs < end;) {
		wchar_t wc;
		std::size_t const max = static_cast<std::size_t>(end - mbs);
		std::mbstate_t nstate = state;
		std::size_t length = std::mbrtowc(&wc, mbs, max, &nstate);
		if (length <= max) {
			to.push_back(wc);
			if (0 == length) {
				// skip shift codes and null character
				for (length = 1; length < max; ++length) {
					std::mbstate_t sstate = state;
					if (std::mbrlen(mbs, length, &sstate) != static_cast<std::size_t>(-2)) {
						break;
					}
				}
			}
			state = nstate;
		} else {
			// check for terminating shift codes
			if (static_cast<std::size_t>(-2) == length) {
				std::mbstate_t sstate = state;
				if (0 == std::mbrlen(mbs, max + 1, &sstate)) {
					break;
				}
			}
			to.push_back(0x003F);  // QUESTION MARK
			converted = false;
			length = 1;
		}
		mbs += length;
	}
	return (converted);
}

bool
string_converter<byte_string, std::string>::operator()(byte_string const& from, std::string& to) const
{
	std::wstring wstr;
	bool const w = string_converter<byte_string, std::wstring>().operator()(from, wstr);
	bool const s = string_converter<std::wstring, std::string>().operator()(wstr, to);
	return (w && s);
}

bool
string_converter<std::string, byte_string>::operator()(std::string const& from, byte_string& to) const
{
	std::wstring wstr;
	bool const w = string_converter<std::string, std::wstring>().operator()(from, wstr);
	bool const s = string_converter<std::wstring, byte_string>().operator()(wstr, to);
	return (w && s);
}

bool
string_converter<wide_string, std::string>::operator()(wide_string const& from, std::string& to) const
{
	std::wstring wstr;
	bool const w = string_converter<wide_string, std::wstring>().operator()(from, wstr);
	bool const s = string_converter<std::wstring, std::string>().operator()(wstr, to);
	return (w && s);
}

bool
string_converter<std::string, wide_string>::operator()(std::string const& from, wide_string& to) const
{
	std::wstring wstr;
	bool const w = string_converter<std::string, std::wstring>().operator()(from, wstr);
	bool const s = string_converter<std::wstring, wide_string>().operator()(wstr, to);
	return (w && s);
}

//
// Misc
//

std::string
get_exception_name(std::exception const& e)
{
	std::string name(typeid(e).name());
#ifdef __GLIBCXX__
	char* dname = abi::__cxa_demangle(name.c_str(), NULL, NULL, NULL);
	if (dname) {
		name.assign(dname);
		std::free(dname);
	}
#endif
	return (name);
}

//
// Wrappers
//

#if defined(__MINGW32__) || defined(_CRT_NON_CONFORMING_SWPRINTFS)
int
swprintf(wchar_t* buffer, std::size_t size, wchar_t const* format, ...)
{
	int result;
	va_list vlist;
	va_start(vlist, format);
#if defined(__MINGW32__)
	result = ::_vsnwprintf(buffer, size, format, vlist);
#else
	unused_parameter(size);
	result = ::swprintf(buffer, format, vlist);
#endif
	va_end(vlist);
	return (result);
}
#endif

} // namespace genome
