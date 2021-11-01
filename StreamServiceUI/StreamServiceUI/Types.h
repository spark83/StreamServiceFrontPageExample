/*
 *! \brief Types used throughout the code
 *! \author Sang Park
 *! \date Oct 2021
 */
#ifndef TYPES_H
#define TYPES_H

#if !defined NULL
#define NULL 0
#endif

#define EPSILON 1.192092896e-07F

#ifdef __cplusplus
extern "C" {
#endif

typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef __int64 s64;
typedef unsigned __int64 u64;
typedef float f32;
typedef double f64;

#ifdef __cplusplus
}
#endif

#endif
