#include <stdint.h>
#include "paging_audit.h"
#include "memory_accounting.h"

static uint64_t max_value(void) {
    return ~(uint64_t)0;
}

static int range_end(uint64_t base, uint64_t length, uint64_t *end) {
    if (base > max_value() - length) return 0;
    *end = base + length;
    return 1;
}

static int type5_contains(const struct limine_memmap_response *map,
                          uint64_t base, uint64_t length) {
    uint64_t end;
    if (map == 0 || map->entries == 0 || !range_end(base, length, &end)) return 0;
    for (uint64_t index = 0; index < map->entry_count; ++index) {
        const struct limine_memmap_entry *entry = map->entries[index];
        uint64_t entry_end;
        if (entry == 0 || !range_end(entry->base, entry->length, &entry_end)) return 0;
        if (entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE &&
            entry->base <= base && end <= entry_end) return 1;
    }
    return 0;
}

int nexus_x86_virtual_is_canonical(uint64_t address, uint64_t virtual_bits) {
    uint64_t sign_bit;
    uint64_t upper_mask;
    if (virtual_bits != 48 && virtual_bits != 57) return 0;
    sign_bit = 1ULL << (virtual_bits - 1);
    upper_mask = ~((1ULL << virtual_bits) - 1);
    if ((address & sign_bit) == 0) return (address & upper_mask) == 0;
    return (address & upper_mask) == upper_mask;
}

int nexus_paging_cr3_physical(uint64_t cr3, uint64_t *physical) {
    uint64_t allowed = NEXUS_X86_PHYSICAL_ADDRESS_MASK | 0xfffULL;
    if (physical == 0 || (cr3 & ~allowed) != 0) return 0;
    *physical = cr3 & NEXUS_X86_PHYSICAL_ADDRESS_MASK;
    return 1;
}

int nexus_paging_entry_physical(uint64_t entry, uint64_t *physical) {
    if (physical == 0 || (entry & 1ULL) == 0) return 0;
    *physical = entry & NEXUS_X86_PHYSICAL_ADDRESS_MASK;
    return 1;
}

int nexus_paging_large_page_size(uint64_t entry, uint64_t level,
                                 uint64_t *page_size) {
    uint64_t physical;
    uint64_t size;
    if (page_size == 0 || !nexus_paging_entry_physical(entry, &physical)) return 0;
    if ((entry & (1ULL << 7)) == 0) return 0;
    if (level == 3) size = NEXUS_X86_1G_PAGE_SIZE;
    else if (level == 2) size = NEXUS_X86_2M_PAGE_SIZE;
    else return 0;
    if ((physical & (size - 1)) != 0) return 0;
    *page_size = size;
    return 1;
}

int nexus_paging_physical_to_hhdm(uint64_t hhdm_offset, uint64_t physical,
                                  uint64_t virtual_bits, uint64_t *virtual_address) {
    if (virtual_address == 0 || physical > max_value() - hhdm_offset) return 0;
    *virtual_address = hhdm_offset + physical;
    return nexus_x86_virtual_is_canonical(*virtual_address, virtual_bits);
}

int nexus_paging_find_mapping(const struct limine_memmap_response *memory_map,
                              uint64_t hhdm_offset, uint64_t cr0, uint64_t cr3,
                              uint64_t cr4, uint64_t efer,
                              uint64_t virtual_address,
                              struct nexus_paging_mapping *mapping) {
    uint64_t physical;
    uint64_t table_virtual;
    uint64_t levels;
    volatile const uint64_t *table;
    if (mapping == 0 || memory_map == 0 || memory_map->entries == 0 ||
        memory_map->entry_count > NEXUS_MAX_MEMMAP_ENTRIES) return NEXUS_PAGING_WALK_ERROR;
    if ((cr0 & (1ULL << 31)) == 0 || (cr4 & (1ULL << 5)) == 0 ||
        (efer & (1ULL << 10)) == 0 ||
        (cr4 & (1ULL << 12)) != 0) return NEXUS_PAGING_WALK_ERROR;
    levels = 4;
    if (!nexus_x86_virtual_is_canonical(virtual_address, 48)) return NEXUS_PAGING_WALK_ERROR;
    if (!nexus_paging_cr3_physical(cr3, &physical) || physical == 0 ||
        (physical & (NEXUS_X86_PAGE_SIZE - 1)) != 0 ||
        !type5_contains(memory_map, physical, NEXUS_X86_PAGE_SIZE) ||
        !nexus_paging_physical_to_hhdm(hhdm_offset, physical, 48, &table_virtual)) {
        return NEXUS_PAGING_WALK_ERROR;
    }
    table = (volatile const uint64_t *)(uintptr_t)table_virtual;
    for (uint64_t level = levels; level != 0; --level) {
        uint64_t shift = 12 + 9 * (level - 1);
        uint64_t entry = table[(virtual_address >> shift) & 0x1ffULL];
        mapping->entry = entry;
        mapping->level = level;
        if ((entry & 1ULL) == 0) return NEXUS_PAGING_WALK_UNMAPPED;
        if (nexus_paging_large_page_size(entry, level, &mapping->page_size)) {
            mapping->physical_base = entry & NEXUS_X86_PHYSICAL_ADDRESS_MASK;
            return NEXUS_PAGING_WALK_MAPPED;
        }
        if (level == 1) {
            if (!nexus_paging_entry_physical(entry, &mapping->physical_base)) return NEXUS_PAGING_WALK_ERROR;
            mapping->page_size = NEXUS_X86_PAGE_SIZE;
            return NEXUS_PAGING_WALK_MAPPED;
        }
        if (!nexus_paging_entry_physical(entry, &physical) ||
            (physical & (NEXUS_X86_PAGE_SIZE - 1)) != 0 ||
            !type5_contains(memory_map, physical, NEXUS_X86_PAGE_SIZE) ||
            !nexus_paging_physical_to_hhdm(hhdm_offset, physical, 48, &table_virtual)) {
            return NEXUS_PAGING_WALK_ERROR;
        }
        table = (volatile const uint64_t *)(uintptr_t)table_virtual;
    }
    return NEXUS_PAGING_WALK_ERROR;
}
