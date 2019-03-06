// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <new>
#include <type_traits>
#include <utility>

#include <exl/in_place.hpp>

#include <exl/details/box/deleter_function.hpp>
#include <exl/details/box/deleter_object.hpp>
#include <exl/details/box/get_default_deleter.hpp>

#include <exl/impl/box/boxed_ptr.hpp>

namespace exl
{
    /// @brief Represents type which provides exception-less dynamic allocation mechanism
    /// result of allocation can be checked with is_valid method
    ///
    /// @tparam T type to wrap
    /// @tparam Deleter custom deleter type, optional
    ///
    /// @warning If passing deleter to .from_ptr(...) by copy, copy constructor should be nothrow
    template <
            typename T,
            typename Deleter = typename get_default_deleter<T>::type
    >
    class box
    {
    public:
        using boxed_ptr_t = impl::boxed_ptr<T, Deleter>;

        using element_t = typename std::remove_all_extents<T>::type;
        using const_element_t = typename std::add_const<element_t>::type;

        using ptr_t = typename std::add_pointer<element_t>::type;
        using const_ptr_t = typename std::add_pointer<const_element_t>::type;

        using ref_t = typename std::add_lvalue_reference<element_t>::type;
        using const_ref_t = typename std::add_lvalue_reference<const_element_t>::type;

        /// @brief Constructs box from pointer with provided deleter
        ///
        /// is_valid will return false if provided pointer is nullptr
        template <typename RhsDeleter>
        explicit box(ptr_t ptr, RhsDeleter&& rhs_deleter) noexcept
                : ptr_(ptr, std::forward<Deleter>(rhs_deleter)) {}

        /// @brief Constructs box from pointer with default-constructed deleter
        ///
        /// is_valid will return false if provided pointer is nullptr
        explicit box(ptr_t ptr) noexcept
                : ptr_(ptr) {}

        /// @brief Creates boxed type
        ///
        /// When allocation was failed, is_valid() will return false
        ///
        /// @tparam Args object constructor arguments types
        /// @param args object constructor arguments
        /// @return boxed object
        template <
                typename U = T,
                typename = typename std::enable_if<!std::is_array<U>::value>::type,
                typename ... Args
        >
        static box make(Args&& ... args)
        noexcept(std::is_nothrow_constructible<element_t, Args...>::value)
        {
            return box(new(std::nothrow) element_t(std::forward<Args>(args)...));
        }

        /// @brief Creates boxed array
        ///
        /// When allocation was failed, is_valid() will return false
        ///
        /// @param size array elements to create
        /// @return boxed array
        template <
                typename U = T,
                typename = typename std::enable_if<std::is_array<U>::value>::type
        >
        static box make(size_t size)
        noexcept(std::is_nothrow_constructible<element_t>::value)
        {
            return box(new(std::nothrow) element_t[size]);
        }

        /// @brief Constructs box from pointer with default-constructed deleter
        ///
        /// is_valid will return false if provided pointer is nullptr
        static box from_ptr(ptr_t ptr) noexcept
        {
            return box(ptr);
        }

        /// @brief Constructs box from pointer with provided deleter
        ///
        /// is_valid will return false if provided pointer is nullptr
        template <typename RhsDeleter>
        static box from_ptr(ptr_t ptr, RhsDeleter&& deleter) noexcept
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
        box& operator=(box<U, UDeleter>&& rhs) noexcept
        {
            ptr_ = std::move(rhs.ptr_);
            return *this;
        }

        /// @brief Returns false if allocation of box has been failed
        bool is_valid() const noexcept
        {
            return ptr_.get() != nullptr;
        }

        /// @brief Destroys previously contained object and owns provided pointer
        void reset(ptr_t rhs) noexcept
        {
            ptr_.reset(rhs);
        }

        /// @brief Releases ownership of contained pointer and returns it
        ptr_t release() noexcept
        {
            return ptr_.release();
        }

        /// @brief Dereferences contained value. or first element in case of array value
        /// Calls std::terminate if allocation has been failed
        const_ref_t operator*() const noexcept
        {
            asset_valid();
            return *ptr_.get();
        }

        /// @brief Dereferences contained value. or first element in case of array value
        /// Calls std::terminate if allocation has been failed
        ref_t operator*() noexcept
        {
            asset_valid();
            return *ptr_.get();
        }

        /// @brief element reference from underlying array by index
        /// Calls std::terminate if allocation has been failed
        ///
        /// @note defined only when T is array
        template <
                typename U = T,
                typename = typename std::enable_if<std::is_array<U>::value>::type
        >
        ref_t operator[](size_t index) noexcept
        {
            asset_valid();
            return ptr_.get()[index];
        }

        /// @brief returns element const reference from underlying array by index
        /// Calls std::terminate if allocation has been failed
        ///
        /// @note defined only when T is array
        template <
                typename U = T,
                typename = typename std::enable_if<std::is_array<U>::value>::type
        >
        const_ref_t operator[](size_t index) const noexcept
        {
            asset_valid();
            return ptr_.get()[index];
        }

        /// @brief Dereferences contained value.
        /// Calls std::terminate if allocation has been failed
        const_ptr_t operator->() const noexcept
        {
            asset_valid();
            return ptr_.get();
        }

        /// @brief Dereferences contained value.
        /// Calls std::terminate if allocation has been failed
        ptr_t operator->() noexcept
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
        void asset_valid() const noexcept
        {
            if (!is_valid())
            {
                std::terminate();
            }
        }

    private:
        boxed_ptr_t ptr_;
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