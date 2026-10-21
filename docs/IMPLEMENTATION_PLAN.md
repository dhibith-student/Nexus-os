# Implementation plan

## Phase 1

Phase 1 established the pinned Limine build, bootable ISO, COM1 diagnostics, memory-map dump, and controlled halt. Its working commit remains the rollback milestone.

## Phase 2A — complete

Phase 2A added and tested five CPU exception gates, normalized exception frames, fatal diagnostics, overflow-safe usable-memory accounting, and separate normal/exception QEMU checks.

## Phase 2B — complete

Implemented and tested:

- Read-only diagnostics for inherited segment selectors, GDTR, IDTR, TR, CR0, CR2, CR3, CR4, and EFER.
- Explicit documentation of ring-0 and boot-stack assumptions.
- A pure segment-descriptor constructor.
- A packed 104-byte x86-64 TSS layout and initializer.
- Focused freestanding tests for code/data descriptor encodings and TSS initialization.
- Normal QEMU evidence that Limine-provided long-mode paging remains active.
- Regression coverage for normal serial acceptance, memory accounting, and the controlled invalid-opcode path.

The implementation deliberately does not execute LGDT, LTR, reload segment registers, replace CR3, walk or modify page tables, allocate a TSS stack, enter user mode, or add a scheduler. The descriptor and TSS helpers are construction foundations only.
The normal make check target builds the artifacts and focused tests, boots QEMU with the documented timeout, captures fresh build/serial.log evidence, and validates the required startup, architectural-state, memory-accounting, deferred-feature, and controlled-halt lines.

## Reproducible commands

Install in Ubuntu under WSL2:

    wsl --update
    wsl -d Ubuntu -- sudo apt update
    wsl -d Ubuntu -- sudo apt install -y gcc-x86-64-linux-gnu binutils-x86-64-linux-gnu make xorriso qemu-system-x86 git curl ca-certificates

Verify:

    wsl -d Ubuntu -- bash -lc 'command -v x86_64-linux-gnu-gcc x86_64-linux-gnu-ld make xorriso qemu-system-x86_64'

Obtain pinned assets:

    make limine-assets

Build and normal boot:

    make clean
    make
    rm -f build/serial.log
    timeout 8s qemu-system-x86_64 -M q35 -m 128M -serial file:build/serial.log -display none -no-reboot -no-shutdown -boot d -cdrom build/nexus.iso
    bash tests/check-serial.sh build/serial.log build/nexus.elf build/nexus.iso
    make test-memory-accounting
    make test-descriptors
    make check

Controlled exception boot:

    make clean
    make EXCEPTION_TEST=1
    rm -f build/serial-exception.log
    timeout 8s qemu-system-x86_64 -M q35 -m 128M -serial file:build/serial-exception.log -display none -no-reboot -no-shutdown -boot d -cdrom build/nexus.iso
    bash tests/check-exception.sh build/serial-exception.log

## Phase 2B evidence

On 2026-10-22, the default build completed and QEMU booted the kernel. The serial log reported ring-0 selectors CS=0x28, DS/ES/SS=0x30, TR=0x38, GDTR limit 0x47, an initially empty IDTR, and active inherited long-mode paging with CR3=0x7f9b000. The normal serial check passed, the memory-accounting and descriptor tests passed, and git diff --check passed. The controlled exception procedure remains required after the final default rebuild and must continue to pass; it does not run during normal startup.

QEMU is terminated by the eight-second timeout because the guest intentionally remains in a halt loop. This is expected and is not a guest shutdown result.

## Phase 2C — recommended next milestone

Design and review ownership of a NEXUS GDT/TSS and page-table subsystem, including allocation and mapping requirements for a dedicated TSS stack. Validate the design before any LGDT, LTR, CR3, or page-table mutation. Scheduling, IPC, user mode, service supervision, and recovery remain out of scope.
