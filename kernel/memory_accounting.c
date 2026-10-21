#include <stdint.h>
#include "memory_accounting.h"

int nexus_memory_accounting(const struct limine_memmap_response *response,
                             struct nexus_memory_summary *summary) {
    const uint64_t max_value = ~(uint64_t)0;
    if (response == 0 || summary == 0 || response->entries == 0 ||
        response->entry_count > NEXUS_MAX_MEMMAP_ENTRIES) return 0;
    summary->usable_bytes = 0;
    summary->usable_regions = 0;
    for (uint64_t index = 0; index < response->entry_count; ++index) {
        const struct limine_memmap_entry *entry = response->entries[index];
        if (entry == 0 || entry->base > max_value - entry->length) return 0;
        if (entry->type != LIMINE_MEMMAP_USABLE) continue;
        if (summary->usable_bytes > max_value - entry->length) return 0;
        summary->usable_bytes += entry->length;
        ++summary->usable_regions;
    }
    return 1;
}
