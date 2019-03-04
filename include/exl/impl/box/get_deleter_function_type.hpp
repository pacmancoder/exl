// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace exl { namespace impl
{
    template <typename T>
    struct get_deleter_function_type
    {
        using type = void (&)(T*);
    };

    template <typename T>
    struct get_deleter_function_type<T[]>
    {
        using type = void (&)(T*);
    };
}}