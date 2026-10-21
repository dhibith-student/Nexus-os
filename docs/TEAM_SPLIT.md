# Two-person team split

## Member A — kernel and architecture

Owns the bootable kernel, x86-64 entry and low-level architecture, memory-map handling, task structures, and scheduler. Member A documents and tests the interfaces exported to later services.

## Member B — services and recovery

Owns service lifecycle design, IPC contracts, the service registry, supervisor state machine, restart policy, endpoint reconnection, and recovery tests. Member B works against documented interfaces and must not claim kernel integration until the corresponding kernel mechanisms exist and have integration evidence.

## Shared integration checkpoint

At each checkpoint both members agree on interface definitions and ownership rules, run the combined build, boot the result in QEMU, and execute the recovery-path tests when those mechanisms exist. If the kernel boot milestone is not validated, recovery implementation pauses while the boot path is repaired. Both members review the changed files and preserve the last known-working commit.
