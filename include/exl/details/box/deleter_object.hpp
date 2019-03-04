// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>
#include <utility>

namespace exl
{
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
}