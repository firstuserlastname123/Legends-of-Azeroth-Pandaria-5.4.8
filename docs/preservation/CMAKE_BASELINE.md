# Task 002: CMake Baseline Configuration

## Configuration Plan

- **Task:** Task 002
- **Starting branch:** `work`
- **Starting commit:** `f3ab9d05a0a7f0dc0a9e0593728ac1c41b80f75c`
- **Starting working tree:** Clean
- **Environment:** Ubuntu 24.04.4 LTS (x86_64), GCC/G++ 13.3.0, CMake 3.28.3, and OpenSSL 3.0.13, as recorded for the Codex Cloud environment and confirmed by quick version checks.
- **Source directory:** `/workspace/Legends-of-Azeroth-Pandaria-5.4.8`
- **Build directory:** `/tmp/mop-preservation-cmake-baseline`
- **Install prefix:** `/tmp/mop-preservation-install`

The current Linux GCC workflow configures with `TOOLS=1` and `ELUNA=0` and supplies an install prefix. It does not explicitly set `PLAYERBOTS`, so this baseline does not add a Playerbots flag and leaves the repository default unchanged.

The intended command is:

```sh
timeout 10m cmake \
  -S . \
  -B /tmp/mop-preservation-cmake-baseline \
  -DTOOLS=1 \
  -DELUNA=0 \
  -DCMAKE_INSTALL_PREFIX=/tmp/mop-preservation-install
```

This task performs CMake configuration only. Compilation is explicitly reserved for Task 003; Task 002 will not run `make`, `ninja`, or `cmake --build`.

## Result

**SUCCESS**

- **Command executed:** `timeout 10m cmake -S . -B /tmp/mop-preservation-cmake-baseline -DTOOLS=1 -DELUNA=0 -DCMAKE_INSTALL_PREFIX=/tmp/mop-preservation-install`
- **Exit code:** `0`
- **Approximate configure duration:** 4 seconds (CMake reported 4.2 seconds configuring and 0.4 seconds generating)
- **Starting source commit:** `f3ab9d05a0a7f0dc0a9e0593728ac1c41b80f75c`
- **Repository commit at configuration:** `ed5975bbcc0129fbb2a09aae49bc54173ab3c75b` (the required documentation-only plan checkpoint)
- **Compiler:** GCC 13.3.0 and G++ 13.3.0; CMake identified both C and C++ compilers as GNU 13.3.0
- **CMake:** 3.28.3
- **OpenSSL:** 3.0.13; CMake found Crypto and SSL, with the required minimum reported as 1.1
- **Boost:** Debian package `libboost-dev` 1.83.0.1ubuntu2; CMake found Boost 1.83.0 with filesystem, program_options, iostreams, regex, and locale
- **MySQL client:** Debian package `default-libmysqlclient-dev` 1.1.0build1; CMake used `/usr/bin/mysql_config` and found `/usr/lib/x86_64-linux-gnu/libmysqlclient.so` and headers under `/usr/include/mysql`
- **Other queried dependencies:** `libreadline-dev` 8.2-4build1 and `libbz2-dev` 1.0.8-5.1build0.1; CMake found Readline 8.2 and BZip2 1.0.8
- **Configured options:** tools enabled, Eluna disabled, and Playerbots left at its repository default of enabled, matching the workflow's omission of an explicit Playerbots option
- **Generated build system:** Yes; CMake generated Unix Makefiles in `/tmp/mop-preservation-cmake-baseline`

The first meaningful warning was:

```text
WARNING - No revision-information found - have you been tampering with the sources?
```

CMake initially reported that the optional MySQL `binary` component was missing, then successfully found the required MySQL client library and headers. No configuration errors occurred.

One retry occurred. The first wrapper invocation failed with exit code 127 before CMake started because `/usr/bin/time` is unavailable in this environment. This was a trivial task-introduced timing-command mistake; the single meaningful CMake configure attempt then used the exact planned `timeout` command above and succeeded. No dependency, CMake, compiler, or repository failure was retried.

## Task 003 Readiness

**READY.** The repository is ready for Task 003 baseline compilation using the generated configuration model. No compilation was performed as part of Task 002.
