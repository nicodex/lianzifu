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
#ifndef GENOME_LOCALE_IPP
#define GENOME_LOCALE_IPP

#include <algorithm>
#include <stdexcept>

#include <genome/locale_detail.ipp>

namespace genome {

//
// codecvt_utf8
//

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
codecvt_utf8<WideT, Maxcode, Mode>::codecvt_utf8(std::size_t refs)
	: base_type(refs)
{
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
codecvt_utf8<WideT, Maxcode, Mode>::~codecvt_utf8(void)
{
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
int
codecvt_utf8<WideT, Maxcode, Mode>::do_max_length(void) const GENOME_NOEXCEPT_NOTHROW
{
	using namespace genome::codecvt;
	// codecvt::max_length() is intended for use with codecvt::in()
	// but we also test generate_header for use with codecvt::out()
	return (
		(((consume_header | generate_header) & Mode) ? 3 : 0) +
		1 +
		(static_cast<unsigned long>(max_code) >= 0x00000080UL) +
		(static_cast<unsigned long>(max_code) >= 0x00000800UL) +
		(static_cast<unsigned long>(max_code) >= 0x00010000UL)
	);
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
typename codecvt_utf8<WideT, Maxcode, Mode>::result
codecvt_utf8<WideT, Maxcode, Mode>::do_in(state_type& state,
	extern_type const* from, extern_type const* from_end, extern_type const*& from_next,
	intern_type* to, intern_type* to_end, intern_type*& to_next) const
{
	using namespace genome::codecvt;
	return (
		detail::utf8_from_bytes<
			intern_type,
			extern_type,
			false,
			static_cast<unsigned long>(max_code),
			(consume_header & Mode) != 0>(
			state, from, from_end, from_next, to, to_end, to_next
		)
	);
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
typename codecvt_utf8<WideT, Maxcode, Mode>::result
codecvt_utf8<WideT, Maxcode, Mode>::do_out(state_type& state,
	intern_type const* from, intern_type const* from_end, intern_type const*& from_next,
	extern_type* to, extern_type* to_end, extern_type*& to_next) const
{
	using namespace genome::codecvt;
	return (
		detail::utf8_to_bytes<
			intern_type,
			extern_type,
			false,
			static_cast<unsigned long>(max_code),
			(generate_header & Mode) != 0>(
			state, from, from_end, from_next, to, to_end, to_next
		)
	);
}

//
// codecvt_utf8_utf16
//

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
codecvt_utf8_utf16<WideT, Maxcode, Mode>::codecvt_utf8_utf16(std::size_t refs)
	: base_type(refs)
{
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
codecvt_utf8_utf16<WideT, Maxcode, Mode>::~codecvt_utf8_utf16(void)
{
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
int
codecvt_utf8_utf16<WideT, Maxcode, Mode>::do_max_length(void) const GENOME_NOEXCEPT_NOTHROW
{
	using namespace genome::codecvt;
	// codecvt::max_length() is intended for use with codecvt::in()
	// but we also test generate_header for use with codecvt::out()
	return (
		(((consume_header | generate_header) & Mode) ? 3 : 0) +
		1 +
		(static_cast<unsigned long>(max_code) >= 0x00000080UL) +
		(static_cast<unsigned long>(max_code) >= 0x00000800UL) +
		(static_cast<unsigned long>(max_code) >= 0x00010000UL)
	);
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
typename codecvt_utf8_utf16<WideT, Maxcode, Mode>::result
codecvt_utf8_utf16<WideT, Maxcode, Mode>::do_in(state_type& state,
	extern_type const* from, extern_type const* from_end, extern_type const*& from_next,
	intern_type* to, intern_type* to_end, intern_type*& to_next) const
{
	using namespace genome::codecvt;
	return (
		detail::utf8_from_bytes<
			intern_type,
			extern_type,
			true,
			static_cast<unsigned long>(max_code),
			(consume_header & Mode) != 0>(
			state, from, from_end, from_next, to, to_end, to_next
		)
	);
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
typename codecvt_utf8_utf16<WideT, Maxcode, Mode>::result
codecvt_utf8_utf16<WideT, Maxcode, Mode>::do_out(state_type& state,
	intern_type const* from, intern_type const* from_end, intern_type const*& from_next,
	extern_type* to, extern_type* to_end, extern_type*& to_next) const
{
	using namespace genome::codecvt;
	return (
		detail::utf8_to_bytes<
			intern_type,
			extern_type,
			true,
			static_cast<unsigned long>(max_code),
			(generate_header & Mode) != 0>(
			state, from, from_end, from_next, to, to_end, to_next
		)
	);
}

//
// codecvt_utf16
//

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
codecvt_utf16<WideT, Maxcode, Mode>::codecvt_utf16(std::size_t refs)
	: base_type(refs)
{
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
codecvt_utf16<WideT, Maxcode, Mode>::~codecvt_utf16(void)
{
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
int
codecvt_utf16<WideT, Maxcode, Mode>::do_max_length(void) const GENOME_NOEXCEPT_NOTHROW
{
	using namespace genome::codecvt;
	// codecvt::max_length() is intended for use with codecvt::in()
	// but we also test generate_header for use with codecvt::out()
	return (
		2 * (
			(((consume_header | generate_header) & Mode) ? 1 : 0) +
			1 +
			(static_cast<unsigned long>(max_code) >= 0x00010000UL)
		)
	);
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
typename codecvt_utf16<WideT, Maxcode, Mode>::result
codecvt_utf16<WideT, Maxcode, Mode>::do_in(state_type& state,
	extern_type const* from, extern_type const* from_end, extern_type const*& from_next,
	intern_type* to, intern_type* to_end, intern_type*& to_next) const
{
	using namespace genome::codecvt;
	return (
		detail::utf16_from_bytes<
			intern_type,
			extern_type,
			(little_endian & Mode) != 0,
			static_cast<unsigned long>(max_code),
			(consume_header & Mode) != 0>(
			state, from, from_end, from_next, to, to_end, to_next
		)
	);
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
typename codecvt_utf16<WideT, Maxcode, Mode>::result
codecvt_utf16<WideT, Maxcode, Mode>::do_out(state_type& state,
	intern_type const* from, intern_type const* from_end, intern_type const*& from_next,
	extern_type* to, extern_type* to_end, extern_type*& to_next) const
{
	using namespace genome::codecvt;
	return (
		detail::utf16_to_bytes<
			intern_type,
			extern_type,
			(little_endian & Mode) != 0,
			static_cast<unsigned long>(max_code),
			(generate_header & Mode) != 0>(
			state, from, from_end, from_next, to, to_end, to_next
		)
	);
}

//
// wstring_convert<...>
//

template<typename Codecvt, typename Elem, typename Wide_alloc, typename Byte_alloc>
wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::wstring_convert(Codecvt* pcvt)
	: m_codecvt(pcvt)
	, m_locale(std::locale(), m_codecvt)
	, m_state(state_type())
	, m_byte_err()
	, m_wide_err()
	, m_converted(0)
	, m_has_state(false)
	, m_has_byte_err(false)
	, m_has_wide_err(false)
{
}

template<typename Codecvt, typename Elem, typename Wide_alloc, typename Byte_alloc>
wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::wstring_convert(Codecvt* pcvt, state_type state)
	: m_codecvt(pcvt)
	, m_locale(std::locale(), m_codecvt)
	, m_state(state)
	, m_byte_err()
	, m_wide_err()
	, m_converted(0)
	, m_has_state(true)
	, m_has_byte_err(false)
	, m_has_wide_err(false)
{
}

template<typename Codecvt, typename Elem, typename Wide_alloc, typename Byte_alloc>
wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::wstring_convert(byte_string const& byte_err)
	: m_codecvt(new Codecvt)
	, m_locale(std::locale(), m_codecvt)
	, m_state(state_type())
	, m_byte_err(byte_err)
	, m_wide_err()
	, m_converted(0)
	, m_has_state(false)
	, m_has_byte_err(true)
	, m_has_wide_err(false)
{
}

template<typename Codecvt, typename Elem, typename Wide_alloc, typename Byte_alloc>
wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::wstring_convert(byte_string const& byte_err, wide_string const& wide_err)
	: m_codecvt(new Codecvt)
	, m_locale(std::locale(), m_codecvt)
	, m_state(state_type())
	, m_byte_err(byte_err)
	, m_wide_err(wide_err)
	, m_converted(0)
	, m_has_state(false)
	, m_has_byte_err(true)
	, m_has_wide_err(true)
{
}

template<typename Codecvt, typename Elem, typename Wide_alloc, typename Byte_alloc>
wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::~wstring_convert(void) GENOME_NOEXCEPT_NOTHROW
{
}

template<typename Codecvt, typename Elem, typename Wide_alloc, typename Byte_alloc>
typename wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::wide_string
wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::from_bytes(char byte)
{
	char const* const first = &byte;
	char const* const last = first + 1;
	return (from_bytes(first, last));
}

template<typename Codecvt, typename Elem, typename Wide_alloc, typename Byte_alloc>
typename wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::wide_string
wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::from_bytes(char const* ptr)
{
	char const* const first = ptr;
	char const* const last = first + (ptr ? std::char_traits<char>::length(ptr) : 0);
	return (from_bytes(first, last));
}

template<typename Codecvt, typename Elem, typename Wide_alloc, typename Byte_alloc>
typename wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::wide_string
wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::from_bytes(byte_string const& str)
{
	char const* const first = str.c_str();
	char const* const last = first + str.length();
	return (from_bytes(first, last));
}

template<typename Codecvt, typename Elem, typename Wide_alloc, typename Byte_alloc>
typename wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::wide_string
wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::from_bytes(char const* first, char const* last)
{
	enum config {
		buffer_size = 16  // expected to be large enough for all converters
	};
	wide_string wstr;
	if (!m_has_state) {
		m_state = state_type();
	}
	m_converted = 0;
	char const* from = first;
	char const* const from_end = last;
	char const* from_next;
	Elem to[buffer_size];
	Elem* const to_end = &to[buffer_size];
	Elem* to_next;
	for (; from != from_end; from = from_next) {
		switch (m_codecvt->in(m_state, from, from_end, from_next, to, to_end, to_next)) {
		case std::codecvt_base::ok:
		case std::codecvt_base::partial:
			{
				std::ptrdiff_t to_count = to_next - to;
				if (0 < to_count) {
					wstr.append(to, static_cast<typename wide_string::size_type>(to_count));
				} else if (from_next == from) {
					if (m_has_wide_err) {
						return (m_wide_err);
					}
					throw std::range_error("bad conversion");
				}
			}
			break;
		case std::codecvt_base::noconv:
			wstr.reserve(wstr.size() + static_cast<typename wide_string::size_type>(from_end - from));
			for (from_next = from; from_next != from_end; ++from_next) {
				wstr.append(typename wide_string::size_type(1), static_cast<typename wide_string::value_type>(static_cast<unsigned char>(*from_next)));
			}
			break;
		default:
			if (m_has_wide_err) {
				return (m_wide_err);
			}
			throw std::range_error("bad conversion");
		}
		m_converted = static_cast<std::size_t>(from_next - first);
	}
	return (wstr);
}

template<typename Codecvt, typename Elem, typename Wide_alloc, typename Byte_alloc>
typename wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::byte_string
wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::to_bytes(Elem wchar)
{
	Elem const* const first = &wchar;
	Elem const* const last = first + 1;
	return (to_bytes(first, last));
}

template<typename Codecvt, typename Elem, typename Wide_alloc, typename Byte_alloc>
typename wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::byte_string
wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::to_bytes(Elem const* wptr)
{
	Elem const* const first = wptr;
	Elem const* const last = first + (wptr ? std::char_traits<Elem>::length(wptr) : 0);
	return (to_bytes(first, last));
}

template<typename Codecvt, typename Elem, typename Wide_alloc, typename Byte_alloc>
typename wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::byte_string
wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::to_bytes(wide_string const& wstr)
{
	Elem const* const first = wstr.c_str();
	Elem const* const last = first + wstr.length();
	return (to_bytes(first, last));
}

template<typename Codecvt, typename Elem, typename Wide_alloc, typename Byte_alloc>
typename wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::byte_string
wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::to_bytes(Elem const* first, Elem const* last)
{
	enum config {
		buffer_size = 32  // expected to be large enough for all converters
	};
	byte_string str;
	if (!m_has_state) {
		m_state = state_type();
	}
	m_converted = 0;
	Elem const* from = first;
	Elem const* const from_end = last;
	Elem const* from_next;
	char to[buffer_size];
	char* const to_end = &to[buffer_size];
	char* to_next;
	for (; from != from_end; from = from_next) {
		switch (m_codecvt->out(m_state, from, from_end, from_next, to, to_end, to_next)) {
		case std::codecvt_base::ok:
		case std::codecvt_base::partial:
			{
				std::ptrdiff_t to_count = to_next - to;
				if (0 < to_count) {
					str.append(to, static_cast<typename byte_string::size_type>(to_count));
				} else if (from_next == from) {
					if (m_has_byte_err) {
						return (m_byte_err);
					}
					throw std::range_error("bad conversion");
				}
			}
			break;
		case std::codecvt_base::noconv:
			str.reserve(str.size() + static_cast<typename byte_string::size_type>(from_end - from));
			for (from_next = from; from_next != from_end; ++from_next) {
				str.append(typename byte_string::size_type(1), static_cast<typename byte_string::value_type>(static_cast<unsigned char>(*from_next)));
			}
			break;
		default:
			if (m_has_byte_err) {
				return (m_byte_err);
			}
			throw std::range_error("bad conversion");
		}
		m_converted = static_cast<std::size_t>(from_next - first);
	}
	return (str);
}

template<typename Codecvt, typename Elem, typename Wide_alloc, typename Byte_alloc>
typename wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::state_type
wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::state(void) const GENOME_NOEXCEPT_NOTHROW
{
	return (m_state);
}

template<typename Codecvt, typename Elem, typename Wide_alloc, typename Byte_alloc>
std::size_t
wstring_convert<Codecvt, Elem, Wide_alloc, Byte_alloc>::converted(void) const GENOME_NOEXCEPT_NOTHROW
{
	return (m_converted);
}

//
// ctype_genome<...>
//

template<typename WideT>
ctype_genome<WideT>::ctype_genome(std::size_t refs)
	: base_type(refs)
{
}

template<typename WideT>
ctype_genome<WideT>::~ctype_genome(void)
{
}

template<typename WideT>
bool
ctype_genome<WideT>::do_is(mask m, char_type c) const
{
	return (
		0 != (m &
			detail::ctype_genome_table[
				(char_type() == c)
				? static_cast<unsigned char>(char())
				: static_cast<unsigned char>(do_narrow(c, char()))
			]
		)
	);
}

template<typename WideT>
typename ctype_genome<WideT>::char_type const*
ctype_genome<WideT>::do_is(char_type const* str, char_type const* end, mask* m) const
{
	for (; str < end; ++str, ++m) {
		*m = detail::ctype_genome_table[
			(char_type() == *str)
			? static_cast<unsigned char>(char())
			: static_cast<unsigned char>(do_narrow(*str, char()))
		];
	}
	return (str);
}

template<typename WideT>
typename ctype_genome<WideT>::char_type const*
ctype_genome<WideT>::do_scan_is(mask m, char_type const* str, char_type const* end) const
{
	for (; (str < end) && !do_is(m, *str); ++str) {
	}
	return (str);
}

template<typename WideT>
typename ctype_genome<WideT>::char_type const*
ctype_genome<WideT>::do_scan_not(mask m, char_type const* str, char_type const* end) const
{
	for (; (str < end) && do_is(m, *str); ++str) {
	}
	return (str);
}

template<typename WideT>
typename ctype_genome<WideT>::char_type
ctype_genome<WideT>::do_toupper(char_type c) const
{
	if (c != char_type()) {
		char const nc = do_narrow(c, char());
		if (nc != char()) {
			char_type const wc = do_widen(static_cast<char>(byte_code::to_upper(static_cast<byte_char>(static_cast<unsigned char>(nc)))));
			if (wc != char_type(-1)) {
				c = wc;
			}
		}
	}
	return (c);
}

template<typename WideT>
typename ctype_genome<WideT>::char_type const*
ctype_genome<WideT>::do_toupper(char_type* str, char_type const* end) const
{
	for (; str < end; ++str) {
		*str = do_toupper(*str);
	}
	return (str);
}

template<typename WideT>
typename ctype_genome<WideT>::char_type
ctype_genome<WideT>::do_tolower(char_type c) const
{
	if (c != char_type()) {
		char const nc = do_narrow(c, char());
		if (nc != char()) {
			char_type const wc = do_widen(static_cast<char>(byte_code::to_lower(static_cast<byte_char>(static_cast<unsigned char>(nc)))));
			if (wc != char_type(-1)) {
				c = wc;
			}
		}
	}
	return (c);
}

template<typename WideT>
typename ctype_genome<WideT>::char_type const*
ctype_genome<WideT>::do_tolower(char_type* str, char_type const* end) const
{
	for (; str < end; ++str) {
		*str = do_tolower(*str);
	}
	return (str);
}

template<typename WideT>
typename ctype_genome<WideT>::char_type
ctype_genome<WideT>::do_widen(char c) const
{
	char const* c_next;
	char_type wc;
	char_type* wc_next;
	std::codecvt_base::result const result = detail::cp1252_from_bytes<char_type, char>(&c, &c + 1, c_next, &wc, &wc + 1, wc_next);
	if (result != std::codecvt_base::ok) {
		wc = char_type(-1);
	}
	return (wc);
}

template<typename WideT>
char const*
ctype_genome<WideT>::do_widen(char const* from, char const* from_end, char_type* to) const
{
	if (to) {
		for (; from < from_end; ++from, ++to) {
			char const* from_next;
			char_type* to_next;
			std::codecvt_base::result const result = detail::cp1252_from_bytes<char_type, char>(from, from + 1, from_next, to, to + 1, to_next);
			if (result != std::codecvt_base::ok) {
				*to = char_type(-1);
			}
		}
	}
	return (from);
}

template<typename WideT>
char
ctype_genome<WideT>::do_narrow(char_type c, char dflt) const
{
	char_type const* c_next;
	char nc;
	char* nc_next;
	std::codecvt_base::result const result = detail::cp1252_to_bytes<char_type, char>(&c, &c + 1, c_next, &nc, &nc + 1, nc_next);
	if (result != std::codecvt_base::ok) {
		nc = dflt;
	}
	return (nc);
}

template<typename WideT>
typename ctype_genome<WideT>::char_type const*
ctype_genome<WideT>::do_narrow(char_type const* from, char_type const* from_end, char dflt, char* to) const
{
	for (; from < from_end; ++from, ++to) {
		char_type const* from_next;
		char* to_next;
		std::codecvt_base::result const result = detail::cp1252_to_bytes<char_type, char>(from, from + 1, from_next, to, to + 1, to_next);
		if (result != std::codecvt_base::ok) {
			*to = dflt;
		}
	}
	return (from);
}

//
// codecvt_genome<...>
//

template<typename InternT, typename ExternT, typename StateT>
codecvt_genome<InternT, ExternT, StateT>::codecvt_genome(std::size_t refs)
	: base_type(refs)
{
}

template<typename InternT, typename ExternT, typename StateT>
codecvt_genome<InternT, ExternT, StateT>::~codecvt_genome(void)
{
}

template<typename InternT, typename ExternT, typename StateT>
bool
codecvt_genome<InternT, ExternT, StateT>::do_always_noconv(void) const GENOME_NOEXCEPT_NOTHROW
{
	return (false);
}

template<typename InternT, typename ExternT, typename StateT>
int
codecvt_genome<InternT, ExternT, StateT>::do_encoding(void) const GENOME_NOEXCEPT_NOTHROW
{
	return (1);
}

template<typename InternT, typename ExternT, typename StateT>
typename codecvt_genome<InternT, ExternT, StateT>::result
codecvt_genome<InternT, ExternT, StateT>::do_in(state_type& state,
	extern_type const* from, extern_type const* from_end, extern_type const*& from_next,
	intern_type* to, intern_type* to_end, intern_type*& to_next) const
{
	unused_parameter(state);
	return (detail::cp1252_from_bytes(from, from_end, from_next, to, to_end, to_next));
}

template<typename InternT, typename ExternT, typename StateT>
int
codecvt_genome<InternT, ExternT, StateT>::do_length(do_length_state_type& state,
	extern_type const* from, extern_type const* from_end, std::size_t max) const
{
	unused_parameter(state);
	return (static_cast<int>(std::min(max, static_cast<std::size_t>(from_end - from))));
}

template<typename InternT, typename ExternT, typename StateT>
int
codecvt_genome<InternT, ExternT, StateT>::do_max_length(void) const GENOME_NOEXCEPT_NOTHROW
{
	return (1);
}

template<typename InternT, typename ExternT, typename StateT>
typename codecvt_genome<InternT, ExternT, StateT>::result
codecvt_genome<InternT, ExternT, StateT>::do_out(state_type& state,
	intern_type const* from, intern_type const* from_end, intern_type const*& from_next,
	extern_type* to, extern_type* to_end, extern_type*& to_next) const
{
	unused_parameter(state);
	return (detail::cp1252_to_bytes(from, from_end, from_next, to, to_end, to_next));
}

template<typename InternT, typename ExternT, typename StateT>
typename codecvt_genome<InternT, ExternT, StateT>::result
codecvt_genome<InternT, ExternT, StateT>::do_unshift(state_type& state,
	extern_type* to, extern_type* to_end, extern_type*& to_next) const
{
	unused_parameter(state);
	unused_parameter(to_end);
	to_next = to;
	return (std::codecvt_base::ok);
}

} // namespace genome

#endif // GENOME_LOCALE_IPP
