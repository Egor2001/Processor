//
// Created by geome_try on 9/26/19.
//

#ifndef PROCESSOR_MEMORYENUMS_H
#define PROCESSOR_MEMORYENUMS_H

#include <cstdint>
#include <cstddef>

namespace course {

const size_t   MEM_PAGE_SIZE = 64;
const uint32_t MEM_NULL_ADDR = 0;

typedef uint32_t TAddr;
typedef uint32_t TPage;
typedef uint8_t  THash;

enum EPageFlag
{
    NODE_CLEAR = 0x1,
    NODE_EQUAL = 0x2,
    NODE_DIRTY = 0x4,
    NODE_ERROR = 0x8000
};

enum EMemoryResult
{
    MEM_RES_SUCCESS = 0x0,
    MEM_RES_SYGSEGV = 0x1
};


} //namespace course

#endif //PROCESSOR_MEMORYENUMS_H
