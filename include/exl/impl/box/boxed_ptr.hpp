// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
#pragma once

#include <type_traits>
#include <utility>

#include <exl/details/box/get_default_deleter.hpp>

namespace exl { namespace impl
{
    template <typename T, typename Deleter = typename get_default_deleter<T>::type>
    class boxed_ptr : public Deleter
    {
    public:
        template <typename FT, typename FDeleter>
        friend
        class boxed_ptr;

        using ptr_t = typename Deleter::ptr_t;

    public:
        template <
                typename = typename std::enable_if<
                        std::is_default_constructible<Deleter>::value
                >::type
        >
        explicit boxed_ptr(ptr_t ptr) noexcept
                : Deleter()
                , ptr_(ptr) {}

        template <typename U, typename RhsDeleter>
        explicit boxed_ptr(boxed_ptr<U, RhsDeleter>&& rhs) noexcept
                : Deleter(std::move(rhs))
                , ptr_(rhs.ptr_)
        {
            rhs.ptr_ = nullptr;
        }

        template <typename RhsDeleter>
        explicit boxed_ptr(ptr_t ptr, RhsDeleter&& rhs_deleter) noexcept
                : Deleter(std::forward<RhsDeleter>(rhs_deleter))
                , ptr_(ptr) {}

        template <typename U, typename RhsDeleter>
        boxed_ptr& operator=(boxed_ptr<U, RhsDeleter>&& rhs) noexcept
        {
            destroy_with_deleter();
            Deleter::operator=(std::move(rhs));

            ptr_ = rhs.ptr_;
            rhs.ptr_ = nullptr;

            return *this;
        }

        ptr_t get() const noexcept
        {
            return ptr_;
        }

        void reset(ptr_t value) noexcept
        {
            destroy_with_deleter();
            ptr_ = value;
        }

        ptr_t release() noexcept
        {
            ptr_t ptr = nullptr;
            std::swap(ptr, ptr_);

            return ptr;
        }

        void swap(boxed_ptr& rhs) noexcept
        {
            std::swap(ptr_, rhs.ptr_);
        }

        ~boxed_ptr() noexcept
        {
            destroy_with_deleter();
        }

    private:
        void destroy_with_deleter() noexcept
        {
            if (ptr_ != nullptr)
            {
                Deleter::destroy(ptr_);
                ptr_ = nullptr;
            }
        }

    private:
        ptr_t ptr_;
    };
}}