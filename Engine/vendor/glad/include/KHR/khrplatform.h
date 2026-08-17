#ifndef __khrplatform_h_
#define __khrplatform_h_

#if defined(_WIN32) && !defined(__SC__) && !defined(__GCC__)
typedef signed   __int64   khronos_int64_t;
typedef unsigned __int64   khronos_uint64_t;
#elif defined(__GNUC__) || defined(__clang__)
#include <stdint.h>
typedef int64_t            khronos_int64_t;
typedef uint64_t           khronos_uint64_t;
#else
typedef signed   long long khronos_int64_t;
typedef unsigned long long khronos_uint64_t;
#endif

typedef signed   int       khronos_int32_t;
typedef unsigned int       khronos_uint32_t;
typedef signed   short int khronos_int16_t;
typedef unsigned short int khronos_uint16_t;
typedef signed   char      khronos_int8_t;
typedef unsigned char      khronos_uint8_t;
typedef signed   long int  khronos_intptr_t;
typedef unsigned long int  khronos_uintptr_t;
typedef signed   long int  khronos_ssize_t;
typedef unsigned long int  khronos_usize_t;
typedef float              khronos_float_t;
typedef khronos_uint64_t   khronos_utime_nanoseconds_t;
typedef khronos_int64_t    khronos_stime_nanoseconds_t;

#endif /* __khrplatform_h_ */
