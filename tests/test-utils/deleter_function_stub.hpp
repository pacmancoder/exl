// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <stub_class_hierarchy.hpp>

namespace exl { namespace test
{
    inline void scalar_deleter_stub(int* p)
    {
        *p = 42;
    }

    inline void array_deleter_stub(int* values)
    {
        values[0] = 1;
        values[1] = 2;
        values[2] = 3;
    }

    inline void static_base_class_deleter(StubBaseClass* p)
    {
        p->base_tag = 1;
    }

    inline void static_derived_class_deleter(StubDerivedClass* p)
    {
        p->base_tag = 2;
    }
}}