#if !defined(_cnake_H)
#define _cnake_H (1)

#include <sys/types.h>

typedef float f32;
typedef double f64;
typedef u_int8_t u8;
typedef u_int16_t u16;
typedef u_int32_t u32;
typedef u_int64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef size_t usize;
typedef ssize_t isize;

void cnake(void);
#define 🐍 cnake();
#include <krad/app/debug.h>
#include "stdheaders.h"

#endif
