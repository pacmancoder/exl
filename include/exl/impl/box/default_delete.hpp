// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace exl { namespace impl
{
    template <typename T>
    void default_delete_scalar(T* p) noexcept { delete p; }

    template <typename T>
    void default_delete_array(T* p) noexcept { delete[] p; }
}}