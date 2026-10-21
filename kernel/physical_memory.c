#include <stdint.h>
#include "memory_accounting.h"
#include "physical_memory.h"

static uint64_t max_value(void) {
    return ~(uint64_t)0;
}

static int range_end(uint64_t base, uint64_t length, uint64_t *end) {
    if (base > max_value() - length) return 0;
    *end = base + length;
    return 1;
}

static int align_up(uint64_t value, uint64_t *aligned) {
    const uint64_t mask = NEXUS_PMM_FRAME_SIZE - 1;
    if (value > max_value() - mask) return 0;
    *aligned = (value + mask) & ~mask;
    return 1;
}

static uint64_t align_down(uint64_t value) {
    return value & ~(NEXUS_PMM_FRAME_SIZE - 1);
}

static int range_overlaps(uint64_t base, uint64_t end,
                          uint64_t other_base, uint64_t other_end) {
    return base < other_end && other_base < end;
}

static void bit_set(uint8_t *bitmap, uint64_t bit) {
    bitmap[bit / 8] |= (uint8_t)(1U << (bit % 8));
}

static void bit_clear(uint8_t *bitmap, uint64_t bit) {
    bitmap[bit / 8] &= (uint8_t)~(1U << (bit % 8));
}

static int bit_is_set(const uint8_t *bitmap, uint64_t bit) {
    return (bitmap[bit / 8] & (uint8_t)(1U << (bit % 8))) != 0;
}

static int find_frame(const struct nexus_pmm_allocator *allocator,
                      uint64_t *region_index, uint64_t *frame_index) {
    for (uint64_t region = 0; region < allocator->region_count; ++region) {
        const struct nexus_pmm_region *entry = &allocator->regions[region];
        for (uint64_t frame = 0; frame < entry->frame_count; ++frame) {
            uint64_t bit = entry->bitmap_bit_offset + frame;
            if (!bit_is_set(allocator->allocation_bitmap, bit)) {
                *region_index = region;
                *frame_index = frame;
                return 1;
            }
        }
    }
    return 0;
}

int nexus_pmm_init(struct nexus_pmm_allocator *allocator,
                   const struct limine_memmap_response *response,
                   const struct nexus_pmm_reserved_range *reserved_ranges,
                   uint64_t reserved_count,
                   uint8_t *allocation_bitmap,
                   uint8_t *reserved_bitmap,
                   uint64_t bitmap_bytes) {
    const uint64_t max = max_value();
    if (allocator == 0 || response == 0 || reserved_count > NEXUS_PMM_MAX_RESERVED_RANGES ||
        (reserved_count != 0 && reserved_ranges == 0)) return 0;
    if (response->entry_count > NEXUS_MAX_MEMMAP_ENTRIES ||
        (response->entry_count != 0 && response->entries == 0)) return 0;
    if (allocation_bitmap == 0 || reserved_bitmap == 0 ||
        bitmap_bytes > max / 8ULL) return 0;
    for (uint64_t range = 0; range < reserved_count; ++range) {
        uint64_t reserved_end;
        if (!range_end(reserved_ranges[range].base,
                       reserved_ranges[range].length, &reserved_end)) return 0;
    }

    allocator->region_count = 0;
    allocator->total_frames = 0;
    allocator->allocation_bitmap = allocation_bitmap;
    allocator->reserved_bitmap = reserved_bitmap;
    allocator->bitmap_bytes = bitmap_bytes;
    for (uint64_t byte = 0; byte < bitmap_bytes; ++byte) {
        allocation_bitmap[byte] = 0;
        reserved_bitmap[byte] = 0;
    }

    for (uint64_t index = 0; index < response->entry_count; ++index) {
        const struct limine_memmap_entry *entry = response->entries[index];
        uint64_t end;
        uint64_t first_frame;
        uint64_t last_frame;
        if (entry == 0 || !range_end(entry->base, entry->length, &end)) return 0;
        if (entry->type != LIMINE_MEMMAP_USABLE || entry->length == 0) continue;
        if (!align_up(entry->base, &first_frame)) return 0;
        last_frame = align_down(end);
        if (first_frame >= last_frame) continue;
        if (allocator->region_count >= NEXUS_PMM_MAX_REGIONS) return 0;

        uint64_t frame_count = (last_frame - first_frame) / NEXUS_PMM_FRAME_SIZE;
        if (allocator->total_frames > max - frame_count) return 0;
        for (uint64_t prior = 0; prior < allocator->region_count; ++prior) {
            uint64_t prior_end = allocator->regions[prior].base +
                allocator->regions[prior].frame_count * NEXUS_PMM_FRAME_SIZE;
            if (range_overlaps(first_frame, last_frame,
                               allocator->regions[prior].base, prior_end)) return 0;
        }
        allocator->regions[allocator->region_count++] = (struct nexus_pmm_region){
            .base = first_frame,
            .frame_count = frame_count,
            .bitmap_bit_offset = allocator->total_frames
        };
        allocator->total_frames += frame_count;
    }

    if (allocator->total_frames > bitmap_bytes * 8ULL) return 0;

    for (uint64_t region = 0; region < allocator->region_count; ++region) {
        const struct nexus_pmm_region *entry = &allocator->regions[region];
        for (uint64_t frame = 0; frame < entry->frame_count; ++frame) {
            uint64_t address = entry->base + frame * NEXUS_PMM_FRAME_SIZE;
            uint64_t bit = entry->bitmap_bit_offset + frame;
            if (address == 0) {
                bit_set(reserved_bitmap, bit);
                bit_set(allocation_bitmap, bit);
                continue;
            }
            for (uint64_t range = 0; range < reserved_count; ++range) {
                uint64_t reserved_end;
                if (!range_end(reserved_ranges[range].base,
                          reserved_ranges[range].length, &reserved_end)) return 0;
                if (range_overlaps(address, address + NEXUS_PMM_FRAME_SIZE,
                                    reserved_ranges[range].base, reserved_end)) {
                    bit_set(reserved_bitmap, bit);
                    bit_set(allocation_bitmap, bit);
                    break;
                }
            }
        }
    }
    return 1;
}

int nexus_pmm_alloc(struct nexus_pmm_allocator *allocator,
                    uint64_t *physical_address) {
    uint64_t region_index;
    uint64_t frame_index;
    if (allocator == 0 || physical_address == 0 || allocator->allocation_bitmap == 0 ||
        !find_frame(allocator, &region_index, &frame_index)) return 0;
    const struct nexus_pmm_region *region = &allocator->regions[region_index];
    uint64_t bit = region->bitmap_bit_offset + frame_index;
    bit_set(allocator->allocation_bitmap, bit);
    *physical_address = region->base + frame_index * NEXUS_PMM_FRAME_SIZE;
    return 1;
}

int nexus_pmm_free(struct nexus_pmm_allocator *allocator,
                   uint64_t physical_address) {
    if (allocator == 0 || allocator->allocation_bitmap == 0 ||
        physical_address == 0 || (physical_address % NEXUS_PMM_FRAME_SIZE) != 0) return 0;
    for (uint64_t region = 0; region < allocator->region_count; ++region) {
        const struct nexus_pmm_region *entry = &allocator->regions[region];
        uint64_t end = entry->base + entry->frame_count * NEXUS_PMM_FRAME_SIZE;
        if (physical_address < entry->base || physical_address >= end) continue;
        uint64_t frame = (physical_address - entry->base) / NEXUS_PMM_FRAME_SIZE;
        uint64_t bit = entry->bitmap_bit_offset + frame;
        if (bit_is_set(allocator->reserved_bitmap, bit) ||
            !bit_is_set(allocator->allocation_bitmap, bit)) return 0;
        bit_clear(allocator->allocation_bitmap, bit);
        return 1;
    }
    return 0;
}
