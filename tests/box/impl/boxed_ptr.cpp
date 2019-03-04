// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)#include "deleter_function.hpp"

#include <catch2/catch.hpp>

#include <exl/details/box/deleter_object.hpp>
#include <exl/details/box/deleter_function.hpp>

#include <exl/impl/box/boxed_ptr.hpp>

#include <FakeDeletionObject.hpp>
#include <StubDeleter.hpp>
#include <deleter_function_stub.hpp>
#include <stub_class_deleter_hierarchy.hpp>

using namespace exl::test;

TEST_CASE("impl::boxed_ptr can be destroyed with base class deleter", "[impl::boxed_ptr]")
{
    StubDerivedClass value;

    {
        exl::impl::boxed_ptr<
                StubDerivedClass,
                exl::deleter_function<StubBaseClass, static_base_class_deleter>
        > boxed(&value);
    }

    REQUIRE(value.base_tag == 1);
}

TEST_CASE("impl::boxed_ptr destruction test", "[impl::boxed_ptr]")
{
    SECTION("RAII enforced with default deleter for scalars")
    {
        FakeDeletionObject obj;
        {
            exl::impl::boxed_ptr<FakeDeletionObject> i(&obj);
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
            exl::impl::boxed_ptr<FakeDeletionObject[]> i(objs_ptr);
        }

        REQUIRE(objs[0].tag == 42);
        REQUIRE(objs[1].tag == 43);
        REQUIRE(objs[2].tag == 44);
    }

    SECTION("Custom function deleter is called")
    {
        int value = 0;

        {
            exl::impl::boxed_ptr<int, exl::deleter_function<int, scalar_deleter_stub>> i(&value);
        }

        REQUIRE(value == 42);
    }

    SECTION("Custom object deleter is called")
    {
        int value = 0;
        const int new_value = 99;

        {
            exl::impl::boxed_ptr<
                    int,
                    exl::deleter_object<
                            int,
                            StubDeleter
                    >
            > i(&value, StubDeleter(&new_value));
        }

        REQUIRE(value == new_value);
    }
}

TEST_CASE("impl::boxed_ptr size test", "[impl::boxed_ptr]")
{
    using namespace exl::impl;

    int v = 0;

    SECTION("Size with deleter function equals to pointer size")
    {
        boxed_ptr<int, exl::deleter_function<int, scalar_deleter_stub>> i(&v);
        REQUIRE(sizeof(decltype(i)) == sizeof(int*));
    }

    SECTION("Size with deleter object is sizeof(deleter) + sizeof(pointer)")
    {
        boxed_ptr<int, exl::deleter_object<int, StubDeleter>> i(&v);
        REQUIRE(sizeof(decltype(i)) == (sizeof(StubDeleter) + sizeof(int*)));
    }
}

TEST_CASE(
        "impl::boxed_impl can be move-constructed with base class function deleters",
        "[impl::boxed_ptr]")
{
    using namespace exl::impl;

    StubDerivedClass value;

    {
        boxed_ptr<
                StubDerivedClass,
                exl::deleter_function<StubDerivedClass, static_derived_class_deleter>
        > derived_ptr(&value);

        boxed_ptr<
                StubBaseClass,
                exl::deleter_function<StubBaseClass, static_base_class_deleter>
        >(std::move(derived_ptr));
    }

    REQUIRE(value.base_tag == 1);
}

TEST_CASE(
        "impl::boxed_impl can be move-assigned with base class function deleters",
        "[impl::boxed_ptr]")
{
    using namespace exl::impl;

    StubDerivedClass derived_value;
    StubBaseClass base_value;
    {
        boxed_ptr<
                StubDerivedClass,
                exl::deleter_function<StubDerivedClass, static_derived_class_deleter>
        > derived_ptr(&derived_value);

        boxed_ptr<
                StubBaseClass,
                exl::deleter_function<StubBaseClass, static_base_class_deleter>
        > base_ptr(&base_value);

        base_ptr = std::move(derived_ptr);
    }

    // Deleted before assigment of new ptr
    REQUIRE(base_value.base_tag == 1);
    // Deleted on the scope exit with base deleter
    REQUIRE(derived_value.base_tag == 1);
}

TEST_CASE(
        "impl::boxed_impl can be move-constructed with base class object deleters",
        "[impl::boxed_ptr]")
{
    StubDerivedClass derived;

    {
        auto boxed = exl::impl::boxed_ptr<
                StubDerivedClass,
                exl::deleter_object<StubDerivedClass, StubDerivedClassDeleter>
        >(&derived, StubDerivedClassDeleter());

        auto boxed2 = exl::impl::boxed_ptr<
                StubDerivedClass,
                exl::deleter_object<StubBaseClass, StubBaseClassDeleter>
        >(std::move(boxed));

        auto* boxed_ptr = boxed.get();
        REQUIRE(boxed_ptr == nullptr);
    }

    REQUIRE(derived.base_tag == 1);
}

TEST_CASE(
        "impl::boxed_impl can be move-assigned with base class object deleters",
        "[impl::boxed_ptr]")
{
    StubDerivedClass derived;

    {
        auto boxed = exl::impl::boxed_ptr<
                StubDerivedClass,
                exl::deleter_object<StubDerivedClass, StubDerivedClassDeleter>
        >(&derived, StubDerivedClassDeleter());

        auto boxed2 = exl::impl::boxed_ptr<
                StubDerivedClass,
                exl::deleter_object<StubBaseClass, StubBaseClassDeleter>
        >(nullptr);

        boxed2 = std::move(boxed);

        auto* boxed_ptr = boxed.get();
        REQUIRE(boxed_ptr == nullptr);
    }

    REQUIRE(derived.base_tag == 1);
}

TEST_CASE(
        "impl::boxed_impl can be move-constructed with base class array function deleters",
        "[impl::boxed_ptr]")
{
    using namespace exl::impl;

    StubDerivedClass value[3];
    StubDerivedClass* value_ptr = value;

    {
        boxed_ptr<
                StubDerivedClass[],
                exl::deleter_function<StubDerivedClass[], static_derived_class_deleter>
        > derived_ptr(value_ptr);

        boxed_ptr<
                StubBaseClass[],
                exl::deleter_function<StubBaseClass[], static_base_class_deleter>
        > base_ptr(std::move(derived_ptr));
    }

    REQUIRE(value[0].base_tag == 1);
    REQUIRE(value[1].base_tag == 0);
}

TEST_CASE(
        "impl::boxed_impl can be move-assigned with base class array function deleters",
        "[impl::boxed_ptr]")
{
    using namespace exl::impl;

    StubDerivedClass value[3];
    StubDerivedClass* value_ptr = value;

    {
        boxed_ptr<
                StubDerivedClass[],
                exl::deleter_function<StubDerivedClass[], static_derived_class_deleter>
        > derived_ptr(value_ptr);

        boxed_ptr<
                StubBaseClass[],
                exl::deleter_function<StubBaseClass[], static_base_class_deleter>
        > base_ptr(nullptr);

        base_ptr = std::move(derived_ptr);
    }

    REQUIRE(value[0].base_tag == 1);
    REQUIRE(value[1].base_tag == 0);
}

TEST_CASE(
        "impl::boxed_impl can be move-constructed with base class array object deleters",
        "[impl::boxed_ptr]")
{
    StubDerivedClass value[3];
    StubDerivedClass* value_ptr = value;

    {
        auto derived_ptr = exl::impl::boxed_ptr<
                StubDerivedClass[],
                exl::deleter_object<StubDerivedClass[], StubDerivedClassDeleter>
        >(value_ptr, StubDerivedClassDeleter());

        auto base_ptr = exl::impl::boxed_ptr<
                StubDerivedClass[],
                exl::deleter_object<StubBaseClass[], StubBaseClassDeleter>
        >(std::move(derived_ptr));

        auto* boxed_ptr = derived_ptr.get();
        REQUIRE(boxed_ptr == nullptr);
    }

    REQUIRE(value[0].base_tag == 1);
    REQUIRE(value[1].base_tag == 0);
}

TEST_CASE(
        "impl::boxed_impl can be move-assigned with base class array object deleters",
        "[impl::boxed_ptr]")
{
    StubDerivedClass value[3];
    StubDerivedClass* value_ptr = value;

    {
        auto derived_ptr = exl::impl::boxed_ptr<
                StubDerivedClass[],
                exl::deleter_object<StubDerivedClass[], StubDerivedClassDeleter>
        >(value_ptr, StubDerivedClassDeleter());

        auto base_ptr = exl::impl::boxed_ptr<
                StubDerivedClass[],
                exl::deleter_object<StubBaseClass[], StubBaseClassDeleter>
        >(nullptr);

        base_ptr = std::move(derived_ptr);

        auto* boxed_ptr = derived_ptr.get();
        REQUIRE(boxed_ptr == nullptr);
    }

    REQUIRE(value[0].base_tag == 1);
    REQUIRE(value[1].base_tag == 0);
}
