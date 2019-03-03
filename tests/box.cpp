// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <catch2/catch.hpp>

#include <exl/box.hpp>

#include "utils/ClassMock.hpp"
#include "utils/AllocObject.hpp"

using namespace exl::test;


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

    class StubDeleter
    {
    public:
        StubDeleter()
                : value_(nullptr) {}

        StubDeleter(StubDeleter&& rhs) noexcept
                : value_(rhs.value_)
                , is_move_constructed_(true)
        {
            rhs.is_moved_ = true;
        }

        explicit StubDeleter(const int* value)
                : value_(value) {}

        StubDeleter& operator=(StubDeleter&& rhs) noexcept
        {
            value_ = rhs.value_;
            rhs.is_moved_ = true;
            is_move_assigned_ = true;
            return *this;
        }

        void operator()(int* obj)
        {
            if (!value_)
            {
                *obj = 399;
                return;
            }
            *obj = *value_;
        }

        bool is_moved() const { return is_moved_; }
        bool is_move_constructed() const { return is_move_constructed_; }
        bool is_move_assigned() const { return is_move_assigned_; }

    private:
        const int* value_;
        bool is_moved_ = false;
        bool is_move_constructed_ = false;
        bool is_move_assigned_ = false;
    };

    struct StubBaseClass
    {
        virtual ~StubBaseClass() = default;
        int base_tag = 0;
    };

    struct StubDerivedClass : public StubBaseClass {};

    struct StubBaseClassDeleter
    {
        virtual void operator()(StubBaseClass* p)
        {
            p->base_tag = 1;
        }
    };

    struct StubDerivedClassDeleter : public StubBaseClassDeleter
    {
        void operator()(StubBaseClass* p) override
        {
            p->base_tag = 2;
        }
    };

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

    class DeleterWithTag
    {
    public:
        DeleterWithTag(int newTag = 0)
                : tag(newTag) {}

        void operator()(StubBaseClass* p)
        {
            delete p;
        }

    public:
        int tag;
    };

    void static_base_class_deleter(StubBaseClass* p)
    {
        p->base_tag = 1;
    }

    void static_derived_class_deleter(StubDerivedClass* p)
    {
        p->base_tag = 2;
    }
}

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

TEST_CASE("exl::impl::static deleter test")
{
    SECTION("Scalar specialization")
    {
        exl::impl::deleter_function<int, scalar_deleter_stub> deleter;
        int value = 0;
        deleter.destroy(&value);
        REQUIRE(value == 42);
    }

    SECTION("Array specialization")
    {
        exl::impl::deleter_function<int[], array_deleter_stub> deleter;
        int values[3] = { 0 };
        int* values_ptr = values;
        deleter.destroy(values_ptr);
        REQUIRE(values[0] == 1);
        REQUIRE(values[1] == 2);
        REQUIRE(values[2] == 3);
    }

    SECTION("When function is static method")
    {
        exl::impl::deleter_function<int[], StubStaticDeleterStruct::destroy> deleter;
        int value = 0;
        deleter.destroy(&value);
        REQUIRE(value == 42);
    }
}

TEST_CASE("exl::impl_is_static_deleter test")
{
    REQUIRE(
            exl::impl::is_deleter_function<
                    exl::impl::deleter_function<int[], array_deleter_stub>
            >::value()
    );
    REQUIRE(!exl::impl::is_deleter_function<int>::value());
}

TEST_CASE("exl::impl::box_impl has size of pointer when deleter is static function")
{
    using namespace exl::impl;

    REQUIRE(sizeof(box_impl<int, deleter_function<int, scalar_deleter_stub>>) == sizeof(int*));
}

TEST_CASE("exl::impl::box_impl with static deleter test")
{
    using namespace exl::impl;

    SECTION("Can be destroyed with base class deleter")
    {
        StubDerivedClass value;

        {
            box_impl<
                    StubDerivedClass,
                    deleter_function<StubBaseClass, static_base_class_deleter>
            > boxed(&value);
        }

        REQUIRE(value.base_tag == 1);
    }
}

TEST_CASE("exl::impl::deleter_object test")
{
    using namespace exl::impl;

    SECTION("Has minimal size")
    {
        REQUIRE(sizeof(deleter_object<int, StubDeleter>) == sizeof(StubDeleter));
    }

    SECTION("Default constructs")
    {
        deleter_object<int, StubDeleter> deleter;
        int value = 0;
        deleter.destroy(&value);
        REQUIRE(value == 399);
    }

    SECTION("Move constructs")
    {
        const int new_value = 42;
        deleter_object<int, StubDeleter> deleter((StubDeleter(&new_value)));
        int value = 0;
        deleter.destroy(&value);
        REQUIRE(value == new_value);
    }

    SECTION("Move assigns")
    {
        const int new_value = 42;
        deleter_object<int, StubDeleter> deleter;
        deleter = deleter_object<int, StubDeleter>((StubDeleter(&new_value)));
        int value = 0;
        deleter.destroy(&value);
        REQUIRE(value == new_value);
    }

    SECTION("Move constructs from convertible deleter_object")
    {
        deleter_object<StubBaseClass, StubBaseClassDeleter> deleter(
                (deleter_object<StubDerivedClass, StubDerivedClassDeleter>())
        );

        StubDerivedClass v;

        // Derived object can be passed
        deleter.destroy(&v);

        // Derived deleter was called
        REQUIRE(v.base_tag == 1);
    }
}

TEST_CASE("exl::impl::box_impl destruction test")
{
    using namespace exl::impl;

    SECTION("RAII enforced with default deleter for scalars")
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

    SECTION("Custom static deleter is called")
    {
        int value = 0;

        {
            box_impl<int, deleter_function<int, scalar_deleter_stub>> i(&value);
        }

        REQUIRE(value == 42);
    }

    SECTION("Custom dynamic deleter is called")
    {
        int value = 0;
        const int new_value = 99;

        {
            box_impl<int, deleter_object<int, StubDeleter>> i(&value, StubDeleter(&new_value));
        }

        REQUIRE(value == new_value);
    }
}

TEST_CASE("exl::impl::box_impl properties test")
{
    using namespace exl::impl;

    int v = 0;

    SECTION("Size with static deleter equals to pointer size")
    {
        box_impl<int, deleter_function<int, scalar_deleter_stub>> i(&v);
        REQUIRE(sizeof(decltype(i)) == sizeof(int*));
    }

    SECTION("With dynamic deleter has minimal size")
    {
        box_impl<int, deleter_object<int, StubDeleter>> i(&v);
        REQUIRE(sizeof(decltype(i)) == (sizeof(StubDeleter) + sizeof(int*)));
    }
}

TEST_CASE("exl::impl::boxed_impl can be constructed with compatible function deleters")
{
    using namespace exl::impl;

    StubDerivedClass value;

    {
        box_impl<
                StubDerivedClass,
                deleter_function<StubDerivedClass, static_derived_class_deleter>
        > derived_ptr(&value);

        box_impl<
                StubBaseClass,
                deleter_function<StubBaseClass, static_base_class_deleter>
        >(std::move(derived_ptr));
    }

    REQUIRE(value.base_tag == 1);
}

TEST_CASE("exl::impl::boxed_impl can be assigned with compatible function deleters")
{
    using namespace exl::impl;

    StubDerivedClass derived_value;
    StubBaseClass base_value;
    {
        box_impl<
                StubDerivedClass,
                deleter_function<StubDerivedClass, static_derived_class_deleter>
        > derived_ptr(&derived_value);

        box_impl<
                StubBaseClass,
                deleter_function<StubBaseClass, static_base_class_deleter>
        > base_ptr(&base_value);

        base_ptr = std::move(derived_ptr);
    }

    // Deleted before assigment of new ptr
    REQUIRE(base_value.base_tag == 1);
    // Deleted on the scope exit with base deleter
    REQUIRE(derived_value.base_tag == 1);
}

TEST_CASE("exl::deleter_object can be obtained and changed")
{
    auto boxed = exl::impl::box_impl<
            StubBaseClass,
            exl::impl::deleter_object<StubBaseClass, DeleterWithTag>
    >(new StubBaseClass, DeleterWithTag(42));

    REQUIRE(boxed.get_deleter().tag == 42);

    boxed.set_deleter(DeleterWithTag(399));
    REQUIRE(boxed.get_deleter().tag == 399);
}

TEST_CASE("exl::impl::box_impl can be constructed with convertible dynamic deleter")
{
    StubDerivedClass derived;

    {
        auto boxed = exl::impl::box_impl<
                StubDerivedClass,
                exl::impl::deleter_object<StubDerivedClass, StubDerivedClassDeleter>
        >(&derived, StubDerivedClassDeleter());

        auto boxed2 = exl::impl::box_impl<
                StubDerivedClass,
                exl::impl::deleter_object<StubBaseClass, StubBaseClassDeleter>
        >(std::move(boxed));

        auto* boxed_ptr = boxed.get();
        REQUIRE(boxed_ptr == nullptr);
    }

    REQUIRE(derived.base_tag == 1);
}

TEST_CASE("exl::impl::box_impl can be assigned with convertible dynamic deleter")
{
    StubDerivedClass derived;

    {
        auto boxed = exl::impl::box_impl<
                StubDerivedClass,
                exl::impl::deleter_object<StubDerivedClass, StubDerivedClassDeleter>
        >(&derived, StubDerivedClassDeleter());

        auto boxed2 = exl::impl::box_impl<
                StubDerivedClass,
                exl::impl::deleter_object<StubBaseClass, StubBaseClassDeleter>
        >(nullptr);

        boxed2 = std::move(boxed);

        auto* boxed_ptr = boxed.get();
        REQUIRE(boxed_ptr == nullptr);
    }

    REQUIRE(derived.base_tag == 1);
}

TEST_CASE("exl::impl::boxed_impl can be constructed with compatible array function deleters")
{
    using namespace exl::impl;

    StubDerivedClass value[3];
    StubDerivedClass* value_ptr = value;

    {
        box_impl<
                StubDerivedClass[],
                deleter_function<StubDerivedClass[], static_derived_class_deleter>
        > derived_ptr(value_ptr);

        box_impl<
                StubBaseClass[],
                deleter_function<StubBaseClass[], static_base_class_deleter>
        > base_ptr(std::move(derived_ptr));
    }

    REQUIRE(value[0].base_tag == 1);
    REQUIRE(value[1].base_tag == 0);
}

TEST_CASE("exl::impl::boxed_impl can be assigned with compatible array function deleters")
{
    using namespace exl::impl;

    StubDerivedClass value[3];
    StubDerivedClass* value_ptr = value;

    {
        box_impl<
                StubDerivedClass[],
                deleter_function<StubDerivedClass[], static_derived_class_deleter>
        > derived_ptr(value_ptr);

        box_impl<
                StubBaseClass[],
                deleter_function<StubBaseClass[], static_base_class_deleter>
        > base_ptr(nullptr);

        base_ptr = std::move(derived_ptr);
    }

    REQUIRE(value[0].base_tag == 1);
    REQUIRE(value[1].base_tag == 0);
}

TEST_CASE("exl::impl::box_impl can be constructed with convertible array dynamic deleter")
{
    StubDerivedClass value[3];
    StubDerivedClass* value_ptr = value;

    {
        auto derived_ptr = exl::impl::box_impl<
                StubDerivedClass[],
                exl::impl::deleter_object<StubDerivedClass[], StubDerivedClassDeleter>
        >(value_ptr, StubDerivedClassDeleter());

        auto base_ptr = exl::impl::box_impl<
                StubDerivedClass[],
                exl::impl::deleter_object<StubBaseClass[], StubBaseClassDeleter>
        >(std::move(derived_ptr));

        auto* boxed_ptr = derived_ptr.get();
        REQUIRE(boxed_ptr == nullptr);
    }

    REQUIRE(value[0].base_tag == 1);
    REQUIRE(value[1].base_tag == 0);
}

TEST_CASE("exl::impl::box_impl can be assigned with convertible array dynamic deleter")
{
    StubDerivedClass value[3];
    StubDerivedClass* value_ptr = value;

    {
        auto derived_ptr = exl::impl::box_impl<
                StubDerivedClass[],
                exl::impl::deleter_object<StubDerivedClass[], StubDerivedClassDeleter>
        >(value_ptr, StubDerivedClassDeleter());

        auto base_ptr = exl::impl::box_impl<
                StubDerivedClass[],
                exl::impl::deleter_object<StubBaseClass[], StubBaseClassDeleter>
        >(nullptr);

        base_ptr = std::move(derived_ptr);

        auto* boxed_ptr = derived_ptr.get();
        REQUIRE(boxed_ptr == nullptr);
    }

    REQUIRE(value[0].base_tag == 1);
    REQUIRE(value[1].base_tag == 0);
}