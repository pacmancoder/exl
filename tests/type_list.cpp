// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <algorithm>

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

TEST_CASE("Type list push back adds type to the type list")
{
    using TL = type_list<int, char>;

    REQUIRE(std::is_same<
            typename type_list_push_back<TL, std::string>::type,
            type_list<int, char, std::string>
    >::value);
}

TEST_CASE("Type list push back adds type to the empty type list")
{
    using TL = type_list<>;

    REQUIRE(std::is_same<
            typename type_list_push_back<TL, int>::type,
            type_list<int>
    >::value);
}

TEST_CASE("Type list push back of null type adds nothing")
{
    using TL = type_list<int, char>;
    REQUIRE(std::is_same<typename type_list_push_back<TL, type_list_null>::type, TL>::value);
}

TEST_CASE("Type list push front adds type to the type list")
{
    using TL = type_list<int, char>;

    REQUIRE(std::is_same<
            typename type_list_push_front<TL, std::string>::type,
            type_list<std::string, int, char>
    >::value);
}

TEST_CASE("Type list push front adds type to the empty type list")
{
    using TL = type_list<>;

    REQUIRE(std::is_same<
            typename type_list_push_front<TL, int>::type,
            type_list<int>
    >::value);
}

TEST_CASE("Type list push front of null type adds nothing")
{
    using TL = type_list<int, char>;
    REQUIRE(std::is_same<typename type_list_push_front<TL, type_list_null>::type, TL>::value);
}


TEST_CASE("Type list type_list_remove_same_or_derived removes value of the same type")
{
    SECTION("Deleted at the beginning")
    {
        using TL = type_list<std::string, int, char>;
        REQUIRE(std::is_same<
                typename type_list_remove_same_or_derived<TL, std::string>::type,
                type_list<int, char>
        >::value);
    }

    SECTION("Deleted at the middle")
    {
        using TL = type_list<int, std::string, char>;
        REQUIRE(std::is_same<
                typename type_list_remove_same_or_derived<TL, std::string>::type,
                type_list<int, char>
        >::value);
    }

    SECTION("Deleted at the end")
    {
        using TL = type_list<int, char, std::string>;
        REQUIRE(std::is_same<
                typename type_list_remove_same_or_derived<TL, std::string>::type,
                type_list<int, char>
        >::value);
    }
}

TEST_CASE("Type list type_list_remove_same_or_derived removes value of the derived type")
{
    SECTION("Deleted at the beginning")
    {
        using TL = type_list<std::runtime_error, int, char>;
        REQUIRE(std::is_same<
                typename type_list_remove_same_or_derived<TL, std::exception>::type,
                type_list<int, char>
        >::value);
    }

    SECTION("Deleted at the middle")
    {
        using TL = type_list<int, std::logic_error, char>;
        REQUIRE(std::is_same<
                typename type_list_remove_same_or_derived<TL, std::exception>::type,
                type_list<int, char>
        >::value);
    }

    SECTION("Deleted at the end")
    {
        using TL = type_list<int, char, std::invalid_argument>;
        REQUIRE(std::is_same<
                typename type_list_remove_same_or_derived<TL, std::exception>::type,
                type_list<int, char>
        >::value);
    }
}

TEST_CASE("Type list type_list_remove_same_or_derived removes value of the same or derived type")
{
    using TL = type_list<
            std::string,
            std::exception,
            std::runtime_error,
            int,
            char,
            std::logic_error
    >;

    REQUIRE(std::is_same<
            typename type_list_remove_same_or_derived<TL, std::exception>::type,
            type_list<std::string, int, char>
    >::value);
}

TEST_CASE("Type list id set type params test")
{
    using TypeListIdSet = type_list_id_set<42, 5, 22>;
    REQUIRE(TypeListIdSet::value() == 42);
    REQUIRE(TypeListIdSet::tail::value() == 5);
    REQUIRE(TypeListIdSet::tail::tail::value() == 22);
}

TEST_CASE("Type list id set push front adds id to the type list id set")
{
    using Set1 = type_list_id_set<42, 11>;
    using Set2 = typename type_list_id_set_push_front<Set1, 2>::type;

    REQUIRE(Set2::value() == 2);
    REQUIRE(Set2::tail::value() == 42);
    REQUIRE(Set2::tail::tail::value() == 11);
}

TEST_CASE("Type list id set push front adds id to the empty type id list")
{
    using Set1 = type_list_id_set<>;
    using Set2 = typename type_list_id_set_push_front<Set1, 55>::type;

    REQUIRE(Set2::value() == 55);
}

TEST_CASE("get_ids_of_same_or_derived_types returns single element for same type")
{
    SECTION("Type at the beginning")
    {
        using TL = type_list<std::string, char, int>;
        using TypeIdList = type_list_get_ids_of_same_or_derived_types<TL, std::string>::type;
        REQUIRE(TypeIdList::value() == 2);
    }

    SECTION("Type at the middle")
    {
        using TL = type_list<char, std::string, int>;
        using TypeIdList = type_list_get_ids_of_same_or_derived_types<TL, std::string>::type;
        REQUIRE(TypeIdList::value() == 1);
    }

    SECTION("Type at the end")
    {
        using TL = type_list<char, int, std::string>;
        using TypeIdList = type_list_get_ids_of_same_or_derived_types<TL, std::string>::type;
        REQUIRE(TypeIdList::value() == 0);
    }
}

TEST_CASE("get_ids_of_same_or_derived_types returns single element for derived type")
{
    SECTION("Type at the beginning")
    {
        using TL = type_list<std::runtime_error, char, int>;
        using TypeIdList = type_list_get_ids_of_same_or_derived_types<TL, std::exception>::type;
        REQUIRE(TypeIdList::value() == 2);
    }

    SECTION("Type at the middle")
    {
        using TL = type_list<char, std::runtime_error, int>;
        using TypeIdList = type_list_get_ids_of_same_or_derived_types<TL, std::exception>::type;
        REQUIRE(TypeIdList::value() == 1);
    }

    SECTION("Type at the end")
    {
        using TL = type_list<char, int, std::runtime_error>;
        using TypeIdList = type_list_get_ids_of_same_or_derived_types<TL, std::exception>::type;
        REQUIRE(TypeIdList::value() == 0);
    }
}

TEST_CASE("get_ids_of_same_or_derived_types returns many element for derived and same types")
{
    using TL = type_list<
            std::runtime_error,
            char,
            std::exception,
            int,
            std::logic_error,
            std::invalid_argument
    >;
    using TypeIdList = type_list_get_ids_of_same_or_derived_types<TL, std::exception>::type;

    REQUIRE(TypeIdList::value() == 5);
    REQUIRE(TypeIdList::tail::value() == 3);
    REQUIRE(TypeIdList::tail::tail::value() == 1);
    REQUIRE(TypeIdList::tail::tail::tail::value() == 0);
}

// TODO: Type list id set <contains> id