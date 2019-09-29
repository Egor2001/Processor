#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include <cstdlib>

#include "../Logger/Logger.h"
#include "../Logger/CourseException.h"

#define CRS_GUARD_LEVEL 3

#include "../Logger/Guard.h"

namespace course_util {

template<typename ElemType, size_t BufSize>
class CStaticStack
{
public:
    static const size_t BUFFER_CAPASITY = BufSize;

    typedef ElemType type_t_;

    typedef       type_t_&       reference_t_;
    typedef const type_t_& const_reference_t_;
    typedef       type_t_*       pointer_t_;
    typedef const type_t_* const_pointer_t_;

public:
    static const size_t CANARY_VALUE = "CStaticStack"_crs_hash;

public:
    CStaticStack();

    CStaticStack(const CStaticStack& assign_stack);

    CStaticStack& operator = (const CStaticStack& assign_stack);

    ~CStaticStack();

private:
    [[nodiscard]] size_t calc_hash_value_() const;

public:
    [[nodiscard]] size_t size() const;

    reference_t_       top();
    const_reference_t_ top() const;
    type_t_            pop();
    reference_t_       push(const_reference_t_ elem);

    void clear();

public:
    [[nodiscard]] size_t get_hash_value() const;

    [[nodiscard]] bool ok() const noexcept;
                  bool dump() const noexcept;

private:
    CRS_IF_CANARY_GUARD(size_t beg_canary_;)
    CRS_IF_HASH_GUARD  (size_t hash_value_;)

    type_t_ buffer_[BUFFER_CAPASITY];
    size_t  size_;

    CRS_IF_CANARY_GUARD(size_t end_canary_;)
};

template<typename ElemType, size_t BufSize>
CStaticStack<ElemType, BufSize>::CStaticStack():
        CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
        CRS_IF_HASH_GUARD  (hash_value_(0),)

        buffer_{},
        size_{}

        CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
{
    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_CONSTRUCT_CHECK();)
}

template<typename ElemType, size_t BufSize>
CStaticStack<ElemType, BufSize>::CStaticStack(const CStaticStack& assign_stack):
        CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
        CRS_IF_HASH_GUARD  (hash_value_(0),)

        buffer_{},
        size_{}

        CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
{
    memcpy(buffer_, assign_stack.buffer_, assign_stack.size());

    size_ = assign_stack.size();

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_CONSTRUCT_CHECK();)
}

template<typename ElemType, size_t BufSize>
CStaticStack<ElemType, BufSize>& CStaticStack<ElemType, BufSize>::operator =(const CStaticStack& assign_stack)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    memcpy(buffer_, assign_stack.buffer_, assign_stack.size());
    size_ = assign_stack.size();

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)

    return *this;
}

template<typename ElemType, size_t BufSize>
CStaticStack<ElemType, BufSize>::~CStaticStack()
{
    CRS_IF_GUARD(CRS_DESTRUCT_CHECK();)

    size_ = 0;
    memset(buffer_, 0x00, BUFFER_CAPASITY*sizeof(type_t_));
}

template<typename ElemType, size_t BufSize>
size_t CStaticStack<ElemType, BufSize>::calc_hash_value_() const
{
    size_t result = 0;

    const char* byte_buffer = reinterpret_cast<const char*>(buffer_);

    if (size_)
    {
        for (size_t i = 0; i < (size_-1)*sizeof(type_t_); i++)
            result ^= static_cast<uint8_t>(byte_buffer[i]) << (i % (0x8*sizeof(size_t)));
    }

    result ^= (CRS_IF_CANARY_GUARD((beg_canary_ ^ end_canary_) ^)
               (BUFFER_CAPASITY >> size_t(1)) ^ size_ ^
               reinterpret_cast<std::uintptr_t>(buffer_));

    return result;
}

template<typename ElemType, size_t BufSize>
size_t CStaticStack<ElemType, BufSize>::size() const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return size_;
}

template<typename ElemType, size_t BufSize>
typename CStaticStack<ElemType, BufSize>::reference_t_
CStaticStack<ElemType, BufSize>::top()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (size_ == 0)
        throw CCourseException("top() was called on empty stack");

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)

    return buffer_[size_-1];
}

template<typename ElemType, size_t BufSize>
typename CStaticStack<ElemType, BufSize>::const_reference_t_
CStaticStack<ElemType, BufSize>::top() const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (size_ == 0)
        throw CCourseException("top() was called on empty stack");

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)

    return buffer_[size_-1];
}

template<typename ElemType, size_t BufSize>
typename CStaticStack<ElemType, BufSize>::type_t_
CStaticStack<ElemType, BufSize>::pop()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (size_ == 0)
        throw CCourseException("trying pop() when empty");

    type_t_ result = buffer_[size_-1]; size_--;

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

template<typename ElemType, size_t BufSize>
typename CStaticStack<ElemType, BufSize>::reference_t_
CStaticStack<ElemType, BufSize>::push(const_reference_t_ elem)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (size_ == BUFFER_CAPASITY)
        throw CCourseException("push() causes buffer overflow");

    size_++;
    buffer_[size_-1] = elem;

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)

    return buffer_[size_-1];
}

template<typename ElemType, size_t BufSize>
void CStaticStack<ElemType, BufSize>::clear()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    size_ = 0;
    memset(buffer_, 0x00, BUFFER_CAPASITY*sizeof(type_t_));

    CRS_IF_HASH_GUARD(hash_value_ = calc_hash_value_();)

    CRS_IF_GUARD(CRS_END_CHECK();)
}

template<typename ElemType, size_t BufSize>
size_t CStaticStack<ElemType, BufSize>::get_hash_value() const
{
    return hash_value_;
}

template<typename ElemType, size_t BufSize>
bool CStaticStack<ElemType, BufSize>::ok() const noexcept
{
    return (this && CRS_IF_CANARY_GUARD(beg_canary_ == CANARY_VALUE &&
                                        end_canary_ == CANARY_VALUE &&)
            CRS_IF_HASH_GUARD(hash_value_ == calc_hash_value_() &&)
            buffer_ && (size_ <= BUFFER_CAPASITY));
}

template<typename ElemType, size_t BufSize>
bool CStaticStack<ElemType, BufSize>::dump() const noexcept
{
    CRS_STATIC_DUMP("CStaticStack[%s, this : %p] \n"
                    "{ \n"
                    CRS_IF_CANARY_GUARD(
                    "    beg_canary_[%s] : %#X \n"
                    ) //CRS_IF_CANARY_GUARD
                    CRS_IF_HASH_GUARD  (
                    "    hash_value_[%s] : %#X \n"
                    ) //CRS_IF_CANARY_GUARD
                    "    \n"
                    "    size_           : %d \n"
                    "    buffer_         : %p \n"
                    "    \n"
                    CRS_IF_CANARY_GUARD(
                    "    end_canary_[%s] : %#X \n"
                    ) //CRS_IF_CANARY_GUARD
                    "} \n",

                    (ok() ? "OK" : "ERROR"), this,
                    CRS_IF_CANARY_GUARD((beg_canary_ == CANARY_VALUE       ? "OK" : "ERROR"), beg_canary_,)
                    CRS_IF_HASH_GUARD  ((hash_value_ == calc_hash_value_() ? "OK" : "ERROR"), hash_value_,)

                    size_,
                    buffer_

                    CRS_IF_CANARY_GUARD(, (end_canary_ == CANARY_VALUE ? "OK" : "ERROR"), end_canary_));

    return ok();
}

}

#undef CRS_GUARD_LEVEL

#endif // STACK_H_INCLUDED
