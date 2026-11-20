# Zuma Deluxe HD — Android Edition

An independent, non-commercial fan project: the Zuma Deluxe HD remake brought to Android phones,
running at a steady **60 FPS** with touch controls, full **Arabic** support and new features.

> [!IMPORTANT]
> This is an **independent fan project**. It is **not** made, endorsed or supported by PopCap Games
> or Electronic Arts. Zuma is a trademark of its owners, and the original game graphics, sounds
> and music belong to PopCap / EA. The code is a fan remake (MIT License).
> If you enjoy Zuma, please support the original game.

![preview](https://user-images.githubusercontent.com/52833080/162574925-d6c9f657-c2e4-43e3-860d-412e60f14834.png)

## Download
The APK is in **[Releases](../../releases)**. Install it on your Android phone and play — no internet needed.

## What's new in this edition

### Android
* Runs on Android phones at a steady **60 FPS** (also on 90 / 120 Hz screens).
* **Full screen** with the right proportions — nothing is stretched.
* Works fully **offline**.

### Touch controls
* **Aim:** keep your finger on the screen and move it — the frog follows it.
* **Shoot:** lift your finger, or simply tap anywhere.
* **Swap balls:** tap the frog.
* Large, easy buttons; no accidental shots from the menu bar or the screen edges.

### Arabic language — اللغة العربية
* The whole game can be switched to Arabic from **Options**: menus, stones, buttons,
  windows, level names and messages.
* Arabic text in the game's own style: Kufi lettering carved into the stones, clear text in windows.
* Player names can be typed in Arabic (right to left, joined letters).
* The game starts in English; the language is remembered.

### Players (accounts)
* **ACCOUNTS** stone on the main menu: up to 6 players, each with their own progress.
* Create, choose and delete players.

### Levels
* Levels are **locked** like in the original game — win a level to open the next one.
* After a win: **NEXT** (go straight to the next level) or **MAIN MENU**.
* After a loss: **TRY AGAIN** or **LEAVE**.
* Four difficulty levels.

### Power-ups (like the original Zuma)
Special balls appear in the chain from time to time. Destroy them to get:
* **Accuracy** — a beam of light shows exactly where your ball will go, and balls fly faster.
* **Reverse** — the chain moves backwards for a short time.
* **Slow down** — the chain slows down for a few seconds.
* **Bomb** — the frog gets a bomb: shoot it anywhere to blow up all nearby balls.

### Fixes
* Balls no longer leave their track on phones (ARM processors).
* No freeze when tapping during the end-of-level explosions.
* Smooth start of levels, several old bugs fixed.

## Media
![Zuma Deluxe HD screenshot](https://github.com/GalaxyShad/Zuma-Deluxe-HD/assets/52833080/a84a71f4-385a-4d01-b4ce-b9ccf47d6f85)
![Zuma Deluxe HD gameplay](https://github.com/GalaxyShad/Zuma-Deluxe-HD/assets/52833080/a006a96f-805f-46d1-ac8f-340524df3cdd)

## Credits

**Project Owner & Creator:** Muhamed Alzarqi  
**AI Programming Assistant:** Claude (Anthropic)  
*Created through human-AI collaboration*

Based on the open-source **Zuma Deluxe HD** remake by [GalaxyShad](https://github.com/GalaxyShad) and [s4lat](https://github.com/s4lat)
([original project](https://github.com/GalaxyShad/Zuma-Deluxe-HD)).

* Level path format: article by [hclxing](https://hclxing.wordpress.com/2017/06/27/reverse-engineering-zuma-deluxes-level-file/).
* Music: [libopenmpt](https://lib.openmpt.org/libopenmpt/). Sound: miniaudio, libvorbis, [SDL2](https://github.com/libsdl-org/SDL/tree/SDL2).
* Arabic fonts: Reem Kufi and Lalezar (SIL Open Font License, see `tools/fonts`).

## Building
The APK is built automatically by GitHub Actions on every push (`.github/workflows/android.yml`).

To build the desktop version (Windows / macOS / Linux) you need a C compiler, CMake 4.0+ and Git:
```
git clone --recurse-submodules https://github.com/himohammed53-lab/zuma-deluxe-hd.git
cd zuma-deluxe-hd
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cd build/bin && ./ZumaHD
```

## License
Source code: MIT License. All game assets (the `content` folder) belong to
[PopCap and EA](https://www.ea.com/ea-studios/popcap). This project is not affiliated with them.
