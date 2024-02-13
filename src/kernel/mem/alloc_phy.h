#ifndef _ALLOC_PHYS_H_
#define _ALLOC_PHYS_H_

#include "desc.h"

#define DEFAULT_MINIMUM_ALIGNMENT 64

#define INVALID_PHYS_ADDR	(~(phys_addr_t)0)

#define PRId32  	"d"
#define PRIu32  	"u"
#define PRIx32  	"x"


extern int printf(const char *fmt, ...);

/*
 * phys_alloc_init creates the initial free memory region of size @size
 * at @base. The minimum alignment is set to DEFAULT_MINIMUM_ALIGNMENT.
 */
extern void phys_alloc_init(phys_addr_t base, phys_addr_t size);

/*
 * phys_alloc_set_minimum_alignment sets the minimum alignment to
 * @align.
 */
extern void phys_alloc_set_minimum_alignment(phys_addr_t align);

/*
 * phys_alloc_show outputs all currently allocated regions with the
 * following format
 *   <start_addr>-<end_addr> [<USED|FREE>]
 */
extern void phys_alloc_show(void);

/*
 * phys_alloc_get_unused allocates all remaining memory from the region
 * passed to phys_alloc_init, returning the newly allocated memory's base
 * and top addresses. phys_allo_get_unused will still return base and top
 * when no free memory is remaining, but base will equal top.
 */
extern void phys_alloc_get_unused(phys_addr_t *p_base, phys_addr_t *p_top);

#endif /* _ALLOC_PHYS_H_ */