// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <map>
#include <utility>

namespace exl { namespace test
{
    using Tag = int;

    Tag as_copied_tag(Tag tag);

    Tag as_moved_tag(Tag tag);

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
        void register_call(CallType callType, Tag tag);

        size_t count(CallType callType, Tag tag);

    private:
        CallCount calls_;
    };

    class ClassMock
    {
    public:
        explicit ClassMock(Tag tag = 0, CallCounter* calls = nullptr);

        ClassMock(const ClassMock& rhs);

        ClassMock(ClassMock&& rhs) noexcept;

        ClassMock& operator=(const ClassMock& rhs);

        ClassMock& operator=(ClassMock&& rhs) noexcept;


        Tag tag() const;

        Tag original_tag() const;

        void set_tag(Tag tag);

        bool is_called_as_const();

        bool is_called_as_const() const;

        ~ClassMock();

    public:
        Tag original_tag_;
        Tag tag_;
        CallCounter* calls_;
    };

    class SecondClassMock : public ClassMock
    {
    public:
        explicit SecondClassMock(Tag tag, CallCounter* calls = nullptr)
                : ClassMock(tag, calls) {}
    };

}}
