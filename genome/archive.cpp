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
#include <genome/archive.hpp>
#include <stdexcept>

namespace genome {

//
// archive
//

void
archive::does_not_support_comparisons(void) const
{
}

archive::operator archive::bool_type(void) const
{
	return (
		operator_bool()
		? &archive::does_not_support_comparisons
		: 0
	);
}

archive::operator archive::bool_type(void)
{
	return (
		operator_bool()
		? &archive::does_not_support_comparisons
		: 0
	);
}

archive::~archive(void)
{
}

bool
archive::good(void) const
{
	return (std::ios_base::goodbit == rdstate());
}

bool
archive::eof(void) const
{
	return ((std::ios_base::eofbit & rdstate()) != 0);
}

bool
archive::fail(void) const
{
	return (((std::ios_base::failbit | std::ios_base::badbit) & rdstate()) != 0);
}

bool
archive::bad(void) const
{
	return ((std::ios_base::badbit & rdstate()) != 0);
}

//
// archive::streamref
//

archive::streamref::streamref(void)
	: size(0)
	, pos(0)
{
}

archive::streamref::streamref(streampos p, streamsize n)
	: size(n)
	, pos(p)
{
}

void
archive::streamref::clear(void)
{
	size = 0;
	pos = 0;
}

bool
archive::streamref::empty(void) const
{
	return (0 == size);
}

bool
archive::streamref::eof(void) const
{
	return (
		(streampos(-1) == pos) ||
		(pos + size < pos)
	);
}

bool
archive::streamref::valid(void) const
{
	return (
		(pos >= static_cast<streampos>(sizeof(header))) &&
		(pos + size > pos)
	);
}

//
// archive::header
//

archive::header::header(iarchive& input)
{
	if (!(input >> data)) {
		throw std::runtime_error("failed to read Genome Archive header");
	}
	if ((data[0] != 0x47) ||  // 'G'
	    (data[1] != 0x41) ||  // 'A'
	    (data[2] != 0x52)) {  // 'R'
		throw std::invalid_argument("invalid Genome Archive signature");
	}
	if (data[3] != 0x35) {  // '5'
		throw std::invalid_argument("unsupported Genome Archive version");
	}
	if (((data[4] & 0x30) < 0x10) || (0x20 < (data[4] & 0x30))) {
		throw std::invalid_argument("invalid Genome Archive endianness");
	}
	if ((data[4] & ~0x30) ||
	    (data[5] != 0) ||
	    (data[6] != 0) ||
	    (data[7] != 0)) {
		throw std::invalid_argument("unsupported Genome Archive attributes");
	}
}

archive::header::header(byte_order endianness)
{
	data[0] = 0x47;  // 'G'
	data[1] = 0x41;  // 'A'
	data[2] = 0x52;  // 'R'
	data[3] = 0x35;  // '5'
	data[4] = static_cast<u8>(
		(big_endian == endianness)
		? 0x10
		: 0x20
	);
	data[5] = 0x00;
	data[6] = 0x00;
	data[7] = 0x00;
}

oarchive&
archive::header::write(oarchive& output)
{
	return (output << data);
}

archive::byte_order
archive::header::endianness(void) const
{
	return (
		(0x10 & data[4])
		? big_endian
		: little_endian
	);
}

//
// iarchive
//

iarchive&
iarchive::read(u8 values[], streamsize count)
{
	return (read_octets(values, count));
}

iarchive&
iarchive::read(i8 values[], streamsize count)
{
	return (read(reinterpret_cast<u8*>(&values[0]), count));
}

iarchive&
iarchive::read(u16 values[], streamsize count)
{
	if (count > 0) {
		bool const overflow = (count > streamsize_limits<u16>::max_count());
		std::size_t const size = overflow
			? static_cast<std::size_t>(streamsize_limits<u16>::max_size())
			: static_cast<std::size_t>(count) * sizeof(values[0]);
		switch (endianness()) {
		case big_endian:
			if (target_integer_big_endian) {
				if (!read(reinterpret_cast<u8*>(&values[0]), static_cast<streamsize>(size))) {
					streamsize end = gcount() / static_cast<streamsize>(sizeof(values[0]));
					for (streamsize i = end; i < count; ++i) {
						values[i] = 0;
					}
				}
			} else {
				std::vector<u8> octets(size);
				if (!read(&octets[0], static_cast<streamsize>(size))) {
					streamsize end = gcount() / static_cast<streamsize>(sizeof(values[0]));
					for (streamsize i = end; i < count; ++i) {
						values[i] = 0;
					}
					count = end;
				}
				for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
					detail::read_big_endian(values[i],
						reinterpret_cast<u8 const(&)[sizeof(values[0])]>(octets[i * sizeof(values[0])]));
				}
			}
			break;
		case little_endian:
			if (target_integer_little_endian) {
				if (!read(reinterpret_cast<u8*>(&values[0]), static_cast<streamsize>(size))) {
					streamsize end = gcount() / static_cast<streamsize>(sizeof(values[0]));
					for (streamsize i = end; i < count; ++i) {
						values[i] = 0;
					}
				}
			} else {
				std::vector<u8> octets(size);
				if (!read(&octets[0], static_cast<streamsize>(size))) {
					streamsize end = gcount() / static_cast<streamsize>(sizeof(values[0]));
					for (streamsize i = end; i < count; ++i) {
						values[i] = 0;
					}
					count = end;
				}
				for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
					detail::read_little_endian(values[i],
						reinterpret_cast<u8 const(&)[sizeof(values[0])]>(octets[i * sizeof(values[0])]));
				}
			}
			break;
		}
		if (overflow && good()) {
			setstate(std::ios_base::failbit);
		}
	}
	return (*this);
}

iarchive&
iarchive::read(i16 values[], streamsize count)
{
	return (read(reinterpret_cast<u16*>(values), count));
}

iarchive&
iarchive::read(u32 values[], streamsize count)
{
	if (count > 0) {
		bool const overflow = (count > streamsize_limits<u32>::max_count());
		std::size_t const size = overflow
			? static_cast<std::size_t>(streamsize_limits<u32>::max_size())
			: static_cast<std::size_t>(count) * sizeof(values[0]);
		switch (endianness()) {
		case big_endian:
			if (target_integer_big_endian) {
				if (!read(reinterpret_cast<u8*>(&values[0]), static_cast<streamsize>(size))) {
					streamsize end = gcount() / static_cast<streamsize>(sizeof(values[0]));
					for (streamsize i = end; i < count; ++i) {
						values[i] = 0;
					}
				}
			} else {
				std::vector<u8> octets(size);
				if (!read(&octets[0], static_cast<streamsize>(size))) {
					streamsize end = gcount() / static_cast<streamsize>(sizeof(values[0]));
					for (streamsize i = end; i < count; ++i) {
						values[i] = 0;
					}
					count = end;
				}
				for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
					detail::read_big_endian(values[i],
						reinterpret_cast<u8 const(&)[sizeof(values[0])]>(octets[i * sizeof(values[0])]));
				}
			}
			break;
		case little_endian:
			if (target_integer_little_endian) {
				if (!read(reinterpret_cast<u8*>(&values[0]), static_cast<streamsize>(size))) {
					streamsize end = gcount() / static_cast<streamsize>(sizeof(values[0]));
					for (streamsize i = end; i < count; ++i) {
						values[i] = 0;
					}
				}
			} else {
				std::vector<u8> octets(size);
				if (!read(&octets[0], static_cast<streamsize>(size))) {
					streamsize end = gcount() / static_cast<streamsize>(sizeof(values[0]));
					for (streamsize i = end; i < count; ++i) {
						values[i] = 0;
					}
					count = end;
				}
				for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
					detail::read_little_endian(values[i],
						reinterpret_cast<u8 const(&)[sizeof(values[0])]>(octets[i * sizeof(values[0])]));
				}
			}
			break;
		}
		if (overflow && good()) {
			setstate(std::ios_base::failbit);
		}
	}
	return (*this);
}

iarchive&
iarchive::read(i32 values[], streamsize count)
{
	return (read(reinterpret_cast<u32*>(values), count));
}

iarchive&
iarchive::read(u64 values[], streamsize count)
{
	if (count > 0) {
		bool const overflow = (count > streamsize_limits<u64>::max_count());
		std::size_t const size = overflow
			? static_cast<std::size_t>(streamsize_limits<u64>::max_size())
			: static_cast<std::size_t>(count) * sizeof(values[0]);
		switch (endianness()) {
		case big_endian:
			if (target_integer_big_endian) {
				if (!read(reinterpret_cast<u8*>(&values[0]), static_cast<streamsize>(size))) {
					streamsize end = gcount() / static_cast<streamsize>(sizeof(values[0]));
					for (streamsize i = end; i < count; ++i) {
						values[i] = 0;
					}
				}
			} else {
				std::vector<u8> octets(size);
				if (!read(&octets[0], static_cast<streamsize>(size))) {
					streamsize end = gcount() / static_cast<streamsize>(sizeof(values[0]));
					for (streamsize i = end; i < count; ++i) {
						values[i] = 0;
					}
					count = end;
				}
				for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
					detail::read_big_endian(values[i],
						reinterpret_cast<u8 const(&)[sizeof(values[0])]>(octets[i * sizeof(values[0])]));
				}
			}
			break;
		case little_endian:
			if (target_integer_little_endian) {
				if (!read(reinterpret_cast<u8*>(&values[0]), static_cast<streamsize>(size))) {
					streamsize end = gcount() / static_cast<streamsize>(sizeof(values[0]));
					for (streamsize i = end; i < count; ++i) {
						values[i] = 0;
					}
				}
			} else {
				std::vector<u8> octets(size);
				if (!read(&octets[0], static_cast<streamsize>(size))) {
					streamsize end = gcount() / static_cast<streamsize>(sizeof(values[0]));
					for (streamsize i = end; i < count; ++i) {
						values[i] = 0;
					}
					count = end;
				}
				for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
					detail::read_little_endian(values[i],
						reinterpret_cast<u8 const(&)[sizeof(values[0])]>(octets[i * sizeof(values[0])]));
				}
			}
			break;
		}
		if (overflow && good()) {
			setstate(std::ios_base::failbit);
		}
	}
	return (*this);
}

iarchive&
iarchive::read(i64 values[], streamsize count)
{
	return (read(reinterpret_cast<u64*>(values), count));
}

iarchive&
iarchive::read(streamref values[], streamsize count)
{
	if (count > 0) {
		bool const overflow = (count > streamsize_limits<streamref>::max_count());
		if (overflow) {
			count = streamsize_limits<streamref>::max_count();
		}
		read(reinterpret_cast<u32*>(&values[0]), count * 2);
		if (overflow && good()) {
			setstate(std::ios_base::failbit);
		}
	}
	return (*this);
}

//
// operator>>(iarchive, ...)
//

iarchive&
operator>>(iarchive& archive, u8& value)
{
	return (archive.read(&value, 1));
}

iarchive&
operator>>(iarchive& archive, i8& value)
{
	return (operator>>(archive, reinterpret_cast<u8&>(value)));
}

iarchive&
operator>>(iarchive& archive, u16& value)
{
	switch (archive.endianness()) {
	case archive::big_endian:
		if (target_integer_big_endian) {
			archive.read(reinterpret_cast<u8*>(&value), sizeof(value));
		} else {
			u8 octets[sizeof(value)];
			archive.read(&octets[0], sizeof(octets));
			detail::read_big_endian(value, octets);
		}
		break;
	case archive::little_endian:
		if (target_integer_little_endian) {
			archive.read(reinterpret_cast<u8*>(&value), sizeof(value));
		} else {
			u8 octets[sizeof(value)];
			archive.read(&octets[0], sizeof(octets));
			detail::read_little_endian(value, octets);
		}
		break;
	}
	return (archive);
}

iarchive&
operator>>(iarchive& archive, i16& value)
{
	return (operator>>(archive, reinterpret_cast<u16&>(value)));
}

iarchive&
operator>>(iarchive& archive, u32& value)
{
	switch (archive.endianness()) {
	case archive::big_endian:
		if (target_integer_big_endian) {
			archive.read(reinterpret_cast<u8*>(&value), sizeof(value));
		} else {
			u8 octets[sizeof(value)];
			archive.read(&octets[0], sizeof(octets));
			detail::read_big_endian(value, octets);
		}
		break;
	case archive::little_endian:
		if (target_integer_little_endian) {
			archive.read(reinterpret_cast<u8*>(&value), sizeof(value));
		} else {
			u8 octets[sizeof(value)];
			archive.read(&octets[0], sizeof(octets));
			detail::read_little_endian(value, octets);
		}
		break;
	}
	return (archive);
}

iarchive&
operator>>(iarchive& archive, i32& value)
{
	return (operator>>(archive, reinterpret_cast<u32&>(value)));
}

iarchive&
operator>>(iarchive& archive, u64& value)
{
	switch (archive.endianness()) {
	case archive::big_endian:
		if (target_integer_big_endian) {
			archive.read(reinterpret_cast<u8*>(&value), sizeof(value));
		} else {
			u8 octets[sizeof(value)];
			archive.read(&octets[0], sizeof(octets));
			detail::read_big_endian(value, octets);
		}
		break;
	case archive::little_endian:
		if (target_integer_little_endian) {
			archive.read(reinterpret_cast<u8*>(&value), sizeof(value));
		} else {
			u8 octets[sizeof(value)];
			archive.read(&octets[0], sizeof(octets));
			detail::read_little_endian(value, octets);
		}
		break;
	}
	return (archive);
}

iarchive&
operator>>(iarchive& archive, i64& value)
{
	return (operator>>(archive, reinterpret_cast<u64&>(value)));
}

iarchive&
operator>>(iarchive& archive, archive::streamref& value)
{
	return (archive >> reinterpret_cast<i32(&)[2]>(value));
}

//
// oarchive
//

oarchive&
oarchive::write(u8 const values[], streamsize count)
{
	return (write_octets(values, count));
}

oarchive&
oarchive::write(i8 const values[], streamsize count)
{
	return (write(reinterpret_cast<u8 const*>(values), count));
}

oarchive&
oarchive::write(u16 const values[], streamsize count)
{
	if (count > 0) {
		bool const overflow = (count > streamsize_limits<u16>::max_count());
		std::size_t const size = overflow
			? static_cast<std::size_t>(streamsize_limits<u16>::max_size())
			: static_cast<std::size_t>(count) * sizeof(values[0]);
		switch (endianness()) {
		case big_endian:
			if (target_integer_big_endian) {
				write(reinterpret_cast<u8 const*>(&values[0]), static_cast<streamsize>(size));
			} else {
				std::vector<u8> octets(size);
				for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
					detail::write_big_endian(values[i],
						reinterpret_cast<u8(&)[sizeof(values[0])]>(octets[i * sizeof(values[0])]));
				}
				write(&octets[0], static_cast<streamsize>(size));
			}
			break;
		case little_endian:
			if (target_integer_little_endian) {
				write(reinterpret_cast<u8 const*>(&values[0]), static_cast<streamsize>(size));
			} else {
				std::vector<u8> octets(size);
				for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
					detail::write_little_endian(values[i],
						reinterpret_cast<u8(&)[sizeof(values[0])]>(octets[i * sizeof(values[0])]));
				}
				write(&octets[0], static_cast<streamsize>(size));
			}
			break;
		}
		if (overflow && good()) {
			setstate(std::ios_base::failbit);
		}
	}
	return (*this);
}

oarchive&
oarchive::write(i16 const values[], streamsize count)
{
	return (write(reinterpret_cast<u16 const*>(values), count));
}

oarchive&
oarchive::write(u32 const values[], streamsize count)
{
	if (count > 0) {
		bool const overflow = (count > streamsize_limits<u32>::max_count());
		std::size_t const size = overflow
			? static_cast<std::size_t>(streamsize_limits<u32>::max_size())
			: static_cast<std::size_t>(count) * sizeof(values[0]);
		switch (endianness()) {
		case big_endian:
			if (target_integer_big_endian) {
				write(reinterpret_cast<u8 const*>(&values[0]), static_cast<streamsize>(size));
			} else {
				std::vector<u8> octets(size);
				for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
					detail::write_big_endian(values[i],
						reinterpret_cast<u8(&)[sizeof(values[0])]>(octets[i * sizeof(values[0])]));
				}
				write(&octets[0], static_cast<streamsize>(size));
			}
			break;
		case little_endian:
			if (target_integer_little_endian) {
				write(reinterpret_cast<u8 const*>(&values[0]), static_cast<streamsize>(size));
			} else {
				std::vector<u8> octets(size);
				for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
					detail::write_little_endian(values[i],
						reinterpret_cast<u8(&)[sizeof(values[0])]>(octets[i * sizeof(values[0])]));
				}
				write(&octets[0], static_cast<streamsize>(size));
			}
			break;
		}
		if (overflow && good()) {
			setstate(std::ios_base::failbit);
		}
	}
	return (*this);
}

oarchive&
oarchive::write(i32 const values[], streamsize count)
{
	return (write(reinterpret_cast<u32 const*>(values), count));
}

oarchive&
oarchive::write(u64 const values[], streamsize count)
{
	if (count > 0) {
		bool const overflow = (count > streamsize_limits<u64>::max_count());
		std::size_t const size = overflow
			? static_cast<std::size_t>(streamsize_limits<u64>::max_size())
			: static_cast<std::size_t>(count) * sizeof(values[0]);
		switch (endianness()) {
		case big_endian:
			if (target_integer_big_endian) {
				write(reinterpret_cast<u8 const*>(&values[0]), static_cast<streamsize>(size));
			} else {
				std::vector<u8> octets(size);
				for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
					detail::write_big_endian(values[i],
						reinterpret_cast<u8(&)[sizeof(values[0])]>(octets[i * sizeof(values[0])]));
				}
				write(&octets[0], static_cast<streamsize>(size));
			}
			break;
		case little_endian:
			if (target_integer_little_endian) {
				write(reinterpret_cast<u8 const*>(&values[0]), static_cast<streamsize>(size));
			} else {
				std::vector<u8> octets(size);
				for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
					detail::write_little_endian(values[i],
						reinterpret_cast<u8(&)[sizeof(values[0])]>(octets[i * sizeof(values[0])]));
				}
				write(&octets[0], static_cast<streamsize>(size));
			}
			break;
		}
		if (overflow && good()) {
			setstate(std::ios_base::failbit);
		}
	}
	return (*this);
}

oarchive&
oarchive::write(i64 const values[], streamsize count)
{
	return (write(reinterpret_cast<u64 const*>(values), count));
}

oarchive&
oarchive::write(streamref const values[], streamsize count)
{
	if (count > 0) {
		bool const overflow = (count > streamsize_limits<streamref>::max_count());
		if (overflow) {
			count = streamsize_limits<streamref>::max_count();
		}
		write(reinterpret_cast<i32 const*>(&values[0]), count * 2);
		if (overflow && good()) {
			setstate(std::ios_base::failbit);
		}
	}
	return (*this);
}

//
// operator<<(oarchive, ...)
//

oarchive&
operator<<(oarchive& archive, u8 const& value)
{
	return (archive.write(&value, 1));
}

oarchive&
operator<<(oarchive& archive, i8 const& value)
{
	return (operator<<(archive, reinterpret_cast<u8 const&>(value)));
}

oarchive&
operator<<(oarchive& archive, u16 const& value)
{
	switch (archive.endianness()) {
	case archive::big_endian:
		if (target_integer_big_endian) {
			archive.write(reinterpret_cast<u8 const*>(&value), sizeof(value));
		} else {
			u8 octets[sizeof(value)];
			detail::write_big_endian(value, octets);
			archive.write(&octets[0], sizeof(octets));
		}
		break;
	case archive::little_endian:
		if (target_integer_little_endian) {
			archive.write(reinterpret_cast<u8 const*>(&value), sizeof(value));
		} else {
			u8 octets[sizeof(value)];
			detail::write_little_endian(value, octets);
			archive.write(&octets[0], sizeof(octets));
		}
		break;
	}
	return (archive);
}

oarchive&
operator<<(oarchive& archive, i16 const& value)
{
	return (operator<<(archive, reinterpret_cast<u16 const&>(value)));
}

oarchive&
operator<<(oarchive& archive, u32 const& value)
{
	switch (archive.endianness()) {
	case archive::big_endian:
		if (target_integer_big_endian) {
			archive.write(reinterpret_cast<u8 const*>(&value), sizeof(value));
		} else {
			u8 octets[sizeof(value)];
			detail::write_big_endian(value, octets);
			archive.write(&octets[0], sizeof(octets));
		}
		break;
	case archive::little_endian:
		if (target_integer_little_endian) {
			archive.write(reinterpret_cast<u8 const*>(&value), sizeof(value));
		} else {
			u8 octets[sizeof(value)];
			detail::write_little_endian(value, octets);
			archive.write(&octets[0], sizeof(octets));
		}
		break;
	}
	return (archive);
}

oarchive&
operator<<(oarchive& archive, i32 const& value)
{
	return (operator<<(archive, reinterpret_cast<u32 const&>(value)));
}

oarchive&
operator<<(oarchive& archive, u64 const& value)
{
	switch (archive.endianness()) {
	case archive::big_endian:
		if (target_integer_big_endian) {
			archive.write(reinterpret_cast<u8 const*>(&value), sizeof(value));
		} else {
			u8 octets[sizeof(value)];
			detail::write_big_endian(value, octets);
			archive.write(&octets[0], sizeof(octets));
		}
		break;
	case archive::little_endian:
		if (target_integer_little_endian) {
			archive.write(reinterpret_cast<u8 const*>(&value), sizeof(value));
		} else {
			u8 octets[sizeof(value)];
			detail::write_little_endian(value, octets);
			archive.write(&octets[0], sizeof(octets));
		}
		break;
	}
	return (archive);
}

oarchive&
operator<<(oarchive& archive, i64 const& value)
{
	return (operator<<(archive, reinterpret_cast<u64 const&>(value)));
}

oarchive&
operator<<(oarchive& archive, archive::streamref const& value)
{
	return (archive << reinterpret_cast<u32 const(&)[2]>(value));
}

//
// onarchive
//

onarchive::onarchive(void)
	: m_except(std::ios_base::goodbit), m_state(std::ios_base::goodbit),
	  m_pos(sizeof(header))
{
}

bool
onarchive::operator_bool(void) const
{
	return (!fail());
}

bool
onarchive::operator!(void) const
{
	return (fail());
}

std::ios_base::iostate
onarchive::rdstate(void) const
{
	return (m_state);
}

void
onarchive::setstate(std::ios_base::iostate state)
{
	clear(m_state | state);
}

void
onarchive::clear(std::ios_base::iostate state)
{
	m_state = state;
	if ((m_except & m_state) != 0) {
		throw std::ios_base::failure("null stream error");
	}
}

std::ios_base::iostate
onarchive::exceptions(void) const
{
	return(m_except);
}

void
onarchive::exceptions(std::ios_base::iostate except)
{
	m_except = except;
	clear(m_state);
}

archive::byte_order
onarchive::endianness(void) const
{
	return (big_endian);
}

archive::streampos
onarchive::tellp(void) const
{
	return (m_pos);
}

onarchive&
onarchive::seekp(streampos pos)
{
	if (pos < static_cast<streampos>(sizeof(header))) {
		setstate(std::ios_base::failbit);
	} else {
		m_pos = pos;
	}
	return (*this);
}

onarchive&
onarchive::write_octets(u8 const* values, streamsize count)
{
	if ((count > 0) && good()) {
		if (!values) {
			setstate(std::ios_base::failbit);
		} else {
			streampos pos = m_pos + count;
			if (m_pos > pos) {
				m_pos = streampos(-1);
				setstate(std::ios_base::failbit);
			} else {
				m_pos = pos;
			}
		}
	}
	return (*this);
}

archive::streamref
onarchive::ref_begin(void) const
{
	return (streamref(m_pos));
}

void
onarchive::ref_end(streamref& ref) const
{
	if (m_pos >= ref.pos) {
		ref.size = m_pos - ref.pos;
	}
}

} // namespace genome
