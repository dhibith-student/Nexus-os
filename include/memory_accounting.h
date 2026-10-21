#ifndef NEXUS_MEMORY_ACCOUNTING_H
#define NEXUS_MEMORY_ACCOUNTING_H

#include <stdint.h>
#include "limine.h"

#define NEXUS_MAX_MEMMAP_ENTRIES 4096U

struct nexus_memory_summary {
    uint64_t usable_bytes;
    uint64_t usable_regions;
};

int nexus_memory_accounting(const struct limine_memmap_response *response,
                             struct nexus_memory_summary *summary);

#endif
