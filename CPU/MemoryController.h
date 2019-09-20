//
// Created by geome_try on 9/13/19.
//

#ifndef MEMORY_CONTROLLER_H_INCLUDED
#define MEMORY_CONTROLLER_H_INCLUDED

#include <memory>
#include "IOController.h"

namespace course {

//TODO: move singleton functionality to mixin class
class CMemoryController final
{
public:
    CMemoryController() = default;

    CMemoryController             (const CMemoryController&) = delete;
    CMemoryController& operator = (const CMemoryController&) = delete;

    //TODO: to implement move-semantics ("rule of 5" dummy realisation)
    CMemoryController             (CMemoryController&&) = delete;
    CMemoryController& operator = (CMemoryController&&) = delete;

    ~CMemoryController() = default;

    uint32_t get_

private:
    [[nodiscard]] bool ok_() const noexcept;
    bool dump_() const noexcept;

    static std::shared_ptr<CMemoryController> instance_;

private:
    std::shared_ptr<CIOController> io_controller_;

};

} //namespace course

#endif //MEMORY_CONTROLLER_H_INCLUDED
