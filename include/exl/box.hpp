// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <new>
#include <utility>

#include <exl/in_place.hpp>

namespace exl
{
    namespace impl
    {
        template <typename T>
        struct get_deleter_function_type
        {
            using type = void (&)(T*);
        };

        template <typename T>
        struct get_deleter_function_type<T[]>
        {
            using type = void (&)(T*);
        };
    }

    /// @brief Provides type to define exl::box deleter with custom deleter function
    /// @tparam T type to delete
    /// @tparam DeleterFunc Custom deleter function reference
    template <
            typename T,
            typename impl::get_deleter_function_type<T>::type DeleterFunc
    >
    class deleter_function
    {
    public:
        using ptr_t = typename std::conditional<
                std::is_array<T>::value,
                typename std::decay<T>::type,
                typename std::add_pointer<typename std::decay<T>::type>::type
        >::type;

    public:
        deleter_function() = default;

        template <
                typename U,
                typename impl::get_deleter_function_type<U>::type UDeleterFunc,
                typename = typename std::enable_if<
                        std::is_convertible<
                                typename std::conditional<
                                        std::is_array<U>::value,
                                        typename std::decay<U>::type,
                                        typename std::add_pointer<U>::type
                                >::type,
                                ptr_t
                        >::value
                >::type
        >
        deleter_function(deleter_function<U, UDeleterFunc>&&) {}

        template <
                typename U,
                typename impl::get_deleter_function_type<U>::type UDeleterFunc,
                typename = typename std::enable_if<
                        std::is_convertible<
                                typename std::conditional<
                                        std::is_array<U>::value,
                                        typename std::decay<U>::type,
                                        typename std::add_pointer<U>::type
                                >::type,
                                ptr_t
                        >::value
                >::type
        >
        deleter_function& operator=(deleter_function<U, UDeleterFunc>&&) noexcept
        {
            return *this;
        }

        static void destroy(ptr_t obj)
        {
            DeleterFunc(obj);
        }
    };

    namespace impl
    {
        template <typename Deleter>
        struct is_deleter_function
        {
            static constexpr bool value()
            {
                return false;
            }
        };

        template <typename T, typename get_deleter_function_type<T>::type DeleterFunc>
        struct is_deleter_function<deleter_function<T, DeleterFunc>>
        {
            static constexpr bool value()
            {
                return true;
            }
        };

        template <typename T>
        void default_delete_scalar(T* p) { delete p; }

        template <typename T>
        void default_delete_array(T* p) { delete[] p; }
    }

    /// @brief returns default deleter for type T
    template <typename T>
    struct get_default_deleter_function
    {
        using type = deleter_function<T, impl::default_delete_scalar<T>>;
    };

    template <typename U>
    struct get_default_deleter_function<U[]>
    {
        using type = deleter_function<U[], impl::default_delete_array<U>>;
    };

    /// @brief Provides type to define exl::box deleter with custom object type
    /// @tparam T type to delete
    /// @tparam Deleter Custom deleter type
    template <typename T, typename Deleter>
    class deleter_object
    {
    public:
        template <typename FT, typename FDeleter>
        friend
        class deleter_object;

        using ptr_t = typename std::conditional<
                std::is_array<T>::value,
                typename std::decay<T>::type,
                typename std::add_pointer<typename std::decay<T>::type>::type
        >::type;

        using deleter_t = Deleter;

    public:
        template <
                typename = typename std::enable_if<
                        std::is_default_constructible<Deleter>::value
                >::type
        >
        deleter_object()
                : deleter_() {}

        template <
                typename RhsDeleterImpl,
                typename = typename std::enable_if<
                        std::is_convertible<
                                typename std::decay<RhsDeleterImpl>::type*,
                                deleter_t*
                        >::value
                >::type
        >
        explicit deleter_object(RhsDeleterImpl&& rhs)
                : deleter_(std::forward<RhsDeleterImpl>(rhs)) {}

        template <
                typename U,
                typename RhsDeleterImpl,
                typename = typename std::enable_if<
                        std::is_convertible<
                                RhsDeleterImpl*,
                                deleter_t*
                        >::value
                >::type,
                typename = typename std::enable_if<
                        std::is_convertible<
                                typename std::conditional<
                                        std::is_array<U>::value,
                                        typename std::decay<U>::type,
                                        typename std::add_pointer<U>::type
                                >::type,
                                ptr_t
                        >::value
                >::type
        >
        explicit deleter_object(deleter_object<U, RhsDeleterImpl>&& rhs)
                : deleter_(std::move(rhs.deleter_)) {}

        template <
                typename U,
                typename RhsDeleterImpl,
                typename = typename std::enable_if<
                        std::is_convertible<
                                RhsDeleterImpl*,
                                deleter_t*
                        >::value
                >::type,
                typename = typename std::enable_if<
                        std::is_convertible<
                                typename std::conditional<
                                        std::is_array<U>::value,
                                        typename std::decay<U>::type,
                                        typename std::add_pointer<U>::type
                                >::type,
                                ptr_t
                        >::value
                >::type
        >
        deleter_object& operator=(deleter_object<U, RhsDeleterImpl>&& rhs)
        {
            deleter_ = std::move(rhs.deleter_);
            return *this;
        }

        void destroy(ptr_t obj)
        {
            deleter_(obj);
        }

    private:
        Deleter deleter_;
    };

    namespace impl
    {
        template <typename T, typename Deleter = typename get_default_deleter_function<T>::type>
        class box_impl : public Deleter
        {
        public:
            template <typename FT, typename FDeleter>
            friend
            class box_impl;

            using ptr_t = typename Deleter::ptr_t;

        public:
            template <
                    typename = typename std::enable_if<
                            std::is_default_constructible<Deleter>::value
                    >::type
            >
            explicit box_impl(ptr_t ptr)
                    : Deleter()
                    , ptr_(ptr) {}

            template <typename U, typename RhsDeleter>
            explicit box_impl(box_impl<U, RhsDeleter>&& rhs) noexcept
                    : Deleter(std::move(rhs))
                    , ptr_(rhs.ptr_)
            {
                rhs.ptr_ = nullptr;
            }

            template <typename RhsDeleter>
            explicit box_impl(ptr_t ptr, RhsDeleter&& rhs_deleter)
                    : Deleter(std::forward<RhsDeleter>(rhs_deleter))
                    , ptr_(ptr) {}

            template <typename U, typename RhsDeleter>
            box_impl& operator=(box_impl<U, RhsDeleter>&& rhs) noexcept
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

            void swap(box_impl& rhs) noexcept
            {
                std::swap(ptr_, rhs.ptr_);
            }

            ~box_impl()
            {
                destroy_with_deleter();
            }

        private:
            void destroy_with_deleter()
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
    }
    /// @brief Represents type which provides exception-less dynamic allocation mechanism
    /// result of allocation can be checked with is_valid method
    /// @note Deleter object constructors should be no-throwing
    template <
            typename T,
            typename Deleter = typename get_default_deleter_function<T>::type
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
        impl::box_impl<T, Deleter> ptr_;
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