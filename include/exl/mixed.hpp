// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <new>
#include <type_traits>
#include <exception>
#include <utility>

#include <exl/matchers.hpp>
#include <exl/in_place.hpp>

#include <exl/impl/mixed/mixed_storage_operations.hpp>
#include <exl/impl/mixed/markers.hpp>

namespace exl
{
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
                typename Decayed = typename std::decay<U>::type,
                typename = typename std::enable_if<
                        !std::is_base_of<impl::marker::mixed, Decayed>::value &&
                                !impl::is_in_place_type_t<Decayed>::value()
                >::type,
                typename T = typename impl::type_list_get_best_match<type_list_t, U>::type,
                typename = typename std::enable_if<std::is_constructible<T, U>::value>::type
        >
        mixed(U&& rhs) noexcept(std::is_rvalue_reference<decltype(std::forward<U>(rhs))>::value)
                : storage_()
                , tag_(tag_of<T>())
        {
            construct_from<T>(std::forward<U>(rhs));
        }

        /// @brief Constructs mixed with value constructed in-place
        /// @tparam U type to in-place construct
        /// @tparam Args in-place construction arguments
        template <
                typename U,
                typename T = typename impl::type_list_get_best_match<type_list_t, U>::type,
                typename = typename std::enable_if<std::is_constructible<T, U>::value>::type,
                typename ... Args
        >
        explicit mixed(in_place_type_t<U>, Args&& ... args)
                : storage_()
                , tag_(tag_of<U>())
        {
            construct_in_place<U>(std::forward<Args>(args)...);
        }

        /// @brief Verbose alias for in-place construction
        template <typename U, typename ... Args>
        static mixed<Types...> make(Args&& ... args)
        {
            return mixed<Types...>(in_place_type_t<U>(), std::forward<Args>(args)...);
        }

        /// @brief Assigns specific union variant of self
        /// @tparam U Union variant type
        template <
                typename U,
                typename Decayed = typename std::decay<U>::type,
                typename = typename std::enable_if<
                        !std::is_base_of<impl::marker::mixed, Decayed>::value
                >::type,
                typename T = typename impl::type_list_get_best_match<type_list_t, U>::type,
                typename = typename std::enable_if<std::is_constructible<T, U>::value>::type
        >
        mixed<Types...>& operator=(const U& rhs)
        {
            constexpr auto newTag = tag_of<T>();

            if (newTag == tag())
            {
                unsafe_unwrap<T>() = rhs;
            }
            else
            {
                destroy();
                construct_from<T>(rhs);
                tag_ = newTag;
            }

            return *this;
        }

        /// @brief Assigns specific union variant of self
        /// @tparam U Union variant type
        template <
                typename U,
                typename Decayed = typename std::decay<U>::type,
                typename = typename std::enable_if<
                        !std::is_base_of<impl::marker::mixed, Decayed>::value
                >::type,
                typename T = typename impl::type_list_get_best_match<type_list_t, U>::type,
                typename = typename std::enable_if<std::is_constructible<T, U>::value>::type
        >
        mixed<Types...>& operator=(U&& rhs) noexcept
        {
            constexpr auto newTag = tag_of<T>();

            if (newTag == tag())
            {
                unsafe_unwrap<T>() = std::forward<U>(rhs);
            }
            else
            {
                destroy();
                construct_from<T>(std::forward<U>(rhs));
                tag_ = newTag;
            }

            return *this;
        }

        /// @brief Copy-assigns exl::mixed of the same type
        mixed<Types...>& operator=(const mixed<Types...>& rhs)
        {
            if (tag_ != rhs.tag_)
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

        /// @brief Move-assigns exl::mixed of subset type
        mixed<Types...>& operator=(mixed<Types...>&& rhs) noexcept
        {
            if (tag_ != rhs.tag_)
            {
                destroy();
                construct_from_subset_by_move(std::move(rhs));
            }
            else
            {
                assign_from_subset_by_move(std::move(rhs));
            }

            return *this;
        }

        /// @brief Copy-assigns exl::mixed of the subset type
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

        /// @brief Move-assigns exl::mixed of subset type
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
                construct_from_subset_by_move(std::move(rhs));
            }
            else
            {
                assign_from_subset_by_move(std::move(rhs));
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

        /// @brief Checks if current stored variant is same as specified type U or derived from it
        /// @tparam U Type to perform check for
        /// @return True when stored type is same as specified type U or derived from it, false in
        /// other case.
        template <typename U>
        bool is() const noexcept
        {
            return
                    impl::type_list_id_set_contains<
                            typename impl::type_list_get_ids_of_derived_types<type_list_t, U>::type
                    >::check(tag_)
                            || impl::type_list_id_set_contains<
                                    typename impl::type_list_get_ids_of_same_types<
                                            type_list_t,
                                            U
                                    >::type
                            >::check(tag_);
        }

        /// @brief Checks if current stored variant is same as specified type U
        /// @tparam U Type to perform check for
        /// @return True when stored type is same as specified type U, false in other case.
        template <typename U>
        bool is_exact() const noexcept
        {
            return impl::type_list_get_type_id<type_list_t, U>::value() == tag_;
        }

        /// @brief Returns reference to the value with specified type.
        ///
        /// Calls std::terminate if stored type is neither equal to the requested type nor derived
        /// from it.
        /// Please use this method only with conjunction with exl::mixed::is() of when type is
        /// clearly known at the moment of call (e.g. right after explicit construction of
        /// ext::mixed) In other cases please use indirect access methods.
        ///
        /// @tparam U Requested type to unwrap
        /// @return Reference to unwrapped type
        template <typename U>
        U& unwrap() noexcept
        {
            assert_type<U>();
            return unsafe_unwrap<U>();
        }

        /// @brief Returns const reference to the value with specified type.
        ///
        /// Calls std::terminate if stored type is neither equal to the requested type nor derived
        /// from it.
        /// Please use this method only with conjunction with exl::mixed::is() of when type is
        /// clearly known at the moment of call (e.g. right after explicit construction of
        /// ext::mixed) In other cases please use indirect access methods.
        ///
        /// @tparam U Requested type to unwrap
        /// @return Const reference to unwrapped type
        template <typename U>
        const U& unwrap() const noexcept
        {
            assert_type<U>();
            return unsafe_unwrap<U>();
        }

        /// @brief Returns reference to the value with specified type.
        ///
        /// Calls std::terminate if stored type is not equal to the requested type.
        /// Please use this method only with conjunction with exl::mixed::is_exact() of when type is
        /// clearly known at the moment of call (e.g. right after explicit construction of
        /// ext::mixed) In other cases please use indirect access methods.
        ///
        /// @tparam U Requested type to unwrap
        /// @return Reference to unwrapped type
        template <typename U>
        U& unwrap_exact() noexcept
        {
            assert_type_exact<U>();
            return unsafe_unwrap<U>();
        }

        /// @brief Returns const reference to the value with specified type.
        ///
        /// Calls std::terminate if stored type is not equal to the requested type.
        /// Please use this method only with conjunction with exl::mixed::is_exact() of when type is
        /// clearly known at the moment of call (e.g. right after explicit construction of
        /// ext::mixed) In other cases please use indirect access methods.
        ///
        /// @tparam U Requested type to unwrap
        /// @return Const reference to unwrapped type
        template <typename U>
        const U& unwrap_exact() const noexcept
        {
            assert_type_exact<U>();
            return unsafe_unwrap<U>();
        }

        /// @brief calls func with argument as argument to requested type if contained type is
        /// same of derived from U
        /// @tparam U type to check for
        /// @tparam Func functor to call when type is same or derived
        template <typename U, typename Func>
        void on(Func&& func) noexcept
        {
            if (is<U>())
            {
                func(unsafe_unwrap<U>());
            }
        }

        /// @brief calls func with argument as argument to requested type if contained type is same
        /// as U
        /// @tparam U type to check for
        /// @tparam Func functor to call when type is same
        template <typename U, typename Func>
        void on_exact(Func&& func) noexcept
        {
            if (is_exact<U>())
            {
                func(unsafe_unwrap<U>());
            }
        }

        /// @brief exl::mixed visiting function. Set of matchers should cover all cases, otherwise
        /// code will not compile. see exl::when, exl::when_exact, exl::otherwise matchers.
        /// All functors of the matchers should return value of type U.
        /// @tparam U return type of map expression
        /// @tparam Matchers set of matcher object types to perform match
        /// @param matchers set of matcher objects to perform match
        template <typename U, typename ... Matchers>
        U map(Matchers&& ... matchers) const noexcept
        {
            return map_internal<U, type_list_t>(std::forward<Matchers>(matchers)...);
        }

        /// @brief exl::mixed visiting function. Set of matchers should cover all cases, otherwise
        /// code will not compile. see exl::when, exl::when_exact, exl::otherwise matchers.
        /// @tparam U return type of map expression
        /// @tparam Matchers set of matcher object types to perform match
        /// @param matchers set of matcher objects to perform match
        template <typename ... Matchers>
        void match(Matchers&& ... matchers) const noexcept
        {
            return map_internal<void, type_list_t>(std::forward<Matchers>(matchers)...);
        }

        /// @brief Returns current tag of mixed type. Please use returned value for check against
        /// exl::mixed::tag_of<T>()
        tag_t tag() const noexcept
        {
            return tag_;
        }

        /// @brief Returns tag for type U
        /// @tparam U Type to search tag for
        template <typename U>
        static constexpr tag_t tag_of() noexcept
        {
            return impl::type_list_get_type_id<type_list_t, U>::value();
        }

        /// Calls destructor for last stored variant in self
        virtual ~mixed()
        {
            destroy();
        }

    private:
        using StorageOperations = impl::mixed_storage_operations<type_list_t, storage_t>;

    private:
        template <typename U>
        void assert_type() const noexcept
        {
            if (!is<U>())
            {
                std::terminate();
            }
        }

        template <typename U>
        void assert_type_exact() const noexcept
        {
            if (!is_exact<U>())
            {
                std::terminate();
            }
        }

        void destroy() noexcept
        {
            StorageOperations::destroy(storage_, tag_);
        }

        template <typename T, typename U>
        void construct_from(
                U&& rhs
        ) noexcept(std::is_rvalue_reference<decltype(std::forward<U>(rhs))>::value)
        {
            new(&storage_) (T)(std::forward<U>(rhs));
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
        void construct_from_subset_by_move(U&& rhs) noexcept
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
        void assign_from_subset_by_move(U&& rhs) noexcept
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
        U& unsafe_unwrap() noexcept
        {
            return reinterpret_cast<U&>(storage_);
        }

        template <typename U>
        const U& unsafe_unwrap() const noexcept
        {
            return reinterpret_cast<const U&>(storage_);
        }

        template <typename U, typename TL, typename Matcher, typename ... Tail>
        typename std::enable_if<
                std::is_same<typename Matcher::kind_t, impl::marker::matcher_when>::value,
                U
        >::type map_internal(Matcher&& matcher, Tail&& ... tail) const noexcept
        {
            using Target = typename Matcher::target_type_t;
            if (is<Target>())
            {
                return static_cast<U>(matcher.impl(unsafe_unwrap<Target>()));
            }

            return static_cast<U>(map_internal<
                    U,
                    typename impl::type_list_remove_derived<
                            typename impl::type_list_remove_same<TL, Target>::type,
                            Target
                    >::type,
                    Tail...
            >(std::forward<Tail>(tail)...));
        }

        template <typename U, typename TL, typename Matcher>
        typename std::enable_if<
                std::is_same<typename Matcher::kind_t, impl::marker::matcher_when>::value &&
                        std::is_same<
                                typename impl::type_list_remove_derived<
                                        typename impl::type_list_remove_same<
                                                TL,
                                                typename Matcher::target_type_t
                                        >::type,
                                        typename Matcher::target_type_t
                                >::type,
                                impl::type_list<>
                        >::value,
                U
        >::type map_internal(Matcher&& matcher) const noexcept
        {
            return static_cast<U>(matcher.impl(unsafe_unwrap<typename Matcher::target_type_t>()));
        }

        template <typename U, typename TL, typename Matcher, typename ... Tail>
        typename std::enable_if<
                std::is_same<typename Matcher::kind_t, impl::marker::matcher_when_exact>::value,
                U
        >::type map_internal(Matcher&& matcher, Tail&& ... tail) const noexcept
        {
            using Target = typename Matcher::target_type_t;
            if (is_exact<Target>())
            {
                return static_cast<U>(matcher.impl(unsafe_unwrap<Target>()));
            }

            return static_cast<U>(map_internal<
                    U,
                    typename impl::type_list_remove_same<TL, Target>::type,
                    Tail...
            >(std::forward<Tail>(tail)...));
        }

        template <typename U, typename TL, typename Matcher>
        typename std::enable_if<
                std::is_same<typename Matcher::kind_t, impl::marker::matcher_when_exact>::value &&
                        std::is_same<
                                typename impl::type_list_remove_same<
                                        TL,
                                        typename Matcher::target_type_t
                                >::type,
                                impl::type_list<>
                        >::value,
                U
        >::type map_internal(Matcher&& matcher) const noexcept
        {
            return static_cast<U>(matcher.impl(unsafe_unwrap<typename Matcher::target_type_t>()));
        }

        template <typename U, typename TL = type_list_t, typename Matcher, typename ... Tail>
        typename std::enable_if<
                std::is_same<typename Matcher::kind_t, impl::marker::matcher_otherwise>::value,
                U
        >::type map_internal(Matcher&& matcher) const noexcept
        {
            return static_cast<U>(matcher.impl());
        }

    private:
        storage_t storage_;
        tag_t tag_;
    };
}