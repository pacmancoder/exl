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
                std::max({ sizeof(int), sizeof(char), sizeof(std::string) });
        REQUIRE(type_list_get_max_sizeof<TL>::value() == MAX_SIZEOF);
    }

    SECTION("Has correct max alignof")
    {
        static const size_t MAX_ALIGNOF =
                std::max({ alignof(int), alignof(char), alignof(std::string) });
        REQUIRE(type_list_get_max_alignof<TL>::value() == MAX_ALIGNOF);
    }

    SECTION("Has correct type for id's")
    {
        REQUIRE(std::is_same<typename type_list_get_type_for_id<TL, 2>::type, int>::value);
        REQUIRE(std::is_same<typename type_list_get_type_for_id<TL, 1>::type, char>::value);
        REQUIRE(std::is_same<typename type_list_get_type_for_id<TL, 0>::type, std::string>::value);
    }
}

TEST_CASE("Type list has type test", "[type_list]")
{
    using TL = type_list<int, char, std::string>;

    SECTION("Has declared types")
    {
        REQUIRE(type_list_has_type<TL, int>::value());
        REQUIRE(type_list_has_type<TL, char>::value());
        REQUIRE(type_list_has_type<TL, std::string>::value());
    }

    SECTION("Undeclared types absent")
    {
        REQUIRE(!type_list_has_type<TL, std::wstring>::value());
        REQUIRE(!type_list_has_type<TL, type_list<int>>::value());
    }
}

TEST_CASE("Type list is subset test", "[type_list]")
{
    using TL = type_list<int, char, std::string>;

    SECTION("Empty type list is subset of any type list")
    {
        REQUIRE(type_list_is_subset_of<type_list<>, TL>::value());
        REQUIRE(type_list_is_subset_of<type_list<>, type_list<int>>::value());
    }

    SECTION("Check is subset of same type list")
    {
        REQUIRE(type_list_is_subset_of<TL, TL>::value());
    }

    SECTION("Check is subset of other type list with full intersection")
    {
        using SupersetTL = type_list<std::string, std::wstring, size_t, char, int>;
        REQUIRE(type_list_is_subset_of<TL, SupersetTL>::value());
    }

    SECTION("Check is not subset f other type list without intersections")
    {
        using SupersetTL = type_list<std::wstring, type_list<int>>;
        REQUIRE(!type_list_is_subset_of<TL, SupersetTL>::value());
    }

    SECTION("Check in not subset of type list with partial intersection")
    {
        using SupersetTL = type_list<int, std::wstring, std::string>;
        REQUIRE(!type_list_is_subset_of<TL, SupersetTL>::value());
    }

    SECTION("Check is not subset of empty type list")
    {
        using SupersetTL = type_list<>;
        REQUIRE(!type_list_is_subset_of<TL, SupersetTL>::value());
    }
}

TEST_CASE("Type list subset id mapping test")
{
    using TL = type_list<std::string, std::wstring, size_t, char, int>;
    using SubsetTL = type_list<int, char, std::string>;

    using Mapper = type_list_subset_id_mapping<TL, SubsetTL>;

    SECTION("First mapping test")
    {
        constexpr auto fromSubsetID = type_list_get_type_id<SubsetTL, std::string>::value();
        REQUIRE(Mapper::get(fromSubsetID) == type_list_get_type_id<TL, std::string>::value());
    }

    SECTION("Second mapping test")
    {
        constexpr auto fromSubsetID = type_list_get_type_id<SubsetTL, char>::value();
        REQUIRE(Mapper::get(fromSubsetID) == type_list_get_type_id<TL, char>::value());
    }

    SECTION("Third mapping test")
    {
        constexpr auto fromSubsetID = type_list_get_type_id<SubsetTL, int>::value();
        REQUIRE(Mapper::get(fromSubsetID) == type_list_get_type_id<TL, int>::value());
    }
}