#ifndef BIOS_H
#define BIOS_H

#include "types.h"

#define BOOT_RAM_REGION_MAX			10		// RAM区最大数量

// reference https://www.jianshu.com/p/703402070090

typedef struct _e820map_t {
int nr_map;     // 1000 - 1004  4 Byte

struct {
long long addr;
long long size;
long type;
} map[BOOT_RAM_REGION_MAX];     // 20 Byte

}e820map_t;

#endif