// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <utility>
#include <type_traits>

namespace exl
{
    namespace impl
    {
        namespace marker
        {
            // exl::mixed
            struct matcher_when {};
            struct matcher_when_exact {};
            struct matcher_otherwise {};

            // exl::box
            struct matcher_when_valid {};
        }

        template <typename Kind, typename Target, typename Func>
        class matcher
        {
        public:
            using kind_t = Kind;
            using target_type_t = Target;

        public:
            explicit matcher(Func rhs)
                    : impl(rhs) {}

        public:
            Func impl;
        };
    }

    template <typename T, typename Func>
    using when_t = impl::matcher<impl::marker::matcher_when, T, Func>;

    template <typename T, typename Func>
    using when_exact_t = impl::matcher<impl::marker::matcher_when_exact, T, Func>;

    template <typename Func>
    using otherwise_t = impl::matcher<impl::marker::matcher_otherwise, void, Func>;

    template <typename Func>
    using when_valid_t = impl::matcher<impl::marker::matcher_when_valid, void, Func>;

    /// @brief Returns matcher for non-strict "is same" comparision
    template <
            typename T,
            typename Func,
            typename Matcher = impl::matcher<
                    impl::marker::matcher_when,
                    T,
                    Func
            >
    >
    Matcher when(
            Func&& func
    ) noexcept(std::is_rvalue_reference<decltype(std::forward<Func>(func))>::value)
    {
        return Matcher(std::forward<Func>(func));
    }

    /// @brief Returns matcher for strict "is same" comparision
    template <
            typename T,
            typename Func,
            typename Matcher = impl::matcher<
                    impl::marker::matcher_when_exact,
                    T,
                    Func
            >
    >
    Matcher when_exact(
            Func&& func
    ) noexcept(std::is_rvalue_reference<decltype(std::forward<Func>(func))>::value)
    {
        return Matcher(std::forward<Func>(func));
    }

    /// @brief Returns matcher for covering "else" clause
    template <
            typename Func,
            typename Matcher = impl::matcher<
                    impl::marker::matcher_otherwise,
                    void,
                    Func
            >
    >
    Matcher otherwise(
            Func&& func
    ) noexcept(std::is_rvalue_reference<decltype(std::forward<Func>(func))>::value)
    {
        return Matcher(std::forward<Func>(func));
    }

    /// @brief Returns matcher for case of valid exl::box pointer
    template <
            typename Func,
            typename Matcher = impl::matcher<
                    impl::marker::matcher_when_valid,
                    void,
                    Func
            >
    >
    Matcher when_valid(
            Func&& func
    ) noexcept(std::is_rvalue_reference<decltype(std::forward<Func>(func))>::value)
    {
        return Matcher(std::forward<Func>(func));
    }
}