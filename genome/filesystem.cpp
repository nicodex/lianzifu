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
#include <genome/filesystem.hpp>
#include <genome/string.hpp>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <map>
#include <utility>

#ifndef GENOME_TARGET_PATH_DELIMITER
# ifdef _WIN32
#  define GENOME_TARGET_PATH_DELIMITER '\\'
# else
#  define GENOME_TARGET_PATH_DELIMITER '/'
# endif
#endif
#ifndef GENOME_HAVE_DIRECT_H
# ifdef _WIN32
#  define GENOME_HAVE_DIRECT_H 1
# else
#  define GENOME_HAVE_DIRECT_H 0
# endif
#endif
#ifndef GENOME_HAVE_GMTIME_R
# ifdef HAVE_GMTIME_R
#  define GENOME_HAVE_GMTIME_R 1
# else
#  define GENOME_HAVE_GMTIME_R 0
# endif
#endif
#ifndef GENOME_HAVE_GMTIME_S
# if defined(__STDC_LIB_EXT1__) && defined(__STDC_WANT_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
// C11 standard (ISO/IEC 9899:2011): K.3.8.2.3 The gmtime_s function (p: 626-627)
#  define GENOME_HAVE_GMTIME_S 1
# elif defined(__STDC_SECURE_LIB__) && defined(__STDC_WANT_SECURE_LIB__) && __STDC_WANT_SECURE_LIB__
// Microsoft/Dinkumware (__STDC_WANT_SECURE_LIB__ defaults to 1)
#  define GENOME_HAVE_GMTIME_S 1
# else
#  define GENOME_HAVE_GMTIME_S 0
# endif
#endif

// stat(char const*, struct stat)
// mkdir(char const*, mode_t)
#include <sys/types.h>
#include <sys/stat.h>
#if !!GENOME_HAVE_DIRECT_H
# include <direct.h>
# define mkdir(P, M) _mkdir(P)
#endif

namespace genome {
namespace filesystem {

namespace /*{anonymous}*/ {

	struct path_compare {
		struct compare {
			bool operator()(char const& a, char const& b) const
			{
				return (
					std::tolower(std::string::traits_type::to_int_type(a)) <
					std::tolower(std::string::traits_type::to_int_type(b))
				);
			}
		};
		bool operator()(std::string const& a, std::string const& b) const
		{
			return (std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), compare()));
		}
	};

	class mount_list {
		typedef std::map<std::string, std::string, path_compare> path_map;
		path_map m_volumes;
		path_map m_mappings;
		mount_list(void)
			: m_volumes()
			, m_mappings()
		{
			// default to current directory, link all platforms to G3
			m_volumes.insert(std::make_pair("#G3:", ""));
			m_volumes.insert(std::make_pair("#PS3:", "#G3:"));
			m_volumes.insert(std::make_pair("#PS4:", "#G3:"));
			m_volumes.insert(std::make_pair("#X360:", "#G3:"));
			m_volumes.insert(std::make_pair("#X64:", "#G3:"));
			m_volumes.insert(std::make_pair("#XOne:", "#G3:"));
			// hard-coded (mount[_packed].xml parsing not implemented)
			m_mappings.insert(std::make_pair("/ini/", "/data/ini/"));
		}
		~mount_list(void)
		{
		}
	public:
		static
		mount_list& instance(void)
		{
			//NOTE: This is not thread-safe until your compiler waits for the completion of variable initializations.
			// However, mount_list::instance() is called from filesystem::init(), which in turn is called from
			// genome::init(), which is expected to be called during application initialization before anything else.
			static mount_list s_instance;
			return (s_instance);
		}
		bool set_volume_root(char const* name, char const* root)
		{
			std::string v(name ? name : "");
			std::string r(root ? root : "");
			// convert volume reference to name
			v.erase(0, v.find_first_not_of('#'));
			v.erase(v.find_last_not_of(':') + 1);
			// validate the volume name (non-empty, alphanumeric characters)
			if (v.empty()) {
				return (false);
			}
			for (std::string::const_iterator i = v.begin(); i != v.end(); ++i) {
				if (!std::isalnum(std::string::traits_type::to_int_type(*i))) {
					return (false);
				}
			}
			// convert to volume reference and add/replace the entry
			v.insert(0, 1, '#');
			v.push_back(':');
			r.erase(r.find_last_not_of(GENOME_TARGET_PATH_DELIMITER) + 1);
			std::pair<path_map::iterator, bool> p = m_volumes.insert(std::make_pair(v, r));
			if (!p.second) {
				p.first->second = r;
			}
			return (true);
		}
		bool get_volume_root(std::string const& volume, std::string& root)
		{
			root.assign(volume);
			// recursively lookup volume references (e.g. "#X64:" -> "#G3:" -> "..")
			while (!root.empty() && ('#' == root[0])) {
				path_map::const_iterator i = m_volumes.find(root);
				if (m_volumes.end() == i) {
					return (false);
				}
				root = i->second;
			}
			return (true);
		}
		bool link_directory(char const* from, char const* to)
		{
			std::string f(from ? from : "");
			std::string t(to ? to : "");
		#if (GENOME_TARGET_PATH_DELIMITER != '/')
			// We are talking about canonical paths here, fix native path delimiters here.
			std::replace(f.begin(), f.end(), GENOME_TARGET_PATH_DELIMITER, '/');
			std::replace(f.begin(), f.end(), GENOME_TARGET_PATH_DELIMITER, '/');
		#endif
			// remove leading path delimiters
			f.erase(0, f.find_first_not_of('/'));
			t.erase(0, t.find_first_not_of('/'));
			// redirecting the root is not allowed (use set_volume_root)
			if (f.empty()) {
				return (false);
			}
			// remove trailing path delimiters
			f.erase(f.find_last_not_of('/') + 1);
			t.erase(t.find_last_not_of('/') + 1);
			// add leading path delimiters (redirecting to root is allowed)
			f.insert(0, 1, '/');
			if (!t.empty()) {
				t.insert(0, 1, '/');
			}
			// add trailing path delimiters and add/replace the entry
			f.push_back('/');
			t.push_back('/');
			std::pair<path_map::iterator, bool> p = m_mappings.insert(std::make_pair(f, t));
			if (!p.second) {
				p.first->second = t;
			}
			return (true);
		}
		void resolve_directory(std::string& path)
		{
			std::string p(path);
			// find the longest directory that is redirected
			while (!p.empty()) {
				path_map::const_iterator i = m_mappings.find(p);
				if (i != m_mappings.end()) {
					path.replace(0, p.size(), i->second);
					return;
				}
				p.erase(p.find_last_not_of('/') + 1);
				p.erase(p.find_last_of('/') + 1);
			}
		}
	};

	bool split_canonical(char const* filename, std::string& root, std::string& path, std::string& name, std::string& type)
	{
		name.assign(filename ? filename : "");
		std::string::size_type n = name.rfind('/');
		if (std::string::npos == n) {
			path.clear();
			n = name.find(':');
			if (std::string::npos == n) {
				root.clear();
			} else {
				root.assign(name, 0, n + 1);
				name.erase(0, n + 1);
			}
		} else {
			path.assign(name, 0, n + 1);
			name.erase(0, n + 1);
			n = path.find(':');
			if (std::string::npos == n) {
				root.clear();
			} else {
				root.assign(path, 0, n + 1);
				path.erase(0, n + 1);
			}
		}
		n = name.rfind('.');
		if ((std::string::npos == n) || (0 == n)) {
			type.clear();
		} else {
			type.assign(name, n, std::string::npos);
			name.erase(n, std::string::npos);
		}
		return (!root.empty() && ('#' == root[0]));
	}

	bool time_to_gmtime(std::time_t const& time, struct std::tm& utc)
	{
	#if   !!GENOME_HAVE_GMTIME_R
		return (gmtime_r(&time, &utc) != 0);
	#elif !!GENOME_HAVE_GMTIME_S
		return (gmtime_s(&utc, &time) == 0);
	#else
		//FIXME: The return value of std::gmtime points to a static object.
		// The structure may be overridden by other functions and/or from other threads.
		struct std::tm const* const ptr = std::gmtime(&time);
		if (ptr != 0) {
			std::memmove(&utc, ptr, sizeof(struct std::tm));
			return (true);
		}
		return (false);
	#endif
	}

} // namespace genome::filesystem::{anonymous}

void
init(void)
{
	mount_list::instance();
}

bool
mount_volume(char const* name, char const* root)
{
	return (mount_list::instance().set_volume_root(name, root));
}

bool
link_directory(char const* from, char const* to)
{
	return (mount_list::instance().link_directory(from, to));
}

std::string
system_complete(char const* filename)
{
	std::string root;
	std::string path;
	std::string name;
	std::string type;
	if (!split_canonical(filename, root, path, name, type)) {
		return (std::string(filename ? filename : ""));
	}
	// resolve volume root
	if (!mount_list::instance().get_volume_root(root, root)) {
		return (std::string(filename ? filename : ""));
	}
	// resolve canonical directory link
	if (!path.empty()) {
		//FIXME: All directory names are converted to lowercase here.
		{
			byte_string n;
			if (string_convert(path, n)) {
				std::transform(n.begin(), n.end(), n.begin(), name_traits::canonicalize);
				std::string s;
				if (string_convert(n, s)) {
					path = s;
				}
			}
		}
		mount_list::instance().resolve_directory(path);
	}
	if (root.empty()) {
		// make the path relative
		path.erase(0, path.find_first_not_of('/'));
	} else if (path.empty() || (path[0] != '/')) {
		// insert path delimiter
		path.insert(0, 1, '/');
	}

#if (GENOME_TARGET_PATH_DELIMITER != '/')
	std::replace(path.begin(), path.end(), '/', GENOME_TARGET_PATH_DELIMITER);
#endif

	//TODO: Handle case-sensitive filenames (existing directories/files).
	return (std::string(root + path + name + type));
}

bool
create_directory(char const* path)
{
	std::string p(system_complete(path));
	//FIXME: Removing trailing path delimiter is not MBCS-safe.
	p.erase(p.find_last_not_of(GENOME_TARGET_PATH_DELIMITER) + 1);
	switch (p.size()) {
	case 0:
		return (false);
	case 1:
		if ('.' == p[0]) {
			return (true);
		}
		break;
	}
	if (0 == mkdir(p.c_str(), S_IRWXU | S_IRWXG | S_IRWXO)) {
		return (true);
	}
	return (EEXIST == errno);
}

bool
create_directories(char const* path)
{
	if (create_directory(path)) {
		return (true);
	}
	std::string p(system_complete(path));
	//FIXME: Removing trailing path delimiter is not MBCS-safe.
	p.erase(p.find_last_not_of(GENOME_TARGET_PATH_DELIMITER) + 1);
	//FIXME: Searching last path delimiter is not MBCS-safe.
	p.erase(p.find_last_of(GENOME_TARGET_PATH_DELIMITER) + 1);
	p.erase(p.find_last_not_of(GENOME_TARGET_PATH_DELIMITER) + 1);
	if (p.empty() || (GENOME_TARGET_PATH_DELIMITER == p[0])) {
		return (false);
	}
	if (!create_directories(p.c_str())) {
		return (false);
	}
	return (create_directory(path));
}

bool
ensure_directories(char const* filename)
{
	std::string p(system_complete(filename));
	//FIXME: Searching last path delimiter is not MBCS-safe.
	p.erase(p.find_last_of(GENOME_TARGET_PATH_DELIMITER) + 1);
	p.erase(p.find_last_not_of(GENOME_TARGET_PATH_DELIMITER) + 1);
	if (p.empty() || (GENOME_TARGET_PATH_DELIMITER == p[0])) {
		return (true);
	}
	return (create_directories(p.c_str()));
}

bool
get_last_write_time(char const* filename, struct std::tm& utc)
{
	struct stat s;
	if (0 == stat(system_complete(filename).c_str(), &s)) {
		return (time_to_gmtime(s.st_mtime, utc));
	}
	return (false);
}

} // namespace genome::filesystem
} // namespace genome
