# Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

add_executable(exl-test
        # Utilities
        mock/ClassMock.cpp
        # Tests
        tests.cpp
        type_list.cpp
        matchers.cpp
        mixed.cpp
)

target_link_libraries(exl-test PRIVATE exl)
target_link_libraries(exl-test PRIVATE Catch2)
target_link_libraries(exl-test PRIVATE exl-coverage-config)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(exl-test PRIVATE
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
    target_compile_options(exl-test PRIVATE
            /WX
            /W4
    )
endif ()

add_executable(exl-mixed-invalid-unwrap-test mixed_invalid_unwrap_test.cpp)
target_link_libraries(exl-mixed-invalid-unwrap-test PRIVATE exl)
target_link_libraries(exl-mixed-invalid-unwrap-test PRIVATE exl-coverage-config)

add_test(NAME exl-test COMMAND exl-test)
add_test(NAME exl-mixed-invalid-unwrap-test COMMAND exl-mixed-invalid-unwrap-test)
set_tests_properties(exl-mixed-invalid-unwrap-test PROPERTIES WILL_FAIL TRUE)