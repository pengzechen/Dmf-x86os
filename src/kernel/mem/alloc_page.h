
#ifndef ALLOC_PAGE_H
#define ALLOC_PAGE_H 1
#include "types.h"
#define PAGE_SHIFT		12
#define PAGE_SIZE		4096

bool page_alloc_initialized(void);
void page_alloc_ops_enable(void);
void *alloc_page(void);
void *alloc_pages(uint32_t order);
void free_page(void *page);
void free_pages(void *mem, uint32_t size);

#endif // ALLOC_PAGE_H