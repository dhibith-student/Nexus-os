#include <stdint.h>
#include "exceptions.h"
#include "serial.h"

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attributes;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct idtr { uint16_t limit; uint64_t base; } __attribute__((packed));
static struct idt_entry idt[256] __attribute__((aligned(16)));
static volatile uint8_t exception_in_progress;

extern void exception_divide_error(void);
extern void exception_invalid_opcode(void);
extern void exception_double_fault(void);
extern void exception_general_protection(void);
extern void exception_page_fault(void);

static uint16_t current_code_segment(void) { uint16_t value; __asm__ volatile ("mov %%cs, %0" : "=r"(value)); return value; }
static uint64_t read_cr0(void) { uint64_t value; __asm__ volatile ("mov %%cr0, %0" : "=r"(value)); return value; }
static uint64_t read_cr2(void) { uint64_t value; __asm__ volatile ("mov %%cr2, %0" : "=r"(value)); return value; }
static uint64_t read_cr3(void) { uint64_t value; __asm__ volatile ("mov %%cr3, %0" : "=r"(value)); return value; }
static uint64_t read_cr4(void) { uint64_t value; __asm__ volatile ("mov %%cr4, %0" : "=r"(value)); return value; }

static void idt_set_gate(uint8_t vector, void (*handler)(void), uint16_t selector) {
    uint64_t address = (uint64_t)handler;
    idt[vector].offset_low = (uint16_t)address;
    idt[vector].selector = selector;
    idt[vector].ist = 0;
    idt[vector].type_attributes = 0x8e;
    idt[vector].offset_middle = (uint16_t)(address >> 16);
    idt[vector].offset_high = (uint32_t)(address >> 32);
    idt[vector].reserved = 0;
}

void exceptions_init(void) {
    for (uint64_t index = 0; index < 256; ++index) idt[index] = (struct idt_entry){0};
    uint16_t selector = current_code_segment();
    idt_set_gate(0, exception_divide_error, selector);
    idt_set_gate(6, exception_invalid_opcode, selector);
    idt_set_gate(8, exception_double_fault, selector);
    idt_set_gate(13, exception_general_protection, selector);
    idt_set_gate(14, exception_page_fault, selector);
    struct idtr descriptor = { .limit = (uint16_t)(sizeof(idt) - 1), .base = (uint64_t)&idt };
    __asm__ volatile ("lidt %0" : : "m"(descriptor));
}

static const char *exception_name(uint64_t vector) {
    switch (vector) {
        case 0: return "divide error";
        case 6: return "invalid opcode";
        case 8: return "double fault";
        case 13: return "general protection";
        case 14: return "page fault";
        default: return "unhandled configured exception";
    }
}

__attribute__((noreturn)) static void exception_halt(void) {
    for (;;) __asm__ volatile ("cli; hlt");
}

__attribute__((noreturn)) void exception_dispatch(const struct nexus_exception_frame *frame) {
    if (exception_in_progress != 0) {
        serial_write("[exception] recursive fault while reporting; controlled halt\r\n");
        exception_halt();
    }
    exception_in_progress = 1;
    serial_write("[exception] vector="); serial_hex(frame->vector);
    serial_write(" name="); serial_write(exception_name(frame->vector)); serial_write("\r\n");
    serial_write("[exception] rip="); serial_hex(frame->rip);
    serial_write(" cs="); serial_hex(frame->cs);
    serial_write(" rflags="); serial_hex(frame->rflags);
    serial_write(" error="); serial_hex(frame->error_code); serial_write("\r\n");
    serial_write("[exception] cr0="); serial_hex(read_cr0());
    serial_write(" cr2="); serial_hex(read_cr2());
    serial_write(" cr3="); serial_hex(read_cr3());
    serial_write(" cr4="); serial_hex(read_cr4()); serial_write("\r\n");
    serial_write("[exception] fatal exception; controlled halt\r\n");
    exception_halt();
}
