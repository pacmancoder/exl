// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <new>
#include <type_traits>

// only for std::terminate
#include <exception>

#include <exl/impl/mixed_storage_operations.hpp>

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
                impl::type_list_get_max_alignof<type_list_t>::value()
        >::type;

        using storage_operations = impl::mixed_storage_operations<type_list_t, storage_t>;

    public:
        template <typename U>
        explicit mixed(U&& rhs)
                : storage_()
                , tag_(tag_of<typename std::decay<U>::type>())
        {
            construct(std::forward<U>(rhs));
        }

        template <typename U>
        mixed<Types...>& operator=(U&& rhs)
        {
            if (tag_of<typename std::decay<U>::type>() == tag())
            {
                unsafe_unwrap<U>() = std::forward<U>(rhs);
            }
            else
            {
                destroy();
                construct(std::forward<U>(rhs));
            }

            return *this;
        }

        template <typename U>
        bool is()
        {
            return tag_of<U>() == tag_;
        }

        template <typename U>
        U& unwrap()
        {
            if (tag() != tag_of<U>())
            {
                std::terminate();
            }

            return unsafe_unwrap<U>();
        }

        ~mixed()
        {
            destroy();
        }

    private:

        void destroy()
        {
            storage_operations::destroy(storage_, tag_);
        }

        template <typename U>
        void construct(U&& rhs)
        {
            new(&storage_) (typename std::decay<U>::type)(std::forward<U>(rhs));
        }

        template <typename U>
        U& unsafe_unwrap()
        {
            return reinterpret_cast<U&>(storage_);
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
