#ifndef GUARD_H_INCLUDED
#define GUARD_H_INCLUDED

#if CRS_GUARD_LEVEL >= 3
    #define   CRS_IF_HASH_GUARD(...) __VA_ARGS__
    #define CRS_IF_CANARY_GUARD(...) __VA_ARGS__
    #define        CRS_IF_GUARD(...) __VA_ARGS__

#elif CRS_GUARD_LEVEL == 2
    #define   CRS_IF_HASH_GUARD(...)
    #define CRS_IF_CANARY_GUARD(...) __VA_ARGS__
    #define        CRS_IF_GUARD(...) __VA_ARGS__

#elif CRS_GUARD_LEVEL == 1
    #define   CRS_IF_HASH_GUARD(...)
    #define CRS_IF_CANARY_GUARD(...)
    #define        CRS_IF_GUARD(...) __VA_ARGS__

#else
    #define   CRS_IF_HASH_GUARD(...)
    #define CRS_IF_CANARY_GUARD(...)
    #define        CRS_IF_GUARD(...)
#endif // CRS_GUARD_LEVEL

#endif // GUARD_H_INCLUDED
