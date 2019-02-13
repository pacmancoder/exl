// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

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

        template <typename Kind, typename Func>
        struct matcher
        {
            using type = Kind;
            Func impl;
        };
    }


    template <
            typename Func,
            typename Matcher = impl::matcher<
                    impl::marker::matcher_when,
                    typename std::decay<Func>::type
            >
    >
    Matcher when(Func&& func)
    {
        return Matcher{ std::forward<Func>(func) };
    }

    template <
            typename Func,
            typename Matcher = impl::matcher<
                    impl::marker::matcher_when_exact,
                    typename std::decay<Func>::type
            >
    >
    Matcher when_exact(Func&& func)
    {
        return Matcher{ std::forward<Func>(func) };
    }

    template <
            typename Func,
            typename Matcher = impl::matcher<
                    impl::marker::matcher_otherwise,
                    typename std::decay<Func>::type
            >
    >
    Matcher otherwise(Func&& func)
    {
        return Matcher{ std::forward<Func>(func) };
    }
}