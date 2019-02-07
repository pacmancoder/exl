// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <exl/impl/type_list.hpp>
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
        static void destroy(Storage& storage, type_list_tag_t actualTag)
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

        template <typename RhsStorageOperations, typename RhsStorage>
        static void copy_construct_from(
                Storage& dest,
                const RhsStorage& src,
                type_list_tag_t srcTag
        )
        {
            using idMap = type_list_subset_id_mapping<
                    type_list_t,
                    typename RhsStorageOperations::type_list_t
            >;

            if (idMap::get(srcTag) == ExpectedTag)
            {
                new(&dest) (CurrentType)(reinterpret_cast<const CurrentType&>(src));
                return;
            }

            mixed_storage_operations<
                    type_list_t,
                    storage_t,
                    ExpectedTag - 1
            >::template copy_construct_from<RhsStorageOperations>(
                    dest,
                    src,
                    srcTag
            );
        }

        template <typename RhsStorageOperations, typename RhsStorage>
        static void move_construct_from(
                Storage& dest,
                RhsStorage&& src,
                type_list_tag_t srcTag
        )
        {
            using idMap = type_list_subset_id_mapping<
                    type_list_t,
                    typename RhsStorageOperations::type_list_t
            >;

            if (idMap::get(srcTag) == ExpectedTag)
            {
                new(&dest) (CurrentType)(std::move(reinterpret_cast<CurrentType&>(src)));
                return;
            }

            mixed_storage_operations<
                    type_list_t,
                    storage_t,
                    ExpectedTag - 1
            >::template move_construct_from<RhsStorageOperations>(
                    dest,
                    std::forward<RhsStorage>(src),
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
        static void destroy(Storage& storage, type_list_tag_t)
        {
            reinterpret_cast<CurrentType*>(&storage)->~CurrentType();
        }

        template <typename RhsStorageOperations, typename RhsStorage>
        static void copy_construct_from(
                Storage& dest,
                const RhsStorage& src,
                type_list_tag_t
        )
        {
            new(&dest) (CurrentType)(reinterpret_cast<const CurrentType&>(src));
        }

        template <typename RhsStorageOperations, typename RhsStorage>
        static void move_construct_from(
                Storage& dest,
                RhsStorage&& src,
                type_list_tag_t
        )
        {
            new(&dest) (CurrentType)(std::move(reinterpret_cast<CurrentType&>(src)));
        }

    private:
        using CurrentType = typename impl::type_list_get_type_for_id<TL, 0>::type;
    };
}}
