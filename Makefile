CC ?= gcc
HOSTCC ?= gcc
LD ?= ld
NM ?= nm
CFLAGS := -m32 -march=i386 -mtune=generic -std=gnu99 -O2 -ffreestanding -fno-pie -fno-pic -fno-plt -fno-stack-protector -fno-builtin -fno-asynchronous-unwind-tables -fno-unwind-tables -mpreferred-stack-boundary=2 -mincoming-stack-boundary=2 -Wall -Wextra -Wno-unused-parameter -Wno-misleading-indentation -Iinclude
LDFLAGS := -m elf_i386 -T linker.ld
C_SRC := $(wildcard kernel/*.c drivers/*.c ui/*.c apps/*.c fs/*.c net/*.c media/*.c audio/*.c dos/*.c)
OBJ := $(patsubst %.c,build/%.o,$(C_SRC)) build/boot/boot.o build/boot/legacy.o build/boot/isr.o
ASSET_TAR := build/pcfs.tar
ISO := build/pcos-0.7.iso

.PHONY: all clean assets iso run run-audio run-debug run-legacy run-doom web-bridge check test-dos86 add-game legacy-add-game doom-install

all: build/pcos.elf assets

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/boot/boot.o: boot/boot.S
	@mkdir -p $(dir $@)
	$(CC) -m32 -march=i386 -ffreestanding -fno-pie -fno-pic -c $< -o $@

build/boot/legacy.o: boot/legacy.S
	@mkdir -p $(dir $@)
	$(CC) -m32 -march=i386 -ffreestanding -fno-pie -fno-pic -c $< -o $@

build/boot/isr.o: boot/isr.S
	@mkdir -p $(dir $@)
	$(CC) -m32 -march=i386 -ffreestanding -fno-pie -fno-pic -c $< -o $@

build/pcos.elf: $(OBJ) linker.ld
	$(LD) $(LDFLAGS) $(OBJ) -o $@

assets:
	@mkdir -p build
	python3 tools/make_assets.py
	python3 tools/english_assets.py
	tar --format=ustar -cf $(ASSET_TAR) -C assets pc

test-dos86: assets
	$(HOSTCC) -std=gnu99 -O0 -Iinclude tools/dos86_smoketest.c dos/dos86.c kernel/lib.c -o build/dos86-smoketest
	./build/dos86-smoketest

check: build/pcos.elf assets test-dos86
	@echo "== ELF =="
	@file build/pcos.elf
	@echo "== Kernel =="
	@ls -lh build/pcos.elf
	@echo "== Initrd =="
	@ls -lh $(ASSET_TAR)
	@echo "== Multiboot =="
	@python3 tools/check_multiboot.py build/pcos.elf
	@echo "== Legacy DOS chainloader =="
	@python3 tools/check_legacy.py build/boot/legacy.o
	@echo "== pcfs =="
	@tar tf $(ASSET_TAR) | head -n 40
	@python3 tools/check_assets.py
	@echo "== Undefined symbols =="
	@test -z "$$($(NM) -u build/pcos.elf 2>/dev/null)" || ( $(NM) -u build/pcos.elf; exit 1 )

iso: build/pcos.elf assets
	@mkdir -p iso/boot/grub
	cp build/pcos.elf iso/boot/pcos.elf
	cp $(ASSET_TAR) iso/boot/pcfs.tar
	grub-mkrescue -o $(ISO) iso
	@echo "Created $(ISO)"

run: iso
	qemu-system-i386 -m 32M -vga std -drive file=data/pcos-data.img,format=raw,if=ide,index=0 -cdrom $(ISO) -boot d -nic user,model=rtl8139 -device sb16 -no-reboot -no-shutdown

run-audio: iso
	qemu-system-i386 -m 32M -vga std -drive file=data/pcos-data.img,format=raw,if=ide,index=0 -cdrom $(ISO) -boot d -nic user,model=rtl8139 -audiodev pa,id=snd0 -device sb16,audiodev=snd0 -no-reboot -no-shutdown

run-legacy: iso
	@test -n "$(DOSIMG)" || (echo "Usage: make run-legacy DOSIMG=/path/to/bootable-dos-disk.img"; exit 1)
	qemu-system-i386 -m 32M -vga std \
		-drive file=data/pcos-data.img,format=raw,if=ide,index=0 \
		-drive file="$(DOSIMG)",format=raw,if=ide,index=1 \
		-cdrom $(ISO) -boot d \
		-nic user,model=rtl8139 -device sb16

run-doom: iso
	@test -n "$(DOOMIMG)" || (echo "Usage: make run-doom DOOMIMG=/path/to/bootable-dos-with-doom.img"; exit 1)
	qemu-system-i386 -m 32M -vga std \
		-drive file=data/pcos-data.img,format=raw,if=ide,index=0 \
		-drive file="$(DOOMIMG)",format=raw,if=ide,index=1 \
		-cdrom $(ISO) -boot d \
		-nic user,model=rtl8139 -device sb16

web-bridge:
	python3 tools/web_bridge.py

run-debug: iso
	qemu-system-i386 -m 32M -vga std -drive file=data/pcos-data.img,format=raw,if=ide,index=0 -cdrom $(ISO) -boot d -nic user,model=rtl8139 -device sb16 -no-reboot -no-shutdown -d guest_errors,cpu_reset -debugcon stdio -global isa-debugcon.iobase=0xe9

clean:
	rm -rf build/* iso/boot/pcos.elf iso/boot/pcfs.tar

add-game:
	@test -n "$(GAME)" || (echo "Usage: make add-game GAME=/path/to/game-or-exe"; exit 1)
	python3 tools/add_dos_game.py "$(GAME)"
	$(MAKE) assets

legacy-add-game:
	@test -n "$(DOSIMG)" -a -n "$(GAME)" || (echo "Usage: make legacy-add-game DOSIMG=/path/dos.img GAME=/path/game"; exit 1)
	./tools/inject_legacy_game.sh "$(DOSIMG)" "$(GAME)"

doom-install:
	@test -n "$(DOSIMG)" -a -n "$(GAME)" || (echo "Usage: make doom-install DOSIMG=/path/dos.img GAME=/path/to/DOOM-directory-or-files"; exit 1)
	./tools/inject_legacy_game.sh "$(DOSIMG)" "$(GAME)"
