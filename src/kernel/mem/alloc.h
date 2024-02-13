#ifndef ALLOC_H
#define ALLOC_H

#include "types.h"


#define __ALIGN_MASK(x, mask)	(((x) + (mask)) & ~(mask))
#define __ALIGN(x, a)		__ALIGN_MASK(x, (typeof(x))(a) - 1)
#define ALIGN(x, a)		__ALIGN((x), (a))



struct alloc_ops {
	void *(*memalign)(size_t alignment, size_t size);
	void (*free)(void *ptr, size_t size);
	size_t align_min;
};

extern struct alloc_ops * alloc_ops;

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);
void *memalign(size_t alignment, size_t size);

#endif /* ALLOC_H */