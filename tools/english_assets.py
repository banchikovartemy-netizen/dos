#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
A = ROOT / "assets" / "pc"

(A / "files" / "readme.txt").write_text(
    "PCOS 0.7\n"
    "RETRO-FUTURE LOW-POWER OPERATING SYSTEM\n\n"
    "Use the mouse to click applications and controls.\n"
    "TAB switches keyboard focus between the sidebar and the active app.\n"
    "F12 or Alt+Shift switches EN/RU keyboard input.\n"
    "Top HUD shows CMOS date, HH:MM:SS and DAY/NIGHT.\n"
    "ANIMATIONS contains 10 procedural ASCII scenes with inline, left-HUD and fullscreen modes.\n"
    "MAP contains COUNTRIES, ROADS, RAIL, CITIES and TIME ZONES schematic layers.\n"
    "Terminal: type help.\n",
    encoding="utf-8",
)

(A / "files" / "notes.txt").write_text(
    "PCOS // NOTES\n\nAUTOSAVE IS ENABLED. WITH A PCOSDATA DISK ATTACHED, CHANGES CAN PERSIST ACROSS REBOOTS.\n",
    encoding="utf-8",
)

(A / "config" / "display.cfg").write_text(
    "MODE=AUTO\nTHEME=TERMINAL_RED\nRENDER=ASCII_HUD\nLANGUAGE=ENGLISH\nCLOCK=CMOS_RTC\n",
    encoding="utf-8",
)
(A / "config" / "network.cfg").write_text(
    "MODE=DHCP\nDRIVER=RTL8139\nWEB_BRIDGE=10.0.2.2:7777\n",
    encoding="utf-8",
)

apps = [
    "files", "terminal", "notes", "player", "photos", "video",
    "calculator", "games", "browser", "animations", "map", "network", "system", "settings",
]
for name in apps:
    (A / "apps" / f"{name}.app").write_text(
        f"PCOS APP DESCRIPTOR\nNAME={name.upper()}\n",
        encoding="utf-8",
    )

(A / "games" / "DOOM1.TXT").write_text(
    "DOOM 1 LAUNCHER\n\n"
    "PCOS does not redistribute the commercial DOOM1.WAD.\n"
    "Attach a bootable DOS disk containing your legally obtained DOOM files as HDD2,\n"
    "then open GAMES -> DOOM 1 or run the terminal command: doom\n",
    encoding="utf-8",
)

(A / "system" / "version.txt").write_text(
    "PCOS 0.7\nKERNEL=i386\nUI=ASCII_HUD\nLANGUAGE=ENGLISH\nINPUT=EN_RU_SWITCHABLE\nCLOCK=CMOS_RTC_DAY_NIGHT\nANIMATIONS=10_PROCEDURAL_ASCII\nMAP=OFFLINE_HUD_ATLAS\nBROWSER=CHROMIUM_WEB_BRIDGE\nDOOM=LEGACY_DOS_HDD2_LAUNCHER\n",
    encoding="utf-8",
)
