// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>
#include <utility>

#include <exl/in_place.hpp>

#include <exl/details/box/deleter_function.hpp>
#include <exl/details/box/deleter_object.hpp>
#include <exl/details/box/get_default_deleter.hpp>

#include <exl/impl/box/boxed_ptr.hpp>

namespace exl
{
    namespace impl
    {
    }
    /// @brief Represents type which provides exception-less dynamic allocation mechanism
    /// result of allocation can be checked with is_valid method
    /// @note Deleter object constructors should be no-throwing
    template <
            typename T,
            typename Deleter = typename get_default_deleter<T>::type
    >
    class box
    {
    public:
        /// @brief Creates boxed type
        ///
        /// When allocation was failed, is_valid() will return false
        ///
        /// @tparam Args object constructor arguments types
        /// @param args object constructor arguments
        /// @return boxed object
        template <typename ... Args>
        static box make(Args&& ... args)
        {
            return box(in_place, std::forward<Args>(args)...);
        }

        /// @brief Constructs box from pointer with default-constructed deleter
        ///
        /// is_valid will return false if provided pointer is nullptr
        static box from_ptr(T* ptr) noexcept
        {
            return box(ptr);
        }

        /// @brief Constructs box from pointer with provided deleter
        ///
        /// is_valid will return false if provided pointer is nullptr
        template <typename RhsDeleter>
        static box from_ptr(T* ptr, RhsDeleter&& deleter) noexcept
        {
            return box(ptr, std::forward<RhsDeleter>(deleter));
        }

        /// @brief Copy construction is not permitted
        box(const box&) = delete;

        /// @brief Copy assignment is not permitted
        box& operator=(const box&) = delete;

        /// @brief Constructs box by moving rhs
        /// @param rhs Source object to move from
        box(box&& rhs) noexcept
                : ptr_(std::move(rhs.ptr_)) {}

        /// @brief Swaps two boxes
        void swap(box& rhs) noexcept
        {
            ptr_.swap(rhs.ptr_);
        }

        /// @brief Assigns new value to box, destroying previously contained value
        template <typename U, typename UDeleter>
        box& operator=(box<U, UDeleter>&& rhs)
        {
            ptr_ = std::move(rhs.ptr_);
            return *this;
        }

        /// @brief Returns reference to the contained object.
        /// Calls std::terminate if box is not valid
        T& get() noexcept
        {
            asset_valid();
            return *ptr_.get();
        }

        /// @brief Returns const reference to the contained object.
        /// Calls std::terminate if box is not valid
        const T& get() const noexcept
        {
            asset_valid();
            return *ptr_.get();
        }

        /// @brief Returns false if allocation of box has been failed
        bool is_valid() const noexcept
        {
            return ptr_.get() != nullptr;
        }

        /// @brief Destroys previously contained object and owns provided pointer
        void reset(T* rhs)
        {
            ptr_.reset(rhs);
        }

        /// @brief Releases ownership of contained pointer and returns it
        T* release() noexcept
        {
            return ptr_.release();
        }

        /// @brief Dereferences contained value.
        /// Calls std::terminate if allocation has been failed
        T& operator*() const noexcept
        {
            asset_valid();
            return *ptr_.get();
        }

        /// @brief Dereferences contained value.
        /// Calls std::terminate if allocation has been failed
        T* operator->() const noexcept
        {
            asset_valid();
            return ptr_.get();
        }

        /// @brief Returns is_valid() value
        explicit operator bool() const noexcept
        {
            return is_valid();
        }

    private:
        template <typename ... Args>
        explicit box(in_place_t, Args&& ... args)
                : ptr_(new(std::nothrow) T(std::forward<Args>(args)...)) {}

        template <typename RhsDeleter>
        explicit box(T* ptr, RhsDeleter&& rhs_deleter)
                noexcept(std::is_rvalue_reference<
                        decltype(std::forward<RhsDeleter>(rhs_deleter))
                >::value)
                : ptr_(ptr, std::forward<Deleter>(rhs_deleter)) {}

        explicit box(T* ptr) noexcept
                : ptr_(ptr) {}

        void asset_valid() const noexcept
        {
            if (!is_valid())
            {
                std::terminate();
            }
        }

    private:
        impl::boxed_ptr<T, Deleter> ptr_;
    };
}

namespace std
{
    template <typename T, typename Deleter>
    void swap(exl::box<T, Deleter>& lhs, exl::box<T, Deleter>& rhs) noexcept
    {
        lhs.swap(rhs);
    }
}