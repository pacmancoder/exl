// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)#include "deleter_function.hpp"

#include <catch2/catch.hpp>

#include <exl/details/box/deleter_function.hpp>
#include <exl/impl/box/is_deleter_function.hpp>

#include <deleter_function_stub.hpp>

using namespace exl::test;

TEST_CASE("impl::is_static_deleter performs correct check", "[impl::is_deleter_function]")
{
    SECTION("When type is deleter_function .value() returns true")
    {
        REQUIRE(
                exl::impl::is_deleter_function<
                        exl::deleter_function<int[], array_deleter_stub>
                >::value()
        );
    }

    SECTION("When type is not deleter_function .value() returns false")
    {
        REQUIRE(!exl::impl::is_deleter_function<int>::value());
    }
}