// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>

#include <exl/impl/box/get_deleter_function_type.hpp>

namespace exl
{
    /// @brief Provides type to define exl::box deleter with custom deleter function
    /// @tparam T type to delete
    /// @tparam DeleterFunc Custom deleter function reference
    template <
            typename T,
            typename impl::get_deleter_function_type<T>::type DeleterFunc
    >
    class deleter_function
    {
    public:
        using ptr_t = typename std::conditional<
                std::is_array<T>::value,
                typename std::decay<T>::type,
                typename std::add_pointer<typename std::decay<T>::type>::type
        >::type;

    public:
        deleter_function() = default;

        template <
                typename U,
                typename impl::get_deleter_function_type<U>::type UDeleterFunc,
                typename = typename std::enable_if<
                        std::is_convertible<
                                typename std::conditional<
                                        std::is_array<U>::value,
                                        typename std::decay<U>::type,
                                        typename std::add_pointer<U>::type
                                >::type,
                                ptr_t
                        >::value
                >::type
        >
        deleter_function(deleter_function<U, UDeleterFunc>&&) {}

        template <
                typename U,
                typename impl::get_deleter_function_type<U>::type UDeleterFunc,
                typename = typename std::enable_if<
                        std::is_convertible<
                                typename std::conditional<
                                        std::is_array<U>::value,
                                        typename std::decay<U>::type,
                                        typename std::add_pointer<U>::type
                                >::type,
                                ptr_t
                        >::value
                >::type
        >
        deleter_function& operator=(deleter_function<U, UDeleterFunc>&&) noexcept
        {
            return *this;
        }

        static void destroy(ptr_t obj)
        {
            DeleterFunc(obj);
        }
    };
}

#include <exl/impl/box/is_deleter_function.hpp>