// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)



#pragma once

#include <exl/impl/box/default_delete.hpp>
#include <exl/details/box/deleter_function.hpp>

namespace exl
{
    /// @brief returns default deleter for type T
    template <typename T>
    struct get_default_deleter
    {
        using type = deleter_function<T, impl::default_delete_scalar<T>>;
    };

    template <typename U>
    struct get_default_deleter<U[]>
    {
        using type = deleter_function<U[], impl::default_delete_array<U>>;
    };
}