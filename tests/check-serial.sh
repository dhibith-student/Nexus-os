#!/usr/bin/env bash
set -eu
log="${1:?usage: check-serial.sh serial.log kernel.elf kernel.iso}"
kernel="${2:?usage: check-serial.sh serial.log kernel.elf kernel.iso}"
iso="${3:?usage: check-serial.sh serial.log kernel.elf kernel.iso}"

test -s "$log"
test -s "$kernel"
test -s "$iso"

if test "$log" -ot "$kernel" || test "$log" -ot "$iso"; then
    echo "serial evidence is older than the kernel or ISO" >&2
    exit 1
fi

grep -F "NEXUS OS - Phase 1" "$log" >/dev/null
grep -F "[init] serial console: ready" "$log" >/dev/null
grep -F "[init] kernel entry: reached" "$log" >/dev/null
grep -Eq '^\[init\] memory map entries: 0x0*[1-9a-fA-F][0-9a-fA-F]*[[:space:]]*$' "$log"
test "$(grep -Ec '^\[mem\] base=0x[0-9a-fA-F]+ length=0x[0-9a-fA-F]+ type=0x[0-9a-fA-F]+[[:space:]]*$' "$log")" -gt 0
grep -F "[init] scheduler: deferred to Phase 2" "$log" >/dev/null
grep -F "[init] IPC and recovery: deferred to later phases" "$log" >/dev/null
grep -F "[halt] initialization complete; controlled halt" "$log" >/dev/null

if grep -F "NEXUS PANIC:" "$log" >/dev/null; then
    echo "serial evidence contains a kernel panic" >&2
    exit 1
fi

echo "serial acceptance checks passed"
