// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <algorithm>
#include <type_traits>

#include <catch2/catch.hpp>

#include <exl/impl/type_list.hpp>

using namespace exl::impl;

TEST_CASE("Type list properties test", "[type_list]")
{
    using TL = type_list<int, char, std::string>;

    SECTION("Types are correct")
    {
        REQUIRE(std::is_same<int, typename TL::head>::value);
        REQUIRE(std::is_same<char, typename TL::tail::head>::value);
        REQUIRE(std::is_same<std::string, typename TL::tail::tail::head>::value);
    }

    SECTION("Has correct size")
    {
        REQUIRE(type_list_get_size<TL>::value() == 3);
    }

    SECTION("Has correct type id's")
    {
        REQUIRE(type_list_get_type_id<TL, std::string>::value() == 0);
        REQUIRE(type_list_get_type_id<TL, char>::value() == 1);
        REQUIRE(type_list_get_type_id<TL, int>::value() == 2);
    }

    SECTION("Has correct max sizeof")
    {
        static const size_t MAX_SIZEOF =
            std::max({sizeof(int), sizeof(char), sizeof(std::string)});
        REQUIRE(type_list_get_max_sizeof<TL>::value() == MAX_SIZEOF);
    }

    SECTION("Has correct max alignof")
    {
        static const size_t MAX_ALIGNOF =
            std::max({alignof(int), alignof(char), alignof(std::string)});
        REQUIRE(type_list_get_max_alignof<TL>::value() == MAX_ALIGNOF);
    }

    SECTION("Has correct type for id's")
    {
        REQUIRE(std::is_same<typename type_list_get_type_for_id<TL, 2>::type, int>::value);
        REQUIRE(std::is_same<typename type_list_get_type_for_id<TL, 1>::type, char>::value);
        REQUIRE(std::is_same<typename type_list_get_type_for_id<TL, 0>::type, std::string>::value);
    }
}

