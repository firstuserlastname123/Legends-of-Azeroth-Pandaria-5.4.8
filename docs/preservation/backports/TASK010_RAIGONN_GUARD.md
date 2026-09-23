# Task 010: Isolated Raigonn State Guard

## Review checkpoint and backport plan

- **Starting commit:** `59287a53114db16ba0e96ed79cdbd7fd2aa4e449`
  (`Merge pull request #9 from
  firstuserlastname123/codex/backport-arc4-openssl-3-handling`). The working
  tree was clean on the task branch `work`. Both Task 009's implementation
  commit and merge commit are ancestors, so this task starts from the current
  supplied `preservation/main` integration history after Task 009; `master`
  was not checked out.
- **Source repository and branch:**
  `https://github.com/ingussuveiks-dev/Legends-of-Azeroth-Pandaria-5.4.8.git`,
  `master`.
- **Exact source candidate:**
  `00d1cd9976a874c65daf901e1a2740d4393e7be5`, `Fix Gate cannon feedback and
  stray headbutt stun`, authored and committed by ingussuveiks-dev
  `<ingussuveiks@gmail.com>` on 2026-09-17 17:05:53 +0300.
- **Scoped source file:**
  `src/server/scripts/Pandaria/GateOfTheSettingSun/boss_raigonn.cpp`. The
  candidate also changes `boss_striker_gadok.cpp`; that cannon hunk is outside
  Task 010 and will not be imported.
- **Exact scoped candidate hunk:** change
  `BatteringHeadbuttEffectTargetSelector` from an impossible conjunction to an
  outside-rectangle removal predicate, and return from the headbutt script
  unless an instance exists and `DATA_RAIGONN` is `IN_PROGRESS`.

## Static semantic analysis

### Predicate and collection behavior

The spell-effect callback obtains the caster, populates a
`std::list<Player*>` named `PlayersOnGates` with players within 100 units, calls
`std::list::remove_if`, and applies aura 130772 to every retained, non-falling
player.

- **Current predicate (exact):**

  ```cpp
  return object
      && object->GetPositionY() > 2372.0f && object->GetPositionY() < 2360.0f
      && object->GetPositionX() > 997.0f  && object->GetPositionX() < 919.0f;
  ```

- **Candidate predicate (exact):**

  ```cpp
  return !object
      || object->GetPositionY() < 2360.0f || object->GetPositionY() > 2372.0f
      || object->GetPositionX() < 919.0f  || object->GetPositionX() > 997.0f;
  ```

- **FACT:** `remove_if` erases elements for which its predicate returns true.
  For every non-null player, the current predicate requires Y to be both
  greater than 2372 and less than 2360, and X to be both greater than 997 and
  less than 919. Those strict inequalities cannot simultaneously hold. A null
  element also returns false because of the leading `object &&`. Therefore the
  current predicate removes no elements.
- **FACT:** the candidate predicate removes null elements and players outside
  the inclusive rectangle X `[919, 997]`, Y `[2360, 2372]`. It retains players
  on or inside all four bounds, and only those retained non-falling players
  receive the stun.
- **FACT:** the current predicate is logically impossible, not merely a
  different rectangle choice. The candidate is the Boolean complement of
  membership outside those existing bounds, expressed with ordered minima and
  maxima, and is internally consistent with `remove_if` and the later aura
  loop.
- **INFERENCE:** names `PlayersOnGates` and
  `BatteringHeadbuttEffectTargetSelector`, plus the candidate comment, support
  retaining players in a bounded impact area rather than every nearby player.
  Static semantics establish the existing filter defect without relying on
  that gameplay interpretation.
- **UNKNOWN:** no authoritative build-18414 capture or coordinate source was
  found. Static review does not prove that this rectangle is retail-accurate,
  that its edges should be inclusive, or that a two-dimensional filter fully
  describes the intended encounter volume.

### Encounter-state and lifecycle behavior

- **FACT:** current `HandleSpellEffectHit` has no encounter-state check. The
  candidate obtains `caster->GetInstanceScript()` and returns when it is null
  or when `GetBossState(DATA_RAIGONN) != IN_PROGRESS`.
- **FACT:** `EncounterState` defines `NOT_STARTED`, `IN_PROGRESS`, `FAIL`,
  `DONE`, `SPECIAL`, and `TO_BE_DECIDED`. Raigonn reset records
  `NOT_STARTED`; first accepted damage after Rimok is complete records
  `IN_PROGRESS`; evade records `FAIL`; and death records `DONE`.
- **FACT:** Battering Headbutt is cast from the Raigonn charge cycle and its
  script callback is the only caller of the affected filter. The candidate
  guard consequently allows selection/stun only during the state already used
  by the encounter to denote active combat.
- **INFERENCE:** guarding a hostile encounter aura with the encounter's
  `IN_PROGRESS` state is internally consistent and prevents a delayed or stray
  spell effect from selecting players after reset, failure, or completion.
- **UNKNOWN:** static inspection does not reproduce such a delayed cast and
  does not prove the actual runtime ordering of spell callbacks, creature
  removal, evade, and scheduler cancellation.

### Boundary and dependency findings

- **FACT:** all four proposed rectangle coordinates are already present in
  `preservation/main`, in the current impossible predicate. The candidate does
  not introduce new coordinate values or SQL/data requirements.
- **FACT:** the candidate's parent has the same relevant predicate, callback,
  spell IDs, `DATA_RAIGONN` identifier, and encounter-state API as the current
  tree. Candidate-parent blame traces the relevant block to the fork's imported
  baseline rather than to the intervening Gate series.
- **FACT:** earlier candidate commit
  `566d274cc60f9af2b08eaa5610b9911a44fe76d1` changes two unrelated Raigonn
  lines (`SetGuidData` API use and the swarm-bringer reschedule event), not this
  predicate or callback. The intervening Gate commits do not modify
  `boss_raigonn.cpp`.
- **FACT:** the broader candidate's Gadok cannon change depends on prior Gate
  commits `3eb3c38bdbfec4281918b622573f242bdb40cea1` and
  `67a6a17dcabc25d5a1cf29e59a89eced2f04b9e1`; it remains excluded.
- **INFERENCE:** the isolated Raigonn hunk has no prerequisite Gate behavior:
  it uses APIs and state transitions already present in `preservation/main`
  and can be applied independently of the cannon series.
- **UNKNOWN:** the complete fork Gate series has not been approved, and no
  conclusion about its other Raigonn, cannon, elevator, or trash changes is
  made here.

## Decision and intended validation

**Decision: statically justified for an isolated backport.** The current
predicate is provably unsatisfiable for a non-null player's coordinates, the
candidate restores a consistent removal predicate using values already in the
current code, and the adjacent state guard matches the current encounter-state
transitions without a prerequisite. Only the two Raigonn hunks will be applied
exactly; no Gadok/cannon, SQL, Playerbots, Eluna, or unrelated encounter change
is in scope.

After applying the isolated hunk, review the source-only diff, reuse the
compatible `/tmp/mop-preservation-dev` tree if available, build the `scripts`
target, and link `worldserver`. Then re-read the compiled source and compare
repository patterns for `remove_if` selectors and `IN_PROGRESS` guards.

Compilation can establish syntax/API/link compatibility only. Actual rectangle
placement, vertical-floor behavior, stun timing, evade/reset behavior, and
full encounter correctness remain for later dungeon testing with a build-18414
client on the persistent test server.

## Final implementation and validation

- **Checkpoint commit:** `ad445b5447df24eedd7442e83b3383d067855845`
  (`chore(preservation): record Raigonn guard review plan`). The tree was clean
  immediately after this plan commit.
- **Application:** the two `boss_raigonn.cpp` hunks are byte-for-byte the scoped
  candidate diff: the corrected removal predicate (including its explanatory
  comment) and the adjacent instance/`IN_PROGRESS` guard. No adaptation was
  required. The candidate's `boss_striker_gadok.cpp` cannon hunk and every other
  Gate change remain excluded.
- **Configure:** `/tmp/mop-preservation-dev/CMakeCache.txt` was absent. The
  documented fast profile configured successfully with GCC 13.3.0, Release
  mode, ccache launchers, and Playerbots, modules, tools, and Eluna disabled.
  The generated build tree is outside the repository.
- **`scripts` build:** succeeded with
  `cmake --build /tmp/mop-preservation-dev --target scripts --parallel 4`.
  This initial development-tree build compiled the changed Gate translation
  unit and linked `libscripts.a`.
- **`worldserver` build:** succeeded with
  `cmake --build /tmp/mop-preservation-dev --target worldserver --parallel 4`,
  including the final executable link.
- **Post-build static re-read:** `PlayersOnGates` remains a
  `std::list<Player*>` populated within 100 units. Its sole affected
  `remove_if` now removes null/outside-rectangle entries; the loop still skips
  falling players and adds only aura 130772 to retained entries. The callback
  now returns before collection or aura processing unless the caster has an
  instance whose Raigonn boss state is `IN_PROGRESS`. No other spell,
  encounter, lifecycle, or caller changed.
- **Repository comparison:** other script selectors use `remove_if` with the
  same convention that `true` means exclusion. Other encounter scripts guard
  effects or AI work by returning/skipping when `GetBossState(...) !=
  IN_PROGRESS`. These comparisons support the candidate's container and state
  semantics but do not independently validate the Raigonn rectangle.
- **Dependencies:** no prerequisite was exposed by compilation or the final
  static review. The scoped code relies only on existing Gate identifiers,
  encounter transitions, coordinates, spell registration, and core APIs. No
  SQL, data migration, Playerbots, Eluna, Gadok/cannon, or other Gate change
  was needed.
- **Remaining uncertainty:** compilation does not prove gameplay correctness.
  Build-18414 dungeon testing must verify the rectangle and boundary edges,
  vertically stacked players, falling-player exclusion, headbutt timing,
  delayed/stray callback behavior across reset and evade, post-kill behavior,
  and the complete Raigonn encounter on the persistent test server.
