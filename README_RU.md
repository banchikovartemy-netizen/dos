# PCOS 0.7 — лёгкая ретро-футуристическая ОС

PCOS — маленькая 32-битная x86/i386 ОС с собственным freestanding-ядром на C + Assembly. Linux под ней нет. Основная идея: слабое железо, очень простой интерфейс и ASCII/HUD-дизайн в стиле ретро-футуристического машинного зрения.

## Что изменилось в 0.7

Интерфейс снова английский. Русская раскладка не удалена: `F12` или `Alt+Shift` по-прежнему переключают `EN/RU`. По умолчанию используется английская раскладка.

Главный режим теперь `FULL / 800x600`: он сразу включает мышь, RTL8139 и SB16. На экране есть собственный HUD-курсор. Мышью можно выбирать приложения в левой панели и нажимать элементы внутри приложений. `Tab` переключает клавиатурный фокус между меню и приложением.

Приложения: FILES, TERMINAL, NOTES, PLAYER, PHOTOS, VIDEO, CALCULATOR, GAMES, BROWSER, NETWORK, SYSTEM, SETTINGS.

## Браузер

PCOS остаётся слишком маленькой для встроенного Chromium, поэтому BROWSER использует PCOS Web Bridge. На хост-компьютере запускается настоящий Chromium через Playwright: именно он обрабатывает HTTPS, современный HTML/CSS и JavaScript. PCOS получает уменьшенный ASCII-кадр страницы и показывает его в своём HUD, поэтому внешний стиль системы не меняется.

Один раз установи Web Bridge:

```bash
make web-setup
```

После этого PCOS вместе с браузерным мостом запускается одной командой:

```bash
make run-web
```

В PCOS открой `BROWSER`, введи адрес и нажми Enter или `[GO]`. Для QEMU мост доступен как `10.0.2.2:7777`.

ASCII-страница интерактивная: клик по символам переводится в координаты настоящего Chromium, а клавиатура после клика передаётся выбранному полю или элементу страницы. Enter, Backspace и стрелки тоже отправляются Chromium. Чтобы снова редактировать адрес, кликни по строке URL. Cookies и сессия сохраняются внутри работающего Web Bridge. Некоторые сайты с DRM, CAPTCHA или жёсткой антибот-защитой всё равно могут ограничивать автоматизированный Chromium.

## DOOM 1

В GAMES появился пункт `DOOM 1 [LEGACY DOS HDD2]`, а в Terminal есть команда:

```text
doom
```

PCOS не распространяет коммерческий `DOOM1.WAD` или MS-DOS. Нужны твои легально полученные `DOOM.EXE` + `DOOM1.WAD`/`DOOM.WAD` и загрузочный DOS/FreeDOS FAT-образ.

Подготовить отдельный Doom-образ можно так:

```bash
make doom-install DOSIMG=/path/to/freedos.img GAME=/path/to/DOOM
```

Установщик копирует игру в `C:\DOOM` и добавляет запуск `DOOM.EXE` в `AUTOEXEC.BAT`. После этого запускай PCOS с этим диском:

```bash
make run-doom DOOMIMG=/path/to/freedos.img
```

В PCOS нажми `GAMES -> DOOM 1`. PCOS передаст управление второму DOS-диску, а его `AUTOEXEC.BAT` автоматически запустит Doom.

## Управление

- мышь — перемещение HUD-курсора и клики;
- `Tab` — переключить клавиатурный фокус меню/приложение;
- `↑ / ↓` — выбор в текущем фокусе;
- `Enter` — открыть/подтвердить;
- `F1..F11` — быстрый запуск первых приложений;
- `F12` или `Alt+Shift` — `EN/RU`.

## Терминал

```text
ls cd pwd cat echo touch mkdir rm mv write
mem net ping dhcp play dos doom legacydos browse
theme clear reboot shutdown
```

Пример открытия сайта из терминала:

```text
browse example.com
```

## Сборка на Arch Linux

```bash
sudo pacman -S --needed base-devel grub xorriso qemu-system-x86 mtools python
cd ~/dos
git pull
make clean
make check
make iso
make run
```

ISO создаётся как:

```text
build/pcos-0.7.iso
```

Если FULL-режим не работает на конкретном железе, в GRUB остаются SAFE и EMERGENCY VGA TEXT режимы.
