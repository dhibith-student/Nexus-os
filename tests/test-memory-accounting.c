#include <stdint.h>
#include "memory_accounting.h"

int main(void) {
    struct limine_memmap_entry first = { .base = 0x1000, .length = 0x2000, .type = LIMINE_MEMMAP_USABLE };
    struct limine_memmap_entry second = { .base = 0x100000, .length = 0x3000, .type = LIMINE_MEMMAP_RESERVED };
    struct limine_memmap_entry third = { .base = 0x200000, .length = 0x4000, .type = LIMINE_MEMMAP_USABLE };
    struct limine_memmap_entry *entries[] = { &first, &second, &third };
    struct limine_memmap_response response = { .entry_count = 3, .entries = entries };
    struct nexus_memory_summary summary;
    if (!nexus_memory_accounting(&response, &summary)) return 1;
    if (summary.usable_regions != 2 || summary.usable_bytes != 0x6000) return 2;
    struct limine_memmap_entry overflow = { .base = 0, .length = ~(uint64_t)0, .type = LIMINE_MEMMAP_USABLE };
    struct limine_memmap_entry one = { .base = 0, .length = 1, .type = LIMINE_MEMMAP_USABLE };
    struct limine_memmap_entry *overflow_entries[] = { &overflow, &one };
    struct limine_memmap_response overflow_response = { .entry_count = 2, .entries = overflow_entries };
    if (nexus_memory_accounting(&overflow_response, &summary)) return 3;
    struct limine_memmap_response invalid = { .entry_count = 1, .entries = 0 };
    if (nexus_memory_accounting(&invalid, &summary)) return 4;
    return 0;
}
