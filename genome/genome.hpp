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
#ifndef GENOME_GENOME_HPP
#define GENOME_GENOME_HPP

////////////////////////////////////////////////////////////////////////////////
//
// This header is expected to be included first in all headers/source files.
//
//  It's important to be included before <stdint.h> and <inttypes.h>
//  to ensure that the C99 limit/constant/format macros are defined.
//
////////////////////////////////////////////////////////////////////////////////

// C11 standard (ISO/IEC 9899:2011) Annex K
//#define __STDC_WANT_LIB_EXT1__ 1
// Microsoft/Dinkumware
//#define __STDC_WANT_SECURE_LIB__ 1

//
// Initialize framework (singletons)
//
//  The global locale and the default C++ streams are expected to be
//  initialized before init_genome() and not changed after the call.
//

void init_genome(void);

//
// C++ version override/detection
//

#ifndef GENOME_CXX_VERSION
# if defined(_MSVC_LANG) && (_MSVC_LANG > 199711L) && (199711L == __cplusplus)
#  define GENOME_CXX_VERSION _MSVC_LANG
# else
#  define GENOME_CXX_VERSION __cplusplus
# endif
#endif
#define GENOME_CXX17 (GENOME_CXX_VERSION >= 201703L)
#define GENOME_CXX14 (GENOME_CXX_VERSION >= 201402L)
#define GENOME_CXX11 (GENOME_CXX_VERSION >= 201103L)

#if !!GENOME_CXX11
# include <cstdint>
# include <cinttypes>
#else
# ifndef       __STDC_LIMIT_MACROS
#  define      __STDC_LIMIT_MACROS
#  define GENOME_STDC_LIMIT_MACROS_UNDEF
# endif
# ifndef       __STDC_CONSTANT_MACROS
#  define      __STDC_CONSTANT_MACROS
#  define GENOME_STDC_CONSTANT_MACROS_UNDEF
# endif
# ifndef       __STDC_FORMAT_MACROS
#  define      __STDC_FORMAT_MACROS
#  define GENOME_STDC_FORMAT_MACROS_UNDEF
# endif
# include <stdint.h>    // https://github.com/chemeris/msinttypes
# include <inttypes.h>  // https://github.com/chemeris/msinttypes
# ifdef   GENOME_STDC_FORMAT_MACROS_UNDEF
#  undef  GENOME_STDC_FORMAT_MACROS_UNDEF
#  undef       __STDC_FORMAT_MACROS
# endif
# ifdef   GENOME_STDC_CONSTANT_MACROS_UNDEF
#  undef  GENOME_STDC_CONSTANT_MACROS_UNDEF
#  undef       __STDC_CONSTANT_MACROS
# endif
# ifdef   GENOME_STDC_LIMIT_MACROS_UNDEF
#  undef  GENOME_STDC_LIMIT_MACROS_UNDEF
#  undef       __STDC_LIMIT_MACROS
# endif
#endif
#include <climits>
#include <cstddef>

namespace genome {

//
// expanded token concatenation
//

#define GENOME_PASTE(a, b) a##b
#define GENOME_CONCAT(a, b) GENOME_PASTE(a, b)

//
// expanded token stringification
//

#define GENOME_STRINGIFY(s) #s
#define GENOME_EXPSTRING(x) GENOME_STRINGIFY(x)

//
// language standard specific features
//

#if !!GENOME_CXX11
# define GENOME_CONSTEXPR constexpr
# define GENOME_CONSTEXPR_CONST constexpr
# define GENOME_CONSTEXPR_INLINE constexpr
# define GENOME_DELETE_FUNCTION = delete
# define GENOME_EXTERN_TEMPLATES 1
# define GENOME_NOEXCEPT noexcept
# define GENOME_NOEXCEPT_NOTHROW noexcept
# define GENOME_OVERRIDE override
# define GENOME_STATIC_ASSERT(name, cond, text) static_assert(cond, text)
#else
# define GENOME_CONSTEXPR
# define GENOME_CONSTEXPR_CONST const
# define GENOME_CONSTEXPR_INLINE inline
# define GENOME_DELETE_FUNCTION
# define GENOME_EXTERN_TEMPLATES 0
# define GENOME_NOEXCEPT
# define GENOME_NOEXCEPT_NOTHROW throw()
# define GENOME_OVERRIDE
template<bool> struct static_assertion; template<> struct static_assertion<true> {}; template<int> struct static_assertion_test {};
# define GENOME_STATIC_ASSERT(name, cond, text) struct GENOME_CONCAT(static_assert_, name) { genome::static_assertion<static_cast<bool>(cond)> GENOME_CONCAT(static_assert_failed_, name); }; typedef genome::static_assertion_test<sizeof(GENOME_CONCAT(static_assert_, name))> GENOME_CONCAT(static_assert_test_, name)
#endif

//
// suppress unreferenced parameter warning
//

template<typename T>
inline void unused_parameter(T const&);

//
// Fixed width integer types (C99/C++11) with exact bit counts.
// Not required by the standards, but required for binary data.
// If your standard library does not provide the required types
// you have to find types with the exact bit count, no padding,
// and using the two's complement for (signed) negative values.
//
// The platform endianness is only detected for internal optimizations.
// If the automatic compile-time detection does not work you can define
// the preprocessor symbol "GENOME_TARGET_INTEGER_(BIG|LITTLE)_ENDIAN".
//

GENOME_STATIC_ASSERT(target_char_size, CHAR_BIT >= 8,
	"The platform char must have at least 8 bits, sorry.");

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#if defined(GENOME_TARGET_INTEGER_BIG_ENDIAN)
# define GENOME_INTEGER_BIG_ENDIAN 1
#elif defined(GENOME_TARGET_INTEGER_LITTLE_ENDIAN)
# define GENOME_INTEGER_LITTLE_ENDIAN 1
#elif defined(__ORDER_BIG_ENDIAN__) && defined(__ORDER_LITTLE_ENDIAN__)
# if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#  define GENOME_INTEGER_BIG_ENDIAN 1
# endif
# if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#  define GENOME_INTEGER_LITTLE_ENDIAN 1
# endif
#elif defined(__x86_64__) || defined(_M_X64)
# define GENOME_INTEGER_LITTLE_ENDIAN 1
#elif defined(__i386) || defined(_M_IX86)
# define GENOME_INTEGER_LITTLE_ENDIAN 1
#endif
#ifndef GENOME_INTEGER_BIG_ENDIAN
# define GENOME_INTEGER_BIG_ENDIAN 0
#endif
#ifndef GENOME_INTEGER_LITTLE_ENDIAN
# define GENOME_INTEGER_LITTLE_ENDIAN 0
#endif
#if !!GENOME_INTEGER_BIG_ENDIAN && !!GENOME_INTEGER_LITTLE_ENDIAN
# error "mixed integer endianness"
#endif
enum target_config {
	target_char_8bit = bool(CHAR_BIT == 8),
	target_wchar_16bit = bool((CHAR_BIT * sizeof(wchar_t)) == 16),
	target_wchar_17planes = bool((CHAR_BIT * sizeof(wchar_t)) >= 21),
	target_integer_big_endian = bool(!!GENOME_INTEGER_BIG_ENDIAN),
	target_integer_little_endian = bool(!!GENOME_INTEGER_LITTLE_ENDIAN)
};
#undef GENOME_INTEGER_BIG_ENDIAN
#undef GENOME_INTEGER_LITTLE_ENDIAN

//
// platforms
//

enum platform {
	platform_unknown = -1,
	platform_pc,    // Windows 32-bit (x86)
	platform_x64,   // Windows 64-bit (x86-64)
	platform_ps3,   // PlayStation 3
	platform_ps4,   // PlayStation 4
	platform_x360,  // Xbox 360
	platform_xone   // Xbox One
};

char const* platform_name(platform target);
char const* platform_desc(platform target);
platform platform_from_name(char const* name);

} // namespace genome

#include <genome/genome.ipp>

#endif // GENOME_GENOME_HPP
