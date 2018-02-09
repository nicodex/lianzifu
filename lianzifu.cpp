#include <genome/genome.hpp>
#include <genome/filesystem.hpp>
#include <genome/locale.hpp>
#include <genome/localization/stringtable.hpp>
#include <cstdlib>
#include <iostream>
#include <locale>
#include <string>
#include <stdexcept>

namespace /*{anonymous}*/ {

wchar_t const* const program_name = L"lianzifu";
wchar_t const* const program_desc = L"Risen 3 string table [un]packer";
wchar_t const* const program_vers = L"2.1";

char const* const default_ini = "#G3:/ini/loc.ini";
char const* const default_bin = "#G3:/data/compiled/localization/w_strings.bin";
char const* const default_map = "#G3:/lianzifu.csv";
char const* const default_flt = "*_Text;*_StageDir";
genome::platform const default_plt = genome::platform_x64;
int const default_ver = 6;
int const default_cmp = 9;
int const default_utf = 1;

void
init_locale(void)
{
	std::ios_base::sync_with_stdio(false);
	try {
		std::locale::global(std::locale(""));
	} catch (std::runtime_error&) {
		std::wclog << L";info: failed to use environment's default locale" << std::endl;
		return;
	}
	{
		std::locale loc = std::locale();
		std::cin.imbue(loc);
		std::wcin.imbue(loc);
		std::cout.imbue(loc);
		std::wcout.imbue(loc);
		std::cerr.imbue(loc);
		std::wcerr.imbue(loc);
		std::clog.imbue(loc);
		std::wclog.imbue(loc);
	}
}

void
print_version(void)
{
	std::wostream& out = std::wcout;
	out <<
		std::endl <<
		program_name << L" (" << program_desc << L") " << program_vers << std::endl <<
		L"Copyright (c) 2018 Nico Bendlin <nico@nicode.net>." << std::endl <<
		L"License MIT: The MIT License <http://opensource.org/licenses/MIT>." << std::endl <<
		L"This is free software: you are free to change and redistribute it." << std::endl <<
		L"There is NO WARRANTY, to the extent permitted by law." << std::endl <<
		std::endl;
}

void
print_help(int exit_code)
{
	std::wostream& out = (EXIT_SUCCESS == exit_code) ? std::wcout : std::wcerr;
	out << std::endl;
	out << L"Usage: " << program_name << L" <command [argument]...>..." << std::endl;
	out << std::endl;
	out << L"  The commands are executed in command line order." << std::endl;
	out << L"  Therefore, everything after --exit is ignored." << std::endl;
	out << std::endl;
	out << L"Commands:" << std::endl;
	out << std::endl;
	out << L"  --version                                print program version" << std::endl;
	out << L"  --help                                   print this help" << std::endl;
	out << L"  --exit                                   exit the program now" << std::endl;
	out << L"  --clear                                  reset string table state" << std::endl;
	out << L"  --read-ini [ini]                         add prefix/csv from <ini>" << std::endl;
	out << L"  --read-csv [utf]                         add strings from all csv" << std::endl;
	out << L"  --save-map [map]                         save [prefix:]id to <map>" << std::endl;
	out << L"  --save-bin [plt] [ver] [bin] [cmp] [flt] save string table to <bin>" << std::endl;
	out << L"  --read-map [map]                         add [prefix:]id from <map>" << std::endl;
	out << L"  --read-bin [bin]                         add csv/strings from <bin>" << std::endl;
	out << L"  --save-csv                               save strings to all csv" << std::endl;
	out << std::endl;
	out << L"Defaults:" << std::endl;
	out << std::endl;
	out << L"  <ini>  " << genome::to_wstring(std::string(default_ini)) << std::endl;
	out << L"  <utf>  " << genome::to_wstring(default_utf) << std::endl;
	out << L"  <map>  " << genome::to_wstring(std::string(default_map)) << std::endl;
	out << L"  <plt>  " << genome::to_wstring(std::string(genome::platform_name(default_plt))) << std::endl;
	out << L"  <ver>  " << genome::to_wstring(default_ver) << std::endl;
	out << L"  <bin>  " << genome::to_wstring(std::string(default_bin)) << std::endl;
	out << L"  <cmp>  " << genome::to_wstring(default_cmp) << std::endl;
	out << L"  <flt>  " << genome::to_wstring(std::string(default_flt)) << std::endl;
	out << std::endl;
	out << L"Platforms:" << std::endl;
	out << std::endl;
	for (int i = genome::platform_unknown;;) {
		char const* name = genome::platform_name(genome::platform(++i));
		if (genome::platform_from_name(name) != i) {
			break;
		}
		out << L"  " << genome::to_wstring(std::string(name)) << std::wstring(6 - std::char_traits<char>::length(name), L' ') << genome::to_wstring(std::string(genome::platform_desc(genome::platform(i)))) << std::endl;
	}
	out << std::endl;
	out << L"Map format:" << std::endl;
	out << std::endl;
	out << L"  The map files are used to restore the string identifiers" << std::endl;
	out << L"  (the binary string table contains only the hash values)." << std::endl;
	out << std::endl;
	out << L"  Map files have to be encoded in UTF-8 (BOM is optional)." << std::endl;
	out << L"  Every non-empty line contains <[prefix:]id>[|...], where" << std::endl;
	out << L"  the prefix is optional (used to select the unpacked CSV," << std::endl;
	out << L"  defaults to first CSV in loc.ini for unknown id hashes)," << std::endl;
	out << L"  and everything after | will be ignored on read (hashes)." << std::endl;
	out << std::endl;
	out << L"CSV format:" << std::endl;
	out << std::endl;
	out << L"  The first line defines the case-insensitive column names." << std::endl;
	out << L"  The first row contains the case-insensitive string ids or" << std::endl;
	out << L"  an eight-character, hexadecimal, and case-sensitive hash." << std::endl;
	out << std::endl;
	out << L"  Without a BOM the encoding is detected based on the first" << std::endl;
	out << L"  two octets (X,0 = UTF-16LE; 0,X = UTF-16BE; X,X = UTF-8)." << std::endl;
	out << L"  This extension to the Genome CSV format has been added to" << std::endl;
	out << L"  simplify the work flow with CSV tools that do not write a" << std::endl;
	out << L"  BOM and could be disabled with --read-csv 0 if necessary." << std::endl;
	out << std::endl;
	out << L"  Records are separated with a LINE FEED (U+000A), the NULL" << std::endl;
	out << L"  (U+0000) is not allowed, and all CARRIAGE RETURN (U+000D)" << std::endl;
	out << L"  are ignored during read. Record fields are separated with" << std::endl;
	out << L"  VERTICAL LINE (U+007C), there's no field quote character," << std::endl;
	out << L"  leading and trailing white space is not removed, and only" << std::endl;
	out << L"  the following escape sequences are available:" << std::endl;
	out << L"    \"\\a\"  COMMERCIAL AT (U+0040)" << std::endl;
	out << L"    \"\\n\"  LINE FEED (U+000A)" << std::endl;
	out << L"    \"\\r\"  CARRIAGE RETURN (U+000D)" << std::endl;
	out << L"    \"\\\\\"  REVERSE SOLIDUS (U+005C)" << std::endl;
	out << L"    \"\\v\"  VERTICAL LINE (U+007C)" << std::endl;
	out << L"  The latter two are extensions to the Genome CSV format to" << std::endl;
	out << L"  permit encoding of field separators and escape sequences." << std::endl;
	out << std::endl;
	out << L"Examples:" << std::endl;
	out << std::endl;
	out << L"  create " << genome::to_wstring(std::string(default_bin)) << L" (" << genome::to_wstring(std::string(genome::platform_name(default_plt))) << L" v" << genome::to_wstring(default_ver) << L") from CSVs" << std::endl;
	out << std::endl;
	out << L"    " << program_name << L" --read-ini --read-csv --save-bin" << std::endl;
	out << std::endl;
	out << L"  export " << genome::to_wstring(std::string(default_map)) << L" from CSVs in " << genome::to_wstring(std::string(default_ini)) << std::endl;
	out << std::endl;
	out << L"    " << program_name << L" --read-ini --read-csv --save-map" << std::endl;
	out << std::endl;
	out << L"  unpack " << genome::to_wstring(std::string(default_bin)) << L" with " << genome::to_wstring(std::string(default_map)) << std::endl;
	out << std::endl;
	out << L"    " << program_name << L" --read-map --read-ini --read-bin --save-csv" << std::endl;
	out << std::endl;
}

bool
cmd_next(int& argc, char**& argv, std::string& cmd, std::vector<std::string>& args)
{
	args.clear();
	if (0 >= argc) {
		cmd.erase();
		return (false);
	}
	--argc;
	cmd.assign(*argv ? *argv : "");
	++argv;
	if ((cmd.size() <= 2) || ('-' != cmd[0]) || ('-' != cmd[1])) {
		return (false);
	}
	cmd.erase(0, 2);
	while (argc > 0) {
		std::string arg(*argv ? *argv: "");
		if ((arg.size() >= 2) && ('-' == arg[0]) && ('-' == arg[1])) {
			return (true);
		}
		args.push_back(arg);
		--argc; ++argv;
	}
	return (true);
}

} // namespace {anonymous}

int
main(int argc, char* argv[])
{
	int exit_code = EXIT_SUCCESS;
	try {
		init_locale();
		init_genome();

		--argc; ++argv;
		if (argc <= 0) {
			exit_code = EXIT_FAILURE;
			print_help(exit_code);
		} else {
			std::string cmd;
			std::vector<std::string> args;
			genome::localization::stringtable stb;
			do {
				if (!cmd_next(argc, argv, cmd, args)) {
					throw std::invalid_argument("invalid command '" + cmd + "'");
				}
				if (cmd == "version") {

					print_version();
					if (args.size() > 0) {
						throw std::invalid_argument("too many arguments for --" + cmd);
					}

				} else if ("help" == cmd) {

					print_help(exit_code);
					if (args.size() > 0) {
						throw std::invalid_argument("too many arguments for --" + cmd);
					}

				} else if ("exit" == cmd) {

					// additional arguments and the following commands are ignored
					exit(exit_code);

				} else if ("clear" == cmd) {

					stb.clear();

				} else if ("read-ini" == cmd) {

					if (args.size() > 1) {
						throw std::invalid_argument("too many arguments for --" + cmd);
					}
					if (args.size() < 1) {
						args.push_back(default_ini);
					}
					stb.read_ini(args[0].c_str());

				} else if ("read-csv" == cmd) {

					if (args.size() > 1) {
						throw std::invalid_argument("too many arguments for --" + cmd);
					}
					if (args.size() < 1) {
						args.push_back(genome::to_string(default_utf));
					}
					int utf = atoi(args[0].c_str());
					if ((utf < 0) || (1 < utf) || (genome::to_string(utf) !=  args[0])) {
						throw std::invalid_argument("invalid UTF flag");
					}
					stb.read_csv(!!utf);

				} else if ("save-map" == cmd) {

					if (args.size() > 1) {
						throw std::invalid_argument("too many arguments for --" + cmd);
					}
					if (args.size() < 1) {
						args.push_back(default_map);
					}
					stb.save_map(args[0].c_str());

				} else if ("save-bin" == cmd) {

					if (args.size() > 5) {
						throw std::invalid_argument("too many arguments for --" + cmd);
					}
					if (args.size() < 1) {
						args.push_back(genome::platform_name(genome::platform_x64));
					}
					if (args.size() < 2) {
						args.push_back(genome::to_string(default_ver));
					}
					if (args.size() < 3) {
						args.push_back(default_bin);
					}
					if (args.size() < 4) {
						args.push_back(genome::to_string(default_cmp));
					}
					if (args.size() < 5) {
						args.push_back(default_flt);
					}
					genome::platform target = genome::platform_from_name(args[0].c_str());
					if (genome::platform_unknown == target) {
						throw std::invalid_argument("invalid target platform");
					}
					int version = atoi(args[1].c_str());
					if ((version < 5) || (255 < version) || (genome::to_string(version) !=  args[1])) {
						throw std::invalid_argument("invalid string table version");
					}
					int level = atoi(args[3].c_str());
					if ((level < 0) || (9 < level) || (genome::to_string(level) != args[3])) {
						throw std::invalid_argument("invalid compression level");
					}
					genome::localization::stringtable::compression comp =
						(level <= 0) ? genome::localization::stringtable::compression_none : (
						(level <= 1) ? genome::localization::stringtable::compression_fast : (
						(level <= 4) ? genome::localization::stringtable::compression_lzpb : (
						(level <= 6) ? genome::localization::stringtable::compression_lzex : (
						(level <= 8) ? genome::localization::stringtable::compression_tree :
						               genome::localization::stringtable::compression_best))));
					genome::byte_string filter;
					if (!genome::string_convert(args[4], filter)) {
						throw std::invalid_argument("invalid column filter");
					}
					stb.save_bin(target, genome::u8(version), args[2].c_str(), comp, filter);

				} else if ("read-map" == cmd) {

					if (args.size() > 1) {
						throw std::invalid_argument("too many arguments for --" + cmd);
					}
					if (args.size() < 1) {
						args.push_back(default_map);
					}
					stb.read_map(args[0].c_str());

				} else if ("read-bin" == cmd) {

					if (args.size() > 1) {
						throw std::invalid_argument("too many arguments for --" + cmd);
					}
					if (args.size() < 1) {
						args.push_back(default_bin);
					}
					stb.read_bin(args[0].c_str());

				} else if ("save-csv" == cmd) {

					if (args.size() > 0) {
						throw std::invalid_argument("too many arguments for --" + cmd);
					}
					stb.save_csv();

				} else {

					throw std::invalid_argument("unsupported command '" + cmd + "'");

				}
			} while (argc > 0);
		}

	} catch (std::exception& e) {
		std::cerr << ";fail: (" << genome::get_exception_name(e) << ") " << e.what() << std::endl;
		exit_code = EXIT_FAILURE;
	}
	return (exit_code);
}
