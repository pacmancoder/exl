// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <exception>

#include <catch2/catch.hpp>

#include <exl/mixed_option.hpp>

#include "mock/ClassMock.hpp"

using namespace exl::mock;

TEST_CASE("Mixed optional take method test", "[mixed_optional]")
{
    using Mixed = exl::mixed_option<ClassMock, int>;
    CallCounter counter;

    SECTION("Returned value")
    {
        Mixed m(exl::in_place_type_t<ClassMock>(), 1, &counter);
        auto value = m.take<ClassMock>();
        SECTION("Is move constructed")
        {
            REQUIRE(counter.count(CallType::Construct, as_moved_tag(1)) == 1);
            REQUIRE(counter.count(CallType::Move, 1) == 1);
        }

        SECTION("Is value same")
        {
            REQUIRE(value.original_tag() == 1);
        }
    }

    SECTION("New mixed value is exl::none")
    {
        Mixed m(42);
        m.take<int>();

        REQUIRE(m.is<exl::none>());
    }

    SECTION("New mixed value is exl::none")
    {
        Mixed m1(42);
        m1.take<int>();

        REQUIRE(m1.is<exl::none>());
    }
}

TEST_CASE("Mixed option forward constructor", "[mixed_optional]")
{
    using Mixed = exl::mixed_option<ClassMock, int, std::string>;

    CallCounter counter;

    SECTION("From best match")
    {
        Mixed m("hello");

        REQUIRE(m.is<std::string>());
        REQUIRE(m.unwrap<std::string>() == std::string("hello"));
    }

    SECTION("In-place")
    {
        Mixed m(exl::in_place_type_t<ClassMock>(), 4, &counter);

        REQUIRE(m.is<ClassMock>());
        REQUIRE(m.unwrap<ClassMock>().tag() == 4);

        REQUIRE(counter.count(CallType::Construct, 4) == 1);
        REQUIRE(counter.count(CallType::Copy, 4) == 0);
        REQUIRE(counter.count(CallType::Move, 4) == 0);
    }

    SECTION("From subset")
    {
        using MixedSuperset = exl::mixed_option<std::string, char, ClassMock, std::exception, int>;
        Mixed m_subset("hi");
        MixedSuperset m(m_subset);

        REQUIRE(m.is<std::string>());
        REQUIRE(m.unwrap<std::string>() == std::string("hi"));
    }
}

TEST_CASE("Mixed option forward assignment", "[mixed_optional]")
{
    using Mixed = exl::mixed_option<ClassMock, int, std::string>;

    CallCounter counter;

    SECTION("From best match")
    {
        Mixed m = exl::none();
        m = "hello";

        REQUIRE(m.is<std::string>());
        REQUIRE(m.unwrap<std::string>() == std::string("hello"));
    }

    SECTION("From subset")
    {
        using MixedSuperset = exl::mixed_option<std::string, char, ClassMock, std::exception, int>;
        Mixed m_subset("hi");

        MixedSuperset m = exl::none();
        m = m_subset;

        REQUIRE(m.is<std::string>());
        REQUIRE(m.unwrap<std::string>() == std::string("hi"));
    }
}

TEST_CASE("Mixed option make test", "[mixed]")
{
    using Mixed = exl::mixed_option<int, std::runtime_error, std::string, ClassMock>;

    SECTION("Forwards elements")
    {
        auto m = Mixed::make<std::string>("hello");
        REQUIRE(m.is<std::string>());
        REQUIRE(m.unwrap<std::string>() == std::string("hello"));
    }

    SECTION("From none")
    {
        auto m = Mixed::make<exl::none>();
        REQUIRE(m.is<exl::none>());
    }

    SECTION("RVO triggered")
    {
        CallCounter counter;
        auto m = Mixed::make<ClassMock>(1, &counter);

        REQUIRE(counter.count(CallType::Construct, 1) == 1);
        REQUIRE(counter.count(CallType::Move, 1) == 0);
        REQUIRE(counter.count(CallType::Copy, 1) == 0);
    }
}

TEST_CASE("Mixed option is_none test", "[mixed_optional]")
{
    using Mixed = exl::mixed_option<int>;

    auto m = Mixed::make<exl::none>();
    REQUIRE(m.is_none());

    m = 42;
    REQUIRE(!m.is_none());
}

TEST_CASE("Mixed option make_none test", "[mixed_optional]")
{
    using Mixed = exl::mixed_option<int>;
    auto m = Mixed::make_none();

    REQUIRE(m.is<exl::none>());
}