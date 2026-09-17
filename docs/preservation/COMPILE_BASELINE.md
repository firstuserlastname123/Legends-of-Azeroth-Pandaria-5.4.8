# Task 003: Baseline Compilation

## Compilation Plan

- **Starting branch:** `work`
- **Starting commit:** `437b239e6dc01584608d5fc215f9684e1386dc45`
- **Starting working tree:** Clean
- **Environment:** Ubuntu 24.04.4 LTS (x86_64), GCC/G++ 13.3.0, CMake 3.28.3, and OpenSSL 3.0.13. The installed development packages are `libboost-dev` 1.83.0.1ubuntu2, `default-libmysqlclient-dev` 1.1.0build1, `libreadline-dev` 8.2-4build1, and `libbz2-dev` 1.0.8-5.1build0.1.
- **Source directory:** `/workspace/Legends-of-Azeroth-Pandaria-5.4.8`
- **Build directory:** `/tmp/mop-preservation-cmake-baseline`
- **Install prefix:** `/tmp/mop-preservation-install`
- **Intended parallel job count:** 4, matching the current upstream Linux GCC workflow
- **Configuration timeout:** 10 minutes
- **Compilation hard timeout:** 20 minutes

The bounded configuration command is:

```sh
timeout 10m cmake \
  -S . \
  -B /tmp/mop-preservation-cmake-baseline \
  -DTOOLS=1 \
  -DELUNA=0 \
  -DCMAKE_INSTALL_PREFIX=/tmp/mop-preservation-install
```

The intended compilation command is:

```sh
timeout 20m cmake --build \
  /tmp/mop-preservation-cmake-baseline \
  --parallel 4
```

Generated build files will remain outside the repository. This task permits no source repair if configuration or compilation fails. Installation, binary smoke checks, and all runtime testing are reserved for Task 004.

## Result

**TIMEOUT**

- **Starting source commit:** `437b239e6dc01584608d5fc215f9684e1386dc45`
- **Repository commit at configuration and compilation:** `0142eab` (`chore(preservation): record Task 003 compile plan`)
- **Configuration command:** `timeout 10m cmake -S . -B /tmp/mop-preservation-cmake-baseline -DTOOLS=1 -DELUNA=0 -DCMAKE_INSTALL_PREFIX=/tmp/mop-preservation-install`
- **Configuration result:** SUCCESS (exit code `0`, approximately 4 seconds)
- **Build command:** `timeout 20m cmake --build /tmp/mop-preservation-cmake-baseline --parallel 4`
- **Parallel job count:** 4
- **Build timeout limit:** 20 minutes
- **Build exit code:** `124` from GNU `timeout`
- **Approximate build duration:** 1,200 seconds (20 minutes)
- **Compiler:** GCC/G++ 13.3.0 (`Ubuntu 13.3.0-6ubuntu2~24.04.1`)
- **CMake:** 3.28.3
- **OpenSSL:** 3.0.13
- **Dependency packages:** `libboost-dev` 1.83.0.1ubuntu2, `default-libmysqlclient-dev` 1.1.0build1, `libreadline-dev` 8.2-4build1, and `libbz2-dev` 1.0.8-5.1build0.1
- **Furthest observed progress:** 27%; compilation had reached the `game` target after completing the shared and database libraries, authserver, and the requested map tools.
- **Last meaningful target:** The `game` target was compiling `DungeonFinding/LFGMgr.cpp`; GNU Make also reported termination of the in-flight `DBCStores.cpp`, `M2Stores.cpp`, and `LFGGroupData.cpp` object builds when the timeout expired.
- **Warnings:** Configuration and the generated `revision.h` step warned that no revision information was found. Compiler warnings are disabled by the repository's default configuration, and no other warning was observed in the captured build output.
- **First meaningful error:** None. The termination messages were caused by the required hard timeout and are not classified as a source-code failure.
- **Produced binaries:** `authserver` was produced; `worldserver` was not produced. The `mapextractor`, `vmap4extractor`, `vmap4assembler`, and `mmaps_generator` tools were produced. No produced binary was run.
- **Overall build completed:** No; the single meaningful compilation attempt reached the hard wall-clock limit.

The timeout records incomplete baseline compilation only. It does not establish a source-code, linker, dependency, or build-system failure. No compilation retry, source repair, installation, or runtime check was performed.

## Continuation Result

**SUCCESS**

The bounded continuation retained the established configuration options and four-job parallelism while allowing up to 180 minutes for the single compile attempt.

- **Continuation source commit:** `a033e92` (`chore(preservation): record Task 003 baseline compilation`)
- **Configuration command:** `timeout 30m cmake -S . -B /tmp/mop-preservation-cmake-baseline -DTOOLS=1 -DELUNA=0 -DCMAKE_INSTALL_PREFIX=/tmp/mop-preservation-install`
- **Configuration result:** SUCCESS (exit code `0`, approximately 4 seconds)
- **Build command:** `timeout 180m cmake --build /tmp/mop-preservation-cmake-baseline --parallel 4`
- **Parallel job count:** 4
- **Build timeout limit:** 180 minutes
- **Build exit code:** `0`
- **Approximate build duration:** 9,233 seconds (2 hours, 33 minutes, 53 seconds)
- **Final build progress:** 100%; the final target was `worldserver`.
- **First meaningful error:** None. No compiler, linker, dependency, generated-code, or build-system failure was observed.
- **Warnings:** Configuration and the generated `revision.h` step repeated the warning that no revision information was found. Compiler warnings remained disabled by the repository's default configuration.
- **Produced binaries:** `authserver` and `worldserver` were produced. The `mapextractor`, `vmap4extractor`, `vmap4assembler`, and `mmaps_generator` tools were also produced.
- **Runtime testing:** None. No produced binary was run, and installation and binary smoke checks remain reserved for Task 004.

This successful continuation supersedes the earlier timeout classification as the final Task 003 compilation outcome. The earlier bounded-attempt record remains above as historical evidence; no compile retry was performed within either attempt.

## Task 004 Readiness

**READY.** The established baseline completed compilation. Task 004 may perform installation and binary smoke checks; neither was performed during Task 003.
