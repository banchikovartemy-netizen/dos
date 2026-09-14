# PCOS 0.7 — лёгкая ретро-футуристическая ОС

PCOS — маленькая 32-битная x86/i386 bare-metal ОС на C + Assembly. Главная идея — слабое железо, минимум визуального шума и собственный ASCII/HUD-интерфейс с атмосферой CRT, военной телеметрии и интерфейсов игр конца 80-х/90-х.

## Новый ультра-простой интерфейс

Экран разделён только на три части:

```text
TOP STATUS BAR
------------------+-----------------------------------------------------------
[icon] FILES      :
[icon] TERMINAL   :
[icon] NOTES      :                  ACTIVE APPLICATION
[icon] PLAYER     :
...               :
------------------+-----------------------------------------------------------
```

Слева находится только список приложений с маленькими ASCII-иконками. Никаких подсказок, статусов и дополнительных панелей в боковой полосе больше нет. Выбранное приложение отмечается как отдельный inventory-slot: `> [icon] NAME <`.

Справа — только рабочая область текущего приложения. Сверху — тонкая системная строка: `PCOS`, текущая раскладка, дата, `HH:MM:SS` и `DAY/NIGHT` из CMOS RTC.

Интерфейс английский, но `F12` или `Alt+Shift` по-прежнему переключают `EN/RU`.

Приложения: FILES, TERMINAL, NOTES, PLAYER, PHOTOS, VIDEO, CALCULATOR, GAMES, BROWSER, ANIMATIONS, MAP, GUIDE, FETCH SETUP, NETWORK, SYSTEM, SETTINGS.

## GUIDE

Все инструкции вынесены из рабочего стола в отдельное приложение `GUIDE`. Там находятся основные клавиши, управление мышью, браузером, анимациями, картой, Doom и терминалом.

Основное управление:

- мышь — выбрать приложение или нажать элемент;
- `Tab` — переключить фокус `sidebar <-> application`;
- `↑ / ↓` — выбор;
- `Enter` — открыть/подтвердить;
- `F1..F11` — быстрый запуск первых приложений;
- `F12` или `Alt+Shift` — `EN/RU`.

## PCFETCH

В Terminal появился живой аналог fastfetch:

```text
pcfetch
```

Также работают алиасы `fastfetch` и `fetch`.

Вместо статичного ASCII-логотипа слева вращается процедурный ASCII-куб. Справа показываются выбранные системные параметры: CPU, RAM, display, network/IP, keyboard layout, clock, theme и shell. Экран обновляется в реальном времени.

`Esc`, `Enter` или `Q` закрывают PCFETCH.

### FETCH SETUP

Отдельное приложение `FETCH SETUP` управляет PCFETCH. Можно включать/выключать CPU, RAM, display, network, clock и input layout, выбирать стиль `TACTICAL / CLEAN / DIAGNOSTIC` и скорость вращения куба `SLOW / NORMAL / FAST`.

## ANIMATIONS

Встроены 10 процедурных ASCII-анимаций: `SPIN CUBE`, `TARGET LOCK`, `RADAR SWEEP`, `DNA HELIX`, `ORBIT CORE`, `WAVEFORM`, `MATRIX RAIN`, `DATA TUNNEL`, `SATELLITE`, `REACTOR`.

## MAP

`MAP` — офлайн ASCII/HUD-атлас мира. Режимы: `COUNTRIES`, `ROADS`, `RAIL`, `CITIES`, `TIME ZONES`. Это схематическая эстетичная карта, а не навигационный сервис.

## Браузер

BROWSER использует PCOS Web Bridge: настоящий Chromium на хосте обрабатывает HTTPS/HTML/CSS/JavaScript, а PCOS показывает интерактивный ASCII-кадр.

```bash
make web-setup
make run-web
```

## DOOM 1

В `GAMES` есть `DOOM 1 [LEGACY DOS HDD2]`, а в Terminal — команда `doom`. Коммерческий `DOOM1.WAD` не включён в репозиторий.

```bash
make doom-install DOSIMG=/path/to/freedos.img GAME=/path/to/DOOM
make run-doom DOOMIMG=/path/to/freedos.img
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

ISO: `build/pcos-0.7.iso`.
