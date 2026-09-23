# Task 032: Battle-Pet Aura Expiration Timing

## Starting State

- **FACT:** The checkout was clean but initially at Task 030's integration merge
  `2bdda51c00d6ad16e31735fa4aead7d28d5fbbcf`. A fetch by URL (without adding a
  remote) found current `preservation/main` at
  `32a8565336a9533509998b973cd16f6e57d82a5f`; the task branch was fast-forwarded
  there before review. That merge contains Task 031's documentation commit
  `b2571eb`.
- **FACT:** Task 029's modifier-only reversal remains at
  `BattlePetAura.cpp:38`. Tasks 030 and 031 add documentation only. Task 025's
  `EndBattle` exactly-once guard still returns for `Finished` and sets
  `Finished` before cleanup.
- **FACT:** `/tmp/mop-preservation-dev/CMakeCache.txt` was absent initially and
  ccache reported 0.0 GiB used of 5.0 GiB. No tree was configured because this
  review does not authorize source modification.
- The scope is finite aura expiry placement only. Removal-proc identity,
  boolean ownership, stacking, delayed auras, weather, handlers, SQL,
  Playerbots, and protocol layout are unchanged.

## Andrew Provenance

- **FACT:** Andrew commit
  `867b69b0d142efaaea10b83970342900d3405f2e`, whose parent is
  `1e138b1f1d7e1660eb28b647b7193afe961acb72`, replaces the leading expiry block
  with a captured boolean:

  ```cpp
  bool shouldExpire =
      (m_duration != -1 && m_turn > m_maxDuration && !m_expired);
  ```

  It then runs the unchanged effect scan and inserts `if (shouldExpire) {
  Expire(); return; }` immediately before `AURA_CHANGE` construction.
- **FACT:** The comparison remains `m_turn > m_maxDuration`; the finite and
  already-expired guards remain unchanged. The normal `m_turn++` followed by
  finite `m_duration--` remains after `AURA_CHANGE`. Therefore the moved block
  neither mutates the clocks nor reaches those mutations on the expiry pass.
- **FACT:** The same commit separately changes removal-proc `Cast` from
  `m_target` to `m_caster`. That mechanically separate hunk is not imported.
- **FACT:** Path-limited history after `867b69b0` through
  `bd8ad5515418d94abd35ba7bf71430979243a09c` contains no later
  `BattlePetAura.cpp` change. Andrew's final requested state retains this
  timing.

## Current Process Order

**FACT:** Current `Process()` performs, in order:

1. if finite, not expired, and `m_turn > m_maxDuration`, call `Expire()` and
   return;
2. look up the aura ability;
3. count its `BattlePetAbilityTurn` rows and find their greatest `Duration`;
4. rescan matching turn rows and execute joined effect rows selected by the
   turn condition;
5. enqueue `PET_BATTLE_EFFECT_AURA_CHANGE` containing the entry values of
   `m_duration` and `m_turn`;
6. increment `m_turn`;
7. decrement `m_duration` only when it is not `-1`.

Prowl and next-attack expiry are not inside `Process`; `ProcessAuras()` checks
those after `Process()` returns. The constructor initializes `m_turn=1`, both
`m_duration` and `m_maxDuration` to the supplied duration, and
`m_expired=false`.

## Andrew Process Order

**FACT:** Andrew computes `shouldExpire` first, performs steps 2--4 above, then
expires and returns when that entry snapshot was true. Otherwise it performs
steps 5--7. Thus the only additional work on a natural-expiry call is the
ability/turn/effect scan and any effects it executes. It does not send an extra
`AURA_CHANGE` or advance either clock on that call.

## Aura Clock Semantics

- **FACT:** `m_turn` is a one-based count of completed/scheduled aura processing
  passes: it starts at 1, selects exact-duration turn rows in the general case,
  is sent in `AURA_CHANGE`, then increments.
- **FACT:** `m_duration` starts at the requested duration and is the
  client-reported remaining-duration counter. It is sent before decrement, so
  finite successful passes report `D, D-1, ..., 1`; after pass D it is zero.
- **FACT:** `m_maxDuration` is an immutable copy of the normalized creation
  duration. Natural expiry compares the turn clock to this original value; it
  does not compare remaining duration.
- **INFERENCE:** For positive D, these clocks are algebraically redundant during
  uninterrupted processing (`m_duration = D-m_turn+1` on entry), but separate
  fields support wire reporting and the `-1` sentinel. Source does not document
  a retail contract for the D+1 boundary.

## D=1 / D=2 / D=3 Simulation

`Rows` below means rows/effects eligible under the selection rule: normally
rows whose `Duration` equals entry `m_turn`; with one turn row or greatest
Duration 1, all matching rows execute on every processing pass. `Change(x,y)`
contains entry `(duration,turn)`.

### Current preservation

| D | Call | Entry `(turn,duration)` | Expiry test | Rows | Client event | Exit `(turn,duration)` | Expire? |
|---:|---:|---:|---|---|---|---:|---|
| 1 | 1 | (1,1) | false | turn 1 / repeating rows | Change(1,1) | (2,0) | no |
| 1 | 2 | (2,0) | true | none | Remove | (2,0) | yes |
| 2 | 1 | (1,2) | false | turn 1 / repeating rows | Change(2,1) | (2,1) | no |
| 2 | 2 | (2,1) | false | turn 2 / repeating rows | Change(1,2) | (3,0) | no |
| 2 | 3 | (3,0) | true | none | Remove | (3,0) | yes |
| 3 | 1 | (1,3) | false | turn 1 / repeating rows | Change(3,1) | (2,2) | no |
| 3 | 2 | (2,2) | false | turn 2 / repeating rows | Change(2,2) | (3,1) | no |
| 3 | 3 | (3,1) | false | turn 3 / repeating rows | Change(1,3) | (4,0) | no |
| 3 | 4 | (4,0) | true | none | Remove | (4,0) | yes |

### Andrew timing

| D | Call | Entry `(turn,duration)` | Captured expiry | Rows | Client event | Exit `(turn,duration)` | Expire? |
|---:|---:|---:|---|---|---|---:|---|
| 1 | 1 | (1,1) | false | turn 1 / repeating rows | Change(1,1) | (2,0) | no |
| 1 | 2 | (2,0) | true | turn 2 / repeating rows | Remove | (2,0) | yes |
| 2 | 1 | (1,2) | false | turn 1 / repeating rows | Change(2,1) | (2,1) | no |
| 2 | 2 | (2,1) | false | turn 2 / repeating rows | Change(1,2) | (3,0) | no |
| 2 | 3 | (3,0) | true | turn 3 / repeating rows | Remove | (3,0) | yes |
| 3 | 1 | (1,3) | false | turn 1 / repeating rows | Change(3,1) | (2,2) | no |
| 3 | 2 | (2,2) | false | turn 2 / repeating rows | Change(2,2) | (3,1) | no |
| 3 | 3 | (3,1) | false | turn 3 / repeating rows | Change(1,3) | (4,0) | no |
| 3 | 4 | (4,0) | true | turn 4 / repeating rows | Remove | (4,0) | yes |

## Effect Tick Count

- **FACT:** Both variants invoke `Process()` D+1 times before removal for
  positive uninterrupted D. Current performs D effect-scan passes; Andrew
  performs D+1.
- **FACT:** Current therefore gives a one-row/repeating aura exactly D executions.
  Andrew gives it D+1 executions. For general multi-row metadata, current
  executes rows at exact durations 1 through D; Andrew additionally executes
  rows at duration D+1. Empty scans are still processing passes, not actual
  effect ticks.
- **FACT:** Consequently Andrew's hunk does not merely move removal after the
  current final D tick. That tick already occurs in current. It permits a new
  boundary-plus-one scan, which may be a full additional tick under the special
  repeating rule or may execute an explicit D+1 row.

## BattlePetAbilityTurn Contract

- **FACT:** Runtime treats `BattlePetAbilityTurn::Duration` as an exact aura
  turn index only when the ability has more than one turn row and its greatest
  duration differs from 1.
- **FACT:** The condition is
  `Duration != m_turn && turnCount != 1 && topMaxTurnId != 1`. Therefore any
  one-row ability executes that row every aura processing pass regardless of
  its `Duration`; likewise, when the greatest duration is 1, every matching row
  executes every pass. Otherwise only rows with `Duration == m_turn` execute.
- **INFERENCE:** `Duration` is overloaded by this implementation: an exact
  schedule index for multi-turn schedules and effectively ignored for the
  single/turn-one periodic form. It is not used as an expiry threshold.
- **UNKNOWN:** No repository comment or authenticated build-18414 row dump
  establishes whether the special condition is correct retail behavior or how
  aura effect duration is intended to relate to the maximum turn-row duration.

## Aura Duration Source

- **FACT:** All current `AddAura` calls pass the triggering
  `BattlePetAbilityEffect` row's `TriggerAbility` as the aura ability and
  property 2 as duration. Positive aura effect 26 hard-codes `maxAllowed=1`;
  negative effect 50 and periodic trigger 54 take maximum from property 3;
  periodic-positive 63 and powerless aura 178 leave maximum zero; toggle aura
  76 uses property 2 and default maximum.
- **FACT:** These are effect-property durations, not derived from the triggered
  aura's `BattlePetAbilityTurn::Duration`. Current weather architecture has no
  separate handler and does not alter this conclusion.
- **FACT:** `AddAura` normalizes duration 0 to 1, leaves positive values intact,
  and leaves `-1` intact. It emits apply first, returns without creating an aura
  on failure flags, performs maximum-count replacement, then constructs the
  aura and calls `OnApply()` immediately.

## Infinite Duration

- **FACT:** `-1` bypasses natural expiry and duration decrement. Its turn still
  increments and it still emits `AURA_CHANGE(-1,turn)` on each processed pass.
- **FACT:** Infinite auras can expire via Prowl 543 after turn 2 plus the
  did-damage condition, the generic next-attack check with the same turn/state
  condition, toggle removal, maximum-count replacement, victim death, or
  battle cleanup. Explicit `Expire()` is guarded.
- **FACT:** Andrew's captured finite predicate is always false for `-1`, so its
  placement does not itself alter these special paths. Task 032 makes no such
  change.

## Application vs First Tick

- **FACT:** Successful `AddAura` queues `AURA_APPLY`, inserts the aura, and calls
  `OnApply()` synchronously during the creating effect. `HandleRound()` executes
  round-start procs, chosen/non-proc actions, then round-end procs, and only
  afterward processes auras.
- **FACT:** An aura created by any of those casts in round N is therefore in its
  team's aura container when that same round's aura phase is reached and may
  receive its turn-1 periodic effect in round N. Application/state changes,
  periodic execution, `AURA_CHANGE`, and later expiration are distinct events.
- **UNKNOWN:** Mutation of an aura container while that same container is being
  range-iterated is not protected by this code; a periodic effect that adds an
  aura to its current target may invalidate iteration. This pre-existing issue
  is not solved by either expiry placement.

## Round Processing Order

**FACT:** `HandleRound()` resolves pending swaps/cast preparation, resets
per-round damage conditions and snapshots stats, selects speed order, executes
round-start procs, executes chosen actions in team order, executes round-end
procs, queues aura-processing-begin, calls `firstTeam->ProcessAuras()` then the
second team, queues aura-processing-end, emits changed speed/power, increments
active-ability progress, creates/sends the round result, removes expired auras
and reduces cooldowns in `TurnFinished`, increments/resets round state, clears
packet effects, then counts survivors and invokes guarded `EndBattle()` if a
team has none. Death may be recorded during actions or aura effects, but battle
termination is checked only after result/reset work.

## Final-Tick Consequences

- **FACT:** The generic aura scan can dispatch any implemented effect row joined
  to its selected turn: damage/nonlethal damage, healing, state updates,
  triggered aura application, conditionals, toggles/removals, and other mapped
  handlers. Damage can call `Kill`; aura handlers can create or replace auras;
  toggle can expire an aura. Resurrection is not established as a distinct
  safe boundary contract by this review.
- **FACT:** Andrew permits all such work on D+1 before natural removal. It can
  kill a pet, expire other auras, create another aura, or set a terminal round
  result. `Process()` does not re-test `m_expired` after executing effects.

## Reentrancy / Self-Expiration

- **FACT:** An executed toggle can call `Expire()` on a found aura; maximum
  replacement can expire a matching aura; damage can call `Kill()`, which
  expires all victim auras. These paths can reach the currently processing aura.
- **FACT:** `Expire()`'s guard prevents duplicate remove events, removal procs,
  and state reversal. However, after an effect expires this aura, both variants
  continue the enclosing scan and can execute later rows. On ordinary passes
  they then emit `AURA_CHANGE` and mutate clocks even though expired.
- **FACT:** On Andrew's D+1 pass, it calls guarded `Expire()` after the scan and
  returns, so it does not duplicate removal but also does not undo effects or
  stop later effects that followed self-expiry. This is not enough to prove the
  moved boundary safe.

## AURA_CHANGE Contract

- **FACT:** `AURA_CHANGE` reports entry values before increment/decrement.
  Current and Andrew both report D=1 as `(1,1)`; D=2 as `(2,1),(1,2)`; and D=3
  as `(3,1),(2,2),(1,3)`. Neither reports duration zero on natural expiry.
- **FACT:** Andrew's extra D+1 scan returns after removal, so wire-visible change
  sequences are identical even if that scan emits other effect events.

## AURA_REMOVE Ordering

- **FACT:** Current natural order is final eligible turn-D effect event(s),
  `AURA_CHANGE(D entry values)`, increment/decrement, then on the next aura
  phase `AURA_REMOVE`, current target-cast `ON_AURA_REMOVED` effects, and
  `OnExpire` state reversals.
- **FACT:** Andrew's order adds D+1 effect event(s) immediately before
  `AURA_REMOVE`; it then runs the unchanged target-cast removal proc and state
  reversal without `AURA_CHANGE` or clock mutation on that pass.
- **FACT:** Task 031 left the target/caster question unresolved; this report and
  source leave `m_target` unchanged.

## Death During Final Tick

- **FACT:** Damage calls `Kill`, which expires every aura on the victim before
  emitting the dead-state update. If the processing aura belongs to that
  victim, its guarded later expiry cannot duplicate removal. If it belongs to
  another pet, its natural removal remains separate. The loop can nevertheless
  continue processing already-expired aura effects because there is no re-test.
- **FACT:** `ProcessAuras()` does not skip a pet because it is dead; only each
  aura's expired flag is checked before entry. Andrew can therefore perform its
  extra D+1 scan on a living or dead bearer unless another path already expired
  it.
- **FACT:** A terminal kill sets the round result, while survivor counting and
  `EndBattle()` occur after packet creation/reset. Task 025 prevents duplicate
  finalization, but that guard does not establish retail ordering for an
  additional D+1 effect or removal proc. The overlap is mechanically
  duplicate-remove-safe, not semantically proven.

## Build-18414 Data

- **FACT:** Repository searches find DB2 structure/load declarations and runtime
  joins, but no extracted `BattlePetAbilityEffect`/`BattlePetAbilityTurn`
  build-18414 row data. No concrete local record can join property-2 aura
  duration to triggered-ability turn rows or expected tick count.
- **FACT:** Attempted Internet search was unavailable in this environment (the
  search service returned HTTP 401). The already cloned Andrew and Flamehawk
  source lineages contain code, not an authenticated build-18414 timing dump.
- **UNKNOWN:** No concrete ability ID, duration, turn-row sequence, and retail
  tick count can responsibly be named from the available evidence.

`BUILD18414_AURA_TIMING_DATA = INSUFFICIENT`.

## Flamehawk Comparison

- **FACT:** Flamehawk final relevant commit
  `e91819a4c2acb6cc8eea09ed264d30aec6ed12b9` has the same leading finite check,
  one-based turn initialization, pre-mutation `AURA_CHANGE`, turn increment,
  and finite duration decrement as current preservation.
- **FACT:** Later Flamehawk history replaces the BattlePet folder with a
  Draenor-derived implementation, which is not independent MoP timing evidence.

Classification: **`AGREES_WITH_CURRENT`**. Shared lineage is corroboration only.

## Other Lineage Comparison

- **FACT:** Andrew is the only inspected MoP-derived lineage that moves the
  check, and its commit supplies no boundary proof or data fixture. Flamehawk
  retains current behavior. No trustworthy independently derived build-18414
  implementation or retail packet trace was recovered.
- **UNKNOWN:** Newer-expansion behavior may use different data/counters and was
  not treated as MoP evidence.

## Off-by-One Analysis

With one-based entry turn and positive D, turn D is the nominal last pass and
D+1 is the first pass satisfying `turn > max`.

| Policy | Positive-D effect scans before removal | Consequence |
|---|---:|---|
| A: expire before `>` | D | current; turns 1..D |
| B: process then expire on `>` | D+1 | Andrew; includes D+1 |
| C: expire before `>=` | D-1 | omits nominal turn D |
| D: process then expire on `>=` | D | processes turn D, then removes without its change/mutation |

- **FACT:** Current policy A already processes boundary turn D. Andrew policy B
  processes beyond the nominal boundary. The existing `>` is consistent with
  one-based inclusive turns; changing it with placement would create a separate
  contract not present in Andrew's hunk.
- **UNKNOWN:** Static arithmetic alone cannot prove whether property-2 D denotes
  D periodic passes, D subsequent rounds in addition to application round, or
  some data-specific convention. Therefore none of A/B/D is selected as retail
  truth solely for elegance.

## Effect-Row Completion

- **FACT:** For general multi-row schedules, current can never execute an
  explicit row at `Duration=D+1` when the creating effect supplies D. Andrew
  can. Conversely, for the special repeating form Andrew necessarily permits
  an additional execution after remaining duration reached zero.
- **UNKNOWN:** Because no build-18414 row join is present, there is no concrete
  example proving `max(turn Duration) == D+1`, `== D`, or unrelated. The
  expected “execute every defined row” invariant and the relationship between
  the two independently stored metadata fields are unestablished.

## Semantic Invariant

**`INSUFFICIENT_EVIDENCE`.** Current gives D scans and Andrew D+1, including a
provable extra tick for repeating metadata. Either could encode application-
round versus subsequent-round semantics, and no build-18414 data or packet
trace chooses between them. Client change/remove sequences alone do not expose
Andrew's added scan, while damage, death, self-expiry, and no-effect D+1 passes
have materially different server consequences.

## Final Classification

**`NEEDS_BUILD18414_DATA`.** The mechanical difference, clocks, tick counts,
turn-row selection, application/round order, packet events, reentrancy, and
kill paths are reconstructed. The decision gate nevertheless fails requirements
7, 8, and 11 at the semantic level: guards prevent duplicate removal/finalize,
but continuation after self/death expiry remains questionable, and no
source/data establishes which boundary is retail-correct. No C++ source is
changed.

Remaining A7 work includes authenticated ability/effect/turn row joins,
build-18414 packet or gameplay traces for duration-one and multi-turn periodic
abilities, the separately unresolved removal-proc caster identity, boolean
state ownership, delayed harmful aura activation, stacking/maxAllowed policy,
and weather timing. None is silently combined here.

## Validation Plan

- **FACT:** Documentation was checked with `git diff --check` and source scope
  review. Because classification is not `STATICALLY_JUSTIFIED_CORE_FIX`, the
  prescribed translation-unit and optional aggregate builds are not applicable
  and `/tmp/mop-preservation-dev` remains unconfigured.
- **FACT:** Static review covered D=1/2/3, single/repeating and multi-turn rows,
  absent final rows, explicit final and D+1 rows, infinite/Prowl/next-attack,
  replacement, target/caster death, self-expiry, event ordering, unchanged
  target-based removal proc, and guarded end-battle cleanup. It found no source
  regression because there is no implementation patch; it did identify the
  documented pre-existing continuation/reentrancy uncertainty.
- Future implementation requires authenticated build-18414 evidence selecting a
  boundary, followed by the exact `BattlePetAura.cpp.o` build before any source
  commit. Aggregate validation may then be attempted once as prescribed.
