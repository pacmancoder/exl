// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <new>
#include <type_traits>
#include <exception>

#include <exl/impl/mixed_storage_operations.hpp>

namespace exl
{
    namespace impl { namespace marker
    {
        class mixed {};
    }}

    template <typename T>
    class in_place_type_t
    {
    public:
        explicit in_place_type_t() = default;
    };

#if __cpp_variable_templates >= 201304

    template <typename T>
    constexpr in_place_type_t<T> in_place_type;

#endif

    /// @brief Represents tagged union type
    ///
    /// This class provides tagged union implementation. Any subset of instantiated class can be
    /// implicitly casted to the its superset (without rtti).
    /// Value can be acquired either using direct access methods exl::mixed::is() and
    /// exl::mixed::unwrap() or indirect access methods like exl::mixed::map(), exl::mixed::when()
    /// exl::mixed::catch(), exl::mixed::unwrap_or, etc.
    ///
    /// @tparam Types List of union variants
    template <typename ... Types>
    class mixed : impl::marker::mixed
    {
    public:
        template <typename ... FTypes>
        friend
        class mixed;

    public:
        using tag_t = uint8_t;
        using type_list_t = impl::type_list<Types...>;
        using storage_t = typename std::aligned_storage<
                impl::type_list_get_max_sizeof<type_list_t>::value(),
                impl::type_list_get_max_alignof<type_list_t>::value()
        >::type;

    public:
        /// @brief Copy-constructs self from exl::mixed of same type
        mixed(const mixed<Types...>& rhs)
                : storage_()
                , tag_(0)
        {
            construct_from_subset_by_copy(rhs);
        }

        /// @brief Move-constructs self from exl::mixed of same type
        mixed(mixed<Types...>&& rhs) noexcept
                : storage_()
                , tag_(0)
        {
            construct_from_subset_by_move(std::move(rhs));
        }

        /// @brief Copy-constructs self from exl::mixed of different type
        template <typename ... RhsTypes>
        mixed(const mixed<RhsTypes...>& rhs)
                : storage_()
                , tag_(0)
        {
            construct_from_subset_by_copy(rhs);
        }

        /// @brief Move-constructs self from exl::mixed of different type
        template <typename ... RhsTypes>
        mixed(mixed<RhsTypes...>&& rhs) noexcept
                : storage_()
                , tag_(0)
        {
            construct_from_subset_by_move(std::move(rhs));
        }

        /// @brief Constructs self from specific union variant of self
        /// @tparam U Union variant type
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

        /// @brief Constructs mixed with value constructed in-place
        /// @tparam U type to in-place construct
        /// @tparam Args in-place construction arguments
        template <typename U, typename ... Args>
        explicit mixed(in_place_type_t<U>, Args&& ... args)
                : storage_()
                , tag_(tag_of<U>())
        {
            construct_in_place<U>(std::forward<Args>(args)...);
        }

        /// @brief Assigns specific union variant of self
        /// @tparam U Union variant type
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

        /// @brief Copy-assigns exl::mixed of the same type
        template <typename ... RhsTypes>
        mixed<Types...>& operator=(const mixed<RhsTypes...>& rhs)
        {
            using IDMap = impl::type_list_subset_id_mapping<
                    type_list_t,
                    typename mixed<RhsTypes...>::type_list_t
            >;

            if (tag_ != IDMap::get(rhs.tag_))
            {
                destroy();
                construct_from_subset_by_copy(rhs);
            }
            else
            {
                assign_from_subset_by_copy(rhs);
            }

            return *this;
        }

        /// @brief Move-assigns exl::mixed of same type
        template <typename ... RhsTypes>
        mixed<Types...>& operator=(mixed<RhsTypes...>&& rhs) noexcept
        {
            using IDMap = impl::type_list_subset_id_mapping<
                    type_list_t,
                    typename mixed<RhsTypes...>::type_list_t
            >;

            if (tag_ != IDMap::get(rhs.tag_))
            {
                destroy();
                construct_from_subset_by_move(std::forward<mixed<RhsTypes...>>(rhs));
            }
            else
            {
                assign_from_subset_by_move(std::forward<mixed<RhsTypes...>>(rhs));
            }

            return *this;
        }

        /// @brief Constructs new value in-place
        /// @tparam U Type of new value to construct
        /// @tparam Args Arguments to the value which is being constructed
        template <typename U, typename ... Args>
        void emplace(Args&& ... args)
        {
            destroy();
            construct_in_place<U>(std::forward<Args>(args)...);
            tag_ = tag_of<U>();
        }

        /// @brief Checks if current stored variant is same as specified type U
        /// @tparam U Type to perform check for
        /// @return True when stored type is same as specified type U, false in other cases.
        template <typename U>
        bool is()
        {
            return tag_of<U>() == tag_;
        }

        /// @brief Returns reference to the value with specified type.
        ///
        /// Calls std::terminate if stored type is not equal to the requested type.
        /// Please use this method only with conjunction with exl::mixed::is() of when type is
        /// clearly known at the moment of call (e.g. right after explicit construction of
        /// ext::mixed) In other cases please use indirect access methods.
        ///
        /// @tparam U Requested type to unwrap
        /// @return Reference to unwrapped type
        template <typename U>
        U& unwrap()
        {
            assert_type<U>();
            return unsafe_unwrap<U>();
        }

        /// Calls destructor for last stored variant in self
        ~mixed()
        {
            destroy();
        }

    private:
        using StorageOperations = impl::mixed_storage_operations<type_list_t, storage_t>;

    private:
        template <typename U>
        void assert_type()
        {
            if (tag() != tag_of<U>())
            {
                std::terminate();
            }
        }

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
            using RhsStorage = typename U::storage_t;

            using RhsIDMap = impl::type_list_subset_id_mapping<
                    type_list_t,
                    typename U::type_list_t
            >;

            RhsStorageOperations::copy_construct_from(
                    reinterpret_cast<RhsStorage&>(storage_),
                    rhs.storage_,
                    rhs.tag_
            );
            tag_ = RhsIDMap::get(rhs.tag_);
        }

        template <typename U>
        void construct_from_subset_by_move(U&& rhs)
        {
            using RhsStorageOperations = typename U::StorageOperations;
            using RhsStorage = typename U::storage_t;

            using RhsIDMap = impl::type_list_subset_id_mapping<
                    type_list_t,
                    typename U::type_list_t
            >;

            RhsStorageOperations::move_construct_from(
                    reinterpret_cast<RhsStorage&>(storage_),
                    std::move(rhs.storage_),
                    rhs.tag_
            );
            tag_ = RhsIDMap::get(rhs.tag_);
        }

        template <typename T, typename ... Args>
        void construct_in_place(Args&& ... args)
        {
            new(&storage_) T(std::forward<Args>(args)...);
        }

        template <typename U>
        void assign_from_subset_by_copy(const U& rhs)
        {
            using RhsStorageOperations = typename U::StorageOperations;
            using RhsStorage = typename U::storage_t;

            using RhsIDMap = impl::type_list_subset_id_mapping<
                    type_list_t,
                    typename U::type_list_t
            >;

            RhsStorageOperations::copy_assign_from(
                    reinterpret_cast<RhsStorage&>(storage_),
                    rhs.storage_,
                    rhs.tag_
            );
            tag_ = RhsIDMap::get(rhs.tag_);
        }

        template <typename U>
        void assign_from_subset_by_move(U&& rhs)
        {
            using RhsStorageOperations = typename U::StorageOperations;
            using RhsStorage = typename U::storage_t;

            using RhsIDMap = impl::type_list_subset_id_mapping<
                    type_list_t,
                    typename U::type_list_t
            >;

            RhsStorageOperations::move_assign_from(
                    reinterpret_cast<RhsStorage&>(storage_),
                    std::move(rhs.storage_),
                    rhs.tag_
            );
            tag_ = RhsIDMap::get(rhs.tag_);
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
