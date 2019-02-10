// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <algorithm>
#include <functional>

#include <catch2/catch.hpp>

#include <exl/mixed.hpp>

#include "class_mock.h"

using namespace exl::impl;
using namespace exl::mock;

TEST_CASE("Mixed type construction test", "[mixed]")
{
    using Mixed = exl::mixed<int, char, std::string, ClassMock, SecondClassMock>;
    CallCounter calls;
    ClassMock mock(1, &calls);

    SECTION("Copy construction")
    {
        Mixed m(mock);

        SECTION("Was not moved")
        {
            REQUIRE(calls.count(CallType::Move, mock.tag()) == 0);
        }

        SECTION("Was copy constructed")
        {
            REQUIRE(calls.count(CallType::Copy, mock.tag()) == 1);
            REQUIRE(calls.count(CallType::Construct, as_copied_tag(mock.tag())) == 1);
        }

        SECTION("Has correct type")
        {
            REQUIRE(m.is<ClassMock>());
        }
    }

    SECTION("Move construction")
    {
        Mixed m(std::move(mock));

        SECTION("Was not copied")
        {
            REQUIRE(calls.count(CallType::Copy, mock.tag()) == 0);
        }

        SECTION("Was move constructed")
        {
            REQUIRE(calls.count(CallType::Move, mock.tag()) == 1);
            REQUIRE(calls.count(CallType::Construct, as_moved_tag(mock.tag())) == 1);
        }

        SECTION("Has correct type")
        {
            REQUIRE(m.is<ClassMock>());
        }
    }
}

TEST_CASE("Mixed type correct destructor call test", "[mixed]")
{
    using Mixed = exl::mixed<int, ClassMock, SecondClassMock>;
    CallCounter calls;
    ClassMock mock(1, &calls);

    SECTION("Mock is destroyed")
    {
        {
            Mixed m(mock);
        }

        REQUIRE(calls.count(CallType::Destroy, as_copied_tag(mock.tag())) == 1);
    }
}

TEST_CASE("Mixed type assignment operators test", "[mixed]")
{
    using Mixed = exl::mixed<int, ClassMock, SecondClassMock>;
    CallCounter calls;

    Mixed m1(ClassMock(1, &calls));
    auto m1_tag = m1.unwrap<ClassMock>().tag();

    SECTION("Assign to different type")
    {
        SECTION("Copy assign")
        {
            SecondClassMock mockForCopy(2, &calls);
            m1 = mockForCopy;

            SECTION("Is copy constructed")
            {
                REQUIRE(calls.count(CallType::Construct, as_copied_tag(2)) == 1);
                REQUIRE(calls.count(CallType::Copy, 2) == 1);
            }

            SECTION("Is previous value deleted")
            {
                REQUIRE(calls.count(CallType::Destroy, m1_tag) == 1);
            }
        }

        SECTION("Move assign")
        {
            m1 = SecondClassMock(2, &calls);

            SECTION("Is move constructed")
            {
                REQUIRE(calls.count(CallType::Construct, as_moved_tag(2)) == 1);
                REQUIRE(calls.count(CallType::Move, 2) == 1);
            }

            SECTION("Is previous value deleted")
            {
                REQUIRE(calls.count(CallType::Destroy, m1_tag) == 1);
            }
        }
    }

    SECTION("Assign to same type")
    {
        SECTION("Copy assign")
        {
            ClassMock mockForCopy(2, &calls);
            m1 = mockForCopy;

            SECTION("Is not constructed")
            {
                REQUIRE(calls.count(CallType::Construct, as_copied_tag(2)) == 0);
            }

            SECTION("Is previous value alive")
            {
                REQUIRE(calls.count(CallType::Destroy, m1_tag) == 0);
            }

            SECTION("Is copy assigned")
            {
                REQUIRE(calls.count(CallType::Assign, m1_tag) == 1);
                REQUIRE(calls.count(CallType::Copy, 2) == 1);
            }

            SECTION("Is assigned value correct")
            {
                REQUIRE(m1.is<ClassMock>());
                REQUIRE(m1.unwrap<ClassMock>().original_tag() == 2);
            }
        }

        SECTION("Move assign")
        {
            m1 = ClassMock(2, &calls);

            SECTION("Is not constructed")
            {
                REQUIRE(calls.count(CallType::Construct, as_moved_tag(2)) == 0);
            }

            SECTION("Is previous value alive")
            {
                REQUIRE(calls.count(CallType::Destroy, m1_tag) == 0);
            }

            SECTION("Is move assigned")
            {
                REQUIRE(calls.count(CallType::Assign, m1_tag) == 1);
                REQUIRE(calls.count(CallType::Move, 2) == 1);
            }

            SECTION("Is assigned value correct")
            {
                REQUIRE(m1.is<ClassMock>());
                REQUIRE(m1.unwrap<ClassMock>().original_tag() == 2);
            }
        }
    }
}

TEST_CASE("Mixed type construct from another mixed test", "[mixed]")
{
    using Mixed = exl::mixed<int, std::string, ClassMock, char>;
    using MixedSubset = exl::mixed<ClassMock, char, std::string>;

    CallCounter calls;

    SECTION("Constructed with same mixed type")
    {
        MixedSubset m1(ClassMock(1, &calls));
        auto m1_tag = m1.unwrap<ClassMock>().tag();

        SECTION("By copy")
        {
            Mixed m2(m1);

            SECTION("Is assigned value correct")
            {
                REQUIRE(m2.is<ClassMock>());
                REQUIRE(m2.unwrap<ClassMock>().original_tag() == 1);
            }

            SECTION("Is copy constructed")
            {
                REQUIRE(calls.count(CallType::Construct, as_copied_tag(m1_tag)) == 1);
                REQUIRE(calls.count(CallType::Copy, m1_tag) == 1);
            }
        }

        SECTION("By move")
        {
            Mixed m2(std::move(m1));

            SECTION("Is assigned value correct")
            {
                REQUIRE(m2.is<ClassMock>());
                REQUIRE(m2.unwrap<ClassMock>().original_tag() == 1);
            }

            SECTION("Is move constructed")
            {
                REQUIRE(calls.count(CallType::Construct, as_moved_tag(m1_tag)) == 1);
                REQUIRE(calls.count(CallType::Move, m1_tag) == 1);
            }
        }
    }

    SECTION("Constructed with different mixed type")
    {
        Mixed m1(ClassMock(1, &calls));
        auto m1_tag = m1.unwrap<ClassMock>().tag();

        SECTION("By copy")
        {
            Mixed m2(m1);

            SECTION("Is assigned value correct")
            {
                REQUIRE(m2.is<ClassMock>());
                REQUIRE(m2.unwrap<ClassMock>().original_tag() == 1);
            }

            SECTION("Is copy constructed")
            {
                REQUIRE(calls.count(CallType::Construct, as_copied_tag(m1_tag)) == 1);
                REQUIRE(calls.count(CallType::Copy, m1_tag) == 1);
            }
        }

        SECTION("By move")
        {
            Mixed m2(std::move(m1));

            SECTION("Is assigned value correct")
            {
                REQUIRE(m2.is<ClassMock>());
                REQUIRE(m2.unwrap<ClassMock>().original_tag() == 1);
            }

            SECTION("Is move constructed")
            {
                REQUIRE(calls.count(CallType::Construct, as_moved_tag(m1_tag)) == 1);
                REQUIRE(calls.count(CallType::Move, m1_tag) == 1);
            }
        }
    }
}

TEST_CASE("Mixed type assign from another mixed test", "[mixed]")
{
    using Mixed = exl::mixed<std::string, char, SecondClassMock, ClassMock>;
    using MixedSubset = exl::mixed<ClassMock, SecondClassMock>;

    CallCounter calls;

    SECTION("Copy-assigned correctly when variants are different")
    {
        Mixed m1(ClassMock(1, &calls));
        MixedSubset m2(SecondClassMock(2, &calls));

        auto m1_tag = m1.unwrap<ClassMock>().tag();
        auto m2_tag = m2.unwrap<SecondClassMock>().tag();

        m1 = m2;

        SECTION("New value is correct")
        {
            REQUIRE(m1.is<SecondClassMock>());
            REQUIRE(m1.unwrap<SecondClassMock>().original_tag() == 2);
        }

        SECTION("Assignment operator wasn't called")
        {
            REQUIRE(calls.count(CallType::Assign, m1_tag) == 0);
        }

        SECTION("Old value was destroyed")
        {
            REQUIRE(calls.count(CallType::Destroy, m1_tag) == 1);
        }

        SECTION("Copy constructor was called")
        {
            REQUIRE(calls.count(CallType::Construct, as_copied_tag(m2_tag)) == 1);
            REQUIRE(calls.count(CallType::Copy, m2_tag) == 1);
        }
    }

    SECTION("Copy-assigned correctly when variants are the same")
    {
        Mixed m1(ClassMock(1, &calls));
        MixedSubset m2(ClassMock(2, &calls));

        auto m1_tag = m1.unwrap<ClassMock>().tag();
        auto m2_tag = m2.unwrap<ClassMock>().tag();

        m1 = m2;

        SECTION("New value is correct")
        {
            REQUIRE(m1.is<ClassMock>());
            REQUIRE(m1.unwrap<ClassMock>().original_tag() == 2);
        }

        SECTION("Old value wasn't destroyed")
        {
            REQUIRE(calls.count(CallType::Destroy, m1_tag) == 0);
        }

        SECTION("New value wasn't constructed")
        {
            REQUIRE(calls.count(CallType::Construct, as_copied_tag(m2_tag)) == 0);
        }

        SECTION("Copy assignment was called")
        {
            REQUIRE(calls.count(CallType::Assign, m1_tag) == 1);
            REQUIRE(calls.count(CallType::Copy, m2_tag) == 1);
        }
    }

    SECTION("Move-assigned correctly when variants are different")
    {
        Mixed m1(ClassMock(1, &calls));
        MixedSubset m2(SecondClassMock(2, &calls));

        auto m1_tag = m1.unwrap<ClassMock>().tag();
        auto m2_tag = m2.unwrap<SecondClassMock>().tag();

        m1 = std::move(m2);

        SECTION("New value is correct")
        {
            REQUIRE(m1.is<SecondClassMock>());
            REQUIRE(m1.unwrap<SecondClassMock>().original_tag() == 2);
        }

        SECTION("Assignment operator wasn't called")
        {
            REQUIRE(calls.count(CallType::Assign, m1_tag) == 0);
        }

        SECTION("Old value was destroyed")
        {
            REQUIRE(calls.count(CallType::Destroy, m1_tag) == 1);
        }

        SECTION("Move constructor was called")
        {
            REQUIRE(calls.count(CallType::Construct, as_moved_tag(m2_tag)) == 1);
            REQUIRE(calls.count(CallType::Move, m2_tag) == 1);
        }
    }

    SECTION("Move-assigned correctly when variants are the same")
    {
        Mixed m1(ClassMock(1, &calls));
        MixedSubset m2(ClassMock(2, &calls));

        auto m1_tag = m1.unwrap<ClassMock>().tag();
        auto m2_tag = m2.unwrap<ClassMock>().tag();

        m1 = std::move(m2);

        SECTION("New value is correct")
        {
            REQUIRE(m1.is<ClassMock>());
            REQUIRE(m1.unwrap<ClassMock>().original_tag() == 2);
        }

        SECTION("Old value wasn't destroyed")
        {
            REQUIRE(calls.count(CallType::Destroy, m1_tag) == 0);
        }

        SECTION("New value wasn't constructed")
        {
            REQUIRE(calls.count(CallType::Construct, as_moved_tag(m2_tag)) == 0);
        }

        SECTION("Move assignment was called")
        {
            REQUIRE(calls.count(CallType::Assign, m1_tag) == 1);
            REQUIRE(calls.count(CallType::Move, m2_tag) == 1);
        }
    }
}

TEST_CASE("Mixed type in-place construction test", "[mixed][.]") {}

TEST_CASE("Mixed type emplace test", "[mixed][.]") {}
