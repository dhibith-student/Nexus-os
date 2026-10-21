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

Key audit result: the ELF PAddr values equal higher-half VMAs because the linker does not define a physical load address. The current Limine requests do not provide module, framebuffer, or complete boot-information ranges. Therefore no physical kernel-image reservation or safe bitmap placement can be derived, and PMM integration is correctly deferred.

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

## Phase 2D evidence

The default build and fresh QEMU boot passed. Runtime serial output reported:

    [kernel] linked virtual image: start=0xffffffff80000000 end=0xffffffff80005010 size=20496
    [kernel] physical load range: unproven by current linker and boot requests
    [arch] paging cr0=0x0000000080010011 cr2=0x0000000000000000 cr3=0x0000000007f9b000 cr4=0x0000000000000020 efer=0x0000000000000d00
    [mem] usable regions: 3
    [mem] usable bytes: 132689920
    [mem] usable MiB: 126
    [init] physical memory allocator: unit-tested only; runtime integration deferred
    [halt] initialization complete; controlled halt

The five-vector invalid-opcode regression also passes and reaches fatal controlled halt. QEMU termination by the eight-second timeout is expected.

## Phase 2E — recommended next milestone

Request and audit the Limine responses needed to identify modules, framebuffer, and boot information, and establish a linker/load strategy that provides a trustworthy physical kernel image range. Define page-table ownership and a mapped location for PMM metadata before activating allocation or allocating a TSS stack/page-table structures. Do not add scheduling, IPC, user mode, service supervision, or recovery.
