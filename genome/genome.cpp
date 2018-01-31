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
#include <genome/genome.hpp>
#include <genome/locale.hpp>
#include <genome/filesystem.hpp>

void
init_genome(void)
{
	genome::locale::init();
	genome::filesystem::init();
}

namespace genome {

namespace /*{anonymous}*/ {

	char const*
	skip_space(char const*& s)
	{
		if (!s) {
			s = "";
		} else {
			while (('\t' == *s) || (' ' == *s)) {
				++s;
			}
		}
		return (s);
	}

	bool
	is_end(char const*& s)
	{
		return ('\0' == *skip_space(s));
	}

} // namespace genome::{anonymous}

//
// platforms
//

char const*
platform_name(platform target)
{
	switch (target) {
	default:
	case platform_unknown:
		return ("<invalid>");
	case platform_pc:
		return ("pc");
	case platform_x64:
		return ("x64");
	case platform_ps3:
		return ("ps3");
	case platform_ps4:
		return ("ps4");
	case platform_x360:
		return ("x360");
	case platform_xone:
		return ("xone");
	}
}

char const*
platform_desc(platform target)
{
	switch (target) {
	default:
	case platform_unknown:
		return ("<invalid>");
	case platform_pc:
		return ("Windows (32-bit)");
	case platform_x64:
		return ("Windows (64-bit)");
	case platform_ps3:
		return ("PlayStation 3");
	case platform_ps4:
		return ("PlayStation 4");
	case platform_x360:
		return ("Xbox 360");
	case platform_xone:
		return ("Xbox One");
	}
}

platform
platform_from_name(char const* name)
{
	switch (*skip_space(name)) {
	case 'p': case 'P':
		switch (*++name) {
		case 'c': case 'C':
			if (is_end(++name)) {
				return (platform_pc);
			}
			break;
		case 's': case 'S':
			switch (*++name) {
			case '3':
				if (is_end(++name)) {
					return (platform_ps3);
				}
				break;
			case '4':
				if (is_end(++name)) {
					return (platform_ps4);
				}
				break;
			}
			break;
		}
		break;
	case 'x': case 'X':
		switch (*++name) {
		case '3':
			if ('6' == *++name) {
				if ('0' == *++name) {
					if (is_end(++name)) {
						return (platform_x360);
					}
				}
			}
			break;
		case '6':
			if ('4' == *++name) {
				if (is_end(++name)) {
					return (platform_x64);
				}
			}
			break;
		case 'o': case 'O':
			if (++name, ('n' == *name) || ('N' == *name)) {
				if (++name, ('e' == *name) || ('E' == *name)) {
					if (is_end(++name)) {
						return (platform_xone);
					}
				}
			}
			break;
		}
		break;
	}
	return (platform_unknown);
}

} // namespace genome
