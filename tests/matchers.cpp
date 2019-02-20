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

        CallableMock(CallableMock&& rhs) noexcept
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

TEST_CASE("Matcher exl::when can be called with any callable", "[matchers]")
{
    SECTION("With lambda")
    {
        int result = 1;
        auto matcher = exl::when<int>(
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
        auto matcher = exl::when<int>(&square);

        REQUIRE(matcher.impl(5) == 25);
    }

    SECTION("With callable object copied")
    {
        CallableMock callable;
        auto matcher = exl::when<int>(callable);

        REQUIRE(matcher.impl(5) == 25);
        REQUIRE(matcher.impl.is_copied);
        REQUIRE(!matcher.impl.is_moved);
    }

    SECTION("With callable object moved")
    {
        auto matcher = exl::when<int>(CallableMock());

        REQUIRE(matcher.impl(5) == 25);
        REQUIRE(!matcher.impl.is_copied);
        REQUIRE(matcher.impl.is_moved);
    }
}

TEST_CASE("Matcher exl::when result matcher has correct properties", "[matchers]")
{
    auto matcher = exl::when<int>(&square);

    SECTION("Kind is correct")
    {
        REQUIRE(std::is_same<decltype(matcher)::kind_t, exl::impl::marker::matcher_when>::value);
    }

    SECTION("Target type is correct")
    {
        REQUIRE(std::is_same<typename decltype(matcher)::target_type_t, int>::value);
    }
}

TEST_CASE("Matcher exl::when_exact can be called with any callable", "[matchers]")
{
    SECTION("With lambda")
    {
        int result = 1;
        auto matcher = exl::when_exact<int>(
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
        auto matcher = exl::when_exact<int>(&square);

        REQUIRE(matcher.impl(5) == 25);
    }

    SECTION("With callable object copied")
    {
        CallableMock callable;
        auto matcher = exl::when_exact<int>(callable);

        REQUIRE(matcher.impl(5) == 25);
        REQUIRE(matcher.impl.is_copied);
        REQUIRE(!matcher.impl.is_moved);
    }

    SECTION("With callable object moved")
    {
        auto matcher = exl::when_exact<int>(CallableMock());

        REQUIRE(matcher.impl(5) == 25);
        REQUIRE(!matcher.impl.is_copied);
        REQUIRE(matcher.impl.is_moved);
    }
}

TEST_CASE("Matcher exl::when_exact result matcher has correct properties", "[matchers]")
{
    auto matcher = exl::when_exact<int>(&square);

    SECTION("Kind is correct")
    {
        REQUIRE(std::is_same<
                decltype(matcher)::kind_t,
                exl::impl::marker::matcher_when_exact
        >::value);
    }

    SECTION("Target type is correct")
    {
        REQUIRE(std::is_same<typename decltype(matcher)::target_type_t, int>::value);
    }
}

TEST_CASE("Matcher exl::otherwise can be called with any callable", "[matchers]")
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

TEST_CASE("Matcher exl::otherwise result matcher has correct properties", "[matchers]")
{
    auto matcher = exl::otherwise(&square);

    SECTION("Kind is correct")
    {
        REQUIRE(std::is_same<
                decltype(matcher)::kind_t,
                exl::impl::marker::matcher_otherwise
        >::value);
    }

    SECTION("Target type is correct")
    {
        REQUIRE(std::is_same<typename decltype(matcher)::target_type_t, void>::value);
    }
}