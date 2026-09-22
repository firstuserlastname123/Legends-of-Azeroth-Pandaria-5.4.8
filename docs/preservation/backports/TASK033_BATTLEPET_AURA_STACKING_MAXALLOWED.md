# Task 033: Battle-Pet Aura Stacking and maxAllowed

## Starting State

- **FACT:** Review started from clean branch `work` at
  `e14f0acef9a414a91531f35fb5ba5e31fdd8e458`, the merge of Task 032. A direct
  `ls-remote` query of the preservation repository reported the same object for
  `refs/heads/preservation/main`; no remote was added or changed.
- **FACT:** Task 029's per-aura numeric modifier bookkeeping remains in
  `BattlePetAura::OnApply`/`OnExpire`. Tasks 030, 031, and 032 are documentation
  commits and introduced no C++ changes. Task 025's early `Finished` guard and
  state transition remain in `PetBattle::EndBattle`.
- **FACT:** `/tmp/mop-preservation-dev/CMakeCache.txt` was absent initially.
  ccache reported 0.0 GiB used of 5.0 GiB. A build tree was not configured
  because this review does not authorize a source change.

## Andrew Provenance

A temporary clone of
`https://github.com/andrew-downey/Legends-of-Azeroth-Pandaria-5.4.8` was used.
The preservation repository's remotes were not changed.

| Commit (exact parent) | Relevant final-state contribution | Disposition |
|---|---|---|
| `1e338126a8f3e5381218dc90a50185119beeb592` (`2e4a6f7fa9311ca217ac647dafe0a908e8cd948e`) | Gives periodic-positive, toggle, and powerless calls explicit `maxAllowed=1`; moves the `flags` return in `AddAura` before `AURA_APPLY`. Mixed trainer/weather/swap commit. | The three stack arguments survive; failure ordering survives. |
| `987cdb972e1fc5ce3580fd8b6d8c545c4df8fd10` (`1e338126a8f3e5381218dc90a50185119beeb592`) | Changes negative and periodic harmful maximum from property 3 to literal 1. Stacking-only source hunk. | Survives through the requested final commit. |
| `867b69b0d142efaaea10b83970342900d3405f2e` (`1e138b1f1d7e1660eb28b647b7193afe961acb72`) | Retains literal 1, but adds delayed-trigger detection/hit behavior and makes negative aura call `AddAura` with no failure flags. Also changes expiry timing and removal-proc caster in other files. | Stack literal survives, but negative failure behavior is mixed with delayed activation. |
| `aeef0c9d8e6484278ea98d07dde4cc608b077ff4` (`867b69b0d142efaaea10b83970342900d3405f2e`) | Maps more effect IDs to existing handlers; does not change the reviewed call arguments or `AddAura`. | No superseding stack rule. |
| `bd8ad5515418d94abd35ba7bf71430979243a09c` (`aeef0c9d8e6484278ea98d07dde4cc608b077ff4`) | Retains the reviewed calls; replaces weather architecture in a mixed commit. | Requested final point; no stack supersession. |

- **FACT:** Path-limited history from `1e338126` through `bd8ad551` contains
  only the five commits above for the relevant BattlePet files. No later hunk
  in that interval changes `PetBattle::AddAura`'s counting/replacement loop.
- **INFERENCE:** Andrew's commit messages describe a stacking guard, but provide
  neither build-18414 rows nor a packet/gameplay trace proving literal one.

## Current Aura-Creating Handlers

`P<n>` means `BattlePetAbilityEffectEntry::Properties[n]`. All current calls use
`TriggerAbility` for the created aura ability and the current effect `Id` as
the ability-effect/event source.

| Effect | Handler / target class | Duration | Maximum | Hit and failure behavior | Character |
|---:|---|---|---|---|---|
| 26 | `HandlePositiveAura`; caster target selected by table | P2 | literal 1 | `CalculateHit(P1)`; passes `m_flags` | self/friendly positive |
| 50 | `HandleNegativeAura`; target | P2 | P3 | `CalculateHit(P1)`; passes `m_flags`; TODO for chain/state fields | hostile |
| 54 | `HandlePeriodicTrigger`; target | P2 | P3 | returns on P0 chain failure; then `CalculateHit(P1)` and passes flags | periodic hostile/trigger |
| 63 | `HandlePeriodicPositiveTrigger`; caster | P2 | default 0 | `CalculateHit(P1)` and passes flags | periodic positive |
| 76 | `HandleDamageToggleAura`; handler target is opponent, aura target is caster | P2 | default 0 | hit is resolved before toggle; any fail returns before lookup | self toggle |
| 178 | `HandlePowerlessAura`; target | P2 | default 0 | optional precheck can set immune/return; then hit and flags | hostile control |
| 80 | no current live handler (`HandleNull`) | n/a | n/a | Andrew uses P2/P3, but weather is reference-only and out of scope | weather reference |

- **FACT:** Defaults come from `PetBattle.h`: flags are none and `maxAllowed`
  is zero. Therefore effects 63, 76, and 178 are the current zero callers.
- **FACT:** Current effect comments label P1 accuracy and P2 duration for every
  listed family. Only effects 50 and 54 label P3 `MaxAllowed`. Those comments
  and code are schema interpretations, not authenticated row evidence.

## AddAura Contract

**FACT:** Current `PetBattle::AddAura` executes this exact order:

1. Scan every aura in the target container, choosing `id` as one greater than
   the greatest still-contained aura ID and counting non-expired entries whose
   aura ability equals the requested `ability`.
2. Normalize duration zero to one. Positive values and `-1` are unchanged.
3. Construct and enqueue `AURA_APPLY`, including the proposed ID, normalized
   duration, flags, effect ID, source index, target index, and ability.
4. If `flags` is nonzero, return. No replacement, allocation, insertion, or
   `OnApply` occurs.
5. If maximum is nonzero and current matching count is at least maximum,
   calculate `1 + auraCount - maxAllowed` and expire that many matching auras.
6. Allocate one `BattlePetAura`, append it to the target's vector, and invoke
   `OnApply` synchronously.

`duration` is the aura object's initial remaining and maximum duration after
zero normalization. `maxAllowed` is a pre-insertion cap on active auras sharing
the target and aura ability, when nonzero. `flags` is both event failure/status
metadata and, by the broad `if (flags)` test, the creation veto.

| Input maximum vs current matching count `C` | Current behavior after successful flags |
|---|---|
| 0 | Never enters replacement; inserts, so count becomes `C+1`. |
| 1 | If `C>=1`, expires `C` matches; inserts, leaving one active. |
| 2 | If `C<2`, inserts; otherwise expires `C-1`, then inserts, leaving two. |
| `M>C` | No expiry; inserts, leaving `C+1<=M`. |
| `M==C` | Expires one, then inserts, leaving `M`. |
| `0<M<C` | Expires `1+C-M`, then inserts, leaving `M`. |

## Counting Scope

- **FACT:** The key is `(target aura container, requested aura ability)` plus
  `!HasExpired()`. It does not compare ability-effect ID, trigger origin,
  caster, state rows, target class, duration, or C++ instance type.
- **FACT:** Same ability/same caster and same ability/different caster count
  together on one target. Different abilities affecting the same state do not.
  Two effects producing the same `TriggerAbility` count together. Self versus
  hostile is immaterial once both are in the same target container; different
  targets never share a count.
- **INFERENCE:** The code implements target-wide copies of an aura ability, not
  caster-owned stacks. Whether retail intended this scope is unknown.

## Replacement Policy

- **FACT:** `Auras` is a vector. Replacement traverses from `begin()` and calls
  `Expire()` on the first required non-expired matching entries. Since new
  auras append and removal is deferred to `TurnFinished`, this is normally the
  oldest/lowest still-contained matching aura, including entries from any
  caster. It is deterministic vector order, not an explicit age/ID policy.
- **FACT:** `Expire()` marks the entry expired, queues `AURA_REMOVE`, calls the
  ability's `ON_AURA_REMOVED` proc using the current aura target as caster, then
  runs `OnExpire`. Task 029 makes numeric reversal subtract exactly that aura's
  recorded contribution. Actual vector erasure happens later in
  `PetBattleTeam::TurnFinished`.
- **FACT:** Replacement does not invalidate the vector because it does not
  erase. After all required expirations, the new aura is appended and its
  state changes occur in `OnApply`.
- **FACT:** Successful at-limit ordering is `AURA_APPLY(new)` first, then for
  each replacement `AURA_REMOVE(old)`, removal-proc effects, old state reversal,
  then new `OnApply` state events. This order can look counterintuitive but is
  the explicit current client-event contract.

## maxAllowed Zero Semantics

**FACT:** Zero is an unlimited/no-replacement sentinel (interpretation A). The
condition is `if (maxAllowed && auraCount >= maxAllowed)`, so it does not mean
zero copies, default one, or implicit refresh. Current explicit/default zero
callers are periodic-positive 63, toggle 76, and powerless 178. Toggle normally
prevents duplicates itself by finding and expiring an existing aura before it
ever calls `AddAura`; the other two have no `AddAura` bound.

## Andrew Parameter Changes

| Family | Current / pre-Andrew call | After `1e338126` | After `987cdb97` and final through `bd8ad551` |
|---|---|---|---|
| Positive 26 | `(..., P2, flags, 1)` | unchanged | unchanged |
| Negative 50 | `(..., P2, flags, P3)` | unchanged | `(..., P2, flags, 1)`; final delayed version passes `NONE, 1` |
| Periodic harmful 54 | `(..., P2, flags, P3)` | unchanged | `(..., P2, flags, 1)` |
| Periodic positive 63 | `(..., P2, flags)` | `(..., P2, flags, 1)` | unchanged thereafter |
| Toggle 76 | `(..., P2, flags)` | `(..., P2, flags, 1)` plus all-match toggle expiry | unchanged thereafter |
| Powerless 178 | `(..., P2, flags)` | `(..., P2, flags, 1)` | unchanged thereafter |
| Weather 80 (architectural reference) | source contains P2 duration/P3 maximum, but handler is not live | unchanged | Andrew retains P2/P3 in its later battle-wide implementation |

- **FACT:** Andrew changes maximum arguments, not duration: P2 remains duration.
  P1 remains hit accuracy in non-delayed paths. Andrew changes P3 from the
  current claimed maximum into ignored metadata for effects 50 and 54, while
  retaining P3 as weather maximum.
- **UNKNOWN:** No authenticated build-18414 row establishes whether P3 really
  is a maximum, whether zero is intentionally unbounded for any concrete row,
  or whether all these families should be limited to one.

## Failed-Aura Ordering

- **FACT:** Current preservation queues `AURA_APPLY` before returning on any
  nonzero flags. It constructs no aura, does not expire an existing aura, does
  not consume a persistent ID, does not insert, and does not mutate aura-owned
  state. A later successful attempt may reuse the proposed ID because IDs are
  recomputed from contained auras.
- **FACT:** `1e338126` moves the broad flags return before event construction.
  Andrew therefore emits no failed apply event. `867b69b0` additionally forces
  negative aura flags to none, which guarantees creation and is coupled to its
  delayed-activation rationale.
- **UNKNOWN:** Whether build 18414 requires a failed `AURA_APPLY` for combat
  feedback is not documented. Current behavior is classified
  **`CLIENT_CONTRACT_UNKNOWN`**, not statically inconsistent. Crucially, neither
  ordering replaces an old aura before discovering failure.

## 987cdb97 Guard

- **FACT:** Before the commit, harmful effects 50 and 54 alone used their P3
  values while positive 26 was already literal one and three other families
  defaulted to zero. The commit changes exactly effects 50 and 54 from P3 to
  literal one; it adds no conditional guard.
- **FACT:** With current `AddAura`, literal one causes every successful
  duplicate of the same triggered ability on the target to expire all active
  prior copies before inserting one. It prevents coexistence above one via
  replacement; it does not reject the duplicate and does not validate P3.
- **INFERENCE:** The subject “harmful ability stacking guard” indicates an
  attempt to stop harmful duplicate coexistence, perhaps because a zero P3 was
  interpreted as unlimited. Source alone cannot prove that the data value was
  invalid or that one is retail policy.
- **FACT:** Literal one remains in final Andrew.

## Delayed-Trigger Boundary

**`PARTIALLY_COUPLED`.** Literal-one maximum is mechanically separable in both
handlers. However, Andrew's final negative application also suppresses the
initial hit roll for removal-proc abilities and always passes no flags; that
changes whether an aura exists at all. Periodic harmful likewise conditionally
skips its hit roll. Reproducing the full final negative/periodic behavior would
therefore require delayed activation, hit-timing, and Task 031 proc assumptions.
No part is imported.

## Negative Aura Contract

- **FACT:** Current effect 50 performs `CalculateHit(P1)` before `AddAura`.
  Miss/immune/etc. flags reach `AddAura`; it queues failed apply and returns
  without replacement. Thus a failed duplicate cannot expire a successful
  existing aura.
- **FACT:** A successful duplicate uses P3. P3 zero coexists without bound;
  positive P3 at/below count replaces enough oldest target-wide same-ability
  copies to leave P3 after insertion. Caster identity is ignored.
- **UNKNOWN:** Refresh versus coexistence, P3 meaning, cross-caster ownership,
  and the client failure-event requirement remain retail questions.

## Periodic Aura Contract

- **FACT:** Effect 54 first honors P0 chain failure, then follows the same hit,
  failure-event, P2-duration, P3-maximum, and replacement rules as negative 50.
  Its successful repeated casts coexist below a positive P3, replace oldest
  copies at/above P3, and are unbounded for P3 zero.
- **FACT:** Effect 63 uses P2 but default maximum zero, so successful repeated
  positive periodic casts append independent instances. Each receives its own
  unique-in-container ID, immediate `OnApply`, later periodic processing, and
  client change/remove events.
- **UNKNOWN:** No authenticated row or trace proves whether these are intended
  independent ticks, refreshes, or bounded stacks.

## Positive/Harmful Comparison

- **FACT:** Current deliberately differs in source: positive 26 is fixed at
  one; harmful 50/54 use P3; periodic-positive 63 is unlimited; powerless and
  toggle pass zero (with toggle enforcing its own presence rule). Andrew makes
  all six reviewed families effectively maximum one.
- **INFERENCE:** Andrew increases uniformity, but uniformity is not evidence.
  Current comments distinguish effect schemas, so normalizing them merely for
  symmetry would erase a possibly data-driven distinction.

## Structural Safety

- **FACT:** For ordinary representable counts, the replacement arithmetic
  expires precisely `1+C-M` matches and insertion leaves `M`; no erase occurs
  during iteration, `Expire()` is idempotent, and allocation follows expiry.
- **FACT:** The code uses `uint8 removeCount` for an expression involving
  `uint32 auraCount`; a pathological count above the byte range can truncate.
  Establishing a reachable build-18414 path from a zero-unbounded caller to a
  later positive maximum for the same target/ability requires missing row and
  runtime evidence. It is recorded, not promoted into an unrelated repair.
- **FACT:** No dereference-after-erase, empty-range removal, `maxAllowed-1`
  underflow, infinite loop, or duplicate contained ID was found. Failed calls
  do not replace. `id=max+1` could overflow only after exhausting `uint32`, an
  infeasible/unestablished battle path.
- **UNKNOWN:** Creating an aura while the same target aura vector is being
  range-iterated during processing may invalidate iterators; Task 032 already
  records that broader reentrancy issue. It is not a maxAllowed defect.

## Client Event Order

| Scenario | Current order |
|---|---|
| First successful application | `AURA_APPLY`; insertion; new `OnApply` state events. |
| Second below positive max / any zero max | `AURA_APPLY`; insertion; new `OnApply`; both later receive independent `AURA_CHANGE`. |
| Application with current count exactly max | `AURA_APPLY(new)`; `AURA_REMOVE(oldest needed)`; removal proc; old state reversal; insertion; new state apply. |
| Application with count above max | Apply event; enough oldest matching remove/proc/reversal sequences to make room; insertion/apply. |
| Failed harmful application | Failed `AURA_APPLY` only; no remove, state change, insertion, or later change event. |

- **FACT:** Andrew `1e338126` removes the failed event. Its successful order is
  otherwise the same, but literal one increases replacement/removal-proc
  frequency. Andrew's delayed negative path can turn what current treats as a
  failure into an unconditional successful application.

## Task 029 Interaction

- **FACT:** If A owns `+X` and B owns `+Y`, state after both applies is
  `base+X+Y`. Replacing A runs its `OnExpire`, subtracting X exactly once, then
  C adds its own contribution Z, leaving `base+Y+Z`. Vector retention and the
  guarded `Expire()` prevent a second reversal.
- **FACT:** No Task 033 source patch changes that bookkeeping. Boolean ownership
  remains the separate Task 030 blocker and is not inferred from numeric math.

## Task 031 Interaction

- **FACT:** Every maximum-driven replacement invokes `ON_AURA_REMOVED` with the
  unresolved current target-as-caster behavior. Changing P3/zero limits to one
  would change how frequently and when those procs run, including cross-caster
  duplicate cases.
- **INFERENCE:** Selecting Andrew's replacement frequency without resolving
  applicable removal-proc rows could change gameplay beyond stacking. Full
  correctness can therefore depend on Task 031 evidence; caster identity itself
  remains untouched.

## Task 032 Interaction

- **FACT:** Maximum replacement calls explicit `Expire()` immediately and does
  not consult the natural `m_turn > m_maxDuration` boundary. Its mechanics are
  separable from Task 032's natural timing.
- **INFERENCE:** Overall observed count still depends on whether an older aura
  naturally expires before a later cast, and Andrew's delayed/removal proc
  model shares the changed timing commit. No timing change is imported.

## Build-18414 Data

- **FACT:** Repository searches recover DBC/DB2 structures, handler comments,
  and runtime store scans, but no extracted build-18414
  `BattlePetAbilityEffect`/`BattlePetAbilityTurn` rows joining an effect ID,
  trigger, properties, duration, target, and asserted stack behavior.
- **FACT:** Internet searches for build-18414 dumps and independent
  implementations were attempted, but the available search service returned
  HTTP 401. Andrew and Flamehawk code are lineages, not data specifications.
- **UNKNOWN:** No authenticated P3 value, especially a zero or value above one,
  can be mapped to observed 5.4.8 behavior.

`BUILD18414_AURA_STACK_DATA = INSUFFICIENT`.

## Concrete Row Tests

**UNKNOWN:** No authenticated rows were recovered. Accordingly, this report
does not invent positive, harmful, periodic, or maximum-greater-than-one
examples. Required future fixtures must record effect ID, target, trigger,
P0..P5, joined turn rows, and retail/packet result.

## Flamehawk Comparison

Flamehawk final relevant commit
`e91819a4c2acb6cc8eea09ed264d30aec6ed12b9` has an `AddAura` implementation
identical to current counting, apply-before-failure, and replacement behavior.

| Family | Flamehawk | Classification |
|---|---|---|
| Positive 26 | P2 duration, maximum 1 | `AGREES_WITH_CURRENT` (also Andrew) |
| Negative 50 | P2 duration, P3 maximum, normal hit/flags | `AGREES_WITH_CURRENT` |
| Periodic harmful 54 | P2 duration, P3 maximum, normal hit/flags | `AGREES_WITH_CURRENT` |
| Periodic positive 63 | P2 duration, default zero | `AGREES_WITH_CURRENT` |
| Toggle 76 / powerless 178 | P2 duration, default zero | `AGREES_WITH_CURRENT` |

**INFERENCE:** Exact agreement is corroborating ancestry, not independent retail
proof. Flamehawk does not validate P3, zero, cross-caster, or failure semantics.

## Other Lineages

- **FACT:** No clearly independent MoP/build-18414 implementation with concrete
  rows or tests was recovered. Later Flamehawk BattlePet replacement work is
  Draenor-derived and is not imported as MoP policy.
- **UNKNOWN:** Newer-expansion cores may encode different stack keys, data
  columns, and event contracts; they were not treated as 5.4.8 evidence.

## Generic vs Retail Invariants

Generic implementation invariants established by current source are:

- a failed application does not mutate aura ownership or state;
- for a positive established maximum within ordinary count range, successful
  insertion leaves no more than that maximum active matching auras;
- replacement expires exactly the number required, without erasing during the
  scan, and each old aura reverses its numeric contribution once;
- a successful new aura gets one unique ID among currently contained entries;
- zero coherently means no replacement bound in current source.

Retail-policy unknowns are which property/literal establishes the maximum,
whether zero in concrete data means intentional coexistence, refresh versus
replacement, cross-caster stacking, oldest-selection policy, whether failure
needs an apply event, and the removal-proc effects caused by replacement.

## Final Classification

**`NEEDS_BUILD18414_DATA`.** Exact Andrew final behavior, current zero/count
scope/replacement/failure mechanics, and interactions with Tasks 029--032 are
reconstructed. There is no independently authenticated property mapping or
stack policy. Andrew's only isolated stack change replaces P3/default zero with
literal one; the task explicitly forbids treating that alone as proof. No C++
source is changed.

Remaining A7 issues are authenticated aura-effect/turn rows, packet/gameplay
traces for duplicate positive/harmful/periodic applications (including
different casters and maximum above one), failed-apply event semantics,
delayed harmful activation, removal-proc caster identity, natural expiry
timing, boolean ownership, weather, and aura-vector mutation/reentrancy.

## Validation Plan

- **FACT:** Documentation validation consists of `git diff --check`, focused
  source/history inspection, and the static matrix below. No translation unit
  or aggregate build applies because source did not change.
- **FACT:** The static matrix found: (1) first positive inserts; (2) duplicate
  positive replaces one; (3) first hostile inserts on success; (4) failed
  hostile duplicate only emits failed apply; (5) successful hostile duplicate
  follows P3; (6--8) periodic harmful appends below P3 and replaces enough at
  or above P3; (9) zero is unbounded; (10) one leaves one; (11) two leaves two;
  (12--13) same-ability copies share a target-wide count regardless of caster;
  (14) replacement reverses the old contribution once; (15) it runs one
  removal proc per expired aura; (16) overlapping numeric modifiers retain the
  Task 029 sum; (17) infinite duration is still replaceable; (18) death expires
  victim auras, so later guarded replacement ignores expired copies; (19)
  client apply precedes replacement remove; and (20) Task 031 caster and Task
  032 natural timing remain unchanged.
- **UNKNOWN:** Runtime/build-18414 validation remains required before choosing
  P3 versus literal one or changing failed-event ordering.
