// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <new>

#include <exl/in_place.hpp>

namespace exl
{
    /// @brief Represents type which provides exception-less dynamic allocation mechanism
    /// result of allocation can be checked with is_valid method
    template <typename T>
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
        static box<T> make(Args&& ... args)
        {
            return box(in_place, std::forward<Args>(args)...);
        }

        static box<T> from_ptr(T* ptr) noexcept
        {
            return box(ptr);
        }

        /// @brief Copy construction is not permitted
        box(const box<T>&) = delete;

        /// @brief Copy assignment is not permitted
        box<T>& operator=(const box<T>&) = delete;

        /// @brief Constructs box by moving rhs
        /// @param rhs Source object to move from
        box(box<T>&& rhs) noexcept
                : obj_(rhs.obj_)
        {
            rhs.obj_ = nullptr;
        }

        /// @brief Swaps two boxes
        void swap(box<T>& rhs) noexcept
        {
            std::swap(obj_, rhs.obj_);
        }

        /// @brief Assigns new value to box, destroying previously contained value
        box<T>& operator=(box<T>&& rhs)
        {
            reset(nullptr);
            swap(rhs);
            return *this;
        }

        /// @brief Returns reference to the contained object.
        /// Calls std::terminate if box is not valid
        T& get() noexcept
        {
            asset_valid();
            return *obj_;
        }

        /// @brief Returns const reference to the contained object.
        /// Calls std::terminate if box is not valid
        const T& get() const noexcept
        {
            asset_valid();
            return *obj_;
        }

        /// @brief Returns false if allocation of box has been failed
        bool is_valid() const noexcept
        {
            return obj_ != nullptr;
        }

        /// @brief Destroys previously contained object and owns provided pointer
        void reset(T* rhs)
        {
            destroy();
            obj_ = rhs;
        }

        /// @brief Releases ownership of contained pointer and returns it
        T* release() noexcept
        {
            T* released = nullptr;
            std::swap(released, obj_);
            return released;
        }

        /// @brief Dereferences contained value.
        /// Calls std::terminate if allocation has been failed
        T& operator*() const noexcept
        {
            asset_valid();
            return *obj_;
        }

        /// @brief Dereferences contained value.
        /// Calls std::terminate if allocation has been failed
        T* operator->() const noexcept
        {
            asset_valid();
            return obj_;
        }

        /// @brief Returns is_valid() value
        explicit operator bool() const noexcept
        {
            return is_valid();
        }

        ~box()
        {
            destroy();
        }

    private:
        template <typename ... Args>
        explicit box(in_place_t, Args&& ... args)
                : obj_(new(std::nothrow) T(std::forward<Args>(args)...)) {}

        explicit box(T* ptr) noexcept
                : obj_(ptr) {}

        void asset_valid() const noexcept
        {
            if (!is_valid())
            {
                std::terminate();
            }
        }

        void destroy()
        {
            if (is_valid())
            {
                obj_->~T();
            }
        }

    private:
        T* obj_;
    };
}

namespace std
{
    template <typename T>
    void swap(exl::box<T>& lhs, exl::box<T>& rhs)
    {
        lhs.swap(rhs);
    }
}