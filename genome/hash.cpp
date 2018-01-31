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
#include <genome/hash.hpp>

namespace genome {

namespace /*{anonymous}*/ {

	GENOME_CONSTEXPR_INLINE
	u32
	hash_char_cast(byte_char c)
	{
		// sign-extended to 32-bit because MSVC char is signed by default
		// and the value is not explicitly casted in the hash calculation
		return (static_cast<u32>(static_cast<i32>(static_cast<i8>(c))));
	}

} // namespace genome::{anonymous}

//
// ASCII (Windows-1252) case-sensitive hash
//

string_hash
hash_byte_update(byte_char const* str, string_hash hash)
{
	u32 h = u32(hash);
	if (str) {
		for (byte_char c = *str; c; c = *++str) {
			h += (h << 5) + hash_char_cast(c);
		}
	}
	return (string_hash(h));
}

//
// ASCII (Windows-1252) identifier hash (lower-case)
//

string_hash
hash_name_update(byte_char const* name, string_hash hash)
{
	u32 h = hash;
	if (name) {
		for (byte_char c = *name; c; c = *++name) {
			h += (h << 5) + hash_char_cast(name_traits::canonicalize(c));
		}
	}
	return (string_hash(h));
}

//
// ASCII (Windows-1252) filename hash (lower-case, without tags)
//

string_hash
hash_filename(byte_char const* filename)
{
	if (!filename) {
		return (string_hash(u32(0)));
	}
	u32 h = u32(hash_name_init());
	for (byte_char c = *filename; c; c = *++filename) {
		// skip tags starting with '%' and continue with the next '.'
		if (byte_code::percent_sign == c) {
			for (c = *++filename; c != byte_code::full_stop; c = *++filename) {
				if (!c) {
					return (string_hash(h));
				}
			}
		}
		h += (h << 5) + hash_char_cast(name_traits::canonicalize(c));
	}
	return (string_hash(h));
}

string_hash
hash_filename(byte_string const& filename)
{
	if (filename.empty()) {
		return (string_hash(u32(0)));
	}
	return (hash_filename(filename.c_str()));
}

//
// ASCII (Windows-1252) hash string conversion
//

byte_string
hash_to_string(string_hash h)
{
	byte_string s(8, byte_code::digit_first);
	if (h) {
		for (byte_string::reverse_iterator i = s.rbegin(); i != s.rend(); ++i) {
			byte_char c = static_cast<byte_char>(h & 0x0F);
			if (c <= (byte_code::digit_last - byte_code::digit_first)) {
				c = static_cast<byte_char>(c + byte_code::digit_first);
			} else {
				c = static_cast<byte_char>(c - (byte_code::digit_last - byte_code::digit_first + 1) + byte_code::hex_first);
			}
			*i = c;
			h >>= 4;
			if (!h) {
				break;
			}
		}
	}
	return (s);
}

bool
string_to_hash(byte_string const& s, string_hash& h)
{
	h = string_hash(u32(0));
	for (byte_string::const_iterator i = s.begin(); i != s.end(); ++i) {
		byte_char c = *i;
		if ((byte_code::digit_first <= c) && (c <= byte_code::digit_last)) {
			c = static_cast<byte_char>(c - byte_code::digit_first);
		} else if ((byte_code::hex_first <= c) && (c <= byte_code::hex_last)) {
			c = static_cast<byte_char>(c - byte_code::hex_first + (byte_code::digit_last - byte_code::digit_first + 1));
		} else {
			return (false);
		}
		h = (h << 4) + c;
	}
	return (8 == s.size());
}

} // namespace genome
