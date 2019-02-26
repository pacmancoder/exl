// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace exl
{
    /// @brief In place construction marker type. Usually used in conjunction with constructors.
    struct in_place_t
    {
        explicit in_place_t() = default;
    };

    /// @brief In place construction marker type. Usually used in conjunction with constructors.
    template <typename T>
    struct in_place_type_t
    {
        explicit in_place_type_t() = default;
    };

    namespace impl
    {
        /// @brief Checks if provided type T is in_place_type_t
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

    /// @brief In place construction marker. Usually used in conjunction with constructors
    constexpr in_place_t in_place = in_place_t();

#if __cpp_variable_templates >= 201304

    /// @brief In place construction marker. Usually used in conjunction with constructors
    template <typename T>
    constexpr in_place_type_t<T> in_place_type = in_place_type_t<T>();

#endif
}