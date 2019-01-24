// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <exl/impl/type_list.hpp>

namespace exl { namespace impl
{
    template <
        typename TL,
        typename Storage,
        type_list_tag_t ExpectedTag = impl::type_list_get_type_id<TL, typename TL::head>::value()>
    struct mixed_storage_operations
    {
    public:
        static void destroy(Storage& storage, type_list_tag_t actualTag)
        {
            using T = typename impl::type_list_get_type_for_id<TL, ExpectedTag>::type;

            if (actualTag == ExpectedTag)
            {
                reinterpret_cast<T*>(&storage)->~T();
                return;
            }

            mixed_storage_operations<TL, Storage, ExpectedTag - 1>::destroy(
                    storage,
                    actualTag
            );
        }
    };


    template <
        typename TL,
        typename Storage>
    struct mixed_storage_operations<TL, Storage, 0>
    {
    public:
        static void destroy(Storage& storage, type_list_tag_t)
        {
            using T = typename impl::type_list_get_type_for_id<TL, 0>::type;

            reinterpret_cast<T*>(&storage)->~T();
        }
    };
}}
