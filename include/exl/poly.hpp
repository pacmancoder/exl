// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "none.hpp"

namespace exl
{
    namespace impl
    {
        struct type_list_null {};

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
            static constexpr uint8_t value =
                type_list_get_size<typename type_list<Head, Types...>::tail>::value + 1;
        };

        template <>
        struct type_list_get_size<type_list_null>
        {
        public:
            static constexpr uint8_t value = 0;
        };

        template <typename TL, typename T>
        struct type_list_get_type_id;

        template <typename T, typename ... Types>
        struct type_list_get_type_id<type_list<T, Types...>, T>
        {
        public:
            static constexpr uint8_t value = type_list_get_size<type_list<T, Types...>>::value - 1;
        };

        template <typename T, typename Head, typename ... Types>
        struct type_list_get_type_id<type_list<Head, Types...>, T>
        {
        public:
            static constexpr uint8_t value = type_list_get_type_id<type_list<Types...>, T>::value;
        };

        template <typename T>
        struct type_list_get_type_id<type_list<T>, T>
        {
        public:
            static constexpr uint8_t value = 0;
        };

        template <typename T>
        struct type_list_get_type_id<type_list<>, T>
        {
        public:
            static_assert(false, "Specified type not found in the type list");
            static constexpr uint8_t value = 0;
        };
    }
}
