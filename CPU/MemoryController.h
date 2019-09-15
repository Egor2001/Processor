//
// Created by geome_try on 9/13/19.
//

#ifndef MEMORY_CONTROLLER_H_INCLUDED
#define MEMORY_CONTROLLER_H_INCLUDED

namespace course {

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

private:
    [[nodiscard]] bool ok_() const noexcept;
    bool dump_() const noexcept;

private:

};

} //namespace course

#endif //MEMORY_CONTROLLER_H_INCLUDED
