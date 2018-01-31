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
#ifndef GENOME_TIME_HPP
#define GENOME_TIME_HPP

#include <genome/genome.hpp>
#include <genome/string.hpp>
#include <genome/archive.hpp>
#include <ctime>
#include <string>

namespace genome {

// Windows FILETIME, 100-nanosecond intervals since January 1, 1601 (UTC).
// Value is interpreted as 63-bit (maximum: 30828-09-14T02:48:05.4775807Z)
// and all values with MSB set are treated as invalid. 0 (the start of the
// epoch) is defined as "empty". Because time_t might be limited to 32-bit
// (or a floating point type), datetime::gmtime() / datetime::timegm() are
// provided to convert a datetime to/from the portable C++ struct std::tm.
class datetime {
public:
	datetime(void) GENOME_NOEXCEPT_NOTHROW;
	explicit datetime(u64 ticks) GENOME_NOEXCEPT_NOTHROW;
	explicit datetime(struct std::tm const& utc, long int nanoseconds = 0) GENOME_NOEXCEPT_NOTHROW;
	u64 const& ticks(void) const GENOME_NOEXCEPT_NOTHROW;
	void clear(void) GENOME_NOEXCEPT_NOTHROW;
	bool empty(void) const GENOME_NOEXCEPT_NOTHROW;
	bool valid(void) const GENOME_NOEXCEPT_NOTHROW;
	void gmtime(struct std::tm& utc, long int* nanoseconds = 0) const GENOME_NOEXCEPT_NOTHROW;
	bool timegm(struct std::tm const& utc, long int nanoseconds = 0) GENOME_NOEXCEPT_NOTHROW;
	std::string to_string(bool force_fraction) const GENOME_NOEXCEPT_NOTHROW;
	std::wstring to_wstring(bool force_fraction) const GENOME_NOEXCEPT_NOTHROW;
protected:
	u64 m_ticks;
};
iarchive& operator>>(iarchive& archive, datetime& value);
oarchive& operator<<(oarchive& archive, datetime const& value);
template<>
struct string_converter<datetime, std::string> {
	bool operator()(datetime const& from, std::string& to) const;
};
template<>
struct string_converter<datetime, std::wstring> {
	bool operator()(datetime const& from, std::wstring& to) const;
};

// filetime is a datetime that is truncated to 1-second intervals.
class filetime : public datetime {
public:
	filetime(void) GENOME_NOEXCEPT_NOTHROW;
	explicit filetime(u64 ticks) GENOME_NOEXCEPT_NOTHROW;
	explicit filetime(struct std::tm const& utc, long int nanoseconds = 0) GENOME_NOEXCEPT_NOTHROW;
	explicit filetime(datetime const& time) GENOME_NOEXCEPT_NOTHROW;
	explicit filetime(const char* filename);
	void truncate(void) GENOME_NOEXCEPT_NOTHROW;
};
template<>
struct string_converter<filetime, std::string> {
	bool operator()(filetime const& from, std::string& to) const;
};
template<>
struct string_converter<filetime, std::wstring> {
	bool operator()(filetime const& from, std::wstring& to) const;
};

} // namespace genome

#endif // GENOME_TIME_HPP
