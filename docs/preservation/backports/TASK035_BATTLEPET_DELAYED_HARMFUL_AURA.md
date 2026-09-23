# Task 035: Battle-Pet Delayed Harmful Aura Activation

## Starting State

- **FACT:** Review started from clean commit `8e6d7c2c8ff6161ce2ef8cb7501498cbf3f7bbf6`, the merge of Task 034, on task branch `work`.
- **FACT:** Task 034 (`0aa161a`) is present. Task 029's numeric contribution bookkeeping remains `m_auraStates[stateEntry->StateId] += stateEntry->Value`; Tasks 030--034 made no later battle-pet source-policy change. Task 025's early `Finished` guard and immediate state transition remain in `PetBattle::EndBattle`.
- **FACT:** The working tree was clean. `/tmp/mop-preservation-dev/CMakeCache.txt` was absent and ccache occupied 0.0 GiB of 5.0 GiB.
- **FACT:** This task changes documentation only. It does not select delayed gameplay behavior.

## Andrew Provenance

A temporary clone of `https://github.com/andrew-downey/Legends-of-Azeroth-Pandaria-5.4.8` was inspected without changing preservation remotes. Andrew's inspected final revision is `f1abdd1a2754f4a39f9e5d2cd5112e63899a86c4`; the requested final boundary `bd8ad551` contains the same delayed helpers and handlers as the later tip.

| Commit | Function | Old behavior | New behavior | Survives `bd8ad551`? | Coupling |
|---|---|---|---|---|---|
| `1e338126` | `PetBattle::AddAura` | emitted failed apply, then returned | returns before constructing an apply event when flags are nonzero | yes | event ordering and every aura handler |
| `1e338126` | positive/powerless/toggle handlers | default/P3 maximum | literal maximum one in several handlers | yes | Task 033 stack policy |
| `987cdb97` | negative and periodic harmful handlers | P3 maximum | literal maximum one | yes | Task 033 |
| `867b69b0` (parent `1e138b1f`) | `IsTriggerDelayedActivation` | absent | scans the triggered ability's turn rows for `HasProcType == 1 && ProcType == ON_AURA_REMOVED` | yes | DB2 metadata |
| `867b69b0` | `HandleNegativeAura` | always calculates P1 and passes `m_flags` | skips P1 for detected delayed triggers and always passes `NONE` | yes | also changes every ordinary failure into a successful aura application |
| `867b69b0` | `HandlePeriodicTrigger` | always calculates P1 | skips P1 only for detected delayed triggers; still passes `m_flags` | yes | targetability and inherited flags remain effective |
| `867b69b0` | `BattlePetAura::Process` | expires before scanning effects | computes expiry, scans effects, then expires | yes | unresolved Task 032 D+1 boundary; can duplicate removal-row execution |
| `867b69b0` | `BattlePetAura::Expire` | removal proc caster is target | caster is original aura caster | yes | unresolved Task 031 |
| `867b69b0` | `Execute` | flags can leak between targets | saves/restores flags per target | yes | broad unrelated correctness change |
| `aeef0c9d` | dispatch table | negative: 50; periodic: 54 | adds negative mappings 22, 52, 139, 145, 165, 168 | yes | broad A8 handler backport, out of scope |
| `aeef0c9d` | `PetBattle::Cast` | for duration >1, accepts only equal turn; otherwise permissive | direct turn requires exact duration; proc `turn == 0` accepts only duration 1 | yes | multi-turn/proc semantics; does not repair nonzero removal casts |
| `bd8ad551` | relevant delayed code | Andrew state above | no delayed-handler/aura change | yes | commit's battle-pet work is unrelated passives/weather |

**FACT:** `987cdb97` is an ancestor of `867b69b0`; `aeef0c9d` is its child and `bd8ad551` is the requested later boundary. No requested commit after `867b69b0` repairs delayed cleanup or provides data validation.

## Delayed Detection Rule

Andrew final uses exactly:

```cpp
turn && turn->AbilityId == m_effectEntry->TriggerAbility
    && turn->HasProcType == 1
    && turn->ProcType == PET_BATTLE_ABILITY_PROC_ON_AURA_REMOVED
```

- **FACT:** Detection is called per executing outer effect, but queries the outer effect's `TriggerAbility` (the prospective aura ability), not the outer ability ID or outer turn.
- **FACT:** It scans the entire `BattlePetAbilityTurn` store and returns on the first matching turn row. Duration, turn ID, effect rows, target, P values, and number of matches are ignored.
- **FACT:** Several matching rows still produce only Boolean `true`; activation later can execute several matching rows. No matching row returns `false` and selects ordinary P1 calculation.
- **FACT:** It is specifically keyed to proc type 5 plus `HasProcType == 1`, not merely any proc or any multi-turn metadata.

## Current Negative Aura Path

Current pseudocode is:

```text
Execute target -> mark m_flags MISS if target is not targetable
HandleNegativeAura -> CalculateHit(outer P1)
AddAura(caster, target, TriggerAbility, effect ID, outer P2, m_flags, outer P3)
  count active target-wide same-ability auras; choose ID; normalize duration 0 to 1
  emit AURA_APPLY carrying m_flags
  if any flags: return (no replacement, object, OnApply, or later Process)
  expire enough old copies to satisfy P3
  append aura; OnApply immediately applies TriggerAbility state rows
```

- **FACT:** A clean hit creates the aura, emits apply, may emit replacement removals/procs after apply, applies state rows, and becomes eligible for `Process`.
- **FACT:** A random miss, untargetability miss, immune flag, or any other nonzero flag emits a failed `AURA_APPLY` but creates no aura and applies no state. `CalculateHit` only adds miss; immune/other flags must already have arisen elsewhere.

## Andrew Negative Aura Path

Andrew-final pseudocode is:

```text
Execute target -> mark local m_flags MISS if untargetable
if triggered ability lacks any HasProcType=1/proc=5 turn: CalculateHit(outer P1)
AddAura(..., flags = NONE, maxAllowed = 1)
  emit successful AURA_APPLY
  replace active copy at limit one
  append aura; OnApply immediately applies state rows
```

- **FACT:** Detection suppresses only `CalculateHit`; it does not itself schedule activation.
- **FACT:** Passing `NONE` is unconditional. Thus Andrew discards not only flags for detected delayed effects, but also the ordinary P1 miss and preexisting targetability/failure flags for every negative aura.
- **FACT:** Aura duration is outer P2, maximum is literal one, and removal activation relies on `BattlePetAura::Expire` calling `Cast(..., proc=5)`.
- **INFERENCE:** The unconditional flag erasure is a likely Andrew correctness defect independent of whether a pending marker is desirable. Importing the hunk would regress ordinary negative auras.

## Current Periodic-Harmful Path

```text
if outer P0 != 0 and parent chain failed: return without event
CalculateHit(outer P1)
AddAura(..., duration=P2, flags=m_flags, maxAllowed=P3)
```

The same AddAura failure behavior described above applies. On success the aura's `TriggerAbility` effects are scanned by `BattlePetAura::Process`; on failure there is an apply failure event but no aura.

## Andrew Periodic-Harmful Path

```text
if outer P0 != 0 and parent chain failed: return
if not delayed by the exact triggered-ability row scan: CalculateHit(outer P1)
AddAura(..., duration=P2, flags=m_flags, maxAllowed=1)
```

- **FACT:** P0 remains active. A detected delayed row suppresses only the random P1 roll. Unlike `HandleNegativeAura`, preexisting flags (including untargetability MISS from `Execute`) still prevent creation.
- **FACT:** Literal one is the separate Task 033 policy. No delayed-specific pending bit, phase, cleanup, or activation ownership exists.

## Handler / Effect-ID Inventory

| Lineage/final state | `HandleNegativeAura` IDs | `HandlePeriodicTrigger` IDs |
|---|---|---|
| current preservation | 50 | 54 |
| Andrew at `bd8ad551` | 22, 50, 52, 139, 145, 165, 168 | 54 |
| Flamehawk final `0ff23eff` | 50 (22 is `Dummy`; 52 is `ControlAura`) | 54 |

**FACT:** Andrew's extra mappings arrived in the broad `aeef0c9d` effect implementation and are not backported here. Flamehawk has no delayed-detection helper.

## Proc-Type Relationship

- **FACT:** `PetBattleAbilityProcType` defines -1 none; 0 apply; 1 damage taken; 2 damage dealt; 3 heal taken; 4 heal dealt; 5 aura removed; 6 round start; 7 round end; 8 turn; 9 ability; 10 swap in; and 11 swap out.
- **FACT:** Only `PET_BATTLE_ABILITY_PROC_ON_AURA_REMOVED` (5) participates in Andrew's delayed predicate.
- **FACT:** `BattlePetAbilityTurnEntry` separately contains `HasProcType` and `ProcType`; Andrew requires both the Boolean marker and value 5 for detection. `PetBattle::Cast` later compares only `ProcType`, not `HasProcType`.

## Initial Hit Suppression

- **FACT:** Andrew suppresses the outer P1 accuracy calculation for a triggered aura ability having any matching proc-5 row.
- **FACT:** Negative aura application then also erases every failure flag. Periodic harmful application does not.
- **INFERENCE:** The structural intent is a pending marker whose removal proc supplies the payload, but source does not prove that the removal payload has an accuracy-bearing handler or that it is executed exactly once.

## Eventual Hit Calculation

- **FACT:** There is no generic later `CalculateHit` in `BattlePetAura::Expire` or `PetBattle::Cast`. `Expire` calls `Cast` for the aura ability, current `m_turn`, and proc 5. `Cast` selects matching ability turn/effect rows and each inner effect handler independently decides whether to call `CalculateHit` and which inner P property it uses.
- **FACT:** Therefore the outer P1 is not deferred; it is discarded. An inner damage/aura handler may roll its own P1, a handler without accuracy may perform no roll, and an unimplemented handler may do nothing.
- **FACT:** A nonzero removal cast requires `abilityTurnEntry->Duration == m_turn` before proc comparison. Detection ignores this duration relationship, so “delayed” can be detected yet execute no removal payload.
- **FACT:** `BattlePetAura::Process` does not filter `ProcType`. A proc-5 row can also be executed by the normal turn scan. Under Andrew's post-scan expiration, the same row can execute once in `Process` and again in nested removal `Cast` at the same `m_turn`.
- **INFERENCE:** The hard gate fails: equivalent accuracy/failure handling exactly once is not established.

## Pending Aura State

- **FACT:** A created marker is a normal aura. It is immediately visible through `AURA_APPLY`, counted for stacking, targetable by every explicit expiry path, and eligible for normal `Process`.
- **FACT:** It can emit `AURA_CHANGE`, be replaced, expire on target death, invoke proc 5 on every `Expire`, and affect target state through `OnApply` before any alleged eventual hit.
- **FACT:** No pending/dormant flag prevents normal turn-row effects, state changes, replacement, or removal activation.

## OnApply State Interaction

**Classification: `STATE_ROWS_APPLY_IMMEDIATELY`.** `AddAura` calls `OnApply` synchronously, and `OnApply` applies every `BattlePetAbilityState` row belonging to `TriggerAbility`. Andrew adds no suppression. Repository-local material contains no authenticated joined build-18414 rows proving that all detected delayed aura abilities lack state rows. Task 029 makes numeric reversal correct; it does not make early application semantically justified.

## Failure Cleanup

- **FACT:** A miss in an inner removal-proc effect is local to that inner handler. The outer aura is already marked expired before the proc cast and `OnExpire` runs afterward, so natural/removal activation does reverse state rows and cannot continue processing as an active aura.
- **FACT:** There is no delayed activation status and no cleanup conditional on success. If the removal row is absent at the expiring `m_turn`, the aura simply expires without payload. If an inner handler emits a failed effect, no special pending object remains because expiry is already underway.
- **FACT:** Before expiry, normal aura turn effects can miss without expiring the pending aura. Andrew does not distinguish that from an activation failure.
- **INFERENCE:** This is not a coherent “create first, validate later” transaction; it is an ordinary aura plus a possible removal proc.

## Client Event Sequence

- **FACT:** Current ordinary success: `AURA_APPLY` (success), replacement `AURA_REMOVE`/proc events if required, then state events from `OnApply`; later `AURA_CHANGE` per completed process and eventually `AURA_REMOVE`, proc payload events, and state reversal events.
- **FACT:** Current ordinary failure: failed `AURA_APPLY` only; no object, state event, change, remove, or removal proc.
- **FACT:** Andrew detected negative initial cast: successful `AURA_APPLY` even if untargetable/otherwise flagged, possible replacement remove/proc, immediate state events. Periodic detected cast has the same sequence only when inherited flags are zero; otherwise its failed apply creates nothing.
- **FACT:** Intermediate processing can emit payload damage/heal/state events from all matching turn rows and then `AURA_CHANGE`. Expiry emits `AURA_REMOVE` before removal-proc payload events, followed by `OnExpire` state events.
- **FACT:** Under Andrew's D+1 ordering, an expiry-phase proc-5 effect can emit payload once in the unrestricted process scan, then `AURA_REMOVE`, then the same payload again through `Cast`. Failure flags belong to each inner event; there is no client event identifying “pending activation failed” as a lifecycle result.

## Delay-Length Semantics

- **FACT:** Marker lifetime comes from outer P2 (`duration`/`m_maxDuration`), while normal aura effect eligibility uses each triggered ability turn row's `Duration` against `m_turn` (with a single-row/top-duration-one shortcut).
- **FACT:** Removal activation is not hard-coded to one round: it occurs on any call to `Expire`, and `Cast` only selects a proc-5 row whose `Duration` equals the aura's current `m_turn` for nonzero turns.
- **FACT:** For outer delay 1 or 2, current pre-scan expiration gives D normal scans; Andrew's post-scan version gives D+1, as Task 032 established. Multiple rows are all scanned, and all rows matching ability/duration/proc execute; detection merely returns Boolean at the first match.
- **UNKNOWN:** No authenticated rows establish which P2, proc-row Duration, and intended payload timing combinations build 18414 uses.

## Task 031 Dependency

**`DEPENDS_ON_TASK031`.** Removal is the proposed activation point. Current preservation calls proc 5 with `m_target` as caster; Andrew calls with original `m_caster`. That choice affects accuracy states, family, target acquisition, event source, and effects. Equivalence is not statically established.

## Task 032 Dependency

**`FUNCTIONS_UNDER_CURRENT_TIMING`**, narrowly: current pre-scan expiry still invokes the proc-5 removal cast and therefore can activate a duration-matching removal row. **However**, Andrew's complete final lifecycle includes D+1 scanning and can process the proc row before removal; importing the design as a whole would select the unresolved Task 032 boundary and can double-execute. Exact retail timing remains coupled/unknown.

## Task 033 Dependency

**`DEPENDS_ON_TASK033`.** Andrew forces maximum one. Under current P3, multiple pending copies can coexist when P3 permits. At a reached limit, replacement calls `Expire` on old copies, and each expiry can run proc 5 immediately. Thus maximum/count policy controls whether and when the delayed payload fires.

## Task 034 Scheduling Dependency

**`DELAY_SEMANTICS_DEPEND_ON_SCHEDULING`.** `AddAura` appends to `std::list`. If a handler creates the marker on the list currently traversed, the new tail is eligible later in that same `ProcessAuras` phase; on a not-yet-visited pet it is also eligible that phase. Its `m_turn` can advance immediately, shortening the apparent delay. Changing scheduling is out of scope and unauthenticated.

## Death Before Activation

- **FACT:** If the target dies, `Kill` expires every target aura. The marker emits remove, attempts proc 5 immediately, and reverses state before dead state is set. It therefore activates rather than silently cancels whenever a matching turn row exists.
- **FACT:** If the original caster dies first, only that caster's own auras are expired. A marker resident on the target remains. Later activation uses current target-as-caster policy, while Andrew final uses the possibly dead original caster; behavior therefore depends on Task 031 and handler targetability/death checks.
- **UNKNOWN:** No data or packet evidence proves intended cancellation/activation on either death ordering.

## Replacement Before Activation

- **FACT:** Reaching the maximum expires oldest active same-ability copies before appending the new one, but after the new apply event is queued. Expiry runs proc 5, so replacement can activate A early while B is being applied.
- **FACT:** Andrew's literal one makes every successful duplicate do this. Current P3 controls whether it happens. This is the direct Task 033 coupling.

## Explicit Removal

- **FACT:** Implemented explicit expiry routes include toggle logic (`HandleDamageToggleAura`), maximum replacement, `Kill`, natural expiration, and end-of-battle cleanup. Toggle/replacement/kill/natural paths calling `Expire` can run proc 5; end battle directly calls `OnExpire` then clears, so it does not call the removal proc.
- **FACT:** No general dispel architecture was found or claimed. Cleanse-like handlers in other lineages/mappings are not evidence for current behavior.

## Build-18414 Data

`BUILD18414_DELAYED_AURA_DATA = INSUFFICIENT`.

- **FACT:** Repository-local searches found DB2 structure/format declarations and runtime stores, but no committed extracted `BattlePetAbilityEffect`, `BattlePetAbilityTurn`, and `BattlePetAbilityState` rows from build 18414 with the required join.
- **FACT:** Internet search was attempted but the available search service returned HTTP 401; no external row dump was authenticated.
- **FACT:** Andrew comments name an example, but Andrew source is lineage intent, not data specification, and is not accepted as row evidence.

## Concrete Ability Examples

- **UNKNOWN:** None are enumerated because no authenticated joined build-18414 rows were recovered. In particular, the name in Andrew's comment is not promoted to evidence.

## Flamehawk Comparison

**Classification: `AGREES_WITH_CURRENT`.** Flamehawk final calculates outer P1 in both handlers, passes flags and P3, has no delayed predicate/pending marker, and retains target-as-caster removal. Its mappings are negative 50, periodic 54, while 22 is `Dummy` and 52 is `ControlAura`. It is lineage evidence only.

## Other Lineages

- **FACT:** Current preservation/master ancestry and Flamehawk are the practical independent comparison available in the prior reconstruction set; both use the ordinary initial-hit architecture.
- **UNKNOWN:** No additional independently derived, build-18414-authenticated implementation was located. Search unavailability and absent local rows preclude claiming newer or other-expansion code as evidence.

## Generic Safety Findings

- **FACT:** No isolated current-core defect was proven. Current failure flags prevent aura construction, current list lifetime is safe, and proc removal has an expired guard.
- **FACT:** `BattlePetAura::m_trigger` defaults to zero and is never read anywhere in current or Andrew battle-pet source. The constructor parameter is not assigned to it, but delayed logic does not read it; changing it would be cosmetic/unjustified in this task.
- **FACT:** The unrestricted `BattlePetAura::Process` proc-row scan and Andrew D+1 double-execution risk are architectural findings, but changing them would select broad proc/timing semantics without authenticated rows. They are not repaired here.

## Delayed Lifecycle Model

**Classification: `COMBINATION` (`PENDING_MARKER_AURA` + `TRIGGER_ON_AURA_REMOVAL`, with ordinary aura processing still active).**

1. Outer harmful handler classifies the triggered ability by existence of any marked proc-5 turn row.
2. It suppresses the outer P1 roll; negative forcibly clears flags, periodic does not.
3. A normal aura is applied, states take effect immediately, and ordinary turn processing remains enabled.
4. Any expiry emits remove and casts proc 5 for the current aura turn.
5. Matching inner handlers may independently roll their own accuracy, or may have no accuracy roll.
6. State reversal happens after the removal cast. There is no explicit activation-success state.
7. Replacement/death can activate early; same-phase traversal can advance early; natural timing and proc caster alter behavior; Andrew D+1 can double-execute a removal row.

This is not a complete, exactly-once delayed-delivery lifecycle.

## Final Classification

**`NEEDS_BUILD18414_DATA`.** The exact Andrew mechanism is reconstructed, but the decision gate fails at least equivalent exactly-once hit handling, state timing, Task 031 independence, Task 033 independence, Task 034 scheduling independence, coherent early-removal behavior, and authenticated metadata. Task 032's current boundary can call removal procs, but Andrew's combined D+1 behavior remains unresolved. No C++ source change is permitted.

Remaining A7 blockers are authenticated joined ability/turn/effect/state rows or packet/runtime traces for delayed abilities, proc-5 caster ownership (Task 031), expiry boundary (Task 032), pending-stack/replacement policy (Task 033), first-process scheduling (Task 034), and boolean aura ownership (Task 030).

## Validation Plan

- **FACT:** Documentation-only validation consists of source/history inspection, lineage comparison, repository row search, `git diff --check`, and final status/log checks. Translation-unit and aggregate builds do not apply because source did not change.
- **FACT:** Static lifecycle matrix: (1) ordinary hit creates and applies; (2) ordinary miss produces failed apply only; (3) Andrew delayed negative creates a normal marker; (4) removal payload hit depends on inner handler; (5) removal payload miss has no special transaction but outer expiry completes; (6) target death expires/activates; (7) caster death leaves target marker and exposes caster ambiguity; (8) replacement can activate early; (9) toggle/explicit expiry can activate; (10) natural expiry casts proc 5; (11) same-list creation can process same phase; (12) later-pet creation can process same phase; (13) state rows apply immediately; (14) Task 029 reverses recorded numeric deltas once; (15) Task 031 target-as-caster remains; (16) Task 032 pre-scan expiry remains; (17) Task 033 P3 remains; (18) Task 034 list scheduling remains; (19) removal event precedes proc payload; and (20) exactly-once activation is not proven and Andrew D+1 can violate it.
- **FACT:** No translation-unit build and no aggregate build were run because the only changed file is Markdown.
