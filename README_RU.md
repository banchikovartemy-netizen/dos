# PCOS 0.7 — лёгкая ретро-футуристическая ОС

PCOS — маленькая 32-битная x86/i386 ОС с собственным freestanding-ядром на C + Assembly. Linux под ней нет. Основная идея: слабое железо, очень простой интерфейс и ASCII/HUD-дизайн в стиле ретро-футуристического машинного зрения.

## Что изменилось в 0.7

Интерфейс снова английский. Русская раскладка не удалена: `F12` или `Alt+Shift` по-прежнему переключают `EN/RU`. По умолчанию используется английская раскладка.

Главный режим теперь `FULL / 800x600`: он сразу включает мышь, RTL8139 и SB16. На экране есть собственный HUD-курсор. Мышью можно выбирать приложения в левой панели и нажимать элементы внутри приложений. `Tab` переключает клавиатурный фокус между меню и приложением.

Приложения: FILES, TERMINAL, NOTES, PLAYER, PHOTOS, VIDEO, CALCULATOR, GAMES, BROWSER, NETWORK, SYSTEM, SETTINGS.

## Браузер

PCOS остаётся слишком маленькой для встроенного Chromium, поэтому BROWSER использует PCOS Web Bridge. На хост-компьютере запускается настоящий Chromium через Playwright, а PCOS отправляет URL по своему лёгкому сетевому протоколу и получает текстовое представление страницы. Это позволяет загружать современные HTTPS/HTML/CSS/JavaScript сайты, не превращая ядро PCOS в многогигабайтную систему.

Установка Web Bridge на Arch:

```bash
python -m venv .venv
.venv/bin/pip install playwright
.venv/bin/playwright install chromium
.venv/bin/python tools/web_bridge.py
```

После этого во втором терминале:

```bash
make run
```

В PCOS открой `BROWSER`, введи адрес и нажми Enter или `[GO]`. Для QEMU мост доступен как `10.0.2.2:7777`.

Текущая версия браузера передаёт заголовок и видимый текст страницы. Полное потоковое графическое отображение Chromium, клики по DOM, cookies/login и загрузки файлов — следующий этап протокола Web Bridge.

## DOOM 1

В GAMES появился пункт `DOOM 1 [LEGACY DOS HDD2]`, а в Terminal есть команда:

```text
doom
```

DOOM требует 386/DOS4GW, поэтому запускается через уже существующий Legacy DOS chainloader. PCOS не распространяет коммерческий `DOOM1.WAD` или MS-DOS. Нужен твой легально полученный DOOM и загрузочный DOS-образ.

Запуск в QEMU:

```bash
make run-doom DOOMIMG=/path/to/bootable-dos-with-doom.img
```

После запуска PCOS выбери `GAMES -> DOOM 1`.

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
sudo pacman -S --needed base-devel grub xorriso qemu-system-x86 mtools
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
