// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <algorithm>
#include <functional>

#include <catch2/catch.hpp>

#include <exl/mixed.hpp>

#include "mock/ClassMock.hpp"

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

TEST_CASE("Type check test", "[mixed]")
{
    using Mixed = exl::mixed<std::runtime_error, int, char, std::logic_error>;

    SECTION("When type is exact equal")
    {
        Mixed m(399);
        REQUIRE(m.is<int>());
    }

    SECTION("When type is derived from")
    {
        Mixed m(std::runtime_error("hello"));
        REQUIRE(m.is<std::exception>());
    }
}

TEST_CASE("Type exact check test", "[mixed]")
{
    using Mixed = exl::mixed<std::runtime_error, int, char, std::exception>;

    SECTION("When type is exact equal int")
    {
        Mixed m(399);
        REQUIRE(m.is_exact<int>());
    }

    SECTION("When type is exact equal std::exception")
    {
        std::exception ex;
        Mixed m(std::move(ex));
        REQUIRE(m.is_exact<std::exception>());
    }

    SECTION("When type is derived from")
    {
        Mixed m(std::runtime_error("hello"));
        REQUIRE(!m.is_exact<std::exception>());
    }
}

TEST_CASE("Unwrap test", "[mixed]")
{
    using Mixed = exl::mixed<std::runtime_error, int, char, std::logic_error>;

    SECTION("When type is exact equal")
    {
        Mixed m(399);
        REQUIRE(m.unwrap<int>() == 399);
    }

    SECTION("When type is exact equal")
    {
        Mixed m(std::logic_error("Hello"));
        REQUIRE(std::string(m.unwrap<std::exception>().what()) == std::string("Hello"));
    }
}

TEST_CASE("Unwrap exact test", "[mixed]")
{
    using Mixed = exl::mixed<std::runtime_error, int, char, std::logic_error>;

    SECTION("When type is exact equal")
    {
        Mixed m(422);
        REQUIRE(m.unwrap_exact<int>() == 422);
    }

    SECTION("When type is exact equal on std::runtime_error")
    {
        Mixed m(std::runtime_error("hello"));
        REQUIRE(std::string(m.unwrap_exact<std::runtime_error>().what()) == std::string("hello"));
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
    using Mixed = exl::mixed<int, SecondClassMock, ClassMock>;
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

template <typename Mixed, typename MixedSubset>
static void generic_test_construct_from_subset()
{
    CallCounter calls;

    SECTION("Constructed with same mixed type")
    {
        MixedSubset m1(ClassMock(1, &calls));
        auto m1_tag = m1.template unwrap<ClassMock>().tag();

        SECTION("By copy")
        {
            Mixed m2(m1);

            SECTION("Is assigned value correct")
            {
                REQUIRE(m2.template is<ClassMock>());
                REQUIRE(m2.template unwrap<ClassMock>().original_tag() == 1);
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
                REQUIRE(m2.template is<ClassMock>());
                REQUIRE(m2.template unwrap<ClassMock>().original_tag() == 1);
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
        auto m1_tag = m1.template unwrap<ClassMock>().tag();

        SECTION("By copy")
        {
            Mixed m2(m1);

            SECTION("Is assigned value correct")
            {
                REQUIRE(m2.template is<ClassMock>());
                REQUIRE(m2.template unwrap<ClassMock>().original_tag() == 1);
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
                REQUIRE(m2.template is<ClassMock>());
                REQUIRE(m2.template unwrap<ClassMock>().original_tag() == 1);
            }

            SECTION("Is move constructed")
            {
                REQUIRE(calls.count(CallType::Construct, as_moved_tag(m1_tag)) == 1);
                REQUIRE(calls.count(CallType::Move, m1_tag) == 1);
            }
        }
    }
}

TEST_CASE("Mixed type construct from subset test", "[mixed]")
{
    using Mixed = exl::mixed<int, std::string, ClassMock, char>;
    using MixedSubset = exl::mixed<char, ClassMock, std::string>;

    generic_test_construct_from_subset<Mixed, MixedSubset>();
}

TEST_CASE("Mixed type construct from subset test when tail type", "[mixed]")
{
    using Mixed = exl::mixed<int, std::string, ClassMock, char>;
    using MixedSubset = exl::mixed<char, std::string, ClassMock>;

    generic_test_construct_from_subset<Mixed, MixedSubset>();
}

TEST_CASE("Mixed type construct from subset test when head type", "[mixed]")
{
    using Mixed = exl::mixed<int, std::string, ClassMock, char>;
    using MixedSubset = exl::mixed<ClassMock, char, std::string>;

    generic_test_construct_from_subset<Mixed, MixedSubset>();
}

template <typename Mixed, typename MixedSubset>
static void generic_test_assign_from_subset()
{
    CallCounter calls;

    SECTION("Copy-assigned correctly when variants are different")
    {
        Mixed m1(ClassMock(1, &calls));
        MixedSubset m2(SecondClassMock(2, &calls));

        auto m1_tag = m1.template unwrap<ClassMock>().tag();
        auto m2_tag = m2.template unwrap<SecondClassMock>().tag();

        m1 = m2;

        SECTION("New value is correct")
        {
            REQUIRE(m1.template is<SecondClassMock>());
            REQUIRE(m1.template unwrap<SecondClassMock>().original_tag() == 2);
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

        auto m1_tag = m1.template unwrap<ClassMock>().tag();
        auto m2_tag = m2.template unwrap<ClassMock>().tag();

        m1 = m2;

        SECTION("New value is correct")
        {
            REQUIRE(m1.template is<ClassMock>());
            REQUIRE(m1.template unwrap<ClassMock>().original_tag() == 2);
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

        auto m1_tag = m1.template unwrap<ClassMock>().tag();
        auto m2_tag = m2.template unwrap<SecondClassMock>().tag();

        m1 = std::move(m2);

        SECTION("New value is correct")
        {
            REQUIRE(m1.template is<SecondClassMock>());
            REQUIRE(m1.template unwrap<SecondClassMock>().original_tag() == 2);
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

        auto m1_tag = m1.template unwrap<ClassMock>().tag();
        auto m2_tag = m2.template unwrap<ClassMock>().tag();

        m1 = std::move(m2);

        SECTION("New value is correct")
        {
            REQUIRE(m1.template is<ClassMock>());
            REQUIRE(m1.template unwrap<ClassMock>().original_tag() == 2);
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

TEST_CASE("Mixed type assign from subset test", "[mixed]")
{
    using Mixed = exl::mixed<std::string, char, SecondClassMock, ClassMock, std::exception>;
    using MixedSubset = exl::mixed<std::string, SecondClassMock, ClassMock, char>;

    generic_test_assign_from_subset<Mixed, MixedSubset>();
}

TEST_CASE("Mixed type assign from subset test when head type", "[mixed]")
{
    using Mixed = exl::mixed<std::string, char, SecondClassMock, ClassMock, std::exception>;
    using MixedSubset = exl::mixed<ClassMock, char, SecondClassMock>;

    generic_test_assign_from_subset<Mixed, MixedSubset>();
}

TEST_CASE("Mixed type assign from subset test when tail type", "[mixed]")
{
    using Mixed = exl::mixed<std::string, char, SecondClassMock, ClassMock, std::exception>;
    using MixedSubset = exl::mixed<SecondClassMock, char, ClassMock>;

    generic_test_assign_from_subset<Mixed, MixedSubset>();
}

TEST_CASE("Mixed type emplace test", "[mixed]")
{
    using Mixed = exl::mixed<SecondClassMock, ClassMock, std::string>;

    CallCounter calls;

    Mixed m(ClassMock(1, &calls));
    auto m_tag = m.unwrap<ClassMock>().tag();

    m.emplace<SecondClassMock>(2, &calls);

    SECTION("Is old class destroyed")
    {
        REQUIRE(calls.count(CallType::Destroy, m_tag));
    }

    SECTION("Is new value constructed")
    {
        REQUIRE(calls.count(CallType::Construct, 2) == 1);
    }

    SECTION("Is mixed holds new value")
    {
        REQUIRE(m.is<SecondClassMock>());
        REQUIRE(m.unwrap<SecondClassMock>().original_tag() == 2);
    }

    SECTION("New value is not constructed from intermediate object")
    {
        REQUIRE(calls.count(CallType::Construct, as_copied_tag(2)) == 0);
        REQUIRE(calls.count(CallType::Construct, as_moved_tag(2)) == 0);
    }

    SECTION("New value is not assigned from intermediate object")
    {
        REQUIRE(calls.count(CallType::Assign, 1) == 0);
    }

}

TEST_CASE("Mixed type in-place construction test", "[mixed]")
{
    using Mixed = exl::mixed<SecondClassMock, ClassMock, std::string>;

    CallCounter calls;

    Mixed m(exl::in_place_type_t<ClassMock>(), 1, &calls);

    SECTION("Is new value constructed")
    {
        REQUIRE(calls.count(CallType::Construct, 1) == 1);
    }

    SECTION("New value is correct")
    {
        REQUIRE(m.is<ClassMock>());
        REQUIRE(m.unwrap<ClassMock>().original_tag() == 1);
        REQUIRE(m.unwrap<ClassMock>().tag() == 1);
    }

    SECTION("New value is not constructed from intermediate value")
    {
        REQUIRE(calls.count(CallType::Construct, as_copied_tag(1)) == 0);
        REQUIRE(calls.count(CallType::Construct, as_moved_tag(1)) == 0);
    }
}

TEST_CASE("Mixed type on() test")
{
    using Mixed = exl::mixed<ClassMock, SecondClassMock, std::string>;

    CallCounter counter;
    Mixed m(exl::in_place_type_t<SecondClassMock>(), 1, &counter);
    bool functor_called = false;

    SECTION("Triggers functor when same type")
    {
        m.on<ClassMock>(
                [&functor_called](ClassMock&)
                {
                    functor_called = true;
                }
        );

        REQUIRE(functor_called);
        REQUIRE(counter.count(CallType::Copy, 1) == 0);
        REQUIRE(counter.count(CallType::Move, 1) == 0);
    }

    SECTION("Not triggers when different type")
    {
        m.on<std::string>(
                [&functor_called](const std::string&)
                {
                    functor_called = true;
                }
        );

        REQUIRE(!functor_called);
    }
}

TEST_CASE("Mixed type on_exact() test")
{
    using Mixed = exl::mixed<ClassMock, SecondClassMock>;

    CallCounter counter;
    Mixed m(exl::in_place_type_t<SecondClassMock>(), 1, &counter);
    bool functor_called = false;

    SECTION("Triggers functor when same type")
    {
        m.on_exact<SecondClassMock>(
                [&functor_called](SecondClassMock&)
                {
                    functor_called = true;
                }
        );

        REQUIRE(functor_called);
        REQUIRE(counter.count(CallType::Copy, 1) == 0);
        REQUIRE(counter.count(CallType::Move, 1) == 0);
    }

    SECTION("Not triggers when different type")
    {
        m.on_exact<ClassMock>(
                [&functor_called](const ClassMock&)
                {
                    functor_called = true;
                }
        );

        REQUIRE(!functor_called);
    }
}

TEST_CASE("Map without otherwise test")
{
    using Mixed = exl::mixed<int, std::runtime_error, ClassMock, SecondClassMock>;

    Mixed m(0);

    int result = 0;
    int mockTag = 0;

    auto do_map = [&result, &mockTag, &m]() -> void
    {
        result = m.map<int>(
                exl::when<std::exception>([](const std::exception&)
                {
                    return 2;
                }),
                exl::when_exact<int>([](const int&)
                {
                    return 1;
                }),
                exl::when_exact<ClassMock>([&mockTag](const ClassMock& mock)
                {
                    mockTag = mock.original_tag();
                    return 3;
                }),
                exl::when<ClassMock>([&mockTag](const ClassMock& mock)
                {
                    mockTag = mock.original_tag();
                    return 4;
                })
        );
    };

    SECTION("On int")
    {
        m = int(42);
        do_map();
        REQUIRE(result == 1);
    }

    SECTION("On std::exception")
    {
        m = std::runtime_error("hi");
        do_map();
        REQUIRE(result == 2);
    }

    SECTION("On exact ClassMock")
    {
        m = ClassMock(1);
        do_map();
        REQUIRE(result == 3);
        REQUIRE(mockTag == 1);
    }

    SECTION("On derived from ClassMock")
    {
        m = SecondClassMock(2);
        do_map();
        REQUIRE(result == 4);
        REQUIRE(mockTag == 2);
    }
}

TEST_CASE("Map with otherwise test")
{
    using Mixed = exl::mixed<int, std::runtime_error, std::string, std::logic_error>;

    Mixed m(0);

    int result = 0;

    auto do_map = [&result, &m]() -> void
    {
        result = m.map<int>(
                exl::when_exact<std::logic_error>([](const std::logic_error&)
                {
                    return 4;
                }),
                exl::when_exact<int>([](const int&)
                {
                    return 1;
                }),
                exl::otherwise([]()
                {
                    return 42;
                })
        );
    };

    SECTION("On int")
    {
        m = int(42);
        do_map();
        REQUIRE(result == 1);
    }

    SECTION("On std::logic_error")
    {
        m = std::logic_error("hi");
        do_map();
        REQUIRE(result == 4);
    }

    SECTION("On otherwise when std::string")
    {
        m = std::string("hi");
        do_map();
        REQUIRE(result == 42);
    }

    SECTION("On otherwise when std::runtime_error")
    {
        m = std::runtime_error("hi");
        do_map();
        REQUIRE(result == 42);
    }
}

TEST_CASE("Map with void return type (match) test")
{
    using Mixed = exl::mixed<int, std::runtime_error, std::string, std::logic_error>;

    Mixed m(0);

    int result = 0;

    auto do_map = [&result, &m]() -> void
    {
        m.match(
                exl::when_exact<std::logic_error>([&result](const std::logic_error&)
                {
                    result = 4;
                }),
                exl::when_exact<int>([&result](const int&)
                {
                    result = 1;
                }),
                exl::otherwise([&result]()
                {
                    result = 42;
                })
        );
    };

    SECTION("On int")
    {
        m = int(42);
        do_map();
        REQUIRE(result == 1);
    }

    SECTION("On std::logic_error")
    {
        m = std::logic_error("hi");
        do_map();
        REQUIRE(result == 4);
    }

    SECTION("On otherwise when std::string")
    {
        m = std::string("hi");
        do_map();
        REQUIRE(result == 42);
    }
}

// with otherwise
