// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
#pragma once

namespace exl { namespace test
{
    class StubDeleter
    {
    public:
        StubDeleter() noexcept
                : value_(nullptr) {}

        StubDeleter(StubDeleter&& rhs) noexcept
                : value_(rhs.value_)
                , is_move_constructed_(true)
        {
            rhs.is_moved_ = true;
        }

        explicit StubDeleter(const int* value)
                : value_(value) {}

        StubDeleter& operator=(StubDeleter&& rhs) noexcept
        {
            value_ = rhs.value_;
            rhs.is_moved_ = true;
            is_move_assigned_ = true;
            return *this;
        }

        void operator()(int* obj) noexcept
        {
            if (!value_)
            {
                *obj = 399;
                return;
            }
            *obj = *value_;
        }

        bool is_moved() const { return is_moved_; }
        bool is_move_constructed() const { return is_move_constructed_; }
        bool is_move_assigned() const { return is_move_assigned_; }

    private:
        const int* value_;
        bool is_moved_ = false;
        bool is_move_constructed_ = false;
        bool is_move_assigned_ = false;
    };
}}