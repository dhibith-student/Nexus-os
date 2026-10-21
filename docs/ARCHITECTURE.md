# NEXUS OS architecture

## Phase 2D physical ownership audit

NEXUS requests only the Limine memory-map response through include/limine.h. The response contains an entry count and pointers to entries with physical base, byte length, and memory type. Type 0 is the only type treated as usable by memory accounting and by the offline PMM core. Types 1 through 7 represented in this repository are reserved, ACPI reclaimable, ACPI NVS, bad memory, bootloader reclaimable, executable/modules, and framebuffer respectively; none is immediately allocatable.

The current source does not request Limine module, framebuffer, higher-half relocation, or complete boot-information responses. Consequently it has no source-level physical ranges for Limine response storage, modules, framebuffer backing memory, or other boot objects. Bootloader-reclaimable entries remain reserved because their lifetime and safe-reclamation point have not been established.

## ELF and linker evidence

The linker script starts the image at virtual address 0xffffffff80000000 and now exports __kernel_image_start and __kernel_image_end. readelf on build/nexus.elf reports:

- Entry point: 0xffffffff80001000.
- Linked virtual image: start 0xffffffff80000000, end 0xffffffff80005010, span 0x5010 (20496) bytes.
- Limine-request segment: VMA/PAddr fields 0xffffffff80000000, file/memory size 0x80, read/write.
- Text segment: VMA/PAddr fields 0xffffffff80001000, file/memory size 0x12b1, read/execute.
- Read-only segment: VMA/PAddr fields 0xffffffff80003000, file/memory size 0x63a, read-only.
- BSS segment: VMA/PAddr fields 0xffffffff80004000, file size 0, memory size 0x1010, read/write.

The section layout contains .limine_requests, .text, .rodata, and .bss. The ELF PAddr fields equal the higher-half VMAs because the linker has no AT physical-load expression. They are therefore not accepted as physical addresses. The exported image symbols are virtual linked bounds and include BSS; they do not identify a physical reservation.

## Runtime paging and address evidence

QEMU serial diagnostics report CR0=0x80010011, CR2=0, CR3=0x7f9b000, CR4=0x20, and EFER=0xd00, confirming inherited long-mode paging bits in this tested boot. NEXUS does not replace CR3, walk page tables, change mappings, or dereference arbitrary physical addresses. A canonical-looking virtual address is not treated as a physical address.

The runtime reports the linked virtual image and explicitly reports its physical load range as unproven. The current mapping inventory is unknown: source and runtime evidence do not establish which physical pages back the higher-half image or where Limine boot objects are mapped.

## Reservation model

A future PMM reservation set must include:

- Frame zero.
- The complete physical kernel image, including loaded sections, BSS, and relevant alignment padding.
- Limine response/request storage and any retained boot information.
- All modules and their backing ranges.
- Framebuffer memory, if requested and present.
- Allocator bitmaps and metadata.
- Bootloader-reclaimable memory until all dependent information is copied and a verified reclamation point exists.
- Other firmware/platform ranges not explicitly identified as usable.

The existing PMM core accepts explicit reserved physical ranges and is unit-tested, but it is not initialized from the live map. The missing physical kernel range, missing boot-object requests, unknown mappings, and unknown safe bitmap location prevent safe integration.

## Pure range tests

memory_ranges.c provides checked half-open range construction, reversed/overflow rejection, adjacency-aware overlap tests, frame alignment, and a reservation set that rejects duplicate or overlapping reservations while accepting empty no-op ranges. Tests cover boundaries, overflow, alignment, kernel-image overlap, reservations, and unsupported memory types. These tests do not perform privileged operations.

A dedicated TSS stack or owned page-table structures cannot be allocated until physical reservations, page-table ownership, and a safe mapped PMM metadata location are established. Scheduling, IPC, user-mode isolation, service supervision, and recovery remain deferred.
