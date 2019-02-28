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


        template <
                typename T,
                typename get_deleter_function_type<T>::type DeleterFunc
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
                    typename get_deleter_function_type<U>::type UDeleterFunc,
                    typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type
            >
            deleter_function(deleter_function<U, UDeleterFunc>&&) {}

            template <
                    typename U,
                    typename get_deleter_function_type<U>::type UDeleterFunc,
                    typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type
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

        template <typename T>
        void default_delete_scalar(T* p) { delete p; }

        template <typename T>
        void default_delete_array(T* p) { delete[] p; }

        template <typename T>
        struct get_default_deleter_function
        {
            using type = deleter_function<T, default_delete_scalar<T>>;
        };

        template <typename U>
        struct get_default_deleter_function<U[]>
        {
            using type = deleter_function<U[], default_delete_array<U>>;
        };

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
                                    typename std::decay<RhsDeleterImpl>::type*,
                                    deleter_t*
                            >::value
                    >::type,
                    typename = typename std::enable_if<std::is_convertible<U*, T*>::value>::type
            >
            explicit deleter_object(deleter_object<U, RhsDeleterImpl>&& rhs)
                    : deleter_(std::move(rhs.deleter_)) {}

            template <
                    typename RhsDeleter,
                    typename = typename std::enable_if<
                            std::is_convertible<
                                    typename RhsDeleter::deleter_t*,
                                    deleter_t*
                            >::value
                    >::type
            >
            deleter_object& operator=(RhsDeleter&& rhs)
            {
                deleter_ = std::move(rhs.deleter_);
                return *this;
            }

            Deleter& get_deleter()
            {
                return deleter_;
            }

            template <typename RhsDeleter>
            void set_deleter(RhsDeleter&& deleter)
            {
                deleter_ = std::forward<RhsDeleter>(deleter);
            }

            void destroy(ptr_t obj)
            {
                deleter_(obj);
            }

        private:
            Deleter deleter_;
        };

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
        static box make(Args&& ... args)
        {
            return box(in_place, std::forward<Args>(args)...);
        }

        static box from_ptr(T* ptr) noexcept
        {
            return box(ptr);
        }

        /// @brief Copy construction is not permitted
        box(const box&) = delete;

        /// @brief Copy assignment is not permitted
        box& operator=(const box&) = delete;

        /// @brief Constructs box by moving rhs
        /// @param rhs Source object to move from
        box(box&& rhs) noexcept
                : obj_(rhs.obj_)
        {
            rhs.obj_ = nullptr;
        }

        /// @brief Swaps two boxes
        void swap(box& rhs) noexcept
        {
            std::swap(obj_, rhs.obj_);
        }

        /// @brief Assigns new value to box, destroying previously contained value
        box& operator=(box&& rhs)
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
                delete obj_;
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