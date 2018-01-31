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
#ifndef GENOME_FILESYSTEM_HPP
#define GENOME_FILESYSTEM_HPP

#include <genome/genome.hpp>
#include <ctime>
#include <string>

//
// VERY simple wrappers around canonical Genome filenames.
//
// - NO mount_list support
// - NO packed volume support
//
// Directory names (except volume roots) will be converted to lower-case
// because I'm currently too lazy to do it right (and do not want to add
// the Boost.Filesystem library to the dependencies - at least not yet).
//

namespace genome {
namespace filesystem {

// called by init_genome()
void init(void);
// add/set volume root by name ("G3") to native path or volume reference ("#G3:")
bool mount_volume(char const* name, char const* root);
// add/set canonical directory link ("ini" is already linked to "data/ini")
bool link_directory(char const* from, char const* to);
// convert canonical to native filename (or return as-is if not canonical)
std::string system_complete(char const* filename);
// create native/canonical directory
bool create_directory(char const* path);
// recursively create native/canonical directories
bool create_directories(char const* path);
// recursively create directories for a native/canonical file
bool ensure_directories(char const* filename);
// get last modification timestamp (in UTC) of a native/canonical file
bool get_last_write_time(char const* filename, struct std::tm& utc);

} // namespace genome::filesystem
} // namespace genome

#endif // GENOME_FILESYSTEM_HPP
