TARGET ?= x86_64-linux-gnu
CC := $(TARGET)-gcc
LD := $(TARGET)-ld
OBJCOPY := $(TARGET)-objcopy
QEMU ?= qemu-system-x86_64
QEMU_TIMEOUT ?= 8
EXCEPTION_TEST ?= 0
CFLAGS := -std=c11 -ffreestanding -fno-stack-protector -fno-pie -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -Wall -Wextra -Werror -O2 -Iinclude -DNEXUS_EXCEPTION_TEST=$(EXCEPTION_TEST)
LDFLAGS := -nostdlib -z max-page-size=0x1000 -T linker.ld
BUILD := build
LIMINE := tools/limine
KERNEL := $(BUILD)/nexus.elf
ISO := $(BUILD)/nexus.iso
SERIAL_LOG := $(BUILD)/serial.log
MEMORY_TEST := $(BUILD)/memory-accounting-test
DESCRIPTOR_TEST := $(BUILD)/descriptor-test
PMM_TEST := $(BUILD)/physical-memory-test

.PHONY: all clean limine-assets check test-memory-accounting test-descriptors test-physical-memory run
all: $(ISO)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/main.o: kernel/main.c include/limine.h include/exceptions.h include/memory_accounting.h include/serial.h include/arch_state.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/serial.o: kernel/serial.c include/serial.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/memory_accounting.o: kernel/memory_accounting.c include/memory_accounting.h include/limine.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/descriptors.o: kernel/descriptors.c include/descriptors.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/arch_state.o: kernel/arch_state.c include/arch_state.h include/serial.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/physical_memory.o: kernel/physical_memory.c include/physical_memory.h include/limine.h include/memory_accounting.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/exceptions.o: kernel/exceptions.c include/exceptions.h include/serial.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/entry.o: arch/x86_64/entry.S | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/exceptions-asm.o: arch/x86_64/exceptions.S | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(BUILD)/entry.o $(BUILD)/exceptions-asm.o $(BUILD)/main.o $(BUILD)/serial.o $(BUILD)/memory_accounting.o $(BUILD)/descriptors.o $(BUILD)/arch_state.o $(BUILD)/physical_memory.o $(BUILD)/exceptions.o linker.ld
	$(LD) $(LDFLAGS) -o $@ $(BUILD)/entry.o $(BUILD)/exceptions-asm.o $(BUILD)/main.o $(BUILD)/serial.o $(BUILD)/memory_accounting.o $(BUILD)/descriptors.o $(BUILD)/arch_state.o $(BUILD)/physical_memory.o $(BUILD)/exceptions.o
	$(OBJCOPY) --only-keep-debug $@ $(BUILD)/nexus.debug

$(ISO): $(KERNEL) limine.conf $(LIMINE)/limine-bios-cd.bin $(LIMINE)/limine-uefi-cd.bin $(LIMINE)/limine-bios.sys
	rm -rf $(BUILD)/iso_root
	mkdir -p $(BUILD)/iso_root/boot/limine $(BUILD)/iso_root/EFI/BOOT
	cp $(KERNEL) $(BUILD)/iso_root/boot/nexus.elf
	cp limine.conf $(BUILD)/iso_root/boot/limine/limine.conf
	cp $(LIMINE)/limine-bios-cd.bin $(BUILD)/iso_root/boot/limine/
	cp $(LIMINE)/limine-uefi-cd.bin $(BUILD)/iso_root/boot/limine/
	cp $(LIMINE)/limine-bios.sys $(BUILD)/iso_root/boot/limine/
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label $(BUILD)/iso_root -o $(ISO)

limine-assets:
	bash scripts/fetch-limine.sh

$(BUILD)/memory-test-start.o: tests/test-start.S | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(MEMORY_TEST): tests/test-memory-accounting.c kernel/memory_accounting.c $(BUILD)/memory-test-start.o include/memory_accounting.h include/limine.h | $(BUILD)
	$(CC) $(CFLAGS) -nostdlib -static -no-pie -Wl,-e,_start tests/test-memory-accounting.c kernel/memory_accounting.c $(BUILD)/memory-test-start.o -o $@

test-memory-accounting: $(MEMORY_TEST)
	$(MEMORY_TEST)

$(DESCRIPTOR_TEST): tests/test-descriptors.c kernel/descriptors.c $(BUILD)/memory-test-start.o include/descriptors.h | $(BUILD)
	$(CC) $(CFLAGS) -nostdlib -static -no-pie -Wl,-e,_start tests/test-descriptors.c kernel/descriptors.c $(BUILD)/memory-test-start.o -o $@

test-descriptors: $(DESCRIPTOR_TEST)
	$(DESCRIPTOR_TEST)

$(PMM_TEST): tests/test-physical-memory.c kernel/physical_memory.c $(BUILD)/memory-test-start.o include/physical_memory.h include/limine.h include/memory_accounting.h | $(BUILD)
	$(CC) $(CFLAGS) -nostdlib -static -no-pie -Wl,-e,_start tests/test-physical-memory.c kernel/physical_memory.c $(BUILD)/memory-test-start.o -o $@

test-physical-memory: $(PMM_TEST)
	$(PMM_TEST)

check: $(ISO) test-memory-accounting test-descriptors test-physical-memory
	@test -s $(KERNEL)
	@test -s $(ISO)
	@rm -f $(SERIAL_LOG)
	@set +e; timeout $(QEMU_TIMEOUT)s $(QEMU) -M q35 -m 128M -serial file:$(SERIAL_LOG) -display none -no-reboot -no-shutdown -boot d -cdrom $(ISO); status=$$?; set -e; if [ "$$status" -ne 124 ]; then echo "QEMU did not reach the expected controlled-halt timeout (status=$$status)" >&2; exit "$$status"; fi
	@bash tests/check-serial.sh $(SERIAL_LOG) $(KERNEL) $(ISO)
	@echo "Kernel, ISO, architecture helpers, memory accounting, physical memory core, and fresh QEMU serial boot evidence verified."

run: $(ISO)
	$(QEMU) -M q35 -m 128M -serial stdio -display none -no-reboot -no-shutdown -cdrom $(ISO)

clean:
	rm -rf $(BUILD)
