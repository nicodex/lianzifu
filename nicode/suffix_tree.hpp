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
#ifndef NICODE_SUFFIX_TREE_HPP
#define NICODE_SUFFIX_TREE_HPP

#include <climits>
#include <cstddef>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <deque>

namespace nicode {

template<
	typename CharT,
	typename StringT = std::basic_string<CharT>,
	template<typename> class AllocatorT = std::allocator,
	typename DelimiterT = std::size_t,
	typename PositionT = std::size_t>
struct suffix_tree_traits {
	typedef CharT char_type;
	typedef StringT string_type;
	typedef AllocatorT<void> allocator;
	typedef DelimiterT delimiter;
	typedef PositionT position;
	typedef bool assert_char_type[(
		std::numeric_limits<delimiter>::is_integer) ? 1 : -1];
	typedef bool assert_delimiter_type[(
		std::numeric_limits<delimiter>::is_integer &&
		!std::numeric_limits<delimiter>::is_signed) &&
		(sizeof(delimiter) >= sizeof(char_type)) ? 1 : -1];
	enum config {
		verify_char_symbol = bool(sizeof(char_type) == sizeof(delimiter))
	};
};

//
// Generalized suffix tree (using Ukkonen's algorithm)
//
//  CharT is expected to be smaller than delimiter (the most
//  significant bit is used to distinguish between character
//  and delimiter symbols). If CharT and delimiter are equal
//  in size, an exception is thrown for characters with MSB.
//  Therefore UTF-32 code points (char32_t / 32-bit wchar_t)
//  have to be validated (U+0000,U+10FFFF) before the string
//  is added to the tree, if delimiter (size_t) has 32 bits.
//
template<typename CharT, typename Traits = suffix_tree_traits<CharT> >
class suffix_tree {
	suffix_tree(suffix_tree const&);  // = delete;
	suffix_tree& operator=(suffix_tree const&);  // = delete;
public:
	typedef typename Traits::char_type char_type;
	typedef typename Traits::allocator allocator;
	typedef typename Traits::string_type string_type;
	typedef typename Traits::delimiter delimiter;
	typedef typename Traits::position position;
	template<typename T>
	struct rebind_allocator {
		typedef typename allocator::template rebind<T>::other other;
	};

	class symbol {
	public:
		symbol(char_type c);
		explicit symbol(delimiter d);
		bool is_char(void) const;
		bool is_delimiter(void) const;
		char_type get_char(void) const;
		delimiter get_delimiter(void) const;
		bool operator<(symbol const& rhs) const;
		bool operator==(symbol const& rhs) const;
		enum limits
		#if (__cplusplus >= 201103L) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 201103L))
			: delimiter
		#endif
		{
			max_delimiter = delimiter(delimiter(-1) >> 1)
		};
	private:
		delimiter m_cd;
		enum bits
		#if (__cplusplus >= 201103L) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 201103L))
			: delimiter
		#endif
		{
			delimiter_mask = delimiter(max_delimiter),
			delimiter_flag = delimiter(~delimiter_mask),
			char_mask = delimiter(delimiter_mask & (delimiter(-1) >>
				((sizeof(delimiter) - sizeof(char_type)) * CHAR_BIT)))
		};
	};
	typedef std::vector<symbol
		, typename rebind_allocator<symbol>::other
		> symbol_string;

	class substring {
	public:
		substring(position begin, position end);
		position size(void) const;
		position begin(void) const;
		position end(void) const;
	private:
		position m_begin;
		position m_end;
	};

	class node {
		friend class suffix_tree;
		node(std::size_t index);
		node(node const&);  // = delete;
		node& operator=(node const&);  // = delete;
		typedef std::map<symbol, node*
			, std::less<symbol>
			, typename rebind_allocator<std::pair<symbol const, node*> >::other
			> child_map;
		typedef typename child_map::iterator iterator;
	public:
		typedef typename child_map::const_iterator const_iterator;
		node const* link(void) const;
		node const* parent(void) const;
		node const* sibling(void) const;
		substring const& edge(void) const;
		bool empty(void) const;
		std::size_t size(void) const;
		const_iterator begin(void) const;
		const_iterator end(void) const;
		node const* front(void) const;
		const_iterator find(symbol const& s) const;
		node const& at(symbol const& s) const;
		position depth(void) const;
		position parent_depth(void) const;
		std::size_t index(void) const;
		position length(symbol_string const& s, bool full = true) const;
		string_type to_string(symbol_string const& s, bool full = true) const;
	private:
		node* add_child(node* c, substring const& e, symbol_string const& s);
	private:
		node* m_link;
		node* m_parent;
		node* m_sibling;
		substring m_edge;
		child_map m_children;
		position m_parent_depth;
		std::size_t m_index;
	};
	typedef std::vector<node*
		, typename rebind_allocator<node*>::other
		> node_pointers;

	// random access const node reference iterator
	class const_iterator {
		typedef typename node_pointers::const_iterator iter_type;
	public:
		typedef std::random_access_iterator_tag iterator_category;
		typedef node const value_type;
		typedef typename iter_type::difference_type difference_type;
		typedef node const* pointer;
		typedef node const& reference;
		const_iterator(void);
		const_iterator(iter_type const& iter);
		reference operator*(void) const;
		pointer operator->(void) const;
		reference operator[](difference_type rhs) const;
		const_iterator& operator++(void);
		const_iterator operator++(int);
		const_iterator& operator--(void);
		const_iterator operator--(int);
		const_iterator& operator+=(difference_type rhs);
		const_iterator operator+(difference_type rhs) const;
		friend  // static
		const_iterator operator+(difference_type lhs, const_iterator const& rhs)
		{
			return (rhs.operator+(lhs));
		}
		const_iterator& operator-=(difference_type rhs);
		const_iterator operator-(difference_type rhs) const;
		difference_type operator-(const_iterator const& rhs) const;
		bool operator==(const_iterator const& rhs) const;
		bool operator!=(const_iterator const& rhs) const;
		bool operator<(const_iterator const& rhs);
		bool operator<=(const_iterator const& rhs);
		bool operator>(const_iterator const& rhs);
		bool operator>=(const_iterator const& rhs);
	private:
		iter_type m_iter;
	};
	// depth-first search const node reference iterator
	class const_dfs_iterator {
		typedef node const* iter_type;
	public:
		typedef std::forward_iterator_tag iterator_category;
		typedef node const value_type;
		typedef std::ptrdiff_t difference_type;
		typedef node const* pointer;
		typedef node const& reference;
		const_dfs_iterator(void);
		const_dfs_iterator(iter_type const& iter);
		reference operator*(void) const;
		pointer operator->(void) const;
		const_dfs_iterator& operator++(void);
		const_dfs_iterator operator++(int) const;
		bool operator==(const_dfs_iterator const& rhs) const;
		bool operator!=(const_dfs_iterator const& rhs) const;
	private:
		iter_type m_iter;
	};
	// breadth-first search const node reference iterator (node hierarchy)
	class const_bfs_iterator {
		typedef node const* iter_type;
	public:
		typedef std::forward_iterator_tag iterator_category;
		typedef node const value_type;
		typedef std::ptrdiff_t difference_type;
		typedef node const* pointer;
		typedef node const& reference;
		const_bfs_iterator(void);
		const_bfs_iterator(iter_type const& iter);
		reference operator*(void) const;
		pointer operator->(void) const;
		const_bfs_iterator& operator++(void);
		const_bfs_iterator operator++(int) const;
		bool operator==(const_bfs_iterator const& rhs) const;
		bool operator!=(const_bfs_iterator const& rhs) const;
	private:
		iter_type m_iter;
		std::deque<iter_type> m_queue;
	};
	// breadth-first search const node reference iterator (parent depth)
	class const_bfs_sort_iterator {
		typedef node const* iter_type;
		struct compare {
			bool operator()(iter_type const& a, iter_type const& b) const;
		};
	public:
		typedef std::forward_iterator_tag iterator_category;
		typedef node const value_type;
		typedef std::ptrdiff_t difference_type;
		typedef node const* pointer;
		typedef node const& reference;
		const_bfs_sort_iterator(void);
		const_bfs_sort_iterator(iter_type const& iter);
		reference operator*(void) const;
		pointer operator->(void) const;
		const_bfs_sort_iterator& operator++(void);
		const_bfs_sort_iterator operator++(int) const;
		bool operator==(const_bfs_sort_iterator const& rhs) const;
		bool operator!=(const_bfs_sort_iterator const& rhs) const;
	private:
		iter_type m_iter;
		std::deque<iter_type> m_queue;
	};

public:
	suffix_tree(void);
	suffix_tree(string_type const& s);  // implicit build()
	~suffix_tree(void);
	void reserve(std::size_t symbols, std::size_t nodes = std::size_t(-1));
	void append(string_type const& s);
	delimiter string_count(void) const;
	symbol_string const& symbols(void) const;
	void build(void);
	std::size_t size(void) const;
	node const& root(void) const;
	node const& at(std::size_t i) const;
	const_iterator begin(void) const;
	const_iterator end(void) const;
	const_dfs_iterator dfs_begin(void) const;
	const_dfs_iterator dfs_end(void) const;
	const_bfs_iterator bfs_begin(void) const;
	const_bfs_iterator bfs_end(void) const;
	const_bfs_sort_iterator bfs_sort_begin(void) const;
	const_bfs_sort_iterator bfs_sort_end(void) const;
	struct is_root {
		bool operator()(node const& n) const;
	};
	struct is_leaf {
		bool operator()(node const& n) const;
	};
	void clear(void);
private:
	node* add_node(void);
	node* new_node(std::size_t index) const;
	void delete_nodes(void);
	node* clear_nodes(void);
private:
	delimiter m_delim;
	symbol_string m_symbols;
	node_pointers m_nodes;
};

} // namespace nicode

#include <nicode/suffix_tree.ipp>

#endif // NICODE_SUFFIX_TREE_HPP
