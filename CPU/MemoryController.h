//
// Created by geome_try on 9/13/19.
//

#ifndef MEMORY_CONTROLLER_H_INCLUDED
#define MEMORY_CONTROLLER_H_INCLUDED

#include <memory>

#include "../Logger/Logger.h"
#include "../Logger/Guard.h"

#include "IOController.h"

#include "ProcessorEnums.h"
#include "MemoryEnums.h"

namespace course {

using course_util::CCourseException;
using course_util::CLogger;
using course_util::operator""_crs_hash;

//TODO: move singleton functionality to mixin class
class CMemoryController final
{
    static const size_t CANARY_VALUE = "CMemoryController"_crs_hash;

public:
    enum EPageDest
    {
        ADDR_RAM = 0x0,
        ADDR_GPU = 0x1,
        ADDR_IO  = 0x2
    };

    struct SPhysPage
    {
        EPageDest dest;
        TPage page;
    };

    static const size_t RAM_BUF_PAGES_CNT = (0x1000) / MEM_PAGE_SIZE;
    static const size_t GPU_BUF_PAGES_CNT = (4*0x400*0x1000) / MEM_PAGE_SIZE;

public:
    CMemoryController             (const CMemoryController&) = delete;
    CMemoryController& operator = (const CMemoryController&) = delete;

    CMemoryController             (CMemoryController&&) = delete;
    CMemoryController& operator = (CMemoryController&&) = delete;

    ~CMemoryController() = default;

    static std::shared_ptr<CMemoryController> get_instance() noexcept
    {
        if (!instance_)
            set_instance();

        return instance_;
    }

    template<typename... Types>
    static void set_instance(Types&&... args) noexcept
    {
        instance_.reset();
        instance_ = std::shared_ptr<CMemoryController>(new CMemoryController(std::forward<Types>(args)...));
    }

    EMemoryResult mem_read_word (uint32_t addr,       UWord* dest) const;
    EMemoryResult mem_write_word(uint32_t addr, const UWord* src);

    EMemoryResult mem_write_page(TPage page, const uint8_t* buf);
    EMemoryResult mem_fetch_page(TPage page,       uint8_t* buf) const;

    [[nodiscard]] bool ok()   const noexcept;
                  bool dump() const noexcept;

private:
    CMemoryController();
    CMemoryController(const std::shared_ptr<CIOController>& io_controller_set);

    size_t get_hash_value_() const noexcept
    {
        return size_t(ram_buffer_) ^ size_t(gpu_buffer_) ^ size_t(io_controller_.get());
    }

//TODO: implement virtual addressing
/*
    EMemoryResult write_phys_page_(SPhysPage phys_page, const uint8_t* buf);
    EMemoryResult fetch_phys_page_(SPhysPage phys_page,       uint8_t* buf) const;
*/
private:
    static std::shared_ptr<CMemoryController> instance_;

    CRS_IF_CANARY_GUARD(size_t beg_canary_;)
    CRS_IF_HASH_GUARD  (size_t hash_value_;)

    std::shared_ptr<CIOController> io_controller_;

    uint8_t ram_buffer_[MEM_PAGE_SIZE*RAM_BUF_PAGES_CNT];
    uint8_t gpu_buffer_[MEM_PAGE_SIZE*GPU_BUF_PAGES_CNT];

    CRS_IF_CANARY_GUARD(size_t end_canary_;)
};

std::shared_ptr<CMemoryController> CMemoryController::instance_ = CMemoryController::get_instance();

CMemoryController::CMemoryController():
    CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
    CRS_IF_HASH_GUARD  (hash_value_(0),)

    io_controller_(nullptr),
    ram_buffer_{}, gpu_buffer_{}

    CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
{
    CRS_CHECK_MEM_OPER(memset(ram_buffer_, 0xFF, MEM_PAGE_SIZE*RAM_BUF_PAGES_CNT))
    CRS_CHECK_MEM_OPER(memset(gpu_buffer_, 0xFF, MEM_PAGE_SIZE*GPU_BUF_PAGES_CNT))

    CRS_CONSTRUCT_CHECK()
}

CMemoryController::CMemoryController(const std::shared_ptr<CIOController>& io_controller_set):
    CRS_IF_CANARY_GUARD(beg_canary_(CANARY_VALUE),)
    CRS_IF_HASH_GUARD  (hash_value_(0),)

    io_controller_(io_controller_set),
    ram_buffer_{}, gpu_buffer_{}

    CRS_IF_CANARY_GUARD(, end_canary_(CANARY_VALUE))
{
    CRS_CHECK_MEM_OPER(memset(ram_buffer_, 0xFF, MEM_PAGE_SIZE*RAM_BUF_PAGES_CNT))
    CRS_CHECK_MEM_OPER(memset(gpu_buffer_, 0xFF, MEM_PAGE_SIZE*GPU_BUF_PAGES_CNT))

    CRS_CONSTRUCT_CHECK()
}

EMemoryResult CMemoryController::mem_read_word(uint32_t addr, UWord* dest) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)
    EMemoryResult result = EMemoryResult::MEM_RES_SUCCESS;

    memcpy(dest, ram_buffer_ + addr, sizeof(UWord));

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

EMemoryResult CMemoryController::mem_write_word(uint32_t addr, const UWord* src)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)
    EMemoryResult result = EMemoryResult::MEM_RES_SUCCESS;

    memcpy(ram_buffer_ + addr, src, sizeof(UWord));

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

EMemoryResult CMemoryController::mem_write_page(TPage page, const uint8_t* buf)
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)
    EMemoryResult result = EMemoryResult::MEM_RES_SUCCESS;

    if (!buf)
        CRS_PROCESS_ERROR("CMemoryController::mem_write_page error: invalid buf: %p", buf)

    if (page < RAM_BUF_PAGES_CNT)
    {
        memcpy(ram_buffer_ + (MEM_PAGE_SIZE * page), buf, MEM_PAGE_SIZE);
        result = EMemoryResult::MEM_RES_SYGSEGV;
    }
    else
        result = EMemoryResult::MEM_RES_SYGSEGV;

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

EMemoryResult CMemoryController::mem_fetch_page(TPage page, uint8_t* buf) const
{
    CRS_IF_GUARD(CRS_BEG_CHECK();)
    EMemoryResult result = EMemoryResult::MEM_RES_SUCCESS;

    if (!buf)
        CRS_PROCESS_ERROR("CMemoryController::mem_fetch_page error: invalid buf: %p", buf)

    if (page < RAM_BUF_PAGES_CNT)
    {
        memcpy(buf, ram_buffer_ + (MEM_PAGE_SIZE * page), MEM_PAGE_SIZE);
        result = EMemoryResult::MEM_RES_SYGSEGV;
    }
    else
        result = EMemoryResult::MEM_RES_SYGSEGV;

    CRS_IF_GUARD(CRS_END_CHECK();)

    return result;
}

bool CMemoryController::ok() const noexcept
{
    //TODO: add && io_controller_
    return ram_buffer_ && gpu_buffer_;
}

bool CMemoryController::dump() const noexcept
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
                    "    ram_buffer_     : %p"
                    "    gpu_buffer_     : %p"
                    "    \n"
                    CRS_IF_CANARY_GUARD(
                    "    end_canary_[%s] : %#X \n"
                    ) //CRS_IF_CANARY_GUARD
                    "} \n",

                    (ok() ? "OK" : "ERROR"), this,
                    CRS_IF_CANARY_GUARD((beg_canary_ == CANARY_VALUE      ? "OK" : "ERROR"), beg_canary_,)
                    CRS_IF_HASH_GUARD  ((hash_value_ == get_hash_value_() ? "OK" : "ERROR"), hash_value_,)
/*
                    instance_.get(),
                    io_controller_.get()
*/
                    ram_buffer_,
                    gpu_buffer_

                    CRS_IF_CANARY_GUARD(, (end_canary_ == CANARY_VALUE ? "OK" : "ERROR"), end_canary_));

    return ok();
}

} //namespace course

#endif //MEMORY_CONTROLLER_H_INCLUDED
