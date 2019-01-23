// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <new>
#include <type_traits>

#include <exl/impl/type_list.hpp>

namespace exl
{
    template <typename ... Types>
    class mixed
    {
    public:
        using tag_t = uint8_t;
        using type_list_t = impl::type_list<Types...>;
        using storage_t = typename std::aligned_storage<
            impl::type_list_get_max_sizeof<type_list_t>::value(),
            impl::type_list_get_max_alignof<type_list_t>::value()>::type;

    public:
        template <typename U>
        mixed(const U& rhs)
            : storage_()
            , tag_(tag_of<U>())
        {
            new (&storage_) U(rhs);
        }

        template <typename U>
        mixed(U&& rhs)
            : storage_()
            , tag_(tag_of<typename std::decay<U>::type>())
        {
            new (&storage_) (typename std::decay<U>::type)(std::forward<U>(rhs));
        }

        template <typename U>
        bool is()
        {
            return tag_of<U>() == tag_;
        }

        tag_t tag()
        {
            return tag_;
        }

        template <typename U>
        static constexpr tag_t tag_of()
        {
            return impl::type_list_get_type_id<type_list_t, U>::value();
        }

    private:
        storage_t storage_;
        tag_t tag_;
    };
}
