// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <string>
#include <algorithm>
#include <type_traits>
#include <functional>
#include <map>
#include <utility>

#include <catch2/catch.hpp>

#include <exl/mixed.hpp>

using namespace exl::impl;

namespace
{
    using Tag = int;

    Tag AsCopiedTag(Tag tag)
    {
        if (tag < 0)
        {
            tag = -tag;
        }

        return tag * 16;
    }

    Tag AsMovedTag(Tag tag)
    {
        if (tag > 0)
        {
            tag = -tag;
        }

        return tag * 16;
    }

    enum class CallType
    {
        Construct,
        Assign,
        Copy,
        Move,
        Destroy
    };

    class CallCounter
    {
    public:
        using CallCount = std::map<std::pair<CallType, Tag>, size_t>;

    public:
        void RegisterCall(CallType callType, Tag tag)
        {
            calls_[std::make_pair(callType, tag)] += 1;
        }

        size_t GetCallCount(CallType callType, Tag tag)
        {
            return calls_[std::make_pair(callType, tag)];
        }

        void Reset()
        {
            calls_.clear();
        }

    private:
        CallCount calls_;
    };

    class ClassMock
    {
    public:
        ClassMock(Tag tag, CallCounter* callCounter = nullptr)
            : tag_(tag)
            , callCounter_(callCounter) {}

        ClassMock(const ClassMock& rhs)
            : tag_(AsCopiedTag(rhs.tag_))
            , callCounter_(rhs.callCounter_)
        {
            if (callCounter_)
            {
                callCounter_->RegisterCall(CallType::Construct, tag_);
            }

            if (rhs.callCounter_)
            {
                rhs.callCounter_->RegisterCall(CallType::Copy, rhs.tag_);
            }
        }

        ClassMock(ClassMock&& rhs)
            : tag_(AsMovedTag(rhs.tag_))
            , callCounter_(rhs.callCounter_)
        {
            if (callCounter_)
            {
                callCounter_->RegisterCall(CallType::Construct, tag_);
            }

            if (rhs.callCounter_)
            {
                rhs.callCounter_->RegisterCall(CallType::Move, rhs.tag_);
            }
        }

        const ClassMock& operator=(const ClassMock& rhs)
        {
            if (callCounter_)
            {
                callCounter_->RegisterCall(CallType::Assign, tag_);
            }

            if (rhs.callCounter_)
            {
                rhs.callCounter_->RegisterCall(CallType::Copy, rhs.tag_);
            }

            return *this;
        }

        const ClassMock& operator=(ClassMock&& rhs)
        {
            if (callCounter_)
            {
                callCounter_->RegisterCall(CallType::Assign, tag_);
            }

            if (rhs.callCounter_)
            {
                rhs.callCounter_->RegisterCall(CallType::Move, rhs.tag_);
            }

            return *this;
        }

        ~ClassMock()
        {
            if (callCounter_)
            {
                callCounter_->RegisterCall(CallType::Destroy, tag_);
            }
        }

        Tag GetTag() const
        {
            return tag_;
        }

    public:
        Tag tag_;
        CallCounter* callCounter_;
    };

    class SecondClassMock : public ClassMock
    {
    public:
        SecondClassMock(Tag tag, CallCounter* callCounter = nullptr)
            : ClassMock(tag, callCounter) {}
    };
}

TEST_CASE("Mixed type construction test", "[mixed]")
{
    using Mixed = exl::mixed<int, char, std::string, ClassMock, SecondClassMock>;
    CallCounter callCounter;
    ClassMock mock(1, &callCounter);

    SECTION("Copy constructor called")
    {
        Mixed m(mock);
        REQUIRE(callCounter.GetCallCount(CallType::Move, mock.GetTag()) == 0);
        REQUIRE(callCounter.GetCallCount(CallType::Copy, mock.GetTag()) == 1);
        REQUIRE(callCounter.GetCallCount(CallType::Construct, AsCopiedTag(mock.GetTag())) == 1);
    }

    SECTION("Move constructor called")
    {
        Mixed m(std::move(mock));
        REQUIRE(callCounter.GetCallCount(CallType::Move, mock.GetTag()) == 1);
        REQUIRE(callCounter.GetCallCount(CallType::Copy, mock.GetTag()) == 0);
        REQUIRE(callCounter.GetCallCount(CallType::Construct, AsMovedTag(mock.GetTag())) == 1);
    }

    SECTION("Correct tag assigned on construction")
    {
        Mixed m(ClassMock(1));
        REQUIRE(m.is<ClassMock>() == true);

        Mixed m2(char(255));
        REQUIRE(m2.is<char>() == true);
    }
}

TEST_CASE("Mixed type correct destructor call test", "[mixed]")
{
    using Mixed = exl::mixed<int, ClassMock, SecondClassMock>;
    CallCounter callCounter;
    ClassMock mock(1, &callCounter);

    SECTION("Mock is destroyed")
    {
        {
            Mixed m(mock);
        }
        // Destroyed

        REQUIRE(callCounter.GetCallCount(CallType::Destroy, AsCopiedTag(mock.GetTag())) == 1);
    }
}

TEST_CASE("Mixed type assignment operators test", "[mixed]")
{
    using Mixed = exl::mixed<int, ClassMock, SecondClassMock>;
    CallCounter callCounter;

    ClassMock mock1(1, &callCounter);
    ClassMock mock2(2, &callCounter);
    SecondClassMock mock3(3, &callCounter);
    SecondClassMock mock4(4, &callCounter);

    // TODO
}

TEST_CASE("Mixed type in-place construction test", "[mixed][.]")
{
    REQUIRE(false);
}

TEST_CASE("Mixed type emplace test", "[mixed][.]")
{
    REQUIRE(false);
}
