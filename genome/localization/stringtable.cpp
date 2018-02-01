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
#include <genome/localization/stringtable.hpp>
#include <genome/filesystem.hpp>
#include <genome/time.hpp>
#include <genome/tstream.hpp>
#include <nicode/suffix_tree.hpp>
#include <cstdlib>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>

namespace genome {
namespace localization {

#ifndef GENOME_DEBUG_STB_ALLOCATOR
#define stb_allocator std::allocator
#else
namespace /*{anonymous}*/ {

// This custom allocator is essential for the suffix_tree performance
// when running under the MSVC debugger. The memory is only freed when
// all allocations are deallocated. With the std::allocator it takes
// a lot of time (about two minutes) just to free all the suffix_tree
// memory (around 500 MB with 10 million allocs for 3 million nodes).
//NOTE: the grow_only_paged_memory implementation is not thread-safe.

class grow_only_paged_memory {
private:
	struct page {
		std::size_t m_free;
		std::size_t m_next;
	//	unsigned char memory[];
	};
	enum config {
		page_size = (4 * 1024 * 1024) - (4 * sizeof(void*)),
		page_data = page_size - sizeof(page)
	};
private:
	std::size_t m_count;
	std::list<page*> m_pages;
public:
	grow_only_paged_memory(void)
		: m_count(0)
		, m_pages()
	{
	}
	void* allocate(std::size_t n)
	{
		if (0 == n) {
			n = sizeof(void*);
		} else {
			std::size_t const m = n % sizeof(void*);
			if (m) {
				n += sizeof(void*) - m;
			}
		}
		if (page_data <= n) {
			page* const p = static_cast<page*>(std::malloc(sizeof(page) + n));
			if (!p) {
				throw std::bad_alloc();
			}
			p->m_free = 0;
			p->m_next = sizeof(page) + n;
			m_pages.push_back(p);
			++m_count;
			return (reinterpret_cast<unsigned char*>(p) + sizeof(page));
		}
		for (std::list<page*>::iterator i = m_pages.begin(); i != m_pages.end(); ++i) {
			page* const p = *i;
			if (n <= p->m_free) {
				void* const m = reinterpret_cast<unsigned char*>(p) + p->m_next;
				p->m_free -= n;
				p->m_next += n;
				++m_count;
				return (m);
			}
		}
		page* const p = static_cast<page*>(std::malloc(page_size));
		if (!p) {
			throw std::bad_alloc();
		}
		p->m_free = page_data - n;
		p->m_next = sizeof(page) + n;
		m_pages.push_front(p);
		++m_count;
		return (reinterpret_cast<unsigned char*>(p) + sizeof(page));
	}
	void deallocate(void* m, std::size_t n)
	{
		if (m && m_count) {
			if (page_data <= n) {
				page* const p = reinterpret_cast<page*>(static_cast<unsigned char*>(m) - sizeof(page));
				std::list<page*>::iterator i = std::find(m_pages.begin(), m_pages.end(), p);
				if (i != m_pages.end()) {
					--m_count;
					m_pages.erase(i);
					return;
				}
			}
			if (0 == --m_count) {
				for (std::list<page*>::iterator i = m_pages.begin(); i != m_pages.end(); ++i) {
					std::free(*i);
				}
				m_pages.clear();
			}
		}
	}
	std::size_t get_page_count(void) const
	{
		return (m_pages.size());
	}
	std::size_t get_alloc_count(void) const
	{
		return (m_count);
	}
} stb_memory;

template<typename T>
class stb_allocator {
public:
	typedef T value_type;
	typedef value_type* pointer;
	typedef value_type& reference;
	typedef value_type const* const_pointer;
	typedef value_type const& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	template<typename U>
	struct rebind {
		typedef stb_allocator<U> other;
	};
	stb_allocator(void) GENOME_NOEXCEPT_NOTHROW
	{
	}
	stb_allocator(stb_allocator<T> const&) GENOME_NOEXCEPT_NOTHROW
	{
	}
	template<typename U>
	stb_allocator(stb_allocator<U> const&) GENOME_NOEXCEPT_NOTHROW
	{
	}
	~stb_allocator(void)
	{
	}
	template<typename U>
	stb_allocator<T>& operator=(stb_allocator<U> const&)
	{
		return (*this);
	}
	pointer allocate(size_type n, void const* = 0)
	{
		if (n <= max_size()) {
			n *= sizeof(value_type);
			pointer const p = static_cast<pointer>(stb_memory.allocate(n));
			if (p) {
				return (p);
			}
		}
		throw std::bad_alloc();
	}
	void deallocate(pointer p, size_type n)
	{
		stb_memory.deallocate(p, n);
	}
	pointer address(reference r) const GENOME_NOEXCEPT_NOTHROW
	{
		return (&r);
	}
	const_pointer address(const_reference c) const GENOME_NOEXCEPT_NOTHROW
	{
		return (&c);
	}
	void construct(pointer p, const_reference c)
	{
		::new((void*)p) value_type(c);
	}
	void destroy(pointer p)
	{
		if (p) {
			p->~value_type();
		}
	}
	GENOME_CONSTEXPR_INLINE size_type max_size(void) const GENOME_NOEXCEPT_NOTHROW
	{
		return (std::numeric_limits<size_type>::max() / sizeof(value_type));
	}
};
template<typename T, typename U>
bool operator==(stb_allocator<T> const&, stb_allocator<U> const&) GENOME_NOEXCEPT_NOTHROW
{
	return (true);
}
template<typename T, typename U>
bool operator!=(stb_allocator<T> const&, stb_allocator<U> const&) GENOME_NOEXCEPT_NOTHROW
{
	return (false);
}
template<>
class stb_allocator<void> {
public:
	typedef void value_type;
	typedef void* pointer;
	typedef void const* const_pointer;
	template<typename U>
	struct rebind {
		typedef stb_allocator<U> other;
	};
	stb_allocator(void) GENOME_NOEXCEPT_NOTHROW
	{
	}
	stb_allocator(stb_allocator<void> const&) GENOME_NOEXCEPT_NOTHROW
	{
	}
	template<typename U>
	stb_allocator(stb_allocator<U> const&) GENOME_NOEXCEPT_NOTHROW
	{
	}
	~stb_allocator(void)
	{
	}
	template<typename U>
	stb_allocator<void>& operator=(stb_allocator<U> const&)
	{
		return (*this);
	}
};

} // namespace genome::localization::{anonymous}
#endif//GENOME_DEBUG_STB_ALLOCATOR

//
// stringtable::bin_header
//

stringtable::bin_header::bin_header(iarchive& bin)
{
	if (!read(bin)) {
		throw std::invalid_argument("failed to read string table header");
	}
	if ((magic & 0x00FFFFFFUL) != 0x00425453UL) {
		throw std::invalid_argument("invalid string table signature");
	}
	if (version() < 5) {
		throw std::invalid_argument("invalid string table version");
	}
}

stringtable::bin_header::bin_header(u8 version)
	: magic((u32(version) << 24) | 0x00425453UL)
	, src_count(0)
	, reserved(0)
	, col_count(0)
	, row_count(0)
	, src_table(0)
	, col_names(0)
	, col_table(0)
	, key_table(0)
{
}

iarchive&
stringtable::bin_header::read(iarchive& bin)
{
	return (bin >> reinterpret_cast<u32(&)[sizeof(bin_header) / sizeof(u32)]>(magic));
}

oarchive&
stringtable::bin_header::write(oarchive& bin) const
{
	return (bin << reinterpret_cast<u32 const(&)[sizeof(bin_header) / sizeof(u32)]>(magic));
}

u8
stringtable::bin_header::version(void) const
{
	return (static_cast<u8>(magic >> 24));
}

//
// stringtable::bin_source
//

stringtable::bin_source::bin_source(iarchive& bin)
	: csv_path()
	, modified()
{
	read(bin);
}

stringtable::bin_source::bin_source(source const& src)
	: csv_path(src.get_csv())
	, modified(src.get_modified())
{
}

iarchive&
stringtable::bin_source::read(iarchive& bin)
{
	return (bin >> csv_path >> modified);
}

oarchive&
stringtable::bin_source::write(oarchive& bin) const
{
	return (bin << csv_path << modified);
}

//
// stringtable::bin_table
//

void
stringtable::bin_table::add_string(u32 seq)
{
	str_tab.push_back(seq);
}

void
stringtable::bin_table::add_new_string(void)
{
	add_string(get_next_sequence());
}

void
stringtable::bin_table::add_empty_string(void)
{
	add_string(u32(-1));
}

void
stringtable::bin_table::add_string_sequence(std::vector<u16>& seq, bool ext)
{
	seq.push_back(u16(0));
	if (ext) {
		// AFAIK this optimization is not included in the Genome engine
		std::vector<u16>::iterator pos = std::search(seq_tab.begin(), seq_tab.end(), seq.begin(), seq.end());
		if (pos != seq_tab.end()) {
			add_string(u32(std::distance(seq_tab.begin(), pos)));
			return;
		}
	}
	add_new_string();
	seq_tab.insert(seq_tab.end(), seq.begin(), seq.end());
}

void
stringtable::bin_table::add_sequence(u16 sym)
{
	seq_tab.push_back(sym);
}

void
stringtable::bin_table::add_sequence_end(void)
{
	add_sequence(u16(0));
}

void
stringtable::bin_table::add_symbol(u32 sym)
{
	sym_tab.push_back(sym);
}

void
stringtable::bin_table::add_char_symbol(wide_char chr)
{
	add_symbol(u32(u32(chr) << 16));
}

void
stringtable::bin_table::add_link_symbol(wide_char chr, u16 sym)
{
	add_symbol(u32(u32(u32(chr) << 16) + sym));
}

u32
stringtable::bin_table::get_next_sequence(void) const
{
	return (u32(seq_tab.size()));
}

u16
stringtable::bin_table::get_next_symbol(void) const
{
	//NOTE: also returns 0 if symbol table is full
	return (u16(sym_tab.size()));
}

bool
stringtable::bin_table::symbols_full(void) const
{
	return (u16(-1) < sym_tab.size());
}

//
// stringtable::source
//

stringtable::source::source(byte_string const& csv_path)
	: m_csv(csv_path)
	, m_csv_hash(hash_name(m_csv))
	, m_prefix()
	, m_prefix_hash()
	, m_modified()
{
}

byte_string const&
stringtable::source::get_csv(void) const
{
	return (m_csv);
}

string_hash const&
stringtable::source::get_csv_hash(void) const
{
	return (m_csv_hash);
}

byte_string const&
stringtable::source::get_prefix(void) const
{
	return (m_prefix);
}

string_hash const&
stringtable::source::get_prefix_hash(void) const
{
	return (m_prefix_hash);
}

filetime const&
stringtable::source::get_modified(void) const
{
	return (m_modified);
}

void
stringtable::source::set_prefix(byte_string const& prefix)
{
	m_prefix.assign(prefix);
	m_prefix_hash = m_prefix.empty() ? string_hash() : hash_name(m_prefix);
}

void
stringtable::source::set_modified(filetime const& modified)
{
	m_modified = modified;
}

//
// stringtable
//

stringtable::stringtable(void)
	: m_map()
	, m_src()
	, m_ids()
	, m_col()
{
}

stringtable::~stringtable(void)
{
	clear();
}

void
stringtable::clear(void)
{
	m_map.clear();
	m_src.clear();
	m_ids.clear();
	m_col.clear();
}

stringtable::source&
stringtable::add_src(byte_string const& csv_path)
{
	string_hash const h = hash_name(csv_path);
	src_list::iterator s = m_src.begin();
	while (s != m_src.end()) {
		if (h == s->get_csv_hash()) {
			return (*s);
		}
		++s;
	}
	return (*m_src.insert(s, source(csv_path)));
}

std::size_t
stringtable::add_col(byte_string const& col_name)
{
	std::size_t i;
	string_hash const h = hash_name(col_name);
	for (i = 0; i < m_col.size(); ++i) {
		if (h == m_col[i].name_hash) {
			return (i);
		}
	}
	m_col.push_back(column(col_name));
	return (i);
}

void
stringtable::read_map(char const* csv_path)
{
	std::string fname((csv_path && *csv_path) ? csv_path : "#G3:/lianzifu.csv");
	std::wcout << L"[" << genome::to_wstring(fname) << L"]" << std::endl;
	u16itfstream ift(fname.c_str(), tstream::encoding_utf8);
	if (!ift) {
		throw std::runtime_error("failed to open idname mapping file");
	}
	u32 lno = 0;
	u32 cnt = 0;
	u16itfstream::string_type str;
	while (!ift.eof() && ift.getline(str)) {
		++lno;
		{
			u16itfstream::string_type::size_type n = str.find_first_of(0x007C);  // '|'
			if (n != u16itfstream::string_type::npos) {
				str.resize(n);
			}
		}
		if (str.empty()) {
			if (ift.eof()) {
				--lno;
			}
			continue;
		}
		byte_string name;
		if (!string_convert(str, name)) {
			std::wclog << L";warn: invalid characters in " << to_wstring(fname) << L"," << to_wstring(lno) << std::endl;
			continue;
		}
		byte_string id(name, name.find_first_of(byte_code::colon) + 1);
		if (id.empty() || (id.find_first_of(byte_code::colon) != byte_string::npos)) {
			std::wclog << L";warn: invalid identifier in " << to_wstring(fname) << L"," << to_wstring(lno) << std::endl;
			continue;
		}
		string_hash key = hash_name(id);
		std::pair<name_map::iterator, bool> p = m_map.insert(std::make_pair(key, name));
		if (!p.second) {
			byte_string const& n = p.first->second;
			if (name_traits::compare(n.c_str(), name.c_str(), std::max<std::size_t>(n.size(), name.size()) + 1)) {
				std::wclog << L";warn: hash collison between '" << to_wstring(n) << L"' and '" << to_wstring(name) << L"' in " << to_wstring(fname) << L"," << to_wstring(lno) << std::endl;
			} else {
				std::wclog << L";info: duplicate entry in " << to_wstring(fname) << L"," << to_wstring(lno) << std::endl;
			}
		} else {
			++cnt;
		}
	}
	if (!ift) {
		throw std::runtime_error("failed to read idname mapping file line " + to_string(lno));
	}
	std::wcout << L"idname.lines=" << to_wstring(lno) << std::endl;
	std::wcout << L"idname.valid=" << to_wstring(cnt) << std::endl;
	std::wcout << L"idname.count=" << to_wstring(m_map.size()) << std::endl;
	std::wcout << std::endl;
}

void
stringtable::read_bin(char const* bin_path)
{
	std::string fname((bin_path && *bin_path) ? bin_path : "#G3:/data/compiled/localization/w_strings.bin");
	std::wcout << L"[" << to_wstring(fname) << L"]" << std::endl;
	ifarchive ifa(fname.c_str());
	bin_header hdr(ifa);
	std::wcout << L"version=" << to_wstring(hdr.version()) << std::endl;
	std::wcout << L"reserved=" << to_wstring(hdr.reserved) << std::endl;
	std::wcout << L"source.count=" << to_wstring(hdr.src_count) << std::endl;
	std::wcout << L"column.count=" << to_wstring(hdr.col_count) << std::endl;
	std::wcout << L"string.count=" << to_wstring(hdr.row_count) << std::endl;
	std::wcout << L"source.table=0x" << to_wstring(hash_to_string(hdr.src_table)) << std::endl;
	std::wcout << L"idhash.table=0x" << to_wstring(hash_to_string(hdr.key_table)) << std::endl;
	std::wcout << L"column.names=0x" << to_wstring(hash_to_string(hdr.col_names)) << std::endl;
	std::wcout << L"column.table=0x" << to_wstring(hash_to_string(hdr.col_table)) << std::endl;
	read_bin_src(ifa, hdr);
	read_bin_col(ifa, hdr, read_bin_ids(ifa, hdr));
	std::wcout << std::endl;
}

void
stringtable::read_bin_src(iarchive& bin, bin_header const& hdr)
{
	if ((hdr.src_count > 0) && bin.good()) {
		if (!bin.seekg(hdr.src_table)) {
			std::wcout << L";warn: invalid source table offset" << std::endl;
		} else {
			for (archive::streamsize i = 0; i < hdr.src_count; ++i) {
				bin_source src(bin);
				if (!bin) {
					std::wcout << L";warn: failed to read source entry #" << to_wstring(i + 1) << std::endl;
					break;
				}
				add_src(src.csv_path).set_modified(src.modified);
				std::wcout << L"source." << to_wstring(i + 1) << L"=" << to_wstring(src.modified) << L" " << to_wstring(src.csv_path) << std::endl;
			}
		}
		if (bin.fail()) {
			bin.clear();
		}
	}
}

stringtable::key_list
stringtable::read_bin_ids(iarchive& bin, bin_header const& hdr)
{
	key_list keys;
	u32 cnt = 0;
	if (hdr.row_count > 0) {
		archive::streamref ref;
		if (!bin.seekg(hdr.key_table) || !(bin >> ref)) {
			throw std::invalid_argument("invalid idhash table offset");
		}
		if (!bin.seekg(ref.pos) || (ref.size < hdr.row_count * sizeof(string_hash))) {
			throw std::invalid_argument("invalid idhash table reference");
		}
		if (!bin.read(keys, hdr.row_count)) {
			throw std::invalid_argument("failed to read idhash table");
		}
		name_map::iterator id = m_ids.end();
		for (key_list::const_iterator i = keys.begin(); i != keys.end(); ++i) {
			string_hash const& key = *i;
			id = m_ids.insert(id, std::make_pair(key, byte_string()));
			if (id->second.empty()) {
				name_map::const_iterator name = m_map.find(id->first);
				if (name != m_map.end()) {
					id->second = name->second;
					++cnt;
				}
			}
			++id;
		}
	}
	std::wcout << L"idhash.names=" << to_wstring(cnt) << std::endl;
	return (keys);
}

void
stringtable::read_bin_col(iarchive& bin, bin_header const& hdr, key_list const& ids)
{
	if (hdr.col_count > 0) {
		std::vector<std::size_t> col_indices;
		{
			std::vector<archive::streamref> refs;
			if (!bin.seekg(hdr.col_names) || !bin.read(refs, hdr.col_count)) {
				throw std::invalid_argument("invalid column name table offset");
			}
			for (archive::streamsize i = 0; i < hdr.col_count; ++i) {
				byte_string name;
				if (!read_ref_string(bin, refs[i], name) || name.empty()) {
					throw std::invalid_argument("invalid column name reference");
				}
				col_indices.push_back(add_col(name));
				std::wcout << L"column.name." << to_wstring(i + 1) << L"=" << to_wstring(name) << std::endl;
			}
		}
		{
			std::vector<bin_column> refs;
			refs.resize(hdr.col_count);
			if (!bin.seekg(hdr.col_table) || !bin.read(&refs[0].str_tab.size, hdr.col_count * 4)) {
				throw std::invalid_argument("invalid column data table offset");
			}
			for (std::size_t i = 0; i < hdr.col_count; ++i) {
				bin_column const& ref = refs[i];
				std::wcout << L"column.data." << to_wstring(i + 1) << L".strings=0x" << to_wstring(hash_to_string(ref.str_tab.pos)) << L"[0x" << to_wstring(hash_to_string(ref.str_tab.size)) << L"]" << std::endl;
				std::wcout << L"column.data." << to_wstring(i + 1) << L".symbols=0x" << to_wstring(hash_to_string(ref.sym_tab.pos)) << L"[0x" << to_wstring(hash_to_string(ref.sym_tab.size)) << L"]" << std::endl;

				// u32[row_count] indices to first symbol sequence
				// u16[] indices to symbol sequences, 0-terminated
				std::vector<u32> str_beg;
				std::vector<u16> str_seq;
				if ((ref.str_tab.size < hdr.row_count * sizeof(u32)) ||
					!bin.seekg(ref.str_tab.pos) ||
					!bin.read(str_beg, hdr.row_count) ||
					!bin.read(str_seq, (ref.str_tab.size - (hdr.row_count * sizeof(u32))) / sizeof(u16))) {
					throw std::invalid_argument("invalid string table reference");
				}
				std::wcout << L"column.data." << to_wstring(i + 1) << L".seq_num=" << to_wstring(str_seq.size()) << std::endl;

				// u32[] symbol table, low u16 prev symbol (0 = rend), high u16 UTF-16 code
				std::vector<u32> seq_sym;
				if (!bin.seekg(ref.sym_tab.pos) ||
					!bin.read(seq_sym, ref.sym_tab.size / sizeof(u32))) {
					throw std::invalid_argument("invalid symbol table reference");
				}
				std::wcout << L"column.data." << to_wstring(i + 1) << L".sym_num=" << to_wstring(seq_sym.size()) << std::endl;

				wide_string::size_type max_sub = 0;
				wide_string::size_type max_str = 0;
				column& col = m_col[col_indices[i]];
				for (archive::streamsize j = 0; j < hdr.row_count; ++j) {
					string_hash const& hash = ids[j];
				//	byte_string const& name = m_ids[hash];
					u32 const beg = str_beg[j];
					if (u32(-1) == beg) {  // empty string
						continue;
					} else if (beg >= str_seq.size()) {
						throw std::out_of_range("invalid string sequence index");
					}
					wide_string str;
					std::vector<u16>::const_iterator seq = str_seq.begin();
					std::advance(seq, beg);
					do {
						if (*seq >= seq_sym.size()) {
							throw std::out_of_range("invalid string symbol index");
						}
						wide_string sub;
						u32 sym = seq_sym[*seq];
						//TODO: scan for invalid UTF-16 code sequences
						for (;;) {
							wide_char const c = static_cast<wide_char>((sym >> 16) & 0xFFFFU);
							if (0 == c) {
								throw std::out_of_range("invalid string symbol character");
							}
							sub.push_back(c);
							u16 const p = static_cast<u16>(sym & 0xFFFFU);
							if (0 == p) {
								break;
							}
							if (p >= seq_sym.size()) {
								throw std::out_of_range("invalid string symbol reference");
							}
							sym = seq_sym[p];
						}
						if (max_sub < sub.size()) {
							max_sub = sub.size();
						}
						str.append(sub.rbegin(), sub.rend());
						if (str_seq.end() == ++seq) {
							throw std::overflow_error("unterminated string sequence");
						}
					} while (*seq);
					if (max_str < str.size()) {
						max_str = str.size();
					}
					std::pair<text_map::iterator, bool> row = col.rows.insert(std::make_pair(hash, str));
					if (!row.second) {
						//TODO: make text override optional during merge
						row.first->second = str;
					}
				}
				std::wcout << L"column.data." << to_wstring(i + 1) << L".max_sub=" << to_wstring(max_sub) << std::endl;
				std::wcout << L"column.data." << to_wstring(i + 1) << L".max_str=" << to_wstring(max_str) << std::endl;
			}
		}
	}
}

void
stringtable::read_ini(char const* ini_path)
{
	//NOTE: manual parsing (engine expects the line to start with a
	// case-sensitive "csv=" and whitespace is not skipped/ignored)
	std::string fname((ini_path && *ini_path) ? ini_path : "#G3:/ini/loc.ini");
	std::wcout << L"[" << to_wstring(fname) << L"]" << std::endl;
	witfstream ift(fname.c_str());
	if (!ift) {
		throw std::runtime_error("failed to open localization config file");
	}
	byte_string prefix_tag = to_byte_string(std::string("prefix="));
	byte_string csv_tag = to_byte_string(std::string("csv="));
	byte_string prefix;
	byte_string csv;
	witfstream::string_type wstr;
	while (!ift.eof() && ift.getline(wstr)) {
		if (wstr.empty()) {
			continue;
		}
		byte_string bstr;
		if (!string_convert(wstr, bstr)) {
			continue;
		}
		if (string_startswith(bstr, prefix_tag)) {
			prefix.assign(bstr, prefix_tag.size(), byte_string::npos);
			continue;
		}
		if (string_startswith(bstr, csv_tag)) {
			csv.assign(bstr, csv_tag.size(), byte_string::npos);
			if (!csv.empty()) {
				source& src = add_src(csv);
				if (prefix.empty()) {
					prefix.assign(src.get_prefix());
				} else {
					src.set_prefix(prefix);
				}
				std::wcout << to_wstring(prefix) << L"=" << to_wstring(csv) << std::endl;
			}
			prefix.clear();
		}
	}
	std::wcout << std::endl;
}

void
stringtable::save_csv(void)
{
	if (m_ids.empty() || m_col.empty()) {
		return;
	}
	if (m_src.empty()) {
		add_src(
			to_byte_string(std::string("#G3:/Data/Raw/Strings/strings.csv"))).set_prefix(
			to_byte_string(std::string("default")));
	}

	std::vector<std::size_t> id_src;
	std::vector<wide_string> id_str;
	std::vector<string_hash> id_key;
	for (name_map::const_iterator i = m_ids.begin(); i != m_ids.end(); ++i) {
		std::size_t src = 0;  // first is the default
		byte_string str;
		byte_string const& name = i->second;
		byte_string::size_type const pos = name.find_first_of(byte_code::colon);
		if (pos != byte_string::npos) {
			str.assign(name, pos + 1, byte_string::npos);
			byte_string const p(name, 0, pos);
			string_hash const h = hash_name(p);
			for (src_list::const_iterator j = m_src.begin(); j != m_src.end(); ++j) {
				if (j->get_prefix_hash() == h) {
					src = static_cast<std::size_t>(j - m_src.begin());
					break;
				}
			}
		} else {
			str.assign(name);
		}
		if (str.empty()) {
			str = hash_to_string(i->first);
		}
		id_src.push_back(src);
		id_str.push_back(to_wide_string(str));
		id_key.push_back(i->first);
	}

	wide_string head(to_wide_string(std::wstring(L"ID")));
	for (col_list::const_iterator i = m_col.begin(); i != m_col.end(); ++i) {
		head.push_back(0x007C);  // '|'
		head += to_wide_string(i->name);
	}

	for (std::size_t i = 0; i < m_src.size(); ++i) {
		std::vector<std::size_t> src_idx;
		for (std::size_t j = 0; j < id_src.size(); ++j) {
			if (id_src[j] == i) {
				src_idx.push_back(j);
			}
		}
		std::string csv(to_string(m_src[i].get_csv()));
		std::wcout << L"[" << to_wstring(csv) << L"]" << std::endl;
		filesystem::ensure_directories(csv.c_str());
		u16otfstream oft(csv.c_str(), tstream::encoding_utf8, tstream::newline_unix);
		if (!oft.putline(head)) {
			throw std::runtime_error("failed to write csv header");
		}
		for (std::vector<std::size_t>::const_iterator j = src_idx.begin(); j != src_idx.end(); ++j) {
			wide_string line = id_str[*j];
			for (col_list::const_iterator k = m_col.begin(); k != m_col.end(); ++k) {
				line.push_back(0x007C);  // '|'
				wide_string text;
				text_map::const_iterator row = k->rows.find(id_key[*j]);
				if (row != k->rows.end()) {
					text.assign(row->second);
				}
				for (wide_string::const_iterator m = text.begin(); m != text.end(); ++m) {
					switch (*m) {
					case 0x0000:  // '\0'
						line.push_back(0x005C);  // '\\'
						line.push_back(0x0030);  // '0'
						break;
					case 0x000A:  // '\n'
						line.push_back(0x005C);  // '\\'
						line.push_back(0x006E);  // 'n'
						break;
					case 0x000D:  // '\r'
						line.push_back(0x005C);  // '\\'
						line.push_back(0x0072);  // 'r'
						break;
					case 0x0040:  // '@'
						line.push_back(0x005C);  // '\\'
						line.push_back(0x0061);  // 'a'
						break;
					case 0x005C:  // '\\'
						line.push_back(0x005C);  // '\\'
						line.push_back(0x005C);  // '\\'
						break;
					case 0x007C:  // '|'
						line.push_back(0x005C);  // '\\'
						line.push_back(0x0076);  // 'v'
						break;
					default:
						line.push_back(*m);
						break;
					}
				}
			}
			if (!oft.putline(line)) {
				throw std::runtime_error("failed to write csv line");
			}
		}
		std::wcout << std::endl;
	}
}

stringtable::text_list
stringtable::split_csv_line(wide_string const& csv_line)
{
	text_list lst;
	wide_string str;
	bool esc = false;
	for (wide_string::const_iterator pos = csv_line.begin(); pos != csv_line.end(); ++pos) {
		wide_char const& chr = *pos;
		if (0x005C == chr) {  // '\\'
			if (esc) {
				str.push_back(0x005C);  // '\\'
				esc = false;
				continue;
			}
			esc = true;
			continue;
		}
		if (0x007C == chr) {  // '|'
			if (esc) {
				str.push_back(0x005C);  // '\\'
				esc = false;
			}
			lst.push_back(str);
			str.erase();
			continue;
		}
		if (esc) {
			esc = false;
			switch (chr) {
			case 0x0061:  // "\a"
				str.push_back(0x0040);  // '@'
				continue;
			case 0x006E:  // "\n"
				str.push_back(0x000A);  // '\n'
				continue;
			case 0x0072:  // "\r"
				str.push_back(0x000D);  // '\r'
				continue;
			case 0x0076:  // "\v"
				str.push_back(0x007C);  // '|'
				continue;
			default:
				str.push_back(0x005C);  // '\\'
				break;
			}
		}
		str.push_back(chr);
	}
	lst.push_back(str);
	return (lst);
}

void
stringtable::read_csv(bool utf)
{
	string_hash id_hash = hash_name(to_byte_string(std::wstring(L"ID")));

	for (src_list::iterator psrc = m_src.begin(); psrc != m_src.end(); ++psrc) {
		source& src = *psrc;
		std::string const fname(to_string(src.get_csv()));
		std::wcout << L"[" << to_wstring(fname) << L"]" << std::endl;
		{
			filetime ftime(fname.c_str());
			if (!ftime.valid()) {
				throw std::runtime_error("failed to get csv time");
			}
			src.set_modified(ftime);
			std::wcout << L"modtime=" << to_wstring(ftime) << std::endl;
		}

		u16itfstream ift(fname.c_str(), tstream::encoding_unknown, utf);
		if (tstream::encoding_genome == ift.getenc()) {
			std::wclog << L";warn: CSV with Windows-1252 encoding (UTF-8 without BOM?)" << std::endl;
		}
		wide_string csv_rec;
		if (!ift.getline(csv_rec)) {
			throw std::runtime_error("failed to read csv head");
		}
		std::vector<std::size_t> col_idx;
		{
			text_list csv_fld = split_csv_line(csv_rec);
			for (text_list::const_iterator pfld = ++csv_fld.begin(); pfld != csv_fld.end(); ++pfld) {
				byte_string col_name;
				if (!string_convert(*pfld, col_name) || col_name.empty() || (id_hash == hash_name(col_name))) {
					throw std::invalid_argument("invalid csv column name");
				}
				std::size_t idx = add_col(col_name);
				for (std::vector<std::size_t>::const_iterator pidx = col_idx.begin(); pidx != col_idx.end(); ++pidx) {
					if (*pidx == idx) {
						throw std::invalid_argument("duplicate csv column name");
					}
				}
				col_idx.push_back(idx);
			}
		}

		u32 csv_lno = 1;
		u32 rec_cnt = 0;
		u32 no_name = 0;
		while (!ift.eof() && ift.getline(csv_rec)) {
			++csv_lno;
			if (csv_rec.empty()) {
				continue;
			}
			text_list csv_fld = split_csv_line(csv_rec);
			if (csv_fld.size() - 1 > col_idx.size()) {
				throw std::invalid_argument("too many csv fields in line " + to_string(csv_lno));
			}

			text_list::const_iterator pfld = csv_fld.begin();
			byte_string id_name;
			if (!string_convert(*pfld, id_name) || id_name.empty()) {
				throw std::invalid_argument("invalid csv id in line " + to_string(csv_lno));
			}
			string_hash id_hash;
			if (string_to_hash(id_name, id_hash)) {
				//TODO: idhash parsing should be optional
				id_name.erase();
				++no_name;
			} else {
				id_hash = hash_name(id_name);
				byte_string const& prefix = src.get_prefix();
				if (!prefix.empty()) {
					id_name.insert(0, 1, byte_code::colon);
					id_name.insert(0, prefix);
				}
			}
			{
				std::pair<name_map::iterator, bool> id = m_ids.insert(std::make_pair(id_hash, id_name));
				if (!id.second) {
					std::string info;
					info.assign("hash conflict in csv line ");
					info.append(to_string(csv_lno));
					info.append(" (");
					info.append(to_string(id_hash));
					info.append("|");
					info.append(to_string(id_name));
					info.append("|");
					info.append(to_string(id.first->second));
					info.append(")");
					throw std::invalid_argument(info);
				}
			}
			std::vector<std::size_t>::const_iterator pidx = col_idx.begin();
			for (++pfld; pfld != csv_fld.end(); ++pfld, ++pidx) {
				wide_string const& id_text = *pfld;
				if (!id_text.empty()) {
					column& col = m_col[*pidx];
					std::pair<text_map::iterator, bool> row = col.rows.insert(std::make_pair(id_hash, id_text));
					if (!row.second) {
						//FIXME: this should never happen
						row.first->second = id_text;
					}
				}
			}
			++rec_cnt;
		}
		if (!ift) {
			throw std::runtime_error("failed to read csv line " + to_string(csv_lno));
		}
		std::wcout << L"records=" << to_wstring(rec_cnt) << std::endl;
		std::wcout << L"unnamed=" << to_wstring(no_name) << std::endl;
		std::wcout << std::endl;
	}
}

void
stringtable::save_map(char const* csv_path)
{
	std::string fname((csv_path && *csv_path) ? csv_path : "#G3:/lianzifu.csv");
	std::wcout << L"[" << to_wstring(fname) << L"]" << std::endl;
	u16otfstream oft(fname.c_str(), tstream::encoding_utf8, tstream::newline_unix);
	if (!oft) {
		throw std::runtime_error("failed to create idname mapping file");
	}
	u32 rec_cnt = 0;
	for (name_map::const_iterator id = m_ids.begin(); id != m_ids.end(); ++id) {
		byte_string const& name = id->second;
		if (!name.empty()) {
			u16otfstream::string_type line = string_cast<byte_string, u16otfstream::string_type>(name);
			line.push_back(0x007C);  // '|'
			line.append(string_cast<byte_string, u16otfstream::string_type>(hash_to_string(id->first)));
			if (!oft.putline(line)) {
				throw std::runtime_error("failed to write idname mapping line");
			}
			++rec_cnt;
		}
	}
	std::wcout << L"idnames=" << to_wstring(rec_cnt) << std::endl;
	std::wcout << std::endl;
}

void
stringtable::pack_col_none(column const& col, bin_table& tab) const
{
	// symbol index = UTF-16 code, full code table, no symbol links
	tab.sym_tab.reserve(1 << 16);
	// symbol #0->0 is added in pack_col()
	for (u32 chr = 1; chr <= u16(-1); ++chr) {
		tab.add_char_symbol(u16(chr));
	}
	tab.seq_tab.reserve(col.rows.size() * 64);
	for (name_map::const_iterator id = m_ids.begin(); id != m_ids.end(); ++id) {
		text_map::const_iterator row = col.rows.find(id->first);
		if (col.rows.end() == row) {
			tab.add_empty_string();
		} else {
			tab.add_new_string();
			wide_string const& str = row->second;
			for (wide_string::const_iterator chr = str.begin(); chr != str.end(); ++chr) {
				u16 const sym = u16(*chr);
				tab.add_sequence(sym);
			}
			tab.add_sequence_end();
		}
	}
}

void
stringtable::pack_col_fast(column const& col, bin_table& tab) const
{
	typedef std::map<wide_char, u16> chr2sym_map;

	chr2sym_map chr2sym;
	// symbol #0->0 is added in pack_col()
	chr2sym.insert(std::make_pair(wide_char(0), u16(0)));

	// one symbol per UTF-16 code, no symbol links
	tab.seq_tab.reserve(col.rows.size() * 64);
	tab.sym_tab.reserve(128);
	for (name_map::const_iterator id = m_ids.begin(); id != m_ids.end(); ++id) {
		text_map::const_iterator row = col.rows.find(id->first);
		if (col.rows.end() == row) {
			tab.add_empty_string();
		} else {
			tab.add_new_string();
			wide_string const& str = row->second;
			for (wide_string::const_iterator chr = str.begin(); chr != str.end(); ++chr) {
				chr2sym_map::iterator sym = chr2sym.find(*chr);
				if (chr2sym.end() == sym) {
					sym = chr2sym.insert(std::make_pair(*chr, tab.get_next_symbol())).first;
					tab.add_char_symbol(*chr);
				}
				tab.add_sequence(sym->second);
			}
			tab.add_sequence_end();
		}
	}
}

void
stringtable::pack_col_lzpb(column const& col, bin_table& tab, bool ext) const
{
	typedef std::map<u32, u16> key2sym_map;

	key2sym_map key2sym;
	// symbol #0->0 is added in pack_col()
	key2sym.insert(std::make_pair(u32(0), u16(0)));

	// ensure that all used UTF-16 codes are present as unlinked symbols
	tab.sym_tab.reserve(1 << 16);
	for (name_map::const_iterator id = m_ids.begin(); id != m_ids.end(); ++id) {
		text_map::const_iterator row = col.rows.find(id->first);
		if (row != col.rows.end()) {
			wide_string const& str = row->second;
			for (wide_string::const_iterator chr = str.begin(); chr != str.end(); ++chr) {
				u32 const key = bin_table::make_char_symbol(*chr);
				if (key2sym.end() == key2sym.find(key)) {
					key2sym.insert(std::make_pair(key, tab.get_next_symbol()));
					tab.add_symbol(key);
				}
			}
		}
	}

	// add one-growing sequences until the symbol table is full
	tab.seq_tab.reserve(col.rows.size() * 16);
	std::vector<u16> str_seq;
	for (name_map::const_iterator id = m_ids.begin(); id != m_ids.end(); ++id) {
		text_map::const_iterator row = col.rows.find(id->first);
		if (col.rows.end() == row) {
			tab.add_empty_string();
		} else {
			str_seq.clear();
			bool str_ext = ext;
			u16 seq_sym = 0;
			std::size_t seq_len = 0;
			wide_string const& str = row->second;
			for (wide_string::const_iterator chr = str.begin(); chr != str.end(); ++chr) {
				u32 const key = bin_table::make_link_symbol(*chr, seq_sym);
				key2sym_map::iterator sym = key2sym.find(key);
				if (sym != key2sym.end()) {
					// existing (possibly linked) symbol found
					seq_sym = sym->second;
				} else {
					if (tab.symbols_full()) {
						// add last found sequence first
						if (seq_sym) {
							str_seq.push_back(seq_sym);
						}
						// restart with existing (unlinked) symbol
						seq_sym = key2sym.find(bin_table::make_char_symbol(*chr))->second;
						seq_len = 0;
					} else {
						// add new (linked) symbol
						str_ext = false;  // no need to search for existing string sequences
						seq_sym = tab.get_next_symbol();
						tab.add_symbol(key);
						key2sym.insert(std::make_pair(key, seq_sym));
						// stop generating linked symbols (avoid fast growing symbol table)
						// Even with this method the symbol table is full after 25% of the
						// strings in Risen 3 (but without it, the table is full after 6%).
						seq_len = max_sequence_length;
					}
				}
				++seq_len;
				if (max_sequence_length <= seq_len) {
					str_seq.push_back(seq_sym);
					seq_sym = 0;
					seq_len = 0;
				}
			}
			// add last found sequence first
			if (seq_sym) {
				str_seq.push_back(seq_sym);
			}
			tab.add_string_sequence(str_seq, str_ext);
		}
	}
}

void
stringtable::pack_col_tree_char(column const& col, bin_table& tab, bool ext) const {
	typedef nicode::suffix_tree<wide_char, nicode::suffix_tree_traits<wide_char, wide_string, stb_allocator, u32> > tree_type;
	typedef std::pair<u16, u16> symbol_info;  // first: last symbol index, second: current symbol length (in table)
	typedef std::pair<std::size_t, tree_type::position> char_info;  // first: node index, second: sequence length

	tab.sym_tab.reserve(1 << 16);
	tab.seq_tab.reserve(col.rows.size() * 16);

	// build generalized suffix tree from all non-empty strings
	tree_type tree;
	tree.reserve(col.rows.size() * 64, col.rows.size() * 96);
	for (name_map::const_iterator id = m_ids.begin(); id != m_ids.end(); ++id) {
		text_map::const_iterator row = col.rows.find(id->first);
		if (row != col.rows.end()) {
			wide_string const &str = row->second;
			if (!str.empty()) {
				tree.append(str);
			}
		}
	}
	tree.build();

	// calc weight (suffix frequency) of all non-leaf nodes
	std::vector<tree_type::position> node_weight(tree.size(), 0);
	for (tree_type::const_iterator iter = tree.begin(); iter != tree.end(); ++iter) {
		tree_type::node const &node = *iter;
		if (node.empty()) {
			// advance all non-root parents
			for (tree_type::node const *parent = node.parent(); parent && parent->parent(); parent = parent->parent()) {
				tree_type::position &parent_value = node_weight[parent->index()];
				tree_type::position const parent_weight = parent_value + 1;
				if (parent_value < parent_weight) {
					parent_value = parent_weight;
				}
				// advance all non-root suffix links (suffix without the first character)
				for (tree_type::node const *link = parent->link(); link && link->parent(); link = link->link()) {
					tree_type::position &link_value = node_weight[link->index()];
					tree_type::position const link_weight = link_value + 1;
					if (link_value < link_weight) {
						link_value = link_weight;
					}
				}
			}
		}
	}

	std::vector<symbol_info> node_symbol(tree.size(),
										 std::make_pair(symbol_info::first_type(0), symbol_info::second_type(0)));

	// ensure that all used UTF-16 codes are present as unlinked symbols
	// (this also guarantees that we never reach root while searching for exiting node symbols)
	for (tree_type::node const *node = tree.root().front(); node; node = node->sibling()) {
		tree_type::symbol const &sym = tree.symbols()[node->edge().begin()];
		if (sym.is_char()) {
			symbol_info &info = node_symbol[node->index()];
			u32 const key = bin_table::make_char_symbol(sym.get_char());
			if (key != 0) {  // symbol #0->0 is added in pack_col()
				info.first = tab.get_next_symbol();
				tab.add_symbol(key);
			}
			info.second = 1;
		}
	}

	// iterate over all strings and add the symbols/sequences
	std::vector<char_info> char_node;
	std::vector<u16> str_seq;
	for (name_map::const_iterator id = m_ids.begin(); id != m_ids.end(); ++id) {
		text_map::const_iterator row = col.rows.find(id->first);
		if (col.rows.end() == row) {
			tab.add_empty_string();
			continue;
		}
		wide_string const &str = row->second;
		if (str.empty()) {
			tab.add_empty_string();
			continue;
		}

		// find suffix node indices and sequence length for every character position
		char_node.clear();
		char_node.reserve(str.length());
		{
			tree_type::node const *prev = &tree.root();
			for (wide_string::const_iterator pos = str.begin(); pos != str.end(); ++pos) {
				tree_type::position length;
				wide_string::const_iterator chr = pos;
				tree_type::node const *node = prev->link();
				if (node && node->parent()) {
					// non-root link found (previous suffix without the first character)
					length = node->length(tree.symbols(), true);
					wide_string::size_type const pos_len = static_cast<wide_string::size_type>(std::distance(pos,
																											 str.end()));
					if (length >= pos_len) {
						length = pos_len;
						chr = str.end();
					} else {
						std::advance(chr, length);
					}
				} else {
					// restart from root
					length = 0;
					node = &tree.root();
				}

				// find longest suffix
				for (; (length < max_sequence_length) && (chr != str.end()); ++chr) {
					tree_type::node::const_iterator next = node->find(*chr);
					if (node->end() == next) {
						// current node is a leaf node (occurs only once)
						break;
					}
					node = next->second;
					length = node->length(tree.symbols(), true);
					wide_string::size_type const chr_len = static_cast<wide_string::size_type>(std::distance(chr,
																											 str.end()));
					if (length >= chr_len) {
						length = chr_len;
						break;
					}
					tree_type::position const node_len = length - node->parent_depth();
					if (node_len > 1) {
						std::advance(chr, node_len - 1);
					}
				}

				if (length > max_sequence_length) {
					length = max_sequence_length;
				}
				char_node.push_back(std::make_pair(node->index(), length));
				prev = node;
			}
		}

		class build_sequence_function {
			// get rid of MSVC warning C4512: assignment operator could not be generated
			build_sequence_function &operator=(build_sequence_function const &) { return (*this); }  // = delete
		private:
			tree_type const &tree;
			std::vector<tree_type::position> const &node_weight;
			std::vector<symbol_info> &node_symbol;
			std::vector<char_info> const &char_node;
			std::vector<u32> &sym_tab;
			std::vector<u16> &str_seq;
		private:
			u16 insert_node_symbol(tree_type::node const &node, tree_type::position length) const {
				symbol_info &info = node_symbol[node.index()];
				u16 index = info.first;
				if (info.second >= length) {
					// symbols already added, resolve symbol links if the node has more
					for (u16 back = static_cast<u16>(info.second - static_cast<u16>(length)); back > 0; --back) {
						// bin_table::get_symbol_link(symbol)
						index = static_cast<u16>(sym_tab[index]);
					}
					return (index);
				}
				wide_string node_str = node.to_string(tree.symbols(), false);
				wide_string::size_type const node_len = length - node.parent_depth();
				if (node_str.length() > node_len) {
					node_str.resize(node_len);
				}
				wide_string::const_iterator node_chr = node_str.begin();
				if (0 == info.second) {
					// no symbols in the table, find/add the parent symbols
					index = insert_node_symbol(*node.parent(), node.parent_depth());
				} else {
					// skip characters for the existing symbols in the table
					u16 const skip = static_cast<u16>(info.second - static_cast<u16>(node.parent_depth()));
					std::advance(node_chr, skip);
				}
				for (; node_chr != node_str.end(); ++node_chr) {
					// bin_table::make_link_symbol(*chr, symbol)
					u32 const value = static_cast<u32>(static_cast<u32>(static_cast<u32>(*node_chr) << 16) + index);
					// tab.get_next_symbol()
					index = static_cast<u16>(sym_tab.size());
					sym_tab.push_back(value);
				}
				info.first = index;
				info.second = static_cast<u16>(length);
				return (index);
			}

			symbol_info::second_type get_symbol_count(tree_type::node const *node) const {
				symbol_info::second_type count = node_symbol[node->index()].second;
				while (0 == count) {
					node = node->parent();
					count = node_symbol[node->index()].second;
				}
				return (count);
			}

			tree_type::position
			max_char_length(char_info const &info, std::vector<char_info>::difference_type max_len) const {
				tree_type::position length = static_cast<tree_type::position>(std::min(info.second,
																					   static_cast<char_info::second_type>(max_len)));
				tree_type::node const *node = &tree.at(info.first);
				while (length <= node->parent_depth()) {
					node = node->parent();
				}
				symbol_info::second_type const count = get_symbol_count(node);
				if (count < length) {
					// make sure that we can added the required symbols to the table
					tree_type::position const space = std::vector<u32>::size_type(1 << 16) - sym_tab.size();
					if (length - count > space) {
						length = count + space;
					}
				}
				return (length);
			}

			tree_type::position get_char_rating(char_info const &info, char_info::second_type length) const {
				tree_type::position const weight = node_weight[info.first];
				tree_type::position rating = weight * length;
				if (rating < weight) {
					rating = std::numeric_limits<tree_type::position>::max();
				}
				return (rating);
			}

			void compress_char_nodes(std::vector<char_info>::const_iterator begin,
									 std::vector<char_info>::const_iterator end) const {
				std::vector<char_info>::const_iterator best_pos = begin;
				char_info::second_type best_length = max_char_length(*best_pos, std::distance(best_pos, end));
				tree_type::position best_rating = get_char_rating(*best_pos, best_length);
				for (std::vector<char_info>::const_iterator pos = begin + 1; pos != end; ++pos) {
					char_info::second_type const length = max_char_length(*pos, std::distance(pos, end));
					tree_type::position const rating = get_char_rating(*pos, length);
					if (best_rating <= rating) {
						if ((best_rating < rating) || (best_length < length)) {
							best_pos = pos;
							best_length = length;
							best_rating = rating;
						}
					}
				}
				{
					tree_type::node const *best_node = &tree.at(best_pos->first);
					while (best_length <= best_node->parent_depth()) {
						best_node = best_node->parent();
					}
					u16 const best_symbol = insert_node_symbol(*best_node, best_length);
					str_seq[static_cast<std::vector<u16>::size_type>(std::distance(char_node.begin(),
																				   best_pos))] = best_symbol;
				}
				if (best_pos != begin) {
					compress_char_nodes(begin, best_pos);
				}
				std::advance(best_pos, best_length);
				if (best_pos != end) {
					compress_char_nodes(best_pos, end);
				}
			}

		public:
			build_sequence_function(
					tree_type const &tree,
					std::vector<tree_type::position> const &node_weight,
					std::vector<symbol_info> &node_symbol,
					std::vector<char_info> const &char_node,
					std::vector<u32> &sym_tab,
					std::vector<u16> &str_seq)
					: tree(tree), node_weight(node_weight), node_symbol(node_symbol), char_node(char_node),
					  sym_tab(sym_tab), str_seq(str_seq) {
				str_seq.clear();
				str_seq.insert(str_seq.end(), char_node.size(), u16(0));
				compress_char_nodes(char_node.begin(), char_node.end());
				str_seq.erase(std::remove(str_seq.begin(), str_seq.end(), u16(0)), str_seq.end());
			}
		} build_sequence(tree, node_weight, node_symbol, char_node, tab.sym_tab, str_seq);

		tab.add_string_sequence(str_seq, ext);
	}
}

void
stringtable::pack_col_tree_node(column const& col, bin_table& tab, bool ext) const
{
	typedef std::map<u32, u16> key2sym_map;

	key2sym_map key2sym;
	// symbol #0->0 is added in pack_col()
	key2sym.insert(std::make_pair(u32(0), u16(0)));
	{
		typedef nicode::suffix_tree<wide_char, nicode::suffix_tree_traits<wide_char, wide_string, stb_allocator, u32> > tree_type;

		tree_type tree;
		tree.reserve(col.rows.size() * 64, col.rows.size() * 96);
		for (name_map::const_iterator id = m_ids.begin(); id != m_ids.end(); ++id) {
			text_map::const_iterator row = col.rows.find(id->first);
			if (row != col.rows.end()) {
				wide_string const& str = row->second;
				tree.append(str);
			}
		}
		tree.build();

		// ensure that all used UTF-16 codes are present as unlinked symbols
		for (tree_type::node const* node = tree.root().front(); node; node = node->sibling()) {
			tree_type::symbol const& symbol = tree.symbols()[node->edge().begin()];
			if (symbol.is_char()) {
				u32 const key = bin_table::make_char_symbol(symbol.get_char());
				if (key != 0) {
					key2sym.insert(std::make_pair(key, tab.get_next_symbol()));
					tab.add_symbol(key);
				}
			}
		}

		if (!tab.symbols_full()) {
			typedef std::vector<std::pair<std::size_t, tree_type::position> > weight_type;

			// build a sorted list of weighted node indices
			weight_type weight(tree.size(), std::make_pair(0, 0));
			for (tree_type::const_iterator iter = tree.begin(); iter != tree.end(); ++iter) {
				tree_type::node const& node = *iter;
				std::size_t const index = node.index();
				// initialize node index
				weight[index].first = index;
				// increase weight of all leaf parents
				if (node.empty()) {
					for (tree_type::node const* parent = node.parent(); parent && parent->parent(); parent = parent->parent()) {
						weight_type::value_type& pair = weight[parent->index()];
						weight_type::value_type::second_type const second = pair.second + 1;
						if (pair.second > second) {
							pair.second = std::numeric_limits<weight_type::value_type::second_type>::max();
						} else {
							pair.second = second;
						}
					}
				}
			}
			for (weight_type::iterator iter = weight.begin(); iter != weight.end(); ++iter) {
				weight_type::value_type& value = *iter;
				if (value.second > 0) {
					// lower weight for suffix link targets (same sequence without the first char)
					tree_type::node const* link = tree.at(value.first).link();
					if (link) {
						weight_type::value_type& target = weight[link->index()];
						if (target.second > 0) {
							target.second -= 1;
						}
					}
				}
			}
			for (weight_type::iterator iter = weight.begin(); iter != weight.end(); ++iter) {
				weight_type::value_type& value = *iter;
				if (value.second > 0) {
					tree_type::node const& node = tree.at(value.first);
					tree_type::position const length = node.length(tree.symbols(), true);
					if ((length <= 0) || (max_sequence_length < length)) {
						// root or too long
						value.second = 0;
					}
				}
			}
			std::sort(weight.begin(), weight.end(), pack_col_tree_node_sort_weight());

			// fill the symbol table
			std::vector<u16> node2sym(tree.size(), 0);
			struct add_node_type {
				tree_type const& m_tree;
				std::vector<u16>& m_node2sym;
				key2sym_map& m_key2sym;
				std::vector<u32>& m_table;
				add_node_type(tree_type const& tree, std::vector<u16>& node2sym, key2sym_map& key2sym, std::vector<u32>& table)
					: m_tree(tree)
					, m_node2sym(node2sym)
					, m_key2sym(key2sym)
					, m_table(table)
				{
				}
				u16 operator()(std::size_t index) const
				{
					u16 symbol = m_node2sym[index];
					if (0 == symbol) {
						tree_type::node const& node = m_tree.at(index);
						if (node.parent_depth() > 0) {
							symbol = operator()(node.parent()->index());
						}
						wide_string str = node.to_string(m_tree.symbols(), false);
						if (max_sequence_length < str.length()) {
							str.resize(max_sequence_length);
						}
						for (wide_string::const_iterator chr = str.begin(); (chr != str.end()) && (symbol != u16(-1)); ++chr) {
							u32 const key = static_cast<u32>(static_cast<u32>(static_cast<u32>(*chr) << 16) + symbol);
							std::pair<key2sym_map::iterator, bool> sym = m_key2sym.insert(std::make_pair(key, static_cast<u16>(m_table.size())));
							if (sym.second) {
								m_table.push_back(key);
							}
							symbol = sym.first->second;
						}
						m_node2sym[index] = symbol;
					}
					return (symbol);
				}
			private:
				add_node_type& operator=(add_node_type const&)
				{
					return (*this);
				}
			} add_node(tree, node2sym, key2sym, tab.sym_tab);
			// add/complete the unlinked symbols first
			for (tree_type::node const* node = tree.root().front(); node; node = node->sibling()) {
				if (node->length(tree.symbols(), false) > 0) {
					if (u16(-1) == add_node(node->index())) {
						break;
					}
				}
			}
			// add nodes by weight until table is full
			if (!tab.symbols_full()) {
				for (weight_type::const_iterator iter = weight.begin(); (iter != weight.end()) && (iter->second > 0); ++iter) {
					if ((u16(-1) == add_node(iter->first))) {
						break;
					}
				}
			}
		}
	}

	std::vector<u16> str_seq;
	for (name_map::const_iterator id = m_ids.begin(); id != m_ids.end(); ++id) {
		text_map::const_iterator row = col.rows.find(id->first);
		if (col.rows.end() == row) {
			tab.add_empty_string();
		} else {
			str_seq.clear();
			u16 seq_sym = 0;
			std::size_t seq_len = 0;
			wide_string const& str = row->second;
			for (wide_string::const_iterator chr = str.begin(); chr != str.end(); ++chr) {
				u32 const key = bin_table::make_link_symbol(*chr, seq_sym);
				key2sym_map::iterator sym = key2sym.find(key);
				if (sym != key2sym.end()) {
					// existing symbol found
					seq_sym = sym->second;
				} else {
					// add last found sequence first
					if (seq_sym) {
						str_seq.push_back(seq_sym);
					}
					// restart with unlinked symbol
					seq_sym = key2sym.find(bin_table::make_char_symbol(*chr))->second;
					seq_len = 0;
				}
				++seq_len;
			}
			// add last found sequence first
			if (seq_sym) {
				str_seq.push_back(seq_sym);
			}
			tab.add_string_sequence(str_seq, ext);
		}
	}
	/**/
}

void
stringtable::pack_col(column const& col, bin_table& tab, compression comp) const
{
	tab.str_tab.clear();
	tab.str_tab.reserve(m_ids.size());
	tab.seq_tab.clear();
	tab.sym_tab.clear();
	// the symbol table cannot be empty and
	// the symbol #0->0 is always added first
	tab.add_symbol(u32(0));
	if (col.rows.empty()) {
		tab.str_tab.insert(tab.str_tab.end(), m_ids.size(), u32(-1));
	} else {
		switch (comp) {
		case compression_none:
			pack_col_none(col, tab);
			break;
		case compression_fast:
			pack_col_fast(col, tab);
			break;
		case compression_lzpb:
			pack_col_lzpb(col, tab, false);
			break;
		case compression_lzex:
			pack_col_lzpb(col, tab, true);
			break;
		default:
		case compression_tree:
		case compression_best:
			// fill symbol table with 'best' suffix nodes first (more strings)
			pack_col_tree_node(col, tab, comp != compression_tree);
			if (tab.seq_tab.size() < tab.sym_tab.size()) {
				// add symbols while adding 'best' string nodes (less strings)
				tab.str_tab.clear();
				tab.seq_tab.clear();
				tab.sym_tab.clear();
				tab.add_symbol(u32(0));
				pack_col_tree_char(col, tab, comp != compression_tree);
			} else {
				//TODO: remove unused symbols from the table
			}
			break;
		}
	}
	// u32-align the size of the table data
	if (tab.get_next_sequence() % 2) {
		tab.add_sequence_end();
	}
}

void
stringtable::save_bin(platform bin_plat, u8 bin_vers, char const* bin_path, compression comp)
{
	if (bin_plat == platform_unknown) {
		bin_plat = platform_pc;
	}
	if (bin_vers == 0) {
		switch (bin_plat) {
		default:
		case platform_unknown:
		case platform_ps3:
		case platform_x360:
			bin_vers = 5;
			break;
		case platform_pc:
		case platform_x64:
		case platform_ps4:
		case platform_xone:
			bin_vers = 6;
			break;
		}
	}
	if (!bin_path || ('\0' == *bin_path)) {
		switch (bin_plat) {
		default:
		case platform_unknown:
		case platform_pc:
		case platform_x64:
			bin_path = "#G3:/data/compiled/localization/w_strings.bin";
			break;
		case platform_ps3:
			bin_path = "#PS3:/data/compiled/localization/x_strings.bin";
			break;
		case platform_ps4:
			bin_path= "#PS4:/data/compiled/localization/x_strings.bin";
			break;
		case platform_x360:
			bin_path = "#X360:/data/compiled/localization/x_strings.bin";
			break;
		case platform_xone:
			bin_path = "#XOne:/data/compiled/localization/x_strings.bin";
			break;
		}
	}
	std::string fname(bin_path);
	std::wcout << L"[" << to_wstring(fname) << L"]" << std::endl;
	filesystem::ensure_directories(fname.c_str());
	ofarchive ofa(fname.c_str(), bin_plat);
	if (!ofa) {
		throw std::runtime_error("failed to create binary string table");
	}
	bin_header hdr(bin_vers);
	hdr.src_count = static_cast<archive::streamsize>(m_src.size());
	hdr.col_count = static_cast<archive::streamsize>(m_col.size());
	hdr.row_count = static_cast<archive::streamsize>(m_ids.size());
	std::vector<bin_source> src_tab; src_tab.reserve(m_src.size());
	std::vector<archive::streamref> col_str(m_col.size());
	std::vector<bin_column> col_tab(m_col.size());
	archive::streamref key_ref;
	std::vector<string_hash> key_tab; key_tab.reserve(m_ids.size());
	std::vector<bin_table> str_tab;
	{
		onarchive ona;
		std::wcout << L"target=" << to_wstring(std::string(platform_name(bin_plat))) << std::endl;
		// header
		hdr.write(ona);
		std::wcout << L"version=" << to_wstring(hdr.version()) << std::endl;
		std::wcout << L"strings=" << to_wstring(m_ids.size()) << std::endl;
		std::wcout << L"columns=" << to_wstring(m_col.size()) << std::endl;
		// source table
		hdr.src_table = ona.tellp();
		for (src_list::const_iterator psrc = m_src.begin(); psrc != m_src.end(); ++psrc) {
			bin_source src(*psrc);
			src_tab.push_back(src);
			src.write(ona);
		}
		while (ona && (ona.tellp() % sizeof(u32))) {
			ona << u8(0);
		}
		// column names
		hdr.col_names = ona.tellp();
		ona << col_str;
		for (col_list::const_iterator pcol = m_col.begin(); pcol != m_col.end(); ++pcol) {
			byte_string const& str = pcol->name;
			write_ref_string(ona, str, col_str[static_cast<std::vector<archive::streamref>::size_type>(pcol - m_col.begin())], true, archive::streamsize(sizeof(u32)));
		}
		// column table
		hdr.col_table = ona.tellp();
		ona.write(&col_tab[0].str_tab.size, hdr.col_count * 4);
		// key table
		hdr.key_table = ona.tellp();
		ona << key_ref;
		key_ref = ona.ref_begin();
		for (name_map::const_iterator pkey = m_ids.begin(); pkey != m_ids.end(); ++pkey) {
			key_tab.push_back(pkey->first);
		}
		ona << key_tab;
		ona.ref_end(key_ref);
		// string tables
		std::size_t empty_tab = std::size_t(-1);
		for (std::size_t i = 0; i < col_tab.size(); ++i) {
			column const& col = m_col[i];
			bin_column& bin = col_tab[i];
			std::wcout << L"column." << to_wstring(i) << L".name=" << to_wstring(col.name) << std::endl;
			if (col.rows.empty() && (empty_tab != std::size_t(-1))) {
				bin = col_tab[empty_tab];
			} else {
				bin_table& tab = *str_tab.insert(str_tab.end(), bin_table());

				pack_col(col, tab, comp);

				bin.str_tab = ona.ref_begin();
				ona << tab.str_tab;
				ona << tab.seq_tab;
				ona.ref_end(bin.str_tab);
				bin.sym_tab = ona.ref_begin();
				ona << tab.sym_tab;
				ona.ref_end(bin.sym_tab);
				if (col.rows.empty()) {
					if (std::size_t(-1) == empty_tab) {
						// save index to merge the next empty column
						empty_tab = i;
					}
				} else {
					std::wcout << L"column." << to_wstring(i) << L".seq_avg=" << std::fixed << ((double)tab.seq_tab.size() / (double)col.rows.size()) << std::endl;
					std::wcout << L"column." << to_wstring(i) << L".seq_num=" << to_wstring(tab.seq_tab.size()) << std::endl;
					std::wcout << L"column." << to_wstring(i) << L".sym_num=" << to_wstring(tab.sym_tab.size()) << std::endl;
				}
			}
		}
	}
	// header
	hdr.write(ofa);
	// source table
	for (std::vector<bin_source>::const_iterator psrc = src_tab.begin(); psrc != src_tab.end(); ++psrc) {
		psrc->write(ofa);
	}
	while (ofa && (ofa.tellp() % sizeof(u32))) {
		ofa << u8(0);
	}
	// column names
	ofa << col_str;
	for (col_list::const_iterator pcol = m_col.begin(); pcol != m_col.end(); ++pcol) {
		archive::streamref ref;
		write_ref_string(ofa, pcol->name, ref, true, sizeof(u32));
	}
	// column table
	ofa.write(&col_tab[0].str_tab.size, hdr.col_count * 4);
	// key table
	ofa << key_ref;
	ofa << key_tab;
	// strings tables
	for (std::vector<bin_table>::const_iterator ptab = str_tab.begin(); ptab != str_tab.end(); ++ptab) {
		ofa << ptab->str_tab;
		ofa << ptab->seq_tab;
		ofa << ptab->sym_tab;
	}
	if (!ofa) {
		throw std::runtime_error("failed to write binary string table");
	}
	std::wcout << std::endl;
}

} // namespace genome::localization
} // namespace genome
