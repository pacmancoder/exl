// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
#include <exl/mixed.hpp>

#include <catch2/catch.hpp>

#include <ClassMock.hpp>

#include <string>

using namespace exl::impl;
using namespace exl::test;

TEST_CASE("Constructor forwards its arguments to the underlying exl::mixed", "[nested_mixed]")
{
    CallCounter counter;
    auto nested = exl::nested_mixed<ClassMock, size_t>::make<ClassMock>(Tag(42), &counter);

    REQUIRE(nested->is<ClassMock>());
    REQUIRE((*nested).unwrap<ClassMock>().tag() == 42);

    REQUIRE(counter.count(CallType::Construct, 42) == 1);
    REQUIRE(counter.count(CallType::Assign, 42) == 0);
    REQUIRE(counter.count(CallType::Copy, 42) == 0);
    REQUIRE(counter.count(CallType::Move, 42) == 0);
}

TEST_CASE("Constant nested mixed is guaranteed", "[nested_mixed]")
{
    const auto nested = exl::nested_mixed<ClassMock, size_t>::make<size_t>(42);

    REQUIRE((*nested).is<size_t>());
    REQUIRE(nested->unwrap<size_t>() == 42);

    // If the following compiles than implementation is broken
    // nested->unwrap<size_t>() = 42;
    // (*nested).unwrap<size_t>() = 42;
}

TEST_CASE("exl::mixed can contain nested type")
{
    using Nested = exl::nested_mixed<size_t, std::string>;
    using Mixed = exl::mixed<size_t, Nested>;

    auto mixed = Mixed::make<Nested>(42);

    REQUIRE(mixed.is<Nested>());
    REQUIRE(mixed.unwrap<Nested>()->is<size_t>());

    mixed = Nested::make<std::string>("hello");

    REQUIRE(mixed.is<Nested>());
    REQUIRE(mixed.unwrap<Nested>()->is<std::string>());
    REQUIRE(mixed.unwrap<Nested>()->unwrap<std::string>() == "hello");
}