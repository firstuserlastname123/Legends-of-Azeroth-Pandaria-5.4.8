# Legends-of-Azeroth Pandaria 5.4.8 — Agent Instructions

## Build

```
mkdir build && cd build
cmake .. -D<OPTION>=<VAL>
make -j$(nproc)
```

Default build type is `Release`. CMake disallows in-source builds.

**Key CMake options** (`cmake/options.cmake`):
- `SCRIPTS=1` — build core with custom scripts (default ON)
- `TOOLS=0` — map/vmap/mmap extractors (default OFF; enable for map tooling)
- `ELUNA=1` — Lua engine (default ON)
- `PLAYERBOTS=1` — player bot AI (default ON; see README quirks below)
- `COREPCH/SCRIPTPCH=1` — precompiled headers (default ON; set to 0 for faster incremental rebuilds)
- `BUILD_DEPLOY=1` — Unix only (default ON)

Debug builds: `cmake .. -DCMAKE_BUILD_TYPE=Debug`

## Requirements

- **Compiler**: GCC ≥ 13 or Clang ≥ 12 (Windows: MSVC ≥ 2019 v16.4)
- **CMake** ≥ 3.16 (README says 3.27.2)
- **MySQL** 5.7 or 8.0-8.1 (note: OpenSSL 3.2.0 not supported with MySQL ≥ 8.0.33)
- **Boost** ≥ 1.85 (Linux), ≥ 1.81 MSVC
- **OpenSSL** 1.1.1 or 3.0–3.1.1
- **C++20** (enforced via `CMAKE_CXX_STANDARD`)

## Architecture

This is a **WoW private server** (Mists of Pandaria 5.4.8 client patch).

- `src/server/shared/` — shared library (packets, networking, data stores, threading)
- `src/server/authserver/` — authentication server
- `src/server/worldserver/` — game world server
- `src/server/game/` — core game logic (entities, spells, quests, combat, maps, AI, etc.)
- `src/server/scripts/` — custom C++ game scripts, organized by region/expansion (Pandaria, EasternKingdoms, Kalimdor, etc.)
- `src/server/database/` — database record definitions
- `modules/` — pluggable module system (loaded via `ModulesLoader`)
- `mod_playerbots/` — AI player bot module (early stage)
- `src/tools/` — map_extractor, vmap4_extractor/assembler, mmaps_generator
- `dep/` — vendored dependencies (Boost, fmt, StormLib, g3dlite, MySQL client, OpenSSL, RecastNavigation, etc.)
- `sql/base/` — base DB schema (`auth.sql`, `characters.sql`, `world.sql`)
- `sql/updates/` — incremental DB migration SQL, organized by date
- `contrib/lua_scripts/` — live Eluna Lua scripts (deployed alongside binaries)

## SQL / Database

- Base schema lives in `sql/base/` — apply these first
- Incremental updates in `sql/updates/` — ordered by filename prefix (date-based)
- Three databases: `auth`, `characters`, `world`
- Playerbots module uses its own DB: `mop_playerbots` (configured in `worldserver.conf`)

## Eluna Lua Engine

When `ELUNA=1`, the server loads `contrib/lua_scripts/` at runtime.
- **Quirk**: `mod_playerbots` is **HIGHLY recommended to disable** when Eluna is enabled (conflicts/instability).

## Player Bots Quirks

To enable bots beyond building:
1. Import playerbots database
2. Copy `playerbots.conf` into your build directory
3. Must use **enUS** DBC files
4. Add to `worldserver.conf`:
   ```
   PlayerbotsDatabaseInfo = "127.0.0.1;3306;root;root;mop_playerbots"
   PlayerbotsDatabase.WorkerThreads = 1
   PlayerbotsDatabase.SynchThreads = 1
   Logger.playerbots = 3,Console Server
   ```
5. First startup is slow — let it load and randomize

## Code Style

- Sun/Oracle C++ conventions
- 4 spaces (no tabs)
- LF line endings only (`core.autocrlf=true`)
- No trailing whitespace
- Squash PR commits; keep PRs tested (compile + functional)
- See `doc/code_standards.md` for full details

## CI

- Linux GCC: `.github/workflows/linux_gcc.yml` — GCC 13, Ubuntu 24.04
- Windows: `.github/workflows/windows-build-release.yml` — VS 2022, Boost 1.87, OpenSSL 3.1.1
- SQL changes are ignored in CI (`paths-ignore: sql/**`)
- Travis CI (`build/` dir, `cmake .. -DSCRIPTS=1 -DTOOLS=1`) is legacy

## MoP Preservation Project Rules

1. `master` is treated as the upstream mirror.
2. `preservation/main` is the long-lived integration branch for preservation development.
3. Individual fixes and features should be developed as isolated Codex/task branches and merged through pull requests.
4. Preserve compatibility with WoW Mists of Pandaria 5.4.8 client build 18414 unless a task explicitly states otherwise.
5. Prefer small, evidence-backed changes over broad rewrites.
6. Do not modify unrelated code while fixing a specific problem.
7. Never invent expected retail 5.4.8 behavior when evidence is unavailable. Record uncertainty instead.
8. Gameplay, quest, spell, packet, protocol, database, scripting, networking, Playerbots, and Eluna behavior must not be changed unless explicitly in the task scope.
9. Do not commit build directories, compiler output, extracted WoW client data, maps, vmaps, mmaps, DBC files, database dumps, passwords, credentials, or secrets.
10. Successful compilation proves only that code compiled. It does not prove correct gameplay behavior.
11. For C++ changes, perform the most relevant feasible build/test validation before declaring the task complete.
12. Do not silently fix additional problems discovered during a task. Document them for separate tasks.
13. If a task encounters a major unexpected blocker after useful changes have been made, preserve valid work in a checkpoint commit before stopping when possible.
14. Avoid long uncontrolled retry loops. A failing command should be diagnosed, not repeatedly retried with speculative changes.
