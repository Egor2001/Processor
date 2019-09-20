//
// Created by geome_try on 9/20/19.
//

#ifndef IOCONTROLLER_H_INCLUDED
#define IOCONTROLLER_H_INCLUDED

#include <memory>

namespace course {

//TODO: move singleton functionality to mixin class
class CIOController final
{
public:
    CIOController() = default;

    CIOController             (const CIOController&) = delete;
    CIOController& operator = (const CIOController&) = delete;

    //TODO: to implement move-semantics ("rule of 5" dummy realisation)
    CIOController             (CIOController&&) = delete;
    CIOController& operator = (CIOController&&) = delete;

    ~CIOController() = default;

private:
    [[nodiscard]] bool ok_() const noexcept;
    bool dump_() const noexcept;

private:
    static std::shared_ptr<CIOController> instance_;

private:
};

} //namespace course

#endif //IOCONTROLLER_H_INCLUDED
