// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <exl/box.hpp>

#include "utils/ClassMock.hpp"
#include "utils/AllocObject.hpp"

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

namespace
{
    void scalar_deleter_stub(int* p)
    {
        *p = 42;
    }

    void array_deleter_stub(int* values)
    {
        values[0] = 1;
        values[1] = 2;
        values[2] = 3;
    }

    struct StubStaticDeleterStruct
    {
        static void destroy(int* p)
        {
            scalar_deleter_stub(p);
        }
    };
}

TEST_CASE("exl::impl::static deleter test")
{
    SECTION("Scalar specialization")
    {
        exl::impl::static_deleter<int, scalar_deleter_stub> deleter;
        int value = 0;
        deleter.destroy(&value);
        REQUIRE(value == 42);
    }

    SECTION("Array specialization")
    {
        exl::impl::static_deleter<int[], array_deleter_stub> deleter;
        int values[3] = { 0 };
        int* values_ptr = values;
        deleter.destroy(values_ptr);
        REQUIRE(values[0] == 1);
        REQUIRE(values[1] == 2);
        REQUIRE(values[2] == 3);
    }

    SECTION("When function is static method")
    {
        exl::impl::static_deleter<int[], StubStaticDeleterStruct::destroy> deleter;
        int value = 0;
        deleter.destroy(&value);
        REQUIRE(value == 42);
    }
}

TEST_CASE("exl::impl_is_static_deleter test")
{
    REQUIRE(
            exl::impl::is_static_deleter<
                    exl::impl::static_deleter<int[], array_deleter_stub>
            >::value()
    );
    REQUIRE(!exl::impl::is_static_deleter<int>::value());
}

TEST_CASE("exl::impl::box_impl has size of pointer when deleter is static function")
{
    using namespace exl::impl;

    REQUIRE(sizeof(box_impl<int, static_deleter<int, scalar_deleter_stub>>) == sizeof(int*));
}

namespace
{
    class StubDeleter
    {
    public:
        static const int default_value = 399;

    public:
        StubDeleter()
                : value_(&default_value) {}

        StubDeleter(StubDeleter&& rhs) noexcept
                : value_(rhs.value_) {}

        explicit StubDeleter(const int* value)
                : value_(value) {}

        StubDeleter& operator=(StubDeleter&& rhs) noexcept
        {
            value_ = rhs.value_;
            return *this;
        }

        void operator()(int* obj)
        {
            *obj = *value_;
        }

    private:
        const int* value_;
    };
}

TEST_CASE("exl::impl_dynamic_deleter_impl test")
{
    using namespace exl::impl;

    SECTION("Has minimal size")
    {
        REQUIRE(sizeof(dynamic_deleter<int, StubDeleter>) == sizeof(StubDeleter));
    }

    SECTION("Default constructs")
    {
        dynamic_deleter<int, StubDeleter> deleter;
        int value = 0;
        deleter.destroy(&value);
        REQUIRE(value == 399);
    }

    SECTION("Move constructs")
    {
        const int new_value = 42;
        dynamic_deleter<int, StubDeleter> deleter((StubDeleter(&new_value)));
        int value = 0;
        deleter.destroy(&value);
        REQUIRE(value == new_value);
    }

    SECTION("Move assigns")
    {
        const int new_value = 42;
        dynamic_deleter<int, StubDeleter> deleter;
        deleter = StubDeleter(&new_value);
        int value = 0;
        deleter.destroy(&value);
        REQUIRE(value == new_value);
    }
}

namespace
{
    class FakeDeletionObject
    {
    public:
        static void operator delete[](void* obj)
        {
            reinterpret_cast<FakeDeletionObject*>(obj)[0].tag = 42;
            reinterpret_cast<FakeDeletionObject*>(obj)[1].tag = 43;
            reinterpret_cast<FakeDeletionObject*>(obj)[2].tag = 44;
        }

        static void operator delete(void* obj)
        {
            reinterpret_cast<FakeDeletionObject*>(obj)->tag = 42;
        }

        int tag = 0;
    };
}

TEST_CASE("exl::impl::box_impl test")
{
    using namespace exl::impl;

    SECTION("RAII enforced with  default deleter for scalars")
    {
        FakeDeletionObject obj;
        {
            box_impl<FakeDeletionObject> i(&obj);
        }
        REQUIRE(obj.tag == 42);
    }

    SECTION("RAII enforced with default deleter for arrays")
    {
        FakeDeletionObject objs[3];
        FakeDeletionObject* objs_ptr = objs;

        objs[0].tag = 1;
        objs[1].tag = 2;
        objs[2].tag = 3;

        {
            box_impl<FakeDeletionObject[]> i(objs_ptr);
        }

        REQUIRE(objs[0].tag == 42);
        REQUIRE(objs[1].tag == 43);
        REQUIRE(objs[2].tag == 44);
    }
}

/* TODO:
 * - box_impl with custom static deleter
 * - box_impl with dynamic deleter
 * - box_impl create/move/assign
 *     - to scalar
 *     - to array
 *     - to subset with deleted for based type
 * - static sized arrays!
 */