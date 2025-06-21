//
//  c_common.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/9/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef c_common_h
#define c_common_h

#include "types_common.h"

#ifdef __cplusplus
#define extern_link_begin() extern "C" {





#define extern_link_end() }
#else
#define extern_link_begin()
#define extern_link_end()
#endif

#ifdef __cplusplus
extern "C" {
#endif

isize gb_fprintf_va(char const *fmt, va_list va);

#ifdef __cplusplus
}
#endif

static inline isize gb_printf_err_va(char const *fmt, va_list va)
{
    return gb_fprintf_va(fmt, va);
}

static inline isize gb_printf_err(char const *fmt, ...)
{
    isize res;
    va_list va;
    va_start(va, fmt);
    res = gb_printf_err_va(fmt, va);
    va_end(va);
    return res;
}

static inline void gb_assert_handler(char const *prefix, char const *condition, char const *file, i32 line, char const *msg, ...)
{
#ifndef NDEBUG
    gb_printf_err("%s(%d): %s: ", file, line, prefix);
    if (condition)
        gb_printf_err( "`%s` ", condition);
    if (msg) {
        va_list va;
        va_start(va, msg);
        gb_printf_err_va(msg, va);
        va_end(va);
    }
    gb_printf_err("\n");
#endif
}

#ifndef NDEBUG
#define ASSERT_MSG(cond, msg, ...) do { \
if (!(cond)) { \
gb_assert_handler("Assertion Failure", #cond, __FILE__, (i64)__LINE__, msg, ##__VA_ARGS__); \
abort(); \
} \
} while (0)

#else

#define ASSERT_MSG(cond, msg, ...) do { \
} while (0) \

#endif

inline static uint64 modify_bit(uint64 n, uint64 p, uint64 b)
{
    uint64 mask = 1 << p;
    return (n & ~mask) | ((b << p) & mask);
}


#define bool_str(x) ((x)?"true":"false")
#define bool_name_str(x) ((x)? #x ":true": #x ":false")

inline static float64 num_ratio(uint64 x, uint64 y)
{
    return ((float64)x) / ((float64)y);
}


#endif /* c_common_h */
