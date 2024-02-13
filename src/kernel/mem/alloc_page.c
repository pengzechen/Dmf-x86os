
#include "cfg.h"
#include "types.h"
#include "alloc.h"
#include "alloc_page.h"
#include "spin_lock.h"
#include "cfg.h"

int printf(const char *fmt, ...);

static struct spinlock lock;
static void *freelist = (void*)0x200000;

bool page_alloc_initialized(void) { return freelist != 0; }

/*
 * Allocates (1 << order) physically contiguous and naturally aligned pages.
 * Returns NULL if there's no memory left.
 */
void *alloc_pages(uint32_t order)
{
	/* Generic list traversal. */
	void *prev;
	void *curr = NULL;
	void *next = freelist;

	/* Looking for a run of length (1 << order). */
	uint32_t run = 0;
	const uint32_t n = 1ul << order;
	const uint32_t align_mask = (n << PAGE_SHIFT) - 1;
	void *run_start = NULL;
	void *run_prev = NULL;
	uint32_t run_next_pa = 0;
	uint32_t pa;

	assert(order < sizeof(uint32_t) * 8);

	spin_lock(&lock);
	for (;;) {
		prev = curr;
		curr = next;

		if (!curr) {
			run_start = NULL;
			break;
		}

		next = *((void **) curr);
		// pa = virt_to_phys(curr);
		pa = (uint32_t)curr;

		if (run == 0) {
			if (!(pa & align_mask)) {
				run_start = curr;
				run_prev = prev;
				run_next_pa = pa + PAGE_SIZE;
				run = 1;
			}
		} else if (pa == run_next_pa) {
			run_next_pa += PAGE_SIZE;
			run += 1;
		} else {
			run = 0;
		}

		if (run == n) {
			if (run_prev)
				*((void **) run_prev) = next;
			else
				freelist = next;
			break;
		}
	}
	spin_unlock(&lock);
	return run_start;
}

void free_pages(void *mem, uint32_t size)
{
	void *old_freelist;
	void *end;

	assert_msg((uint32_t) mem % PAGE_SIZE == 0, "mem not page aligned: %p", mem);

	assert_msg(size % PAGE_SIZE == 0, "size not page aligned: %#lx", size);

	assert_msg(size == 0 || (uintptr_t)mem == -size || (uintptr_t)mem + size > (uintptr_t)mem, "mem + size overflow: %p + %#lx", mem, size);

	if (size == 0) {
		freelist = NULL;
		return;
	}

	spin_lock(&lock);
	old_freelist = freelist;
	freelist = mem;
	end = (u_char*)mem + size;
	while ((u_char*)mem + PAGE_SIZE != end) {
		*(void **)mem = ((u_char*)mem + PAGE_SIZE);
		mem += PAGE_SIZE;
	}

	*(void **)mem = old_freelist;
	spin_unlock(&lock);
}

static void *page_memalign(size_t alignment, size_t size)
{
	uint32_t n = ALIGN(size, PAGE_SIZE) >> PAGE_SHIFT;
	uint32_t order;

	if (!size)
		return NULL;

	order = is_power_of_2(n) ? fls(n) : fls(n) + 1;

	return alloc_pages(order);
}

static void page_free(void *mem, size_t size)
{
	free_pages(mem, size);
}

static struct alloc_ops page_alloc_ops = {
	.memalign = page_memalign,
	.free = page_free,
	.align_min = PAGE_SIZE,
};

void page_alloc_ops_enable(void)
{
	alloc_ops = &page_alloc_ops;
}

void free_page(void *page)
{
	spin_lock(&lock);
	*(void **)page = freelist;
	freelist = page;
	spin_unlock(&lock);
}

void *alloc_page()
{
	void *p;

	if (!freelist)
		return 0;

	spin_lock(&lock);
	p = freelist;
	freelist = *(void **)freelist;
	spin_unlock(&lock);

	return p;
}