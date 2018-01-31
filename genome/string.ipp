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
#ifndef GENOME_STRING_IPP
#define GENOME_STRING_IPP

#include <stdexcept>

namespace genome {

//
// Utilities
//

template<typename StringT>
bool
string_startswith(StringT const& str, StringT const& prefix) GENOME_NOEXCEPT_NOTHROW
{
	typename StringT::size_type const size = prefix.size();
	if ((0 == size) || (str.size() < size)) {
		return (false);
	}
	return (0 == StringT::traits_type::compare(str.c_str(), prefix.c_str(), size));
}

//
// Canonically equivalent conversion (default char '?')
//

template<typename T, typename U>
bool
string_converter<T, U>::operator()(T const& from, U& to) const
{
	return (to.assign(from.c_str(), to.size()), true);
}

template<typename T>
bool
string_converter<T, T>::operator()(T const& from, T& to) const
{
	return (to = from, true);
}

template<typename T, typename U>
bool
string_convert(T const& from, U& to)
{
	return (string_converter<T, U>().operator()(from, to));
}

//
// Conversion cast (throws std::range_error if not canonically equivalent)
//

template<typename T, typename U>
U
string_caster<T, U>::operator()(T const& value) const
{
	U result;
	if (!string_converter<T, U>().operator()(value, result)) {
		throw std::range_error("bad string conversion");
	}
	return (result);
}

template<typename T>
T
string_caster<T, T>::operator()(T const& value) const
{
	return (value);
}

template<typename T, typename U>
U string_cast(T const& value)
{
	return (string_caster<T, U>().operator()(value));
}

template<typename T>
std::string
to_string(T const& value)
{
	return (string_caster<T, std::string>().operator()(value));
}

template<typename T>
std::wstring
to_wstring(T const& value)
{
	return (string_caster<T, std::wstring>().operator()(value));
}

template<typename T>
byte_string
to_byte_string(T const& value)
{
	return (string_caster<T, byte_string>().operator()(value));
}

template<typename T>
wide_string
to_wide_string(T const& value)
{
	return (string_caster<T, wide_string>().operator()(value));
}

} // namespace genome

#endif // GENOME_STRING_IPP
