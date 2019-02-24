// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdlib>
#include <stdexcept>

void on_terminate()
{
    std::exit(1);
}

void termination_test();

int main(int, char**)
{
    std::set_terminate(on_terminate);
    termination_test();
    return 0;
}