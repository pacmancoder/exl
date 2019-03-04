// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <exl/mixed.hpp>

#include <termination_test.hpp>

void termination_test()
{
    exl::mixed<int, std::runtime_error, std::exception> m(std::runtime_error("boom"));
    m.unwrap_exact<std::exception>();
}