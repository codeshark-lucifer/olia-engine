# Olia Engine Development & Platformer Game Report

This document details all the changes applied to the Olia Engine codebase, including compiler warning resolutions, rendering pipeline fixes, the implementation of custom UV mapping (sprite sheet support), the rewrite of the game application into a professional 2D platformer, the integration of audio effects/music, and the packaging of a standalone demo bundle.

---

## 1. Summary of Engine Changes & Fixes

### Custom UV & Sprite-Sheet Support (New Feature)
* **Goal**: Enable rendering specific frames from a sprite sheet or tiles from a tileset.
* **Changes**:
  * Updated `SpriteRenderer` struct in [built-in.h](file:///D:/Program%20Education/c++/olia-engine/engine/include/core/built-in.h#L24) to contain texture coordinates (`texCoords`) and a toggle flag (`useTexCoords`).
  * Overloaded `DrawQuad` in [batch.h](file:///D:/Program%20Education/c++/olia-engine/engine/include/renderer/batch.h#L34) and [batch.cpp](file:///D:/Program%20Education/c++/olia-engine/engine/src/core/batch.cpp#L194) to take an array of custom texture coordinates.
  * Overloaded `DrawQuad` in [renderer2D.h](file:///D:/Program%20Education/c++/olia-engine/engine/include/renderer/renderer2D.h#L17) and [renderer2D.cpp](file:///D:/Program%20Education/c++/olia-engine/engine/src/renderer/renderer2D.cpp#L66) to expose immediate quad rendering to the engine context.
  * Declared `Olia::RenderQuad` in [olia.h](file:///D:/Program%20Education/c++/olia-engine/engine/include/olia/olia.h#L47) and implemented in [main.cpp](file:///D:/Program%20Education/c++/olia-engine/engine/src/main.cpp#L286) to buffer immediate quad render commands, flushing them efficiently at the end of the scene render pass.

### Sprite Transparency Fix
* **Fix**: Updated the fragment shader's chroma-key branch in [main.cpp](file:///D:/Program%20Education/c++/olia-engine/engine/src/main.cpp#L168) to output `c.a` instead of forcing `1.0` alpha. This correctly respects PNG transparency while preserving black-background JPG keying.

### Immediate-mode Text Rendering Fix
* **Fix**: Implemented a deferred text-rendering queue `s_QueuedTexts` in [main.cpp](file:///D:/Program%20Education/c++/olia-engine/engine/src/main.cpp#L268). Text drawing calls are buffered and rendered at the end of `handle_render()`, preventing text from being covered by UI/background sprites.

### Bullet Physics Warning Fixes
* **Fixes**:
  * Changed bitfields in [btSoftBody.h](file:///D:/Program%20Education/c++/olia-engine/vendor/bullet3/src/BulletSoftBody/btSoftBody.h#L281) from `int` to `unsigned int` to fix Wsingle-bit-bitfield-constant-conversion warnings.
  * Cast `memset` target pointer in [btSoftBodyInternals.h](file:///D:/Program%20Education/c++/olia-engine/vendor/bullet3/src/BulletSoftBody/btSoftBodyInternals.h#L791) to `void*` to resolve Wnontrivial-memcall warnings.

---

## 2. 7-Level Platformer Game Rewrite

The game code in [main.cpp](file:///D:/Program%20Education/c++/olia-engine/app/src/main.cpp) was rewritten to implement a polished, single-screen platformer containing 7 distinct level variations using standard assets inside `assets/Free`.

### Core Physics & Logic
1. **Axis-Aligned Bounding Box (AABB) Collisions**: Applied strict AABB collision resolution layers between the Player, Ground, Boundaries, Hazards, and Collectibles.
2. **One-Way Pass-Through Rails**: Implemented one-way platform logic for dotted mechanical rails. The player passes through them from below but lands solidly on top of them when falling.
3. **Immediate Win Condition**: A level transition is triggered instantly when the total count of Collectibles in the active scene reaches 0.
4. **Immediate Lose Condition**: The current level resets instantly if the player overlaps with any Hazard bounding box or falls off-screen.
5. **Interactive HUD & UI Utility Buttons**:
   * Outlined HUD strip displaying the score, lives, and level name.
   * Functional top-right utility buttons sitting outside collision layers: `[Rewind]` (go to previous level), `[Skip]` (skip to next level), and `[Restart]` (reloads active level).

### Level Archetypes & Level Design
* **Level 1: "Sky Orchard"**: Stepped mud terrain rising to the right under three fat-birds. Player is Pink Man (Pink astronaut).
* **Level 2: "Rhino Run"**: Central cliff and metallic pass-through rails with charging Rhinos. Player is Virtual Guy (Blue astronaut).
* **Level 3: "Toucan Valley"**: Staggered Toucans flying through the center gap with grassy dirt cliffs on each side. Player is Mask Dude (Brown tribal character).
* **Level 4: "Snail Pace"**: Red brick overhang, slow snails, and an arch of watermelons. Player is Pink Man (Pink astronaut).
* **Level 5: "Peashooter Garden"**: Central earthen staircase leading up to pass-through ledges under Peashooters. Player is Mask Dude (Brown tribal character).
* **Level 6: "Barnyard Jump"**: Long floating central tier with chickens, apples, and bananas. Player is Virtual Guy (Blue astronaut).
* **Level 7: "Ninja Frog Woods"**: Golden corner blocks, a bottom-left hollowed-out tunnel, and pass-through step rails. Player is Ninja Frog (Green ninja frog).

---

## 3. Audio & Music Integration

We integrated full audio playback using the native Windows Multimedia API (`winmm`).

* **Build Integration**: Configured [CMakeLists.txt](file:///D:/Program%20Education/c++/olia-engine/app/CMakeLists.txt#L14) to link the binary against the `winmm` library.
* **Music**: The soundtrack [time_for_adventure.mp3](file:///D:/Program%20Education/c++/olia-engine/demo/assets/music/time_for_adventure.mp3) loops continuously in the background using Media Control Interface (`mciSendStringA`).
* **Sound Effects**: WAV sound effects are loaded and played asynchronously in the background using `PlaySoundA`:
  * **Jump**: [jump.wav](file:///D:/Program%20Education/c++/olia-engine/demo/assets/sounds/jump.wav) (triggered on jumps, double jumps, and wall jumps).
  * **Fruit Collection**: [coin.wav](file:///D:/Program%20Education/c++/olia-engine/demo/assets/sounds/coin.wav) (triggered on collecting fruits).
  * **Collision Damage / Pit Fall**: [hurt.wav](file:///D:/Program%20Education/c++/olia-engine/demo/assets/sounds/hurt.wav) (triggered when hitting hazards or falling off-screen).
  * **Level Transition**: [power_up.wav](file:///D:/Program%20Education/c++/olia-engine/demo/assets/sounds/power_up.wav) (triggered on clearing a level or clicking Skip).
  * **Victory Game Win**: [explosion.wav](file:///D:/Program%20Education/c++/olia-engine/demo/assets/sounds/explosion.wav) (triggered upon completing Level 7).
  * **UI Buttons**: [tap.wav](file:///D:/Program%20Education/c++/olia-engine/demo/assets/sounds/tap.wav) (triggered on clicking menu, skip, rewind, restart, or back buttons).

---

## 4. Standalone Demo Bundle

A self-contained [demo](file:///D:/Program%20Education/c++/olia-engine/demo) folder has been generated in the workspace root. It packages the game to run on any standard Windows machine out-of-the-box:

* **Executable**: [application.exe](file:///D:/Program%20Education/c++/olia-engine/demo/application.exe) (Compiled C++ Ninja Frog Platformer).
* **Assets**: Copy of the [assets](file:///D:/Program%20Education/c++/olia-engine/demo/assets) folder containing all levels, textures, characters, fonts, sounds, and music.
* **Dependencies**: Copy of the required MSYS2/MinGW/UCRT64 shared libraries:
  * `libwinpthread-1.dll`
  * `libgcc_s_seh-1.dll`
  * `libstdc++-6.dll`
  * `libbz2-1.dll`
  * `libbrotlidec.dll`
  * `libbrotlicommon.dll`
  * `libpng16-16.dll`
  * `zlib1.dll`

You can compress this entire `demo` directory into a `.zip` archive to share it.

---

## 5. Build & Verification

Compilation was successfully verified using:
```powershell
cmake --build build
```
* **Output**: Successfully compiled and linked [application.exe](file:///D:/Program%20Education/c++/olia-engine/build/app/application.exe).
* **Warning Count**: 0 warnings.
