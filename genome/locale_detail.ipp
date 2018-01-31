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
#ifndef GENOME_LOCALE_DETAIL_IPP
#define GENOME_LOCALE_DETAIL_IPP

namespace genome {
namespace detail {

//
// codecvt_base<...>
//

template<typename InternT, typename ExternT, typename StateT>
codecvt_base<InternT, ExternT, StateT>::codecvt_base(std::size_t refs)
	: base_type(refs)
{
}

template<typename InternT, typename ExternT, typename StateT>
codecvt_base<InternT, ExternT, StateT>::~codecvt_base(void)
{
}

//
// UTF-8/16 state (std::mbstate_t)
//

template<typename StateT>
codecvt_utf_state<StateT>::codecvt_utf_state(StateT& state)
	: utf_state(reinterpret_cast<utf_state_t&>(state))
{
}

template<typename StateT>
bool
codecvt_utf_state<StateT>::is_initial(void) const
{
	return (!has_surrogate());
}

template<typename StateT>
bool
codecvt_utf_state<StateT>::do_header(void)
{
	if (0 == (bitfield_header_mask & utf_state.bitfield)) {
		utf_state.bitfield |= bitfield_header_mask;
		return (true);
	}
	return (false);
}

template<typename StateT>
void
codecvt_utf_state<StateT>::undo_header(void)
{
	utf_state.bitfield &= u16(~bitfield_header_mask);
}

template<typename StateT>
bool
codecvt_utf_state<StateT>::is_little(void) const
{
	return ((utf_state.bitfield & bitfield_is_little_mask) != 0);
}

template<typename StateT>
void
codecvt_utf_state<StateT>::set_little(void)
{
	utf_state.bitfield |= bitfield_is_little_mask;
}

template<typename StateT>
bool
codecvt_utf_state<StateT>::has_surrogate(void) const
{
	return ((utf_state.bitfield & bitfield_has_surrogate_mask) != 0);
}

template<typename StateT>
void
codecvt_utf_state<StateT>::set_surrogate(unsigned short bits)
{
	utf_state.bitfield |= bitfield_has_surrogate_mask;
	utf_state.surrogate = bits;
}

template<typename StateT>
void
codecvt_utf_state<StateT>::unset_surrogate(void)
{
	utf_state.bitfield &= u16(~bitfield_has_surrogate_mask);
}

template<typename StateT>
unsigned short
codecvt_utf_state<StateT>::get_surrogate(void) const
{
	return (utf_state.surrogate);
}

template<typename StateT>
void
codecvt_utf_state<StateT>::set_high_surrogate(unsigned short bits)
{
	utf_state.bitfield &= bitfield_high_surrogate_merge;
	utf_state.bitfield |= static_cast<u16>
		(bits << bitfield_high_surrogate_shift);
}

template<typename StateT>
unsigned long
codecvt_utf_state<StateT>::get_full_surrogate(unsigned char lso) const
{
	unsigned long code = static_cast<unsigned long>
		(utf_state.bitfield >> bitfield_high_surrogate_shift) << 10;
	code |= get_surrogate() | (lso & 0x3F);
	return (code);
}

//
// codecvt_utf_base<...>
//

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
codecvt_utf_base<WideT, Maxcode, Mode>::codecvt_utf_base(std::size_t refs)
	: base_type(refs)
{
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
codecvt_utf_base<WideT, Maxcode, Mode>::~codecvt_utf_base(void)
{
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
bool
codecvt_utf_base<WideT, Maxcode, Mode>::do_always_noconv(void) const GENOME_NOEXCEPT_NOTHROW
{
	return (false);
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
int
codecvt_utf_base<WideT, Maxcode, Mode>::do_encoding(void) const GENOME_NOEXCEPT_NOTHROW
{
	return (0);
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
int
codecvt_utf_base<WideT, Maxcode, Mode>::do_length(do_length_state_type& state,
	extern_type const* from, extern_type const* from_end, std::size_t max) const
{
	do_length_state_alias alias = state;
	extern_type const* const from_begin = from;
	extern_type const* from_next;
	intern_type to[1];
	intern_type* const to_end = to + 1;
	intern_type* to_next;
	for (std::size_t count = 0; (count < max) && (from != from_end); from = from_next) {
		switch (do_in(alias, from, from_end, from_next, to, to_end, to_next)) {
		case std::codecvt_base::ok:
		case std::codecvt_base::partial:
			if (to_end == to_next) {
				++count;
			} else if (from == from_next) {
				return (static_cast<int>(from - from_begin));
			}
			break;
		case std::codecvt_base::error:
		default:
			return (static_cast<int>(from - from_begin));
		case std::codecvt_base::noconv:
			return (static_cast<int>(std::min(max, static_cast<std::size_t>(from_end - from_begin))));
		}
	}
	return (static_cast<int>(from - from_begin));
}

template<typename WideT, unsigned long Maxcode, codecvt::mode Mode>
typename codecvt_utf_base<WideT, Maxcode, Mode>::result
codecvt_utf_base<WideT, Maxcode, Mode>::do_unshift(state_type& state,
	extern_type* to, extern_type* to_end, extern_type*& to_next) const
{
	unused_parameter(to_end);
	to_next = to;
	if ((Maxcode >= 0x00010000UL) && !state_cast(state).is_initial()) {
		return (std::codecvt_base::error);
	}
	return (std::codecvt_base::ok);
}

///
/// @brief Convert UTF-8 octets to UTF-16 or UTF-32 codes.
///
///  Translates the UTF-8 octets from the source range [from, from_end) to
///  UTF-16/UTF-32 codes, placing the results in the subsequent locations
///  starting at to. Converts no more than from_end - from UTF-8 octets and
///  writes no more than to_end - to UTF-16/UTF-32 codes. Leaves from_next
///  and to_next pointing one beyond the last element successfully converted.
///  Legacy UTF-8 codes (beyond U+10FFFF) and overlong encodings are invalid.
///
///  Requires from <= from_end, to <= to_end, and state is the initial state
///  or is obtained by converting the preceding UTF-8 octets in the sequence.
///
///  The output character type is expected to have at least 16 bits for UTF-16
///  output and at least 21 bits for UTF-32 with code points beyond the BMP.
///  A Maxcode of 0xFFFF limits the output to UCS-2 (Basic Multilingual Plane).
///
///  If UTF-16 output beyond the BMP is enabled (surrogate pairs) the last
///  continuation octet of four-octet UTF-8 codes is separately read to
///  support N:1 conversions (required for std::basic_filebuf). Note that
///  N:1 conversions might fail (if pending fourth octet is invalid)
///  after the high (leading) surrogate has already been written.
///
/// @tparam WideT   Output character type with at least 16 or 21 bits.
/// @tparam ByteT   Input character type with at least 8 bits.
/// @tparam UTF16   Output surrogate pairs for code points beyond the BMP.
/// @tparam Maxcode Largest code point that will be read without error.
/// @tparam Header  Consume BOM, if present at the start of input sequence.
/// @param[in]  from      Begin or continuation of UTF-8 input sequence.
/// @param[in]  from_end  End of the UTF-8 input sequence (>= from).
/// @param[out] from_next End of successfully converted input sequence.
/// @param[in]  to        Begin of UTF-16/UTF-32 output buffer.
/// @param[in]  to_end    End of the UTF-16/UTF-32 output buffer (>= to).
/// @param[out] to_next   End of successfully converted output sequence.
/// @retval std::codecvt_base::ok      Conversion completed.
/// @retval std::codecvt_base::error   Encountered an invalid character.
/// @retval std::codecvt_base::partial Not enough space in the output buffer
///                                    or unexpected end of source buffer.
///
template<typename WideT, typename ByteT, bool UTF16, unsigned long Maxcode, bool Header>
std::codecvt_base::result
utf8_from_bytes(std::mbstate_t& state,  // codecvt_utf_state
	ByteT const* from, ByteT const* from_end, ByteT const*& from_next,
	WideT* to, WideT* to_end, WideT*& to_next)
{
	// unused features are expected to be optimized away by the compiler
	// due to constant expressions that depend on template parameters or
	// the size of the external (ByteT) and internal (WideT) characters
	typedef codecvt_utf_state<std::mbstate_t> state_cast;
	enum config {
		surrogates = bool(
			UTF16 &&
			(Maxcode >= 0x00010000UL)
		),
		beyond_bmp = bool(
			surrogates ||
			(
				!UTF16 &&
				(Maxcode >= 0x00010000UL) &&
				((sizeof(WideT) * CHAR_BIT) >= 21)
			)
		),
		max_code = static_cast<unsigned long>(
			(beyond_bmp && (Maxcode >= 0x0010FFFFUL)) ? 0x0010FFFFUL : (
			(!beyond_bmp && (Maxcode >= 0xFFFFUL)) ? 0xFFFFUL : (
			Maxcode)))
	};
	for (from_next = from, to_next = to; (from != from_end) && (to_next != to_end); from_next = from, ++to_next) {
		if ((CHAR_BIT * sizeof(ByteT)) > 8) {
			if ((*from < 0) || (0xFF < *from)) {
				return (std::codecvt_base::error);
			}
		}
		unsigned char octet = static_cast<unsigned char>(*from++);
		// The last continuation octet of surrogate pairs is separately read
		// to support N:1 conversions (is required for std::basic_filebuf).
		if (surrogates && state_cast(state).has_surrogate()) {
			// complete low (trailing) surrogate with last 6 bits
			if ((octet < 0x80) || ((0x80 | 0x3F) < octet) ||
			    (
			        ((max_code & 0x3F) != 0x3F) &&
			        ((max_code - 0x00010000UL) < state_cast(state).get_full_surrogate(octet)))
			    ) {
				// revert source octets and high surrogate for N:M conversions
				if (to_next != to) {
					from_next -= 4 - 1;
					to_next -= 1;
					state_cast(state).unset_surrogate();
				}
				return (std::codecvt_base::error);
			}
			unsigned short word = static_cast<unsigned short>(
				0xDC00U |
				state_cast(state).get_surrogate() |
				(octet & 0x3F)
			);
			*to_next = static_cast<WideT>(word);
			state_cast(state).unset_surrogate();
			continue;
		}
		std::ptrdiff_t extra;
		unsigned long code;
		if (octet <= 0x7F) {
			extra = 0;
			code = octet;
		} else if (octet <= (0x80 | 0x3F)) {
			// leading continuation octet
			return (std::codecvt_base::error);
		} else if ((max_code >= 0x80) && (octet <= (0xC0 | 0x1F))) {
			extra = 1;
			if ((from_end - from) < extra) {
				break;
			}
			code = static_cast<unsigned char>(octet & 0x1F);
			if (code < 0x02) {
				// overlong encoding
				return (std::codecvt_base::error);
			}
		} else if ((max_code >= 0x0800) && (octet <= (0xE0 | 0x0F))) {
			extra = 2;
			if ((from_end - from) < extra) {
				break;
			}
			code = static_cast<unsigned char>(octet & 0x0F);
			if (0 == code) {
				if (static_cast<unsigned char>(*from) < (0x80 | 0x20)) {
					// overlong encoding (or no continuation octect)
					return (std::codecvt_base::error);
				}
			}
		} else if ((max_code >= 0x00010000UL) && (octet <= (0xF0 | 0x07))) {
			extra = 3;
			if ((from_end - from) < (extra - surrogates)) {
				break;
			}
			code = static_cast<unsigned char>(octet & 0x07);
			if (0 == code) {
				if (static_cast<unsigned char>(*from) < (0x80 | 0x10)) {
					// overlong encoding (or no continuation octect)
					return (std::codecvt_base::error);
				}
			}
		} else {
			return (std::codecvt_base::error);
		}
		if ((max_code >= 0x80) && extra) {
			bool const skip = surrogates && (extra >= 3);
			if (skip) {
				--extra;
			}
			for (; extra; --extra) {
				if ((CHAR_BIT * sizeof(ByteT)) > 8) {
					if ((*from < 0) || (0xFF < *from)) {
						return (std::codecvt_base::error);
					}
				}
				octet = static_cast<unsigned char>(*from++);
				if ((octet < 0x80) || ((0x80 | 0x3F) < octet)) {
					return (std::codecvt_base::error);
				}
				code = (code << 6) | (octet & 0x3F);
			}
			if (skip) {
				code <<= 6;
			} else if ((0xD800 <= code) && (code <= (0xDC00 | 0x03FF))) {
				// CESU-8 (UTF-8 encoded UTF-16 surrogates)
				return (std::codecvt_base::error);
			}
		}
		if (max_code < code) {
			return (std::codecvt_base::error);
		}
		if (surrogates && (code >= 0x00010000UL)) {
			code -= 0x00010000UL;
			// output high (leading) surrogate
			unsigned short bits = static_cast<unsigned short>(code >> 10);
			unsigned short word = static_cast<unsigned short>(0xD800U | bits);
			*to_next = static_cast<WideT>(word);
			// save low (trailing) surrogate bits (4 MSB of 10)
			state_cast(state).set_surrogate(static_cast<unsigned short>(code & 0x03FF));
			if ((max_code & 0x3F) != 0x3F) {
				// save high (leading) surrogate bits for code point limitations
				state_cast(state).set_high_surrogate(bits);
			}
			continue;
		}
		if (Header) {
			if (state_cast(state).do_header()) {
				if (0xFEFF == code) {
					std::codecvt_base::result result = utf8_from_bytes<WideT, ByteT, UTF16, Maxcode, Header>(
						state, from, from_end, from_next, to, to_end, to_next
					);
					if (std::codecvt_base::partial == result) {
						// revert header detection only for empty output
						if (to_next == to) {
							state_cast(state).undo_header();
							from_next = from - 3;
						}
					}
					return (result);
				}
			}
		}
		*to_next = static_cast<WideT>(code);
	}
	// unexpected end of source buffer or not enough space in the output buffer
	if ((from_next != from_end) || (to_next == to)) {
		return (std::codecvt_base::partial);
	}
	// only for N:M conversions if last continuation octet is missing
	if (surrogates && state_cast(state).has_surrogate() && ((to_end - to) > 1)) {
		return (std::codecvt_base::partial);
	}
	return (std::codecvt_base::ok);
}

///
/// @brief Convert UTF-16 or UTF-32 codes to UTF-8 octets.
///
///  Translates the UTF-16/UTF-32 codes from the source range [from, from_end)
///  to UTF-8 octets, placing the results in the subsequent locations
///  starting at to. Converts no more than from_end - from UTF-16/UTF-32 codes
///  and writes no more than to_end - to UTF-8 octets. Leaves from_next
///  and to_next pointing one beyond the last element successfully converted.
///
///  Requires from <= from_end, to <= to_end, and state is the initial state
///  or is obtained by converting the preceding UTF-16/UTF-32 codes.
///
///  The input character type is expected to have at least 16 bits for UTF-16
///  input and at least 21 bits for UTF-32 with code points beyond the BMP.
///  A Maxcode of 0xFFFF limits the input to UCS-2 (Basic Multilingual Plane).
///
///  If UTF-16 input beyond the BMP is enabled (surrogate pairs) the lead octet
///  and continuation octets of four-octet UTF-8 codes are separately written to
///  support 1:N conversions (required for std::basic_filebuf). Note that
///  1:N conversions might fail (if pending low (trailing) surrogate is invalid)
///  after the lead octet of a four-octet UTF-8 code has already been written.
///
/// @tparam WideT   Input character type with at least 16 or 21 bits.
/// @tparam ByteT   Output character type with at least 8 bits.
/// @tparam UTF16   Support surrogate pairs for code points beyond the BMP.
/// @tparam Maxcode Largest code point that will be written without error.
/// @tparam Header  Generate UTF-8 BOM at the start of the output sequence.
/// @param[in]  from      Begin or continuation of UTF-16/UTF-32 input sequence.
/// @param[in]  from_end  End of the UTF-16/UTF-32 input sequence (>= from).
/// @param[out] from_next End of successfully converted input sequence.
/// @param[in]  to        Begin of UTF-8 output buffer.
/// @param[in]  to_end    End of the UTF-8 output buffer (>= to).
/// @param[out] to_next   End of successfully converted output sequence.
/// @retval std::codecvt_base::ok      Conversion completed.
/// @retval std::codecvt_base::error   Encountered an invalid character.
/// @retval std::codecvt_base::partial Not enough space in the output buffer
///                                    or unexpected end of source buffer.
///
template<typename WideT, typename ByteT, bool UTF16, unsigned long Maxcode, bool Header>
std::codecvt_base::result
utf8_to_bytes(std::mbstate_t& state,  // codecvt_utf_state
	WideT const* from, WideT const* from_end, WideT const*& from_next,
	ByteT* to, ByteT* to_end, ByteT*& to_next)
{
	// unused features are expected to be optimized away by the compiler
	// due to constant expressions that depend on template parameters or
	// the size of the internal (WideT) and external (ByteT) characters
	typedef codecvt_utf_state<std::mbstate_t> state_cast;
	enum config {
		surrogates = bool(
			UTF16 &&
			(Maxcode >= 0x00010000UL)
		),
		beyond_bmp = bool(
			surrogates ||
			(
				!UTF16 &&
				(Maxcode >= 0x00010000UL) &&
				((sizeof(WideT) * CHAR_BIT) >= 21)
			)
		),
		max_code = static_cast<unsigned long>(
			(beyond_bmp && (Maxcode >= 0x0010FFFFUL)) ? 0x0010FFFFUL : (
			(!beyond_bmp && (Maxcode >= 0xFFFFUL)) ? 0xFFFFUL : (
			Maxcode)))
	};
	for (from_next = from, to_next = to; (from_next != from_end) && (to != to_end); ++from_next, to_next = to) {
		unsigned long code = static_cast<unsigned long>(*from_next);
		// Surrogate pairs are split into lead octet and continuation octets
		// to fake 1:N conversion that is required for std::basic_filebuf.
		bool saved = false;
		if (surrogates) {
			if (state_cast(state).has_surrogate()) {
				// low (trailing) surrogate required
				if ((code < 0xDC00) || ((0xDC00 | 0x03FF) < code)) {
					return (std::codecvt_base::error);
				}
				unsigned long bits = state_cast(state).get_surrogate();
				code = ((bits << 10) | (code - 0xDC00UL)) + 0x00010000UL;
			} else if ((0xD800 <= code) && (code <= (0xD800 | 0x03FF))) {
				// high (leading) surrogate
				unsigned short bits = static_cast<unsigned short>(code - 0xD800U);
				saved = (state_cast(state).set_surrogate(bits), true);
				code = (static_cast<unsigned long>(bits) << 10) + 0x00010000UL;
			} else if ((0xDC00 <= code) && (code <= (0xDC00 | 0x03FF))) {
				// unpaired low (trailing) surrogate
				return (std::codecvt_base::error);
			}
		} else if ((0xD800 <= code) && (code <= (0xDC00 | 0x03FF))) {
			return (std::codecvt_base::error);
		}
		if (max_code < code) {
			if (surrogates) {
				if (saved) {
					state_cast(state).unset_surrogate();
				} else if (state_cast(state).has_surrogate()) {
					// revert high surrogate and lead octet for M:N conversions
					if (from_next != from) {
						--from_next;
						--to_next;
						state_cast(state).unset_surrogate();
					}
				}
			}
			return (std::codecvt_base::error);
		}
		unsigned char first;
		std::ptrdiff_t extra;
		if (code <= 0x7F) {
			first = static_cast<unsigned char>(code);
			extra = 0;
		} else if ((max_code >= 0x80) && (code <= 0x07FF)) {
			first = static_cast<unsigned char>(0xC0 | static_cast<unsigned char>(code >> 6));
			extra = 1;
		} else if ((max_code >= 0x0800) && (code <= 0xFFFF)) {
			first = static_cast<unsigned char>(0xE0 | static_cast<unsigned char>(code >> 12));
			extra = 2;
		} else if (max_code >= 0x00010000UL) {
			first = static_cast<unsigned char>(0xF0 | static_cast<unsigned char>(code >> 18));
			extra = (surrogates && saved) ? 0 : 3;
		} else {
			return (std::codecvt_base::error);
		}
		bool const lead = !surrogates || (extra < 3);
		if ((max_code >= 0x80) && ((to_end - to) < (lead + extra))) {
			if (surrogates && saved) {
				state_cast(state).unset_surrogate();
			}
			break;
		}
		if (Header) {
			if (state_cast(state).do_header()) {
				if ((to_end - to) < (3 + lead + extra)) {
					state_cast(state).undo_header();
					if (surrogates && saved) {
						state_cast(state).unset_surrogate();
					}
					break;
				}
				*to++ = static_cast<ByteT>(static_cast<unsigned char>(0xEF));
				*to++ = static_cast<ByteT>(static_cast<unsigned char>(0xBB));
				*to++ = static_cast<ByteT>(static_cast<unsigned char>(0xBF));
			}
		}
		if (!surrogates || lead) {
			*to++ = static_cast<ByteT>(first);
		}
		if (max_code >= 0x80) {
			while (extra) {
				*to++ = static_cast<ByteT>(static_cast<unsigned char>(((code >> (6 * --extra)) & 0x3F) | 0x80));
			}
		}
		if (surrogates && !saved) {
			state_cast(state).unset_surrogate();
		}
	}
	// unexpected end of source buffer or not enough space in the output buffer
	if ((from == from_next) || (from_next != from_end)) {
		return (std::codecvt_base::partial);
	}
	// only for M:N conversions if low (trailing) surrogate is missing
	if (surrogates && state_cast(state).has_surrogate() && ((from_end - from) > 1)) {
		return (std::codecvt_base::partial);
	}
	return (std::codecvt_base::ok);
}

///
/// @brief Convert UTF-16 octets to UCS-2 or UCS-4 codes.
///
///  Translates the UTF-16 octets from the source range [from, from_end) to
///  UCS-2/UCS-4 codes, placing the results in the subsequent locations
///  starting at to. Converts no more than from_end - from UTF-16 octets and
///  writes no more than to_end - to UCS-2/UCS-4 codes. Leaves from_next
///  and to_next pointing one beyond the last element successfully converted.
///
///  Requires from <= from_end, to <= to_end, and state is the initial state
///  or is obtained by converting the preceding UTF-16 octets in the sequence.
///
///  The output character type is expected to have at least 16 bits for UCS-2
///  output and at least 21 bits for UCS-4 with code points beyond the BMP.
///  A Maxcode of 0xFFFF limits the output to UCS-2 (Basic Multilingual Plane).
///
/// @tparam WideT   Output character type with at least 16 or 21 bits.
/// @tparam ByteT   Input character type with at least 8 bits.
/// @tparam Little  Default to little-endian input if BOM is not consumed.
/// @tparam Maxcode Largest code point that will be read without error.
/// @tparam Header  Consume BOM, if present at the start of input sequence.
/// @param[in]  from      Begin or continuation of UTF-16 input sequence.
/// @param[in]  from_end  End of the UTF-16 input sequence (>= from).
/// @param[out] from_next End of successfully converted input sequence.
/// @param[in]  to        Begin of UCS-2/UCS-4 output buffer.
/// @param[in]  to_end    End of the UCS-2/UCS-4 output buffer (>= to).
/// @param[out] to_next   End of successfully converted output sequence.
/// @retval std::codecvt_base::ok      Conversion completed.
/// @retval std::codecvt_base::error   Encountered an invalid character.
/// @retval std::codecvt_base::partial Not enough space in the output buffer
///                                    or unexpected end of source buffer.
///
template<typename WideT, typename ByteT, bool Little, unsigned long Maxcode, bool Header>
std::codecvt_base::result
utf16_from_bytes(std::mbstate_t& state,  // codecvt_utf_state
	ByteT const* from, ByteT const* from_end, ByteT const*& from_next,
	WideT* to, WideT* to_end, WideT*& to_next)
{
	// unused features are expected to be optimized away by the compiler
	// due to constant expressions that depend on template parameters or
	// the size of the external (ByteT) and internal (WideT) characters
	typedef codecvt_utf_state<std::mbstate_t> state_cast;
	enum config {
		beyond_bmp = bool((sizeof(WideT) * CHAR_BIT) >= 21),
		surrogates = bool(
			beyond_bmp &&
			(Maxcode >= 0x00010000UL)
		),
		max_code = static_cast<unsigned long>(
			(surrogates && (Maxcode >= 0x0010FFFFUL)) ? 0x0010FFFFUL :
			(!surrogates && (Maxcode >= 0xFFFFUL)) ? 0xFFFFUL :
			Maxcode)
	};
	for (from_next = from, to_next = to; ((from_end - from) >= 2) && (to_next != to_end); from_next = from, ++to_next) {
		if ((CHAR_BIT * sizeof(ByteT)) > 8) {
			if ((*from < 0) || (0xFF < *from)) {
				return (std::codecvt_base::error);
			}
		}
		unsigned char octet1 = static_cast<unsigned char>(*from++);
		if ((CHAR_BIT * sizeof(ByteT)) > 8) {
			if ((*from < 0) || (0xFF < *from)) {
				return (std::codecvt_base::error);
			}
		}
		unsigned char octet2 = static_cast<unsigned char>(*from++);
		if (CHAR_BIT > 8) {
			if ((octet1 > 0xFF) || (octet2 > 0xFF)) {
				return (std::codecvt_base::error);
			}
		}
		if (Header && state_cast(state).do_header()) {
			if (((octet1 == 0xFE) && (octet2 == 0xFF)) ||
			    ((octet1 == 0xFF) && (octet2 == 0xFE))) {
				if (octet1 == 0xFF) {
					state_cast(state).set_little();
				}
				std::codecvt_base::result result = utf16_from_bytes<WideT, ByteT, Little, Maxcode, Header>(
					state, from, from_end, from_next, to, to_end, to_next
				);
				if (std::codecvt_base::partial == result) {
					// revert header detection only for empty output
					if (to_next == to) {
						state_cast(state).undo_header();
						from_next = from - 2;
					}
				}
				return (result);
			} else if (Little) {
				state_cast(state).set_little();
			}
		}
		unsigned long code = (Header ? state_cast(state).is_little() : Little)
			? static_cast<unsigned long>((static_cast<unsigned short>(octet2) << 8) | octet1)
			: static_cast<unsigned long>((static_cast<unsigned short>(octet1) << 8) | octet2);
		if (surrogates) {
			if ((0xDC00 <= code) && (code <= (0xDC00 | 0x03FF))) {
				// unpaired low (trailing) surrogate
				return (std::codecvt_base::error);
			}
			if ((0xD800 <= code) && (code <= (0xD800 | 0x03FF))) {
				// high (leading) surrogate
				if ((from_end - from) < 2) {
					break;
				}
				if ((CHAR_BIT * sizeof(ByteT)) > 8) {
					if ((*from < 0) || (0xFF < *from)) {
						return (std::codecvt_base::error);
					}
				}
				octet1 = static_cast<unsigned char>(*from++);
				if ((CHAR_BIT * sizeof(ByteT)) > 8) {
					if ((*from < 0) || (0xFF < *from)) {
						return (std::codecvt_base::error);
					}
				}
				octet2 = static_cast<unsigned char>(*from++);
				if (CHAR_BIT > 8) {
					if ((octet1 > 0xFF) || (octet2 > 0xFF)) {
						return (std::codecvt_base::error);
					}
				}
				unsigned long bits = code - 0xD800UL;
				code = (Header ? state_cast(state).is_little() : Little)
					? static_cast<unsigned long>((static_cast<unsigned short>(octet2) << 8) | octet1)
					: static_cast<unsigned long>((static_cast<unsigned short>(octet1) << 8) | octet2);
				// low (trailing) surrogate required
				if ((code < 0xDC00) || ((0xDC00 | 0x03FF) < code)) {
					return (std::codecvt_base::error);
				}
				code = ((bits << 10) | (code - 0xDC00UL)) + 0x00010000UL;
			}
		} else if ((0xD800 <= code) && (code <= (0xDC00 | 0x03FF))) {
			return (std::codecvt_base::error);
		}
		if (max_code < code) {
			return (std::codecvt_base::error);
		}
		*to_next = static_cast<WideT>(code);
	}
	// unexpected end of source buffer or not enough space in the output buffer
	if ((from_next != from_end) || (to_next == to)) {
		return (std::codecvt_base::partial);
	}
	return (std::codecvt_base::ok);
}

///
/// @brief Convert UCS-2 or UCS-4 codes to UTF-16 octets.
///
///  Translates the UCS-2/UCS-4 codes from the source range [from, from_end)
///  to UTF-16 octets, placing the results in the subsequent locations
///  starting at to. Converts no more than from_end - from UCS-2/UCS-4 codes
///  and writes no more than to_end - to UTF-16 octets. Leaves from_next
///  and to_next pointing one beyond the last element successfully converted.
///
///  Requires from <= from_end, to <= to_end, and state is the initial state
///  or is obtained by converting the preceding UCS-2/UCS-4 codes.
///
///  The input character type is expected to have at least 16 bits for UCS-2
///  input and at least 21 bits for UCS-4 with code points beyond the BMP.
///  A Maxcode of 0xFFFF limits the input to UCS-2 (Basic Multilingual Plane).
///
/// @tparam WideT   Input character type with at least 16 or 21 bits.
/// @tparam ByteT   Output character type with at least 8 bits.
/// @tparam Little  Output UTF-16 header and codes in little-endian order.
/// @tparam Maxcode Largest code point that will be written without error.
/// @tparam Header  Generate UTF-16 BOM at the start of the output sequence.
/// @param[in]  from      Begin or continuation of UCS-2/UCS-4 input sequence.
/// @param[in]  from_end  End of the UCS-2/UCS-4 input sequence (>= from).
/// @param[out] from_next End of successfully converted input sequence.
/// @param[in]  to        Begin of UTF-16 output buffer.
/// @param[in]  to_end    End of the UTF-16 output buffer (>= to).
/// @param[out] to_next   End of successfully converted output sequence.
/// @retval std::codecvt_base::ok      Conversion completed.
/// @retval std::codecvt_base::error   Encountered an invalid character.
/// @retval std::codecvt_base::partial Not enough space in the output buffer
///                                    or unexpected end of source buffer.
///
template<typename WideT, typename ByteT, bool Little, unsigned long Maxcode, bool Header>
std::codecvt_base::result
utf16_to_bytes(std::mbstate_t& state,  // codecvt_utf_state
	WideT const* from, WideT const* from_end, WideT const*& from_next,
	ByteT* to, ByteT* to_end, ByteT*& to_next)
{
	// unused features are expected to be optimized away by the compiler
	// due to constant expressions that depend on template parameters or
	// the size of the internal (WideT) and external (ByteT) characters
	typedef codecvt_utf_state<std::mbstate_t> state_cast;
	enum config {
		beyond_bmp = bool((sizeof(WideT) * CHAR_BIT) >= 21),
		surrogates = bool(
			beyond_bmp &&
			(Maxcode >= 0x00010000UL)
		),
		max_code = static_cast<unsigned long>(
			(surrogates && (Maxcode >= 0x0010FFFFUL)) ? 0x0010FFFFUL :
			(!surrogates && (Maxcode >= 0xFFFFUL)) ? 0xFFFFUL :
			Maxcode)
	};
	for (from_next = from, to_next = to; (from_next != from_end) && ((to_end - to) >= 2); ++from_next, to_next = to) {
		unsigned long code = static_cast<unsigned long>(*from_next);
		if (max_code < code) {
			return (std::codecvt_base::error);
		}
		if ((0xD800 <= code) && (code <= (0xDC00 | 0x03FF))) {
			return (std::codecvt_base::error);
		}
		if (Header && state_cast(state).do_header()) {
			std::ptrdiff_t codes = 1 + (surrogates && (code >= 0x00010000UL));
			if ((to_end - to) < ((1 + codes) * 2)) {
				state_cast(state).undo_header();
				break;
			}
			if (Little) {
				*to++ = static_cast<ByteT>(static_cast<unsigned char>(0xFF));
				*to++ = static_cast<ByteT>(static_cast<unsigned char>(0xFE));
			} else {
				*to++ = static_cast<ByteT>(static_cast<unsigned char>(0xFE));
				*to++ = static_cast<ByteT>(static_cast<unsigned char>(0xFF));
			}
		}
		if (surrogates && (code >= 0x00010000UL)) {
			if ((to_end - to) < (2 * 2)) {
				break;
			}
			code -= 0x00010000UL;
			unsigned short bits = static_cast<unsigned short>(0xD800U | (code >> 10));
			if (Little) {
				*to++ = static_cast<ByteT>(static_cast<unsigned char>(bits & 0xFF));
				*to++ = static_cast<ByteT>(static_cast<unsigned char>((bits >> 8) & 0xFF));
			} else {
				*to++ = static_cast<ByteT>(static_cast<unsigned char>((bits >> 8) & 0xFF));
				*to++ = static_cast<ByteT>(static_cast<unsigned char>(bits & 0xFF));
			}
			code = static_cast<unsigned short>(0xDC00U | (code & 0x3FF));
		}
		if (Little) {
			*to++ = static_cast<ByteT>(static_cast<unsigned char>(code & 0xFF));
			*to++ = static_cast<ByteT>(static_cast<unsigned char>((code >> 8) & 0xFF));
		} else {
			*to++ = static_cast<ByteT>(static_cast<unsigned char>((code >> 8) & 0xFF));
			*to++ = static_cast<ByteT>(static_cast<unsigned char>(code & 0xFF));
		}
	}
	// unexpected end of source buffer or not enough space in the output buffer
	if ((from == from_next) || (from_next != from_end)) {
		return (std::codecvt_base::partial);
	}
	return (std::codecvt_base::ok);
}

///
/// @brief Convert Windows-1252 octets to UCS-2 codes.
///
///  Translates the Windows-1252 octets from the source range [from, from_end)
///  to UCS-2 codes, placing the results in the subsequent locations starting
///  at to. Converts no more than from_end - from Windows-1252 octets and
///  writes no more than to_end - to UCS-2 codes. Leaves from_next and to_next
///  pointing one beyond the last element successfully converted.
///
///  Requires from <= from_end, to <= to_end.
///  The output character type is expected to have at least 16 bits.
///
///  For undefined characters (0x81, 0x8D, 0x8F, 0x90, and 0x9D) the
///  Windows API function MultiByteToWideChar is emulated by mapping
///  these to the corresponding C1 control codes. In the end this means
///  that the conversion will never fail due to invalid characters.
///
/// @tparam WideT Output character type with at least 16 bits.
/// @tparam ByteT Input character type with at least 8 bits.
/// @param[in]  from      Begin of Windows-1252 input sequence.
/// @param[in]  from_end  End of Windows-1252 input sequence (>= from).
/// @param[out] from_next End of successfully converted input sequence.
/// @param[in]  to        Begin of UCS-2 output buffer.
/// @param[in]  to_end    End of UCS-2 output buffer (>= to).
/// @param[out] to_next   End of successfully converted output sequence.
/// @retval std::codecvt_base::ok      Conversion completed.
/// @retval std::codecvt_base::error   Encountered an invalid character.
/// @retval std::codecvt_base::partial Not enough space in the output buffer
///                                    or unexpected end of source buffer.
///
template<typename WideT, typename ByteT>
std::codecvt_base::result
cp1252_from_bytes(
	ByteT const* from, ByteT const* from_end, ByteT const*& from_next,
	WideT* to, WideT* to_end, WideT*& to_next)
{
	from_next = from;
	to_next = to;
	if (from_end == from) {
		return (std::codecvt_base::partial);
	}
	for (; (from != from_end) && (to != to_end); from_next = ++from, to_next = ++to) {
		unsigned char octet = static_cast<unsigned char>(*from);
		if (CHAR_BIT > 8) {
			if (octet > 0xFF) {
				return (std::codecvt_base::error);
			}
		}
		if (sizeof(ByteT) > sizeof(unsigned char)) {
			if (static_cast<ByteT>(octet) != *from) {
				return (std::codecvt_base::error);
			}
		}
		switch (octet) {
		case 0x80:  // EURO SIGN
			*to = static_cast<WideT>(0x20AC);
			break;
		case 0x82:  // SINGLE LOW-9 QUOTATION MARK
			*to = static_cast<WideT>(0x201A);
			break;
		case 0x83:  // LATIN SMALL LETTER F WITH HOOK
			*to = static_cast<WideT>(0x0192);
			break;
		case 0x84:  // DOUBLE LOW-9 QUOTATION MARK
			*to = static_cast<WideT>(0x201E);
			break;
		case 0x85:  // HORIZONTAL ELLIPSIS
			*to = static_cast<WideT>(0x2026);
			break;
		case 0x86:  // DAGGER
			*to = static_cast<WideT>(0x2020);
			break;
		case 0x87:  // DOUBLE DAGGER
			*to = static_cast<WideT>(0x2021);
			break;
		case 0x88:  // MODIFIER LETTER CIRCUMFLEX ACCENT
			*to = static_cast<WideT>(0x02C6);
			break;
		case 0x89:  // PER MILLE SIGN
			*to = static_cast<WideT>(0x2030);
			break;
		case 0x8A:  // LATIN CAPITAL LETTER S WITH CARON
			*to = static_cast<WideT>(0x0160);
			break;
		case 0x8B:  // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
			*to = static_cast<WideT>(0x2039);
			break;
		case 0x8C:  // LATIN CAPITAL LIGATURE OE
			*to = static_cast<WideT>(0x0152);
			break;
		case 0x8E:  // LATIN CAPITAL LETTER Z WITH CARON
			*to = static_cast<WideT>(0x017D);
			break;
		case 0x91:  // LEFT SINGLE QUOTATION MARK
			*to = static_cast<WideT>(0x2018);
			break;
		case 0x92:  // RIGHT SINGLE QUOTATION MARK
			*to = static_cast<WideT>(0x2019);
			break;
		case 0x93:  // LEFT DOUBLE QUOTATION MARK
			*to = static_cast<WideT>(0x201C);
			break;
		case 0x94:  // RIGHT DOUBLE QUOTATION MARK
			*to = static_cast<WideT>(0x201D);
			break;
		case 0x95:  // BULLET
			*to = static_cast<WideT>(0x2022);
			break;
		case 0x96:  // EN DASH
			*to = static_cast<WideT>(0x2013);
			break;
		case 0x97:  // EM DASH
			*to = static_cast<WideT>(0x2014);
			break;
		case 0x98:  // SMALL TILDE
			*to = static_cast<WideT>(0x02DC);
			break;
		case 0x99:  // TRADE MARK SIGN
			*to = static_cast<WideT>(0x2122);
			break;
		case 0x9A:  // LATIN SMALL LETTER S WITH CARON
			*to = static_cast<WideT>(0x0161);
			break;
		case 0x9B:  // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
			*to = static_cast<WideT>(0x203A);
			break;
		case 0x9C:  // LATIN SMALL LIGATURE OE
			*to = static_cast<WideT>(0x0153);
			break;
		case 0x9E:  // LATIN SMALL LETTER Z WITH CARON
			*to = static_cast<WideT>(0x017E);
			break;
		case 0x9F:  // LATIN CAPITAL LETTER Y WITH DIAERESIS
			*to = static_cast<WideT>(0x0178);
			break;
		default:
			// U+0000 - U+007F Basic Latin
			// U+0081 <control> (HIGH OCTET PRESET)
			// U+008D <control> (REVERSE LINE FEED)
			// U+008F <control> (SINGLE SHIFT THREE)
			// U+0090 <control> (DEVICE CONTROL STRING)
			// U+009D <control> (OPERATING SYSTEM COMMAND)
			// U+00A0 - U+00FF Latin-1 Supplement
			*to = static_cast<WideT>(octet);
			break;
		}
	}
	if (from_next != from_end) {
		return (std::codecvt_base::partial);
	}
	return (std::codecvt_base::ok);
}

///
/// @brief Convert UCS-2 codes to Windows-1252 octets.
///
///  Translates the UCS-2 codes from the source range [from, from_end) to
///  Windows-1252 octets, placing the results in the subsequent locations
///  starting at to. Converts no more than from_end - from UCS-2 codes and
///  writes no more than to_end - to Windows-1252 octets. Leaves from_next
///  and to_next pointing one beyond the last element successfully converted.
///
///  Requires from <= from_end, to <= to_end.
///  The input character type is expected to have at least 16 bits.
///
///  Input is expected to be normalized with Normalization Form C (NFC).
///
///  For undefined characters (0x81, 0x8D, 0x8F, 0x90, and 0x9D) the
///  Windows API function WideCharToMultiByte is emulated by mapping
///  these from the corresponding C1 control codes.
///
/// @tparam WideT Input character type with at least 16 bits.
/// @tparam ByteT Output character type with at least 8 bits.
/// @param[in]  from      Begin of UCS-2 input sequence.
/// @param[in]  from_end  End of UCS-2 input sequence (>= from).
/// @param[out] from_next End of successfully converted input sequence.
/// @param[in]  to        Begin of Windows-1252 output buffer.
/// @param[in]  to_end    End of Windows-1252 output buffer (>= to).
/// @param[out] to_next   End of successfully converted output sequence.
/// @retval std::codecvt_base::ok      Conversion completed.
/// @retval std::codecvt_base::error   Encountered an invalid character.
/// @retval std::codecvt_base::partial Not enough space in the output buffer
///                                    or unexpected end of source buffer.
///
template<typename WideT, typename ByteT>
std::codecvt_base::result
cp1252_to_bytes(
	WideT const* from, WideT const* from_end, WideT const*& from_next,
	ByteT* to, ByteT* to_end, ByteT*& to_next)
{
	from_next = from;
	to_next = to;
	if (from_end == from) {
		return (std::codecvt_base::partial);
	}
	for (; (from != from_end) && (to != to_end); from_next = ++from, to_next = ++to) {
		unsigned short code = static_cast<unsigned short>(*from);
		if (sizeof(WideT) > sizeof(unsigned short)) {
			if (static_cast<WideT>(code) != *from) {
				return (std::codecvt_base::error);
			}
		}
		if (code <= 0xFF) {
			switch (code) {
			case 0x80:  // EURO SIGN (U+20AC)
			case 0x82:  // SINGLE LOW-9 QUOTATION MARK (U+201A)
			case 0x83:  // LATIN SMALL LETTER F WITH HOOK (U+0192)
			case 0x84:  // DOUBLE LOW-9 QUOTATION MARK (U+201E)
			case 0x85:  // HORIZONTAL ELLIPSIS (U+2026)
			case 0x86:  // DAGGER (U+2020)
			case 0x87:  // DOUBLE DAGGER (U+2021)
			case 0x88:  // MODIFIER LETTER CIRCUMFLEX ACCENT (U+02C6)
			case 0x89:  // PER MILLE SIGN (U+2030)
			case 0x8A:  // LATIN CAPITAL LETTER S WITH CARON (U+0160)
			case 0x8B:  // SINGLE LEFT-POINTING ANGLE QUOTATION MARK (U+2039)
			case 0x8C:  // LATIN CAPITAL LIGATURE OE (U+0152)
			case 0x8E:  // LATIN CAPITAL LETTER Z WITH CARON (U+017D)
			case 0x91:  // LEFT SINGLE QUOTATION MARK (U+2018)
			case 0x92:  // RIGHT SINGLE QUOTATION MARK (U+2019)
			case 0x93:  // LEFT DOUBLE QUOTATION MARK (U+201C)
			case 0x94:  // RIGHT DOUBLE QUOTATION MARK (U+201D)
			case 0x95:  // BULLET (U+2022)
			case 0x96:  // EN DASH (U+2013)
			case 0x97:  // EM DASH (U+2014)
			case 0x98:  // SMALL TILDE (U+02DC)
			case 0x99:  // TRADE MARK SIGN (U+2122)
			case 0x9A:  // LATIN SMALL LETTER S WITH CARON (U+0161)
			case 0x9B:  // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK (U+203A)
			case 0x9C:  // LATIN SMALL LIGATURE OE (U+0153)
			case 0x9E:  // LATIN SMALL LETTER Z WITH CARON (U+017E)
			case 0x9F:  // LATIN CAPITAL LETTER Y WITH DIAERESIS (U+0178)
				return (std::codecvt_base::error);
			default:
				// U+0000 - U+007F Basic Latin
				// U+0081 <control> (HIGH OCTET PRESET)
				// U+008D <control> (REVERSE LINE FEED)
				// U+008F <control> (SINGLE SHIFT THREE)
				// U+0090 <control> (DEVICE CONTROL STRING)
				// U+009D <control> (OPERATING SYSTEM COMMAND)
				// U+00A0 - U+00FF Latin-1 Supplement
				*to = static_cast<ByteT>(code);
				break;
			}
		} else {
			switch (code) {
			case 0x20AC:  // EURO SIGN
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x80));
				break;
			case 0x201A:  // SINGLE LOW-9 QUOTATION MARK
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x82));
				break;
			case 0x0192:  // LATIN SMALL LETTER F WITH HOOK
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x83));
				break;
			case 0x201E:  // DOUBLE LOW-9 QUOTATION MARK
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x84));
				break;
			case 0x2026:  // HORIZONTAL ELLIPSIS
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x85));
				break;
			case 0x2020:  // DAGGER
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x86));
				break;
			case 0x2021:  // DOUBLE DAGGER
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x87));
				break;
			case 0x02C6:  // MODIFIER LETTER CIRCUMFLEX ACCENT
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x88));
				break;
			case 0x2030:  // PER MILLE SIGN
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x89));
				break;
			case 0x0160:  // LATIN CAPITAL LETTER S WITH CARON
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x8A));
				break;
			case 0x2039:  // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x8B));
				break;
			case 0x0152:  // LATIN CAPITAL LIGATURE OE
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x8C));
				break;
			case 0x017D:  // LATIN CAPITAL LETTER Z WITH CARON
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x8E));
				break;
			case 0x2018:  // LEFT SINGLE QUOTATION MARK
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x91));
				break;
			case 0x2019:  // RIGHT SINGLE QUOTATION MARK
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x92));
				break;
			case 0x201C:  // LEFT DOUBLE QUOTATION MARK
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x93));
				break;
			case 0x201D:  // RIGHT DOUBLE QUOTATION MARK
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x94));
				break;
			case 0x2022:  // BULLET
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x95));
				break;
			case 0x2013:  // EN DASH
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x96));
				break;
			case 0x2014:  // EM DASH
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x97));
				break;
			case 0x02DC:  // SMALL TILDE
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x98));
				break;
			case 0x2122:  // TRADE MARK SIGN
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x99));
				break;
			case 0x0161:  // LATIN SMALL LETTER S WITH CARON
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x9A));
				break;
			case 0x203A:  // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x9B));
				break;
			case 0x0153:  // LATIN SMALL LIGATURE OE
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x9C));
				break;
			case 0x017E:  // LATIN SMALL LETTER Z WITH CARON
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x9E));
				break;
			case 0x0178:  // LATIN CAPITAL LETTER Y WITH DIAERESIS
				*to = static_cast<ByteT>(static_cast<unsigned char>(0x9F));
				break;
			default:
				return (std::codecvt_base::error);
			}
		}
	}
	if (from_next != from_end) {
		return (std::codecvt_base::partial);
	}
	return (std::codecvt_base::ok);
}

} // namespace genome::detail
} // namespace genome

#endif // GENOME_LOCALE_DETAIL_IPP
