#!/usr/bin/env bash
set -eu

version="v12.9.3"
archive_url="https://github.com/Limine-Bootloader/Limine/releases/download/${version}/limine-binary.tar.xz"
expected_sha256="e02a473c6dc6df25159697f94930272397f21fd86e5d4eeee61bba72c95ec8d2"
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
asset_dir="${repo_root}/tools/limine"
temp_dir="$(mktemp -d)"
trap 'rm -rf "$temp_dir"' EXIT

archive="${temp_dir}/limine-binary-${version}.tar.xz"
curl -fL --retry 2 "${archive_url}" -o "${archive}"
printf '%s  %s\n' "${expected_sha256}" "${archive}" | sha256sum -c -
mkdir -p "${asset_dir}"
tar -xJf "${archive}" -C "${asset_dir}" --strip-components=1 \
    limine-binary/limine-bios.sys \
    limine-binary/limine-bios-cd.bin \
    limine-binary/limine-uefi-cd.bin
printf 'Limine %s assets installed in %s\n' "${version}" "${asset_dir}"
