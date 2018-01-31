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
#ifndef GENOME_LOCALE_DETAIL_HPP
#define GENOME_LOCALE_DETAIL_HPP

namespace genome {
namespace detail {

//
// codecvt_base<...>
//

template<typename InternT, typename ExternT, typename StateT>
class codecvt_base : public std::codecvt<InternT, ExternT, StateT> {
	typedef std::codecvt<InternT, ExternT, StateT> base_type;
	codecvt_base(codecvt_base const&) GENOME_DELETE_FUNCTION;
	codecvt_base& operator=(codecvt_base const&) GENOME_DELETE_FUNCTION;
public:
	typedef typename base_type::result result;
	typedef typename base_type::intern_type intern_type;
	typedef typename base_type::extern_type extern_type;
	typedef typename base_type::state_type state_type;
#if defined(_CPPLIB_VER) && (0 < _CPPLIB_VER) && (_CPPLIB_VER < 540)  // < Visual C++ 2012
	// older Microsoft/Dinkumware (non-standard do_length signature)
	typedef state_type const do_length_state_type;
	typedef state_type       do_length_state_alias;
#else
	typedef state_type       do_length_state_type;
	typedef state_type&      do_length_state_alias;
#endif
protected:
	explicit codecvt_base(std::size_t refs = 0);
	virtual ~codecvt_base(void);
	virtual bool do_always_noconv(void) const GENOME_NOEXCEPT_NOTHROW GENOME_OVERRIDE = 0;
	virtual int do_encoding(void) const GENOME_NOEXCEPT_NOTHROW GENOME_OVERRIDE = 0;
	virtual result do_in(state_type& state,
		extern_type const* from, extern_type const* from_end, extern_type const*& from_next,
		intern_type* to, intern_type* to_end, intern_type*& to_next) const GENOME_OVERRIDE = 0;
	virtual int do_length(do_length_state_type& state,
		extern_type const* from, extern_type const* from_end, std::size_t max) const GENOME_OVERRIDE = 0;
	virtual int do_max_length(void) const GENOME_NOEXCEPT_NOTHROW GENOME_OVERRIDE = 0;
	virtual result do_out(state_type& state,
		intern_type const* from, intern_type const* from_end, intern_type const*& from_next,
		extern_type* to, extern_type* to_end, extern_type*& to_next) const GENOME_OVERRIDE = 0;
	virtual result do_unshift(state_type& state,
		extern_type* to, extern_type* to_end, extern_type*& to_next) const GENOME_OVERRIDE = 0;
};

//
// UTF-8/16 state (std::mbstate_t)
//

template<typename StateT>
class codecvt_utf_state {
	codecvt_utf_state(codecvt_utf_state const&) GENOME_DELETE_FUNCTION;
	codecvt_utf_state& operator=(codecvt_utf_state const&) GENOME_DELETE_FUNCTION;
	enum config {
		bitfield_header_mask          = u16(1 << 0),
		bitfield_is_little_mask       = u16(1 << 1),
		bitfield_has_surrogate_mask   = u16(1 << 2),
		bitfield_high_surrogate_shift = 3,
		bitfield_high_surrogate_merge = u16((1 << 3) - 1)
	};
	// Internal structure to access the memory of a std::mbstate_t.
	// std::mbstate_t must have at least 32 bits for this to work.
	struct utf_state_t {
		u16 bitfield;
		u16 surrogate;
	} &utf_state;
	GENOME_STATIC_ASSERT(state_size, sizeof(StateT) >= sizeof(utf_state_t),
		"The UTF codecvt state does not fit into the state.");
public:
	codecvt_utf_state(StateT& state);
	bool is_initial(void) const;
	bool do_header(void);
	void undo_header(void);
	bool is_little(void) const;
	void set_little(void);
	bool has_surrogate(void) const;
	void set_surrogate(unsigned short bits);
	void unset_surrogate(void);
	unsigned short get_surrogate(void) const;
	void set_high_surrogate(unsigned short bits);
	unsigned long get_full_surrogate(unsigned char lso) const;
};

//
// codecvt_utf_base<...>
//

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
class codecvt_utf_base : public codecvt_base<WideT, char, std::mbstate_t> {
	typedef codecvt_base<WideT, char, std::mbstate_t> base_type;
	codecvt_utf_base(codecvt_utf_base const&) GENOME_DELETE_FUNCTION;
	codecvt_utf_base& operator=(codecvt_utf_base const&) GENOME_DELETE_FUNCTION;
public:
	typedef typename base_type::result result;
	typedef typename base_type::intern_type intern_type;
	typedef typename base_type::extern_type extern_type;
	typedef typename base_type::state_type state_type;
	typedef codecvt_utf_state<state_type> state_cast;
	typedef typename base_type::do_length_state_type do_length_state_type;
	typedef typename base_type::do_length_state_alias do_length_state_alias;
	GENOME_STATIC_ASSERT(intern_size, (sizeof(intern_type) * CHAR_BIT) >= 16,
		"The internal type of an UTF codecvt must have at least 16 bits.");
protected:
	explicit codecvt_utf_base(std::size_t refs = 0);
	virtual ~codecvt_utf_base(void);
	virtual bool do_always_noconv(void) const GENOME_NOEXCEPT_NOTHROW GENOME_OVERRIDE;
	virtual int do_encoding(void) const GENOME_NOEXCEPT_NOTHROW GENOME_OVERRIDE;
	virtual result do_in(state_type& state,
		extern_type const* from, extern_type const* from_end, extern_type const*& from_next,
		intern_type* to, intern_type* to_end, intern_type*& to_next) const GENOME_OVERRIDE = 0;
	virtual int do_length(do_length_state_type& state,
		extern_type const* from, extern_type const* from_end, std::size_t max) const GENOME_OVERRIDE;
	virtual int do_max_length(void) const GENOME_NOEXCEPT_NOTHROW GENOME_OVERRIDE = 0;
	virtual result do_out(state_type& state,
		intern_type const* from, intern_type const* from_end, intern_type const*& from_next,
		extern_type* to, extern_type* to_end, extern_type*& to_next) const GENOME_OVERRIDE = 0;
	virtual result do_unshift(state_type& state,
		extern_type* to, extern_type* to_end, extern_type*& to_next) const GENOME_OVERRIDE;
};

//
// UTF-8 conversion
//

template<typename WideT, typename ByteT, bool UTF16, unsigned long Maxcode, bool Header>
std::codecvt_base::result
utf8_from_bytes(std::mbstate_t& state,  // codecvt_utf_state
	ByteT const* from, ByteT const* from_end, ByteT const*& from_next,
	WideT* to, WideT* to_end, WideT*& to_next);
template<typename WideT, typename ByteT, bool UTF16, unsigned long Maxcode, bool Header>
std::codecvt_base::result
utf8_to_bytes(std::mbstate_t& state,  // codecvt_utf_state
	WideT const* from, WideT const* from_end, WideT const*& from_next,
	ByteT* to, ByteT* to_end, ByteT*& to_next);

//
// UTF-16 conversion
//

template<typename WideT, typename ByteT, bool Little, unsigned long Maxcode, bool Header>
std::codecvt_base::result
utf16_from_bytes(std::mbstate_t& state,  // codecvt_utf_state
	ByteT const* from, ByteT const* from_end, ByteT const*& from_next,
	WideT* to, WideT* to_end, WideT*& to_next);
template<typename WideT, typename ByteT, bool Little, unsigned long Maxcode, bool Header>
std::codecvt_base::result
utf16_to_bytes(std::mbstate_t& state,  // codecvt_utf_state
	WideT const* from, WideT const* from_end, WideT const*& from_next,
	ByteT* to, ByteT* to_end, ByteT*& to_next);

//
// Windows-1252 conversion
//

template<typename WideT, typename ByteT>
std::codecvt_base::result
cp1252_from_bytes(
	ByteT const* from, ByteT const* from_end, ByteT const*& from_next,
	WideT* to, WideT* to_end, WideT*& to_next);
template<typename WideT, typename ByteT>
std::codecvt_base::result
cp1252_to_bytes(
	WideT const* from, WideT const* from_end, WideT const*& from_next,
	ByteT* to, ByteT* to_end, ByteT*& to_next);

//
// ctype_genome<...>
//

extern
std::ctype_base::mask const
ctype_genome_table[256];

} // namespace genome::detail
} // namespace genome

#endif // GENOME_LOCALE_DETAIL_HPP