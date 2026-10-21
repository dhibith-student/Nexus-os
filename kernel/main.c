#include <stdint.h>
#include "arch_state.h"
#include "exceptions.h"
#include "kernel_image.h"
#include "limine.h"
#include "memory_accounting.h"
#include "serial.h"

#ifndef NEXUS_EXCEPTION_TEST
#define NEXUS_EXCEPTION_TEST 0
#endif

__attribute__((used, section(".limine_requests")))
volatile struct limine_memmap_request memmap_request = { { LIMINE_COMMON_MAGIC, LIMINE_MEMORY_MAP_REQUEST_ID }, 0, 0 };
__attribute__((used, section(".limine_requests")))
volatile uint64_t limine_base_revision[] = { LIMINE_BASE_REVISION(6) };
__attribute__((used, section(".limine_requests_start")))
volatile uint64_t limine_requests_start_marker[] = { LIMINE_REQUESTS_START_MARKER };
__attribute__((used, section(".limine_requests_end")))
volatile uint64_t limine_requests_end_marker[] = { LIMINE_REQUESTS_END_MARKER };

__attribute__((noreturn)) static void halt_forever(void) { for (;;) __asm__ volatile ("cli; hlt"); }
__attribute__((noreturn)) static void panic(const char *message) { serial_write("NEXUS PANIC: "); serial_write(message); serial_write("\r\n"); halt_forever(); }

static void print_memory_map(const struct limine_memmap_response *response) {
    serial_write("[init] memory map entries: "); serial_dec(response->entry_count); serial_write("\r\n");
    for (uint64_t index = 0; index < response->entry_count; ++index) {
        const struct limine_memmap_entry *entry = response->entries[index];
        if (entry == 0) panic("null Limine memory-map entry");
        serial_write("[mem] base="); serial_hex(entry->base); serial_write(" length="); serial_hex(entry->length);
        serial_write(" type="); serial_hex(entry->type); serial_write("\r\n");
    }
}

static void trigger_exception_test(void) {
#if NEXUS_EXCEPTION_TEST
    serial_write("[test] triggering invalid opcode exception\r\n");
    __asm__ volatile ("ud2");
#endif
}

void kernel_main(void) {
    serial_init();
    serial_write("NEXUS OS - Phase 2A\r\n");
    serial_write("[init] serial console: ready\r\n");
    serial_write("[init] kernel entry: reached\r\n");

    struct nexus_arch_state inherited_state;
    nexus_arch_state_read(&inherited_state);
    nexus_arch_state_print(&inherited_state);
    serial_write("[init] architectural state: inspected; inherited GDT/TSS/paging preserved\r\n");
    nexus_kernel_image_print();

    exceptions_init();
    serial_write("[init] exception diagnostics: ready (vectors 0, 6, 8, 13, 14)\r\n");
    const struct limine_memmap_response *response = memmap_request.response;
    if (response == 0 || response->entries == 0 || response->entry_count > NEXUS_MAX_MEMMAP_ENTRIES) panic("invalid Limine memory-map response");
    print_memory_map(response);
    struct nexus_memory_summary summary;
    if (!nexus_memory_accounting(response, &summary)) panic("memory accounting validation failed");
    serial_write("[mem] usable regions: "); serial_dec(summary.usable_regions); serial_write("\r\n[mem] usable bytes: "); serial_dec(summary.usable_bytes); serial_write("\r\n[mem] usable MiB: "); serial_dec(summary.usable_bytes / (1024ULL * 1024ULL)); serial_write("\r\n");
    serial_write("[init] physical memory allocator: unit-tested only; runtime integration deferred\r\n");
    trigger_exception_test();
    serial_write("[init] scheduler: deferred to Phase 2\r\n");
    serial_write("[init] IPC and recovery: deferred to later phases\r\n");
    serial_write("[halt] initialization complete; controlled halt\r\n");
    halt_forever();
}
