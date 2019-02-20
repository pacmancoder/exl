// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>
#include <utility>

namespace exl { namespace impl
{
    /// @brief Represents id of type in the type list
    using type_list_tag_t = uint8_t;

    /// @brief Represents nullptr-like substitution for template parameters
    struct type_list_null {};


    /// @brief Type which can be used as compile-time container for types.
    /// @tparam Types list of types which will be represented as single type list type
    template <typename ... Types>
    struct type_list;

    template <typename Head, typename ... Types>
    struct type_list<Head, Types...>
    {
    public:
        using head = Head;
        using tail = type_list<Types...>;
    };

    template <typename Head>
    struct type_list<Head>
    {
    public:
        using head = Head;
        using tail = type_list_null;
    };

    /// @brief Helper type to calculate size of type list (types count)
    /// @TL Type list which be used for calculations
    template <typename TL>
    struct type_list_get_size;

    template <typename Head, typename ... Types>
    struct type_list_get_size<type_list<Head, Types...>>
    {
    public:
        /// @brief Returns type list size
        static constexpr type_list_tag_t value()
        {
            return type_list_get_size<typename type_list<Head, Types...>::tail>::value()
                    + type_list_tag_t(1);
        }
    };

    template <>
    struct type_list_get_size<type_list_null>
    {
    public:
        static constexpr type_list_tag_t value() { return 0; }
    };


    /// @brief Helper type to obtain unique ID for specified Type in type list
    /// @tparam TL Type list to perform check on
    /// @tparam T Type for which search will be performed
    template <typename TL, typename T>
    struct type_list_get_type_id;

    template <typename T, typename ... Types>
    struct type_list_get_type_id<type_list<T, Types...>, T>
    {
    public:
        /// @brief Returns id of Specified type
        static constexpr type_list_tag_t value()
        {
            return type_list_get_size<type_list<T, Types...>>::value() - type_list_tag_t(1);
        }
    };

    template <typename T, typename Head, typename ... Types>
    struct type_list_get_type_id<type_list<Head, Types...>, T>
    {
    public:
        static constexpr type_list_tag_t value()
        {
            return type_list_get_type_id<type_list<Types...>, T>::value();
        }
    };

    template <typename T>
    struct type_list_get_type_id<type_list<T>, T>
    {
    public:
        static constexpr type_list_tag_t value() { return 0; }
    };

    /// @brief Represents set of type id's for type_list
    template <type_list_tag_t ... Values>
    struct type_list_id_set;

    template <type_list_tag_t Value>
    struct type_list_id_set<Value>
    {
        static constexpr type_list_tag_t value() { return Value; };
        using tail = type_list_null;
    };

    template <type_list_tag_t Head, type_list_tag_t ... Values>
    struct type_list_id_set<Head, Values...>
    {
        static type_list_tag_t value() { return Head; };
        using tail = type_list_id_set<Values...>;
    };

    template <typename Set>
    struct type_list_id_set_contains;

    template <type_list_tag_t Head, type_list_tag_t ... Types>
    struct type_list_id_set_contains<type_list_id_set<Head, Types...>>
    {
        static bool check(type_list_tag_t id)
        {
            if (id == type_list_id_set<Head, Types...>::value())
            {
                return true;
            }
            else
            {
                return type_list_id_set_contains<type_list_id_set<Types...>>::check(id);
            }
        }
    };

    template <>
    struct type_list_id_set_contains<type_list_id_set<>>
    {
        static bool check(type_list_tag_t)
        {
            return false;
        }
    };

    /// @brief Adds new type before type list's head
    template <typename Set, type_list_tag_t Value>
    struct type_list_id_set_push_front;

    template <type_list_tag_t NewValue, type_list_tag_t ... Values>
    struct type_list_id_set_push_front<type_list_id_set<Values...>, NewValue>
    {
        using type = type_list_id_set<NewValue, Values...>;
    };

    /// @brief Adds new type before type list's tail
    template <typename Set, type_list_tag_t Value>
    struct type_list_id_set_push_back;

    template <type_list_tag_t NewValue, type_list_tag_t ... Values>
    struct type_list_id_set_push_back<type_list_id_set<Values...>, NewValue>
    {
        using type = type_list_id_set<Values..., NewValue>;
    };


    /// @brief Concatenates two type id lists
    template <typename Lhs, typename Rhs>
    struct type_list_id_set_concat;

    template <typename Lhs, type_list_tag_t Head, type_list_tag_t ... Tail>
    struct type_list_id_set_concat<Lhs, type_list_id_set<Head, Tail...>>
    {
        using type = typename type_list_id_set_concat<
                typename type_list_id_set_push_back<Lhs, Head>::type,
                type_list_id_set<Tail...>
        >::type;
    };

    template <typename Lhs>
    struct type_list_id_set_concat<Lhs, type_list_id_set<>>
    {
        using type = Lhs;
    };

    /// @brief Returns set of type ID's for derived types
    template <typename TL, typename T>
    struct type_list_get_ids_of_derived_types;

    template <typename T, typename Head, typename ... Tail>
    struct type_list_get_ids_of_derived_types<type_list<Head, Tail...>, T>
    {
        using type = typename std::conditional<
                std::is_base_of<T, Head>::value,
                // push new type to the list if type is same of derived
                typename type_list_id_set_push_front<
                        typename type_list_get_ids_of_derived_types<
                                type_list<Tail...>, T
                        >::type,
                        type_list_get_type_id<type_list<Head, Tail...>, Head>::value()
                >::type,
                // skip current type otherwise
                typename type_list_get_ids_of_derived_types<type_list<Tail...>, T>::type
        >::type;
    };

    template <typename T, typename Head>
    struct type_list_get_ids_of_derived_types<type_list<Head>, T>
    {
        using type = typename std::conditional<
                std::is_base_of<T, Head>::value,
                // add last type if same of derived
                type_list_id_set<type_list_get_type_id<type_list<Head>, Head>::value()>,
                // return empty list otherwise
                type_list_id_set<>
        >::type;
    };

    /// @brief Returns set of type ID's for same types
    template <typename TL, typename T>
    struct type_list_get_ids_of_same_types;

    template <typename T, typename Head, typename ... Tail>
    struct type_list_get_ids_of_same_types<type_list<Head, Tail...>, T>
    {
        using type = typename std::conditional<
                std::is_same<T, Head>::value,
                // push new type to the list if type is same of derived
                typename type_list_id_set_push_front<
                        typename type_list_get_ids_of_same_types<
                                type_list<Tail...>, T
                        >::type,
                        type_list_get_type_id<type_list<Head, Tail...>, Head>::value()
                >::type,
                // skip current type otherwise
                typename type_list_get_ids_of_same_types<type_list<Tail...>, T>::type
        >::type;
    };

    template <typename T, typename Head>
    struct type_list_get_ids_of_same_types<type_list<Head>, T>
    {
        using type = typename std::conditional<
                std::is_same<T, Head>::value,
                // add last type if same of derived
                type_list_id_set<type_list_get_type_id<type_list<Head>, Head>::value()>,
                // return empty list otherwise
                type_list_id_set<>
        >::type;
    };

    /// @brief Helper type to calculate storage type size which suitable for any type in type list
    /// @tparam TL Type list used for calculation of size
    template <typename TL>
    struct type_list_get_max_sizeof;

    template <typename Head, typename ... Types>
    struct type_list_get_max_sizeof<type_list<Head, Types...>>
    {
    public:
        /// @brief Returns storage type size which suitable for any type in type list
        static constexpr size_t value()
        {
            return (HEAD_SIZEOF > TAIL_MAX_SIZEOF) ? HEAD_SIZEOF : TAIL_MAX_SIZEOF;
        }

    private:
        static constexpr size_t HEAD_SIZEOF = sizeof(Head);
        static constexpr size_t TAIL_MAX_SIZEOF =
                type_list_get_max_sizeof<type_list<Types...>>::value();
    };

    template <>
    struct type_list_get_max_sizeof<type_list<>>
    {
    public:
        static constexpr size_t value() { return 0; }
    };

    /// @brief Helper type to calculate storage type align which suitable for any type in type list
    /// @tparam TL Type list used for calculation of alignment
    template <typename TL>
    struct type_list_get_max_alignof;

    template <typename Head, typename ... Types>
    struct type_list_get_max_alignof<type_list<Head, Types...>>
    {
    public:
        /// @brief Returns storage type align which suitable for any type in type list
        static constexpr size_t value()
        {
            return (HEAD_ALIGNOF > TAIL_MAX_ALIGNOF) ? HEAD_ALIGNOF : TAIL_MAX_ALIGNOF;
        }

    private:
        static constexpr size_t HEAD_ALIGNOF = alignof(Head);
        static constexpr size_t TAIL_MAX_ALIGNOF =
                type_list_get_max_alignof<type_list<Types...>>::value();
    };

    template <>
    struct type_list_get_max_alignof<type_list<>>
    {
    public:
        static constexpr size_t value() { return 0; }
    };

    /// @brief Helper class to obtain Type in type list by its ID
    /// @tparam TL Type list to perform search on
    /// @tparam id Id of type to search
    /// @note Will return type_list_null if type not found
    template <typename TL, type_list_tag_t id>
    struct type_list_get_type_for_id;


    template <type_list_tag_t id, typename Head, typename ... Types>
    struct type_list_get_type_for_id<type_list<Head, Types...>, id>
    {
    public:
        /// @brief Found type for specified ID
        using type = typename std::conditional<
                type_list_get_type_id<type_list<Head, Types...>, Head>::value() == id,
                Head,
                typename type_list_get_type_for_id<type_list<Types...>, id>::type
        >::type;
    };

    template <type_list_tag_t id>
    struct type_list_get_type_for_id<type_list<>, id>
    {
    public:
        using type = type_list_null;
    };

    /// @brief Helper type to check that type list contains specified type
    /// @tparam TL Type list to check on
    /// @tparam T Type to check for
    template <typename TL, typename T>
    struct type_list_has_type;

    template <typename T, typename Head, typename ... Types>
    struct type_list_has_type<type_list<Head, Types...>, T>
    {
        /// @brief Returns true if type list has specified type
        static constexpr bool value()
        {
            return type_list_has_type<type_list<Types...>, T>::value();
        }
    };

    template <typename Head, typename ... Types>
    struct type_list_has_type<type_list<Head, Types...>, Head>
    {
        static constexpr bool value() { return true; }
    };

    template <typename T>
    struct type_list_has_type<type_list<>, T>
    {
        static constexpr bool value() { return false; }
    };

    /// @brief Helper type to check if specified type list is subset of another type list
    /// @tparam TL Superset type list to perform check on
    /// @tparam SubsetTL Subset type list to perform check for
    /// @note: Empty type list is subset of any type list
    template <typename Subset, typename Superset>
    struct type_list_is_subset_of;

    template <typename TL, typename Head, typename ... Types>
    struct type_list_is_subset_of<type_list<Head, Types...>, TL>
    {
        /// @brief Returns true if specified type list is subset of specified superset
        static constexpr bool value()
        {
            return type_list_has_type<TL, Head>::value()
                    && type_list_is_subset_of<type_list<Types...>, TL>::value();
        }
    };

    template <typename TL>
    struct type_list_is_subset_of<type_list<>, TL>
    {
        static constexpr bool value()
        {
            return true;
        }
    };

    /// @brief Helper type to get type id of subset's type list in superset type list
    /// @tparam TL Superset type list to perform mapping on
    /// @tparam SubsetTL Subset type list to perform mapping for
    /// @note WARNING: Mapping of type index, which is not bound to TL subset type will cause
    /// undefined behavior.
    template <
            typename TL,
            typename SubsetTL,
            type_list_tag_t ExpectedTag = type_list_get_type_id<
                    SubsetTL,
                    typename SubsetTL::head
            >::value()
    >
    struct type_list_subset_id_mapping
    {
    public:
        static constexpr type_list_tag_t mapped = type_list_get_type_id<
                TL,
                typename type_list_get_type_for_id<SubsetTL, ExpectedTag>::type
        >::value();

    public:
        /// @brief Returns ID which mapped to subset's type in superset type list
        static type_list_tag_t get(type_list_tag_t targetID)
        {
            return (targetID == ExpectedTag)
                    ? mapped
                    : type_list_subset_id_mapping<TL, SubsetTL, ExpectedTag - 1>::get(targetID);
        }
    };

    template <typename TL, typename SubsetTL>
    struct type_list_subset_id_mapping<TL, SubsetTL, 0>
    {
    public:
        static type_list_tag_t get(type_list_tag_t)
        {
            return type_list_get_type_id<
                    TL,
                    typename type_list_get_type_for_id<SubsetTL, 0>::type
            >::value();
        }
    };

    /// @brief Adds new type before type list's head
    template <typename TL, typename T>
    struct type_list_push_front;

    template <typename T, typename ... Types>
    struct type_list_push_front<type_list<Types...>, T>
    {
        using type = type_list<T, Types...>;
    };

    template <typename ... Types>
    struct type_list_push_front<type_list<Types...>, type_list_null>
    {
        using type = type_list<Types...>;
    };

    // @brief removes all types from the type list which are the same type as specified
    template <typename TL, typename T>
    struct type_list_remove_same;

    template <typename T, typename Head, typename ... Types>
    struct type_list_remove_same<type_list<Head, Types...>, T>
    {
        using type = typename std::conditional<
                std::is_same<Head, T>::value,
                typename type_list_remove_same<type_list<Types...>, T>::type,
                typename type_list_push_front<
                        typename type_list_remove_same<type_list<Types...>, T>::type,
                        Head
                >::type
        >::type;
    };

    template <typename T, typename Head>
    struct type_list_remove_same<type_list<Head>, T>
    {
        using type = typename std::conditional<
                std::is_same<Head, T>::value,
                type_list<>,
                type_list<Head>
        >::type;
    };

    // @brief removes all types from the type list which are the same type as specified
    template <typename TL, typename T>
    struct type_list_remove_derived;

    template <typename T, typename Head, typename ... Types>
    struct type_list_remove_derived<type_list<Head, Types...>, T>
    {
        using type = typename std::conditional<
                std::is_base_of<T, Head>::value,
                typename type_list_remove_derived<type_list<Types...>, T>::type,
                typename type_list_push_front<
                        typename type_list_remove_derived<type_list<Types...>, T>::type,
                        Head
                >::type
        >::type;
    };

    template <typename T, typename Head>
    struct type_list_remove_derived<type_list<Head>, T>
    {
        using type = typename std::conditional<
                std::is_base_of<T, Head>::value,
                type_list<>,
                type_list<Head>
        >::type;
    };

    /// @brief Represents list of type IDs. Mostly used in internal methods
    template <type_list_tag_t ...>
    struct type_list_id_sequence {};

    /// @brief Produced a type_list_id_sequence instantiation for specified type list
    /// @tparam TL type list to produce sequence for
    template <typename TL>
    struct type_list_id_sequence_for;

    template <typename ... Types>
    struct type_list_id_sequence_for<type_list<Types...>>
    {
    private:
        using TL = type_list<Types...>;

    public:
        using type = type_list_id_sequence<type_list_get_type_id<TL, Types>::value()...>;
    };

    // NOTE: Design of best-match overload implementation was "greatly inspired" by code from
    // mpark-variant project (https://github.com/mpark/variant)

    template <type_list_tag_t ID, typename T>
    struct type_list_best_match_overload_leaf
    {
        using Func = std::integral_constant<type_list_tag_t, ID> (*)(T);

        constexpr operator Func() const { return nullptr; }
    };

    template <typename TL>
    struct type_list_best_match_overload_impl;

    template <typename ... Types>
    struct type_list_best_match_overload_impl<type_list<Types...>>
    {
    private:
        using TL = type_list<Types...>;

        template <typename Sequence>
        struct Impl;

        template <type_list_tag_t ... IDs>
        struct Impl<type_list_id_sequence<IDs...>>
                : type_list_best_match_overload_leaf<IDs, Types> ...
        {
        };

    public:
        using type = Impl<typename type_list_id_sequence_for<TL>::type>;
    };

    template <typename TL>
    using type_list_best_match_overload = typename type_list_best_match_overload_impl<TL>::type;

    template <typename TL, typename T>
    struct type_list_get_best_match_id
    {
        static type_list_tag_t constexpr value()
        {
            return std::result_of<type_list_best_match_overload<TL>(T&&)>::type::value;
        };
    };

    template <typename TL, typename T>
    struct type_list_get_best_match
    {
        using type = typename type_list_get_type_for_id<
                TL,
                type_list_get_best_match_id<TL, T>::value()
        >::type;
    };

}}
