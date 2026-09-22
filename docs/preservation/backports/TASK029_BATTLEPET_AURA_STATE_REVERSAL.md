# Task 029: Battle-Pet Aura State Reversal

## Continuation State

- **FACT:** Task 029 continued on clean branch `work` at
  `f18068a258d0da5b0b1e482ac52a8f182c407bc7`, the Task 028 documentation
  commit. Task 025's early `Finished` guard/state transition and Task 026's
  inert trainer substrate remain present.
- **FACT:** `/tmp/mop-preservation-dev/CMakeCache.txt` was absent at entry and
  ccache contained 0.0 GiB of its 5.0 GiB capacity. No tree was configured
  during review.
- Scope is only A7A private aura state bookkeeping. Weather, effect dispatch,
  delayed activation, stacking policy, removal-proc identity, expiry timing,
  trainers, SQL, Playerbots, and opcodes are excluded.

## Andrew A7 Provenance

- **FACT:** `c440e10f49cb6d67dea66c3916f1d594fcd7f03d` is the exact
  commit that changes `m_auraStates[stateId] += newValue` to
  `m_auraStates[stateId] += stateEntry->Value`.
- **FACT:** `987cdb972e1fc5ce3580fd8b6d8c545c4df8fd10` changes harmful
  aura maximum/stack arguments but does not touch `BattlePetAura.cpp`.
- **FACT:** `867b69b0d142efaaea10b83970342900d3405f2e` moves duration
  expiry after effect processing and changes removal-proc caster identity; it
  does not change the modifier-only bookkeeping.
- **FACT:** No later commit through
  `bd8ad5515418d94abd35ba7bf71430979243a09c` changes that bookkeeping line.
  The one-line hunk predates and is mechanically independent from Andrew's
  weather handler and later A7 changes.

## Current OnApply Formula

For every `BattlePetAbilityState` row whose `AbilityId` equals the aura's
ability, current code evaluates:

```text
B = target.States[stateId]
M = stateEntry.Value
N = B + M
R[stateId] += N
UpdatePetState(..., stateId, N)
```

- **FACT:** `UpdatePetState` validates the state and modification type, clamps
  boolean states to zero/one, skips an unchanged value, writes the resulting
  absolute value into `target->States`, and queues an absolute `SET_STATE`
  value.
- **FACT:** Multiple rows for one aura/state repeat against the value written
  by the preceding row and accumulate each post-application total in `R`.
- **INFERENCE:** For one un-clamped row, recorded `R = B + M`, although the
  aura owns only contribution `M`.

## Current OnExpire Formula

For each private map entry current code evaluates:

```text
C = target.States[stateId]       // value at expiry
E = C - R[stateId]
UpdatePetState(..., stateId, E)
```

`OnExpire` then clears the private map. `Expire` is guarded by `m_expired`, but
`EndBattle` calls `OnExpire` directly once under Task 025's finalization guard.
With one row and no intervening changes, `E = (B + M) - (B + M) = 0`, not `B`.

## Symbolic Reversal Tests

These calculations use the current arithmetic result before any boolean clamp:

| Initial `B` | Modifier `M` | Applied `N` / recorded `R` | Current expiry `N-R` | Exact restore? |
| ---: | ---: | ---: | ---: | --- |
| 0 | +5 | 5 / 5 | 0 | yes |
| 10 | +5 | 15 / 15 | 0 | **no; expected 10** |
| 10 | -5 | 5 / 5 | 0 | **no; expected 10** |
| 5 | -10 | -5 / -5 | 0 | **no; expected 5** |

- **FACT:** The defect follows algebraically whenever a nonzero base exists.
- **FACT:** Modifier-only recording instead gives expiry `(B + M) - M = B`.
- **UNKNOWN:** Boolean clamping can make the actually applied contribution
  differ from raw `M`; this pre-existing broader boolean/stacking concern is
  not introduced by changing a post-total to a modifier and remains A7B scope.

## Supported Nonzero-State Paths

- **FACT:** Every initialized pet has critical-chance state 5, pet-family ID,
  and one family-passive state 1. Breed and species DB2 rows add further state
  values. Current battles therefore support nonzero pre-existing state in the
  same `States` array used by auras.
- **FACT:** `PetBattle::Cast` also adds `BattlePetAbilityState` values directly
  to current caster states, and generic effect handlers can set states through
  `UpdatePetState`.
- **FACT:** `AddAura` permits separate aura objects and calls `OnApply` for each
  successful instance. Its `maxAllowed` logic may expire older instances but
  does not establish that state IDs are globally unique between abilities.
- **SUPPORTED_BY_API_CONTRACT:** An aura row may address any valid state ID;
  neither `OnApply` nor DB2 structures prohibit a state already being nonzero
  or another aura owning a contribution to the same state. Exact build-18414
  row overlap is unnecessary to preserve this generic additive API invariant.

## Multiple-Aura Interaction

Let base be `B`; Aura A contribute `X`; Aura B contribute `Y`.

Current bookkeeping records `R_A=B+X` and, after A, `R_B=B+X+Y`:

- A then B expiry: after A, current becomes `Y`; after B it becomes
  `Y-(B+X+Y) = -B-X`.
- B then A expiry: after B, current becomes `0`; after A it becomes
  `-(B+X)`.

Modifier-only recording stores `X` and `Y`:

- A then B expiry: `B+X+Y-X-Y = B`.
- B then A expiry: `B+X+Y-Y-X = B`.

Thus delta reversal is order-independent and each instance removes only its
own additive contribution. This does not select or redesign whether duplicate
abilities should stack; it makes any independently existing instance safe.

## Signed / Unsigned Semantics

- **FACT:** pet states and the aura map value are `int32`; `UpdatePetState`
  accepts `int32`. `BattlePetAbilityEffectEntry::Properties` are `int32`.
- **FACT:** `BattlePetAbilityStateEntry::Value` is declared `uint32`, although
  its DB2 format character is signed `i`. Addition with an `int32` state and
  compound addition into an `int32` map therefore cross unsigned/signed types.
- **FACT:** the `SET_STATE` target payload and `PetBattleEffect::UpdateState`
  value are `uint32`; the internal absolute signed result is converted for wire
  storage. The proposed bookkeeping-only change does not alter either event
  conversion or the value sent to `UpdatePetState`.
- **INFERENCE:** Negative DB2 bit patterns are already dependent on the
  implementation's integer conversion behavior in both current and Andrew
  expressions. Recording `Value` rather than `newValue` neither adds a cast nor
  creates a new underflow/overflow path. Correcting the DB2 declaration or
  defining saturation/floors would be a separate data-contract change.

## Client Event Semantics

- **FACT:** On apply, the client-visible event is produced by
  `UpdatePetState(..., newValue)`; on expiry it is produced by
  `UpdatePetState(..., current - recorded)`. These are absolute resulting state
  values, not deltas.
- **FACT:** `m_auraStates` is private reversal data and is never serialized.
  Changing only what it records leaves first-application state and event values
  unchanged, while making the expiry event contain the corrected absolute
  remaining state.
- **FACT:** Existing `PET_BATTLE_EFFECT_SET_STATE` serialization and opcode
  layouts need no change.

## Expiry / Removal Paths

- **FACT:** duration processing calls `Expire`; special Prowl/next-attack
  processing calls `Expire`; toggle/manual ability removal calls `Expire`; aura
  count replacement calls `Expire`; and pet death calls `Expire` on every aura.
  All reach `OnExpire` once because `Expire` guards `m_expired`.
- **FACT:** `EndBattle` calls `OnExpire` directly for all auras, clears their
  containers, then resets remaining mechanic states. Task 025 ensures this
  finalization block executes once.
- **FACT:** expired aura pointers are erased without deletion by
  `RemoveExpiredAuras`, while ordinary pet cleanup owns deletion; erasure and
  destruction do not perform a second state reversal.
- **INFERENCE:** Every legitimate state-removal route uses the same private map,
  so delta storage fixes all routes without changing their timing or ownership.

## Andrew Final Comparison

Andrew stores `stateEntry->Value` in the per-state map immediately before
calling the unchanged `UpdatePetState(..., newValue)`. `OnExpire` remains
`current - stored`. This is the exact delta-reversal invariant. The hunk has no
dependency on weather, delayed triggers, removal-proc caster selection, or
effect mappings. **Classification: `INDEPENDENT_CORE_FIX`.**

## Flamehawk Comparison

Flamehawk `e91819a4c2acb6cc8eea09ed264d30aec6ed12b9` retains
`m_auraStates[stateId] += newValue` and therefore agrees with current behavior,
not Andrew. **Classification: `AGREES_WITH_CURRENT`.** Its independent presence
documents lineage, but the symbolic invariant disproves that behavior; it is
not correctness evidence.

## Bookkeeping Consumers

Repository-wide search finds exactly these consumers:

1. the typedef/member declaration in `BattlePetAura.h`;
2. initialization/accumulation in `BattlePetAura::OnApply`;
3. iteration/subtraction in `BattlePetAura::OnExpire`; and
4. clearing at the end of `OnExpire`.

No external caller reads or writes the map and no serializer observes it.
Changing the stored meaning from post-application total to owned modifier does
not invalidate another consumer or require a header/type change.

## Required Invariant

The architecture is additive, so **delta reversal** is required:

```text
apply:  state = state + aura-owned modifier
expire: state = current state - aura-owned modifier
```

Capturing/restoring an original absolute value would erase changes made after
application and would make overlapping auras expiration-order dependent.
Modifier accumulation also correctly handles multiple DB2 rows for the same
aura/state, subject to the separately deferred boolean-clamping concern.

## Final Classification

**`STATICALLY_JUSTIFIED_CORE_FIX`.** Current bookkeeping provably subtracts the
base along with the aura contribution; nonzero states and independent aura
objects are supported; delta bookkeeping is order-independent; the map has no
other consumer; first-application and wire values are unchanged; every removal
path uses the map; and no weather, delayed-trigger, handler, packet, opcode, or
stacking-policy change is required.

Fixing A7A does **not** approve Andrew weather. Task 028's effect-80/effect-169,
global-state, elemental, timing, replacement, packet, and runtime blockers
remain.

## Validation Plan

After the documentation checkpoint, adapt only Andrew's one-line bookkeeping
hunk in `BattlePetAura.cpp`. Review the diff and Task 025/026/028 boundaries;
configure the prescribed fast tree once because it is absent; build `game` and
`worldserver`; repeat the symbolic matrix for positive/negative, overlapping,
death, end-battle, explicit, already-expired, and event cases; update this
report; and commit the isolated implementation separately.

## Implementation and Validation Result

- **FACT:** Source changed only in `BattlePetAura::OnApply`: the private map now
  accumulates `stateEntry->Value` rather than `newValue`. This is an exact
  adaptation of Andrew's `c440e10f` one-line hunk. No header, weather, effect
  handler, removal-proc, timing, stacking, trainer, SQL, Playerbots, opcode, or
  packet code changed.
- **FACT:** The prescribed fast profile was configured once in the previously
  absent `/tmp/mop-preservation-dev` tree and retained. The affected
  `BattlePetAura.cpp` object compiled successfully with that tree's generated
  game-target flags and PCH.
- **ENVIRONMENT LIMITATION:** Repeated exact
  `cmake --build /tmp/mop-preservation-dev --target game --parallel 4` runs
  advanced the cold build and preserved completed objects, but this execution
  environment ended each long-running tool session at approximately five
  minutes before the cold `game` target completed. No compiler error was
  reported. Consequently `game` did not reach its final archive and
  `worldserver` was not started; these remain required broader checks.
- **FACT:** Static post-change evaluation passes for base zero/positive,
  nonzero base/positive, nonzero base/negative, and both two-aura expiration
  orders. Death, end-battle, explicit/toggle, duration, and replacement paths
  all subtract the stored owned delta. Already-expired `Expire` calls are
  guarded; Task 025 makes direct end-battle reversal exactly once. Apply events
  remain the same absolute `newValue`; expiry events now carry the correct
  absolute remaining value.
- **FACT:** The final classification remains
  `STATICALLY_JUSTIFIED_CORE_FIX`. The incomplete cold aggregate build is an
  environment validation limitation, not new semantic evidence against the
  one-line invariant repair.
- **FACT:** Fixing A7A does **not** approve Andrew weather. Task 028 remains
  `SEMANTICALLY_UNSUPPORTED`. A7B still contains delayed harmful aura behavior,
  stacking policy, removal-proc caster identity, expiration timing, boolean
  clamping/contribution accounting, and associated runtime/client evidence.
