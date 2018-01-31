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
#ifndef GENOME_ARCHIVE_IPP
#define GENOME_ARCHIVE_IPP

#include <genome/archive_detail.ipp>

namespace genome {

//
// operator bool(archive, ...)
//

template<typename T>
bool
operator!=(archive const& lhs, T const&)
{
	lhs.does_not_support_comparisons();
	return (false);
}

template<typename T>
bool
operator==(archive const& lhs, T const&)
{
	lhs.does_not_support_comparisons();
	return (false);
}

//
// iarchive
//

template<typename T>
iarchive&
iarchive::read(std::vector<T>& values, streamsize count)
{
	values.clear();
	if (count > 0) {
		bool const overflow = (count > streamsize_limits<T>::max_count());
		if (overflow) {
			count = streamsize_limits<T>::max_count();
		}
		values.resize(static_cast<std::size_t>(count));
		read(&values[0], count);
		if (overflow && good()) {
			setstate(std::ios_base::failbit);
		}
	}
	return (*this);
}

template<std::size_t N>
iarchive&
operator>>(iarchive& archive, u8(& values)[N])
{
	return (archive.read(values, N));
}

template<std::size_t N>
iarchive&
operator>>(iarchive& archive, i8(& values)[N])
{
	return (operator>>(archive, reinterpret_cast<u8(&)[N]>(values)));
}

template<std::size_t N>
iarchive&
operator>>(iarchive& archive, u16(& values)[N])
{
	switch (archive.endianness()) {
	case archive::big_endian:
		if (target_integer_big_endian) {
			archive.read(reinterpret_cast<u8*>(&values[0]), sizeof(values));
		} else {
			u8 octets[N][sizeof(values[0])];
			archive.read(&octets[0][0], sizeof(octets));
			detail::read_big_endian(values, octets);
		}
		break;
	case archive::little_endian:
		if (target_integer_little_endian) {
			archive.read(reinterpret_cast<u8*>(&values[0]), sizeof(values));
		} else {
			u8 octets[N][sizeof(values[0])];
			archive.read(&octets[0][0], sizeof(octets));
			detail::read_little_endian(values, octets);
		}
		break;
	}
	return (archive);
}

template<std::size_t N>
iarchive&
operator>>(iarchive& archive, i16(& values)[N])
{
	return (operator>>(archive, reinterpret_cast<u16(&)[N]>(values)));
}

template<std::size_t N>
iarchive&
operator>>(iarchive& archive, u32(& values)[N])
{
	switch (archive.endianness()) {
	case archive::big_endian:
		if (target_integer_big_endian) {
			archive.read(reinterpret_cast<u8*>(&values[0]), sizeof(values));
		} else {
			u8 octets[N][sizeof(values[0])];
			archive.read(&octets[0][0], sizeof(octets));
			detail::read_big_endian(values, octets);
		}
		break;
	case archive::little_endian:
		if (target_integer_little_endian) {
			archive.read(reinterpret_cast<u8*>(&values[0]), sizeof(values));
		} else {
			u8 octets[N][sizeof(values[0])];
			archive.read(&octets[0][0], sizeof(octets));
			detail::read_little_endian(values, octets);
		}
		break;
	}
	return (archive);
}

template<std::size_t N>
iarchive&
operator>>(iarchive& archive, i32(& values)[N])
{
	return (operator>>(archive, reinterpret_cast<u32(&)[N]>(values)));
}

template<std::size_t N>
iarchive&
operator>>(iarchive& archive, u64(& values)[N])
{
	switch (archive.endianness()) {
	case archive::big_endian:
		if (target_integer_big_endian) {
			archive.read(reinterpret_cast<u8*>(&values[0]), sizeof(values));
		} else {
			u8 octets[N][sizeof(values[0])];
			archive.read(&octets[0][0], sizeof(octets));
			detail::read_big_endian(values, octets);
		}
		break;
	case archive::little_endian:
		if (target_integer_little_endian) {
			archive.read(reinterpret_cast<u8*>(&values[0]), sizeof(values));
		} else {
			u8 octets[N][sizeof(values[0])];
			archive.read(&octets[0][0], sizeof(octets));
			detail::read_little_endian(values, octets);
		}
		break;
	}
	return (archive);
}

template<std::size_t N>
iarchive&
operator>>(iarchive& archive, i64(& values)[N])
{
	return (operator>>(archive, reinterpret_cast<u64(&)[N]>(values)));
}

template<std::size_t N>
iarchive&
operator>>(iarchive& archive, archive::streamref(& values)[N])
{
	return (operator>>(archive, reinterpret_cast<u32(&)[N * 2]>(values)));
}

//
// iarchive_ref
//

template<typename T>
iarchive_ref<T>::iarchive_ref(T& stream)
	: m_stream(stream)
{
	m_endianness = header(*this).endianness();
}

template<typename T>
bool
iarchive_ref<T>::operator_bool(void) const
{
	return (!fail());
}

template<typename T>
bool
iarchive_ref<T>::operator!(void) const
{
	return (fail());
}

template<typename T>
std::ios_base::iostate
iarchive_ref<T>::rdstate(void) const
{
	return (m_stream.rdstate());
}

template<typename T>
void
iarchive_ref<T>::setstate(std::ios_base::iostate state)
{
	m_stream.setstate(state);
}

template<typename T>
void
iarchive_ref<T>::clear(std::ios_base::iostate state)
{
	m_stream.clear(state);
}

template<typename T>
std::ios_base::iostate
iarchive_ref<T>::exceptions(void) const
{
	return (m_stream.exceptions());
}

template<typename T>
void
iarchive_ref<T>::exceptions(std::ios_base::iostate except)
{
	m_stream.exceptions(except);
}

template<typename T>
archive::byte_order
iarchive_ref<T>::endianness(void) const
{
	return (m_endianness);
}

template<typename T>
archive::streampos
iarchive_ref<T>::tellg(void)
{
	typename T::pos_type const pos = m_stream.tellg();
	if (typename T::pos_type(typename T::off_type(-1)) == pos) {
		return (streampos(-1));
	}
	typename T::off_type const off = pos;
	if ((off < static_cast<typename T::off_type>(sizeof(header))) || (
	    (sizeof(typename T::off_type) > sizeof(streampos)) &&
	    (off > static_cast<typename T::off_type>(streamsize_limits<u8>::max_size())))) {
		m_stream.setstate(m_stream.failbit);
		return (streampos(-1));
	}
	return (static_cast<streampos>(off));
}

template<typename T>
iarchive_ref<T>&
iarchive_ref<T>::seekg(streampos pos)
{
	typename T::off_type const off = static_cast<typename T::off_type>(pos);
	if (off < static_cast<typename T::off_type>(sizeof(header))) {
		m_stream.setstate(m_stream.failbit);
	} else {
		m_stream.seekg(typename T::pos_type(off));
	}
	return (*this);
}

template<typename T>
iarchive_ref<T>&
iarchive_ref<T>::read_octets(u8* values, streamsize count)
{
	if (count > 0) {
		if (target_char_8bit) {
			if (!m_stream.read(reinterpret_cast<typename T::char_type *>(values), static_cast<std::streamsize>(count))) {
				for (streamsize i = gcount(); i < count; ++i) {
					values[i] = 0;
				}
			}
		} else {
			std::vector<typename T::char_type> bytes(static_cast<std::size_t>(count));
			if (!m_stream.read(&bytes[0], static_cast<std::streamsize>(count))) {
				streamsize end = gcount();
				for (streamsize i = end; i < count; ++i) {
					values[i] = 0;
				}
				count = end;
			}
			for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
				values[i] = static_cast<u8>(bytes[i] & 0xFF);
			}
		}
	}
	return (*this);
}

template<typename T>
archive::streamsize
iarchive_ref<T>::gcount(void) const
{
	std::streamsize size = m_stream.gcount();
	if (size < 0) {
		return (0);
	}
	return (static_cast<streamsize>(size));
}

template<typename T>
T&
iarchive_ref<T>::streamg(void)
{
	return (m_stream);
}

//
// iarchive_obj
//

template<typename T>
iarchive_obj<T>::iarchive_obj(char const* filename)
	: m_stream(filesystem::system_complete(filename).c_str(), std::ios_base::in | std::ios_base::binary)
	, m_archive(m_stream)
{
}

template<typename T>
bool
iarchive_obj<T>::operator_bool(void) const
{
	return (m_archive.operator_bool());
}

template<typename T>
bool
iarchive_obj<T>::operator!(void) const
{
	return (m_archive.operator!());
}

template<typename T>
std::ios_base::iostate
iarchive_obj<T>::rdstate(void) const
{
	return (m_archive.rdstate());
}

template<typename T>
void
iarchive_obj<T>::setstate(std::ios_base::iostate state)
{
	m_archive.setstate(state);
}

template<typename T>
void
iarchive_obj<T>::clear(std::ios_base::iostate state)
{
	m_archive.clear(state);
}

template<typename T>
std::ios_base::iostate
iarchive_obj<T>::exceptions(void) const
{
	return (m_archive.exceptions());
}

template<typename T>
void
iarchive_obj<T>::exceptions(std::ios_base::iostate except)
{
	m_archive.exceptions(except);
}

template<typename T>
archive::byte_order
iarchive_obj<T>::endianness(void) const
{
	return (m_archive.endianness());
}

template<typename T>
archive::streampos
iarchive_obj<T>::tellg(void)
{
	return (m_archive.tellg());
}

template<typename T>
iarchive_obj<T>&
iarchive_obj<T>::seekg(streampos pos)
{
	m_archive.seekg(pos);
	return (*this);
}

template<typename T>
iarchive_obj<T>&
iarchive_obj<T>::read_octets(u8* values, streamsize count)
{
	m_archive.read(values, count);
	return (*this);
}

template<typename T>
archive::streamsize
iarchive_obj<T>::gcount(void) const
{
	return (m_archive.gcount());
}

//
// oarchive
//

template<typename T>
oarchive&
oarchive::write(std::vector<T> const& values)
{
	if (!values.empty()) {
		bool const overflow = (values.size() > streamsize_limits<T>::max_count());
		streamsize const count = overflow
			? static_cast<streamsize>(streamsize_limits<T>::max_count())
			: static_cast<streamsize>(values.size());
		write(&values[0], count);
		if (overflow && good()) {
			setstate(std::ios_base::failbit);
		}
	}
	return (*this);
}

//
// operator<<(oarchive, ...)
//

template<std::size_t N>
oarchive&
operator<<(oarchive& archive, u8 const(& values)[N])
{
	return (archive.write(values, N));
}

template<std::size_t N>
oarchive&
operator<<(oarchive& archive, i8 const(& values)[N])
{
	return (operator<<(archive, reinterpret_cast<u8 const(&)[N]>(values)));
}

template<std::size_t N>
oarchive&
operator<<(oarchive& archive, u16 const(& values)[N])
{
	switch (archive.endianness()) {
	case archive::big_endian:
		if (target_integer_big_endian) {
			archive.write(reinterpret_cast<u8 const*>(&values[0]), sizeof(values));
		} else {
			u8 octets[N][sizeof(values[0])];
			detail::write_big_endian(values, octets);
			archive.write(&octets[0][0], sizeof(octets));
		}
		break;
	case archive::little_endian:
		if (target_integer_little_endian) {
			archive.write(reinterpret_cast<u8 const*>(&values[0]), sizeof(values));
		} else {
			u8 octets[N][sizeof(values[0])];
			detail::write_little_endian(values, octets);
			archive.write(&octets[0][0], sizeof(octets));
		}
		break;
	}
	return (archive);
}

template<std::size_t N>
oarchive&
operator<<(oarchive& archive, i16 const(& values)[N])
{
	return (operator<<(archive, reinterpret_cast<u16 const(&)[N]>(values)));
}

template<std::size_t N>
oarchive&
operator<<(oarchive& archive, u32 const(& values)[N])
{
	switch (archive.endianness()) {
	case archive::big_endian:
		if (target_integer_big_endian) {
			archive.write(reinterpret_cast<u8 const*>(&values[0]), sizeof(values));
		} else {
			u8 octets[N][sizeof(values[0])];
			detail::write_big_endian(values, octets);
			archive.write(&octets[0][0], sizeof(octets));
		}
		break;
	case archive::little_endian:
		if (target_integer_little_endian) {
			archive.write(reinterpret_cast<u8 const*>(&values[0]), sizeof(values));
		} else {
			u8 octets[N][sizeof(values[0])];
			detail::write_little_endian(values, octets);
			archive.write(&octets[0][0], sizeof(octets));
		}
		break;
	}
	return (archive);
}

template<std::size_t N>
oarchive&
operator<<(oarchive& archive, i32 const(& values)[N])
{
	return (operator<<(archive, reinterpret_cast<u32 const(&)[N]>(values)));
}

template<std::size_t N>
oarchive&
operator<<(oarchive& archive, u64 const(& values)[N])
{
	switch (archive.endianness()) {
	case archive::big_endian:
		if (target_integer_big_endian) {
			archive.write(reinterpret_cast<u8 const*>(&values[0]), sizeof(values));
		} else {
			u8 octets[N][sizeof(values[0])];
			detail::write_big_endian(values, octets);
			archive.write(&octets[0][0], sizeof(octets));
		}
		break;
	case archive::little_endian:
		if (target_integer_little_endian) {
			archive.write(reinterpret_cast<u8 const*>(&values[0]), sizeof(values));
		} else {
			u8 octets[N][sizeof(values[0])];
			detail::write_little_endian(values, octets);
			archive.write(&octets[0][0], sizeof(octets));
		}
		break;
	}
	return (archive);
}

template<std::size_t N>
oarchive&
operator<<(oarchive& archive, i64 const(& values)[N])
{
	return (operator<<(archive, reinterpret_cast<u64 const(&)[N]>(values)));
}

template<std::size_t N>
oarchive&
operator<<(oarchive& archive, archive::streamref const(& values)[N])
{
	return (operator<<(archive, reinterpret_cast<u32 const(&)[N * 2]>(values)));
}

template<typename T>
oarchive&
operator<<(oarchive& archive, std::vector<T> const& values)
{
	return (archive.write(values));
}

//
// oarchive_ref
//

template<typename T>
oarchive_ref<T>::oarchive_ref(T& stream, byte_order endianness)
	: m_stream(stream)
	, m_endianness(endianness)
{
	header(endianness).write(*this);
}

template<typename T>
oarchive_ref<T>::oarchive_ref(T& stream, platform target)
	: m_stream(stream)
	, m_endianness(platform_endianess(target))
{
	header(endianness()).write(*this);
}

template<typename T>
bool
oarchive_ref<T>::operator_bool(void) const
{
	return (!fail());
}

template<typename T>
bool
oarchive_ref<T>::operator!(void) const
{
	return (fail());
}

template<typename T>
std::ios_base::iostate
oarchive_ref<T>::rdstate(void) const
{
	return (m_stream.rdstate());
}

template<typename T>
void
oarchive_ref<T>::setstate(std::ios_base::iostate state)
{
	m_stream.setstate(state);
}

template<typename T>
void
oarchive_ref<T>::clear(std::ios_base::iostate state)
{
	m_stream.clear(state);
}

template<typename T>
std::ios_base::iostate
oarchive_ref<T>::exceptions(void) const
{
	return (m_stream.exceptions());
}

template<typename T>
void
oarchive_ref<T>::exceptions(std::ios_base::iostate except)
{
	m_stream.exceptions(except);
}

template<typename T>
archive::byte_order
oarchive_ref<T>::endianness(void) const
{
	return (m_endianness);
}

template<typename T>
archive::streampos
oarchive_ref<T>::tellp(void) const
{
	typename T::pos_type const pos = m_stream.tellp();
	if (typename T::pos_type(typename T::off_type(-1)) == pos) {
		return (streampos(-1));
	}
	typename T::off_type const off = pos;
	if ((off < static_cast<typename T::off_type>(sizeof(header))) || (
	    (sizeof(typename T::off_type) > sizeof(streampos)) &&
	    (off > static_cast<typename T::off_type>(streamsize_limits<u8>::max_size())))) {
		m_stream.setstate(m_stream.failbit);
		return (streampos(-1));
	}
	return (static_cast<streampos>(off));
}

template<typename T>
oarchive_ref<T>&
oarchive_ref<T>::seekp(streampos pos)
{
	typename T::off_type const off = static_cast<typename T::off_type>(pos);
	if (off < static_cast<typename T::off_type>(sizeof(header))) {
		m_stream.setstate(m_stream.failbit);
	} else {
		m_stream.seekp(typename T::pos_type(off));
	}
	return (*this);
}

template<typename T>
oarchive_ref<T>&
oarchive_ref<T>::write_octets(u8 const* values, streamsize count)
{
	if (count > 0) {
		if (target_char_8bit) {
			m_stream.write(reinterpret_cast<typename T::char_type const*>(values), static_cast<std::streamsize>(count));
		} else {
			std::vector<typename T::char_type> bytes(static_cast<std::size_t>(count));
			for (std::size_t i = 0; i < static_cast<std::size_t>(count); ++i) {
				bytes[i] = static_cast<typename T::char_type>(static_cast<unsigned char>(values[i]));
			}
			m_stream.write(&bytes[0], static_cast<std::streamsize>(count));
		}
	}
	return (*this);
}

template<typename T>
T&
oarchive_ref<T>::streamp(void)
{
	return (m_stream);
}

//
// oarchive_obj
//

template<typename T>
oarchive_obj<T>::oarchive_obj(char const* filename, byte_order endianness)
	: m_stream(filesystem::system_complete(filename).c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc)
	, m_archive(m_stream, endianness)
{
}

template<typename T>
oarchive_obj<T>::oarchive_obj(char const* filename, platform target)
	: m_stream(filesystem::system_complete(filename).c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc)
	, m_archive(m_stream, target)
{
}

template<typename T>
bool
oarchive_obj<T>::operator_bool(void) const
{
	return (m_archive.operator_bool());
}

template<typename T>
bool
oarchive_obj<T>::operator!(void) const
{
	return (m_archive.operator!());
}

template<typename T>
std::ios_base::iostate
oarchive_obj<T>::rdstate(void) const
{
	return (m_archive.rdstate());
}

template<typename T>
void
oarchive_obj<T>::setstate(std::ios_base::iostate state)
{
	m_archive.setstate(state);
}

template<typename T>
void
oarchive_obj<T>::clear(std::ios_base::iostate state)
{
	m_archive.clear(state);
}

template<typename T>
std::ios_base::iostate
oarchive_obj<T>::exceptions(void) const
{
	return(m_archive.exceptions());
}

template<typename T>
void
oarchive_obj<T>::exceptions(std::ios_base::iostate except)
{
	m_archive.exceptions(except);
}

template<typename T>
archive::byte_order
oarchive_obj<T>::endianness(void) const
{
	return (m_archive.endianness());
}

template<typename T>
archive::streampos
oarchive_obj<T>::tellp(void) const
{
	return (m_archive.tellp());
}

template<typename T>
oarchive_obj<T>&
oarchive_obj<T>::seekp(streampos pos)
{
	m_archive.seekp(pos);
	return (*this);
}

template<typename T>
oarchive_obj<T>&
oarchive_obj<T>::write_octets(u8 const* values, streamsize count)
{
	m_archive.write(values, count);
	return (*this);
}

} // namespace genome

#endif // GENOME_ARCHIVE_IPP
