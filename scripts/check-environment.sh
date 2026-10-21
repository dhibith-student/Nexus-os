#!/usr/bin/env bash
set -u
for tool in x86_64-linux-gnu-gcc x86_64-linux-gnu-ld x86_64-linux-gnu-objcopy make xorriso qemu-system-x86_64; do
    if command -v "$tool" >/dev/null 2>&1; then
        printf '%-24s OK: %s\n' "$tool" "$(command -v "$tool")"
    else
        printf '%-24s MISSING\n' "$tool"
    fi
done
