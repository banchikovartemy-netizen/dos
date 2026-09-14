# PCOS 0.7 — лёгкая ретро-футуристическая ОС

PCOS — маленькая 32-битная x86/i386 ОС с собственным freestanding-ядром на C + Assembly. Linux под ней нет. Основная идея: слабое железо, очень простой интерфейс и ASCII/HUD-дизайн в стиле ретро-футуристического машинного зрения.

## Интерфейс

Интерфейс английский. Русская раскладка не удалена: `F12` или `Alt+Shift` переключают `EN/RU`, английская включена по умолчанию.

Главный режим `FULL / 800x600` сразу включает мышь, RTL8139 и SB16. На экране есть собственный HUD-курсор. Мышью можно выбирать приложения и нажимать элементы внутри них. `Tab` переключает клавиатурный фокус между меню и приложением.

Верхняя HUD-строка теперь постоянно показывает дату и точное время с секундами из CMOS RTC:

```text
2026-09-14 21:18:37 NIGHT
```

С 06:00 до 17:59 выводится `DAY`, в остальное время `NIGHT`.

Приложения: FILES, TERMINAL, NOTES, PLAYER, PHOTOS, VIDEO, CALCULATOR, GAMES, BROWSER, ANIMATIONS, MAP, NETWORK, SYSTEM, SETTINGS.

## ANIMATIONS

В систему встроены 10 процедурных ASCII-анимаций без тяжёлого графического движка:

`SPIN CUBE`, `TARGET LOCK`, `RADAR SWEEP`, `DNA HELIX`, `ORBIT CORE`, `WAVEFORM`, `MATRIX RAIN`, `DATA TUNNEL`, `SATELLITE`, `REACTOR`.

Управление:

- `↑/↓` — выбрать анимацию;
- `Enter` — обычный просмотр в приложении;
- `W` — закрепить маленькое анимированное HUD-окно слева;
- `F` — полноэкранный режим;
- `Esc` — выйти из полноэкранного режима;
- `S` — остановить закрепление/вернуться к обычному просмотру.

## MAP

`MAP` — встроенный офлайн ASCII/HUD-атлас мира в стиле PCOS. Это эстетичная схематическая карта, а не навигационный сервис.

Режимы:

1. `COUNTRIES` — основные страны и их коды;
2. `ROADS` — главные межрегиональные дорожные коридоры;
3. `RAIL` — крупные железнодорожные магистрали;
4. `CITIES` — крупнейшие города мира;
5. `TIME ZONES` — часовые пояса от UTC-12 до UTC+12.

Переключение — клавишами `1..5`, стрелками `←/→` или мышью по верхней панели карты.

## Браузер

PCOS остаётся слишком маленькой для встроенного Chromium, поэтому BROWSER использует PCOS Web Bridge. На хост-компьютере запускается настоящий Chromium через Playwright: он обрабатывает HTTPS, современный HTML/CSS и JavaScript. PCOS получает уменьшенный ASCII-кадр страницы и показывает его в своём HUD.

Один раз установи Web Bridge:

```bash
make web-setup
```

После этого запускай:

```bash
make run-web
```

В PCOS открой `BROWSER`, введи адрес и нажми Enter или `[GO]`. ASCII-страница интерактивная: клики и клавиатурные команды передаются реальному Chromium. Некоторые сайты с DRM, CAPTCHA или жёсткой антибот-защитой могут ограничивать автоматизированный Chromium.

## DOOM 1

В `GAMES` есть `DOOM 1 [LEGACY DOS HDD2]`, а в Terminal — команда `doom`.

PCOS не распространяет коммерческий `DOOM1.WAD` или MS-DOS. Нужны твои легально полученные `DOOM.EXE` + `DOOM1.WAD`/`DOOM.WAD` и загрузочный DOS/FreeDOS FAT-образ.

```bash
make doom-install DOSIMG=/path/to/freedos.img GAME=/path/to/DOOM
make run-doom DOOMIMG=/path/to/freedos.img
```

После клика `GAMES -> DOOM 1` PCOS загружает второй DOS-диск, а его `AUTOEXEC.BAT` запускает Doom.

## Основное управление

- мышь — HUD-курсор и клики;
- `Tab` — фокус меню/приложение;
- `↑ / ↓` — выбор;
- `Enter` — открыть/подтвердить;
- `F1..F11` — быстрый запуск первых приложений;
- `F12` или `Alt+Shift` — `EN/RU`.

## Терминал

```text
ls cd pwd cat echo touch mkdir rm mv write
mem net ping dhcp play dos doom legacydos browse
theme clear reboot shutdown
```

## Сборка на Arch Linux

```bash
sudo pacman -S --needed base-devel grub xorriso qemu-system-x86 mtools python
cd ~/dos
git fetch origin
git reset --hard origin/main
make clean
make check
make iso
make run
```

ISO создаётся как `build/pcos-0.7.iso`. Если FULL-режим не работает на конкретном железе, в GRUB остаются SAFE и EMERGENCY VGA TEXT режимы.
