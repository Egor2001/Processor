//
// Created by geome_try on 9/20/19.
//

#ifndef PROCESSOR_CACHE_H
#define PROCESSOR_CACHE_H

#include <cstdint>
#include <cstdlib>
#include <utility>

#include "../List/List.h"
#include "../CPU/ProcessorEnums.h"
#include "MemoryController.h"
#include "MemoryEnums.h"

namespace course {

class CCache final
{
public:
    struct SNode
    {
        TPage page;

        mutable uint16_t flag;
        mutable uint16_t freq;
    };

    static const size_t CACHE_PAGES_CNT = 256; //cache size in pages
    static const size_t CACHE_HASH_SIZE = 256;

    static const uint32_t CACHE_NULL_ENTRY = CStaticList<SNode, CACHE_PAGES_CNT>::NULL_ITER;

public:
    static const size_t CANARY_VALUE = "CCache"_crs_hash;

public:
    CCache();
    explicit CCache(std::shared_ptr<CMemoryController> mem_controller_set);

    CCache             (const CCache&);
    CCache& operator = (const CCache&);

    CCache             (CCache&&);
    CCache& operator = (CCache&&);

    ~CCache();

    bool try_read (uint32_t addr,       UWord* dest_word_ptr) const;
    bool try_write(uint32_t addr, const UWord*  src_word_ptr);

    bool add_entry(uint32_t addr);

    void clear();

    [[nodiscard]] bool ok()   const noexcept;
                  bool dump() const noexcept;

    [[nodiscard]] size_t get_hash_value() const noexcept
    {
        size_t result = 0;

        result ^= std::hash<std::shared_ptr<CMemoryController>>()(mem_controller_);
        result ^= node_list_.get_hash_value_();

        result ^= reinterpret_cast<size_t>(entry_map_);
        result ^= reinterpret_cast<size_t>(byte_buf_);

        return result;
    }

private:
    bool is_entry_valid_(uint32_t page_entry) const noexcept
    {
        return node_list_.is_iter_valid(page_entry);
    }

    THash get_hash_(TPage page) const noexcept
    {
        page ^= 0xbad7face;
        uint8_t result = (0xFF & (page << 0x00)) ^
                         (0xFF & (page << 0x08)) ^
                         (0xFF & (page << 0x10)) ^
                         (0xFF & (page << 0x18));

        return result;
    }

    EMemoryResult write_entry_(uint32_t page_entry) const;
    EMemoryResult fetch_entry_(uint32_t page_entry);

    TListIter find_page_entry(TPage page) const;
    TAddr     free_page_entry(TListIter page_entry);

    TListIter find_less_used_page_entry() const;
    TListIter find_most_used_page_entry() const;
    TListIter find_last_used_page_entry() const;
    TListIter find_near_used_page_entry() const;

private:
    CRS_IF_CANARY_GUARD(size_t beg_canary_;)
    CRS_IF_HASH_GUARD  (size_t hash_value_;)

    std::shared_ptr<CMemoryController> mem_controller_;

    CStaticList<SNode, CACHE_PAGES_CNT> node_list_;
    uint32_t                            entry_map_[CACHE_HASH_SIZE];

    uint8_t byte_buf_[CACHE_PAGES_CNT * MEM_PAGE_SIZE];

    CRS_IF_CANARY_GUARD(size_t end_canary_;)
};

CCache::CCache():
    CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
    CRS_IF_HASH_GUARD  (hash_value_(0),)

    mem_controller_(),
    node_list_(),
    entry_map_{},
    byte_buf_{}

    CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
{
    for (size_t i = 0; i < CACHE_HASH_SIZE; ++i)
        entry_map_[i] = CACHE_NULL_ENTRY;
}

CCache::CCache(std::shared_ptr<CMemoryController> mem_controller_set):
    CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
    CRS_IF_HASH_GUARD  (hash_value_(0),)

    mem_controller_(std::move(mem_controller_set)),
    node_list_(),
    entry_map_{},
    byte_buf_{}

    CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
{
    for (size_t i = 0; i < CACHE_HASH_SIZE; ++i)
        entry_map_[i] = CACHE_NULL_ENTRY;

    CRS_CONSTRUCT_CHECK()
}

CCache::CCache(const CCache& assign_cache):
    CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
    CRS_IF_HASH_GUARD  (hash_value_(0),)

    mem_controller_(assign_cache.mem_controller_),
    node_list_     (assign_cache.node_list_),
    entry_map_{},
    byte_buf_{}

    CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
{
    memcpy(entry_map_, assign_cache.entry_map_, sizeof(entry_map_[0]) * CACHE_HASH_SIZE);
    memcpy(byte_buf_,  assign_cache.byte_buf_,  sizeof(byte_buf_[0])  * CACHE_PAGES_CNT*MEM_PAGE_SIZE);

    CRS_CONSTRUCT_CHECK()
}

CCache& CCache::operator = (const CCache& assign_cache)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    mem_controller_ = assign_cache.mem_controller_;
    node_list_      = assign_cache.node_list_;

    memcpy(entry_map_, assign_cache.entry_map_, sizeof(entry_map_[0]) * CACHE_HASH_SIZE);
    memcpy(byte_buf_,  assign_cache.byte_buf_,  sizeof(byte_buf_[0])  * CACHE_PAGES_CNT*MEM_PAGE_SIZE);

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return *this;
}

CCache::CCache(CCache&& move_cache):
    CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
    CRS_IF_HASH_GUARD  (hash_value_(0),)

    mem_controller_(std::move(move_cache.mem_controller_)),
    node_list_     (std::move(move_cache.node_list_)),
    entry_map_{},
    byte_buf_{}

    CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
{
    std::swap(entry_map_, move_cache.entry_map_);
    std::swap(byte_buf_,  move_cache.byte_buf_);

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value();)
    CRS_CONSTRUCT_CHECK()
}

CCache& CCache::operator = (CCache&& move_cache)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    mem_controller_ = std::move(move_cache.mem_controller_);
    node_list_      = std::move(move_cache.node_list_);

    std::swap(entry_map_, move_cache.entry_map_);
    std::swap(byte_buf_,  move_cache.byte_buf_);

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return *this;
}

CCache::~CCache()
{
    CRS_DESTRUCT_CHECK()

    mem_controller_ = nullptr;
    node_list_.clear();

    memset(entry_map_, 0xFF, sizeof(entry_map_[0]) * CACHE_HASH_SIZE);
    memset(byte_buf_,  0xFF, sizeof(byte_buf_[0])  * CACHE_PAGES_CNT*MEM_PAGE_SIZE);
}

bool CCache::try_read(uint32_t addr, UWord* dest_word_ptr) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    bool result = false;

    if (dest_word_ptr != nullptr)
    {
        uint32_t memory_page = addr / MEM_PAGE_SIZE;
        uint32_t page_offset = addr % MEM_PAGE_SIZE;

        uint32_t page_entry = find_page_entry(memory_page);

        if (page_entry != CACHE_NULL_ENTRY)
        {
            std::memcpy(dest_word_ptr, byte_buf_ + (MEM_PAGE_SIZE * page_entry + page_offset), sizeof(UWord));
            node_list_.get_elem(page_entry).freq++;

            result = true;
        }
        else
            result = false;
    }
    else
        CRS_PROCESS_ERROR("CCache::try_read error: dest is %p", dest_word_ptr)

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CCache::try_write(uint32_t addr, const UWord* src_word_ptr)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    bool result = false;

    if (src_word_ptr != nullptr)
    {
        uint32_t memory_page = addr / MEM_PAGE_SIZE;
        uint32_t page_offset = addr % MEM_PAGE_SIZE;

        uint32_t page_entry = find_page_entry(memory_page);

        if (page_entry != CACHE_NULL_ENTRY)
        {
            std::memcpy(byte_buf_ + (MEM_PAGE_SIZE * page_entry + page_offset), src_word_ptr, sizeof(UWord));
            node_list_.get_elem(page_entry).freq++;
            node_list_.get_elem(page_entry).flag |= EPageFlag::NODE_DIRTY;

            result = true;
        }
        else
            result = false;
    }
    else
        CRS_PROCESS_ERROR("CCache::try_write error: src_word_ptr = %p", src_word_ptr)

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CCache::add_entry(uint32_t addr)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    bool result = false;

    uint32_t memory_page = addr / MEM_PAGE_SIZE;
    uint32_t page_offset = addr % MEM_PAGE_SIZE;

    uint32_t page_entry = find_page_entry(memory_page);

    if (page_entry != CACHE_NULL_ENTRY)
        result = true;
    else
    {
        if (node_list_.full())
            free_page_entry(find_less_used_page_entry());

        page_entry = entry_map_[get_hash_(addr)];

        if (page_entry != CACHE_NULL_ENTRY)
        {
            page_entry = node_list_.insert(page_entry, SNode{.page = memory_page, .flag = 0, .freq = 0});
            result = (page_entry != CACHE_NULL_ENTRY);
        }
        else
        {
            page_entry = node_list_.push_front(SNode{.page = memory_page, .flag = 0, .freq = 0});
            result = (page_entry != CACHE_NULL_ENTRY);
        }
    }

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

void CCache::clear()
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    for (uint32_t i = 0; i < CACHE_HASH_SIZE; ++i)
        entry_map_[i] = CACHE_NULL_ENTRY;

    node_list_.clear();

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value();)
    CRS_IF_GUARD(CRS_END_CHECK();)
}

EMemoryResult CCache::write_entry_(uint32_t page_entry) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    EMemoryResult result = EMemoryResult::MEM_RES_SUCCESS;

    if (!is_entry_valid_(page_entry))
        CRS_PROCESS_ERROR("CCache::write_entry_ error: iter is not valid: %#X", page_entry)

    uint32_t page = node_list_.get_elem(page_entry).page;
    result = mem_controller_->mem_write_page(page, byte_buf_ + (MEM_PAGE_SIZE * page_entry));

    if (result == EMemoryResult::MEM_RES_SUCCESS)
        node_list_.get_elem(page_entry).flag = EPageFlag::NODE_EQUAL;

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

EMemoryResult CCache::fetch_entry_(uint32_t page_entry)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    EMemoryResult result = EMemoryResult::MEM_RES_SUCCESS;

    if (!is_entry_valid_(page_entry))
        CRS_PROCESS_ERROR("CCache::fetch_entry_ error: iter is not valid: %#X", page_entry)

    uint32_t page = node_list_.get_elem(page_entry).page;
    result = mem_controller_->mem_fetch_page(page, byte_buf_ + (MEM_PAGE_SIZE * page_entry));

    if (result == EMemoryResult::MEM_RES_SUCCESS)
        node_list_.get_elem(page_entry).flag = EPageFlag::NODE_EQUAL;

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

uint32_t CCache::find_page_entry(uint32_t page) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    uint32_t result = CACHE_NULL_ENTRY;
    uint8_t page_hash = get_hash_(page);

    uint32_t iter = entry_map_[page_hash];
    while ((iter != CACHE_NULL_ENTRY) && (get_hash_(node_list_.get_elem(iter).page) == page_hash) &&
           node_list_.get_elem(iter).page != page)
    {
        iter = node_list_.get_next_it(iter);
    }

    if ((iter != CACHE_NULL_ENTRY) && node_list_.get_elem(iter).page == page)
        result = iter;

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

uint32_t CCache::find_less_used_page_entry() const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)

    uint32_t result = node_list_.used_it();

    uint32_t cur_it = result;
    while (cur_it != CACHE_NULL_ENTRY)
    {
        if (node_list_.get_elem(result).freq > node_list_.get_elem(cur_it).freq)
            result = cur_it;

        cur_it = node_list_.get_next_it(cur_it);
    }

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

TListIter CCache::free_page_entry(TListIter page_entry)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)
    uint32_t result = 0;

    if (!is_entry_valid_(page_entry))
        CRS_PROCESS_ERROR("CCache::free_page_entry error: page_entry is not valid: %#X", page_entry)

    if (node_list_.get_elem(page_entry).flag & EPageFlag::NODE_DIRTY)
        write_entry_(page_entry);

    node_list_.erase(page_entry);

    CRS_IF_HASH_GUARD(hash_value_ = get_hash_value();)
    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CCache::ok() const noexcept
{
     return static_cast<bool>(mem_controller_) && mem_controller_->ok() &&
            node_list_.ok() && static_cast<bool>(entry_map_) &&
            static_cast<bool>(byte_buf_);
}

bool CCache::dump() const noexcept
{
    CRS_STATIC_DUMP("CCache[%s, this : %p] \n"
                    "{ \n"
                    CRS_IF_CANARY_GUARD(
                    "    beg_canary_[%s] : %#X \n"
                    ) //CRS_IF_CANARY_GUARD
                    CRS_IF_HASH_GUARD(
                    "    hash_value_[%s] : %#X \n"
                    ) //CRS_IF_HASH_GUARD
                    "    \n"
                    "    mem_controller_ : %p \n"
                    "    node_list_ : \n"
                    "    (",

                    (ok() ? "OK" : "ERROR"), this,
                    CRS_IF_CANARY_GUARD((beg_canary_ == CANARY_VALUE      ? "OK" : "ERROR"), beg_canary_,)
                    CRS_IF_HASH_GUARD  ((hash_value_ == get_hash_value() ? "OK" : "ERROR"), hash_value_,)

                    mem_controller_.get());

    node_list_.dump();

    CRS_STATIC_DUMP("    )"
                    "    entry_map_ : %p \n"
                    "    byte_buf_  : %p \n"
                    "    \n"
                    CRS_IF_CANARY_GUARD(
                    "    end_canary_[%s] : %#X \n"
                    ) //CRS_IF_CANARY_GUARD
                    "} \n",

                    entry_map_,
                    byte_buf_

                    CRS_IF_CANARY_GUARD(, (end_canary_ == CANARY_VALUE ? "OK" : "ERROR"), end_canary_));

    return ok();
}

} //namespace course

#endif //PROCESSOR_CACHE_H
