// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
#pragma once

namespace exl { namespace test
{
    struct StubBaseClass
    {
        virtual ~StubBaseClass() = default;
        int base_tag = 0;
    };

    struct StubDerivedClass : public StubBaseClass {};
}}