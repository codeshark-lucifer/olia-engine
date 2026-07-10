# Olia Engine Development & Platformer Game Report

This document details all the changes applied to the Olia Engine codebase and the platformer application, including compilation and warning resolutions, rendering layering fixes, custom UV mapping (sprite sheet support), the integration of the Bullet3 Physics Engine, looping background music and audio effects, and static demo packaging.

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

### Bullet Physics Warning Fixes (Engine Vendor)
* **Fixes**:
  * Changed bitfields in [btSoftBody.h](file:///D:/Program%20Education/c++/olia-engine/vendor/bullet3/src/BulletSoftBody/btSoftBody.h#L281) from `int` to `unsigned int` to fix bitfield-constant-conversion warnings.
  * Cast `memset` target pointer in [btSoftBodyInternals.h](file:///D:/Program%20Education/c++/olia-engine/vendor/bullet3/src/BulletSoftBody/btSoftBodyInternals.h#L791) to `void*` to resolve nontrivially copied memory warnings.

---

## 2. Refactored Codebase Structure

The game has been modularized by separating declarations and helper inline routines from the main game loop:
1. **[core.h](file:///D:/Program%20Education/c++/olia-engine/app/include/core.h)**: Defines the basic `Animation` structure, links the game framework includes, and declares global `frametime`, `setup()`, and `loop()`.
2. **[game.h](file:///D:/Program%20Education/c++/olia-engine/app/include/game/game.h)**: Houses level layouts (`Platform`), the `Player` state structure, and inline utility routines for animations:
   * `LoadAnimation` for reading and storing sprite frames.
   * `PlayAnimation` for changing state tracks.
   * `UpdateAnimation` for shifting texture coordinates (UVs) inside the ECS `SpriteRenderer` component (including sub-pixel edge bleeding prevention and horizontal flips).
3. **[core.cpp](file:///D:/Program%20Education/c++/olia-engine/app/src/core/core.cpp)**: Initializes the Olia Engine context and runs the main loop forwarding updates to `loop()`.
4. **[game.cpp](file:///D:/Program%20Education/c++/olia-engine/app/src/game.cpp)**: Implements all physics simulation, input handling, and active rendering logic.

---

## 3. Bullet3 Physics Engine Integration

Instead of manually resolving platformer bounding-box overlaps, we integrated the linked **Bullet3 Physics Engine** directly into the game application in [game.cpp](file:///D:/Program%20Education/c++/olia-engine/app/src/game.cpp):

* **Physics World**: Initialized a `btDiscreteDynamicsWorld` with gravity set to pull in the positive Y direction ($1400.0\text{ px/s}^2$) to align with the engine's downwards-increasing screen space coordinates.
* **Static Colliders**: Created static box shapes (`btBoxShape` using half-extents) for all level platforms and added them to the physics world.
* **Dynamic Player Rigid Body**:
  * Bound the player to a dynamic `btRigidBody` with mass `1.0f` and friction `0.1f`.
  * Constrained motion strictly to the 2D plane by setting `linearFactor` to `(1.0, 1.0, 0.0)` (locking Z-depth motion) and locked rotation by setting `angularFactor` to `(0.0, 0.0, 0.0)`.
* **Grounded 3-Ray Raycasting**:
  * Implemented an edge-safe ground check by casting three vertical ray tests (left edge, center, right edge of the player's collider shape) down to the physics world using `dynamicsWorld->rayTest`.
  * This allows the player to correctly jump and reset their double-jump charge even when standing on the extreme edges of platforms.
* **Input-Driven Velocities**:
  * Controls like WASD set horizontal velocities directly on the rigid body, utilizing a blend factor for smooth acceleration and friction.
  * Jumps and double-jumps apply instantaneous upward vertical impulses to the linear velocity.
* **ECS Sync & Screen Resets**:
  * Copied rigid body positions and velocities back to the ECS `Transform` component and the `player.velocity` trackers.
  * Screen boundary clamp checks are performed on the rigid body transform, resetting its forces, velocity, and origin if the player falls off the viewport.

---

## 4. Layer Ordering & Rendering Fix

Because the engine renders ECS `SpriteRenderer` components *before* immediate-mode `RenderQuad` calls, a background quad rendered in immediate-mode would cover the player.
* **Fix**: Moved background and platform rendering to the very beginning of the `loop()` function, and added an explicit player sprite rendering call (`Olia::RenderQuad`) at the end of the `loop()` function. This guarantees the correct layering sequence: **Background** (bottom) $\to$ **Platforms** $\to$ **Player** (top).

---

## 5. Audio & Music Integration

We integrated full audio playback using the native Windows Multimedia API (`winmm`) linked to the game:
* **Music**: The soundtrack [time_for_adventure.mp3](file:///D:/Program%20Education/c++/olia-engine/packaged_demo/assets/music/time_for_adventure.mp3) is opened and played on a continuous loop asynchronously using the Windows Media Control Interface (`mciSendStringA` command with `repeat`).
* **Sound Effects**: WAV sound effects are loaded and played asynchronously in the background using `PlaySoundA` so that audio operations do not block the main game thread:
  * **Jump & Double Jump**: [jump.wav](file:///D:/Program%20Education/c++/olia-engine/packaged_demo/assets/sounds/jump.wav) (triggered on executing jumps).

---

## 6. Standalone Portable Demo Package

Previously, compiling the application required copying dynamic libraries alongside the executable.
* **Static Linking**: Modified [CMakeLists.txt](file:///D:/Program%20Education/c++/olia-engine/app/CMakeLists.txt#L21) to link the C++ runtime libraries statically:
  ```cmake
  target_link_options(application PRIVATE -static -static-libgcc -static-libstdc++)
  ```
  This creates a completely standalone executable `application.exe` that runs on any other Windows device without requiring MSYS2/MinGW DLLs.
* **Tiled Asset Packaging**: Configured installation rules in CMake. Running `cmake --install build --prefix packaged_demo` creates a self-contained [packaged_demo](file:///D:/Program%20Education/c++/olia-engine/packaged_demo) folder containing:
  * `application.exe` (independent 37.5 MB executable).
  * `assets/` (full directory of loop music, sounds, and textures).

Zipping this directory provides a completely portable demo for other devices.

---

## 7. Build & Verification

Compilation and packaging were successfully verified using:
```powershell
cmake --build build
cmake --install build --prefix packaged_demo
```
* **Output**: Generated standalone executable and asset directory inside [packaged_demo](file:///D:/Program%20Education/c++/olia-engine/packaged_demo).
* **Warning / Error Count**: 0.
