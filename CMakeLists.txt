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

if(EXL_ENABLE_COVERAGE)
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/libs/cmake-codecov/cmake" ${CMAKE_MODULE_PATH})
    set(ENABLE_COVERAGE ON CACHE BOOL "Enable coverage build." FORCE)
    find_package(codecov)
endif()

add_library(exl INTERFACE)
target_include_directories(exl INTERFACE include)

if (EXL_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(libs)
    add_subdirectory(tests)
endif ()

if(EXL_ENABLE_COVERAGE)
    list(APPEND LCOV_REMOVE_PATTERNS "'/usr/*'")
    coverage_evaluate()
endif()