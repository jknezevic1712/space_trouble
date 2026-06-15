# Platform Fixes (macOS / Apple Silicon)

This project vendors several third-party libraries under `thirdparty/`. Some required edits to
build and run correctly on **macOS (Apple Silicon, arm64)**. They are documented here so they
survive future library updates and aren't mistaken for upstream behavior.

> If you ever re-vendor / update one of these libraries from upstream, re-check whether the
> corresponding fix below is still needed and re-apply it.

---

## 1. raudio — upgraded wholesale to fix silent audio (this session, 2026-06-15)

**Symptom:** No audio in-game on macOS, while everything reported success
(`IsAudioDeviceReady()==1`, sounds loaded, mixing produced a real signal). macOS `afplay` played
the same files fine.

**Root cause:** The vendored raudio bundled **miniaudio 0.10.14 (2020)**, whose CoreAudio backend
fails to initialize on macOS 26 / Apple Silicon and silently falls back to miniaudio's **Null
backend** (`device='NULL Playback Device'`) — audio is computed and discarded. Confirmed with a
standalone harness that printed `backend=Null`. A pure-miniaudio 0.11.x tone test selected
`Core Audio` and was audible, proving the upgrade fixes it.

**Fix:** Replaced the raudio module with matched current upstream sources (raysan5/raudio master):
- `thirdparty/raudio/src/raudio.c`                       — new (uses `frameCount`, not `sampleCount`)
- `thirdparty/raudio/include/raudio.h`                   — new public API
- `thirdparty/raudio/include/external/miniaudio.h`       — miniaudio 0.11.21 (Apple-Silicon-capable)
- `thirdparty/raudio/include/external/dr_wav.h`          — v0.13.13 (matched)
- `thirdparty/raudio/include/external/dr_flac.h`         — v0.12.42 (matched)
- `thirdparty/raudio/CMakeLists.txt`                     — added required config flags + Apple frameworks

**CMake flags added** (`thirdparty/raudio/CMakeLists.txt`):
- `SUPPORT_MODULE_RAUDIO` — required; gates the entire new raudio.c body (without it, it compiles to nothing)
- `RAUDIO_STANDALONE`     — build without raylib
- `SUPPORT_FILEFORMAT_WAV`, `SUPPORT_FILEFORMAT_FLAC` — only the decoders the game uses
- On `APPLE`: link `CoreAudio`, `AudioToolbox`, `AudioUnit`, `CoreFoundation`

**API impact on game code:** new raudio `Sound`/`Wave` use `frameCount` instead of `sampleCount`.
The game only calls the stable public API (`LoadSound`, `PlaySound`, `SetSoundVolume`,
`IsSoundPlaying`, `InitAudioDevice`), so no game code changed for the upgrade.

**Backup of the old raudio** is at `/tmp/raudio_backup/` (not committed; transient).

**Note:** the FLAC asset (`shoot_1.flac`) is 24-bit; raudio/dr_flac down-converts to 16-bit on load
(raudio doesn't support 24-bit natively). This is fine — not an error.

---

## 2. gl2d — already committed in `3ec8b34 "Third party lib fixes for MacOS"` (pre-existing)

These were already in git history before this session; listed for completeness.

- **`thirdparty/gl2d/src/gl2d.cpp` & `gl2dParticleSystem.cpp` — GLSL shader:**
  `texture2D(...)` → `texture(...)`. macOS only provides core-profile GL contexts (`#version 330`),
  where the legacy `texture2D` builtin was removed. Without this the shaders fail to compile and
  nothing renders ("Invalid call of undeclared identifier 'texture2D'").

- **`thirdparty/gl2d/src/gl2dParticleSystem.cpp` — SSE intrinsic guard:**
  `__m128 _deltaTime = _mm_set1_ps(...)` wrapped in `#if GL2D_SIMD != 0`. On Apple Silicon (arm64)
  there are no x86 SSE intrinsics; the declaration sat outside the SIMD guard and failed to compile.

- **`thirdparty/gl2d/src/gl2d.cpp` — `STBI_FREE` → `stbi_image_free` (this session, 2026-06-15):**
  Two calls used the internal `STBI_FREE` macro, which stb_image v2.30+ no longer exposes to
  header-only (declaration) users — only inside `STB_IMAGE_IMPLEMENTATION`. Switched to the public
  `stbi_image_free((void*)decodedImage)`. Required after the stb update below (see §4).

---

## 3. safeSave — already committed in `3ec8b34` (pre-existing)

- **`thirdparty/safeSave/include/safeSave.h` & `src/safeSave.cpp`:**
  The platform guards `#elif defined __linux__` were extended to
  `#elif defined __linux__ || defined __APPLE__`. macOS defines neither `_WIN32` nor `__linux__`,
  so `FileMapping` and its POSIX `mmap`/`open` implementation were excluded entirely
  ("unknown type name 'FileMapping'"). The Linux branch is pure POSIX and works as-is on macOS.

---

## 4. stb_image / stb_truetype — version updates (this session, 2026-06-15)

Updated the two public-domain single-header stb libraries to current upstream (nothings/stb master).
No local patches existed on either (they were pristine upstream), so these were clean drop-ins:
- `thirdparty/stb_image/include/stb_image/stb_image.h`        — **v2.26 → v2.30**
- `thirdparty/stb_truetype/include/stb_truetype/stb_truetype.h` — **v1.22 → v1.26**

**Knock-on fix:** v2.30 stopped leaking the internal `STBI_FREE` macro to header-only users, which
broke gl2d — fixed by switching to the public `stbi_image_free` (see §2).

Backups of the originals are at `/tmp/stb_backup/` (not committed; transient).

---

## Build configuration notes (not thirdparty edits, but macOS-relevant)

- **`CMakeLists.txt`** sets `CMAKE_POLICY_VERSION_MINIMUM 3.5` so the vendored libs (which declare
  `cmake_minimum_required` < 3.5) configure under CMake 4.x.
- **`PRODUCTION_BUILD`**: `ON` bakes a relative `RESOURCES_PATH="./resources/"` (run from project
  root only); `OFF` bakes an absolute source path so the game runs from any working directory —
  preferred during development.
