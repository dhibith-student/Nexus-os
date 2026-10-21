# NEXUS OS

**NEXUS OS: A Self-Healing Microkernel with Dependency-Aware Service Recovery**

This repository contains the Phase 1 foundation for a real x86-64 kernel prototype. The current milestone is a small Limine-booted freestanding kernel that writes diagnostics to COM1 and safely halts after initialization. It does not yet implement task scheduling, IPC, user-mode isolation, memory protection, or service recovery.

## Current status

Phase 1 boot validation is complete in WSL2 Ubuntu with Limine v12.9.3: the kernel builds, the ISO is produced, QEMU reaches `kernel_main`, COM1 reports 20 memory-map entries, and the kernel reaches its controlled halt. Later-phase scheduling, IPC, user-mode isolation, memory protection, and service recovery remain unimplemented.

## Toolchain

The documented build environment is Ubuntu under WSL2 with:

- `x86_64-elf-gcc` and `x86_64-elf-binutils`, or an equivalent GCC that supports freestanding x86-64 output;
- GNU Make;
- Limine v12.9.3 binary assets;
- `xorriso` for ISO creation;
- `qemu-system-x86_64` for validation.

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) and [docs/IMPLEMENTATION_PLAN.md](docs/IMPLEMENTATION_PLAN.md) for the exact assumptions and setup commands.

## Commands

From WSL, in the repository directory:

```sh
make limine-assets
make
make check
make run
```

`make run` emits serial output in the terminal and leaves QEMU at the kernel's controlled halt. Exit safely with `Ctrl-A`, then `X` in the QEMU terminal. It uses only a disposable ISO under `build/`; it never touches a physical disk.

Before every change, read [AGENTS.md](AGENTS.md), inspect source and Git state, and preserve the last known-working bootable commit.
