#include <stdint.h>
#include "paging_audit.h"

int main(void) {
    uint64_t value;
    if (!nexus_x86_virtual_is_canonical(0x00007fffffffffffULL, 48)) return 1;
    if (!nexus_x86_virtual_is_canonical(0xffff800000000000ULL, 48)) return 2;
    if (nexus_x86_virtual_is_canonical(0x0000800000000000ULL, 48)) return 3;
    if (nexus_x86_virtual_is_canonical(0x0001000000000000ULL, 48)) return 4;
    if (!nexus_x86_virtual_is_canonical(0x00ffffffffffffffULL, 57)) return 5;
    if (nexus_x86_virtual_is_canonical(0x0100000000000000ULL, 57)) return 6;
    if (!nexus_paging_cr3_physical(0x0000000007f84017ULL, &value) ||
        value != 0x0000000007f84000ULL) return 7;
    if (nexus_paging_cr3_physical(0x8000000000001000ULL, &value)) return 8;
    if (nexus_paging_entry_physical(0x8000000000000000ULL, &value)) return 9;
    if (!nexus_paging_entry_physical(0x0000000000001001ULL, &value) ||
        value != 0x1000) return 10;
    if (!nexus_paging_large_page_size(0x0000000000200081ULL, 2, &value) ||
        value != NEXUS_X86_2M_PAGE_SIZE) return 12;
    if (nexus_paging_large_page_size(0x0000000000300081ULL, 2, &value)) return 12;
    if (nexus_paging_large_page_size(0x0000000000000081ULL, 1, &value)) return 13;
    if (!nexus_paging_physical_to_hhdm(0xffff800000000000ULL, 0x7f84000, 48, &value) ||
        value != 0xffff800007f84000ULL) return 14;
    if (nexus_paging_physical_to_hhdm(0xfffffffffffff000ULL, 0x1000ULL, 48, &value)) return 15;
    struct nexus_paging_mapping mapping;
    struct limine_memmap_response empty = { .entry_count = 0, .entries = (void *)1 };
    if (nexus_paging_find_mapping(0, 0, 0, 0, 0, 0, 0, &mapping) != NEXUS_PAGING_WALK_ERROR) return 16;
    if (nexus_paging_find_mapping(&empty, 0, 0, 1ULL << 12, 1ULL << 31,
                                  1ULL << 10, 0, &mapping) != NEXUS_PAGING_WALK_ERROR) return 17;
    if (nexus_x86_virtual_is_canonical(0, 46)) return 18;
    return 0;
}
