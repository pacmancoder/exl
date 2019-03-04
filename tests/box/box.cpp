// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <exl/box.hpp>

#include <ClassMock.hpp>
#include <AllocObject.hpp>

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

TEST_CASE("exl::box non-const get() returns non-const reference")
{
    auto boxed = exl::box<ClassMock>::make(1);
    boxed.get().set_tag(42);
    REQUIRE(boxed.get().tag() == 42);
}

TEST_CASE("exl::box const get() returns const reference")
{
    auto boxed = exl::box<ClassMock>::make(99);
    REQUIRE(boxed.get().tag() == 99);
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
    REQUIRE(boxed2.get().tag() == 1);
}

TEST_CASE("exl::box constructs from pointer")
{
    auto boxed = exl::box<AlwaysGoodAllocObject>::from_ptr(
            new(std::nothrow) AlwaysGoodAllocObject()
    );

    REQUIRE(static_cast<void*>(&boxed.get()) == reinterpret_cast<void*>(1));
}

TEST_CASE("exl::box reset destroys old abject and places new pointer")
{
    CallCounter calls;

    auto boxed = exl::box<ClassMock>::make(1, &calls);
    boxed.reset(new ClassMock(2, &calls));

    REQUIRE(calls.count(CallType::Destroy, 1) == 1);
    REQUIRE(boxed.is_valid());
    REQUIRE(boxed.get().tag() == 2);
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

TEST_CASE("exl::box dereference operator returns mutable reference")
{
    auto boxed = exl::box<ClassMock>::make(1);

    (*boxed).set_tag(42);
    REQUIRE(boxed.get().tag() == 42);
}

TEST_CASE("exl::box arrow operator returns pointer to mutable value")
{
    auto boxed = exl::box<ClassMock>::make(1);

    boxed->set_tag(42);
    REQUIRE(boxed.get().tag() == 42);
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

// TODO:
//   - boxed_ptr::swap
//   - boxed_ptr::reset
//   - boxed_ptr::release
//   - cover exl::box with tests
//   - reorganize/refactor tests