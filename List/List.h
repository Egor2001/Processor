//
// Created by geome_try on 9/20/19.
//

#ifndef PROCESSOR_LIST_H
#define PROCESSOR_LIST_H

#include <cstdlib>

#define CRS_GUARD_LEVEL 3

#include "../Logger/Logger.h"
#include "../Logger/CourseException.h"
#include "../Logger/Guard.h"

namespace course {

using course_util::CCourseException;
using course_util::CLogger;
using course_util::operator""_crs_hash;

typedef uint32_t TListIter;

template<typename ArgType, size_t BufSize>
class CStaticList final
{
public:
    static_assert(BufSize > 0, "CStaticList [static] : SIZE must be more than 0");

    typedef TListIter iter_t_;

    static const iter_t_ NULL_ITER = 0xffffffff; //same as static_cast<uint32_t>(-1)

    static const uint32_t BitNodeSize = sizeof(uint64_t)*8;
    static const uint32_t UsedBitsSize = (BufSize - 1) / BitNodeSize + 1;

    typedef ArgType type_t_;

    typedef       type_t_*       pointer_t_;
    typedef const type_t_* const_pointer_t_;

    typedef       type_t_&       reference_t_;
    typedef const type_t_& const_reference_t_;
    typedef       type_t_&& move_reference_t_;

public:
    static const size_t CANARY_VALUE = "CStaticList"_crs_hash;

public:
    CStaticList();

    CStaticList             (const CStaticList&);
    CStaticList& operator = (const CStaticList&);

    CStaticList             (CStaticList&&);
    CStaticList& operator = (CStaticList&&);

    ~CStaticList();

    [[nodiscard]] size_t get_hash_value_() const noexcept;

    [[nodiscard]] bool is_iter_valid(TListIter it) const noexcept
    {
        return it != NULL_ITER && it < BufSize;
    }

    [[nodiscard]] bool is_it_used(TListIter it) const noexcept
    {
        return is_iter_valid(it) && (used_bits_[it / BitNodeSize] & (0x1u << (it % BitNodeSize)));
    }

    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] bool  full() const noexcept { return size_ == BufSize; }

    [[nodiscard]] size_t    size()    const noexcept { return size_; }
    [[nodiscard]] TListIter used_it() const noexcept { return used_it_; }
    [[nodiscard]] TListIter free_it() const noexcept { return free_it_; }

    [[nodiscard]] TListIter get_prev_it(TListIter it) const;
    [[nodiscard]] TListIter get_next_it(TListIter it) const;

    void set_used_val(TListIter it, bool val);

    reference_t_       get_elem(TListIter elem_it);
    const_reference_t_ get_elem(TListIter elem_it) const;

    TListIter insert(TListIter target_it, const_reference_t_ value);
    TListIter erase (TListIter target_it);

    TListIter push_front(const_reference_t_ value);
    TListIter pop_front();
/*
    TListIter push_back(const_reference_t_ value);
    TListIter pop_back();
*/
    bool clear();

    [[nodiscard]] bool ok() const noexcept;
                  bool dump() const noexcept;

private:
    CRS_IF_CANARY_GUARD(size_t beg_canary_;)
    CRS_IF_HASH_GUARD  (size_t hash_value_;)

    uint32_t size_;
    TListIter used_it_, free_it_;

    TListIter prev_buf_[BufSize];
    TListIter next_buf_[BufSize];

    type_t_  data_buf_[BufSize];

    uint64_t used_bits_[UsedBitsSize];

    CRS_IF_CANARY_GUARD(size_t end_canary_;)
};

template<typename ArgType, size_t BufSize>
CStaticList<ArgType, BufSize>::CStaticList():
    CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
    CRS_IF_HASH_GUARD  (hash_value_(0),)

    size_(0),
    used_it_(NULL_ITER), free_it_(0),
    prev_buf_{}, next_buf_{},
    data_buf_{},
    used_bits_{}

    CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
{
    next_buf_[BufSize-1] = NULL_ITER;
    for (size_t i = 0; i < BufSize-1; ++i)
        next_buf_[i] = i+1;

    prev_buf_[0] = NULL_ITER;
    for (size_t i = 1; i < BufSize; ++i)
        prev_buf_[i] = i-1;

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value_();)

    CRS_CONSTRUCT_CHECK()
}

template<typename ArgType, size_t BufSize>
CStaticList<ArgType, BufSize>::CStaticList(const CStaticList& assign_list):
    CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
    CRS_IF_HASH_GUARD  (hash_value_(0),)

    size_(assign_list.size_),
    used_it_(assign_list.used_it_), free_it_(assign_list.free_it_),
    prev_buf_{}, next_buf_{},
    data_buf_{},
    used_bits_{}

    CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
{
    for (uint32_t i = 0; i < BufSize; ++i)
    {
        prev_buf_[i] = assign_list.prev_buf_[i];
        next_buf_[i] = assign_list.next_buf_[i];

        data_buf_[i] = assign_list.data_buf_[i];
    }

    for (uint32_t i = 0; i < UsedBitsSize; ++i)
        used_bits_[i] = assign_list.used_bits_[i];

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value_();)

    CRS_CONSTRUCT_CHECK()
}

template<typename ArgType, size_t BufSize>
CStaticList<ArgType, BufSize>&
CStaticList<ArgType, BufSize>::operator = (const CStaticList& assign_list)
{
    if (&assign_list == this)
        return *this;

    CRS_IF_GUARD(CRS_BEG_CHECK();)

    size_ = assign_list.size_;

    used_it_ = assign_list.used_it_;
    free_it_ = assign_list.free_it_;

    for (size_t i = 0; i < BufSize; ++i)
    {
        prev_buf_[i] = assign_list.prev_buf_[i];
        next_buf_[i] = assign_list.next_buf_[i];

        data_buf_[i] = assign_list.data_buf_[i];
    }

    for (size_t i = 0; i < UsedBitsSize; ++i)
        used_bits_[i] = assign_list.used_bits_[i];

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return *this;
}

template<typename ArgType, size_t BufSize>
CStaticList<ArgType, BufSize>::CStaticList(CStaticList&& move_list):
    CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
    CRS_IF_HASH_GUARD  (hash_value_(0),)

    size_(std::move(move_list.size_)),
    used_it_(std::move(move_list.used_it_)), free_it_(std::move(move_list.free_it_)),
    prev_buf_{}, next_buf_{},
    data_buf_{},
    used_bits_{}

    CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
{
    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value_();)

    std::swap(prev_buf_, move_list.prev_buf_);
    std::swap(next_buf_, move_list.next_buf_);

    std::swap(data_buf_, move_list.data_buf_);

    std::swap(used_bits_, move_list.used_bits_);

    CRS_CONSTRUCT_CHECK()
}

template<typename ArgType, size_t BufSize>
CStaticList<ArgType, BufSize>& CStaticList<ArgType, BufSize>::operator = (CStaticList&& move_list)
{
    if (&move_list == this)
        return *this;

    CRS_IF_GUARD(CRS_BEG_CHECK();)

    size_ = std::move(move_list.size_);

    used_it_ = std::move(move_list.used_it_);
    free_it_ = std::move(move_list.free_it_);

    std::swap(prev_buf_, move_list.prev_buf_);
    std::swap(next_buf_, move_list.next_buf_);

    std::swap(data_buf_, move_list.data_buf_);

    std::swap(used_bits_, move_list.used_bits_);

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return *this;
}

template<typename ArgType, size_t BufSize>
CStaticList<ArgType, BufSize>::~CStaticList()
{
    CRS_DESTRUCT_CHECK()

    free_it_ = NULL_ITER; used_it_ = NULL_ITER;
    size_ = 0;

    for (size_t i = 0; i < BufSize; ++i)
    {
        prev_buf_[i] = next_buf_[i] = NULL_ITER;
        data_buf_[i] = {};
    }

    std::memset(used_bits_, 0x0, sizeof(used_bits_[0])*UsedBitsSize);
}

template<typename ArgType, size_t BufSize>
size_t CStaticList<ArgType, BufSize>::get_hash_value_() const noexcept
{
    size_t result = 0;
    result ^= size_ ^ used_it_ ^ free_it_ ^ size_t(prev_buf_) ^ size_t(next_buf_);

    size_t shift_delta = 8;
    size_t shift_ratio = sizeof(BufSize);
    for (size_t i = 0; i < BufSize; ++i)
        result ^= (prev_buf_[i] >> (shift_delta*((i + 0)%shift_ratio))) ^
                  (next_buf_[i] >> (shift_delta*((i + 1)%shift_ratio)));

    for (size_t i = 0; i < UsedBitsSize; ++i)
        result ^= used_bits_[i];

    result ^= size_t(prev_buf_) ^ size_t(next_buf_) ^ size_t(data_buf_) ^ size_t(used_bits_);

    return result;
}

template<typename ArgType, size_t BufSize>
typename CStaticList<ArgType, BufSize>::iter_t_
CStaticList<ArgType, BufSize>::get_prev_it(TListIter it) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!is_it_used(it))
        CRS_PROCESS_ERROR("CStaticList::get_prev_it error : elem_it %lu is not used: BufSize=%llu", it, BufSize)

    CRS_IF_GUARD(CRS_END_CHECK();)

    return prev_buf_[it];
}

template<typename ArgType, size_t BufSize>
typename CStaticList<ArgType, BufSize>::iter_t_
CStaticList<ArgType, BufSize>::get_next_it(TListIter it) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!is_it_used(it))
        CRS_PROCESS_ERROR("CStaticList::get_next_it error : elem_it %lu is not used: BufSize=%llu", it, BufSize)

    CRS_IF_GUARD(CRS_END_CHECK();)

    return next_buf_[it];
}

template<typename ArgType, size_t BufSize>
void CStaticList<ArgType, BufSize>::set_used_val(TListIter it, bool val)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    uint64_t bit_val = (val ? (0x1u << (it % BitNodeSize)) : 0);

    used_bits_[it / BitNodeSize] = (used_bits_[it / BitNodeSize] & ~(0x1u << (it % BitNodeSize))) | bit_val;

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)
}

template<typename ArgType, size_t BufSize>
typename CStaticList<ArgType, BufSize>::reference_t_
CStaticList<ArgType, BufSize>::get_elem(TListIter elem_it)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!is_iter_valid(elem_it))
        CRS_PROCESS_ERROR("CStaticList::get_elem error : elem_it %lu is not valid: BufSize=%llu", elem_it, BufSize)

    CRS_IF_GUARD(CRS_END_CHECK();)

    return data_buf_[elem_it];
}

template<typename ArgType, size_t BufSize>
typename CStaticList<ArgType, BufSize>::const_reference_t_
CStaticList<ArgType, BufSize>::get_elem(TListIter elem_it) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    if (!is_iter_valid(elem_it))
        CRS_PROCESS_ERROR("CStaticList::get_elem error : elem_it %lu is not valid: BufSize=%llu", elem_it, BufSize)

    CRS_IF_GUARD(CRS_END_CHECK();)

    return data_buf_[elem_it];
}

template<typename ArgType, size_t BufSize>
typename CStaticList<ArgType, BufSize>::iter_t_
CStaticList<ArgType, BufSize>::insert(TListIter target_it, const_reference_t_ value)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)
    TListIter result = NULL_ITER;

    if (!is_it_used(target_it) && !(target_it == NULL_ITER && size_ == 0))
        CRS_PROCESS_ERROR("CStaticList::insert error : target_it %lu is not used: BufSize=%llu", target_it, BufSize)
    else if (free_it_ == NULL_ITER)
        CRS_PROCESS_ERROR("CStaticList::insert error: buffer overflow: %llu", size_)
    else
    {
        result = free_it_;
        free_it_ = next_buf_[free_it_];

        if (free_it_)
            prev_buf_[free_it_] = NULL_ITER;

        if (target_it == used_it_)
            used_it_ = result;

        if (target_it != NULL_ITER)
        {
            next_buf_[result] =           target_it;
            prev_buf_[result] = prev_buf_[target_it];

            if (prev_buf_[target_it] != NULL_ITER)
                next_buf_[prev_buf_[target_it]] = result;

            prev_buf_[target_it] = result;
        }
        else
            next_buf_[result] = NULL_ITER;
    }

    if (result != NULL_ITER)
    {
        data_buf_[result] = value;
        set_used_val(result, true);
        ++size_;
    }

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

template<typename ArgType, size_t BufSize>
typename CStaticList<ArgType, BufSize>::iter_t_
CStaticList<ArgType, BufSize>::erase(TListIter target_it)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)
    TListIter result = NULL_ITER;

    if (!is_it_used(target_it))
        CRS_PROCESS_ERROR("CStaticList::erase error : target_it %lu is not used: BufSize=%llu", target_it, BufSize)
    else if (target_it != free_it_)
    {
        set_used_val(target_it, false);

        if (target_it == used_it_)
            used_it_ = next_buf_[used_it_];

        result = next_buf_[target_it];

        if (next_buf_[target_it] != NULL_ITER)
            prev_buf_[next_buf_[target_it]] = prev_buf_[target_it];

        if (prev_buf_[target_it] != NULL_ITER)
            next_buf_[prev_buf_[target_it]] = next_buf_[target_it];

        if (free_it_)
            prev_buf_[free_it_] = target_it;

        next_buf_[target_it] = free_it_;
        prev_buf_[target_it] = NULL_ITER;

        free_it_ = target_it;
    }

    if (target_it == free_it_)
        --size_;

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

template<typename ArgType, size_t BufSize>
typename CStaticList<ArgType, BufSize>::iter_t_
CStaticList<ArgType, BufSize>::push_front(const_reference_t_ value)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)
    TListIter result = insert(used_it_, value);

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

template<typename ArgType, size_t BufSize>
typename CStaticList<ArgType, BufSize>::iter_t_
CStaticList<ArgType, BufSize>::pop_front()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)
    TListIter result = erase(used_it_);

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}
/*
template<typename ArgType, size_t BufSize>
TListIter CStaticList<ArgType, BufSize>::push_back(const_reference_t_ value)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)
    TListIter result = NULL_ITER;

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

template<typename ArgType, size_t BufSize>
TListIter CStaticList<ArgType, BufSize>::pop_back()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)
    TListIter result = NULL_ITER;

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}
*/
template<typename ArgType, size_t BufSize>
bool CStaticList<ArgType, BufSize>::clear()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)
    bool result = false;

    std::memset(used_bits_, 0x00, sizeof(used_bits_[0])*UsedBitsSize);

    for (size_t i = 0; i < BufSize; ++i)
        data_buf_[i] = {};

    next_buf_[BufSize-1] = NULL_ITER;
    for (size_t i = 0; i < BufSize-1; ++i)
        next_buf_[i] = i+1;

    prev_buf_[0] = NULL_ITER;
    for (size_t i = 1; i < BufSize; ++i)
        prev_buf_[i] = i-1;

    free_it_ = 0; used_it_ = NULL_ITER;
    size_ = 0;

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value_();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

template<typename ArgType, size_t BufSize>
bool CStaticList<ArgType, BufSize>::ok() const noexcept
{
    bool result = (size_ >= 0) && (size_ <= BufSize) &&
                  (used_it_ < BufSize || used_it_ == NULL_ITER) &&
                  (free_it_ < BufSize || free_it_ == NULL_ITER) &&
                  (used_it_ != NULL_ITER || free_it_ != NULL_ITER) &&
                  (prev_buf_ && next_buf_ && data_buf_) &&
                  (used_it_ == NULL_ITER || prev_buf_[used_it_] == NULL_ITER) &&
                  (free_it_ == NULL_ITER || prev_buf_[free_it_] == NULL_ITER);

    return result;
}

template<typename ArgType, size_t BufSize>
bool CStaticList<ArgType, BufSize>::dump() const noexcept
{
    CRS_STATIC_DUMP("CStaticList[%s, this : %p] \n"
                    "{ \n"
                    CRS_IF_CANARY_GUARD(
                    "    beg_canary_[%s] : %#X \n"
                    ) //CRS_IF_CANARY_GUARD
                    CRS_IF_HASH_GUARD(
                    "    hash_value_[%s] : %#X \n"
                    ) //CRS_IF_HASH_GUARD
                    "    \n"
                    "    size_           : %llu \n"
                    "    \n"
                    "    used_it_        : %lu \n"
                    "    free_it_        : %lu \n"
                    "    \n"
                    "    prev_buf_        : %p \n"
                    "    next_buf_        : %p \n"
                    "    data_buf_        : %p \n"
                    "    used_bits_       : %p \n"
                    "    \n"
                    CRS_IF_CANARY_GUARD(
                    "    end_canary_[%s] : %#X \n"
                    ) //CRS_IF_CANARY_GUARD
                    "} \n",

                    (ok() ? "OK" : "ERROR"), this,
                    CRS_IF_CANARY_GUARD((beg_canary_ == CANARY_VALUE       ? "OK" : "ERROR"), beg_canary_,)
                    CRS_IF_HASH_GUARD  ((hash_value_ == get_hash_value_() ? "OK" : "ERROR"), hash_value_,)

                    size_,
                    used_it_, free_it_,

                    prev_buf_, next_buf_,
                    data_buf_,
                    used_bits_

                    CRS_IF_CANARY_GUARD(, (end_canary_ == CANARY_VALUE ? "OK" : "ERROR"), end_canary_));

    return ok();
}

}

#endif //PROCESSOR_LIST_H
