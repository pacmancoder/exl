// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <exl/none.hpp>
#include <exl/mixed.hpp>

namespace exl
{

    /// @brief special case of exl::mixed with explicit exl::none in type list with addition of
    /// few usefull methods - take, make_none, is_none
    template <typename ... Types>
    class mixed_option : public mixed<exl::none, Types...>
    {
    public:
        using base_mixed_t = mixed<exl::none, Types...>;

    public:
        /// @brief Forwards construction to exl::mixed. see exl::mixed::mixed
        template <typename ... CtorArgs>
        mixed_option(CtorArgs&& ... args)
                : base_mixed_t(std::forward<CtorArgs>(args)...) {}


        /// @brief Verbose alias for in-place construction
        template <typename U, typename ... Args>
        static mixed_option<Types...> make(Args&& ... args)
        {
            return mixed_option<Types...>(in_place_type_t<U>(), std::forward<Args>(args)...);
        }

        /// @brief Returns mixed option with ex::none
        static mixed_option<Types...> make_none()
        {
            return make<exl::none>();
        }

        /// @brief Forwards assignment to exl::mixed. see exl::mixed::operator=
        template <typename U>
        mixed_option<Types...>& operator=(U&& rhs)
        {
            return reinterpret_cast<mixed_option<Types...>&>(
                    base_mixed_t::operator=(std::forward<U>(rhs))
            );
        }

        /// @brief Returns contained value and sets current value to exl::none
        /// @throws When contained value is not ecactly equal to U
        /// @tparam U type to take from mixed
        template <typename U>
        U take()
        {
            base_mixed_t::template assert_type_exact<U>();

            U result = std::move(base_mixed_t::template unwrap_exact<U>());
            *this = exl::none();
            return result;
        }

        /// @brief Returns true if contained type is exl::mixed
        bool is_none()
        {
            return base_mixed_t::template is<exl::none>();
        }
    };
}