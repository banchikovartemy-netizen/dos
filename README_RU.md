# PCOS 0.3 — Retro Future Low-Power OS

PCOS — маленькая 32-битная x86/i686 ОС с собственным freestanding-ядром на C + Assembly. Linux под ней нет. Главная цель — очень слабое железо, простой интерфейс и ретро-футуристический ASCII/HUD в духе машинного зрения из sci-fi.

## Интерфейс

Экран постоянно разделён на две части: слева список приложений, справа — одно активное приложение. Перемещаемых окон и композитора нет. UI строится из bitmap-шрифта, линий, ASCII-символов и простых прямоугольников, поэтому почти не тратит CPU/GPU.

Приложения: Files, Terminal, Notes, Player, Photos, Video, Calculator, Games, Network, System, Settings.

Темы: Terminal Red, Terminal Green, Amber, Ice Blue, Monochrome.

## Видео

PCOS умеет работать через VGA text fallback и Multiboot framebuffer. В GRUB есть профили 800x600, 1024x768, 1280x720, 1920x1080 и 2560x1440. Один и тот же HUD масштабируется без отдельного тяжёлого desktop environment.

## Файлы

Видимая структура максимально простая:

```text
pc/
├── apps/
├── files/
├── games/
├── music/
├── photos/
├── video/
├── system/
├── config/
└── temp/
```

База загружается из маленького `pcfs.tar`. Изменения Notes/Files идут в RAM-overlay. Если присутствует отдельный диск с сигнатурой `PCOSDATA`, изменения сохраняются после перезагрузки. PCOS не включает запись на случайный чужой диск без своей сигнатуры.

## Terminal

Linux-подобные команды первой системы:

```text
ls cd pwd cat echo touch mkdir rm mv write
mem net ping dhcp play dos legacydos
theme clear reboot shutdown
```

## Media

- Photos: BMP 24-bit, масштабирование в framebuffer.
- Video: очень лёгкий uncompressed AVI/DIB — специально для старых CPU.
- Player: PCM WAV 8-bit mono через Sound Blaster 16 DMA.
- Player рисует псевдообложку в ASCII/HUD-стиле.
- В initrd уже лежат синтетические BMP/WAV/AVI для проверки.

## Network

Базовая сеть рассчитана на RTL8139: PCI discovery, Ethernet, ARP, IPv4, DHCP, ICMP ping. Network показывает MAC/IP/gateway/DNS и RX/TX.

## DOS-игры: два режима

### DOS86 — прямо внутри PCOS

`GAMES -> ENTER` запускает программу в песочнице 1 MiB и оставляет PCOS работающей вокруг неё. Реализованы `.COM`, MZ `.EXE`, расширенный 8086/80186 interpreter, BIOS/DOS API, VGA 320x200 Mode 13h, основа Mode X, клавиатура и Sound Blaster bridge.

### Legacy DOS — для 386/DOS4GW

Нажми `L` в Games или введи `legacydos`. PCOS выходит из protected mode обратно в real mode и через BIOS chainload'ит второй HDD (`DL=81h`). Это путь для тяжёлых DOS-игр вроде DOOM/Duke3D. Для возврата в PCOS нужна перезагрузка. Режим требует BIOS/Legacy/CSM.

## Arch Linux: сборка

```bash
sudo pacman -S --needed base-devel grub xorriso qemu-system-x86
make clean
make check
make iso
make run
```

Legacy DOS:

```bash
make run-legacy DOSIMG=/path/to/bootable-dos-hdd.img
```

Добавить игру в FAT DOS-образ:

```bash
sudo pacman -S mtools
make legacy-add-game DOSIMG=/path/to/dos.img GAME=/path/to/DOOM
```

PCOS не включает коммерческие игры или MS-DOS. Используй свои легально полученные игровые файлы и загрузочный DOS-образ.
