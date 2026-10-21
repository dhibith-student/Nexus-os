#include <stdint.h>
#include "limine.h"

__attribute__((used, section(".limine_requests")))
volatile struct limine_memmap_request memmap_request = {
    { LIMINE_COMMON_MAGIC, LIMINE_MEMORY_MAP_REQUEST_ID, 0 }, 0, 0
};

static void serial_init(void) {
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x00), "Nd"((uint16_t)0x3f9));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x80), "Nd"((uint16_t)0x3fb));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x03), "Nd"((uint16_t)0x3f8));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x00), "Nd"((uint16_t)0x3f9));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x03), "Nd"((uint16_t)0x3fb));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0xc7), "Nd"((uint16_t)0x3fa));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x0b), "Nd"((uint16_t)0x3fc));
}

static void serial_putc(char c) {
    uint8_t status;
    do { __asm__ volatile ("inb %1, %0" : "=a"(status) : "Nd"((uint16_t)0x3fd)); } while (!(status & 0x20));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)c), "Nd"((uint16_t)0x3f8));
}

static void serial_write(const char *s) { while (*s) serial_putc(*s++); }

static void serial_hex(uint64_t value) {
    static const char digits[] = "0123456789abcdef";
    serial_write("0x");
    for (int shift = 60; shift >= 0; shift -= 4) serial_putc(digits[(value >> shift) & 0xf]);
}

__attribute__((noreturn)) static void halt_forever(void) {
    for (;;) __asm__ volatile ("cli; hlt");
}

__attribute__((noreturn)) static void panic(const char *message) {
    serial_write("NEXUS PANIC: "); serial_write(message); serial_write("\r\n"); halt_forever();
}

void kernel_main(void) {
    serial_init();
    serial_write("NEXUS OS - Phase 1\r\n");
    serial_write("[init] serial console: ready\r\n");
    serial_write("[init] kernel entry: reached\r\n");
    if (!memmap_request.response) panic("Limine memory-map response unavailable");
    serial_write("[init] memory map entries: "); serial_hex(memmap_request.response->entry_count); serial_write("\r\n");
    for (uint64_t i = 0; i < memmap_request.response->entry_count; ++i) {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];
        serial_write("[mem] base="); serial_hex(entry->base); serial_write(" length="); serial_hex(entry->length);
        serial_write(" type="); serial_hex(entry->type); serial_write("\r\n");
    }
    serial_write("[init] scheduler: deferred to Phase 2\r\n");
    serial_write("[init] IPC and recovery: deferred to later phases\r\n");
    serial_write("[halt] initialization complete; controlled halt\r\n");
    halt_forever();
}
