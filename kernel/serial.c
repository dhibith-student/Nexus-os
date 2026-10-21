#include <stdint.h>
#include "serial.h"

void serial_init(void) {
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x00), "Nd"((uint16_t)0x3f9));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x80), "Nd"((uint16_t)0x3fb));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x03), "Nd"((uint16_t)0x3f8));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x00), "Nd"((uint16_t)0x3f9));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x03), "Nd"((uint16_t)0x3fb));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0xc7), "Nd"((uint16_t)0x3fa));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x0b), "Nd"((uint16_t)0x3fc));
}

void serial_putc(char c) {
    uint8_t status;
    do {
        __asm__ volatile ("inb %1, %0" : "=a"(status) : "Nd"((uint16_t)0x3fd));
    } while (!(status & 0x20));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)c), "Nd"((uint16_t)0x3f8));
}

void serial_write(const char *text) {
    while (*text != '\0') serial_putc(*text++);
}

void serial_hex(uint64_t value) {
    static const char digits[] = "0123456789abcdef";
    serial_write("0x");
    for (int shift = 60; shift >= 0; shift -= 4) serial_putc(digits[(value >> shift) & 0xf]);
}

void serial_dec(uint64_t value) {
    char digits[20];
    uint64_t index = 0;
    if (value == 0) { serial_putc('0'); return; }
    while (value != 0) { digits[index++] = (char)('0' + (value % 10)); value /= 10; }
    while (index != 0) serial_putc(digits[--index]);
}
