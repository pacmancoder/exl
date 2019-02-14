// Copyright (C) 2019 Vladislav Nikonov <mail@pacmancoder.xyz>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ClassMock.hpp"

namespace exl { namespace mock
{
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

    void CallCounter::register_call(CallType callType, Tag tag)
    {
        calls_[std::make_pair(callType, tag)] += 1;
    }

    size_t CallCounter::count(CallType callType, Tag tag)
    {
        return calls_[std::make_pair(callType, tag)];
    }

    ClassMock::ClassMock(Tag tag, CallCounter* calls)
            : original_tag_(tag)
            , tag_(tag)
            , calls_(calls)
    {
        if (calls_)
        {
            calls_->register_call(CallType::Construct, tag_);
        }
    }

    ClassMock::ClassMock(const ClassMock& rhs)
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

    ClassMock::ClassMock(ClassMock&& rhs) noexcept
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

    ClassMock& ClassMock::operator=(const ClassMock& rhs)
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

    ClassMock& ClassMock::operator=(ClassMock&& rhs) noexcept
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

    ClassMock::~ClassMock()
    {
        if (calls_)
        {
            calls_->register_call(CallType::Destroy, tag_);
        }
    }

    Tag ClassMock::tag() const
    {
        return tag_;
    }

    Tag ClassMock::original_tag() const
    {
        return original_tag_;
    }
}}