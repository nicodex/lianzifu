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
#ifndef GENOME_TSTREAM_HPP
#define GENOME_TSTREAM_HPP

#include <genome/genome.hpp>
#include <genome/string.hpp>
#include <fstream>
#include <iostream>

namespace genome {

class tstream {
	void does_not_support_comparisons(void) const;
public:
	typedef void (tstream::*bool_type)(void) const;
	operator bool_type(void) const;
	operator bool_type(void);
	enum encoding {
		encoding_unknown = -1,
		encoding_genome,
		encoding_utf8,
		encoding_utf16le,
		encoding_utf16be
	};
	enum newline {
		newline_windows,
		newline_unix
	};
	virtual ~tstream(void);
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
	virtual encoding getenc(void) const = 0;
};
template<typename T>
bool operator!=(tstream const& lhs, T const& rhs);
template<typename T>
bool operator==(tstream const& lhs, T const& rhs);

template<typename StringT>
class itstream : public tstream {  // virtual inheritance if iotstream is introduced
public:
	typedef StringT string_type;
	typedef typename StringT::traits_type traits_type;
	typedef typename traits_type::char_type char_type;
	virtual itstream& getline(string_type& str) = 0;
};
template<typename StringT>
itstream<StringT>& operator>>(itstream<StringT>& stream, typename itstream<StringT>::string_type& str);

template<typename StringT, typename StreamT>
class itstream_ref : public itstream<StringT> {
	itstream_ref(itstream_ref const&) GENOME_DELETE_FUNCTION;
	itstream_ref& operator=(itstream_ref const&) GENOME_DELETE_FUNCTION;
	typedef itstream<StringT> base_type;
public:
	typedef typename base_type::string_type string_type;
	typedef typename base_type::traits_type traits_type;
	typedef typename base_type::char_type char_type;
	explicit itstream_ref(StreamT& stream, tstream::encoding enc = tstream::encoding_unknown, bool utf = false);
	virtual bool operator_bool(void) const GENOME_OVERRIDE;
	virtual bool operator!(void) const GENOME_OVERRIDE;
	virtual std::ios_base::iostate rdstate(void) const GENOME_OVERRIDE;
	virtual void setstate(std::ios_base::iostate state) GENOME_OVERRIDE;
	virtual void clear(std::ios_base::iostate state = std::ios_base::goodbit) GENOME_OVERRIDE;
	virtual std::ios_base::iostate exceptions(void) const GENOME_OVERRIDE;
	virtual void exceptions(std::ios_base::iostate except) GENOME_OVERRIDE;
	virtual tstream::encoding getenc(void) const GENOME_OVERRIDE;
	virtual itstream<StringT>& getline(string_type& str) GENOME_OVERRIDE;
private:
	StreamT& m_stream;
	tstream::encoding m_encoding;
};

template<typename StringT, typename StreamT>
class itstream_obj : public itstream<StringT> {
	itstream_obj(itstream_obj const&) GENOME_DELETE_FUNCTION;
	itstream_obj& operator=(itstream_obj const&) GENOME_DELETE_FUNCTION;
	typedef itstream<StringT> base_type;
public:
	typedef typename base_type::string_type string_type;
	typedef typename base_type::traits_type traits_type;
	typedef typename base_type::char_type char_type;
	explicit itstream_obj(char const* filename, tstream::encoding enc = tstream::encoding_unknown, bool utf = false);
	virtual bool operator_bool(void) const GENOME_OVERRIDE;
	virtual bool operator!(void) const GENOME_OVERRIDE;
	virtual std::ios_base::iostate rdstate(void) const GENOME_OVERRIDE;
	virtual void setstate(std::ios_base::iostate state) GENOME_OVERRIDE;
	virtual void clear(std::ios_base::iostate state = std::ios_base::goodbit) GENOME_OVERRIDE;
	virtual std::ios_base::iostate exceptions(void) const GENOME_OVERRIDE;
	virtual void exceptions(std::ios_base::iostate except) GENOME_OVERRIDE;
	virtual tstream::encoding getenc(void) const GENOME_OVERRIDE;
	virtual itstream<StringT>& getline(string_type& str) GENOME_OVERRIDE;
private:
	StreamT m_object;
	itstream_ref<StringT, StreamT> m_stream;
};
typedef itstream_obj<std::wstring, std::wifstream> witfstream;
typedef itstream_obj<wide_string, std::basic_ifstream<wide_string::traits_type::char_type, wide_string::traits_type> > u16itfstream;

template<typename StringT>
class otstream : public tstream {  // virtual inheritance if iotstream is introduced
public:
	typedef StringT string_type;
	typedef typename StringT::traits_type traits_type;
	typedef typename traits_type::char_type char_type;
	virtual newline geteol(void) const = 0;
	virtual otstream& putline(string_type const& str) = 0;
};

template<typename StringT, typename StreamT>
class otstream_ref : public otstream<StringT> {
	otstream_ref(otstream_ref const&) GENOME_DELETE_FUNCTION;
	otstream_ref& operator=(otstream_ref const&) GENOME_DELETE_FUNCTION;
	typedef otstream<StringT> base_type;
public:
	typedef typename base_type::string_type string_type;
	typedef typename base_type::traits_type traits_type;
	typedef typename base_type::char_type char_type;
	explicit otstream_ref(StreamT& stream, tstream::encoding enc = tstream::encoding_genome, tstream::newline eol = tstream::newline_windows);
	virtual bool operator_bool(void) const GENOME_OVERRIDE;
	virtual bool operator!(void) const GENOME_OVERRIDE;
	virtual std::ios_base::iostate rdstate(void) const GENOME_OVERRIDE;
	virtual void setstate(std::ios_base::iostate state) GENOME_OVERRIDE;
	virtual void clear(std::ios_base::iostate state = std::ios_base::goodbit) GENOME_OVERRIDE;
	virtual std::ios_base::iostate exceptions(void) const GENOME_OVERRIDE;
	virtual void exceptions(std::ios_base::iostate except) GENOME_OVERRIDE;
	virtual tstream::encoding getenc(void) const GENOME_OVERRIDE;
	virtual tstream::newline geteol(void) const GENOME_OVERRIDE;
	virtual otstream<StringT>& putline(string_type const& str) GENOME_OVERRIDE;
private:
	StreamT& m_stream;
	tstream::encoding m_encoding;
	tstream::newline m_newline;
};

template<typename StringT, typename StreamT>
class otstream_obj : public otstream<StringT> {
	otstream_obj(otstream_obj const&) GENOME_DELETE_FUNCTION;
	otstream_obj& operator=(otstream_obj const&) GENOME_DELETE_FUNCTION;
	typedef otstream<StringT> base_type;
public:
	typedef typename base_type::string_type string_type;
	typedef typename base_type::traits_type traits_type;
	typedef typename base_type::char_type char_type;
	explicit otstream_obj(char const* filename, tstream::encoding enc = tstream::encoding_genome, tstream::newline eol = tstream::newline_windows);
	virtual bool operator_bool(void) const GENOME_OVERRIDE;
	virtual bool operator!(void) const GENOME_OVERRIDE;
	virtual std::ios_base::iostate rdstate(void) const GENOME_OVERRIDE;
	virtual void setstate(std::ios_base::iostate state) GENOME_OVERRIDE;
	virtual void clear(std::ios_base::iostate state = std::ios_base::goodbit) GENOME_OVERRIDE;
	virtual std::ios_base::iostate exceptions(void) const GENOME_OVERRIDE;
	virtual void exceptions(std::ios_base::iostate except) GENOME_OVERRIDE;
	virtual tstream::encoding getenc(void) const GENOME_OVERRIDE;
	virtual tstream::newline geteol(void) const GENOME_OVERRIDE;
	virtual otstream<StringT>& putline(string_type const& str) GENOME_OVERRIDE;
private:
	StreamT m_object;
	otstream_ref<StringT, StreamT> m_stream;
};
typedef otstream_obj<std::wstring, std::wofstream> wotfstream;
typedef otstream_obj<wide_string, std::basic_ofstream<wide_string::traits_type::char_type, wide_string::traits_type> > u16otfstream;

} // namespace genome

#include <genome/tstream.ipp>

#endif // GENOME_TSTREAM_HPP
