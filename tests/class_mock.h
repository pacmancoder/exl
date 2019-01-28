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
        ClassMock(Tag tag, CallCounter* calls = nullptr)
            : tag_(tag)
            , calls_(calls)
        {
            if (calls_)
            {
                calls_->register_call(CallType::Construct, tag_);
            }
        }

        ClassMock(const ClassMock& rhs)
            : tag_(as_copied_tag(rhs.tag_))
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

        ClassMock(ClassMock&& rhs)
            : tag_(as_moved_tag(rhs.tag_))
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

        const ClassMock& operator=(const ClassMock& rhs)
        {
            if (calls_)
            {
                calls_->register_call(CallType::Assign, tag_);
            }

            if (rhs.calls_)
            {
                rhs.calls_->register_call(CallType::Copy, rhs.tag_);
            }

            tag_ = as_copied_tag(rhs.tag_);

            return *this;
        }

        const ClassMock& operator=(ClassMock&& rhs)
        {
            if (calls_)
            {
                calls_->register_call(CallType::Assign, tag_);
            }

            if (rhs.calls_)
            {
                rhs.calls_->register_call(CallType::Move, rhs.tag_);
            }

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

    public:
        Tag tag_;
        CallCounter* calls_;
    };

    class SecondClassMock : public ClassMock
    {
    public:
        SecondClassMock(Tag tag, CallCounter* calls = nullptr)
            : ClassMock(tag, calls) {}
    };

}}
