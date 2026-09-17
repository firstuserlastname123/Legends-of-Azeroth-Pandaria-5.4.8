# Task 004: Installation and Binary Smoke Checks

## Installation and Smoke-Check Plan

- **Starting branch:** `work`
- **Starting commit:** `e2ad040790ae01e7d8f10bf9060b6e52b1dcfb7e`
- **Starting working tree:** Clean
- **Environment:** Ubuntu 24.04.4 LTS (x86_64), GCC/G++ 13.3.0, CMake 3.28.3, and OpenSSL 3.0.13, using the development packages recorded in `CLOUD_ENVIRONMENT.md`.
- **Source directory:** `/workspace/Legends-of-Azeroth-Pandaria-5.4.8`
- **Build directory:** `/tmp/mop-preservation-task004-build`
- **Install directory:** `/tmp/mop-preservation-task004-install`

The configuration command is:

```sh
timeout 10m cmake \
  -S . \
  -B /tmp/mop-preservation-task004-build \
  -DTOOLS=1 \
  -DELUNA=0 \
  -DCMAKE_INSTALL_PREFIX=/tmp/mop-preservation-task004-install
```

The intended build command is:

```sh
timeout 180m cmake --build \
  /tmp/mop-preservation-task004-build \
  --parallel 4
```

The intended install command is:

```sh
timeout 10m cmake --install /tmp/mop-preservation-task004-build
```

The intended smoke checks are:

```sh
timeout 30s /tmp/mop-preservation-task004-install/bin/authserver --version
timeout 30s /tmp/mop-preservation-task004-install/bin/worldserver --version
```

The installed tool binaries will be checked for existence without running extraction jobs. Task 004 does not include database configuration, normal or persistent server startup, runtime validation, or gameplay validation.

## Result

**TIMEOUT**

- **Configuration result:** SUCCESS (exit code `0`, approximately 4 seconds).
- **Configuration warning:** CMake reported `WARNING - No revision-information found - have you been tampering with the sources?`; it also initially reported the optional MySQL `binary` component missing before successfully finding the required MySQL client library and headers.
- **Build result:** TIMEOUT (exit code `124` from GNU `timeout`).
- **Build duration:** 10,800 seconds (3 hours), the full required timeout.
- **Furthest observed build progress:** 87%; the `scripts` target was compiling `Northrend/UtgardeKeep/UtgardePinnacle/boss_palehoof.cpp` when the timeout terminated the build.
- **First meaningful error:** None. The GNU Make `Terminated` messages were caused by the required timeout, not by a compiler, linker, dependency, or source-code error.
- **Install result:** NOT RUN. Task 004 requires stopping without a retry when compilation fails or times out.
- **Configured install path:** `/tmp/mop-preservation-task004-install`
- **Installed directory structure:** Not created because installation was not run.
- **Authserver path:** Not installed.
- **Worldserver path:** Not installed.
- **Authserver version-check result:** NOT RUN because installation was not reached.
- **Worldserver version-check result:** NOT RUN because installation was not reached.
- **Installed tools:** None; installation was not run.
- **Configuration templates installed:** No; installation was not run.
- **Dynamic-library/loading errors:** None observed because binary smoke checks were not run.
- **Unexpected database/runtime initialization:** None; binary smoke checks were not run.
- **Important warning:** The fresh build did not complete within the 180-minute bound. Per the task constraints, there was no compilation retry, installation attempt, binary execution, database configuration, persistent server startup, extraction job, or source repair.

## Baseline Status

Task 004 is blocked at its bounded compilation step. Configuration succeeded, but this run did not demonstrate complete compilation, installation, or basic binary/version execution. The earlier Task 003 compilation success remains recorded separately; its build artifacts were not reused because Task 004 required the dedicated build directory above.

## Continuation Result

**SUCCESS**

The previous attempt timed out at 87% after 180 minutes solely because of the artificial GNU timeout. It had no compiler, linker, dependency, source-code, or build-system failure. The partial build directory and its `CMakeCache.txt` survived, with the expected source directory, install prefix, `TOOLS=1`, and `ELUNA=0` configuration. The continuation therefore reused the existing build tree and resumed compilation without cleaning, recreating, or reconfiguring it.

- **Continuation build command:** `cmake --build /tmp/mop-preservation-task004-build --parallel 4`
- **Compilation mode:** Resumed from the surviving partial build; no GNU timeout and no keep-going option were used.
- **Final compilation result:** SUCCESS (exit code `0`).
- **Continuation duration:** 1,830 seconds (30 minutes, 30 seconds).
- **Final build progress:** 100%; the final target was `worldserver`.
- **Build warning:** The generated `revision.h` step repeated `WARNING - No revision-information found - have you been tampering with the sources?`.
- **First meaningful error:** None.
- **Install command:** `cmake --install /tmp/mop-preservation-task004-build`
- **Installation result:** SUCCESS (exit code `0`).
- **Install path:** `/tmp/mop-preservation-task004-install`
- **Installed authserver:** `/tmp/mop-preservation-task004-install/bin/authserver`
- **Installed worldserver:** `/tmp/mop-preservation-task004-install/bin/worldserver`
- **Installed tools:** `mapextractor`, `mmaps_generator`, `vmap4assembler`, and `vmap4extractor` under `/tmp/mop-preservation-task004-install/bin`; their existence and executable permissions were verified without running extraction jobs.
- **Installed configuration templates:** `/tmp/mop-preservation-task004-install/etc/authserver.conf.dist` and `/tmp/mop-preservation-task004-install/etc/worldserver.conf.dist`.
- **Authserver version check:** `timeout 30s /tmp/mop-preservation-task004-install/bin/authserver --version` exited normally with code `0` and printed `Pandria 5.4.8 rev. Archive  ( branch) (Unix, Release, Static)`.
- **Worldserver version check:** `timeout 30s /tmp/mop-preservation-task004-install/bin/worldserver --version` exited normally with code `0` and printed `Pandria 5.4.8 rev. Archive  ( branch) (Unix, Release, Static)`.
- **Dynamic-library/loading errors:** None. Both version checks exited immediately without a loader error.
- **Unexpected database/runtime initialization:** None observed. Neither version check attempted normal server startup or database initialization.

This successful continuation supersedes the timeout as the final Task 004 outcome.

## Final Baseline Status

The Codex Cloud baseline has now demonstrated:

- CMake configuration
- complete compilation
- installation
- basic binary/version execution

This does **not** prove:

- database correctness
- client connectivity
- gameplay correctness
- quest correctness
- spell correctness
- packet correctness
- Playerbots correctness
- long-term runtime stability

Those require later persistent-server validation. Task 004 performed no database configuration, persistent server startup, extraction job, runtime testing, or gameplay validation.
