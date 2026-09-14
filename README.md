# PCOS 0.3

A tiny freestanding i686 operating system written in C + x86 Assembly, designed for low-end hardware and a retro-futuristic ASCII/HUD interface.

Core features: VGA/framebuffer HUD up to 2560x1440, PS/2 input, simple `pc/` filesystem + safe PCOSDATA persistence, Linux-like shell, Files, Notes, Calculator, BMP Photos, low-power AVI Video, PCM WAV/SB16 Player with ASCII cover art, RTL8139 DHCP/ping networking, System/Settings, and two DOS paths.

**DOS86** runs COM/MZ real-mode programs inside the PCOS Games pane with 8086/80186 emulation, DOS/BIOS services, VGA13/Mode-X foundations and an SB16 bridge.

**Legacy DOS** (`L` in Games or `legacydos`) leaves PCOS, returns the CPU from protected mode to BIOS real mode, and chainloads bootable BIOS HDD2 (`DL=81h`). This is the practical path for native 386/DOS4GW games. Reboot to return to PCOS. BIOS/CSM is required for this mode.

Build on Arch:

```bash
sudo pacman -S --needed base-devel grub xorriso qemu-system-x86
make clean
make check
make iso
make run
```

Legacy DOS under QEMU:

```bash
make run-legacy DOSIMG=/path/to/bootable-dos-hdd.img
```

Optional FAT image injection (requires `mtools`):

```bash
make legacy-add-game DOSIMG=/path/to/dos.img GAME=/path/to/game
```

No proprietary DOS or game binaries are bundled.
