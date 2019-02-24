# Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

add_executable(exl-test
        # Utilities
        utils/ClassMock.cpp
        # Tests
        tests.cpp
        type_list.cpp
        matchers.cpp
        mixed.cpp
        option.cpp
        box.cpp
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

add_test(NAME exl-test COMMAND exl-test)

function(add_termination_test TEST_NAME TEST_FILE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE exl)
    target_link_libraries(${TEST_NAME} PRIVATE exl-coverage-config)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_tests_properties(${TEST_NAME} PROPERTIES WILL_FAIL TRUE)
endfunction()

add_termination_test(exl-mixed-invalid-unwrap-test mixed_invalid_unwrap_test.cpp)
add_termination_test(exl-mixed-invalid-unwrap-exact-test mixed_invalid_unwrap_exact_test.cpp)
add_termination_test(exl-box-invalid-dereferencing-test box_invalid_dereferencing_test.cpp)