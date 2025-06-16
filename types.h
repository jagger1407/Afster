#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

typedef unsigned char u8;
typedef char s8;

typedef unsigned short u16;
typedef short s16;

typedef unsigned int u32;
typedef int s32;

typedef unsigned long long u64;
typedef long long s64;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned long long ulonglong;
typedef unsigned int uint;

#include <stdbool.h>
#define false 0
#define true 1

#define nullptr ((void*)0x00)

#ifdef BUILDING
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif

#endif // TYPES_H_INCLUDED
