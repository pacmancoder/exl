// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <exl/impl/mixed/type_list.hpp>
#include <new>

namespace exl { namespace impl
{
    template <
            typename TL,
            typename Storage,
            type_list_tag_t ExpectedTag = impl::type_list_get_type_id<
                    TL,
                    typename TL::head
            >::value()
    >
    struct mixed_storage_operations
    {
    public:
        using type_list_t = TL;
        using storage_t = Storage;

    public:
        static void destroy(Storage& storage, type_list_tag_t actualTag) noexcept
        {
            if (actualTag == ExpectedTag)
            {
                reinterpret_cast<CurrentType*>(&storage)->~CurrentType();
                return;
            }

            mixed_storage_operations<TL, Storage, ExpectedTag - 1>::destroy(
                    storage,
                    actualTag
            );
        }

        static void copy_construct_from(
                Storage& dest,
                const Storage& src,
                type_list_tag_t srcTag
        )
        {
            if (srcTag == ExpectedTag)
            {
                new(&dest) (CurrentType)(reinterpret_cast<const CurrentType&>(src));
                return;
            }

            mixed_storage_operations<type_list_t, storage_t, ExpectedTag - 1>::copy_construct_from(
                    dest,
                    src,
                    srcTag
            );
        }

        static void move_construct_from(
                Storage& dest,
                Storage&& src,
                type_list_tag_t srcTag
        ) noexcept
        {
            if (srcTag == ExpectedTag)
            {
                new(&dest) (CurrentType)(std::move(reinterpret_cast<CurrentType&>(src)));
                return;
            }

            mixed_storage_operations<type_list_t, storage_t, ExpectedTag - 1>::move_construct_from(
                    dest,
                    std::forward<Storage>(src),
                    srcTag
            );
        }

        static void copy_assign_from(
                Storage& dest,
                const Storage& src,
                type_list_tag_t srcTag
        )
        {
            if (srcTag == ExpectedTag)
            {
                reinterpret_cast<CurrentType&>(dest) = reinterpret_cast<const CurrentType&>(src);
                return;
            }

            mixed_storage_operations<type_list_t, storage_t, ExpectedTag - 1>::copy_assign_from(
                    dest,
                    src,
                    srcTag
            );
        }

        static void move_assign_from(
                Storage& dest,
                Storage&& src,
                type_list_tag_t srcTag
        ) noexcept
        {
            if (srcTag == ExpectedTag)
            {
                reinterpret_cast<CurrentType&>(dest) =
                        std::move(reinterpret_cast<CurrentType&>(src));
                return;
            }

            mixed_storage_operations<type_list_t, storage_t, ExpectedTag - 1>::move_assign_from(
                    dest,
                    std::move(src),
                    srcTag
            );
        }

    private:
        using CurrentType = typename impl::type_list_get_type_for_id<TL, ExpectedTag>::type;
    };


    template <typename TL, typename Storage>
    struct mixed_storage_operations<TL, Storage, 0>
    {
    public:
        static void destroy(Storage& storage, type_list_tag_t) noexcept
        {
            reinterpret_cast<CurrentType*>(&storage)->~CurrentType();
        }

        static void copy_construct_from(
                Storage& dest,
                const Storage& src,
                type_list_tag_t
        )
        {
            new(&dest) (CurrentType)(reinterpret_cast<const CurrentType&>(src));
        }

        static void move_construct_from(
                Storage& dest,
                Storage&& src,
                type_list_tag_t
        ) noexcept
        {
            new(&dest) (CurrentType)(std::move(reinterpret_cast<CurrentType&>(src)));
        }

        static void copy_assign_from(
                Storage& dest,
                const Storage& src,
                type_list_tag_t
        )
        {
            reinterpret_cast<CurrentType&>(dest) = reinterpret_cast<const CurrentType&>(src);
        }

        static void move_assign_from(
                Storage& dest,
                Storage&& src,
                type_list_tag_t
        ) noexcept
        {
            reinterpret_cast<CurrentType&>(dest) =
                    std::move(reinterpret_cast<CurrentType&>(src));
        }

    private:
        using CurrentType = typename impl::type_list_get_type_for_id<TL, 0>::type;
    };
}}
