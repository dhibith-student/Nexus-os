TARGET ?= x86_64-linux-gnu
CC := $(TARGET)-gcc
LD := $(TARGET)-ld
OBJCOPY := $(TARGET)-objcopy
CFLAGS := -std=c11 -ffreestanding -fno-stack-protector -fno-pie -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -Wall -Wextra -Werror -O2 -Iinclude
LDFLAGS := -nostdlib -z max-page-size=0x1000 -T linker.ld
BUILD := build
LIMINE := tools/limine
KERNEL := $(BUILD)/nexus.elf
ISO := $(BUILD)/nexus.iso

.PHONY: all clean limine-assets check run
all: $(ISO)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/main.o: kernel/main.c include/limine.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/entry.o: arch/x86_64/entry.S | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(BUILD)/entry.o $(BUILD)/main.o linker.ld
	$(LD) $(LDFLAGS) -o $@ $(BUILD)/entry.o $(BUILD)/main.o
	$(OBJCOPY) --only-keep-debug $@ $(BUILD)/nexus.debug

$(ISO): $(KERNEL) limine.cfg $(LIMINE)/limine-bios-cd.bin $(LIMINE)/limine-uefi-cd.bin $(LIMINE)/limine-bios.sys
	rm -rf $(BUILD)/iso_root
	mkdir -p $(BUILD)/iso_root/boot/limine $(BUILD)/iso_root/EFI/BOOT
	cp $(KERNEL) $(BUILD)/iso_root/boot/nexus.elf
	cp limine.cfg $(BUILD)/iso_root/boot/limine/limine.cfg
	cp $(LIMINE)/limine-bios-cd.bin $(BUILD)/iso_root/boot/limine/
	cp $(LIMINE)/limine-uefi-cd.bin $(BUILD)/iso_root/boot/limine/
	cp $(LIMINE)/limine-bios.sys $(BUILD)/iso_root/boot/limine/
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image --protective-msdos-label $(BUILD)/iso_root -o $(ISO)

limine-assets:
	@echo "Place pinned Limine binary assets in $(LIMINE). See docs/IMPLEMENTATION_PLAN.md."

check: $(ISO)
	@test -s $(KERNEL)
	@test -s $(ISO)
	@echo "Kernel and ISO artifacts exist. QEMU boot evidence is still required."

run: $(ISO)
	qemu-system-x86_64 -M q35 -m 128M -serial stdio -display none -no-reboot -no-shutdown -cdrom $(ISO)

clean:
	rm -rf $(BUILD)
