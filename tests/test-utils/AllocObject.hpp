// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace exl { namespace test
{
    class AlwaysGoodAllocObject
    {
    public:
        static void* operator new(size_t, std::nothrow_t) noexcept
        {
            return reinterpret_cast<void*>(1);
        }

        static void* operator new(size_t)
        {
            return new(std::nothrow) AlwaysGoodAllocObject();
        }

        static void operator delete(void*, std::nothrow_t) noexcept {}

        static void operator delete(void*) {}
    };

    class AlwaysBadAllocObject
    {
    public:
        static void* operator new(size_t, std::nothrow_t) noexcept
        {
            return reinterpret_cast<void*>(0);
        }

        static void* operator new(size_t)
        {
            return new(std::nothrow) AlwaysBadAllocObject();
        }

        static void operator delete(void*, std::nothrow_t) noexcept {}

        static void operator delete(void*) {}
    };
}}