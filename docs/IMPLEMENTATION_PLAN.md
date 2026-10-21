# Implementation plan

## Phase 1

Established the pinned Limine build, bootable ISO, COM1 diagnostics, memory-map dump, and controlled halt.

## Phase 2A

Added and tested five CPU exception gates, normalized exception frames, fatal diagnostics, and overflow-safe usable-memory accounting.

## Phase 2B

Added read-only inherited GDT/TSS/paging diagnostics and tested descriptor/TSS construction helpers without loading new architectural tables or changing CR3.

## Phase 2C

Added a conservative, unit-tested bitmap PMM core. It aligns usable type-0 entries to 4096-byte frames, reserves frame zero and explicit ranges, rejects unsafe ranges and frees, and remains inactive because physical ownership and bitmap placement were not proven.

## Phase 2D — complete as an audit and non-integrated foundation

Implemented and verified:

- Linker symbols and serial diagnostics for linked virtual image start/end and span.
- ELF inspection of entry point, loadable segments, section layout, VMA/PAddr fields, file sizes, memory sizes, and alignment.
- Documentation of the exact Limine request coverage and memory-type policy.
- Pure range and reservation helpers with deterministic tests for empty/reversed ranges, adjacency, alignment, overflow, kernel-image overlap, duplicate/overlapping reservations, out-of-range values, and unsupported memory types.
- Preservation of the PMM as unit-tested only.
- Normal QEMU evidence that inherited long-mode paging and CR3 remain unchanged.

Key Phase 2D result: the ELF PAddr values equal higher-half VMAs because the linker does not define a physical load address. Phase 2E now obtains the physical base from Limine executable-address and audits optional boot objects; PMM integration remains deferred.

## Reproducible commands

    make clean
    make
    make test-memory-accounting
    make test-physical-memory
    make test-memory-ranges
    make test-descriptors
    make check

make check builds all tests, removes stale build/serial.log, boots QEMU with -M q35 -m 128M and serial file output for eight seconds, accepts only timeout status 124, then runs tests/check-serial.sh with the log, kernel ELF, and ISO. The checker requires fresh architectural-state, kernel-image, memory-accounting, PMM-status, deferred-feature, and controlled-halt evidence.

Exception regression:

    make clean
    make EXCEPTION_TEST=1
    rm -f build/serial-exception.log
    timeout 8s qemu-system-x86_64 -M q35 -m 128M -serial file:build/serial-exception.log -display none -no-reboot -no-shutdown -boot d -cdrom build/nexus.iso
    bash tests/check-exception.sh build/serial-exception.log

## Phase 2D evidence (historical)

The default build and fresh QEMU boot passed. Runtime serial output reported:

    [kernel] linked virtual image: start=0xffffffff80000000 end=0xffffffff80005010 size=20496
    [kernel] physical load range: unproven by the Phase 2D request set
    [arch] paging cr0=0x0000000080010011 cr2=0x0000000000000000 cr3=0x0000000007f9b000 cr4=0x0000000000000020 efer=0x0000000000000d00
    [mem] usable regions: 3
    [mem] usable bytes: 132689920
    [mem] usable MiB: 126
    [init] physical memory allocator: unit-tested only; runtime integration deferred
    [halt] initialization complete; controlled halt

The five-vector invalid-opcode regression also passes and reaches fatal controlled halt. QEMU termination by the eight-second timeout is expected.

## Phase 2E — recommended next milestone

Implemented and verified:

- Pinned Limine v12.9.3 protocol structures for HHDM, executable-address, modules, and framebuffer responses.
- Runtime physical-load evidence from Limine executable-address: physical base 0x7f44000, virtual base 0xffffffff80000000; linked image span remains 20496 bytes through physical end 0x7f49010 in the tested QEMU boot.
- HHDM offset reporting and safe HHDM-pointer range conversion without arbitrary physical dereferences.
- Optional module/framebuffer response validation and bounded diagnostics.
- Conservative reservation audit for frame zero, the full kernel image, bootloader-reclaimable memory, and identified framebuffer/module ranges.

The PMM remains unit-tested only. Limine response storage is retained by leaving bootloader-reclaimable regions reserved; no reclamation point or bitmap placement has been activated. The linker still has no AT() physical-load expression because Limine supplies the physical base at runtime. A future phase must establish page-table ownership and a safe mapped bitmap location before PMM integration, TSS-stack allocation, or owned page tables. Do not add scheduling, IPC, user mode, service supervision, or recovery.
