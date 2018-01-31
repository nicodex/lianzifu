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
#include <cwchar>
#include <locale>
#include <stdexcept>

//
//TODO: Check if the GNU libstdc++ std::ctype and std::codecvt are fixed.
//
// libstdc++ std::ctype and std::codecvt are broken for custom types.
// Therefore we have to add all custom types at the end of this file.
//

#if defined(__GLIBCXX__) && !defined(LOCALE_NO_GLIBCXX_HACK)

#define CTYPE_DEFINE_PURE(CharT)                                               \
namespace std {                                                                \
template<>                                                                     \
ctype<CharT>::~ctype(void)                                                     \
{                                                                              \
}                                                                              \
template<>                                                                     \
bool                                                                           \
ctype<CharT>::do_is(mask, char_type) const                                     \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::ctype<"#CharT">::do_is");                                          \
}                                                                              \
template<>                                                                     \
ctype<CharT>::char_type const*                                                 \
ctype<CharT>::do_is(char_type const*, char_type const*, mask*) const           \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::ctype<"#CharT">::do_is");                                          \
}                                                                              \
template<>                                                                     \
ctype<CharT>::char_type const*                                                 \
ctype<CharT>::do_scan_is(mask, char_type const*, char_type const*) const       \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::ctype<"#CharT">::do_scan_is");                                     \
}                                                                              \
template<>                                                                     \
ctype<CharT>::char_type const*                                                 \
ctype<CharT>::do_scan_not(mask, char_type const*, char_type const*) const      \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::ctype<"#CharT">::do_scan_not");                                    \
}                                                                              \
template<>                                                                     \
ctype<CharT>::char_type                                                        \
ctype<CharT>::do_toupper(char_type) const                                      \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::ctype<"#CharT">::do_toupper");                                     \
}                                                                              \
template<>                                                                     \
ctype<CharT>::char_type const*                                                 \
ctype<CharT>::do_toupper(char_type*, char_type const*) const                   \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::ctype<"#CharT">::do_toupper");                                     \
}                                                                              \
template<>                                                                     \
ctype<CharT>::char_type                                                        \
ctype<CharT>::do_tolower(char_type) const                                      \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::ctype<"#CharT">::do_tolower");                                     \
}                                                                              \
template<>                                                                     \
ctype<CharT>::char_type const*                                                 \
ctype<CharT>::do_tolower(char_type*, char_type const*) const                   \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::ctype<"#CharT">::do_tolower");                                     \
}                                                                              \
template<>                                                                     \
ctype<CharT>::char_type                                                        \
ctype<CharT>::do_widen(char) const                                             \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::ctype<"#CharT">::do_widen");                                       \
}                                                                              \
template<>                                                                     \
char const*                                                                    \
ctype<CharT>::do_widen(char const*, char const*, char_type*) const             \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::ctype<"#CharT">::do_widen");                                       \
}                                                                              \
template<>                                                                     \
char                                                                           \
ctype<CharT>::do_narrow(char_type, char) const                                 \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::ctype<"#CharT">::do_narrow");                                      \
}                                                                              \
template<>                                                                     \
ctype<CharT>::char_type const*                                                 \
ctype<CharT>::do_narrow(char_type const*, char_type const*, char, char*) const \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::ctype<"#CharT">::do_narrow");                                      \
}                                                                              \
}

#define CODECVT_DEFINE_PURE(InternT, ExternT, StateT)                          \
namespace std {                                                                \
template<>                                                                     \
bool                                                                           \
codecvt<InternT, ExternT, StateT>::do_always_noconv(void) const /*throw()*/    \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::codecvt<"#InternT", "#ExternT", "#StateT">::do_always_noconv");    \
}                                                                              \
template<>                                                                     \
int                                                                            \
codecvt<InternT, ExternT, StateT>::do_encoding(void) const /*throw()*/         \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::codecvt<"#InternT", "#ExternT", "#StateT">::do_encoding");         \
}                                                                              \
template<>                                                                     \
codecvt_base::result                                                           \
codecvt<InternT, ExternT, StateT>::do_in(state_type&,                          \
  extern_type const*, extern_type const*, extern_type const*&,                 \
  intern_type*, intern_type*, intern_type*&) const                             \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::codecvt<"#InternT", "#ExternT", "#StateT">::do_in");               \
}                                                                              \
template<>                                                                     \
int                                                                            \
codecvt<InternT, ExternT, StateT>::do_length(state_type&,                      \
  extern_type const*, extern_type const*, size_t) const                        \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::codecvt<"#InternT", "#ExternT", "#StateT">::do_length");           \
}                                                                              \
template<>                                                                     \
int                                                                            \
codecvt<InternT, ExternT, StateT>::do_max_length(void) const /*throw()*/       \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::codecvt<"#InternT", "#ExternT", "#StateT">::do_max_length");       \
}                                                                              \
template<>                                                                     \
codecvt_base::result                                                           \
codecvt<InternT, ExternT, StateT>::do_out(state_type&,                         \
  intern_type const*, intern_type const*, intern_type const*&,                 \
  extern_type*, extern_type*, extern_type*&) const                             \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::codecvt<"#InternT", "#ExternT", "#StateT">::do_out");              \
}                                                                              \
template<>                                                                     \
codecvt_base::result                                                           \
codecvt<InternT, ExternT, StateT>::do_unshift(state_type&,                     \
  extern_type*, extern_type*, extern_type*&) const                             \
{                                                                              \
  throw logic_error("pure virtual method called"                               \
    ": std::codecvt<"#InternT", "#ExternT", "#StateT">::do_unshift");          \
}                                                                              \
}

#include <genome/string.hpp>

CTYPE_DEFINE_PURE(genome::byte_char)
CTYPE_DEFINE_PURE(genome::wide_char)
CODECVT_DEFINE_PURE(genome::byte_char, char, std::mbstate_t)
CODECVT_DEFINE_PURE(genome::wide_char, char, std::mbstate_t)

#endif // __GLIBCXX__ && !LOCALE_NO_GLIBCXX_HACK
