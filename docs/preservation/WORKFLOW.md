# MoP Preservation Development Workflow

This repository preserves and develops the World of Warcraft: Mists of Pandaria 5.4.8 server for client build 18414.

## Branch and Merge Model

- **Upstream:** `master`
- **Integration:** `preservation/main`
- **Task development:** short-lived Codex/task branches
- **Merge path:** task branch -> pull request -> `preservation/main`

`master` remains the upstream mirror. Preservation work is integrated into `preservation/main`, while each fix or feature is kept isolated on a task branch and reviewed through a pull request.

## Validation Path

### Codex Cloud

Codex Cloud should perform as much reproducible development and testing as practical, including:

- repository analysis
- coding
- static checks
- CMake configuration
- compilation
- automated tests where available

### Persistent Linux WoW Server

The persistent Linux WoW server is the final runtime and gameplay validation environment. Its validation includes:

- database validation
- client connection
- maps, vmaps, and mmaps
- runtime stability
- quest testing
- dungeon testing
- spell testing
- packet and client compatibility
- Playerbots runtime testing
- long-duration testing

Cloud validation reduces reproducibility risk and catches development errors early, but it does not replace testing against the persistent server, its databases and extracted client data, and an actual 5.4.8 build 18414 client.

## Staged Task Philosophy

Large jobs must be divided into explicit stages. The initial sequence is:

1. **Task 001:** Preservation workflow and environment record.
2. **Task 002:** CMake baseline configuration only.
3. **Task 003:** Baseline compilation only.
4. **Task 004:** Baseline installation and binary smoke checks.

Source-code repair or other preservation work begins only after these baseline tasks are complete. Each task should remain bounded, preserve useful results in the repository, and stop at its defined checkpoint.
