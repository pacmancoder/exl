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
            struct matcher_when {};
            struct matcher_when_exact {};
            struct matcher_otherwise {};
        }

        template <typename Kind, typename Target, typename Func>
        class matcher
        {
        public:
            using kind_t = Kind;
            using target_type_t = Target;

        public:
            explicit matcher(const Func& rhs)
                    : impl(rhs) {}

            explicit matcher(Func&& rhs) noexcept
                    : impl(std::move(rhs)) {}

        public:
            Func impl;
        };
    }


    /// @brief Returns matcher for non-strict "is same" comparision
    template <
            typename T,
            typename Func,
            typename Matcher = impl::matcher<
                    impl::marker::matcher_when,
                    T,
                    typename std::decay<Func>::type
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
                    typename std::decay<Func>::type
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
                    typename std::decay<Func>::type
            >
    >
    Matcher otherwise(
            Func&& func
    ) noexcept(std::is_rvalue_reference<decltype(std::forward<Func>(func))>::value)
    {
        return Matcher(std::forward<Func>(func));
    }
}