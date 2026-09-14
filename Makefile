CC ?= gcc
HOSTCC ?= gcc
LD ?= ld
NM ?= nm
CFLAGS := -m32 -std=gnu99 -O2 -ffreestanding -fno-pie -fno-stack-protector -fno-builtin -fno-asynchronous-unwind-tables -fno-unwind-tables -Wall -Wextra -Wno-unused-parameter -Wno-misleading-indentation -Iinclude
LDFLAGS := -m elf_i386 -T linker.ld
C_SRC := $(wildcard kernel/*.c drivers/*.c ui/*.c apps/*.c fs/*.c net/*.c media/*.c audio/*.c dos/*.c)
OBJ := $(patsubst %.c,build/%.o,$(C_SRC)) build/boot/boot.o build/boot/legacy.o
ASSET_TAR := build/pcfs.tar

.PHONY: all clean assets iso run run-audio run-debug run-legacy check test-dos86 add-game legacy-add-game

all: build/pcos.elf assets

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/boot/boot.o: boot/boot.S
	@mkdir -p $(dir $@)
	$(CC) -m32 -ffreestanding -fno-pie -c $< -o $@

build/boot/legacy.o: boot/legacy.S
	@mkdir -p $(dir $@)
	$(CC) -m32 -ffreestanding -fno-pie -c $< -o $@

build/pcos.elf: $(OBJ) linker.ld
	$(LD) $(LDFLAGS) $(OBJ) -o $@

assets:
	@mkdir -p build
	python3 tools/make_assets.py

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
	grub-mkrescue -o build/pcos-0.3.iso iso
	@echo "Created build/pcos-0.3.iso"

run: iso
	qemu-system-i386 -m 16M -drive file=data/pcos-data.img,format=raw,if=ide,index=0 -cdrom build/pcos-0.3.iso -nic user,model=rtl8139 -device sb16

run-audio: iso
	qemu-system-i386 -m 32M -drive file=data/pcos-data.img,format=raw,if=ide,index=0 -cdrom build/pcos-0.3.iso -nic user,model=rtl8139 -audiodev pa,id=snd0 -device sb16,audiodev=snd0

run-legacy: iso
	@test -n "$(DOSIMG)" || (echo "Usage: make run-legacy DOSIMG=/path/to/bootable-dos-disk.img"; exit 1)
	qemu-system-i386 -m 32M \
		-drive file=data/pcos-data.img,format=raw,if=ide,index=0 \
		-drive file="$(DOSIMG)",format=raw,if=ide,index=1 \
		-cdrom build/pcos-0.3.iso -boot d \
		-nic user,model=rtl8139 -device sb16

run-debug: iso
	qemu-system-i386 -m 16M -drive file=data/pcos-data.img,format=raw,if=ide,index=0 -cdrom build/pcos-0.3.iso -nic user,model=rtl8139 -device sb16 -debugcon stdio -global isa-debugcon.iobase=0xe9

clean:
	rm -rf build/* iso/boot/pcos.elf iso/boot/pcfs.tar

add-game:
	@test -n "$(GAME)" || (echo "Usage: make add-game GAME=/path/to/game-or-exe"; exit 1)
	python3 tools/add_dos_game.py "$(GAME)"
	$(MAKE) assets

legacy-add-game:
	@test -n "$(DOSIMG)" -a -n "$(GAME)" || (echo "Usage: make legacy-add-game DOSIMG=/path/dos.img GAME=/path/game"; exit 1)
	./tools/inject_legacy_game.sh "$(DOSIMG)" "$(GAME)"
