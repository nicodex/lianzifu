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
#include <genome/tstream.hpp>
#include <stdexcept>

namespace genome {

//
// tstream
//

void
tstream::does_not_support_comparisons(void) const
{
}

tstream::operator tstream::bool_type(void) const
{
	return (
		operator_bool()
		? &tstream::does_not_support_comparisons
		: 0
	);
}

tstream::operator tstream::bool_type(void)
{
	return (
		operator_bool()
		? &tstream::does_not_support_comparisons
		: 0
	);
}

tstream::~tstream(void)
{
}

bool
tstream::good(void) const
{
	return (std::ios_base::goodbit == rdstate());
}

bool
tstream::eof(void) const
{
	return ((std::ios_base::eofbit & rdstate()) != 0);
}

bool
tstream::fail(void) const
{
	return (((std::ios_base::failbit | std::ios_base::badbit) & rdstate()) != 0);
}

bool
tstream::bad(void) const
{
	return ((std::ios_base::badbit & rdstate()) != 0);
}

} // namespace genome
