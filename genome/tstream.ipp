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
#ifndef GENOME_TSTREAM_IPP
#define GENOME_TSTREAM_IPP

#include <genome/filesystem.hpp>
#include <genome/locale.hpp>
#include <climits>
#include <cwchar>
#include <locale>
#include <streambuf>
#include <string>

namespace genome {

//
// operator bool(tstream, ...)
//

template<typename T>
bool
operator!=(tstream const& lhs, T const&)
{
	lhs.does_not_support_comparisons();
	return (false);
}

template<typename T>
bool
operator==(tstream const& lhs, T const&)
{
	lhs.does_not_support_comparisons();
	return (false);
}

//
// itstream_ref
//

template<typename StringT, typename StreamT>
itstream_ref<StringT, StreamT>::itstream_ref(StreamT& stream, tstream::encoding enc, bool utf)
	: m_stream(stream)
	, m_encoding(enc)
{
	if (tstream::encoding_unknown == m_encoding) {
		// default to Windows-1252
		m_encoding = tstream::encoding_genome;
		if (m_stream) {
			// disable exceptions and backup state/pos for BOM check
			std::ios_base::iostate const except = m_stream.exceptions();
			m_stream.exceptions(std::ios_base::goodbit);
			std::ios_base::iostate const state = m_stream.rdstate();
			typename StreamT::pos_type const pos = m_stream.tellg();
			if (pos != typename StreamT::pos_type(typename StreamT::off_type(-1))) {
				char_type bom[2];
				std::locale const loc = m_stream.imbue(std::locale(m_stream.getloc(), new codecvt_genome<char_type, char, std::mbstate_t>()));
				if (m_stream.read(bom, sizeof(bom) / sizeof(bom[0]))) {
					if ((0xEF == bom[0]) && (0xBB == bom[1]) && (0xBF == m_stream.peek())) {
						m_encoding = tstream::encoding_utf8;
					} else if ((0xFF == bom[0]) && (0xFE == bom[1]) && (m_stream.peek() != 0x00)) {
						m_encoding = tstream::encoding_utf16le;
					} else if ((0xFE == bom[0]) && (0xFF == bom[1])) {
						m_encoding = tstream::encoding_utf16be;
					} else if (utf) {
						if ((0x00 == bom[0]) && (bom[1] != 0x00)) {
							m_encoding = tstream::encoding_utf16be;
						} else if ((bom[0] != 0x00) && (0x00 == bom[1])) {
							m_encoding = tstream::encoding_utf16le;
						} else if ((bom[0] != 0x00) && (bom[1] != 0x00)) {
							m_encoding = tstream::encoding_utf8;
						}
					}
				}
				m_stream.imbue(loc);
				m_stream.clear();
				m_stream.seekg(pos);
			}
			m_stream.clear(state);
			m_stream.exceptions(except);
		}
	}
	switch (m_encoding) {
	default:
	case tstream::encoding_unknown:
	case tstream::encoding_genome:
		m_stream.imbue(std::locale(m_stream.getloc(), new codecvt_genome<char_type, char, std::mbstate_t>()));
		break;
	case tstream::encoding_utf8:
		if (sizeof(char_type) * CHAR_BIT >= 21) {
			m_stream.imbue(std::locale(m_stream.getloc(), new codecvt_utf8<char_type, 0x0010FFFFUL, codecvt::mode(codecvt::generate_header | codecvt::consume_header)>()));
		} else {
			m_stream.imbue(std::locale(m_stream.getloc(), new codecvt_utf8_utf16<char_type, 0x0010FFFFUL, codecvt::mode(codecvt::generate_header | codecvt::consume_header)>()));
		}
		break;
	case tstream::encoding_utf16le:
		m_stream.imbue(std::locale(m_stream.getloc(), new codecvt_utf16<char_type, 0x0010FFFFUL, codecvt::mode(codecvt::little_endian | codecvt::generate_header | codecvt::consume_header)>()));
		break;
	case tstream::encoding_utf16be:
		m_stream.imbue(std::locale(m_stream.getloc(), new codecvt_utf16<char_type, 0x0010FFFFUL, codecvt::mode(codecvt::big_endian | codecvt::generate_header | codecvt::consume_header)>()));
		break;
	}
}

template<typename StringT, typename StreamT>
bool
itstream_ref<StringT, StreamT>::operator_bool(void) const
{
	return (!m_stream.fail());
}

template<typename StringT, typename StreamT>
bool
itstream_ref<StringT, StreamT>::operator!(void) const
{
	return (m_stream.fail());
}

template<typename StringT, typename StreamT>
std::ios_base::iostate
itstream_ref<StringT, StreamT>::rdstate(void) const
{
	return (m_stream.rdstate());
}

template<typename StringT, typename StreamT>
void
itstream_ref<StringT, StreamT>::setstate(std::ios_base::iostate state)
{
	m_stream.setstate(state);
}

template<typename StringT, typename StreamT>
void
itstream_ref<StringT, StreamT>::clear(std::ios_base::iostate state)
{
	m_stream.clear(state);
}

template<typename StringT, typename StreamT>
std::ios_base::iostate
itstream_ref<StringT, StreamT>::exceptions(void) const
{
	return (m_stream.exceptions());
}

template<typename StringT, typename StreamT>
void
itstream_ref<StringT, StreamT>::exceptions(std::ios_base::iostate except)
{
	m_stream.exceptions(except);
}

template<typename StringT, typename StreamT>
tstream::encoding
itstream_ref<StringT, StreamT>::getenc(void) const
{
	return (m_encoding);
}

template<typename StringT, typename StreamT>
itstream<StringT>&
itstream_ref<StringT, StreamT>::getline(string_type& str)
{
	str.erase();
	if (!m_stream.good()) {
		setstate(std::ios_base::failbit);
	} else {
		typedef std::basic_streambuf<char_type, traits_type> streambuf;
		typename StreamT::sentry se(m_stream, true);
		streambuf* buf = m_stream.rdbuf();
		for (;;) {
			typename streambuf::pos_type const pos = buf->pubseekoff(0, std::ios_base::cur, std::ios_base::in);
			typename streambuf::int_type const chr = buf->sbumpc();
			switch (chr) {
			case 0x00:  // NUL is not accepted (most likely wrong encoding)
				setstate(std::ios_base::failbit);
				return (*this);
			case 0x0A:  // LF is our newline character
				//NOTE: if the last line ends with newline, EOF is set after reading the next (empty) line
				return (*this);
			case 0x0D:  // CR is always dropped (even if not followed by LF)
				break;
			default:
				if (traits_type::eof() == chr) {
					// assume last line without newline
					std::ios_base::iostate state = std::ios_base::eofbit;
					// try to read with non-converting locale to detect encoding errors
					if ((m_encoding != tstream::encoding_genome) && (pos != typename streambuf::pos_type(typename streambuf::off_type(-1)))) {
						buf->pubseekpos(pos, std::ios_base::in);
						std::locale const loc = buf->pubimbue(std::locale(buf->getloc(), new codecvt_genome<char_type, char, std::mbstate_t>()));
						typename streambuf::int_type const u8c = buf->sbumpc();
						buf->pubimbue(loc);
						if (u8c != traits_type::eof()) {
							state = std::ios_base::failbit;
						}
					}
					setstate(state);
					return (*this);
				}
				str += traits_type::to_char_type(chr);
				break;
			}
		}
	}
	return (*this);
}

//
// itstream_obj
//

template<typename StringT, typename StreamT>
itstream_obj<StringT, StreamT>::itstream_obj(char const* filename, tstream::encoding enc, bool utf)
	: m_object(filesystem::system_complete(filename).c_str(), std::ios_base::in | std::ios_base::binary)
	, m_stream(m_object, enc, utf)
{
}

template<typename StringT, typename StreamT>
bool
itstream_obj<StringT, StreamT>::operator_bool(void) const
{
	return (m_stream.operator_bool());
}

template<typename StringT, typename StreamT>
bool
itstream_obj<StringT, StreamT>::operator!(void) const
{
	return (m_stream.operator!());
}

template<typename StringT, typename StreamT>
std::ios_base::iostate
itstream_obj<StringT, StreamT>::rdstate(void) const
{
	return (m_stream.rdstate());
}

template<typename StringT, typename StreamT>
void
itstream_obj<StringT, StreamT>::setstate(std::ios_base::iostate state)
{
	m_stream.setstate(state);
}

template<typename StringT, typename StreamT>
void
itstream_obj<StringT, StreamT>::clear(std::ios_base::iostate state)
{
	m_stream.clear(state);
}

template<typename StringT, typename StreamT>
std::ios_base::iostate
itstream_obj<StringT, StreamT>::exceptions(void) const
{
	return (m_stream.exceptions());
}

template<typename StringT, typename StreamT>
void
itstream_obj<StringT, StreamT>::exceptions(std::ios_base::iostate except)
{
	m_stream.exceptions(except);
}

template<typename StringT, typename StreamT>
tstream::encoding
itstream_obj<StringT, StreamT>::getenc(void) const
{
	return (m_stream.getenc());
}

template<typename StringT, typename StreamT>
itstream<StringT>&
itstream_obj<StringT, StreamT>::getline(string_type& str)
{
	return (m_stream.getline(str));
}

//
// otstream_ref
//

template<typename StringT, typename StreamT>
otstream_ref<StringT, StreamT>::otstream_ref(StreamT& stream, tstream::encoding enc, tstream::newline eol)
	: m_stream(stream)
	, m_encoding(enc)
	, m_newline(eol)
{
	switch (m_encoding) {
	default:
	case tstream::encoding_unknown:
	case tstream::encoding_genome:
		m_stream.imbue(std::locale(m_stream.getloc(), new codecvt_genome<char_type, char, std::mbstate_t>()));
		break;
	case tstream::encoding_utf8:
		if (sizeof(char_type) * CHAR_BIT >= 21) {
			m_stream.imbue(std::locale(m_stream.getloc(), new codecvt_utf8<char_type, 0x0010FFFFUL, codecvt::mode(codecvt::generate_header | codecvt::consume_header)>()));
		} else {
			m_stream.imbue(std::locale(m_stream.getloc(), new codecvt_utf8_utf16<char_type, 0x0010FFFFUL, codecvt::mode(codecvt::generate_header | codecvt::consume_header)>()));
		}
		break;
	case tstream::encoding_utf16le:
		m_stream.imbue(std::locale(m_stream.getloc(), new codecvt_utf16<char_type, 0x0010FFFFUL, codecvt::mode(codecvt::little_endian | codecvt::generate_header | codecvt::consume_header)>()));
		break;
	case tstream::encoding_utf16be:
		m_stream.imbue(std::locale(m_stream.getloc(), new codecvt_utf16<char_type, 0x0010FFFFUL, codecvt::mode(codecvt::big_endian | codecvt::generate_header | codecvt::consume_header)>()));
		break;
	}
}

template<typename StringT, typename StreamT>
bool
otstream_ref<StringT, StreamT>::operator_bool(void) const
{
	return (!m_stream.fail());
}

template<typename StringT, typename StreamT>
bool
otstream_ref<StringT, StreamT>::operator!(void) const
{
	return (m_stream.fail());
}

template<typename StringT, typename StreamT>
std::ios_base::iostate
otstream_ref<StringT, StreamT>::rdstate(void) const
{
	return (m_stream.rdstate());
}

template<typename StringT, typename StreamT>
void
otstream_ref<StringT, StreamT>::setstate(std::ios_base::iostate state)
{
	m_stream.setstate(state);
}

template<typename StringT, typename StreamT>
void
otstream_ref<StringT, StreamT>::clear(std::ios_base::iostate state)
{
	m_stream.clear(state);
}

template<typename StringT, typename StreamT>
std::ios_base::iostate
otstream_ref<StringT, StreamT>::exceptions(void) const
{
	return (m_stream.exceptions());
}

template<typename StringT, typename StreamT>
void
otstream_ref<StringT, StreamT>::exceptions(std::ios_base::iostate except)
{
	m_stream.exceptions(except);
}

template<typename StringT, typename StreamT>
tstream::encoding
otstream_ref<StringT, StreamT>::getenc(void) const
{
	return (m_encoding);
}

template<typename StringT, typename StreamT>
tstream::newline
otstream_ref<StringT, StreamT>::geteol(void) const
{
	return (m_newline);
}

template<typename StringT, typename StreamT>
otstream<StringT>&
otstream_ref<StringT, StreamT>::putline(string_type const& str)
{
	if (!m_stream.good()) {
		setstate(std::ios_base::failbit);
	} else {
		typedef std::basic_streambuf<char_type, traits_type> streambuf;
		typename StreamT::sentry se(m_stream);
		streambuf* buf = m_stream.rdbuf();
		for (typename string_type::const_iterator itr = str.begin(); itr != str.end(); ++itr) {
			char_type const& chr = *itr;
			switch (chr) {
			case 0x00:  // NUL is not accepted
				setstate(std::ios_base::failbit);
				return (*this);
			case 0x0A:  // LF is our newline character
				if (tstream::newline_windows == m_newline) {
					if (traits_type::eof() == buf->sputc(0x0D)) {
						setstate(std::ios_base::failbit);
						return (*this);
					}
				}
				break;
			case 0x0D:  // CR is always ignored (even if not followed by LF)
				continue;
			}
			if (traits_type::eof() == buf->sputc(chr)) {
				setstate(std::ios_base::failbit);
				return (*this);
			}
		}
		if (tstream::newline_windows == m_newline) {
			if (traits_type::eof() == buf->sputc(0x0D)) {
				setstate(std::ios_base::failbit);
				return (*this);
			}
		}
		if (traits_type::eof() == buf->sputc(0x0A)) {
			setstate(std::ios_base::failbit);
		}
	}
	return (*this);
}

//
// otstream_obj
//

template<typename StringT, typename StreamT>
otstream_obj<StringT, StreamT>::otstream_obj(char const* filename, tstream::encoding enc, tstream::newline eol)
	: m_object(filesystem::system_complete(filename).c_str(), std::ios_base::out | std::ios_base::binary)
	, m_stream(m_object, enc, eol)
{
}

template<typename StringT, typename StreamT>
bool
otstream_obj<StringT, StreamT>::operator_bool(void) const
{
	return (m_stream.operator_bool());
}

template<typename StringT, typename StreamT>
bool
otstream_obj<StringT, StreamT>::operator!(void) const
{
	return (m_stream.operator!());
}

template<typename StringT, typename StreamT>
std::ios_base::iostate
otstream_obj<StringT, StreamT>::rdstate(void) const
{
	return (m_stream.rdstate());
}

template<typename StringT, typename StreamT>
void
otstream_obj<StringT, StreamT>::setstate(std::ios_base::iostate state)
{
	m_stream.setstate(state);
}

template<typename StringT, typename StreamT>
void
otstream_obj<StringT, StreamT>::clear(std::ios_base::iostate state)
{
	m_stream.clear(state);
}

template<typename StringT, typename StreamT>
std::ios_base::iostate
otstream_obj<StringT, StreamT>::exceptions(void) const
{
	return (m_stream.exceptions());
}

template<typename StringT, typename StreamT>
void
otstream_obj<StringT, StreamT>::exceptions(std::ios_base::iostate except)
{
	m_stream.exceptions(except);
}

template<typename StringT, typename StreamT>
tstream::encoding
otstream_obj<StringT, StreamT>::getenc(void) const
{
	return (m_stream.getenc());
}

template<typename StringT, typename StreamT>
tstream::newline
otstream_obj<StringT, StreamT>::geteol(void) const
{
	return (m_stream.geteol());
}

template<typename StringT, typename StreamT>
otstream<StringT>&
otstream_obj<StringT, StreamT>::putline(string_type const& str)
{
	return (m_stream.putline(str));
}

} // namespace genome

#endif // GENOME_TSTREAM_IPP
