// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>
#include <stdexcept>

#include <exl/mixed.hpp>

void on_terminate()
{
    std::exit(1);
}

int main(int, char**)
{
    std::set_terminate(on_terminate);
    exl::mixed<int, std::runtime_error, std::exception> m(std::runtime_error("boom"));
    // Should fail
    m.unwrap_exact<std::exception>();
    return 0;
}