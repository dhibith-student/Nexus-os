#ifndef NEXUS_ARCH_STATE_H
#define NEXUS_ARCH_STATE_H

#include <stdint.h>

struct nexus_arch_state {
    uint64_t gdtr_base;
    uint16_t gdtr_limit;
    uint64_t idtr_base;
    uint16_t idtr_limit;
    uint16_t cs;
    uint16_t ds;
    uint16_t es;
    uint16_t ss;
    uint16_t tr;
    uint64_t cr0;
    uint64_t cr2;
    uint64_t cr3;
    uint64_t cr4;
    uint64_t efer;
};

void nexus_arch_state_read(struct nexus_arch_state *state);
void nexus_arch_state_print(const struct nexus_arch_state *state);

#endif
