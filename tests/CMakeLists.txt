# Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

add_executable(exl-test
        # Utilities
        test-utils/ClassMock.cpp
        # Tests
        tests.cpp
        mixed/impl/type_list.cpp
        matchers/matchers.cpp
        mixed/mixed.cpp
        option/option.cpp
        box/box.cpp
)

include_directories(exl-test ${CMAKE_CURRENT_LIST_DIR}/test-utils)

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
    include_directories(${TEST_NAME} ${CMAKE_CURRENT_LIST_DIR}/test-utils)
    target_link_libraries(${TEST_NAME} PRIVATE exl)
    target_link_libraries(${TEST_NAME} PRIVATE exl-coverage-config)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_tests_properties(${TEST_NAME} PROPERTIES WILL_FAIL TRUE)
endfunction()

add_termination_test(exl-mixed-invalid-unwrap-test mixed/mixed_invalid_unwrap_test.cpp)
add_termination_test(exl-mixed-invalid-unwrap-exact-test mixed/mixed_invalid_unwrap_exact_test.cpp)
add_termination_test(exl-box-invalid-dereferencing-test box/box_invalid_dereferencing_test.cpp)