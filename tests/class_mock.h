// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <map>
#include <utility>

namespace exl { namespace mock
{
    using Tag = int;

    Tag as_copied_tag(Tag tag)
    {
        if (tag < 0)
        {
            tag = -tag;
        }

        return tag * 16;
    }

    Tag as_moved_tag(Tag tag)
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
        void register_call(CallType callType, Tag tag)
        {
            calls_[std::make_pair(callType, tag)] += 1;
        }

        size_t count(CallType callType, Tag tag)
        {
            return calls_[std::make_pair(callType, tag)];
        }

        void reset()
        {
            calls_.clear();
        }

    private:
        CallCount calls_;
    };

    class ClassMock
    {
    public:
        explicit ClassMock(Tag tag, CallCounter* calls = nullptr)
                : original_tag_(tag)
                , tag_(tag)
                , calls_(calls)
        {
            if (calls_)
            {
                calls_->register_call(CallType::Construct, tag_);
            }
        }

        ClassMock(const ClassMock& rhs)
                : original_tag_(rhs.original_tag_)
                , tag_(as_copied_tag(rhs.tag_))
                , calls_(rhs.calls_)
        {
            if (calls_)
            {
                calls_->register_call(CallType::Construct, tag_);
            }

            if (rhs.calls_)
            {
                rhs.calls_->register_call(CallType::Copy, rhs.tag_);
            }
        }

        ClassMock(ClassMock&& rhs) noexcept
                : original_tag_(rhs.original_tag_)
                , tag_(as_moved_tag(rhs.tag_))
                , calls_(rhs.calls_)
        {
            if (calls_)
            {
                calls_->register_call(CallType::Construct, tag_);
            }

            if (rhs.calls_)
            {
                rhs.calls_->register_call(CallType::Move, rhs.tag_);
            }
        }

        ClassMock& operator=(const ClassMock& rhs)
        {
            if (calls_)
            {
                calls_->register_call(CallType::Assign, tag_);
            }

            if (rhs.calls_)
            {
                rhs.calls_->register_call(CallType::Copy, rhs.tag_);
            }

            original_tag_ = rhs.original_tag_;
            tag_ = as_copied_tag(rhs.tag_);

            return *this;
        }

        ClassMock& operator=(ClassMock&& rhs)
        {
            if (calls_)
            {
                calls_->register_call(CallType::Assign, tag_);
            }

            if (rhs.calls_)
            {
                rhs.calls_->register_call(CallType::Move, rhs.tag_);
            }

            original_tag_ = rhs.original_tag_;
            tag_ = as_moved_tag(rhs.tag_);

            return *this;
        }

        ~ClassMock()
        {
            if (calls_)
            {
                calls_->register_call(CallType::Destroy, tag_);
            }
        }

        Tag tag() const
        {
            return tag_;
        }

        Tag original_tag() const
        {
            return original_tag_;
        }

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
