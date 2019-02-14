# Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

project(exl-test)

add_executable(${PROJECT_NAME}
        mock/ClassMock.cpp

        tests.cpp
        type_list.cpp
        matchers.cpp
        mixed.cpp
        )

target_link_libraries(${PROJECT_NAME} PRIVATE exl)
target_link_libraries(${PROJECT_NAME} PRIVATE Catch2)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(
            ${PROJECT_NAME} PRIVATE
            -Werror
            -Wall
            -Wextra
            -Wunreachable-code
            -Wmissing-declarations
            -Wold-style-cast
            -Winit-self
            -pedantic
    )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(
            ${PROJECT_NAME} PRIVATE
            /WX
            /W4
    )
endif ()

add_test(
        NAME ${PROJECT_NAME}
        COMMAND ${PROJECT_NAME}
)