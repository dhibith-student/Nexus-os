# Implementation plan

## Phase 1

Established the pinned Limine build, bootable ISO, COM1 diagnostics, memory-map dump, and controlled halt.

## Phase 2A

Added and tested five CPU exception gates, normalized exception frames, fatal diagnostics, and overflow-safe usable-memory accounting.

## Phase 2B

Added read-only inherited GDT/TSS/paging diagnostics and tested descriptor/TSS construction helpers without loading new architectural tables or changing CR3.

## Phase 2C — complete as a non-integrated foundation

Implemented:

- A physical-frame bitmap core with 4096-byte alignment.
- Explicit separation between usable type-0 memory and all other Limine types.
- Conservative reservation of frame zero and caller-supplied reserved ranges.
- Overflow-safe range validation, bitmap-capacity checks, overlapping-region rejection, exhaustion handling, and strict free validation.
- Freestanding deterministic tests for empty maps, multiple regions, reserved gaps, unaligned and zero-sized regions, overflow, exhaustion, alignment, reserved exclusion, invalid frees, duplicate frees, and reuse.
- Kernel compilation of the PMM core without activating it.

The allocator is unit-tested only. It is not initialized from the live boot map because this repository lacks verified physical reservations for the kernel image, Limine metadata, modules, framebuffer, and a safe bitmap location. Bootloader-reclaimable regions therefore remain unavailable.

## Reproducible commands

    make clean
    make
    make test-memory-accounting
    make test-physical-memory
    make test-descriptors
    make check

make check builds the kernel and tests, removes stale build/serial.log, boots QEMU with -M q35 -m 128M and serial file output for eight seconds, accepts only timeout status 124, and then runs tests/check-serial.sh with the serial log, kernel ELF, and ISO. The checker requires fresh evidence and all existing architectural, memory-accounting, deferred-feature, and controlled-halt diagnostics.

Exception regression:

    make clean
    make EXCEPTION_TEST=1
    rm -f build/serial-exception.log
    timeout 8s qemu-system-x86_64 -M q35 -m 128M -serial file:build/serial-exception.log -display none -no-reboot -no-shutdown -boot d -cdrom build/nexus.iso
    bash tests/check-exception.sh build/serial-exception.log

## Phase 2C evidence

The PMM and existing freestanding tests pass. The normal kernel build and QEMU boot continue to pass, including fresh serial evidence, inherited architectural-state diagnostics, Limine memory accounting, the unit-tested-only PMM status, deferred feature messages, and controlled halt. The invalid-opcode QEMU regression continues to report vector 6 and reaches fatal controlled halt.

## Phase 2D — recommended next milestone

Define and review physical reservations and page-table ownership. Establish how Limine-provided physical addresses relate to the higher-half kernel mapping, where PMM metadata can be safely stored and mapped, and how kernel/module/framebuffer ranges are discovered. Only after that review should the PMM core be integrated or used to allocate a dedicated TSS stack or page-table structures.

No scheduler, IPC, user mode, service supervision, or recovery should begin before these memory-safety prerequisites are established.
