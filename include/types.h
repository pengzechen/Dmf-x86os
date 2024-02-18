#ifndef TYPES_H
#define TYPES_H


/* -------------- Types -------------------------*/


typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef unsigned long long uint64_t;

typedef unsigned int        size_t;
typedef unsigned int        uintptr_t;      

typedef unsigned char       u_char;

#define NULL ((void *)0)

typedef _Bool		bool;
#define false 0
#define true  1


#define MIN(a, b)		((a) < (b) ? (a) : (b))
#define MAX(a, b)		((a) > (b) ? (a) : (b))

#endif // TYPES_H