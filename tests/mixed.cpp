// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <algorithm>
#include <type_traits>
#include <functional>

#include <catch2/catch.hpp>

#include <exl/mixed.hpp>

using namespace exl::impl;

namespace
{
    class ClassMock
    {
    public:
        using Tag = size_t;

    public:
        explicit ClassMock(Tag instanceTag)
            : tag(instanceTag)
            , onCopy(nullptr)
            , onCopyAssign(nullptr)
            , onMove(nullptr)
            , onMoveAssign(nullptr)
            , onDestroy(nullptr) {}

        ClassMock(const ClassMock& rhs)
            : tag(rhs.tag)
            , onCopy(rhs.onCopy)
            , onCopyAssign(rhs.onCopyAssign)
            , onMove(rhs.onMove)
            , onMoveAssign(rhs.onMoveAssign)
            , onDestroy(rhs.onDestroy)
        {
            if (onCopy)
            {
                onCopy(rhs);
            }
        }

        ClassMock(ClassMock&& rhs)
            // mock class properties should be copied even when moving
            : tag(rhs.tag)
            , onCopy(rhs.onCopy)
            , onCopyAssign(rhs.onCopyAssign)
            , onMove(rhs.onMove)
            , onMoveAssign(rhs.onMoveAssign)
            , onDestroy(rhs.onDestroy)
        {
            if (onMove)
            {
                onMove(std::move(rhs));
            }
        }

        const ClassMock& operator=(const ClassMock& rhs)
        {
            tag = rhs.tag;
            onCopy = rhs.onCopy;
            onCopyAssign = rhs.onCopyAssign;
            onMove = rhs.onMove;
            onMoveAssign = rhs.onMoveAssign;
            onDestroy = rhs.onDestroy;

            if (onCopyAssign)
            {
                onCopyAssign(rhs);
            }

            return *this;
        }

        const ClassMock& operator=(ClassMock&& rhs)
        {
            tag = rhs.tag;
            onCopy = rhs.onCopy;
            onCopyAssign = rhs.onCopyAssign;
            onMove = rhs.onMove;
            onMoveAssign = rhs.onMoveAssign;
            onDestroy = rhs.onDestroy;

            if (onMoveAssign)
            {
                onMoveAssign(std::move(rhs));
            }

            return *this;
        }

        ~ClassMock()
        {
            if (onDestroy)
            {
                onDestroy(*this);
            }
        }

    public:
        Tag tag;

        std::function<void(const ClassMock&)> onCopy;
        std::function<void(const ClassMock&)> onCopyAssign;

        std::function<void(ClassMock&&)> onMove;
        std::function<void(ClassMock&&)> onMoveAssign;

        std::function<void(const ClassMock&)> onDestroy;
    };

    class SecondClassMock : public ClassMock {};
}

TEST_CASE("Mixed type construction test", "[mixed]")
{
    using Mixed = exl::mixed<int, char, std::string, ClassMock, SecondClassMock>;

    SECTION("Copy constructor called")
    {
        int copyCount = 0;
        int acquiredTag = 0;

        ClassMock mock(42);
        mock.onCopy = [&copyCount, &acquiredTag](const ClassMock& rhs)
        {
            acquiredTag = rhs.tag;
            ++copyCount;
        };

        Mixed m(mock);

        REQUIRE(copyCount == 1);
        REQUIRE(acquiredTag == 42);
    }

    SECTION("Move constructor called")
    {
        int moveCount = 0;
        int acquiredTag = 0;

        ClassMock mock(22);
        mock.onMove = [&moveCount, &acquiredTag](ClassMock&& rhs)
        {
            acquiredTag = rhs.tag;
            ++moveCount;
        };

        Mixed m(std::move(mock));

        REQUIRE(moveCount == 1);
        REQUIRE(acquiredTag == 22);
    }

    SECTION("Correct tag assigned on construction")
    {
        Mixed m(ClassMock(11));
        REQUIRE(m.is<ClassMock>() == true);

        Mixed m2(char(255));
        REQUIRE(m2.is<char>() == true);
    }
}

TEST_CASE("Midex type assignment operators test", "[mixed]")
{
    REQUIRE(false);
}

TEST_CASE("Mixed type correct destructor call test", "[mixed]")
{
    REQUIRE(false);
}

TEST_CASE("Mixed type in-place construction test", "[mixed]")
{
    REQUIRE(false);
}

TEST_CASE("Mixed type emplace test", "[mixed]")
{
    REQUIRE(false);
}
