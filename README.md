# ORBIT Watch

**A pocket-sized playground for your wrist.**

Alternative, OS-like smartwatch firmware for the **original DSTIKE Deauther Watch X (Micro-USB, 4 MB)**. ORBIT brings a launcher, virtual pet, mini arcade, creative tools and wireless utilities to a tiny monochrome watch.

This is hobby firmware—not a full operating system, a commercial smartwatch replacement, or an official DSTIKE release.

## Why I built it

I wanted to turn my original Watch X into something more personal and fun: a
small, OS-like watch experience with a proper launcher, games, a virtual cat,
and tools I could show to friends. This project is also a way to learn what a
small microcontroller can do and share that work on GitHub.

The goal is not to match a Samsung or Xiaomi smartwatch. It is to make the most
of the hardware I already have, with original mini-apps and honest limitations.

## Highlights

- **Make it yours:** clock faces, a scrolling app launcher, saved favorites and settings.
- **Raise a virtual cat:** persistent growth, kitten/teen/adult stages, feeding, petting, cleaning, medicine and unlocked toys.
- **Play on your wrist:** Snake, Pong, Dino Runner, Maze Blaster, Space Invaders, Tetris, Geometry Jump, Lunar Lander and compact chess.
- **Create and save:** a journal, monochrome Pixel Studio, artwork previews and a slideshow.
- **Explore:** educational planet/moon facts and illustrative solar-system and ISS views.
- **Connect:** Wi-Fi surveying, passive packet-activity counting, ESP-NOW text and browser controls.
- **Take a break:** breathing, focus, stopwatch/timer utilities, calculator, flashlight and manual wellness tools.
- **Chat with rules:** a small ELIZA-inspired conversation app using the shared on-watch keyboard.

## Hardware

Target: **original DSTIKE Deauther Watch X**, Micro-USB revision, ESP8266, 4 MB flash.

| Hardware | Support status |
| --- | --- |
| Original Watch X, Micro-USB, ESP8266, 4 MB | Current firmware target; app-by-app testing still needed |
| Other Watch X revisions | Not validated |
| ESP32 watches or other DSTIKE models | Not supported by this build |

Uses the watch's built-in SH1106 monochrome display, three-way control and WS2812B status LED. No external components are needed. Other Watch X revisions and similarly named watches are **not validated**. This firmware is not an ESP32 build.

## Building and flashing

### Verified workflow: Schematik

1. Open this project in **Schematik**, keeping its configuration and source files together.
2. Connect the supported watch with a **data-capable Micro-USB cable**.
3. Press **Deploy** to compile and flash the firmware.
4. Test navigation, display orientation and individual apps on the actual watch.

Flashing replaces the installed firmware. Export important journal entries and
artwork first. Saved-data compatibility with other firmware or future changes
is not guaranteed. Do not disconnect power while flashing.

### Arduino IDE and PlatformIO

The source uses the **Arduino framework for ESP8266**, not ESP32. External
Arduino IDE and PlatformIO build/upload workflows have **not been validated for
this repository**. This README does not claim they are ready-to-use alternatives.
Their official documentation is available here:

- Arduino IDE: https://docs.arduino.cc/software/ide/
- ESP8266 Arduino core: https://arduino-esp8266.readthedocs.io/
- PlatformIO: https://docs.platformio.org/

For maintainers preparing an external build, the current project targets the
original Watch X's ESP8266 with **4 MB flash**, using these pinned dependencies:

| Dependency | Version |
| --- | --- |
| `olikraus/U8g2` | `2.36.18` |
| `adafruit/Adafruit NeoPixel` | `1.15.5` |

The checked-in `platformio.ini` is Schematik's project configuration, not a
validated standalone PlatformIO upload configuration. Do not assume the generic
ESP8266 board name alone establishes the correct flash layout or upload settings.

## Controls

| Control | Action |
| --- | --- |
| Up / Down | Browse menus or adjust the current selection |
| Short Select | Open an app, confirm or perform its action |
| Hold Select | Go back one level |

The main clock/home screen is the navigation root. App controls vary; read the on-screen hints. Favorites can be changed through **Settings → Edit favorites**.

### Arcade notes

- **Maze Blaster:** original maze shooter, not DOOM. Select moves/fires.
- **Space Invaders:** Up/Down moves horizontally; Select shoots.
- **Tetris:** Up/Down moves horizontally; Select rotates; pieces fall automatically.
- **Geometry Jump:** original one-button obstacle game, not Geometry Dash.
- **Lunar Lander:** Up/Down changes horizontal velocity; Select toggles thrust. Land slowly on the flat center pad.
- **Chess:** select a source square, then a destination. Legal destinations are marked. Selecting the source again cancels. Filled pieces are White; outlined pieces are Black.

Chess is a **mini variant** with king-safe moves, checkmate/stalemate and automatic queen promotion. It does **not** implement castling or en passant. Its computer opponent uses lightweight move scoring, not NNUE or a strong chess engine.

## Saved data and export

Cat progress and favorites persist. Local creative storage includes:

- Four journal entries, up to 200 characters each.
- Six monochrome 16×8 artwork slots.
- Saved Wi-Fi survey and packet-activity records; new recordings replace previous ones.

Saving over or deleting journal/art slots asks for confirmation. Flash storage has finite write endurance; saves are explicit.

To export journal entries and artwork:

1. Start **Web control** on the watch.
2. Join **ORBIT-Watch** using the password shown on its screen.
3. Open **http://192.168.4.1** in your browser.
4. Download journal text and artwork SVGs.
5. Stop Web control when finished.

Exports are downloads, **not an import/restore system**. Treat the watch network as private: connected users can read journal entries and control the watch. Do not store sensitive information in it.

## Wireless and weather

On-watch weather uses Open-Meteo and entered latitude/longitude. Use a **2.4 GHz WPA2-Personal** network. WPA3-only, enterprise, 5 GHz-only and captive-portal networks are not supported by this workflow. Hidden networks are not supported by the on-watch network picker.

Wi-Fi joining remains a known problem requiring physical testing. A timeout reason is displayed, but this does not mean joining is fixed. Credentials are held in RAM rather than saved permanently. The current on-watch weather HTTPS client does not verify the server certificate; do not adapt it for sending secrets.

The browser weather helper needs Internet access on the browser device. It is separate from successful Internet connectivity on the watch.

ESP-NOW messages are public broadcasts on channel 1, up to 31 bytes. A queued message is not proof of delivery. Radio apps can interrupt one another; starting another radio mode may stop Web control.

Wi-Fi surveying and packet activity are passive utilities. This project does not implement deauthentication, credential harvesting or disruptive wireless attacks. Use wireless features only where permitted.

## Status and limitations

**Experimental hobby project: implemented does not mean fully tested.**

- **RTC/timekeeping is unresolved.** Do not rely on retained time, accurate ticking or time-based reminders until verified on the actual watch.
- Display orientation, navigation, persistence, game behavior and wireless connectivity still need device testing.
- A successful compile or flash does not prove that every app works correctly.
- ELIZA chat is a compact original rule-based approximation—not the full historical DOCTOR script, a modern language model, a factual assistant or a therapist.
- Space views are educational illustrations, not live ISS tracking, accurate ephemerides, to-scale physics or a complete moon catalog.
- Wellness tools are manual/visual helpers; no health measurements or medical claims are provided.
- The buzzer is currently disabled.

The status LED is normally green, blue during Wi-Fi scanning, and white in flashlight mode.

## Source layout

| File | Purpose |
| --- | --- |
| `src/main.cpp` | Main firmware, interface and app routing |
| `src/arcade.h` | Arcade and space features |
| `src/cat.h` | Virtual cat |
| `src/chess.h` | Compact chess implementation |
| `src/connected.h` | Keyboard, weather and scripted chat |
| `src/creative.h` | Creative tools |
| `src/rtc.h` | Current RTC driver |
| `src/watchplus.h`, `src/extras.h` | Additional apps and wireless/browser utilities |
| `src/docs/README.md` | Development notes and update-specific details |
| `platformio.ini` | Framework and pinned libraries |
| `board.yaml`, `assembly.yaml` | Schematik hardware configuration and setup guidance |

Current external libraries: **U8g2** and **Adafruit NeoPixel**. Versions are pinned in `platformio.ini`.

## Contributing

Bug reports are welcome. Include your exact watch revision, steps to reproduce, expected/actual behavior and whether the issue happened after compiling, flashing or real on-device testing. Never include Wi-Fi passwords, private journal content or other secrets.

Useful priorities include dependable RTC operation, reliable Wi-Fi joining, device-tested navigation and more complete chess/ELIZA implementations.

## License and acknowledgments

No license has yet been selected for this project's original code. Public availability alone does not grant permission to reuse or redistribute it. Choose a license before presenting this as a licensed open-source release, and preserve third-party library notices.

Thanks to the U8g2 and Adafruit NeoPixel maintainers. Weather data is provided by **Open-Meteo**. Educational space references include NASA's Solar System and ISS resources. Product and game names belong to their respective owners; this project is not affiliated with them.

