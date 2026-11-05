# Life Counter

**Install:** download `LifeCounter-<version>.zip` from [Releases](https://github.com/Jelly-Fish-Dev/Miyoo-Life-Counter/releases), unzip it onto the SD card root (it creates `App/LifeCounter/`), then open **Life Counter** from Onion's Apps list.

A Magic: The Gathering style life counter for the Miyoo Mini and Mini Plus. It runs as a native [OnionOS](https://github.com/OnionUI/Onion) app.

Put the device flat on the table between two players. The screen shows two square panels side by side, one life total in each. The left panel (P1) is drawn upside down so the player across the table can read it.

**Version:** 0.0.1 (see [VERSION](VERSION))

## Demo

[![Life Counter running on a Miyoo Mini Plus](Demo%20Video.gif)](Demo%20Video.mp4)

Click the animation for the full video with sound ([Demo Video.mp4](Demo%20Video.mp4)).

## Start-up

1. **Splash screen:** "Miyoo Mini Life Counter". Press any button, or wait 2.5 s.
2. **Pick colours:** each player picks one of 16 colours in their own square. P1's square is upside down, and their D-pad directions are reversed to match.

| Player | Move | Pick / change |
|---|---|---|
| P1 (left, upside down) | D-pad | SELECT |
| P2 (right) | X up, B down, Y left, A right | START |

When both players have picked, the counter starts.

## Counter controls

| Button | Action |
|---|---|
| **X** / **B** | P2 (right) +1 / −1 |
| **A** / **Y** | P2 (right) +5 / −5 |
| D-pad **DOWN** / **UP** | P1 (left) +1 / −1. Directions are from P1's side of the table |
| D-pad **LEFT** / **RIGHT** | P1 (left) +5 / −5 |
| **START** | Reset both players to the starting life |
| **SELECT** | Switch the starting life between 20 and 40 (Commander), then reset |
| **MENU** | Exit to Onion |

Hold a button to repeat it.

## Building

You need Docker and the Miyoo-UI library, which holds the drawing, font, button and logging code shared with RHSM. Clone it next to this repo (`../Miyoo-UI`), or point `MUI_DIR` at it in `local.mk`.

```sh
git clone https://github.com/Jelly-Fish-Dev/Miyoo-Life-Counter.git
git clone https://github.com/Jelly-Fish-Dev/Miyoo-UI.git
cd Miyoo-Life-Counter
```

The build uses the same toolchain image as RHSM (`rhsm-toolchain:latest`, built from [docker/Dockerfile](docker/Dockerfile) if it isn't there yet).

```sh
make            # cross-compile and stage dist/App/LifeCounter
make debug      # debug build (-O0 -g3, not stripped)
make zip        # dist/LifeCounter-<version>.zip, unzip onto the SD card root
make check      # include-layer check (model → ui → app)
```

## Deploying and debugging (Mini Plus over WiFi)

Put the device's IP in `local.mk`, which git ignores:

```make
LC_HOST ?= 192.168.1.50
LC_SSH_PASS ?= onion
```

```sh
make deploy               # copy to /mnt/SDCARD/App/LifeCounter over SSH
make log                  # show the app's log.txt
make debug-on DEBUG=1     # copy gdbserver over; the app then waits for gdb on :2345
make deploy DEBUG=1 && make gdb DEBUG=1
make debug-off
```

On the original Mini, copy `dist/App/LifeCounter` to `App/` on the SD card.

## Layout

```
src/model/      game.c: life totals (pure logic)
src/ui/         theme (panel layout), palette, splash / colour picker / counter screens
src/app/        main loop and screen order
```

Drawing (including 180° rotation), fonts, colour helpers, button mapping and logging come from Miyoo-UI (`#include "mui/gfx.h"` etc.).

## AI disclosure

I built this project with help from AI. I used Claude (Anthropic) for research, testing, debugging and setting up the build environment. I wrote the code and used AI to clean it up and test it. Everything has been tested on real hardware: a Miyoo Mini Plus running OnionOS.

Much of the groundwork came from my other project, [RHSM](https://github.com/Jelly-Fish-Dev/RHSM). That's where I built the UI code (screen drawing, fonts and button input), now shared by both apps as the Miyoo-UI library, and the build and deploy setup that this app reuses.

## Third-party components

- [Exo 2](https://github.com/googlefonts/Exo-2.0) font, bold instance, shipped by Miyoo-UI in `res/fonts/` (SIL Open Font License 1.1, see `OFL.txt` there)
- Built with OnionOS's [Miyoo Mini toolchain](https://hub.docker.com/r/aemiii91/miyoomini-toolchain) and its SDL 1.2 libraries

## License

[MIT](LICENSE) © 2026 Jelly-Fish-Dev. The Exo 2 font keeps its own license (SIL OFL 1.1).
