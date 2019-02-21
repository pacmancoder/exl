// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <exl/option.hpp>

#include "mock/ClassMock.hpp"

using namespace exl::mock;

TEST_CASE("Option construction test", "[option]")
{
    SECTION("Best match construction")
    {
        exl::option<std::string> option("hello");

        REQUIRE(option.is<std::string>());
        REQUIRE(option.unwrap<std::string>() == std::string("hello"));
    }

    SECTION("In-place")
    {
        CallCounter counter;

        exl::option<ClassMock> m(exl::in_place_type_t<ClassMock>(), 4, &counter);

        REQUIRE(m.is<ClassMock>());
        REQUIRE(m.unwrap<ClassMock>().tag() == 4);

        REQUIRE(counter.count(CallType::Construct, 4) == 1);
        REQUIRE(counter.count(CallType::Copy, 4) == 0);
        REQUIRE(counter.count(CallType::Move, 4) == 0);
    }
}

TEST_CASE("Option forward assignment", "[option]")
{
    CallCounter counter;

    SECTION("From best match")
    {
        auto option = exl::option<std::string>::make_none();
        option = "hello";

        REQUIRE(option.is<std::string>());
        REQUIRE(option.unwrap<std::string>() == std::string("hello"));
    }

    SECTION("Move assignment forwarding")
    {
        auto option = exl::option<ClassMock>::make_none();
        option = ClassMock(1, &counter);

        REQUIRE(counter.count(CallType::Construct, as_moved_tag(1)) == 1);
        REQUIRE(counter.count(CallType::Move, 1) == 1);

        REQUIRE(counter.count(CallType::Copy, as_moved_tag(1)) == 0);
        REQUIRE(counter.count(CallType::Move, as_moved_tag(1)) == 0);
    }

    SECTION("Copy assignment forwarding")
    {
        auto option = exl::option<ClassMock>::make_none();
        ClassMock mock(1, &counter);
        option = mock;

        REQUIRE(counter.count(CallType::Construct, as_copied_tag(1)) == 1);
        REQUIRE(counter.count(CallType::Copy, 1) == 1);

        REQUIRE(counter.count(CallType::Copy, as_copied_tag(1)) == 0);
        REQUIRE(counter.count(CallType::Move, as_copied_tag(1)) == 0);
    }

    SECTION("Forward option assignment")
    {
        const auto option1 = exl::option<int>::make_some(42);
        auto option2 = exl::option<int>::make_none();

        option2 = option1;
        REQUIRE(option2.is_some());
        REQUIRE(option2.unwrap_some() == 42);
    }
}

TEST_CASE("Option is some test", "[option]")
{
    SECTION("Has some")
    {
        exl::option<std::string> option("hello");

        REQUIRE(option.is_some());
    }

    SECTION("Has none")
    {
        auto option = exl::option<std::string>::make<exl::none>();

        REQUIRE(!option.is_some());
    }
}

TEST_CASE("Option is none test", "[option]")
{
    SECTION("Has some")
    {
        exl::option<std::string> option("hello");

        REQUIRE(!option.is_none());
    }

    SECTION("Has none")
    {
        auto option = exl::option<std::string>::make<exl::none>();

        REQUIRE(option.is_none());
    }
}

TEST_CASE("Option make none test", "[option]")
{
    auto option = exl::option<std::string>::make_none();
    REQUIRE(option.is_none());
}

TEST_CASE("Option make some test", "[option]")
{
    auto option = exl::option<std::string>::make_some("hello");
    REQUIRE(option.is_some());
    REQUIRE(option.unwrap<std::string>() == std::string("hello"));
}

TEST_CASE("Option unwrap some test", "[option]")
{
    auto option = exl::option<std::string>::make_some("hello");
    REQUIRE(option.unwrap_some() == std::string("hello"));
}

TEST_CASE("Option const unwrap some test", "[option]")
{
    const auto option = exl::option<std::string>::make_some("hello");
    REQUIRE(option.unwrap_some() == std::string("hello"));
}