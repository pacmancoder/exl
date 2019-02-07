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
    namespace impl { namespace marker
    {
        class mixed {};
    }}

    template <typename ... Types>
    class mixed : impl::marker::mixed
    {
    public:
        template <typename ... FTypes>
        friend
        struct mixed;

    public:
        using tag_t = uint8_t;
        using type_list_t = impl::type_list<Types...>;
        using storage_t = typename std::aligned_storage<
                impl::type_list_get_max_sizeof<type_list_t>::value(),
                impl::type_list_get_max_alignof<type_list_t>::value()
        >::type;

    public:
        mixed(const mixed<Types...>& rhs)
                : storage_()
                , tag_(0)
        {
            // TODO: make specialization without id mapping stage
            construct_from_subset_by_copy(rhs);
        }

        mixed(mixed<Types...>&& rhs) noexcept
                : storage_()
                , tag_(0)
        {
            // TODO: make specialization without id mapping stage
            construct_from_subset_by_move(std::move(rhs));
        }

        template <typename ... RhsTypes>
        mixed(const mixed<RhsTypes...>& rhs)
                : storage_()
                , tag_(0)
        {
            construct_from_subset_by_copy(rhs);
        }

        template <typename ... RhsTypes>
        mixed(mixed<RhsTypes...>&& rhs) noexcept
                : storage_()
                , tag_(0)
        {
            construct_from_subset_by_move(std::move(rhs));
        }

        template <
                typename U,
                typename = typename std::enable_if<
                        !std::is_base_of<
                                impl::marker::mixed,
                                typename std::decay<U>::type
                        >::value
                >::type
        >
        mixed(U&& rhs)
                : storage_()
                , tag_(tag_of<typename std::decay<U>::type>())
        {
            construct_from_specific(std::forward<U>(rhs));
        }

        template <
                typename U,
                typename = typename std::enable_if<
                        !std::is_base_of<
                                impl::marker::mixed,
                                typename std::decay<U>::type
                        >::value
                >::type
        >
        mixed<Types...>& operator=(U&& rhs)
        {
            constexpr auto newTag = tag_of<typename std::decay<U>::type>();

            if (newTag == tag())
            {
                unsafe_unwrap<U>() = std::forward<U>(rhs);
            }
            else
            {
                destroy();
                construct_from_specific(std::forward<U>(rhs));
                tag_ = newTag;
            }

            return *this;
        }

        void operator=(mixed<Types...>&) = delete;

        void operator=(mixed<Types...>&&) = delete;

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
        using StorageOperations = impl::mixed_storage_operations<type_list_t, storage_t>;

    private:
        void destroy()
        {
            StorageOperations::destroy(storage_, tag_);
        }

        template <typename U>
        void construct_from_specific(U&& rhs)
        {
            new(&storage_) (typename std::decay<U>::type)(std::forward<U>(rhs));
        }

        template <typename U>
        void construct_from_subset_by_copy(const U& rhs)
        {
            using RhsStorageOperations = typename U::StorageOperations;
            using RhsTL = typename U::type_list_t;

            StorageOperations::template copy_construct_from<RhsStorageOperations>(
                    storage_,
                    rhs.storage_,
                    rhs.tag_
            );

            tag_ = impl::type_list_subset_id_mapping<type_list_t, RhsTL>::get(
                    rhs.tag_
            );
        }

        template <typename U>
        void construct_from_subset_by_move(U&& rhs)
        {
            using RhsStorageOperations = typename U::StorageOperations;
            using RhsTL = typename U::type_list_t;

            StorageOperations::template move_construct_from<RhsStorageOperations>(
                    storage_,
                    std::move(rhs.storage_),
                    rhs.tag_
            );

            tag_ = impl::type_list_subset_id_mapping<type_list_t, RhsTL>::get(
                    rhs.tag_
            );
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
