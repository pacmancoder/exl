// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <algorithm>
#include <type_traits>
#include <functional>

#include <catch2/catch.hpp>

#include <exl/mixed.hpp>

#include "class_mock.h"

using namespace exl::impl;
using namespace exl::mock;


TEST_CASE("Mixed type construction test", "[mixed]")
{
    using Mixed = exl::mixed<int, char, std::string, ClassMock, SecondClassMock>;
    CallCounter calls;
    ClassMock mock(1, &calls);

    SECTION("Copy constructor called")
    {
        Mixed m(mock);
        REQUIRE(calls.count(CallType::Move, mock.tag()) == 0);
        REQUIRE(calls.count(CallType::Copy, mock.tag()) == 1);
        REQUIRE(calls.count(CallType::Construct, as_copied_tag(mock.tag())) == 1);
    }

    SECTION("Move constructor called")
    {
        Mixed m(std::move(mock));
        REQUIRE(calls.count(CallType::Move, mock.tag()) == 1);
        REQUIRE(calls.count(CallType::Copy, mock.tag()) == 0);
        REQUIRE(calls.count(CallType::Construct, as_moved_tag(mock.tag())) == 1);
    }

    SECTION("Correct tag assigned on construction")
    {
        Mixed m(ClassMock(1));
        REQUIRE(m.is<ClassMock>() == true);

        Mixed m2(char(255));
        REQUIRE(m2.is<char>() == true);
    }
}

TEST_CASE("Mixed type correct destructor call test", "[mixed]")
{
    using Mixed = exl::mixed<int, ClassMock, SecondClassMock>;
    CallCounter calls;
    ClassMock mock(1, &calls);

    SECTION("Mock is destroyed")
    {
        {
            Mixed m(mock);
        }
        // Destroyed

        REQUIRE(calls.count(CallType::Destroy, as_copied_tag(mock.tag())) == 1);
    }
}

TEST_CASE("Mixed type assignment operators test", "[mixed]")
{
    using Mixed = exl::mixed<int, ClassMock, SecondClassMock>;
    CallCounter calls;

    SECTION("Copy-assignment different type")
    {
        ClassMock mock1(1, &calls);
        SecondClassMock mock2(2, &calls);

        Mixed m(mock1);
        m = mock2;

        // Old object destroyed, new -- copy-constructed
        REQUIRE(calls.count(CallType::Destroy, as_copied_tag(1)) == 1);
        REQUIRE(calls.count(CallType::Construct, as_copied_tag(2)) == 1);
        REQUIRE(calls.count(CallType::Copy, 2) == 1);
    }

    SECTION("Move-assignment of different type")
    {
        ClassMock mock1(1, &calls);
        SecondClassMock mock2(2, &calls);

        Mixed m(mock1);
        m = std::move(mock2);

        // Old object destroyed, new -- copy-constructed
        REQUIRE(calls.count(CallType::Destroy, as_copied_tag(1)) == 1);
        REQUIRE(calls.count(CallType::Construct, as_moved_tag(2)) == 1);
        REQUIRE(calls.count(CallType::Move, 2) == 1);
    }

    SECTION("Copy-assignment of same type")
    {
        ClassMock mock1(1, &calls);
        ClassMock mock2(2, &calls);

        Mixed m(mock1);
        m = mock2;

        // Old object NOT destroyed, new -- copied (not constructed)
        REQUIRE(calls.count(CallType::Destroy, as_copied_tag(1)) == 0);
        REQUIRE(calls.count(CallType::Construct, as_copied_tag(2)) == 0);
        REQUIRE(calls.count(CallType::Assign, as_copied_tag(1)) == 1);
        REQUIRE(calls.count(CallType::Copy, 2) == 1);

        REQUIRE(m.is<ClassMock>());
        REQUIRE(m.unwrap<ClassMock>().tag() == as_copied_tag(2));
    }

    SECTION("Move-assignment of same type")
    {
        ClassMock mock1(1, &calls);
        ClassMock mock2(2, &calls);

        Mixed m(mock1);
        m = std::move(mock2);

        // Old object NOT destroyed, new -- moved (not constructed)
        REQUIRE(calls.count(CallType::Destroy, as_copied_tag(1)) == 0);
        REQUIRE(calls.count(CallType::Construct, as_moved_tag(2)) == 0);
        REQUIRE(calls.count(CallType::Assign, as_copied_tag(1)) == 1);
        REQUIRE(calls.count(CallType::Move, 2) == 1);

        REQUIRE(m.is<ClassMock>());
        REQUIRE(m.unwrap<ClassMock>().tag() == as_moved_tag(2));
    }
}

TEST_CASE("Mixed type construct from another mixed test", "[mixed][.]")
{
    REQUIRE(false);
}

TEST_CASE("Mixed type assign from another mixed test", "[mixed][.]")
{
    REQUIRE(false);
}

TEST_CASE("Mixed type in-place construction test", "[mixed][.]")
{
    REQUIRE(false);
}

TEST_CASE("Mixed type emplace test", "[mixed][.]")
{
    REQUIRE(false);
}
