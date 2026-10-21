# Implementation plan

## Phase 1

Phase 1 established the pinned Limine build, bootable ISO, COM1 diagnostics, memory-map dump, and controlled halt. Its working commit remains the rollback milestone.

## Phase 2A — complete

Implemented and tested:

- IDT gates for vectors 0, 6, 8, 13, and 14.
- Assembly normalization for CPU exceptions with and without pushed error codes.
- C diagnostics for vector/name, RIP, CS, RFLAGS, error code, and CR0/CR2/CR3/CR4.
- Fatal exception path with interrupts disabled and controlled halt.
- Overflow-safe accounting of Limine type-0 usable memory.
- Focused memory-accounting tests for valid input, invalid input, and overflow.
- Separate EXCEPTION_TEST=1 QEMU fixture that triggers ud2.

The normal make check target requires current kernel/ISO artifacts, a serial log newer than both artifacts, expected startup and memory-accounting lines, deferred-feature lines, and controlled halt. The exception fixture uses tests/check-exception.sh and is intentionally separate.

## Reproducible setup

Install in Ubuntu under WSL2:

    wsl --update
    wsl -d Ubuntu -- sudo apt update
    wsl -d Ubuntu -- sudo apt install -y gcc-x86-64-linux-gnu binutils-x86-64-linux-gnu make xorriso qemu-system-x86 git curl ca-certificates

Verify:

    wsl -d Ubuntu -- bash -lc 'command -v x86_64-linux-gnu-gcc x86_64-linux-gnu-ld make xorriso qemu-system-x86_64'

Obtain pinned assets:

    make limine-assets

Normal build and boot:

    make clean
    make
    rm -f build/serial.log
    timeout 8s qemu-system-x86_64 -M q35 -m 128M -serial file:build/serial.log -display none -no-reboot -no-shutdown -boot d -cdrom build/nexus.iso
    bash tests/check-serial.sh build/serial.log build/nexus.elf build/nexus.iso
    make check

Controlled exception boot:

    make clean
    make EXCEPTION_TEST=1
    rm -f build/serial-exception.log
    timeout 8s qemu-system-x86_64 -M q35 -m 128M -serial file:build/serial-exception.log -display none -no-reboot -no-shutdown -boot d -cdrom build/nexus.iso
    bash tests/check-exception.sh build/serial-exception.log

On 2026-10-22, normal QEMU evidence and make check passed. The output reported 20 entries, 3 usable regions, 132702208 bytes, 126 MiB, deferred features, and controlled halt. The separate exception QEMU evidence and check-exception.sh also passed; its timeout was expected because the guest halts.

## Phase 2B — recommended next milestone

Review and test the architectural context contract, then add minimum GDT/TSS and page-table foundations. Do not implement scheduling, IPC, user mode, service supervision, or recovery until their mechanisms and tests exist.

Every milestone must report changed files, commands, observed results, defects, and next milestone. Do not commit or push automatically during verification.
