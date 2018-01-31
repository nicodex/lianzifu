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
#include <genome/locale.hpp>
#include <cassert>
#include <algorithm>

#include <iostream>

namespace genome {

namespace locale {

	void
	init(void)
	{
		global();
	}

	std::locale const&
	global(void)
	{
		//NOTE: This is not thread-safe until your compiler waits for the completion of variable initializations.
		// However, locale::genome() is called from locale::init(), which in turn is called from genome::init(),
		// which is expected to be called during application initialization before anything else.
		static std::locale s_global =
			std::locale(
				std::locale(
					std::locale(
						std::locale(
							std::locale(),
							new ctype_genome<byte_char>
						),
						new ctype_genome<wide_char>
					),
					new codecvt_genome<byte_char, char, std::mbstate_t>
				),
				new codecvt_genome<wide_char, char, std::mbstate_t>
			);
		return (s_global);
	}

} // namespace genome::locale

//
// ctype_genome<wide_char>
//

#if !!GENOME_EXTERN_TEMPLATES
template class ctype_genome<wide_char>;
#endif

//
// ctype_genome<byte_char>
//

ctype_genome<byte_char>::ctype_genome(std::size_t refs)
	: base_type(refs)
{
}

ctype_genome<byte_char>::~ctype_genome(void)
{
}

bool
ctype_genome<byte_char>::do_is(mask m, char_type c) const
{
	return ((detail::ctype_genome_table[c] & m) != 0);
}

ctype_genome<byte_char>::char_type const*
ctype_genome<byte_char>::do_is(char_type const* str, char_type const* end, mask* m) const
{
	for (; str < end; ++str, ++m) {
		*m = detail::ctype_genome_table[*str];
	}
	return (str);
}

ctype_genome<byte_char>::char_type const*
ctype_genome<byte_char>::do_scan_is(mask m, char_type const* str, char_type const* end) const
{
	for (; (str < end) && (0 == (detail::ctype_genome_table[*str] & m)); ++str) {
	}
	return (str);
}

ctype_genome<byte_char>::char_type const*
ctype_genome<byte_char>::do_scan_not(mask m, char_type const* str, char_type const* end) const
{
	for (; (str < end) && ((detail::ctype_genome_table[*str] & m) != 0); ++str) {
	}
	return (str);
}

ctype_genome<byte_char>::char_type
ctype_genome<byte_char>::do_toupper(char_type c) const
{
	return (byte_code::to_upper(c));
}

ctype_genome<byte_char>::char_type const*
ctype_genome<byte_char>::do_toupper(char_type* str, char_type const* end) const
{
	for (; str < end; ++str) {
		*str = byte_code::to_upper(*str);
	}
	return (str);
}

ctype_genome<byte_char>::char_type
ctype_genome<byte_char>::do_tolower(char_type c) const
{
	return (byte_code::to_lower(c));
}

ctype_genome<byte_char>::char_type const*
ctype_genome<byte_char>::do_tolower(char_type* str, char_type const* end) const
{
	for (; str < end; ++str) {
		*str = byte_code::to_lower(*str);
	}
	return (str);
}

ctype_genome<byte_char>::char_type
ctype_genome<byte_char>::do_widen(char c) const
{
	return (static_cast<char_type>(static_cast<unsigned char>(c)));
}

char const*
ctype_genome<byte_char>::do_widen(char const* from, char const* from_end, char_type* to) const
{
	for (; from < from_end; ++from, ++to) {
		*to = static_cast<char_type>(static_cast<unsigned char>(*from));
	}
	return (from);
}

char
ctype_genome<byte_char>::do_narrow(char_type c, char dflt) const
{
	unused_parameter(dflt);
	return (static_cast<char>(static_cast<unsigned char>(c)));
}

ctype_genome<byte_char>::char_type const*
ctype_genome<byte_char>::do_narrow(char_type const* from, char_type const* from_end, char dflt, char* to) const
{
	unused_parameter(dflt);
	for (; from < from_end; ++from, ++to) {
		*to = static_cast<char>(static_cast<unsigned char>(*from));
	}
	return (from);
}

#if !!GENOME_EXTERN_TEMPLATES
template class ctype_genome<byte_char>;
#endif

//
// codecvt_genome<wide_char, char, std::mbstate_t>
//

#if !!GENOME_EXTERN_TEMPLATES
template class codecvt_genome<wide_char, char, std::mbstate_t>;
#endif

//
// codecvt_genome<byte_char, char, std::mbstate_t>
//

codecvt_genome<byte_char, char, std::mbstate_t>::codecvt_genome(std::size_t refs)
	: base_type(refs)
{
}

codecvt_genome<byte_char, char, std::mbstate_t>::~codecvt_genome(void)
{
}

bool
codecvt_genome<byte_char, char, std::mbstate_t>::do_always_noconv(void) const GENOME_NOEXCEPT_NOTHROW
{
	return (true);
}

int
codecvt_genome<byte_char, char, std::mbstate_t>::do_encoding(void) const GENOME_NOEXCEPT_NOTHROW
{
	return (1);
}

codecvt_genome<byte_char, char, std::mbstate_t>::result
codecvt_genome<byte_char, char, std::mbstate_t>::do_in(state_type& state,
	extern_type const* from, extern_type const* from_end, extern_type const*& from_next,
	intern_type* to, intern_type* to_end, intern_type*& to_next) const
{
	unused_parameter(state);
	unused_parameter(from_end);
	unused_parameter(to_end);
	from_next = from;
	to_next = to;
	return (std::codecvt_base::noconv);
}

int
codecvt_genome<byte_char, char, std::mbstate_t>::do_length(do_length_state_type& state,
	extern_type const* from, extern_type const* from_end, std::size_t max) const
{
	unused_parameter(state);
	return (static_cast<int>(std::min(max, static_cast<std::size_t>(from_end - from))));
}

int
codecvt_genome<byte_char, char, std::mbstate_t>::do_max_length(void) const GENOME_NOEXCEPT_NOTHROW
{
	return (1);
}

codecvt_genome<byte_char, char, std::mbstate_t>::result
codecvt_genome<byte_char, char, std::mbstate_t>::do_out(state_type& state,
	intern_type const* from, intern_type const* from_end, intern_type const*& from_next,
	extern_type* to, extern_type* to_end, extern_type*& to_next) const
{
	unused_parameter(state);
	unused_parameter(from_end);
	unused_parameter(to_end);
	from_next = from;
	to_next = to;
	return (std::codecvt_base::noconv);
}

codecvt_genome<byte_char, char, std::mbstate_t>::result
codecvt_genome<byte_char, char, std::mbstate_t>::do_unshift(state_type& state,
	extern_type* to, extern_type* to_end, extern_type*& to_next) const
{
	unused_parameter(state);
	unused_parameter(to_end);
	to_next = to;
	return (std::codecvt_base::noconv);
}

#if !!GENOME_EXTERN_TEMPLATES
template class codecvt_genome<byte_char, char, std::mbstate_t>;
#endif

} // namespace genome
