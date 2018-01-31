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
#ifndef GENOME_LOCALIZATION_STRINGTABLE_HPP
#define GENOME_LOCALIZATION_STRINGTABLE_HPP

#include <genome/genome.hpp>
#include <genome/archive.hpp>
#include <genome/hash.hpp>
#include <genome/string.hpp>
#include <genome/time.hpp>
#include <functional>
#include <map>
#include <vector>

namespace genome {
namespace localization {

class stringtable {
	stringtable(stringtable const&) GENOME_DELETE_FUNCTION;
	stringtable& operator=(stringtable const&) GENOME_DELETE_FUNCTION;
public:
	enum compression {
		compression_none,
		compression_fast,
		compression_lzpb,
		compression_lzex,
		compression_tree,
		compression_best
	};
	enum limits {
		// Game engine symbol decoder limit (this class supports any depth on read,
		// but due to the BIN file format, the technical limit is u16(-1) = 65535).
		max_sequence_length = 33  //TODO: analyze all game binaries (stack alignment might allow longer sequences)
	};
	typedef std::vector<string_hash> key_list;
	typedef std::greater<string_hash> key_compare;  // required hash order in binary format
	typedef std::map<string_hash, byte_string, key_compare> name_map;
	typedef std::map<string_hash, wide_string, key_compare> text_map;
	typedef std::vector<wide_string> text_list;

	class source {
	public:
		source(byte_string const& csv_path);
		byte_string const& get_csv(void) const;
		string_hash const& get_csv_hash(void) const;
		byte_string const& get_prefix(void) const;
		string_hash const& get_prefix_hash(void) const;
		filetime const& get_modified(void) const;
		void set_prefix(byte_string const& prefix);
		void set_modified(filetime const& modified);
	private:
		byte_string m_csv;
		string_hash m_csv_hash;
		byte_string m_prefix;
		string_hash m_prefix_hash;
		filetime    m_modified;
	};
	typedef std::vector<source> src_list;

	struct column {
		byte_string name;
		string_hash name_hash;
		text_map    rows;  // only non-empty strings
		column(byte_string const& col_name)
			: name(col_name)
			, name_hash(hash_name(name))
			, rows()
		{
		}
	};
	typedef std::vector<column> col_list;

	stringtable(void);
	~stringtable(void);
	void clear(void);
	source& add_src(byte_string const& csv_path);
	std::size_t add_col(byte_string const& col_name);
	void read_map(char const* csv_path);
	void read_bin(char const* bin_path);
	void read_ini(char const* ini_path);
	void save_csv(void);
	void read_csv(bool utf = false);
	void save_map(char const* csv_path);
	void save_bin(platform bin_plat, u8 bin_vers, char const* bin_path, compression comp);
private:
	struct bin_header {
		// do not change the member types and/or order (streamed as u32[9])
		u32                 magic;      // fourcc_le(u8'S', u8'T', u8'B', version)
		archive::streamsize src_count;
		u32                 reserved;
		archive::streamsize col_count;
		archive::streamsize row_count;
		archive::streampos  src_table;  // offset to bin_source[src_count]
		archive::streampos  col_names;  // offset to streamref[col_count]->byte_char[]
		archive::streampos  col_table;  // offset to bin_column[col_count]
		archive::streampos  key_table;  // offset to streamref->string_hash[row_count]
		explicit bin_header(iarchive& bin);
		explicit bin_header(u8 version);
		iarchive& read(iarchive& bin);
		oarchive& write(oarchive& bin) const;
		u8 version(void) const;
	};
	GENOME_STATIC_ASSERT(bin_header_size, 288 == (sizeof(bin_header) * CHAR_BIT),
		"stringtable::bin_header must be 9 * 32-bit in size.");
	struct bin_source {
		byte_string csv_path;  // u16 length-prefixed, not 0-terminated
		filetime    modified;  // u32[2] (swapped Windows FILETIME)
		explicit bin_source(iarchive& bin);
		explicit bin_source(source const& src);
		iarchive& read(iarchive& bin);
		oarchive& write(oarchive& bin) const;
	};
	struct bin_column {
		// do not change the member types and/or order (read/written as u32[4])
		archive::streamref str_tab;  // u32 str_beg[row_count], u16 str_seq[]
		archive::streamref sym_tab;  // u32 seq_sym[]
	};
	GENOME_STATIC_ASSERT(bin_column_size, 128 == (sizeof(bin_column) * CHAR_BIT),
		"stringtable::bin_column must be 4 * 32-bit in size.");
	struct bin_table {
		std::vector<u32> str_tab;
		std::vector<u16> seq_tab;
		std::vector<u32> sym_tab;
		void add_string(u32 seq);
		void add_new_string(void);
		void add_empty_string(void);
		void add_string_sequence(std::vector<u16>& seq, bool ext);
		void add_sequence(u16 sym);
		void add_sequence_end(void);
		void add_symbol(u32 sym);
		void add_char_symbol(wide_char chr);
		void add_link_symbol(wide_char chr, u16 sym);
		u32 get_next_sequence(void) const;
		u16 get_next_symbol(void) const;
		bool symbols_full(void) const;
		static GENOME_CONSTEXPR_INLINE
		u32 make_char_symbol(wide_char chr)
		{
			return (static_cast<u32>(static_cast<u32>(chr) << 16));
		}
		static GENOME_CONSTEXPR_INLINE
		u32 make_link_symbol(wide_char chr, u16 sym)
		{
			return (static_cast<u32>(make_char_symbol(chr) + sym));
		}
		static GENOME_CONSTEXPR_INLINE
		wide_char get_symbol_char(u32 sym)
		{
			return (static_cast<wide_char>(sym >> 16));
		}
		static GENOME_CONSTEXPR_INLINE
		u16 get_symbol_link(u32 sym)
		{
			return (static_cast<u16>(sym));
		}
	};
	void read_bin_src(iarchive& bin, bin_header const& hdr);
	key_list read_bin_ids(iarchive& bin, bin_header const& hdr);
	void read_bin_col(iarchive& bin, bin_header const& hdr, key_list const& ids);
	void pack_col_none(column const& col, bin_table& tab) const;
	void pack_col_fast(column const& col, bin_table& tab) const;
	void pack_col_lzpb(column const& col, bin_table& tab, bool ext) const;
	void pack_col_tree_char(column const& col, bin_table& tab, bool ext) const;
	void pack_col_tree_node(column const& col, bin_table& tab, bool ext) const;
	void pack_col(column const& col, bin_table& tab, compression comp) const;
private:
	static text_list split_csv_line(wide_string const& csv_line);
private:
	name_map m_map;
	src_list m_src;
	name_map m_ids;
	col_list m_col;
};

} // namespace genome::localization
} // namespace genome

#endif // GENOME_LOCALIZATION_STRINGTABLE_HPP
