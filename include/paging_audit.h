#ifndef NEXUS_PAGING_AUDIT_H
#define NEXUS_PAGING_AUDIT_H

#include <stdint.h>
#include "limine.h"

#define NEXUS_X86_PAGE_SIZE 0x1000ULL
#define NEXUS_X86_2M_PAGE_SIZE 0x200000ULL
#define NEXUS_X86_1G_PAGE_SIZE 0x40000000ULL
#define NEXUS_X86_PHYSICAL_ADDRESS_MASK 0x000ffffffffff000ULL

enum nexus_paging_walk_status {
    NEXUS_PAGING_WALK_ERROR = 0,
    NEXUS_PAGING_WALK_UNMAPPED = 1,
    NEXUS_PAGING_WALK_MAPPED = 2
};

struct nexus_paging_mapping {
    uint64_t physical_base;
    uint64_t page_size;
    uint64_t level;
    uint64_t entry;
};

int nexus_x86_virtual_is_canonical(uint64_t address, uint64_t virtual_bits);
int nexus_paging_cr3_physical(uint64_t cr3, uint64_t *physical);
int nexus_paging_entry_physical(uint64_t entry, uint64_t *physical);
int nexus_paging_large_page_size(uint64_t entry, uint64_t level,
                                 uint64_t *page_size);
int nexus_paging_physical_to_hhdm(uint64_t hhdm_offset, uint64_t physical,
                                  uint64_t virtual_bits, uint64_t *virtual_address);
int nexus_paging_find_mapping(const struct limine_memmap_response *memory_map,
                              uint64_t hhdm_offset, uint64_t cr0, uint64_t cr3,
                              uint64_t cr4, uint64_t efer,
                              uint64_t virtual_address,
                              struct nexus_paging_mapping *mapping);

#endif
