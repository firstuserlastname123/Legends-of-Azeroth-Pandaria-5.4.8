# Fast Development Build Workflow

This workflow applies to preservation development for the Mists of Pandaria
5.4.8 client (build 18414). It separates the known-good validation profile
from a smaller, persistent build tree intended for edit/compile cycles.

## Verified CMake controls

The project declares these controls in `cmake/options.cmake`:

| Control | Default | Effect relevant to development |
| --- | --- | --- |
| `SERVERS` | `1` (ON) | Builds the server libraries, `worldserver`, and, subject to `AUTH_SERVER`, `authserver`. |
| `AUTH_SERVER` | `1` (ON) | Builds `authserver` when `SERVERS` is enabled. There is no separate `WORLDSERVER` option. |
| `SCRIPTS` | `1` (ON) | Includes the C++ game scripts. |
| `TOOLS` | `0` (OFF) | Builds the map/vmap/mmap extraction and assembly tools. |
| `ELUNA` | `1` (ON) | Includes the Eluna Lua engine. |
| `PLAYERBOTS` | `1` (ON) | Includes the Playerbots module. |
| `USE_SCRIPTPCH` | `1` (ON) | Uses precompiled headers for scripts. |
| `USE_COREPCH` | `1` (ON) | Uses precompiled headers for server/core targets. |
| `WITH_WARNINGS` | `0` (OFF) | Enables the compiler warning set. |
| `WITH_COREDEBUG` | `0` (OFF) | Includes additional core debug code (`TRINITY_DEBUG`). |
| `WITH_SANITIZER` | `0` (OFF) | Enables AddressSanitizer. |
| `USE_MODULES` | `1` (ON) | Builds the repository's module system. |
| `UPDATER` | `0` (OFF) | Builds the updater. |
| `BUILD_DEPLOY` | `1` (ON, Unix only) | Selects the Unix deployment-oriented compiler settings. |
| `BUILD_DEV` | `0` (OFF) | Selects an experimental Windows development build; it is not a Linux fast-build mode. |

The top-level `CMakeLists.txt` also recognizes the hidden `NOPCH` setting. If
truthy, it forces both PCH controls off. Prefer the two declared PCH controls
when changing PCH behavior explicitly. The default build type is `Release`;
normal CMake `-DCMAKE_BUILD_TYPE=Debug` configuration is available when a task
actually needs debug code or symbols. A Debug build type and
`WITH_COREDEBUG=1` are distinct choices.

There is no project-provided ccache detection or automatic launcher setup.
The Task 005 environment has ccache 4.9.1 at `/usr/bin/ccache`. Standard CMake
compiler launchers enable it without changing project files:

```sh
-DCMAKE_C_COMPILER_LAUNCHER=ccache \
-DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

## Mode A: validated baseline build

Tasks 002--004 validated GCC 13.3.0 and CMake 3.28.3 with this configuration
shape:

```sh
cmake -S . -B /tmp/mop-preservation-cmake-baseline \
  -DTOOLS=1 \
  -DELUNA=0 \
  -DCMAKE_INSTALL_PREFIX=/tmp/mop-preservation-install
cmake --build /tmp/mop-preservation-cmake-baseline --parallel 4
cmake --install /tmp/mop-preservation-cmake-baseline
```

This deliberately leaves `PLAYERBOTS` unspecified, preserving its repository
default of ON. It builds the servers, C++ scripts, module system and
Playerbots, plus the extraction tools; only Eluna is explicitly disabled.
This is the validated baseline, not the recommended everyday edit/build
profile. The baseline documentation in `CMAKE_BASELINE.md`,
`COMPILE_BASELINE.md`, and `INSTALL_BASELINE.md` remains the authoritative
record of the commands and observed results from those tasks.

A clean baseline build is very expensive in Codex Cloud. Reserve one for a
major core change, a build-system change, a dependency or toolchain change, a
major Playerbots change, release/milestone validation, or deliberate periodic
CI validation. Do not wrap a future full clean build in an artificial short
GNU `timeout` such as 20, 90, or 180 minutes. Let the Codex/platform execution
limit govern it.

## Mode B: fast development build

For ordinary core or C++ script work, configure once as follows:

```sh
cmake -S . -B /tmp/mop-preservation-dev \
  -DPLAYERBOTS=0 \
  -DUSE_MODULES=0 \
  -DTOOLS=0 \
  -DELUNA=0 \
  -DCMAKE_C_COMPILER_LAUNCHER=ccache \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -DCMAKE_INSTALL_PREFIX=/tmp/mop-preservation-dev-install
```

This keeps `SERVERS`, `AUTH_SERVER`, `SCRIPTS`, `USE_COREPCH`, and
`USE_SCRIPTPCH` at their ON defaults. It therefore retains the core,
`authserver`, `worldserver`, and C++ game scripts while excluding:

- Playerbots (`PLAYERBOTS=0`);
- all pluggable modules (`USE_MODULES=0`), including the example module and
  Playerbots;
- map/vmap/mmap extraction and assembly tools (`TOOLS=0`); and
- the Eluna engine (`ELUNA=0`).

Disabling the entire module system is intentional for this core/script
profile. In addition to reducing work, it avoids making the normal core build
depend on module sources. It means this profile cannot validate module or
Playerbots changes. For such work, use a separate persistent build tree with
`USE_MODULES=1` and the required module option; use `PLAYERBOTS=1` for
Playerbots work. Be aware that module configuration generates
`modules/ModulesLoader.cpp` in the source tree, so verify repository status
before and after configuring a module-enabled profile.

Keep PCH enabled for the initial build and broad day-to-day throughput. A task
dominated by repeated edits to headers included by a PCH may choose a separate
tree configured with `-DUSE_COREPCH=0 -DUSE_SCRIPTPCH=0`; this trades a slower
initial compile for avoiding repeated PCH regeneration. Do not routinely
reconfigure the same tree back and forth.

If ccache is absent in a future environment, do not install it merely for a
normal preservation task. Omit both launcher arguments and continue with the
persistent incremental build tree.

## Incremental development loop

After the one-time configuration, use:

```sh
cmake --build /tmp/mop-preservation-dev --parallel 4
```

CMake and Make reuse unchanged object files and rebuild only out-of-date
sources and their affected targets. ccache can additionally reuse matching
compilations retained in its cache. Keep `/tmp/mop-preservation-dev` for the
entire surviving Codex environment. Do **not** delete, clean, or recreate it
between edits unless a diagnosed technical reason requires that action.

Use these expectations when deciding what to rebuild:

- **One `.cpp` file:** build its owning target, or build the normal default
  target. Only that translation unit and necessary downstream link steps
  should normally run.
- **One header:** build the owning target or the default target. Every object
  whose dependency data includes that header may rebuild; a broadly included
  or PCH header can make this substantial.
- **A C++ script:** build `scripts` for a quick compile check, then build
  `worldserver` to validate the dependent final link. A normal default build
  is appropriate when scripts interact with shared core declarations.
- **A live Lua script:** this fast C++ profile has Eluna disabled and does not
  validate Lua runtime behavior. Use an explicit Eluna-enabled profile and
  the appropriate later runtime validation when the task scope permits it.
- **Playerbots:** this profile intentionally excludes it. Use a separate,
  persistent module-enabled tree with `-DPLAYERBOTS=1`; broader validation is
  appropriate because Playerbots is integrated with game and worldserver
  code.
- **A CMake file:** rerun the same `cmake -S . -B ...` configure command, then
  build. CMake may also regenerate automatically, but explicit reconfiguration
  makes option and dependency changes visible. Build broadly after structural
  target or flag changes.
- **A fresh Codex task:** `/tmp` from a prior environment may not exist.
  Configure a new development tree once, accept its necessary initial build,
  and preserve it for all subsequent edit/build cycles in that task.

A stopped or interrupted incremental build normally leaves completed object
files usable. Resume it with the same `cmake --build` command; do not clean it
merely because it was interrupted.

## Discovered targets and targeted builds

Task 005 configured `/tmp/mop-preservation-dev-discovery` without compiling it
and inspected `cmake --build ... --target help`. The fast profile exposes
these useful project targets:

- executables: `authserver`, `worldserver`;
- core libraries: `common`, `database`, `shared`, `game`;
- game scripts: `scripts`;
- supporting libraries: `Detour`, `Recast`, `fmt`, `g3dlib`, `gsoap`,
  `jemalloc`, and `sfmt`;
- maintenance/build targets: `all`, `clean`, `install`, `revision.h`,
  `rebuild_cache`, and `uninstall`.

`modules` is not present because the recommended profile sets
`USE_MODULES=0`. Extractor targets are not present because it sets `TOOLS=0`.
Do not invent or invoke those targets in this profile.

Examples of safe targeted checks are:

```sh
cmake --build /tmp/mop-preservation-dev --target authserver --parallel 4
cmake --build /tmp/mop-preservation-dev --target scripts --parallel 4
cmake --build /tmp/mop-preservation-dev --target game --parallel 4
cmake --build /tmp/mop-preservation-dev --target worldserver --parallel 4
```

Building an executable target also brings its declared prerequisites up to
date. `authserver` is sufficient for an authserver-local change.
`worldserver` is the preferred final targeted check for game or C++ script
changes because it validates the relevant dependency chain and final link.
Building only `game` or `scripts` is a useful fast compile check for that
library, but does not by itself relink or validate `worldserver`. Use the
default build when a change crosses target boundaries, touches shared build
configuration or widely used headers, or when broader pre-commit validation
is warranted.

## Strategy for future Codex preservation tasks

1. Start from the current `preservation/main` history.
2. Create one coherent Codex task/branch for one development issue.
3. Configure once and preserve the build directory throughout the task.
4. Make only the scoped source changes.
5. Use targeted and incremental compilation first.
6. Diagnose and fix compile errors caused by that task.
7. Repeat the edit/build loop without cleaning the tree.
8. Commit coherent working changes.
9. Perform broader validation when the risk and scope make it appropriate.
10. Create a pull request into `preservation/main`.

**NORMAL DEVELOPMENT TASKS SHOULD NOT PERFORM A CLEAN FULL BUILD BY DEFAULT.**
Full clean validation is a separate, deliberate action.
