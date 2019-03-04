// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)#include "deleter_object.hpp"

#include <catch2/catch.hpp>

#include <exl/details/box/deleter_object.hpp>

#include <StubDeleter.hpp>
#include <stub_class_deleter_hierarchy.hpp>

using namespace exl::test;

TEST_CASE("impl::deleter_object size test", "[impl::deleter_object]")
{
    SECTION("Has minimal size")
    {
        REQUIRE(sizeof(exl::deleter_object<int, StubDeleter>) == sizeof(StubDeleter));
    }
}

TEST_CASE("exl::impl::deleter_object construction test", "[impl::deleter_object]")
{
    SECTION("Default constructs")
    {
        exl::deleter_object<int, StubDeleter> deleter;
        int value = 0;
        deleter.destroy(&value);
        REQUIRE(value == 399);
    }

    SECTION("Move constructs")
    {
        const int new_value = 42;
        exl::deleter_object<int, StubDeleter> deleter((StubDeleter(&new_value)));
        int value = 0;
        deleter.destroy(&value);
        REQUIRE(value == new_value);
    }

    SECTION("Move constructs from convertible deleter_object")
    {
        exl::deleter_object<StubBaseClass, StubBaseClassDeleter> deleter(
                (exl::deleter_object<StubDerivedClass, StubDerivedClassDeleter>())
        );

        StubDerivedClass v;

        // Derived object can be passed
        deleter.destroy(&v);

        // Derived deleter was called
        REQUIRE(v.base_tag == 1);
    }
}

TEST_CASE("exl::impl::deleter_object assignment test", "[impl::deleter_object]")
{
    SECTION("Move assigns")
    {
        const int new_value = 42;
        exl::deleter_object<int, StubDeleter> deleter;
        deleter = exl::deleter_object<int, StubDeleter>((StubDeleter(&new_value)));
        int value = 0;
        deleter.destroy(&value);
        REQUIRE(value == new_value);
    }
}