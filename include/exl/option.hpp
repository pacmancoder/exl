// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <exl/mixed.hpp>
#include <exl/none.hpp>

namespace exl
{
    /// @brief class for representation of optional values.
    ///
    /// Based on exl::mixed type with addition of useful methods for mixed type with single
    /// optional variant
    template <typename T>
    class option : public mixed<T, exl::none>
    {
    public:
        using base_mixed_t = mixed<T, exl::none>;

    public:
        /// @brief Forwards construction to exl::mixed. see exl::mixed::mixed
        template <typename ... Args>
        option(Args&& ... args)
                : base_mixed_t(std::forward<Args>(args)...) {}

        /// @brief Verbose alias for in-place construction
        template <typename U, typename ... Args>
        static option<T> make(Args&& ... args)
        {
            return option<T>(in_place_type_t<U>(), std::forward<Args>(args)...);
        }

        /// @brief Returns option with ex::none
        static option<T> make_none() noexcept
        {
            return make<exl::none>();
        }

        /// @brief Returns option with T
        template <typename ... Args>
        static option<T> make_some(Args&& ... args)
        {
            return make<T>(std::forward<Args>(args)...);
        }

        /// @brief Forwards assignment to exl::mixed. see exl::mixed::operator=
        template <typename U>
        option<T>& operator=(const U& rhs)
        {
            return reinterpret_cast<option<T>&>(base_mixed_t::operator=(rhs));
        }

        /// @brief Forwards assignment to exl::mixed. see exl::mixed::operator=
        template <typename U>
        option<T>& operator=(U&& rhs) noexcept
        {
            return reinterpret_cast<option<T>&>(base_mixed_t::operator=(std::forward<U>(rhs)));
        }

        /// @brief Returns true if object is exl::none
        bool is_none() const noexcept
        {
            return base_mixed_t::template is<exl::none>();
        }

        /// @brief Returns true of object is not exl::none
        bool is_some() const noexcept
        {
            return !is_none();
        }

        /// @brief Returns reference to contained value. Calls std::terminate if
        /// object has exl::none
        T& unwrap_some() noexcept
        {
            return base_mixed_t::template unwrap<T>();
        }

        /// @brief Returns const reference to contained value. Calls std::terminate if
        /// object has exl::none
        const T& unwrap_some() const noexcept
        {
            return base_mixed_t::template unwrap<T>();
        }
    };
}