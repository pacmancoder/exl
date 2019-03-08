// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <map>

#include <catch2/catch.hpp>

#include <exl/box.hpp>

#include <ClassMock.hpp>
#include <AllocObject.hpp>
#include <deleter_function_stub.hpp>
#include <StubDeleter.hpp>

using namespace exl::test;

TEST_CASE("exl::box creates object by forwarding", "[box]")
{
    CallCounter calls;
    auto boxed = exl::box<ClassMock>::make(ClassMock(1, &calls));

    REQUIRE(calls.count(CallType::Construct, as_moved_tag(1)) == 1);
    REQUIRE(calls.count(CallType::Move, 1) == 1);

    (void) boxed;
}

TEST_CASE("exl::box is_valid value is correct", "[box]")
{
    SECTION("For successful allocation")
    {
        auto boxed = exl::box<AlwaysGoodAllocObject>::make();
        REQUIRE(boxed.is_valid());
    }

    SECTION("For failed allocation")
    {
        auto boxed = exl::box<AlwaysBadAllocObject>::make();
        REQUIRE(!boxed.is_valid());
    }
}

TEST_CASE("exl::box calls object destructor on destroy", "[box]")
{
    CallCounter calls;
    {
        auto boxed = exl::box<ClassMock>::make(1, &calls);

        (void) boxed;
    }

    REQUIRE(calls.count(CallType::Destroy, 1) == 1);
}

TEST_CASE("exl::box move-constructs from other box")
{
    CallCounter calls;
    auto boxed1 = exl::box<ClassMock>::make(1, &calls);
    auto boxed2 = std::move(boxed1);

    // No copy or move on the contained object
    REQUIRE(calls.count(CallType::Move, 1) == 0);
    REQUIRE(calls.count(CallType::Copy, 1) == 0);

    // Old box is not valid, new - valid
    REQUIRE(!boxed1.is_valid());
    REQUIRE(boxed2.is_valid());

    // Check that box2 contains same object as we created previously
    REQUIRE(boxed2->tag() == 1);
}

TEST_CASE("exl::box constructs from pointer")
{
    auto boxed = exl::box<AlwaysGoodAllocObject>::from_ptr(
            new(std::nothrow) AlwaysGoodAllocObject()
    );

    REQUIRE(static_cast<void*>(&*boxed) == reinterpret_cast<void*>(1));
}

TEST_CASE("exl::box reset destroys old abject and places new pointer")
{
    CallCounter calls;

    auto boxed = exl::box<ClassMock>::make(1, &calls);
    boxed.reset(new ClassMock(2, &calls));

    REQUIRE(calls.count(CallType::Destroy, 1) == 1);
    REQUIRE(boxed.is_valid());
    REQUIRE(boxed->tag() == 2);
}

TEST_CASE("exl::box release releases ownership")
{
    CallCounter calls;

    ClassMock* mock = nullptr;
    {
        auto boxed = exl::box<ClassMock>::make(1, &calls);
        mock = boxed.release();
    }

    REQUIRE(mock != nullptr);
    REQUIRE(mock->tag() == 1);

    REQUIRE(calls.count(CallType::Destroy, 1) == 0);
}

TEST_CASE("exl::box dereference operator returns non-const reference when box is non-const")
{
    auto boxed = exl::box<ClassMock>::make(1);
    REQUIRE(!boxed->is_called_as_const());
}

TEST_CASE("exl::box dereference operator returns const reference when box is const")
{
    const auto boxed = exl::box<ClassMock>::make(1);
    REQUIRE(boxed->is_called_as_const());
}

TEST_CASE("exl::box arrow operator returns non-const reference when box is non-const")
{
    auto boxed = exl::box<ClassMock>::make(1);
    REQUIRE(!boxed->is_called_as_const());
}

TEST_CASE("exl::box arrow operator returns const reference when box is const")
{
    const auto boxed = exl::box<ClassMock>::make(1);
    REQUIRE(boxed->is_called_as_const());
}

TEST_CASE("exl::box bool operator returns is_valid")
{
    SECTION("For successful allocation")
    {
        auto boxed = exl::box<AlwaysGoodAllocObject>::make();
        REQUIRE(boxed);
    }

    SECTION("For failed allocation")
    {
        auto boxed = exl::box<AlwaysBadAllocObject>::make();
        REQUIRE(!boxed);
    }
}

TEST_CASE("exl::box swap swaps pointers")
{
    auto boxed1 = exl::box<ClassMock>::make(1);
    auto boxed2 = exl::box<ClassMock>::make(2);

    std::swap(boxed1, boxed2);

    REQUIRE(boxed1->tag() == 2);
    REQUIRE(boxed2->tag() == 1);
}

TEST_CASE("exl::box assignment operator removes old value and assigns new pointer")
{
    CallCounter calls;

    auto boxed1 = exl::box<ClassMock>::make(1, &calls);
    auto boxed2 = exl::box<ClassMock>::make(2, &calls);
    boxed1 = std::move(boxed2);

    REQUIRE(calls.count(CallType::Destroy, 1) == 1);

    REQUIRE(calls.count(CallType::Copy, 2) == 0);
    REQUIRE(calls.count(CallType::Move, 2) == 0);

    REQUIRE(boxed1.is_valid());
    REQUIRE(boxed1->tag() == 2);

    REQUIRE(!boxed2.is_valid());
}

TEST_CASE("exl::box works with custom deleter function when scalar")
{
    int value = 0;

    {
        auto boxed = exl::box<
                int,
                exl::deleter_function<int, scalar_deleter_stub>
        >::from_ptr(&value);
    }

    REQUIRE(value == 42);
}

TEST_CASE("exl::box works with custom deleter function when array")
{
    int value[] = { 0, 0, 0 };

    {
        auto boxed = exl::box<
                int[],
                exl::deleter_function<int[], array_deleter_stub>
        >::from_ptr(value);
    }

    REQUIRE(value[0] == 1);
    REQUIRE(value[1] == 2);
    REQUIRE(value[2] == 3);
}

TEST_CASE("exl::box index operator returned non-const reference when box is non-const")
{
    auto boxed = exl::box<ClassMock[]>::make_array<5>();
    REQUIRE(!boxed[0].is_called_as_const());
}

TEST_CASE("exl::box index operator returned const reference when box is const")
{
    const auto boxed = exl::box<ClassMock[]>::make_array<5>();
    REQUIRE(boxed[1].is_called_as_const());
}

TEST_CASE("exl::box works with custom deleter object when scalar")
{
    int value = 0;

    {
        auto boxed = exl::box<
                int,
                exl::deleter_object<int, StubDeleter>
        >::from_ptr(&value);
    }

    REQUIRE(value == 399);
}

TEST_CASE("exl::box works with custom deleter object when array")
{
    int value[] = { 0, 0 };

    {
        auto boxed = exl::box<
                int[],
                exl::deleter_object<int[], StubDeleter>
        >::from_ptr(value);
    }

    REQUIRE(value[0] == 399);
    REQUIRE(value[1] == 0);
}

TEST_CASE("exl::box map perform value mapping on const box")
{
    SECTION("When valid test")
    {
        const auto valid_boxed = exl::box<int>::make(5);

        auto mapped = valid_boxed.map<char>(
                exl::when_valid(
                        [](const int& value)
                        {
                            return value * 5;
                        }
                ),
                exl::otherwise(
                        []()
                        {
                            return 0;
                        }
                )
        );

        REQUIRE(mapped == 25);
    }

    SECTION("When invalid test")
    {
        const auto valid_boxed = exl::box<int>::from_ptr(nullptr);

        auto mapped = valid_boxed.map<char>(
                exl::when_valid(
                        [](const int& value)
                        {
                            return value * 5;
                        }
                ),
                exl::otherwise(
                        []()
                        {
                            return 0;
                        }
                )
        );

        REQUIRE(mapped == 0);
    }
}

TEST_CASE("exl::box map perform value mapping on non-const box")
{
    SECTION("When valid test")
    {
        auto valid_boxed = exl::box<std::string>::make("hello");

        auto mapped = valid_boxed.map<std::string>(
                exl::when_valid(
                        [](std::string& value)
                        {
                            value.append("!");
                            return value + "!!";
                        }
                ),
                exl::otherwise(
                        []()
                        {
                            return "hi";
                        }
                )
        );

        REQUIRE(mapped == "hello!!!");
        REQUIRE(*valid_boxed == "hello!");
    }

    SECTION("When invalid test")
    {
        auto valid_boxed = exl::box<int>::from_ptr(nullptr);

        auto mapped = valid_boxed.map<char>(
                exl::when_valid(
                        [](int& value)
                        {
                            ++value;
                            return value * 5;
                        }
                ),
                exl::otherwise(
                        []()
                        {
                            return 0;
                        }
                )
        );

        REQUIRE(mapped == 0);
    }
}

TEST_CASE("exl::box match perform value matching on const box")
{
    const auto boxed = exl::box<int>::make(5);

    int found_value = 0;
    boxed.match(
            exl::when_valid([&found_value](const int& value) { found_value = value; }),
            exl::otherwise([]() {})
    );

    REQUIRE(found_value == 5);
}

TEST_CASE("exl::box match perform value matching on non-const box")
{
    auto boxed = exl::box<int>::make(5);

    boxed.match(
            exl::when_valid([](int& value) { value = 6; }),
            exl::otherwise([]() {})
    );

    REQUIRE(*boxed == 6);
}

TEST_CASE("exl::box on_valid test when box is const")
{
    const auto boxed = exl::box<int>::make(5);
    int value = 0;
    boxed.on_valid([&value](const int& retrieved) { value = retrieved; });

    REQUIRE(value == 5);
}

TEST_CASE("exl::box on_valid test when box is non-const")
{
    auto boxed = exl::box<int>::make(5);
    boxed.on_valid([](int& value) { value = 6; });

    REQUIRE(*boxed == 6);
}

TEST_CASE("exl::box on_invalid test when box is const")
{
    const auto boxed = exl::box<int>::from_ptr(nullptr);
    int value = 0;
    boxed.on_invalid([&value]() { value = 5; });

    REQUIRE(value == 5);
}

TEST_CASE("exl::box on_invalid test when box is non-const")
{
    auto boxed = exl::box<int>::from_ptr(nullptr);
    int value = 0;
    boxed.on_invalid([&value]() { value = 5; });

    REQUIRE(value == 5);
}

TEST_CASE("exl::box can be created as array with dynamic size")
{
    auto boxed = exl::box<int[]>::make_array(3);
    boxed[0] = 1;
    boxed[1] = 2;
    boxed[2] = 3;

    REQUIRE(boxed[0] == 1);
    REQUIRE(boxed[1] == 2);
    REQUIRE(boxed[2] == 3);
}

TEST_CASE("exl::box can be created as array with static size")
{
    auto boxed = exl::box<int[]>::make_array<3>();
    boxed[0] = 1;
    boxed[1] = 2;
    boxed[2] = 3;

    REQUIRE(boxed[0] == 1);
    REQUIRE(boxed[1] == 2);
    REQUIRE(boxed[2] == 3);
}