// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <exl/box.hpp>

#include <termination_test.hpp>

#include <AllocObject.hpp>

using namespace exl::test;

void termination_test()
{
    auto boxed = exl::box<AlwaysBadAllocObject>::make();
    auto& unwrapped = boxed.get();

    (void) unwrapped;
}