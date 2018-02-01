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
        /wd4127
        /wd4365
        /wd4514
        /wd4548
        /wd4571
        /wd4623
        /wd4625
        /wd4626
        /wd4640
        /wd4710
        /wd4711
        /wd4774
        /wd4820
        /wd5026
        /wd5027
        )
    target_compile_definitions(lianzifu PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _SCL_SECURE_NO_WARNINGS
    )
endif()
