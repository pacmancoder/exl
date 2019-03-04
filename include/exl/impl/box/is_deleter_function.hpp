// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace exl { namespace impl
{
    template <typename Deleter>
    struct is_deleter_function
    {
        static constexpr bool value()
        {
            return false;
        }
    };

    template <typename T, typename get_deleter_function_type<T>::type DeleterFunc>
    struct is_deleter_function<deleter_function<T, DeleterFunc>>
    {
        static constexpr bool value()
        {
            return true;
        }
    };
}}