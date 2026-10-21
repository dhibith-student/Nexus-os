#include <stdint.h>
#include "arch_state.h"
#include "serial.h"

struct nexus_descriptor_pointer {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static uint64_t read_efer(void) {
    uint32_t low;
    uint32_t high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(0xc0000080U));
    return ((uint64_t)high << 32) | low;
}

void nexus_arch_state_read(struct nexus_arch_state *state) {
    struct nexus_descriptor_pointer gdtr;
    struct nexus_descriptor_pointer idtr;
    __asm__ volatile ("sgdt %0" : "=m"(gdtr));
    __asm__ volatile ("sidt %0" : "=m"(idtr));
    __asm__ volatile ("mov %%cs, %0" : "=r"(state->cs));
    __asm__ volatile ("mov %%ds, %0" : "=r"(state->ds));
    __asm__ volatile ("mov %%es, %0" : "=r"(state->es));
    __asm__ volatile ("mov %%ss, %0" : "=r"(state->ss));
    __asm__ volatile ("str %0" : "=r"(state->tr));
    __asm__ volatile ("mov %%cr0, %0" : "=r"(state->cr0));
    __asm__ volatile ("mov %%cr2, %0" : "=r"(state->cr2));
    __asm__ volatile ("mov %%cr3, %0" : "=r"(state->cr3));
    __asm__ volatile ("mov %%cr4, %0" : "=r"(state->cr4));
    state->gdtr_base = gdtr.base;
    state->gdtr_limit = gdtr.limit;
    state->idtr_base = idtr.base;
    state->idtr_limit = idtr.limit;
    state->efer = read_efer();
}

void nexus_arch_state_print(const struct nexus_arch_state *state) {
    serial_write("[arch] segments cs="); serial_hex(state->cs);
    serial_write(" ds="); serial_hex(state->ds);
    serial_write(" es="); serial_hex(state->es);
    serial_write(" ss="); serial_hex(state->ss);
    serial_write(" tr="); serial_hex(state->tr); serial_write("\r\n");
    serial_write("[arch] gdtr base="); serial_hex(state->gdtr_base);
    serial_write(" limit="); serial_hex(state->gdtr_limit);
    serial_write(" idtr base="); serial_hex(state->idtr_base);
    serial_write(" limit="); serial_hex(state->idtr_limit); serial_write("\r\n");
    serial_write("[arch] paging cr0="); serial_hex(state->cr0);
    serial_write(" cr2="); serial_hex(state->cr2);
    serial_write(" cr3="); serial_hex(state->cr3);
    serial_write(" cr4="); serial_hex(state->cr4);
    serial_write(" efer="); serial_hex(state->efer); serial_write("\r\n");
    if ((state->cr0 & (1ULL << 31)) != 0 &&
        (state->cr4 & (1ULL << 5)) != 0 &&
        (state->efer & (1ULL << 10)) != 0) {
        serial_write("[arch] paging mode: inherited long mode paging; NEXUS did not replace CR3\r\n");
    } else {
        serial_write("[arch] paging mode: inherited state requires further assessment\r\n");
    }
}
