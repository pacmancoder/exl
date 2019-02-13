// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <functional>

#include <catch2/catch.hpp>

#include <exl/matchers.hpp>

namespace
{
    int square(const int& value)
    {
        return value * value;
    }

    class CallableMock
    {
    public:
        CallableMock()
                : is_copied(false)
                , is_moved(false) {}

        CallableMock(const CallableMock& rhs)
                : is_copied(true)
                , is_moved(rhs.is_moved) {}

        CallableMock(CallableMock&& rhs)
                : is_copied(rhs.is_copied)
                , is_moved(true) {}

        int operator()(const int& value)
        {
            return square(value);
        }

        bool is_copied;
        bool is_moved;
    };
}

TEST_CASE("Matcher exl::when can be called with any callable")
{
    SECTION("With lambda")
    {
        int result = 1;
        auto matcher = exl::when(
                [&result](const int& arg)
                {
                    result = arg;
                    return 399;
                }
        );

        REQUIRE(matcher.impl(42) == 399);
        REQUIRE(result == 42);
    }

    SECTION("With function pointer")
    {
        auto matcher = exl::when(&square);

        REQUIRE(matcher.impl(5) == 25);
    }

    SECTION("With callable object copied")
    {
        CallableMock callable;
        auto matcher = exl::when(callable);

        REQUIRE(matcher.impl(5) == 25);
        REQUIRE(matcher.impl.is_copied);
        REQUIRE(!matcher.impl.is_moved);
    }

    SECTION("With callable object moved")
    {
        auto matcher = exl::when(CallableMock());

        REQUIRE(matcher.impl(5) == 25);
        REQUIRE(!matcher.impl.is_copied);
        REQUIRE(matcher.impl.is_moved);
    }
}

TEST_CASE("Matcher exl::when result matcher has correct matching type")
{
    auto matcher = exl::when(&square);
    REQUIRE(std::is_same<decltype(matcher)::type, exl::impl::marker::matcher_when>::value);
}

TEST_CASE("Matcher exl::when_exact can be called with any callable")
{
    SECTION("With lambda")
    {
        int result = 1;
        auto matcher = exl::when_exact(
                [&result](const int& arg)
                {
                    result = arg;
                    return 399;
                }
        );

        REQUIRE(matcher.impl(42) == 399);
        REQUIRE(result == 42);
    }

    SECTION("With function pointer")
    {
        auto matcher = exl::when_exact(&square);

        REQUIRE(matcher.impl(5) == 25);
    }

    SECTION("With callable object copied")
    {
        CallableMock callable;
        auto matcher = exl::when_exact(callable);

        REQUIRE(matcher.impl(5) == 25);
        REQUIRE(matcher.impl.is_copied);
        REQUIRE(!matcher.impl.is_moved);
    }

    SECTION("With callable object moved")
    {
        auto matcher = exl::when_exact(CallableMock());

        REQUIRE(matcher.impl(5) == 25);
        REQUIRE(!matcher.impl.is_copied);
        REQUIRE(matcher.impl.is_moved);
    }
}

TEST_CASE("Matcher exl::when_exact result matcher has correct matching type")
{
    auto matcher = exl::when_exact(&square);
    REQUIRE(std::is_same<decltype(matcher)::type, exl::impl::marker::matcher_when_exact>::value);
}

TEST_CASE("Matcher exl::otherwise can be called with any callable")
{
    SECTION("With lambda")
    {
        int result = 1;
        auto matcher = exl::otherwise(
                [&result](const int& arg)
                {
                    result = arg;
                    return 399;
                }
        );

        REQUIRE(matcher.impl(42) == 399);
        REQUIRE(result == 42);
    }

    SECTION("With function pointer")
    {
        auto matcher = exl::otherwise(&square);

        REQUIRE(matcher.impl(5) == 25);
    }

    SECTION("With callable object copied")
    {
        CallableMock callable;
        auto matcher = exl::otherwise(callable);

        REQUIRE(matcher.impl(5) == 25);
        REQUIRE(matcher.impl.is_copied);
        REQUIRE(!matcher.impl.is_moved);
    }

    SECTION("With callable object moved")
    {
        auto matcher = exl::otherwise(CallableMock());

        REQUIRE(matcher.impl(5) == 25);
        REQUIRE(!matcher.impl.is_copied);
        REQUIRE(matcher.impl.is_moved);
    }
}

TEST_CASE("Matcher exl::otherwise result matcher has correct matching type")
{
    auto matcher = exl::otherwise(&square);
    REQUIRE(std::is_same<decltype(matcher)::type, exl::impl::marker::matcher_otherwise>::value);
}