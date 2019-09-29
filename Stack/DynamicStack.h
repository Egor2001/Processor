#ifndef DYNAMIC_STACK_H_INCLUDED
#define DYNAMIC_STACK_H_INCLUDED

#include <cstdlib>
#include <assert.h>
#include <type_traits>
#include <memory.h>
#include <cstdint>
#include <random>

#include "../Logger/Logger.h"

#include "../Logger/CourseException.h"

#define CRS_GUARD_LEVEL 3

#include "../Logger/Guard.h"

namespace course_stack {

template<typename ElemType>
class CDynamicStack
{
public:
    typedef ElemType type_t_;

    typedef       type_t_&       reference_t_;
    typedef const type_t_& const_reference_t_;
    typedef       type_t_*       pointer_t_;
    typedef const type_t_* const_pointer_t_;

    typedef type_t_&& rvalue_reference_t_;

public:
    static const size_t MIN_CAPASITY = 2;
    static const size_t CANARY_VALUE = "CDynamicStack"_crs_hash;

public:
    CDynamicStack():
        CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
        CRS_IF_HASH_GUARD  (hash_value_(0),)

        capasity_    (MIN_CAPASITY),
        size_        (0),
        byte_storage_(nullptr),
        buffer_      (nullptr)

        CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
    {
        byte_storage_ = new char[capasity_*sizeof(type_t_) + alignof(type_t_)] { static_cast<char>(0xFF) };
        buffer_ = reinterpret_cast<pointer_t_>((std::uintptr_t(byte_storage_) + alignof(type_t_)-1) &
                                               -alignof(type_t_));
        //(because of two�s complement)

        for (size_t i = 0; i < size_; i++)
            new (buffer_ + i) type_t_();

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_CONSTRUCT_CHECK();)
    }

    CDynamicStack(const CDynamicStack& assign_stack):
        CRS_IF_CANARY_GUARD(beg_canary_(assign_stack.beg_canary_),)
        CRS_IF_HASH_GUARD  (hash_value_(0),)

        capasity_    (assign_stack.capasity_),
        size_        (assign_stack.size_),
        byte_storage_(nullptr),
        buffer_      (nullptr)

        CRS_IF_CANARY_GUARD(, end_canary_(assign_stack.end_canary_))
    {
        byte_storage_ = new char[capasity_*sizeof(type_t_) + alignof(type_t_)] { static_cast<char>(0xFF) };
        buffer_ = reinterpret_cast<pointer_t_>((std::uintptr_t(byte_storage_) + alignof(type_t_)-1) &
                                               -alignof(type_t_));

        for (size_t i = 0; i < size_; i++)
            new (buffer_ + i) type_t_(assign_stack.buffer_[i]);

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_CONSTRUCT_CHECK();)
    }

    CDynamicStack(CDynamicStack&& assign_stack):
        CRS_IF_CANARY_GUARD(beg_canary_(assign_stack.beg_canary_),)
        CRS_IF_HASH_GUARD  (hash_value_(assign_stack.hash_value_),)

        capasity_    (assign_stack.capasity_),
        size_        (assign_stack.size_),
        byte_storage_(assign_stack.byte_storage_),
        buffer_      (assign_stack.buffer_)

        CRS_IF_CANARY_GUARD(, end_canary_(assign_stack.end_canary_))
    {
        assign_stack.capasity_     = 0u;
        assign_stack.size_         = 0u;
        assign_stack.byte_storage_ = nullptr;
        assign_stack.buffer_       = nullptr;

        CRS_IF_HASH_GUARD(assign_stack.hash_value_ = assign_stack.calc_hash_value_();)
        assign_stack = CDynamicStack();

        CRS_IF_GUARD(CRS_CONSTRUCT_CHECK();)
    }

    CDynamicStack& operator = (const CDynamicStack& assign_stack)
    {
        CRS_IF_GUARD(CRS_BEG_CHECK());

        capasity_ = assign_stack.capasity_;
        size_     = assign_stack.size_;

        byte_storage_ = new char[capasity_*sizeof(type_t_) + alignof(type_t_)] { static_cast<char>(0xFF) };
        buffer_ = reinterpret_cast<pointer_t_>((std::uintptr_t(byte_storage_) + alignof(type_t_)-1) &
                                               -alignof(type_t_));

        for (size_t i = 0; i < size_; i++)
            new (buffer_ + i) type_t_(assign_stack.buffer_[i]);

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)

        return *this;
    }

    CDynamicStack& operator = (CDynamicStack&& assign_stack)
    {
        CRS_IF_GUARD(CRS_BEG_CHECK());

        CRS_IF_CANARY_GUARD(beg_canary_ = assign_stack.beg_canary_;)
        CRS_IF_HASH_GUARD  (hash_value_ = assign_stack.hash_value_;)

        capasity_     = assign_stack.capasity_;     assign_stack.capasity_     = 0u;
        size_         = assign_stack.size_;         assign_stack.size_         = 0u;
        byte_storage_ = assign_stack.byte_storage_; assign_stack.byte_storage_ = nullptr;
        buffer_       = assign_stack.buffer_;       assign_stack.buffer_       = nullptr;

        CRS_IF_CANARY_GUARD(end_canary_ = assign_stack.end_canary_;)

        CRS_IF_HASH_GUARD(assign_stack.hash_value_ = assign_stack.calc_hash_value_();)
        assign_stack = CDynamicStack();

        CRS_IF_GUARD(CRS_END_CHECK();)

        return *this;
    }

    ~CDynamicStack()
    {
        CRS_IF_GUARD(CRS_DESTRUCT_CHECK();)

        CRS_IF_CANARY_GUARD(beg_canary_ = end_canary_ = 0;)
        CRS_IF_HASH_GUARD  (hash_value_ = 0;)

        for (size_t i = 0; i < size_; i++)
          buffer_[i].~type_t_();

        memset(reinterpret_cast<void*>(buffer_), 0xFF, capasity_*sizeof(type_t_));

        delete [] byte_storage_;

        buffer_ = nullptr;
        byte_storage_ = nullptr;

        capasity_ = size_ = 0;
    }

private:
    CRS_IF_HASH_GUARD(
    size_t calc_hash_value_() const
    {
        size_t result = 0;

        char* byte_buffer = reinterpret_cast<char*>(buffer_);

        if (size_)
        {
            for (size_t i = 0; i < (size_-1)*sizeof(type_t_); i++)
                result ^= byte_buffer[i] << (i % (0x8*sizeof(size_t)));
        }

        result ^= (CRS_IF_CANARY_GUARD((beg_canary_ ^ end_canary_) ^)
                   (capasity_ >> 1) ^ size_ ^
                   (reinterpret_cast<std::uintptr_t>(byte_storage_) &
                    reinterpret_cast<std::uintptr_t>(buffer_)));

        return result;
    }
    )//CRS_IF_HASH_GUARD

    void expand_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        capasity_ *= 2;

        char* new_byte_storage_ = new char[capasity_*sizeof(type_t_) + alignof(type_t_)] { static_cast<char>(0xFF) };
        pointer_t_ new_buffer_ = reinterpret_cast<pointer_t_>((std::uintptr_t(new_byte_storage_) + alignof(type_t_)-1) &
                                                              -alignof(type_t_));

        for (size_t i = 0; i < size_; i++)
        {
            new (new_buffer_ + i) type_t_(std::move(buffer_[i]));
            buffer_[i].~type_t_();
        }

        memset(reinterpret_cast<void*>(buffer_), 0xFF, size_*sizeof(type_t_));
        delete [] byte_storage_;

        byte_storage_ = new_byte_storage_;
        buffer_ = new_buffer_;

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    void truncate_()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        if (capasity_ < size_*2)
        {
            CRS_IF_GUARD(CRS_END_CHECK();)

            return;
        }

        capasity_ = MIN_CAPASITY;

        while (capasity_ < size_)
            capasity_ *= 2;

        char* new_byte_storage_ = new char[capasity_*sizeof(type_t_) + alignof(type_t_)]{static_cast<char>(0xFF)};
        pointer_t_ new_buffer_ = reinterpret_cast<pointer_t_>((std::uintptr_t(new_byte_storage_) + alignof(type_t_)-1) &
                                                              -alignof(type_t_));

        for (size_t i = 0; i < size_; i++)
        {
            new (new_buffer_ + i) type_t_(std::move(buffer_[i]));
            buffer_[i].~type_t_();
        }

        memset(reinterpret_cast<void*>(buffer_), 0xFF, size_*sizeof(type_t_));
        delete [] byte_storage_;

        byte_storage_ = new_byte_storage_;
        buffer_ = new_buffer_;

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

public:
    size_t size () const { return size_; }
    bool   empty() const { return (size_ != 0u); }

    void shrink_to_fit()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        truncate_();

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    reference_t_ top()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        if (size_ == 0)
            throw CCourseException("top() was called on empty stack");

        CRS_IF_GUARD(CRS_END_CHECK();)

        return buffer_[size_-1];
    }

    const_reference_t_ top() const
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        if (size_ == 0)
            throw CCourseException("top() was called on empty stack");

        CRS_IF_GUARD(CRS_END_CHECK();)

        return buffer_[size_-1];
    }

    void push(const_reference_t_ elem)
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        if (size_ == capasity_)
            expand_();

        new (buffer_ + size_) type_t_(elem);
        size_++;

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    void push(rvalue_reference_t_ elem)
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        if (size_ == capasity_)
            expand_();

        new (buffer_ + size_) type_t_(std::move(elem));
        size_++;

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    template<typename... Types>
    void emplace(Types&&... args)
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        if (size_ == capasity_)
            expand_();

        new (buffer_ + size_) type_t_(std::forward<Types>(args)...);
        size_++;

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

    void pop()
    {
        CRS_IF_GUARD(CRS_BEG_CHECK();)

        if (size_ == 0)
            throw CCourseException("trying pop() when empty");

        buffer_[size_-1].~type_t_();
        memset(static_cast<void*>(buffer_+size_-1), 0xFF, sizeof(type_t_));
        size_--;

        CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

        CRS_IF_GUARD(CRS_END_CHECK();)
    }

public:
    CRS_IF_HASH_GUARD(
    size_t get_hash_value() const
    {
        return hash_value_;
    }
    )//CRS_IF_HASH_GUARD

    void assert_ok() const
    {
        if (!ok())
            throw CCourseException("CDynamicStack is not ok");
    }

    bool ok() const
    {
        return (this &&
                CRS_IF_CANARY_GUARD(beg_canary_ == CANARY_VALUE &&
                                    end_canary_ == CANARY_VALUE &&)
                CRS_IF_HASH_GUARD(hash_value_ == calc_hash_value_() &&)
                byte_storage_ && buffer_ &&
                (capasity_ >=   size_) &&
                !(std::uintptr_t(byte_storage_) % alignof(type_t_)));
    }

    void dump() const
    {
        CRS_STATIC_DUMP("CDynamicStack[%s, this : %p] \n"
                        "{ \n"
                        CRS_IF_CANARY_GUARD("    beg_canary_[%s] : %#X \n")
                        CRS_IF_HASH_GUARD  ("    hash_value_[%s] : %#X \n")
                        "    \n"
                        "    capasity_     : %d \n"
                        "    size_         : %d \n"
                        "    byte_storage_ : %p \n"
                        "    { \n"
                        //CRS_IF_CANARY_GUARD("        canary[%s] : %#X \n")
                        CRS_IF_CANARY_GUARD("        ...              \n")
                        //CRS_IF_CANARY_GUARD("        canary[%s] : %#X \n")
                        "    } \n"
                        "    \n"
                        "    buffer_       : %p \n"
                        "    \n"
                        CRS_IF_CANARY_GUARD("    end_canary_[%s] : %#X \n")
                        "} \n",

                        (ok() ? "OK" : "ERROR"), this,
                        CRS_IF_CANARY_GUARD((beg_canary_ == CANARY_VALUE       ? "OK" : "ERROR"), beg_canary_,)
                        CRS_IF_HASH_GUARD  ((hash_value_ == calc_hash_value_() ? "OK" : "ERROR"), hash_value_,)

                        capasity_,
                        size_,
                        byte_storage_,
                        buffer_

                        CRS_IF_CANARY_GUARD(, (end_canary_ == CANARY_VALUE ? "OK" : "ERROR"), end_canary_));
    }

private:
    CRS_IF_CANARY_GUARD(size_t beg_canary_;)
    CRS_IF_HASH_GUARD  (size_t hash_value_;)

    size_t capasity_;
    size_t size_;
    char* byte_storage_;
    pointer_t_ buffer_;

    CRS_IF_CANARY_GUARD(size_t end_canary_;)
};

}

#undef CRS_GUARD_LEVEL

#endif // DYNAMIC_STACK_H_INCLUDED
