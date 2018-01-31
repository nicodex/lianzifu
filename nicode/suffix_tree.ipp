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
#ifndef NICODE_SUFFIX_TREE_IPP
#define NICODE_SUFFIX_TREE_IPP

#include <cassert>
#include <cstdio>
#include <algorithm>
#include <iterator>
#include <limits>
#include <new>
#include <stdexcept>
#include <utility>

namespace nicode {

//
// suffix_tree::symbol
//

template<typename CharT, typename Traits>
suffix_tree<CharT, Traits>::symbol::symbol(char_type c)
	: m_cd(delimiter(c) & char_mask)
{
	if (Traits::verify_char_symbol) {
		if (get_char() != c) {
			throw std::runtime_error("unsupported suffix tree character");
		}
	}
}

template<typename CharT, typename Traits>
suffix_tree<CharT, Traits>::symbol::symbol(delimiter d)
	: m_cd(d | delimiter_flag)
{
	if (d > max_delimiter) {
		throw std::overflow_error("suffix tree delimiter overflow");
	}
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::symbol::is_char(void) const
{
	return (0 == (delimiter_flag & m_cd));
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::symbol::is_delimiter(void) const
{
	return ((delimiter_flag & m_cd) != 0);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::char_type
suffix_tree<CharT, Traits>::symbol::get_char(void) const
{
	assert(is_char());
	return (char_type(m_cd & char_mask));
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::delimiter
suffix_tree<CharT, Traits>::symbol::get_delimiter(void) const
{
	assert(is_delimiter());
	return (m_cd & delimiter_mask);
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::symbol::operator<(symbol const& rhs) const
{
	return (m_cd < rhs.m_cd);
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::symbol::operator==(symbol const& rhs) const
{
	return (m_cd == rhs.m_cd);
}

//
// suffix_tree::substring
//

template<typename CharT, typename Traits>
suffix_tree<CharT, Traits>::substring::substring(position begin, position end)
	: m_begin(begin)
	, m_end(end)
{
	assert(begin <= end);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::position
suffix_tree<CharT, Traits>::substring::size(void) const
{
	return (m_end - m_begin);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::position
suffix_tree<CharT, Traits>::substring::begin(void) const
{
	return (m_begin);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::position
suffix_tree<CharT, Traits>::substring::end(void) const
{
	return (m_end);
}

//
// suffix_tree::node
//

template<typename CharT, typename Traits>
suffix_tree<CharT, Traits>::node::node(std::size_t index)
	: m_link(0)
	, m_parent(0)
	, m_sibling(0)
	, m_edge(0, 0)
	, m_children()
	, m_parent_depth(0)
	, m_index(index)
{
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::node const*
suffix_tree<CharT, Traits>::node::link(void) const
{
	return (m_link);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::node const*
suffix_tree<CharT, Traits>::node::parent(void) const
{
	return (m_parent);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::node const*
suffix_tree<CharT, Traits>::node::sibling(void) const
{
	return (m_sibling);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::substring const&
suffix_tree<CharT, Traits>::node::edge(void) const
{
	return (m_edge);
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::node::empty(void) const
{
	return (m_children.empty());
}

template<typename CharT, typename Traits>
std::size_t
suffix_tree<CharT, Traits>::node::size(void) const
{
	return (m_children.size());
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::node::const_iterator
suffix_tree<CharT, Traits>::node::begin(void) const
{
	return (m_children.begin());
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::node::const_iterator
suffix_tree<CharT, Traits>::node::end(void) const
{
	return (m_children.end());
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::node const*
suffix_tree<CharT, Traits>::node::front(void) const
{
	const_iterator i = begin();
	if (end() == i) {
		return (0);
	}
	return (i->second);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::node::const_iterator
suffix_tree<CharT, Traits>::node::find(symbol const& s) const
{
	return (m_children.find(s));
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::node const&
suffix_tree<CharT, Traits>::node::at(symbol const& s) const
{
	const_iterator i = find(s);
	if (end() == i) {
		throw std::out_of_range("suffix tree child does not exists");
	}
	return (*i->second);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::position
suffix_tree<CharT, Traits>::node::depth(void) const
{
	return (m_parent_depth + m_edge.size());
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::position
suffix_tree<CharT, Traits>::node::parent_depth(void) const
{
	return (m_parent_depth);
}

template<typename CharT, typename Traits>
std::size_t
suffix_tree<CharT, Traits>::node::index(void) const
{
	return (m_index);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::position
suffix_tree<CharT, Traits>::node::length(symbol_string const& s, bool full) const
{
	position n = full ? parent_depth() : 0;
	for (position i = m_edge.begin(); i != m_edge.end(); ++i) {
		if (s[i].is_delimiter()) {
			break;
		}
		++n;
	}
	return (n);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::string_type
suffix_tree<CharT, Traits>::node::to_string(symbol_string const& s, bool full) const
{
	string_type str;
	if (full && parent()) {
		str = parent()->to_string(s, full);
	}
	for (position i = m_edge.begin(); i != m_edge.end(); ++i) {
		symbol const& c = s[i];
		if (c.is_delimiter()) {
			break;
		}
		str += c.get_char();
	}
	return (str);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::node*
suffix_tree<CharT, Traits>::node::add_child(node* c, substring const& e, symbol_string const& s)
{
	std::pair<iterator, bool> p = m_children.insert(std::make_pair(s[e.begin()], c));
	iterator const& i = p.first;
	if (!p.second) {
		i->second->m_sibling = 0;
		i->second = c;
	}
	c->m_parent = this;
	if (i != m_children.begin()) {
		iterator prev = i; --prev;
		prev->second->m_sibling = c;
	}
	iterator next = i; ++next;
	c->m_sibling = (next == m_children.end()) ? 0 : next->second;
	c->m_edge = e;
	c->m_parent_depth = depth();
	return (c);
}

//
// suffix_tree::const_iterator
//

template<typename CharT, typename Traits>
suffix_tree<CharT, Traits>::const_iterator::const_iterator(void)
	: m_iter()
{
}

template<typename CharT, typename Traits>
suffix_tree<CharT, Traits>::const_iterator::const_iterator(iter_type const& iter)
	: m_iter(iter)
{
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_iterator::reference
suffix_tree<CharT, Traits>::const_iterator::operator*(void) const
{
	return (**m_iter);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_iterator::pointer
suffix_tree<CharT, Traits>::const_iterator::operator->(void) const
{
	return (*m_iter);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_iterator::reference
suffix_tree<CharT, Traits>::const_iterator::operator[](difference_type rhs) const
{
	return (**m_iter[rhs]);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_iterator&
suffix_tree<CharT, Traits>::const_iterator::operator++(void)
{
	++m_iter;
	return (*this);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_iterator
suffix_tree<CharT, Traits>::const_iterator::operator++(int)
{
	iter_type iter = m_iter;
	++*this;
	return (const_iterator(iter));
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_iterator&
suffix_tree<CharT, Traits>::const_iterator::operator--(void)
{
	--m_iter;
	return (*this);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_iterator
suffix_tree<CharT, Traits>::const_iterator::operator--(int)
{
	iter_type iter = m_iter;
	--*this;
	return (const_iterator(iter));
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_iterator&
suffix_tree<CharT, Traits>::const_iterator::operator+=(difference_type rhs)
{
	m_iter += rhs;
	return (*this);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_iterator
suffix_tree<CharT, Traits>::const_iterator::operator+(difference_type rhs) const
{
	return (const_iterator(m_iter + rhs));
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_iterator&
suffix_tree<CharT, Traits>::const_iterator::operator-=(difference_type rhs)
{
	m_iter -= rhs;
	return (*this);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_iterator
suffix_tree<CharT, Traits>::const_iterator::operator-(difference_type rhs) const
{
	return (const_iterator(m_iter - rhs));
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_iterator::difference_type
suffix_tree<CharT, Traits>::const_iterator::operator-(const_iterator const& rhs) const
{
	return (m_iter - rhs.m_iter);
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::const_iterator::operator==(const_iterator const& rhs) const
{
	return (m_iter == rhs.m_iter);
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::const_iterator::operator!=(const_iterator const& rhs) const
{
	return (m_iter != rhs.m_iter);
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::const_iterator::operator<(const_iterator const& rhs)
{
	return (m_iter < rhs.m_iter);
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::const_iterator::operator<=(const_iterator const& rhs)
{
	return (m_iter <= rhs.m_iter);
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::const_iterator::operator>(const_iterator const& rhs)
{
	return (m_iter > rhs.m_iter);
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::const_iterator::operator>=(const_iterator const& rhs)
{
	return (m_iter >= rhs.m_iter);
}

//
// suffix_tree::const_dfs_iterator
//

template<typename CharT, typename Traits>
suffix_tree<CharT, Traits>::const_dfs_iterator::const_dfs_iterator(void)
	: m_iter(0)
{
}

template<typename CharT, typename Traits>
suffix_tree<CharT, Traits>::const_dfs_iterator::const_dfs_iterator(iter_type const& iter)
	: m_iter(iter)
{
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_dfs_iterator::reference
suffix_tree<CharT, Traits>::const_dfs_iterator::operator*(void) const
{
	return (*m_iter);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_dfs_iterator::pointer
suffix_tree<CharT, Traits>::const_dfs_iterator::operator->(void) const
{
	return (m_iter);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_dfs_iterator&
suffix_tree<CharT, Traits>::const_dfs_iterator::operator++(void)
{
	if (m_iter) {
		node const* p = m_iter->parent();
		if (!p || !m_iter->empty()) {
			m_iter = m_iter->front();
		} else {
			m_iter = m_iter->sibling();
			while (!m_iter && p) {
				m_iter = p->sibling();
				p = p->parent();
			}
		}
	}
	return (*this);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_dfs_iterator
suffix_tree<CharT, Traits>::const_dfs_iterator::operator++(int) const
{
	iter_type iter = m_iter;
	++*this;
	return (const_dfs_iterator(iter));
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::const_dfs_iterator::operator==(const_dfs_iterator const& rhs) const
{
	return (rhs.m_iter == m_iter);
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::const_dfs_iterator::operator!=(const_dfs_iterator const& rhs) const
{
	return (rhs.m_iter != m_iter);
}

//
// suffix_tree::const_bfs_iterator
//

template<typename CharT, typename Traits>
suffix_tree<CharT, Traits>::const_bfs_iterator::const_bfs_iterator(void)
	: m_iter(0)
	, m_queue()
{
}

template<typename CharT, typename Traits>
suffix_tree<CharT, Traits>::const_bfs_iterator::const_bfs_iterator(iter_type const& iter)
	: m_iter(iter)
	, m_queue()
{
	if (m_iter) {
		for (iter_type i = m_iter->front(); i; i = i->sibling()) {
			m_queue.push_back(i);
		}
	}
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_bfs_iterator::reference
suffix_tree<CharT, Traits>::const_bfs_iterator::operator*(void) const
{
	return (*m_iter);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_bfs_iterator::pointer
suffix_tree<CharT, Traits>::const_bfs_iterator::operator->(void) const
{
	return (m_iter);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_bfs_iterator&
suffix_tree<CharT, Traits>::const_bfs_iterator::operator++(void)
{
	if (m_iter) {
		if (m_queue.empty()) {
			m_iter = 0;
		} else {
			m_iter = m_queue.front(); m_queue.pop_front();
			iter_type i = m_iter->front();
			if (i) {
				do {
					m_queue.push_back(i);
					i = i->sibling();
				} while (i);
			}
		}
	}
	return (*this);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_bfs_iterator
suffix_tree<CharT, Traits>::const_bfs_iterator::operator++(int) const
{
	iter_type iter = m_iter;
	++*this;
	return (const_bfs_iterator(iter));
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::const_bfs_iterator::operator==(const_bfs_iterator const& rhs) const
{
	return (rhs.m_iter == m_iter);
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::const_bfs_iterator::operator!=(const_bfs_iterator const& rhs) const
{
	return (rhs.m_iter != m_iter);
}

//
// suffix_tree::const_bfs_sort_iterator
//

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::const_bfs_sort_iterator::compare::operator()(iter_type const& a, iter_type const& b) const
{
	return (a->parent_depth() < b->parent_depth());
}

template<typename CharT, typename Traits>
suffix_tree<CharT, Traits>::const_bfs_sort_iterator::const_bfs_sort_iterator(void)
	: m_iter(0)
	, m_queue()
{
}

template<typename CharT, typename Traits>
suffix_tree<CharT, Traits>::const_bfs_sort_iterator::const_bfs_sort_iterator(iter_type const& iter)
	: m_iter(iter)
	, m_queue()
{
	if (m_iter) {
		for (iter_type i = m_iter->front(); i; i = i->sibling()) {
			m_queue.push_back(i);
		}
	}
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_bfs_sort_iterator::reference
suffix_tree<CharT, Traits>::const_bfs_sort_iterator::operator*(void) const
{
	return (*m_iter);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_bfs_sort_iterator::pointer
suffix_tree<CharT, Traits>::const_bfs_sort_iterator::operator->(void) const
{
	return (m_iter);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_bfs_sort_iterator&
suffix_tree<CharT, Traits>::const_bfs_sort_iterator::operator++(void)
{
	if (m_iter) {
		if (m_queue.empty()) {
			m_iter = 0;
		} else {
			m_iter = m_queue.front(); m_queue.pop_front();
			iter_type i = m_iter->front();
			if (i) {
				do {
					m_queue.push_back(i);
					i = i->sibling();
				} while (i);
				std::stable_sort(m_queue.begin(), m_queue.end(), compare());
			}
		}
	}
	return (*this);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_bfs_sort_iterator
suffix_tree<CharT, Traits>::const_bfs_sort_iterator::operator++(int) const
{
	iter_type iter = m_iter;
	++*this;
	return (const_bfs_sort_iterator(iter));
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::const_bfs_sort_iterator::operator==(const_bfs_sort_iterator const& rhs) const
{
	return (rhs.m_iter == m_iter);
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::const_bfs_sort_iterator::operator!=(const_bfs_sort_iterator const& rhs) const
{
	return (rhs.m_iter != m_iter);
}

//
// suffix_tree
//

template<typename CharT, typename Traits>
suffix_tree<CharT, Traits>::suffix_tree(void)
	: m_delim(0)
	, m_symbols()
	, m_nodes(1, new_node(0))
{
}

template<typename CharT, typename Traits>
suffix_tree<CharT, Traits>::suffix_tree(string_type const& s)
	: m_delim(0)
	, m_symbols()
	, m_nodes(1, new_node(0))
{
	append(s);
	build();
}

template<typename CharT, typename Traits>
suffix_tree<CharT, Traits>::~suffix_tree(void)
{
	delete_nodes();
}

template<typename CharT, typename Traits>
void
suffix_tree<CharT, Traits>::reserve(std::size_t symbols, std::size_t nodes)
{
	m_symbols.reserve(symbols);
	// worst-case (Fibonacci word) has 2*symbols nodes
	std::size_t max_nodes = symbols * 2;
	if (max_nodes < symbols) {
		max_nodes = std::numeric_limits<std::size_t>::max();
	}
	m_nodes.reserve(std::min(nodes, max_nodes));
}

template<typename CharT, typename Traits>
void
suffix_tree<CharT, Traits>::append(string_type const& s)
{
	std::copy(s.begin(), s.end(), std::back_inserter(m_symbols));
	m_symbols.push_back(symbol(m_delim++));
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::delimiter
suffix_tree<CharT, Traits>::string_count(void) const
{
	return (m_delim);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::symbol_string const&
suffix_tree<CharT, Traits>::symbols(void) const
{
	return (m_symbols);
}

template<typename CharT, typename Traits>
void
suffix_tree<CharT, Traits>::build(void)
{
	node* root = clear_nodes();
	if (m_symbols.empty()) {
		return;
	}
	struct suffix {
		node* m_node;
		suffix(node* n)
			: m_node(n)
		{
		}
		void link_to(node* n)
		{
			if (m_node) {
				m_node->m_link = n;
				m_node = 0;
			}
		}
	} suffix(0);
	bool walk_down = true;
	node* current = root->add_child(add_node(), substring(0, m_symbols.size()), m_symbols);
	for (position j = 1, i = 0; i < m_symbols.size() - 1; ++i) {
		symbol const& next_sym = m_symbols[i + 1];
		for (; j <= i + 1; ++j) {
			if (walk_down) {
				if (current->m_parent && !current->m_link) {
					current = current->m_parent;
				}
				current = current->m_link ? current->m_link : root;
				if (j <= i) {
					position p = j + current->depth();
					position d = i - j;
					while (d >= current->depth()) {
						current = current->m_children.find(m_symbols[p])->second;
						p += current->edge().size();
					}
				}
			}
			walk_down = true;
			position length = i + 1 - j;
			if (length == current->depth()) {
				suffix.link_to(current);
				typename node::iterator next = current->m_children.find(next_sym);
				if (next != current->m_children.end()) {
					current = next->second;
					walk_down = false;
					break;
				} else {
					position begin = i + 1;
					current->add_child(add_node(), substring(begin, m_symbols.size()), m_symbols);
				}
			} else {
				position b = current->edge().begin() - current->parent_depth();
				position pos = b + length;
				if (next_sym == m_symbols[pos]) {
					suffix.link_to(current);
					if (!current->empty() || j != b) {
						walk_down = false;
						break;
					}
				} else {
					position begin = current->edge().begin();
					position end = i + 1;
					node* split = current->m_parent->add_child(add_node(), substring(begin, pos), m_symbols);
					split->add_child(current, substring(pos, current->edge().end()), m_symbols);
					split->add_child(add_node(), substring(end, m_symbols.size()), m_symbols);
					suffix.link_to(split);
					if (1 == split->depth()) {
						split->m_link = root;
					} else {
						suffix = split;
					}
					current = split;
				}
			}
		}
	}
}

template<typename CharT, typename Traits>
std::size_t
suffix_tree<CharT, Traits>::size(void) const
{
	return (m_nodes.size());
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::node const&
suffix_tree<CharT, Traits>::root(void) const
{
	return (*m_nodes.front());
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::node const&
suffix_tree<CharT, Traits>::at(std::size_t i) const
{
	return (*m_nodes[i]);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_iterator
suffix_tree<CharT, Traits>::begin(void) const
{
	return (const_iterator(m_nodes.begin()));
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_iterator
suffix_tree<CharT, Traits>::end(void) const
{
	return (const_iterator(m_nodes.end()));
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_dfs_iterator
suffix_tree<CharT, Traits>::dfs_begin(void) const
{
	return (const_dfs_iterator(m_nodes.front()));
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_dfs_iterator
suffix_tree<CharT, Traits>::dfs_end(void) const
{
	return (const_dfs_iterator());
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_bfs_iterator
suffix_tree<CharT, Traits>::bfs_begin(void) const
{
	return (const_bfs_iterator(m_nodes.front()));
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_bfs_iterator
suffix_tree<CharT, Traits>::bfs_end(void) const
{
	return (const_bfs_iterator());
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_bfs_sort_iterator
suffix_tree<CharT, Traits>::bfs_sort_begin(void) const
{
	return (const_bfs_sort_iterator(m_nodes.front()));
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::const_bfs_sort_iterator
suffix_tree<CharT, Traits>::bfs_sort_end(void) const
{
	return (const_bfs_sort_iterator());
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::is_root::operator()(node const& n) const
{
	return (!n.parent());
}

template<typename CharT, typename Traits>
bool
suffix_tree<CharT, Traits>::is_leaf::operator()(node const& n) const
{
	return (n.empty() && n.parent());
}

template<typename CharT, typename Traits>
void
suffix_tree<CharT, Traits>::clear(void)
{
	m_delim = 0;
	m_symbols.clear();
	clear_nodes();
	// self-copy-swap to free memory (reserved capacity)
	symbol_string(m_symbols).swap(m_symbols);
	node_pointers(m_nodes).swap(m_nodes);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::node*
suffix_tree<CharT, Traits>::add_node(void)
{
	typename node_pointers::iterator i = m_nodes.insert(m_nodes.end(), new_node(std::size_t(-1)));
	typename node_pointers::iterator::reference n = *i;
	n->m_index = static_cast<std::size_t>(i - m_nodes.begin());
	return (n);
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::node*
suffix_tree<CharT, Traits>::new_node(std::size_t index) const
{
	return (new(typename rebind_allocator<node>::other().allocate(1)) node(index));
}

template<typename CharT, typename Traits>
void
suffix_tree<CharT, Traits>::delete_nodes(void)
{
	for (typename node_pointers::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i) {
		typename node_pointers::iterator::reference n = *i;
		if (n) {
			node* const p = n;
			n = 0;
			p->~node();
			typename rebind_allocator<node>::other().deallocate(p, 1);
		}

	}
}

template<typename CharT, typename Traits>
typename suffix_tree<CharT, Traits>::node*
suffix_tree<CharT, Traits>::clear_nodes(void)
{
	delete_nodes();
	m_nodes.clear();
	return (add_node());
}

} // namespace nicode

#endif // NICODE_SUFFIX_TREE_IPP
