cmake_minimum_required(VERSION 3.2)
project(lianzifu)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

set(LIANZIFU_SOURCE_FILES
    genome/localization/stringtable.cpp
    genome/localization/stringtable.hpp
    genome/archive.cpp
    genome/archive.hpp
    genome/archive.ipp
    genome/archive_detail.ipp
    genome/filesystem.cpp
    genome/filesystem.hpp
    genome/genome.cpp
    genome/genome.hpp
    genome/genome.ipp
    genome/hash.cpp
    genome/hash.hpp
    genome/locale.cpp
    genome/locale.hpp
    genome/locale.ipp
    genome/locale_detail.cpp
    genome/locale_detail.hpp
    genome/locale_detail.ipp
    genome/locale_glibcxx.cpp
    genome/string.cpp
    genome/string.hpp
    genome/string.ipp
    genome/time.cpp
    genome/time.hpp
    genome/tstream.cpp
    genome/tstream.hpp
    genome/tstream.ipp
    nicode/suffix_tree.hpp
    nicode/suffix_tree.ipp
    lianzifu.cpp)

include_directories(.)

add_executable(lianzifu ${LIANZIFU_SOURCE_FILES})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(lianzifu PRIVATE
        -Wall
        -Wextra
        -pedantic
    )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    target_compile_options(lianzifu PRIVATE
        /Wall
        # class configuration is done with enum members and the project doesn't require C++11 (std::enable_if)
        /wd4127 # (level 4) conditional expression is constant
        #TODO: the signed/unsigned mismatch warning shouldn't be disabled, but it's hard to get rid of it with MSVC 2017
        /wd4365 # (level 4) 'action' : conversion from 'type_1' to 'type_2', signed/unsigned mismatch
        /wd4514 # (level 4) 'function' : unreferenced inline function has been removed
        /wd4548 # (level 1) expression before comma has no effect; expected expression with side-effect
        /wd4571 # (level 4) Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
        /wd4623 # (level 4) 'derived class' : default constructor was implicitly defined as deleted because a base class default constructor is inaccessible or deleted
        /wd4625 # (level 4) 'derived class' : copy constructor was implicitly defined as deleted because a base class copy constructor is inaccessible or deleted
        /wd4626 # (level 4) 'derived class' : assignment operator was implicitly defined as deleted because a base class assignment operator is inaccessible or deleted
        # this program is single threaded and the singletons are created during initialization
        /wd4640 # (level 3) 'instance' : construction of local static object is not thread-safe
        /wd4710 # (level 4) 'function' : function not inlined
        /wd4711 # (level 1) function 'function' selected for inline expansion
        /wd4774 # (level 4) 'string' : format string expected in argument number is not a string literal
        /wd4820 # (level 4) 'bytes' bytes padding added after construct 'member_name'
        /wd5026 # (level 1)(level 4) 'type': move constructor was implicitly defined as deleted
        /wd5027 # (level 1)(level 4) 'type': move assignment operator was implicitly defined as deleted
        )
    # Compiler Warning (level 3) C4996
    target_compile_definitions(lianzifu PRIVATE
        # This function or variable may be unsafe. Consider using safe_version instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
        _CRT_SECURE_NO_WARNINGS
        # 'std::function_name::_Unchecked_iterators::_Deprecate' Call to std::function_name with parameters that may be unsafe - this call relies on the caller to check that the passed values are correct. To disable this warning, use -D_SCL_SECURE_NO_WARNINGS. See documentation on how to use Visual C++ 'Checked Iterators'
        _SCL_SECURE_NO_WARNINGS
    )
endif()
