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
#ifndef GENOME_HASH_HPP
#define GENOME_HASH_HPP

#include <genome/genome.hpp>
#include <genome/string.hpp>

namespace genome {

typedef u32 string_hash;

//
// ASCII (Windows-1252) case-sensitive hash
//

GENOME_CONSTEXPR_INLINE
string_hash
hash_byte_init(void)
{
	return (string_hash(u32(5381)));
}

string_hash
hash_byte_update(byte_char const* str, string_hash hash);

GENOME_CONSTEXPR_INLINE
string_hash
hash_byte_final(string_hash hash)
{
	return (hash);
}

inline
string_hash
hash_byte(byte_char const* str)
{
	return (hash_byte_final(hash_byte_update(str, hash_byte_init())));
}

inline
string_hash
hash_byte(byte_string const& str)
{
	return (hash_byte(str.c_str()));
}

//
// ASCII (Windows-1252) identifier hash (lower-case)
//

GENOME_CONSTEXPR_INLINE
string_hash
hash_name_init(void)
{
	return (hash_byte_init());
}

string_hash hash_name_update(byte_char const* name, string_hash hash);

GENOME_CONSTEXPR_INLINE
string_hash
hash_name_final(string_hash hash)
{
	return (hash);
}

inline
string_hash
hash_name(byte_char const* name)
{
	return (hash_name_final(hash_name_update(name, hash_name_init())));
}

inline
string_hash
hash_name(byte_string const& name)
{
	return (hash_name(name.c_str()));
}

//
// ASCII (Windows-1252) filename hash (lower-case, without tags)
//

string_hash hash_filename(byte_char const* filename);
string_hash hash_filename(byte_string const& filename);

//
// hash string conversion ([0-9a-f]{8})
//

byte_string hash_to_string(string_hash h);
bool string_to_hash(byte_string const& s, string_hash& h);

} // namespace genome

#endif // GENOME_HASH_HPP
