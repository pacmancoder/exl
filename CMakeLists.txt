# Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.7)

project(exl)

set(CMAKE_CXX_STANDARD 11)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

set(TEST_PROJECT_NAME ${PROJECT_NAME}-test)

add_executable(${TEST_PROJECT_NAME}
    tests/tests.cpp
    tests/type_list.cpp
    tests/mixed.cpp
)

include_directories(${TEST_PROJECT_NAME} third-party)
target_link_libraries(${TEST_PROJECT_NAME} PRIVATE ${PROJECT_NAME})
