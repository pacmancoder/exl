// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace exl
{
    struct in_place_t
    {
        explicit in_place_t() = default;
    };

    template <typename T>
    struct in_place_type_t
    {
        explicit in_place_type_t() = default;
    };

    namespace impl
    {
        template <typename T>
        struct is_in_place_type_t
        {
            static constexpr bool value() { return false; }
        };

        template <typename U>
        struct is_in_place_type_t<in_place_type_t<U>>
        {
            static constexpr bool value() { return true; }
        };
    }

    constexpr in_place_t in_place;

#if __cpp_variable_templates >= 201304

    template <typename T>
    constexpr in_place_type_t<T> in_place_type;

#endif
}