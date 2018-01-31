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
#include <genome/locale.hpp>

namespace genome {
namespace detail {

namespace /*{anonymous}*/ {

	//
	//TODO: std::ctype_base::blank often missing, even if GENOME_CXX11 is true
	//
	// The type of the std::ctype_base::mask is unspecified and is only required to
	// support the BitmaskType concept (enumeration, integer type, or std::bitset).
	//
	// To define the Genome default ctype table ("C" locale of MSVC10) the masks
	// are first split into separate flags and later merged to character classes.
	//

	// none
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_mask_none = std::ctype_base::mask();

	// cntrl flag
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_flag_cntrl = std::ctype_base::cntrl;
	GENOME_STATIC_ASSERT(ctype_flag_cntrl,
		(ctype_mask_none != ctype_flag_cntrl) &&
		(ctype_mask_none == static_cast<std::ctype_base::mask>((ctype_flag_cntrl & (ctype_flag_cntrl - 1)))),
		"std::ctype_base::cntrl is not a non-empty unique flag");
	// digit flag
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_flag_digit = std::ctype_base::digit;
	GENOME_STATIC_ASSERT(ctype_flag_digit,
		(ctype_mask_none != ctype_flag_digit) &&
		(ctype_mask_none == static_cast<std::ctype_base::mask>((ctype_flag_digit & (ctype_flag_digit - 1)))),
		"std::ctype_base::digit is not a non-empty unique flag");
	// lower flag
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_flag_lower = std::ctype_base::lower;
	GENOME_STATIC_ASSERT(ctype_flag_lower,
		(ctype_mask_none != ctype_flag_lower) &&
		(ctype_mask_none == static_cast<std::ctype_base::mask>((ctype_flag_lower & (ctype_flag_lower - 1)))),
		"std::ctype_base::lower is not a non-empty unique flag");
	// punct flag
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_flag_punct = std::ctype_base::punct;
	GENOME_STATIC_ASSERT(ctype_flag_punct,
		(ctype_mask_none != ctype_flag_punct) &&
		(ctype_mask_none == static_cast<std::ctype_base::mask>((ctype_flag_punct & (ctype_flag_punct - 1)))),
		"std::ctype_base::punct is not a non-empty unique flag");
	// upper flag
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_flag_upper = std::ctype_base::upper;
	GENOME_STATIC_ASSERT(ctype_flag_upper,
		(ctype_mask_none != ctype_flag_upper) &&
		(ctype_mask_none == static_cast<std::ctype_base::mask>((ctype_flag_upper & (ctype_flag_upper - 1)))),
		"std::ctype_base::upper is not a non-empty unique flag");
	// xdigit flag
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_flag_xdigit = static_cast<std::ctype_base::mask>(
		std::ctype_base::xdigit & ~(
			ctype_flag_digit
		)
	);
	GENOME_STATIC_ASSERT(ctype_flag_xdigit,
		(ctype_mask_none != ctype_flag_xdigit) &&
		(ctype_mask_none == static_cast<std::ctype_base::mask>((ctype_flag_xdigit & (ctype_flag_xdigit - 1)))),
		"std::ctype_base::xdigit is not a non-empty unique flag");
	// blank flag
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_flag_blank = static_cast<std::ctype_base::mask>(
		// blank flag in space mask and print mask (MSVC)
		(
			(static_cast<std::ctype_base::mask>(std::ctype_base::space & std::ctype_base::print) != ctype_mask_none) &&
			(static_cast<std::ctype_base::mask>(std::ctype_base::space & std::ctype_base::print) != std::ctype_base::space)
		) ? (
			static_cast<std::ctype_base::mask>(std::ctype_base::space & std::ctype_base::print)
		) : (
			// print mask without space flag and with blank flag (Cygwin)
			(
				(
					(
						std::ctype_base::digit | std::ctype_base::lower |
						std::ctype_base::punct | std::ctype_base::upper
					) == (std::ctype_base::print & (
						std::ctype_base::digit | std::ctype_base::lower |
						std::ctype_base::punct | std::ctype_base::upper))
				) &&
				ctype_mask_none != static_cast<std::ctype_base::mask>
					(std::ctype_base::print & ~(
						std::ctype_base::cntrl | std::ctype_base::digit |
						std::ctype_base::lower | std::ctype_base::punct |
						std::ctype_base::upper | std::ctype_base::xdigit |
						std::ctype_base::space | std::ctype_base::alpha))
			) ? (
				static_cast<std::ctype_base::mask>
					(std::ctype_base::print & ~(
						std::ctype_base::cntrl | std::ctype_base::digit |
						std::ctype_base::lower | std::ctype_base::punct |
						std::ctype_base::upper | std::ctype_base::xdigit |
						std::ctype_base::space | std::ctype_base::alpha))
			) : (
				ctype_mask_none
			)
		)
	);
	GENOME_STATIC_ASSERT(ctype_flag_blank,
		(ctype_mask_none == ctype_flag_blank) ||
		(ctype_mask_none == static_cast<std::ctype_base::mask>((ctype_flag_blank & (ctype_flag_blank - 1)))),
		"std::ctype_base::blank is neither empty nor an unique flag");
	// space flag
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_flag_space = static_cast<std::ctype_base::mask>(
		std::ctype_base::space & ~(
			ctype_flag_blank
		)
	);
	GENOME_STATIC_ASSERT(ctype_flag_space,
		(ctype_mask_none != ctype_flag_space) &&
		(ctype_mask_none == static_cast<std::ctype_base::mask>((ctype_flag_space & (ctype_flag_space - 1)))),
		"std::ctype_base::space is not a non-empty unique flag");
	// alpha flag
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_flag_alpha = static_cast<std::ctype_base::mask>(
		std::ctype_base::alpha & ~(
			ctype_flag_lower |
			ctype_flag_upper
		)
	);
	GENOME_STATIC_ASSERT(ctype_flag_alpha,
		(ctype_mask_none == ctype_flag_alpha) ||
		(ctype_mask_none == static_cast<std::ctype_base::mask>((ctype_flag_alpha & (ctype_flag_alpha - 1)))),
		"std::ctype_base::alpha is neither empty nor an unique flag");
	// print flag
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_flag_print = static_cast<std::ctype_base::mask>(
		std::ctype_base::print & ~(
			ctype_flag_blank |
			ctype_flag_space |
			ctype_flag_digit |
			ctype_flag_lower |
			ctype_flag_punct |
			ctype_flag_upper |
			ctype_flag_xdigit |
			ctype_flag_alpha
		)
	);
	GENOME_STATIC_ASSERT(ctype_flag_print, (ctype_flag_print == ctype_mask_none) ||
		(static_cast<std::ctype_base::mask>((ctype_flag_print & (ctype_flag_print - 1))) == ctype_mask_none),
		"std::ctype_base::print is neither empty nor an unique flag");

	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_mask_control = static_cast<std::ctype_base::mask>(ctype_flag_cntrl);
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_mask_control_space = static_cast<std::ctype_base::mask>(
		ctype_flag_cntrl |
		ctype_flag_space
	);
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_mask_blank_space = static_cast<std::ctype_base::mask>(
		ctype_flag_blank |
		ctype_flag_space |
		ctype_flag_print
	);
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_mask_punct = static_cast<std::ctype_base::mask>(
		ctype_flag_punct |
		ctype_flag_print
	);
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_mask_digit_hex = static_cast<std::ctype_base::mask>(
		ctype_flag_digit |
		ctype_flag_xdigit |
		ctype_flag_print
	);
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_mask_upper_hex = static_cast<std::ctype_base::mask>(
		ctype_flag_upper |
		ctype_flag_xdigit |
		ctype_flag_alpha |
		ctype_flag_print
	);
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_mask_upper = static_cast<std::ctype_base::mask>(
		ctype_flag_upper |
		ctype_flag_alpha |
		ctype_flag_print
	);
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_mask_lower_hex = static_cast<std::ctype_base::mask>(
		ctype_flag_lower |
		ctype_flag_xdigit |
		ctype_flag_alpha |
		ctype_flag_print
	);
	GENOME_CONSTEXPR_CONST std::ctype_base::mask ctype_mask_lower = static_cast<std::ctype_base::mask>(
		ctype_flag_lower |
		ctype_flag_alpha |
		ctype_flag_print
	);

} // namespace genome::detail::{anonymous}

//
// ctype_genome<...>
//

std::ctype_base::mask const
ctype_genome_table[256] = {
	// Basic Latin
	ctype_mask_control,        // 0x00 NULL
	ctype_mask_control,        // 0x01 START OF HEADING
	ctype_mask_control,        // 0x02 START OF TEXT
	ctype_mask_control,        // 0x03 END OF TEXT
	ctype_mask_control,        // 0x04 END OF TRANSMISSION
	ctype_mask_control,        // 0x05 ENQUIRY
	ctype_mask_control,        // 0x06 ACKNOWLEDGE
	ctype_mask_control,        // 0x07 BELL
	ctype_mask_control,        // 0x08 BACKSPACE
	ctype_mask_control_space,  // 0x09 CHARACTER TABULATION (HT)
	ctype_mask_control_space,  // 0x0A LINE FEED (LF)
	ctype_mask_control_space,  // 0x0B LINE TABULATION (VT)
	ctype_mask_control_space,  // 0x0C FORM FEED (FF)
	ctype_mask_control_space,  // 0x0D CARRIAGE RETURN (CR)
	ctype_mask_control,        // 0x0E SHIFT OUT
	ctype_mask_control,        // 0x0F SHIFT IN
	ctype_mask_control,        // 0x10 DATA LINK ESCAPE
	ctype_mask_control,        // 0x11 DEVICE CONTROL ONE
	ctype_mask_control,        // 0x12 DEVICE CONTROL TWO
	ctype_mask_control,        // 0x13 DEVICE CONTROL THREE
	ctype_mask_control,        // 0x14 DEVICE CONTROL FOUR
	ctype_mask_control,        // 0x15 NEGATIVE ACKNOWLEDGE
	ctype_mask_control,        // 0x16 SYNCHRONOUS IDLE
	ctype_mask_control,        // 0x17 END OF TRANSMISSION BLOCK
	ctype_mask_control,        // 0x18 CANCEL
	ctype_mask_control,        // 0x19 END OF MEDIUM
	ctype_mask_control,        // 0x1A SUBSTITUTE
	ctype_mask_control,        // 0x1B ESCAPE
	ctype_mask_control,        // 0x1C INFORMATION SEPARATOR FOUR (FS)
	ctype_mask_control,        // 0x1D INFORMATION SEPARATOR THREE (GS)
	ctype_mask_control,        // 0x1E INFORMATION SEPARATOR TWO (RS)
	ctype_mask_control,        // 0x1F INFORMATION SEPARATOR ONE (US)
	ctype_mask_blank_space,    // 0x20 SPACE
	ctype_mask_punct,          // 0x21 EXCLAMATION MARK
	ctype_mask_punct,          // 0x22 QUOTATION MARK
	ctype_mask_punct,          // 0x23 NUMBER SIGN
	ctype_mask_punct,          // 0x24 DOLLAR SIGN
	ctype_mask_punct,          // 0x25 PERCENT SIGN
	ctype_mask_punct,          // 0x26 AMPERSAND
	ctype_mask_punct,          // 0x27 APOSTROPHE
	ctype_mask_punct,          // 0x28 LEFT PARENTHESIS
	ctype_mask_punct,          // 0x29 RIGHT PARENTHESIS
	ctype_mask_punct,          // 0x2A ASTERISK
	ctype_mask_punct,          // 0x2B PLUS SIGN
	ctype_mask_punct,          // 0x2C COMMA
	ctype_mask_punct,          // 0x2D HYPHEN-MINUS
	ctype_mask_punct,          // 0x2E FULL STOP
	ctype_mask_punct,          // 0x2F SOLIDUS
	ctype_mask_digit_hex,      // 0x30 DIGIT ZERO
	ctype_mask_digit_hex,      // 0x31 DIGIT ONE
	ctype_mask_digit_hex,      // 0x32 DIGIT TWO
	ctype_mask_digit_hex,      // 0x33 DIGIT THREE
	ctype_mask_digit_hex,      // 0x34 DIGIT FOUR
	ctype_mask_digit_hex,      // 0x35 DIGIT FIVE
	ctype_mask_digit_hex,      // 0x36 DIGIT SIX
	ctype_mask_digit_hex,      // 0x37 DIGIT SEVEN
	ctype_mask_digit_hex,      // 0x38 DIGIT EIGHT
	ctype_mask_digit_hex,      // 0x39 DIGIT NINE
	ctype_mask_punct,          // 0x3A COLON
	ctype_mask_punct,          // 0x3B SEMICOLON
	ctype_mask_punct,          // 0x3C LESS-THAN SIGN
	ctype_mask_punct,          // 0x3D EQUALS SIGN
	ctype_mask_punct,          // 0x3E GREATER-THAN SIGN
	ctype_mask_punct,          // 0x3F QUESTION MARK
	ctype_mask_punct,          // 0x40 COMMERCIAL AT
	ctype_mask_upper_hex,      // 0x41 LATIN CAPITAL LETTER A
	ctype_mask_upper_hex,      // 0x42 LATIN CAPITAL LETTER B
	ctype_mask_upper_hex,      // 0x43 LATIN CAPITAL LETTER C
	ctype_mask_upper_hex,      // 0x44 LATIN CAPITAL LETTER D
	ctype_mask_upper_hex,      // 0x45 LATIN CAPITAL LETTER E
	ctype_mask_upper_hex,      // 0x46 LATIN CAPITAL LETTER F
	ctype_mask_upper,          // 0x47 LATIN CAPITAL LETTER G
	ctype_mask_upper,          // 0x48 LATIN CAPITAL LETTER H
	ctype_mask_upper,          // 0x49 LATIN CAPITAL LETTER I
	ctype_mask_upper,          // 0x4A LATIN CAPITAL LETTER J
	ctype_mask_upper,          // 0x4B LATIN CAPITAL LETTER K
	ctype_mask_upper,          // 0x4C LATIN CAPITAL LETTER L
	ctype_mask_upper,          // 0x4D LATIN CAPITAL LETTER M
	ctype_mask_upper,          // 0x4E LATIN CAPITAL LETTER N
	ctype_mask_upper,          // 0x4F LATIN CAPITAL LETTER O
	ctype_mask_upper,          // 0x50 LATIN CAPITAL LETTER P
	ctype_mask_upper,          // 0x51 LATIN CAPITAL LETTER Q
	ctype_mask_upper,          // 0x52 LATIN CAPITAL LETTER R
	ctype_mask_upper,          // 0x53 LATIN CAPITAL LETTER S
	ctype_mask_upper,          // 0x54 LATIN CAPITAL LETTER T
	ctype_mask_upper,          // 0x55 LATIN CAPITAL LETTER U
	ctype_mask_upper,          // 0x56 LATIN CAPITAL LETTER V
	ctype_mask_upper,          // 0x57 LATIN CAPITAL LETTER W
	ctype_mask_upper,          // 0x58 LATIN CAPITAL LETTER X
	ctype_mask_upper,          // 0x59 LATIN CAPITAL LETTER Y
	ctype_mask_upper,          // 0x5A LATIN CAPITAL LETTER Z
	ctype_mask_punct,          // 0x5B LEFT SQUARE BRACKET
	ctype_mask_punct,          // 0x5C REVERSE SOLIDUS
	ctype_mask_punct,          // 0x5D RIGHT SQUARE BRACKET
	ctype_mask_punct,          // 0x5E CIRCUMFLEX ACCENT
	ctype_mask_punct,          // 0x5F LOW LINE
	ctype_mask_punct,          // 0x60 GRAVE ACCENT
	ctype_mask_lower_hex,      // 0x61 LATIN SMALL LETTER A
	ctype_mask_lower_hex,      // 0x62 LATIN SMALL LETTER B
	ctype_mask_lower_hex,      // 0x63 LATIN SMALL LETTER C
	ctype_mask_lower_hex,      // 0x64 LATIN SMALL LETTER D
	ctype_mask_lower_hex,      // 0x65 LATIN SMALL LETTER E
	ctype_mask_lower_hex,      // 0x66 LATIN SMALL LETTER F
	ctype_mask_lower,          // 0x67 LATIN SMALL LETTER G
	ctype_mask_lower,          // 0x68 LATIN SMALL LETTER H
	ctype_mask_lower,          // 0x69 LATIN SMALL LETTER I
	ctype_mask_lower,          // 0x6A LATIN SMALL LETTER J
	ctype_mask_lower,          // 0x6B LATIN SMALL LETTER K
	ctype_mask_lower,          // 0x6C LATIN SMALL LETTER L
	ctype_mask_lower,          // 0x6D LATIN SMALL LETTER M
	ctype_mask_lower,          // 0x6E LATIN SMALL LETTER N
	ctype_mask_lower,          // 0x6F LATIN SMALL LETTER O
	ctype_mask_lower,          // 0x70 LATIN SMALL LETTER P
	ctype_mask_lower,          // 0x71 LATIN SMALL LETTER Q
	ctype_mask_lower,          // 0x72 LATIN SMALL LETTER R
	ctype_mask_lower,          // 0x73 LATIN SMALL LETTER S
	ctype_mask_lower,          // 0x74 LATIN SMALL LETTER T
	ctype_mask_lower,          // 0x75 LATIN SMALL LETTER U
	ctype_mask_lower,          // 0x76 LATIN SMALL LETTER V
	ctype_mask_lower,          // 0x77 LATIN SMALL LETTER W
	ctype_mask_lower,          // 0x78 LATIN SMALL LETTER X
	ctype_mask_lower,          // 0x79 LATIN SMALL LETTER Y
	ctype_mask_lower,          // 0x7A LATIN SMALL LETTER Z
	ctype_mask_punct,          // 0x7B LEFT CURLY BRACKET
	ctype_mask_punct,          // 0x7C VERTICAL LINE
	ctype_mask_punct,          // 0x7D RIGHT CURLY BRACKET
	ctype_mask_punct,          // 0x7E TILDE
	ctype_mask_control,        // 0x7F DELETE
	// Windows-1252
	ctype_mask_none,           // 0x80 EURO SIGN
	ctype_mask_none,           // 0x81 HIGH OCTET PRESET
	ctype_mask_none,           // 0x82 SINGLE LOW-9 QUOTATION MARK
	ctype_mask_none,           // 0x83 LATIN CAPITAL LETTER F WITH HOOK
	ctype_mask_none,           // 0x84 DOUBLE LOW-9 QUOTATION MARK
	ctype_mask_none,           // 0x85 HORIZONTAL ELLIPSIS
	ctype_mask_none,           // 0x86 DAGGER
	ctype_mask_none,           // 0x87 DOUBLE DAGGER
	ctype_mask_none,           // 0x88 MODIFIER LETTER CIRCUMFLEX ACCENT
	ctype_mask_none,           // 0x89 PER MILLE SIGN
	ctype_mask_none,           // 0x8A LATIN CAPITAL LETTER S WITH CARON
	ctype_mask_none,           // 0x8B SINGLE LEFT-POINTING ANGLE QUOTATION MARK
	ctype_mask_none,           // 0x8C LATIN CAPITAL LIGATURE OE
	ctype_mask_none,           // 0x8D REVERSE LINE FEED
	ctype_mask_none,           // 0x8E LATIN CAPITAL LETTER Z WITH CARON
	ctype_mask_none,           // 0x8F SINGLE SHIFT THREE
	ctype_mask_none,           // 0x90 DEVICE CONTROL STRING
	ctype_mask_none,           // 0x91 LEFT SINGLE QUOTATION MARK
	ctype_mask_none,           // 0x92 RIGHT SINGLE QUOTATION MARK
	ctype_mask_none,           // 0x93 LEFT DOUBLE QUOTATION MARK
	ctype_mask_none,           // 0x94 RIGHT DOUBLE QUOTATION MARK
	ctype_mask_none,           // 0x95 BULLET
	ctype_mask_none,           // 0x96 EN DASH
	ctype_mask_none,           // 0x97 EM DASH
	ctype_mask_none,           // 0x98 SMALL TILDE
	ctype_mask_none,           // 0x99 TRADE MARK SIGN
	ctype_mask_none,           // 0x9A LATIN SMALL LETTER S WITH CARON
	ctype_mask_none,           // 0x9B SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
	ctype_mask_none,           // 0x9C LATIN SMALL LIGATURE OE
	ctype_mask_none,           // 0x9D OPERATING SYSTEM COMMAND
	ctype_mask_none,           // 0x9E LATIN SMALL LETTER Z WITH CARON
	ctype_mask_none,           // 0x9F LATIN CAPITAL LETTER Y WITH DIAERESIS
	ctype_mask_none,           // 0xA0 NO-BREAK SPACE
	ctype_mask_none,           // 0xA1 INVERTED EXCLAMATION MARK
	ctype_mask_none,           // 0xA2 CENT SIGN
	ctype_mask_none,           // 0xA3 POUND SIGN
	ctype_mask_none,           // 0xA4 CURRENCY SIGN
	ctype_mask_none,           // 0xA5 YEN SIGN
	ctype_mask_none,           // 0xA6 BROKEN BAR
	ctype_mask_none,           // 0xA7 SECTION SIGN
	ctype_mask_none,           // 0xA8 DIAERESIS
	ctype_mask_none,           // 0xA9 COPYRIGHT SIGN
	ctype_mask_none,           // 0xAA FEMININE ORDINAL INDICATOR
	ctype_mask_none,           // 0xAB LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
	ctype_mask_none,           // 0xAC NOT SIGN
	ctype_mask_none,           // 0xAD SOFT HYPHEN
	ctype_mask_none,           // 0xAE REGISTERED SIGN
	ctype_mask_none,           // 0xAF MACRON
	ctype_mask_none,           // 0xB0 DEGREE SIGN
	ctype_mask_none,           // 0xB1 PLUS-MINUS SIGN
	ctype_mask_none,           // 0xB2 SUPERSCRIPT TWO
	ctype_mask_none,           // 0xB3 SUPERSCRIPT THREE
	ctype_mask_none,           // 0xB4 ACUTE ACCENT
	ctype_mask_none,           // 0xB5 MICRO SIGN
	ctype_mask_none,           // 0xB6 PILCROW SIGN
	ctype_mask_none,           // 0xB7 MIDDLE DOT
	ctype_mask_none,           // 0xB8 CEDILLA
	ctype_mask_none,           // 0xB9 SUPERSCRIPT ONE
	ctype_mask_none,           // 0xBA MASCULINE ORDINAL INDICATOR
	ctype_mask_none,           // 0xBB RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
	ctype_mask_none,           // 0xBC VULGAR FRACTION ONE QUARTER
	ctype_mask_none,           // 0xBD VULGAR FRACTION ONE HALF
	ctype_mask_none,           // 0xBE VULGAR FRACTION THREE QUARTERS
	ctype_mask_none,           // 0xBF INVERTED QUESTION MARK
	ctype_mask_none,           // 0xC0 LATIN CAPITAL LETTER A WITH GRAVE
	ctype_mask_none,           // 0xC1 LATIN CAPITAL LETTER A WITH ACUTE
	ctype_mask_none,           // 0xC2 LATIN CAPITAL LETTER A WITH CIRCUMFLEX
	ctype_mask_none,           // 0xC3 LATIN CAPITAL LETTER A WITH TILDE
	ctype_mask_none,           // 0xC4 LATIN CAPITAL LETTER A WITH DIAERESIS
	ctype_mask_none,           // 0xC5 LATIN CAPITAL LETTER A WITH RING ABOVE
	ctype_mask_none,           // 0xC6 LATIN CAPITAL LETTER AE
	ctype_mask_none,           // 0xC7 LATIN CAPITAL LETTER C WITH CEDILLA
	ctype_mask_none,           // 0xC8 LATIN CAPITAL LETTER E WITH GRAVE
	ctype_mask_none,           // 0xC9 LATIN CAPITAL LETTER E WITH ACUTE
	ctype_mask_none,           // 0xCA LATIN CAPITAL LETTER E WITH CIRCUMFLEX
	ctype_mask_none,           // 0xCB LATIN CAPITAL LETTER E WITH DIAERESIS
	ctype_mask_none,           // 0xCC LATIN CAPITAL LETTER I WITH GRAVE
	ctype_mask_none,           // 0xCD LATIN CAPITAL LETTER I WITH ACUTE
	ctype_mask_none,           // 0xCE LATIN CAPITAL LETTER I WITH CIRCUMFLEX
	ctype_mask_none,           // 0xCF LATIN CAPITAL LETTER I WITH DIAERESIS
	ctype_mask_none,           // 0xD0 LATIN CAPITAL LETTER ETH
	ctype_mask_none,           // 0xD1 LATIN CAPITAL LETTER N WITH TILDE
	ctype_mask_none,           // 0xD2 LATIN CAPITAL LETTER O WITH GRAVE
	ctype_mask_none,           // 0xD3 LATIN CAPITAL LETTER O WITH ACUTE
	ctype_mask_none,           // 0xD4 LATIN CAPITAL LETTER O WITH CIRCUMFLEX
	ctype_mask_none,           // 0xD5 LATIN CAPITAL LETTER O WITH TILDE
	ctype_mask_none,           // 0xD6 LATIN CAPITAL LETTER O WITH DIAERESIS
	ctype_mask_none,           // 0xD7 MULTIPLICATION SIGN
	ctype_mask_none,           // 0xD8 LATIN CAPITAL LETTER O WITH STROKE
	ctype_mask_none,           // 0xD9 LATIN CAPITAL LETTER U WITH GRAVE
	ctype_mask_none,           // 0xDA LATIN CAPITAL LETTER U WITH ACUTE
	ctype_mask_none,           // 0xDB LATIN CAPITAL LETTER U WITH CIRCUMFLEX
	ctype_mask_none,           // 0xDC LATIN CAPITAL LETTER U WITH DIAERESIS
	ctype_mask_none,           // 0xDD LATIN CAPITAL LETTER Y WITH ACUTE
	ctype_mask_none,           // 0xDE LATIN CAPITAL LETTER THORN
	ctype_mask_none,           // 0xDF LATIN SMALL LETTER SHARP S
	ctype_mask_none,           // 0xE0 LATIN SMALL LETTER A WITH GRAVE
	ctype_mask_none,           // 0xE1 LATIN SMALL LETTER A WITH ACUTE
	ctype_mask_none,           // 0xE2 LATIN SMALL LETTER A WITH CIRCUMFLEX
	ctype_mask_none,           // 0xE3 LATIN SMALL LETTER A WITH TILDE
	ctype_mask_none,           // 0xE4 LATIN SMALL LETTER A WITH DIAERESIS
	ctype_mask_none,           // 0xE5 LATIN SMALL LETTER A WITH RING ABOVE
	ctype_mask_none,           // 0xE6 LATIN SMALL LETTER AE
	ctype_mask_none,           // 0xE7 LATIN SMALL LETTER C WITH CEDILLA
	ctype_mask_none,           // 0xE8 LATIN SMALL LETTER E WITH GRAVE
	ctype_mask_none,           // 0xE9 LATIN SMALL LETTER E WITH ACUTE
	ctype_mask_none,           // 0xEA LATIN SMALL LETTER E WITH CIRCUMFLEX
	ctype_mask_none,           // 0xEB LATIN SMALL LETTER E WITH DIAERESIS
	ctype_mask_none,           // 0xEC LATIN SMALL LETTER I WITH GRAVE
	ctype_mask_none,           // 0xED LATIN SMALL LETTER I WITH ACUTE
	ctype_mask_none,           // 0xEE LATIN SMALL LETTER I WITH CIRCUMFLEX
	ctype_mask_none,           // 0xEF LATIN SMALL LETTER I WITH DIAERESIS
	ctype_mask_none,           // 0xF0 LATIN SMALL LETTER ETH
	ctype_mask_none,           // 0xF1 LATIN SMALL LETTER N WITH TILDE
	ctype_mask_none,           // 0xF2 LATIN SMALL LETTER O WITH GRAVE
	ctype_mask_none,           // 0xF3 LATIN SMALL LETTER O WITH ACUTE
	ctype_mask_none,           // 0xF4 LATIN SMALL LETTER O WITH CIRCUMFLEX
	ctype_mask_none,           // 0xF5 LATIN SMALL LETTER O WITH TILDE
	ctype_mask_none,           // 0xF6 LATIN SMALL LETTER O WITH DIAERESIS
	ctype_mask_none,           // 0xF7 DIVISION SIGN
	ctype_mask_none,           // 0xF8 LATIN SMALL LETTER O WITH STROKE
	ctype_mask_none,           // 0xF9 LATIN SMALL LETTER U WITH GRAVE
	ctype_mask_none,           // 0xFA LATIN SMALL LETTER U WITH ACUTE
	ctype_mask_none,           // 0xFB LATIN SMALL LETTER U WITH CIRCUMFLEX
	ctype_mask_none,           // 0xFC LATIN SMALL LETTER U WITH DIAERESIS
	ctype_mask_none,           // 0xFD LATIN SMALL LETTER Y WITH ACUTE
	ctype_mask_none,           // 0xFE LATIN SMALL LETTER THORN
	ctype_mask_none            // 0xFF LATIN SMALL LETTER Y WITH DIAERESIS
};

} // namespace genome::detail
} // namespace genome
