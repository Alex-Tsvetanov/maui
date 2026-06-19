# CLAUDE.md

This repository hosts a **C++23 port of .NET MAUI**, under `port/`.

For any work on the port, always follow these (in order):

- [port/PROJECT.md](port/PROJECT.md) — **what** is being built (layers, milestones, success criteria).
- [port/cpp/PROFILE.md](port/cpp/PROFILE.md) — the **C++23 language profile**: tooling, idiom map, naming, ownership doctrine.
- [port/CLAUDE.md](port/CLAUDE.md) — the **operating manual**: the TDD per-component loop and fidelity rules.

**To resume work:** read [port/STATUS.md](port/STATUS.md) and continue to the next unfinished milestone.

The original C# `src/`, the `vault/` (API contract + conceptual docs), and `graphify-out/`
(dependency graph) are **read-only reference** — never modify them. Behavior is derived from the
C# source and its tests, never invented.

The port lives in `port/cpp/`. **Inner loop (fast — seconds):** `port/cpp/tools/dev.sh [regex]`
incrementally rebuilds the headless backend and runs the matching tests (`ctest -R`). **Full gate
(slow — all backends + sanitizers + clang-tidy):** `port/cpp/tools/gate.sh` (`--fast` for a quick
pre-commit subset, `--clean` for a from-scratch run). Both require `VCPKG_ROOT` set and use `ccache`
(`brew install ccache`) — see the "Build & test" section of `port/STATUS.md` for the raw commands and
the rationale. Do NOT run all presets for routine iteration; reserve the gate for pre-push verification.
