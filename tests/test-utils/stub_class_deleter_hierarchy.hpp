// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <stub_class_hierarchy.hpp>

namespace exl { namespace test
{
    struct StubBaseClassDeleter
    {
        virtual void operator()(StubBaseClass* p)
        {
            p->base_tag = 1;
        }
    };

    struct StubDerivedClassDeleter : public StubBaseClassDeleter
    {
        void operator()(StubBaseClass* p) override
        {
            p->base_tag = 2;
        }
    };
}}