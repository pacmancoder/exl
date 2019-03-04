// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)#include "deleter_function.hpp"

#include <catch2/catch.hpp>

#include <exl/details/box/deleter_function.hpp>

#include <deleter_function_stub.hpp>

using namespace exl::test;

TEST_CASE("deleter_function deletes object of .destroy() call", "[deleter_function]")
{
    SECTION("Scalar specialization")
    {
        exl::deleter_function<int, scalar_deleter_stub> deleter;
        int value = 0;
        deleter.destroy(&value);
        REQUIRE(value == 42);
    }

    SECTION("Array specialization")
    {
        exl::deleter_function<int[], array_deleter_stub> deleter;
        int values[3] = { 0 };
        int* values_ptr = values;
        deleter.destroy(values_ptr);
        REQUIRE(values[0] == 1);
        REQUIRE(values[1] == 2);
        REQUIRE(values[2] == 3);
    }
}