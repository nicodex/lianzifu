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
#ifndef GENOME_LOCALE_HPP
#define GENOME_LOCALE_HPP

#include <genome/genome.hpp>
#include <genome/string.hpp>
#include <cwchar>
#include <locale>
#include <string>

namespace genome {

namespace locale {

// called by init_genome()
void init(void);

// standard Genome locale
std::locale const& global(void);

} // namespace genome::locale

namespace codecvt {

// Like std::codecvt_mode, but it has been deprecated with C++17,
// and I don't like that the base namespace is polluted with very
// generic identifier names that are specific to UTF-8 converters.
enum mode {
	big_endian      = 0,
	little_endian   = (1 << 0),
	generate_header = (1 << 1),
	consume_header  = (1 << 2)
};

} // namespace genome::codecvt

} // namespace genome

#include <genome/locale_detail.hpp>

namespace genome {

//
// codecvt_utf8<...>
//

template<typename WideT, unsigned long Maxcode = 0x0010FFFFUL, codecvt::mode Mode = codecvt::big_endian>
class codecvt_utf8 : public detail::codecvt_utf_base<WideT, Maxcode, Mode> {
	typedef detail::codecvt_utf_base<WideT, Maxcode, Mode> base_type;
	codecvt_utf8(codecvt_utf8 const&) GENOME_DELETE_FUNCTION;
	codecvt_utf8& operator=(codecvt_utf8 const&) GENOME_DELETE_FUNCTION;
public:
	enum config {
		max_code = static_cast<unsigned long>(
			((sizeof(WideT) * CHAR_BIT) >= 21) ? (
				(Maxcode >= 0x0010FFFFUL) ? 0x0010FFFFUL : Maxcode
			) : (
				(Maxcode >= 0xFFFFUL) ? 0xFFFFUL : Maxcode
			))
	};
	typedef typename base_type::result result;
	typedef typename base_type::intern_type intern_type;
	typedef typename base_type::extern_type extern_type;
	typedef typename base_type::state_type state_type;
	typedef typename base_type::state_cast state_cast;
	typedef typename base_type::do_length_state_type do_length_state_type;
	typedef typename base_type::do_length_state_alias do_length_state_alias;
	explicit codecvt_utf8(std::size_t refs = 0);
	virtual ~codecvt_utf8(void);
protected:
	virtual int do_max_length(void) const GENOME_NOEXCEPT_NOTHROW GENOME_OVERRIDE;
	virtual result do_in(state_type& state,
		extern_type const* from, extern_type const* from_end, extern_type const*& from_next,
		intern_type* to, intern_type* to_end, intern_type*& to_next) const GENOME_OVERRIDE;
	virtual result do_out(state_type& state,
		intern_type const* from, intern_type const* from_end, intern_type const*& from_next,
		extern_type* to, extern_type* to_end, extern_type*& to_next) const GENOME_OVERRIDE;
};

//
// codecvt_utf8_utf16<...>
//

template<typename WideT, unsigned long Maxcode = 0x0010FFFFUL, codecvt::mode Mode = codecvt::big_endian>
class codecvt_utf8_utf16 : public detail::codecvt_utf_base<WideT, Maxcode, Mode> {
	typedef detail::codecvt_utf_base<WideT, Maxcode, Mode> base_type;
	codecvt_utf8_utf16(codecvt_utf8_utf16 const&) GENOME_DELETE_FUNCTION;
	codecvt_utf8_utf16& operator=(codecvt_utf8_utf16 const&) GENOME_DELETE_FUNCTION;
public:
	enum config {
		max_code = static_cast<unsigned long>(
			(Maxcode >= 0x0010FFFFUL) ? 0x0010FFFFUL : Maxcode)
	};
	typedef typename base_type::result result;
	typedef typename base_type::intern_type intern_type;
	typedef typename base_type::extern_type extern_type;
	typedef typename base_type::state_type state_type;
	typedef typename base_type::state_cast state_cast;
	typedef typename base_type::do_length_state_type do_length_state_type;
	typedef typename base_type::do_length_state_alias do_length_state_alias;
	explicit codecvt_utf8_utf16(std::size_t refs = 0);
	virtual ~codecvt_utf8_utf16(void);
protected:
	virtual int do_max_length(void) const GENOME_NOEXCEPT_NOTHROW GENOME_OVERRIDE;
	virtual result do_in(state_type& state,
		extern_type const* from, extern_type const* from_end, extern_type const*& from_next,
		intern_type* to, intern_type* to_end, intern_type*& to_next) const GENOME_OVERRIDE;
	virtual result do_out(state_type& state,
		intern_type const* from, intern_type const* from_end, intern_type const*& from_next,
		extern_type* to, extern_type* to_end, extern_type*& to_next) const GENOME_OVERRIDE;
};

//
// codecvt_utf16<...>
//

template<typename WideT, unsigned long Maxcode = 0x0010FFFFUL, codecvt::mode Mode = codecvt::big_endian>
class codecvt_utf16 : public detail::codecvt_utf_base<WideT, Maxcode, Mode> {
	typedef detail::codecvt_utf_base<WideT, Maxcode, Mode> base_type;
	codecvt_utf16(codecvt_utf16 const&) GENOME_DELETE_FUNCTION;
	codecvt_utf16& operator=(codecvt_utf16 const&) GENOME_DELETE_FUNCTION;
public:
	enum config {
		max_code = static_cast<unsigned long>(
			((sizeof(WideT) * CHAR_BIT) >= 21) ? (
				(Maxcode >= 0x0010FFFFUL) ? 0x0010FFFFUL : Maxcode
			) : (
				(Maxcode >= 0xFFFFUL) ? 0xFFFFUL : Maxcode
			))
	};
	typedef typename base_type::result result;
	typedef typename base_type::intern_type intern_type;
	typedef typename base_type::extern_type extern_type;
	typedef typename base_type::state_type state_type;
	typedef typename base_type::state_cast state_cast;
	typedef typename base_type::do_length_state_type do_length_state_type;
	typedef typename base_type::do_length_state_alias do_length_state_alias;
	explicit codecvt_utf16(std::size_t refs = 0);
	virtual ~codecvt_utf16(void);
protected:
	virtual int do_max_length(void) const GENOME_NOEXCEPT_NOTHROW GENOME_OVERRIDE;
	virtual result do_in(state_type& state,
		extern_type const* from, extern_type const* from_end, extern_type const*& from_next,
		intern_type* to, intern_type* to_end, intern_type*& to_next) const GENOME_OVERRIDE;
	virtual result do_out(state_type& state,
		intern_type const* from, intern_type const* from_end, intern_type const*& from_next,
		extern_type* to, extern_type* to_end, extern_type*& to_next) const GENOME_OVERRIDE;
};

//
// wstring_convert<...>
//

template<typename Codecvt, typename Elem = wchar_t, typename Wide_alloc = std::allocator<Elem>, typename Byte_alloc = std::allocator<char> >
class wstring_convert {
	wstring_convert(wstring_convert const&) GENOME_DELETE_FUNCTION;
	wstring_convert& operator=(wstring_convert const&) GENOME_DELETE_FUNCTION;
public:
	typedef std::basic_string<char, std::char_traits<char>, Byte_alloc> byte_string;
	typedef std::basic_string<Elem, std::char_traits<Elem>, Wide_alloc> wide_string;
	typedef typename Codecvt::state_type state_type;
	typedef typename wide_string::traits_type::int_type int_type;
	explicit wstring_convert(Codecvt* pcvt = new Codecvt);
	wstring_convert(Codecvt* pcvt, state_type state);
	explicit wstring_convert(byte_string const& byte_err);
	wstring_convert(byte_string const& byte_err, wide_string const& wide_err);
	virtual ~wstring_convert(void) GENOME_NOEXCEPT_NOTHROW;
	wide_string from_bytes(char byte);
	wide_string from_bytes(char const* ptr);
	wide_string from_bytes(byte_string const& str);
	wide_string from_bytes(char const* first, char const* last);
	byte_string to_bytes(Elem wchar);
	byte_string to_bytes(Elem const* wptr);
	byte_string to_bytes(wide_string const& wstr);
	byte_string to_bytes(Elem const* first, Elem const* last);
	state_type state(void) const GENOME_NOEXCEPT_NOTHROW;
	std::size_t converted(void) const GENOME_NOEXCEPT_NOTHROW;
private:
	Codecvt* const m_codecvt;
	std::locale m_locale;
	state_type m_state;
	byte_string const m_byte_err;
	wide_string const m_wide_err;
	std::size_t m_converted;
	bool const m_has_state;
	bool const m_has_byte_err;
	bool const m_has_wide_err;
};

//
// ctype_genome<...>
//

template<typename WideT>
class ctype_genome : public std::ctype<WideT> {
	typedef std::ctype<WideT> base_type;
	ctype_genome(ctype_genome const&) GENOME_DELETE_FUNCTION;
	ctype_genome& operator=(ctype_genome const&) GENOME_DELETE_FUNCTION;
public:
	typedef typename base_type::char_type char_type;
	typedef typename base_type::mask mask;
	GENOME_STATIC_ASSERT(wide_size, (sizeof(char_type) * CHAR_BIT) >= 16,
		"Type of a wide char Genome ctype must have at least 16 bits.");
	explicit ctype_genome(std::size_t refs = 0);
	virtual ~ctype_genome(void);
protected:
	virtual bool do_is(mask m, char_type c) const GENOME_OVERRIDE;
	virtual char_type const* do_is(char_type const* str, char_type const* end, mask* m) const GENOME_OVERRIDE;
	virtual char_type const* do_scan_is(mask m, char_type const* str, char_type const* end) const GENOME_OVERRIDE;
	virtual char_type const* do_scan_not(mask m, char_type const* str, char_type const* end) const GENOME_OVERRIDE;
	virtual char_type do_toupper(char_type c) const GENOME_OVERRIDE;
	virtual char_type const* do_toupper(char_type* str, char_type const* end) const GENOME_OVERRIDE;
	virtual char_type do_tolower(char_type c) const GENOME_OVERRIDE;
	virtual char_type const* do_tolower(char_type* str, char_type const* end) const GENOME_OVERRIDE;
	virtual char_type do_widen(char c) const GENOME_OVERRIDE;
	virtual char const* do_widen(char const* from, char const* from_end, char_type* to) const GENOME_OVERRIDE;
	virtual char do_narrow(char_type c, char dflt) const GENOME_OVERRIDE;
	virtual char_type const* do_narrow(char_type const* from, char_type const* from_end, char dflt, char* to) const GENOME_OVERRIDE;
};

//
// ctype_genome<wide_char>
//

#if !!GENOME_EXTERN_TEMPLATES
extern template class ctype_genome<wide_char>;
#endif

//
// ctype_genome<byte_char>
//

template<>
class ctype_genome<byte_char> : public std::ctype<byte_char> {
	typedef std::ctype<byte_char> base_type;
	ctype_genome(ctype_genome const&) GENOME_DELETE_FUNCTION;
	ctype_genome& operator=(ctype_genome const&) GENOME_DELETE_FUNCTION;
public:
	typedef base_type::char_type char_type;
	typedef base_type::mask mask;
	explicit ctype_genome(std::size_t refs = 0);
	virtual ~ctype_genome(void);
protected:
	virtual bool do_is(mask m, char_type c) const GENOME_OVERRIDE;
	virtual char_type const* do_is(char_type const* str, char_type const* end, mask* m) const GENOME_OVERRIDE;
	virtual char_type const* do_scan_is(mask m, char_type const* str, char_type const* end) const GENOME_OVERRIDE;
	virtual char_type const* do_scan_not(mask m, char_type const* str, char_type const* end) const GENOME_OVERRIDE;
	virtual char_type do_toupper(char_type c) const GENOME_OVERRIDE;
	virtual char_type const* do_toupper(char_type* str, char_type const* end) const GENOME_OVERRIDE;
	virtual char_type do_tolower(char_type c) const GENOME_OVERRIDE;
	virtual char_type const* do_tolower(char_type* str, char_type const* end) const GENOME_OVERRIDE;
	virtual char_type do_widen(char c) const GENOME_OVERRIDE;
	virtual char const* do_widen(char const* from, char const* from_end, char_type* to) const GENOME_OVERRIDE;
	virtual char do_narrow(char_type c, char dflt) const GENOME_OVERRIDE;
	virtual char_type const* do_narrow(char_type const* from, char_type const* from_end, char dflt, char* to) const GENOME_OVERRIDE;
};

#if !!GENOME_EXTERN_TEMPLATES
extern template class ctype_genome<byte_char>;
#endif

//
// codecvt_genome<...>
//

template<typename InternT, typename ExternT = char, typename StateT = std::mbstate_t>
class codecvt_genome : public detail::codecvt_base<InternT, ExternT, StateT> {
	typedef detail::codecvt_base<InternT, ExternT, StateT> base_type;
	codecvt_genome(codecvt_genome const&) GENOME_DELETE_FUNCTION;
	codecvt_genome& operator=(codecvt_genome const&) GENOME_DELETE_FUNCTION;
public:
	typedef typename base_type::result result;
	typedef typename base_type::intern_type intern_type;
	typedef typename base_type::extern_type extern_type;
	typedef typename base_type::state_type state_type;
	typedef typename base_type::do_length_state_type do_length_state_type;
	typedef typename base_type::do_length_state_alias do_length_state_alias;
	GENOME_STATIC_ASSERT(intern_size, (sizeof(intern_type) * CHAR_BIT) >= 16,
		"Internal type of a Genome codecvt must have at least 16 bits.");
	explicit codecvt_genome(std::size_t refs = 0);
	virtual ~codecvt_genome(void);
protected:
	virtual bool do_always_noconv(void) const GENOME_NOEXCEPT_NOTHROW GENOME_OVERRIDE;
	virtual int do_encoding(void) const GENOME_NOEXCEPT_NOTHROW GENOME_OVERRIDE;
	virtual result do_in(state_type& state,
		extern_type const* from, extern_type const* from_end, extern_type const*& from_next,
		intern_type* to, intern_type* to_end, intern_type*& to_next) const GENOME_OVERRIDE;
	virtual int do_length(do_length_state_type& state,
		extern_type const* from, extern_type const* from_end, std::size_t max) const GENOME_OVERRIDE;
	virtual int do_max_length(void) const GENOME_NOEXCEPT_NOTHROW GENOME_OVERRIDE;
	virtual result do_out(state_type& state,
		intern_type const* from, intern_type const* from_end, intern_type const*& from_next,
		extern_type* to, extern_type* to_end, extern_type*& to_next) const GENOME_OVERRIDE;
	virtual result do_unshift(state_type& state,
		extern_type* to, extern_type* to_end, extern_type*& to_next) const GENOME_OVERRIDE;
};

//
// codecvt_genome<wide_char, char, std::mbstate_t>
//

#if !!GENOME_EXTERN_TEMPLATES
extern template class codecvt_genome<wide_char, char, std::mbstate_t>;
#endif

//
// codecvt_genome<byte_char, char, std::mbstate_t>
//

template<>
class codecvt_genome<byte_char, char, std::mbstate_t> : public detail::codecvt_base<byte_char, char, std::mbstate_t> {
	typedef detail::codecvt_base<byte_char, char, std::mbstate_t> base_type;
	codecvt_genome(codecvt_genome const&) GENOME_DELETE_FUNCTION;
	codecvt_genome& operator=(codecvt_genome const&) GENOME_DELETE_FUNCTION;
public:
	typedef base_type::result result;
	typedef base_type::intern_type intern_type;
	typedef base_type::extern_type extern_type;
	typedef base_type::state_type state_type;
	typedef base_type::do_length_state_type do_length_state_type;
	typedef base_type::do_length_state_alias do_length_state_alias;
	explicit codecvt_genome(std::size_t refs = 0);
	virtual ~codecvt_genome(void);
protected:
	virtual bool do_always_noconv(void) const GENOME_NOEXCEPT_NOTHROW GENOME_OVERRIDE;
	virtual int do_encoding(void) const GENOME_NOEXCEPT_NOTHROW GENOME_OVERRIDE;
	virtual result do_in(state_type& state,
		extern_type const* from, extern_type const* from_end, extern_type const*& from_next,
		intern_type* to, intern_type* to_end, intern_type*& to_next) const GENOME_OVERRIDE;
	virtual int do_length(do_length_state_type& state,
		extern_type const* from, extern_type const* from_end, std::size_t max) const GENOME_OVERRIDE;
	virtual int do_max_length(void) const GENOME_NOEXCEPT_NOTHROW GENOME_OVERRIDE;
	virtual result do_out(state_type& state,
		intern_type const* from, intern_type const* from_end, intern_type const*& from_next,
		extern_type* to, extern_type* to_end, extern_type*& to_next) const GENOME_OVERRIDE;
	virtual result do_unshift(state_type& state,
		extern_type* to, extern_type* to_end, extern_type*& to_next) const GENOME_OVERRIDE;
};
#if !!GENOME_EXTERN_TEMPLATES
extern template class codecvt_genome<byte_char, char, std::mbstate_t>;
#endif

} // namespace genome

#include <genome/locale.ipp>

#endif // GENOME_LOCALE_HPP
