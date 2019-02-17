// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <cstdlib>

#include <exl/mixed.hpp>

void on_terminate()
{
    std::exit(1);
}

int main (int, char**)
{
    std::set_terminate(on_terminate);
    exl::mixed<char, int> m(422);
    // Should fail
    m.unwrap<char>();
    return 0;
}