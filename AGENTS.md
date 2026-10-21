# NEXUS OS project rules

## Mandatory iteration rules

- Read `AGENTS.md` before every Codex iteration, inspect the current source and Git state, and follow these rules throughout every change.
- Preserve the most recent known-working bootable kernel.
- Make small, reviewable, dependency-aware changes.
- Never substitute a simulator, dashboard, or mock for an actual kernel mechanism.
- Never claim user-mode isolation, memory protection, IPC, task switching, or crash recovery without implementing and testing the relevant behavior.
- Use freestanding C for kernel code, with assembly only where necessary.
- Document compiler flags, linker configuration, boot assumptions, architecture-specific behavior, and kernel interfaces.
- Do not introduce unverified context-switch or interrupt-handling code.
- Keep module boundaries explicit and agree on interfaces before parallel implementation.
- Make recovery policies bounded, reject invalid restart requests, track service instances, and invalidate stale endpoints.
- Run relevant build and test commands after meaningful changes and report actual results.
- Never remove failing tests, weaken assertions, or fabricate a successful build.
- Test experimental kernels in QEMU, never by replacing the host operating system or modifying physical boot records.
- Do not overwrite user data or execute privileged host operations without explicit approval.
- Update tests and documentation when behavior or interfaces change.
- Distinguish implemented functionality from stubs, future work, and incomplete features.
- Do not invent citations, benchmark results, test results, or research claims.
- For every iteration, report files changed, commands executed, observed results, defects, and the next milestone.
- Preserve known-working milestones with Git commits.
- Prioritize a working bootable kernel over lower-priority features when the deadline is at risk.

## Definition of done

A feature is complete only when its implementation, documented behavior, relevant tests, and integration evidence support the claim. A source file or successful compilation alone is not proof of a bootable kernel.

All later phases and every future Codex prompt must explicitly instruct Codex to read and follow this file before modifying the project.

## Phase 1 scope

Phase 1 intentionally implements only a bootable diagnostic kernel. Scheduling, IPC, privilege separation, memory protection, service supervision, and recovery remain future work until their mechanisms and tests exist.
