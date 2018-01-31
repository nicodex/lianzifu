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
#ifndef GENOME_ARCHIVE_HPP
#define GENOME_ARCHIVE_HPP

#include <genome/genome.hpp>
#include <genome/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <vector>

namespace genome {

class iarchive;
class oarchive;

class archive {
	void does_not_support_comparisons(void) const;
public:
	typedef void (archive::*bool_type)(void) const;
	operator bool_type(void) const;
	operator bool_type(void);
	enum byte_order {
		big_endian,
		little_endian
	};
	static GENOME_CONSTEXPR_INLINE
	byte_order platform_endianess(platform target)
	{
		return (
			(
				(target == platform_ps3) ||
				(target == platform_x360)
			)
			? big_endian
			: little_endian
		);
	}
	typedef u32 streampos;
	typedef u32 streamsize;
	template<typename T>
	struct streamsize_limits {
		static
		GENOME_CONSTEXPR_INLINE
		streamsize max_count(void)
		{
			return (static_cast<streamsize>(streamsize(-1) / sizeof(T)));
		}
		static
		GENOME_CONSTEXPR_INLINE
		streamsize max_size(void)
		{
			return (static_cast<streamsize>(max_count() * sizeof(T)));
		}
	};
	virtual ~archive(void);
	virtual bool operator_bool(void) const = 0;
	virtual bool operator!(void) const = 0;
	virtual std::ios_base::iostate rdstate(void) const = 0;
	virtual void setstate(std::ios_base::iostate state) = 0;
	virtual void clear(std::ios_base::iostate state = std::ios_base::goodbit) = 0;
	virtual std::ios_base::iostate exceptions(void) const = 0;
	virtual void exceptions(std::ios_base::iostate except) = 0;
	bool good(void) const;
	bool eof(void) const;
	bool fail(void) const;
	bool bad(void) const;
	virtual byte_order endianness(void) const = 0;

	struct streamref {
		// do not change the member types and/or order (streamed as u32[2])
		streamsize size;
		streampos pos;
		streamref(void);
		explicit streamref(streampos p, streamsize n = 0);
		void clear(void);
		bool empty(void) const;
		bool eof(void) const;
		bool valid(void) const;
	};
	GENOME_STATIC_ASSERT(streamref_size, 64 == (sizeof(streamref) * CHAR_BIT),
		"archive::streamref must be 2 * 32-bit in size.");

protected:
	struct header {
		u8 data[8];  // "GAR5", {0x10 big_endian, 0x20 little_endian}, 0, 0, 0
		explicit header(iarchive& input);
		explicit header(byte_order endianness);
		oarchive& write(oarchive& stream);
		byte_order endianness(void) const;
	};
};
template<typename T>
bool operator!=(archive const& lhs, T const& rhs);
template<typename T>
bool operator==(archive const& lhs, T const& rhs);

class iarchive : public archive {  // virtual inheritance if ioarchive is introduced
public:
	virtual streampos tellg(void) = 0;
	virtual iarchive& seekg(streampos pos) = 0;
	virtual iarchive& read_octets(u8* values, streamsize count) = 0;
	virtual streamsize gcount(void) const = 0;
	iarchive& read(u8 values[], streamsize count);
	iarchive& read(i8 values[], streamsize count);
	iarchive& read(u16 values[], streamsize count);
	iarchive& read(i16 values[], streamsize count);
	iarchive& read(u32 values[], streamsize count);
	iarchive& read(i32 values[], streamsize count);
	iarchive& read(u64 values[], streamsize count);
	iarchive& read(i64 values[], streamsize count);
	iarchive& read(streamref values[], streamsize count);
	template<typename T>
	iarchive& read(std::vector<T>& values, streamsize count);
};
iarchive& operator>>(iarchive& archive, u8& value);
iarchive& operator>>(iarchive& archive, i8& value);
iarchive& operator>>(iarchive& archive, u16& value);
iarchive& operator>>(iarchive& archive, i16& value);
iarchive& operator>>(iarchive& archive, u32& value);
iarchive& operator>>(iarchive& archive, i32& value);
iarchive& operator>>(iarchive& archive, u64& value);
iarchive& operator>>(iarchive& archive, i64& value);
iarchive& operator>>(iarchive& archive, archive::streamref& value);
template<std::size_t N>
iarchive& operator>>(iarchive& archive, u8(& values)[N]);
template<std::size_t N>
iarchive& operator>>(iarchive& archive, i8(& values)[N]);
template<std::size_t N>
iarchive& operator>>(iarchive& archive, u16(& values)[N]);
template<std::size_t N>
iarchive& operator>>(iarchive& archive, i16(& values)[N]);
template<std::size_t N>
iarchive& operator>>(iarchive& archive, u32(& values)[N]);
template<std::size_t N>
iarchive& operator>>(iarchive& archive, i32(& values)[N]);
template<std::size_t N>
iarchive& operator>>(iarchive& archive, u64(& values)[N]);
template<std::size_t N>
iarchive& operator>>(iarchive& archive, i64(& values)[N]);
template<std::size_t N>
iarchive& operator>>(iarchive& archive, archive::streamref(& values)[N]);

template<typename T>
class iarchive_ref : public iarchive {
	iarchive_ref(iarchive_ref const&) GENOME_DELETE_FUNCTION;
	iarchive_ref& operator=(iarchive_ref const&) GENOME_DELETE_FUNCTION;
public:
	explicit iarchive_ref(T& stream);
	virtual bool operator_bool(void) const GENOME_OVERRIDE;
	virtual bool operator!(void) const GENOME_OVERRIDE;
	virtual std::ios_base::iostate rdstate(void) const GENOME_OVERRIDE;
	virtual void setstate(std::ios_base::iostate state) GENOME_OVERRIDE;
	virtual void clear(std::ios_base::iostate state = std::ios_base::goodbit) GENOME_OVERRIDE;
	virtual std::ios_base::iostate exceptions(void) const GENOME_OVERRIDE;
	virtual void exceptions(std::ios_base::iostate except) GENOME_OVERRIDE;
	virtual byte_order endianness(void) const GENOME_OVERRIDE;
	virtual streampos tellg(void) GENOME_OVERRIDE;
	virtual iarchive_ref& seekg(streampos pos) GENOME_OVERRIDE;
	virtual iarchive_ref& read_octets(u8* values, streamsize count) GENOME_OVERRIDE;
	virtual streamsize gcount(void) const GENOME_OVERRIDE;
	T& streamg(void);
private:
	T& m_stream;
	byte_order m_endianness;
};
typedef iarchive_ref<std::istream> isarchive;

template<typename T>
class iarchive_obj : public iarchive {
	iarchive_obj(iarchive_obj const&) GENOME_DELETE_FUNCTION;
	iarchive_obj& operator=(iarchive_obj const&) GENOME_DELETE_FUNCTION;
public:
	explicit iarchive_obj(char const* filename);
	virtual bool operator_bool(void) const GENOME_OVERRIDE;
	virtual bool operator!(void) const GENOME_OVERRIDE;
	virtual std::ios_base::iostate rdstate(void) const GENOME_OVERRIDE;
	virtual void setstate(std::ios_base::iostate state) GENOME_OVERRIDE;
	virtual void clear(std::ios_base::iostate state = std::ios_base::goodbit) GENOME_OVERRIDE;
	virtual std::ios_base::iostate exceptions(void) const GENOME_OVERRIDE;
	virtual void exceptions(std::ios_base::iostate except) GENOME_OVERRIDE;
	virtual byte_order endianness(void) const GENOME_OVERRIDE;
	virtual streampos tellg(void) GENOME_OVERRIDE;
	virtual iarchive_obj& seekg(streampos pos) GENOME_OVERRIDE;
	virtual iarchive_obj& read_octets(u8* values, streamsize count) GENOME_OVERRIDE;
	virtual streamsize gcount(void) const GENOME_OVERRIDE;
private:
	T m_stream;
	iarchive_ref<T> m_archive;
};
typedef iarchive_obj<std::ifstream> ifarchive;

class oarchive : public archive {  // virtual inheritance if ioarchive is introduced
public:
	virtual streampos tellp(void) const = 0;
	virtual oarchive& seekp(streampos pos) = 0;
	virtual oarchive& write_octets(u8 const* values, streamsize count) = 0;
	oarchive& write(u8 const values[], streamsize count);
	oarchive& write(i8 const values[], streamsize count);
	oarchive& write(u16 const values[], streamsize count);
	oarchive& write(i16 const values[], streamsize count);
	oarchive& write(u32 const values[], streamsize count);
	oarchive& write(i32 const values[], streamsize count);
	oarchive& write(u64 const values[], streamsize count);
	oarchive& write(i64 const values[], streamsize count);
	oarchive& write(streamref const values[], streamsize count);
	template<typename T>
	oarchive& write(std::vector<T> const& values);
};
oarchive& operator<<(oarchive& archive, u8 const& value);
oarchive& operator<<(oarchive& archive, i8 const& value);
oarchive& operator<<(oarchive& archive, u16 const& value);
oarchive& operator<<(oarchive& archive, i16 const& value);
oarchive& operator<<(oarchive& archive, u32 const& value);
oarchive& operator<<(oarchive& archive, i32 const& value);
oarchive& operator<<(oarchive& archive, u64 const& value);
oarchive& operator<<(oarchive& archive, i64 const& value);
oarchive& operator<<(oarchive& archive, archive::streamref const& value);
template<std::size_t N>
oarchive& operator<<(oarchive& archive, u8 const(& values)[N]);
template<std::size_t N>
oarchive& operator<<(oarchive& archive, i8 const(& values)[N]);
template<std::size_t N>
oarchive& operator<<(oarchive& archive, u16 const(& values)[N]);
template<std::size_t N>
oarchive& operator<<(oarchive& archive, i16 const(& values)[N]);
template<std::size_t N>
oarchive& operator<<(oarchive& archive, u32 const(& values)[N]);
template<std::size_t N>
oarchive& operator<<(oarchive& archive, i32 const(& values)[N]);
template<std::size_t N>
oarchive& operator<<(oarchive& archive, u64 const(& values)[N]);
template<std::size_t N>
oarchive& operator<<(oarchive& archive, i64 const(& values)[N]);
template<std::size_t N>
oarchive& operator<<(oarchive& archive, archive::streamref const(& values)[N]);
template<typename T>
oarchive& operator<<(oarchive& archive, std::vector<T> const& values);

template<typename T>
class oarchive_ref : public oarchive {
	oarchive_ref(oarchive_ref const&) GENOME_DELETE_FUNCTION;
	oarchive_ref& operator=(oarchive_ref const&) GENOME_DELETE_FUNCTION;
public:
	oarchive_ref(T& stream, byte_order endianness);
	oarchive_ref(T& stream, platform target);
	virtual bool operator_bool(void) const GENOME_OVERRIDE;
	virtual bool operator!(void) const GENOME_OVERRIDE;
	virtual std::ios_base::iostate rdstate(void) const GENOME_OVERRIDE;
	virtual void setstate(std::ios_base::iostate state) GENOME_OVERRIDE;
	virtual void clear(std::ios_base::iostate state = std::ios_base::goodbit) GENOME_OVERRIDE;
	virtual std::ios_base::iostate exceptions(void) const GENOME_OVERRIDE;
	virtual void exceptions(std::ios_base::iostate except) GENOME_OVERRIDE;
	virtual byte_order endianness(void) const GENOME_OVERRIDE;
	virtual streampos tellp(void) const GENOME_OVERRIDE;
	virtual oarchive_ref& seekp(streampos pos) GENOME_OVERRIDE;
	virtual oarchive_ref& write_octets(u8 const* values, streamsize count) GENOME_OVERRIDE;
	T& streamp(void);
private:
	T& m_stream;
	byte_order m_endianness;
};
typedef oarchive_ref<std::ostream> osarchive;

template<typename T>
class oarchive_obj : public oarchive {
	oarchive_obj(oarchive_obj const&) GENOME_DELETE_FUNCTION;
	oarchive_obj& operator=(oarchive_obj const&) GENOME_DELETE_FUNCTION;
public:
	oarchive_obj(char const* filename, byte_order endianness);
	oarchive_obj(char const* filename, platform target);
	virtual bool operator_bool(void) const GENOME_OVERRIDE;
	virtual bool operator!(void) const GENOME_OVERRIDE;
	virtual std::ios_base::iostate rdstate(void) const GENOME_OVERRIDE;
	virtual void setstate(std::ios_base::iostate state) GENOME_OVERRIDE;
	virtual void clear(std::ios_base::iostate state = std::ios_base::goodbit) GENOME_OVERRIDE;
	virtual std::ios_base::iostate exceptions(void) const GENOME_OVERRIDE;
	virtual void exceptions(std::ios_base::iostate except) GENOME_OVERRIDE;
	virtual byte_order endianness(void) const GENOME_OVERRIDE;
	virtual streampos tellp(void) const GENOME_OVERRIDE;
	virtual oarchive_obj& seekp(streampos pos) GENOME_OVERRIDE;
	virtual oarchive_obj& write_octets(u8 const* values, streamsize count) GENOME_OVERRIDE;
protected:
	T m_stream;
	oarchive_ref<T> m_archive;
};
typedef oarchive_obj<std::ofstream> ofarchive;

class onarchive : public oarchive {
public:
	onarchive(void);
	virtual bool operator_bool(void) const GENOME_OVERRIDE;
	virtual bool operator!(void) const GENOME_OVERRIDE;
	virtual std::ios_base::iostate rdstate(void) const GENOME_OVERRIDE;
	virtual void setstate(std::ios_base::iostate state) GENOME_OVERRIDE;
	virtual void clear(std::ios_base::iostate state = std::ios_base::goodbit) GENOME_OVERRIDE;
	virtual std::ios_base::iostate exceptions(void) const GENOME_OVERRIDE;
	virtual void exceptions(std::ios_base::iostate except) GENOME_OVERRIDE;
	virtual byte_order endianness(void) const GENOME_OVERRIDE;
	virtual streampos tellp(void) const GENOME_OVERRIDE;
	virtual onarchive& seekp(streampos pos) GENOME_OVERRIDE;
	virtual onarchive& write_octets(u8 const* values, streamsize count) GENOME_OVERRIDE;
	streamref ref_begin(void) const;
	void ref_end(streamref& ref) const;
private:
	std::ios_base::iostate m_except;
	std::ios_base::iostate m_state;
	streampos m_pos;
};

} // namespace genome

#include <genome/archive.ipp>

#endif // GENOME_ARCHIVE_HPP
