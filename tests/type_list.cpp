// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <string>
#include <type_traits>

#include <exl/poly.hpp>

TEST_CASE("Type list has correct types", "[type_list]")
{
    using TL = exl::impl::type_list<int, char, std::string>;

    REQUIRE(std::is_same<int, typename TL::head>::value);
    REQUIRE(std::is_same<char, typename TL::tail::head>::value);
    REQUIRE(std::is_same<std::string, typename TL::tail::tail::head>::value);
}

TEST_CASE("Type list has correct size", "[type_list]")
{
    using TL = exl::impl::type_list<int, char, size_t>;

    REQUIRE(uint8_t(exl::impl::type_list_get_size<TL>::value) == 3);
    REQUIRE(uint8_t(exl::impl::type_list_get_size<exl::impl::type_list_null>::value) == 0);
}

TEST_CASE("Type list has correct type ids", "[type_list]")
{
    using TL = exl::impl::type_list<int, char, std::string>;

    REQUIRE(uint8_t(exl::impl::type_list_get_type_id<TL, std::string>::value) == 0);
    REQUIRE(uint8_t(exl::impl::type_list_get_type_id<TL, char>::value) == 1);
    REQUIRE(uint8_t(exl::impl::type_list_get_type_id<TL, int>::value) == 2);
}
