# NEXUS OS

NEXUS OS is a real freestanding x86-64 kernel prototype. Phase 2A is implemented and verified: Limine v12.9.3 boot, COM1 diagnostics, a bounded exception subset, and read-only usable-memory accounting. Scheduling, IPC, user-mode isolation, memory protection, service supervision, and recovery remain unimplemented.

## Verified status

Normal headless QEMU boot reports the Limine memory map, 3 usable regions, 132702208 usable bytes (126 MiB with the tested 128 MiB QEMU configuration), deferred-feature messages, and controlled halt. A separate EXCEPTION_TEST=1 QEMU boot triggers and diagnoses invalid opcode vector 6, then enters the fatal controlled-halt loop. Installed exception vectors are only 0, 6, 8, 13, and 14.

Memory accounting counts only Limine type-0 usable entries, bounds the entry count, checks base-plus-length and sum overflow, and reports bytes plus integer MiB. It does not allocate or modify physical memory.

## Toolchain and commands

The documented WSL environment uses x86_64-linux-gnu-gcc/binutils, GNU Make, Limine v12.9.3 assets, xorriso, and qemu-system-x86_64. See docs/ARCHITECTURE.md and docs/IMPLEMENTATION_PLAN.md.

    make limine-assets
    make
    make check
    make run

Controlled exception test:

    make clean
    make EXCEPTION_TEST=1
    rm -f build/serial-exception.log
    timeout 8s qemu-system-x86_64 -M q35 -m 128M -serial file:build/serial-exception.log -display none -no-reboot -no-shutdown -boot d -cdrom build/nexus.iso
    bash tests/check-exception.sh build/serial-exception.log

The timeout is expected because the guest intentionally halts. Images are disposable files under build/; no physical disk or host boot record is touched. Read AGENTS.md before every change.
