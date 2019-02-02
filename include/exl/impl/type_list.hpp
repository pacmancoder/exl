// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace exl { namespace impl
{
    // type_list_tag_t

    using type_list_tag_t = uint8_t;

    // type_list_null

    struct type_list_null {};

    // type_list

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

    template <typename TL>
    struct type_list_get_size;

    template <typename Head, typename ... Types>
    struct type_list_get_size<type_list<Head, Types...>>
    {
    public:
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

    // type_list_get_type_id

    template <typename TL, typename T>
    struct type_list_get_type_id;

    template <typename T, typename ... Types>
    struct type_list_get_type_id<type_list<T, Types...>, T>
    {
    public:
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

    // type_list_get_max_sizeof

    template <typename TL>
    struct type_list_get_max_sizeof;

    template <typename Head, typename ... Types>
    struct type_list_get_max_sizeof<type_list<Head, Types...>>
    {
    public:
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

    // type_list_get_max_alignof

    template <typename TL>
    struct type_list_get_max_alignof;

    template <typename Head, typename ... Types>
    struct type_list_get_max_alignof<type_list<Head, Types...>>
    {
    public:
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

    // type_list_get_type_for_id

    template <typename TL, type_list_tag_t id>
    struct type_list_get_type_for_id;


    template <type_list_tag_t id, typename Head, typename ... Types>
    struct type_list_get_type_for_id<type_list<Head, Types...>, id>
    {
    public:
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

    template <typename TL, typename T>
    struct type_list_has_type;

    template <typename T, typename Head, typename ... Types>
    struct type_list_has_type<type_list<Head, Types...>, T>
    {
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

    template <typename Subset, typename Superset>
    struct type_list_is_subset_of;

    template <typename TL, typename Head, typename ... Types>
    struct type_list_is_subset_of<type_list<Head, Types...>, TL>
    {
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
}}
