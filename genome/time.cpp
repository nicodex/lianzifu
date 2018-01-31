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
#include <genome/time.hpp>
#include <genome/filesystem.hpp>
#include <cstdio>
#include <cwchar>

namespace genome {

namespace /*{anonymous}*/ {

	u32 const
	days_before_month[2][12] = {
		{
			0,
			31,
			31 + 28,
			31 + 28 + 31,
			31 + 28 + 31 + 30,
			31 + 28 + 31 + 30 + 31,
			31 + 28 + 31 + 30 + 31 + 30,
			31 + 28 + 31 + 30 + 31 + 30 + 31,
			31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
			31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
			31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
			31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
		},
		{
			0,
			31,
			31 + 29,
			31 + 29 + 31,
			31 + 29 + 31 + 30,
			31 + 29 + 31 + 30 + 31,
			31 + 29 + 31 + 30 + 31 + 30,
			31 + 29 + 31 + 30 + 31 + 30 + 31,
			31 + 29 + 31 + 30 + 31 + 30 + 31 + 31,
			31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
			31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
			31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
		}
	};

} // namespace genome::{anonymous}

//
// datetime
//

datetime::datetime(void) GENOME_NOEXCEPT_NOTHROW
	: m_ticks(0)
{
}

datetime::datetime(u64 ticks) GENOME_NOEXCEPT_NOTHROW
	: m_ticks(ticks)
{
}

datetime::datetime(struct std::tm const& utc, long int nanoseconds) GENOME_NOEXCEPT_NOTHROW
{
	if (!timegm(utc, nanoseconds)) {
		clear();
	}
}

u64 const&
datetime::ticks(void) const GENOME_NOEXCEPT_NOTHROW
{
	return (m_ticks);
}

void
datetime::clear(void) GENOME_NOEXCEPT_NOTHROW
{
	m_ticks = 0;
}

bool
datetime::empty(void) const GENOME_NOEXCEPT_NOTHROW
{
	return (m_ticks == 0);
}

bool
datetime::valid(void) const GENOME_NOEXCEPT_NOTHROW
{
	return (static_cast<i64>(m_ticks) > 0);
}

void
datetime::gmtime(struct std::tm& utc, long int* nanoseconds) const GENOME_NOEXCEPT_NOTHROW
{
	u64 const t = m_ticks / 10000000;
	u32 const s = static_cast<u32>(t % 86400);
	utc.tm_sec = static_cast<int>(s % 60);
	utc.tm_min = static_cast<int>((s / 60) % 60);
	utc.tm_hour = static_cast<int>(s / 3600);
	u32 d = static_cast<u32>(t / 86400);
	utc.tm_wday = static_cast<int>((d + 1) % 7);
	u32 const q = d / 146097;
	d -= q * 146097;
	u32 const c = !d ? 0 : (d - 1) / 36524;
	d -= c * 36524;
	u32 const f = d / 1461;
	d -= f * 1461;
	u32 const y = !d ? 0 : (d - 1) / 365;
	d -= y * 365;
	utc.tm_yday = static_cast<int>(d);
	u32 year = (q * 400) + (c * 100) + (f * 4) + y + 1601;
	utc.tm_year = static_cast<int>(static_cast<i32>(year) - 1900);
	bool const l = ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
	int m;
	u32 const(& b)[12] = days_before_month[l];
	for (m = 11; m > 0; --m) {
		if (d > b[m]) {
			d -= b[m];
			break;
		}
	}
	utc.tm_mon = m;
	utc.tm_mday = static_cast<int>(d + 1);
	utc.tm_isdst = 0;
	if (nanoseconds) {
		*nanoseconds = static_cast<long int>(m_ticks % 10000000) * 100;
	}
}

bool
datetime::timegm(struct std::tm const& utc, long int nanoseconds) GENOME_NOEXCEPT_NOTHROW
{
	static u32 const mdays[2][12] = {
		{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
		{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
	};
	u32 const second = static_cast<u32>(utc.tm_sec);
	u32 const minute = static_cast<u32>(utc.tm_min);
	u32 const hour = static_cast<u32>(utc.tm_hour);
	u32 const day = static_cast<u32>(utc.tm_mday - 1);
	u32 const month = static_cast<u32>(utc.tm_mon);
	u32 const year = static_cast<u32>(utc.tm_year + 1900);
	bool const l = ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
	u32 const m = month % 12;
	u32 const y = year - 1601;
	u32 const d = (y * 365) + ((y / 4) - (y / 100) + (y / 400)) + days_before_month[l][m] + day;
	u64 const s = (static_cast<u64>(d) * 86400) + ((((hour * 60) + minute) * 60) + second);
	m_ticks = (s * 10000000) + ((nanoseconds + 50) / 100);
	return (
		(empty() || valid()) &&
		(1600 < year) && (year < 30829) &&
		(month < 12) && (day < mdays[l][m]) &&
		(hour < 24) && (minute < 60) && (second < 62) &&
		(static_cast<unsigned long int>(nanoseconds) < 1000000000)
	);
}

std::string
datetime::to_string(bool force_fraction) const GENOME_NOEXCEPT_NOTHROW
{
	std::string::value_type str[sizeof("+30828-09-14T02:48:05.477580700\0\0")];
	if (valid()) {
		struct std::tm utc;
		long int nanoseconds;
		gmtime(utc, &nanoseconds);
		if (force_fraction || nanoseconds) {
			double seconds = static_cast<double>(utc.tm_sec) + static_cast<double>(nanoseconds) / 1000000000.0;
			std::sprintf(str,
				"%04d-%02d-%02dT%02d:%02d:%012.9f",
				utc.tm_year + 1900, utc.tm_mon + 1, utc.tm_mday,
				utc.tm_hour, utc.tm_min, seconds
			);
		} else {
			std::sprintf(str,
				"%04d-%02d-%02dT%02d:%02d:%02d",
				utc.tm_year + 1900, utc.tm_mon + 1, utc.tm_mday,
				utc.tm_hour, utc.tm_min, utc.tm_sec
			);
		}
	} else {
		u64 const t = ticks();
		std::sprintf(str,
			"[0x%.8" PRIX32 "%.8" PRIX32 "]",
			static_cast<u32>(t >> 32),
			static_cast<u32>(t)
		);
	}
	return (std::string(str));
}

std::wstring
datetime::to_wstring(bool force_fraction) const GENOME_NOEXCEPT_NOTHROW
{
	std::wstring::value_type str[sizeof("+30828-09-14T02:48:05.477580700\0\0")];
	if (valid()) {
		struct std::tm utc;
		long int nanoseconds;
		gmtime(utc, &nanoseconds);
		if (force_fraction || nanoseconds) {
			double seconds = static_cast<double>(utc.tm_sec) + static_cast<double>(nanoseconds) / 1000000000.0;
			std::swprintf(str, sizeof(str) / sizeof(str[0]),
				L"%04d-%02d-%02dT%02d:%02d:%012.9f",
				utc.tm_year + 1900, utc.tm_mon + 1, utc.tm_mday,
				utc.tm_hour, utc.tm_min, seconds
			);
		} else {
			std::swprintf(str, sizeof(str) / sizeof(str[0]),
				L"%04d-%02d-%02dT%02d:%02d:%02d",
				utc.tm_year + 1900, utc.tm_mon + 1, utc.tm_mday,
				utc.tm_hour, utc.tm_min, utc.tm_sec
			);
		}
	} else {
		u64 const t = ticks();
		std::swprintf(str, sizeof(str) / sizeof(str[0]),
			L"[0x%.8" GENOME_CONCAT(L, PRIX32) L"%.8" GENOME_CONCAT(L, PRIX32) L"]",
			static_cast<u32>(t >> 32),
			static_cast<u32>(t)
		);
	}
	return (std::wstring(str));
}

iarchive&
operator>>(iarchive& archive, datetime& value)
{
	u32 ticks[2];
	archive >> ticks;
	value = datetime((static_cast<u64>(ticks[0]) << 32) | ticks[1]);
	return (archive);
}

oarchive&
operator<<(oarchive& archive, datetime const& value)
{
	u32 ticks[2] = {
		static_cast<u32>(value.ticks() >> 32),
		static_cast<u32>(value.ticks())
	};
	archive << ticks;
	return (archive);
}

bool
string_converter<datetime, std::string>::operator()(datetime const& from, std::string& to) const
{
	return (to = from.to_string(true), true);
}
bool
string_converter<datetime, std::wstring>::operator()(datetime const& from, std::wstring& to) const
{
	return (to = from.to_wstring(true), true);
}

//
// filetime
//

filetime::filetime(void) GENOME_NOEXCEPT_NOTHROW
	: datetime()
{
}

filetime::filetime(u64 ticks) GENOME_NOEXCEPT_NOTHROW
	: datetime(ticks)
{
	truncate();
}

filetime::filetime(struct std::tm const& utc, long int nanoseconds) GENOME_NOEXCEPT_NOTHROW
	: datetime(utc, nanoseconds)
{
	truncate();
}

filetime::filetime(datetime const& time) GENOME_NOEXCEPT_NOTHROW
	: datetime(time)
{
	truncate();
}

filetime::filetime(char const* filename)
	: datetime()
{
	struct std::tm utc;
	if (filesystem::get_last_write_time(filename, utc)) {
		if (!timegm(utc)) {
			clear();
		}
	}
}

void
filetime::truncate(void) GENOME_NOEXCEPT_NOTHROW
{
	if (valid()) {
		m_ticks -= m_ticks % 10000000;
	}
}

bool
string_converter<filetime, std::string>::operator()(filetime const& from, std::string& to) const
{
	return (to = from.to_string(false), true);
}
bool
string_converter<filetime, std::wstring>::operator()(filetime const& from, std::wstring& to) const
{
	return (to = from.to_wstring(false), true);
}

} // namespace genome
