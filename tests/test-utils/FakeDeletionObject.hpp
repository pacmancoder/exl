// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
#pragma once

namespace exl { namespace test
{
    class FakeDeletionObject
    {
    public:
        static void operator delete[](void* obj)
        {
            reinterpret_cast<FakeDeletionObject*>(obj)[0].tag = 42;
            reinterpret_cast<FakeDeletionObject*>(obj)[1].tag = 43;
            reinterpret_cast<FakeDeletionObject*>(obj)[2].tag = 44;
        }

        static void operator delete(void* obj)
        {
            reinterpret_cast<FakeDeletionObject*>(obj)->tag = 42;
        }

        int tag = 0;
    };
}}