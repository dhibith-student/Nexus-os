# NEXUS OS architecture

## Phase 2D physical ownership audit

NEXUS requests the Limine HHDM, executable-address, memory-map, module, and framebuffer responses through include/limine.h. The response contains an entry count and pointers to entries with physical base, byte length, and memory type. Type 0 is the only type treated as usable by memory accounting and by the offline PMM core. Types 1 through 7 represented in this repository are reserved, ACPI reclaimable, ACPI NVS, bad memory, bootloader reclaimable, executable/modules, and framebuffer respectively; none is immediately allocatable.

The source does not request the executable-file or bootloader-info responses because they are not needed for the physical-load audit. HHDM, executable-address, module, and framebuffer responses provide the required address evidence; optional module responses may be absent when no modules are configured. Bootloader-reclaimable entries remain reserved because their lifetime and safe-reclamation point have not been established.

## ELF and linker evidence

The linker script starts the image at virtual address 0xffffffff80000000 and now exports __kernel_image_start and __kernel_image_end. readelf on build/nexus.elf reports:

- Entry point: 0xffffffff80001000.
- Linked virtual image: start 0xffffffff80000000, end 0xffffffff80005010, span 0x5010 (20496) bytes.
- Limine-request segment: VMA/PAddr fields 0xffffffff80000000, file/memory size 0x80, read/write.
- Text segment: VMA/PAddr fields 0xffffffff80001000, file/memory size 0x12b1, read/execute.
- Read-only segment: VMA/PAddr fields 0xffffffff80003000, file/memory size 0x63a, read-only.
- BSS segment: VMA/PAddr fields 0xffffffff80004000, file size 0, memory size 0x1010, read/write.

The section layout contains .limine_requests, .text, .rodata, and .bss. The ELF PAddr fields equal the higher-half VMAs because the linker has no AT physical-load expression. They are therefore not accepted as physical addresses. Limine executable-address runtime evidence supplies physical_base=0x7f44000 and virtual_base=0xffffffff80000000 in the tested boot. The exported image symbols are linked virtual bounds and include BSS; the physical range is derived only after validating that virtual_base matches.

## Runtime paging and address evidence

QEMU serial diagnostics report CR0=0x80010011, CR2=0, CR3=0x7f9b000, CR4=0x20, and EFER=0xd00, confirming inherited long-mode paging bits in this tested boot. NEXUS does not replace CR3, walk page tables, change mappings, or dereference arbitrary physical addresses. A canonical-looking virtual address is not treated as a physical address.

The runtime reports the linked virtual image, the Limine physical load range, and HHDM offset=0xffff800000000000. Limine protocol documentation says response pointers include the HHDM offset and responses reside in bootloader-reclaimable memory. NEXUS still does not walk page tables or dereference arbitrary physical addresses; its inherited mappings are not owned by NEXUS.

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

The reservation audit represents frame zero, the physical kernel image, bootloader-reclaimable map entries, and HHDM-converted module/framebuffer ranges. It remains diagnostic only: the PMM bitmap location and complete physical ownership of all boot-time objects are not yet established, so the PMM is not initialized from the live map.

## Pure range tests

memory_ranges.c provides checked half-open range construction, reversed/overflow rejection, adjacency-aware overlap tests, frame alignment, and a reservation set that rejects duplicate or overlapping reservations while accepting empty no-op ranges. Tests cover boundaries, overflow, alignment, kernel-image overlap, reservations, and unsupported memory types. These tests do not perform privileged operations.

A dedicated TSS stack or owned page-table structures cannot be allocated until physical reservations, page-table ownership, and a safe mapped PMM metadata location are established. Scheduling, IPC, user-mode isolation, service supervision, and recovery remain deferred.


## Phase 2E evidence and limits

The normal QEMU boot reported one framebuffer and no module response. The framebuffer range was converted from its HHDM pointer using the protocol offset and included in the conservative audit; no framebuffer pixels were accessed. The kernel physical image reported base 0x7f42000, end 0x7f48010, and size 24592 bytes. These are runtime observations for the tested boot, not hard-coded addresses.

Limine supports bootloader-info and executable-file responses, but neither was requested because they would not establish a physical load range beyond executable-address. The response model is bounded to 256 optional entries, rejects null arrays for nonzero counts, rejects HHDM underflow and overflowing ranges, and treats absent module/framebuffer responses as valid optional cases.


## Phase 2F inherited page-table audit

The current boot reports CR0=0x80010011, CR3=0x7f84000, CR4=0x20, and EFER=0xd00. CR0.PG, CR4.PAE, and EFER.LMA are set; CR4.LA57 is clear, so the runtime audit uses four-level paging with 48-bit canonical virtual addresses. The CR3 physical address is formed by masking the architectural low flags with 0x000ffffffffff000.

The audit first checks canonical arithmetic, validates the masked CR3 page against a type-5 bootloader-reclaimable memory-map entry, forms its HHDM virtual address, and then walks at most four levels. Every descendant table page must also be a 4 KiB-aligned type-5 page before it is dereferenced. Present level-3 and level-2 large pages are handled; non-present, malformed, noncanonical, unsupported, or unproven paths produce a diagnostic rather than a dereference. No CR3, page-table entry, TLB, paging mode, GDT, TSS, or privilege state is changed.

In the fresh QEMU boot the root candidate was physical 0x7f84000 and HHDM virtual 0xffff800007f84000. Root coverage was confirmed with a 4 KiB mapping, and the linked kernel start was confirmed at physical 0x7f42000 through a level-1 4 KiB mapping. This establishes safe access for those inspected pages only. It does not prove that every physical page is HHDM-mapped or that NEXUS owns any page tables.

The PMM prerequisite checklist remains: reserve the full kernel image; retain bootloader-reclaimable response and page-table storage; reserve memory-map metadata, modules, framebuffer, bitmap and allocator metadata; validate all page-table pages and future runtime objects; and establish safe virtual access for every object before activation. A dedicated TSS stack and owned page tables remain deferred.
