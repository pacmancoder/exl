# Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.7)

project(exl LANGUAGES CXX)

option(EXL_ENABLE_TESTING "Enable exl library testing" OFF)
option(EXL_ENABLE_COVERAGE "Enable exl library coverage reporting" OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_library(exl-coverage-config INTERFACE)
if(EXL_ENABLE_COVERAGE)
    # Add required flags (GCC & LLVM/Clang)
    target_compile_options(exl-coverage-config INTERFACE
            -O0        # no optimization
            -g         # generate debug info
            --coverage # sets all required flags
            )
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
        target_link_options(exl-coverage-config INTERFACE --coverage)
    else()
        target_link_libraries(exl-coverage-config INTERFACE --coverage)
    endif()
endif()

add_library(exl INTERFACE)
target_include_directories(exl INTERFACE include)

if (EXL_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(libs)
    add_subdirectory(tests)
endif ()
