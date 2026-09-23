# Task 028: Battle-Pet Weather Final-State Review

## Scope and starting state

- **FACT:** Review started from clean task branch `work` at
  `ff101baab27a9062c4abca09b278f186fdba4a0b`. This is the merge immediately
  after Task 027's documentation commit (`b08bfd2`); the branch is a task branch
  based on current preservation integration history, rather than a checkout
  named `preservation/main`.
- **FACT:** Task 025's `PetBattle::EndBattle` guard returns when the battle is
  already `Finished`, then marks it `Finished` before finalization side effects.
  Task 026's inert trainer manager/startup load is present. The starting tree
  was clean.
- This review is restricted to A6 weather. It does not change trainers, SQL,
  Playerbots, opcodes, A7 aura behavior, or other effect families.

## Source acquisition and Andrew lineage

The Andrew repository was cloned temporarily without adding a preservation
remote. `git rev-parse`, `git show --summary`, `git show --stat`,
`git show --format=fuller`, and weather-focused full diffs establish these full
SHAs:

| Commit | Final-state relevance |
| --- | --- |
| `c440e10f49cb6d67dea66c3916f1d594fcd7f03d` | Introduces effect 80 as head-target `HandleWeatherAura`, passing trigger ability, property 2 duration, and property 3 maximum to `AddAura`; also changes aura duration initialization. |
| `100303ca22b08f04ee5620dc1622617d5a680cdd` | Candidate 11 changes effect 80 dispatch from `TARGET_HEAD` to `TARGET_ALL`. |
| `1e338126a8f3e5381218dc90a50185119beeb592` | Changes related triggered-aura maximum/stack arguments and moves failed-aura return before aura event creation. |
| `987cdb972e1fc5ce3580fd8b6d8c545c4df8fd10` | Continues harmful/periodic aura stacking changes; it does not independently settle weather. |
| `aeef0c9d8e6484278ea98d07dde4cc608b077ff4` | Maps effect 169 to caster-target `HandleWeatherAura`; later A7 ancestor `867b69b0d142efaaea10b83970342900d3405f2e` had also removed weather's hit calculation and changed expiry processing. |
| `bd8ad5515418d94abd35ba7bf71430979243a09c` | Adds the final explicit battle-global weather identity/state model, per-roster aura application, replacement, and end-battle clearing. |

No commit was applied. The final Andrew implementation was read from
`bd8ad551`, not reconstructed by combining intermediate patches.

## Flamehawk lineage

The Flamehawk repository was also cloned temporarily. The final relevant
pre-overhaul line is:

- `26e16ed4397a441bf877e2fe35c7a30bd739b5ba`;
- `0c279a48030429f9256b7587cc633d9f1c961004`; and
- `e91819a4c2acb6cc8eea09ed264d30aec6ed12b9`.

The inspected `e91819a4` state maps effect 80 to all-target
`HandleWeatherAura`, but implements replacement by scanning each dispatched
target's auras for ten hard-coded weather ability IDs. Its condition lacks
parentheses, so only ability 590 is guarded by `!HasExpired()`; the other IDs
match even when expired. It calculates hit, expires matches, and adds the
trigger aura with a zero stack/maximum argument. It has no battle-global
weather object, state-delta map, elemental exclusion, or dedicated end-battle
weather cleanup. Effect 169 is instead head-target `HandleStateDamage`; effects
170 and 171 consume weather-related states for damage/healing.

Later Flamehawk branch work replaces the folder with a Draenor-derived
implementation and is not independent 5.4.8 evidence for this review.

## Final Andrew source scope

The final A6 surface is confined conceptually to:

- `BattlePetAbilityEffect.cpp`: dispatch entries 80 (`TARGET_ALL`) and 169
  (`TARGET_CASTER`) and `HandleWeatherAura`;
- `BattlePetAbilityEffect.h`: the handler declaration;
- `PetBattle.h`: `m_weatherAbility`, `m_weatherDuration`,
  `m_weatherAbilityEffect`, `std::map<uint32, int32> m_weatherStateDeltas`,
  accessors, `ApplyWeatherStates`, and `ClearWeatherStates`;
- `PetBattle.cpp`: the two state helpers and an `EndBattle` clear; and
- `BattlePetAura.cpp`: ordinary aura state application, ticking, expiration,
  and A7-era removal-proc behavior on which final Andrew runs.

`m_weatherDuration` and `m_weatherAbilityEffect` are written but never read in
the inspected final source. There is no battle-global weather object: the
model is both scalar battle metadata/global state events and ordinary per-pet
auras.

## Current preservation behavior

**Classification: `ABSENT` (with reusable generic infrastructure).** Current
dispatch entries 80 and 169 are null/none. There is no weather identity,
duration, effect ID, delta map, replacement, or cleanup. Generic
`BattlePetAura` can attach triggered abilities, modify per-pet states, tick,
emit apply/change/remove events, and expire. `PetBattleEffect` can already
serialize aura and `SET_STATE` targets, including a null pet index; this proves
representation availability, not the weather event semantics.

Current ordinary aura state tracking is itself unsafe for nonzero pre-existing
state: `BattlePetAura::OnApply` accumulates `newValue` (base plus delta), then
`OnExpire` subtracts that accumulated value. That restores zero rather than the
prior base. Andrew final fixes this in its A7 lineage, so importing only its A6
code would run on a materially different aura contract.

## Andrew versus Flamehawk

| Effect | Andrew final | Flamehawk final relevant line | Classification |
| --- | --- | --- | --- |
| 80 | Generic dispatch is all targets, but the first handler call bypasses that target and manually applies to every roster pet except elementals; global DB2 state events are emitted. Same weather is a no-op; different weather clears global states and expires matching per-pet auras. | Generic all-target dispatch; each invocation scans only that target for ten hard-coded IDs, calculates hit, expires matches, then adds the aura. No global tracking or elemental rule. | `SIMILAR_BUT_DIFFERENT` in dispatch, `DIRECT_CONFLICT` in replacement, state, elemental, hit, and event behavior. |
| 169 | Caster dispatch enters the same handler, which then applies battle-wide weather on its first invocation. | Head-target conditional/periodic damage (`HandleStateDamage`). | `DIRECT_CONFLICT`. |

Both use triggered aura infrastructure for effect 80 and agree that property 2
participates in duration. That limited agreement does not resolve the broader
conflicts. Neither source is treated as a specification.

## Build-18414 DB2/DBC evidence

Repository-local metadata establishes only the record shapes:
`BattlePetAbilityEffectEntry` has a trigger ability, effect-property ID, and
signed properties; `BattlePetAbilityStateEntry` has ability ID, state ID, and
an unsigned value in the current structure. Stores load
`BattlePetAbilityEffect.db2` and `BattlePetAbilityState.db2`. The repository
contains no build-18414 copies or row dumps for either table.

Andrew's `research/DBC-PARSING.md` names the two files, but documents parsed
species rows rather than effect 80/169 rows. The two independent source lines
provide implementation hypotheses, not the originating records. Public search
was attempted, but no independently generated, build-tagged 18414 row dump was
obtained. Therefore target meaning, trigger ability population, exact property
meaning/sign, applicable states, and duration values remain unverified.

- **`EFFECT_80_EVIDENCE = PARTIAL`.** Independent Andrew and Flamehawk lines
  agree on weather aura, all-target dispatch, trigger ability, and property 2
  duration, but disagree on hit, replacement, maximum/stacking, state, and
  elemental semantics. No actual build-18414 row set was recovered.
- **`EFFECT_169_EVIDENCE = INSUFFICIENT`.** Andrew calls it weather while
  Flamehawk calls it state-dependent head damage. Handler reuse alone cannot
  establish weather semantics, and no build-18414 record resolves the conflict.

## Targeting and elemental exception

Andrew effect 80's generic `TARGET_ALL` execution creates multiple handler
invocations, but the first invocation ignores `m_target`, manually walks both
teams' complete `BattlePets` vectors, and records the weather. Subsequent
invocations see the same ability and do nothing. Thus actual aura targets are
all populated roster slots, including dead and inactive pets, except pets whose
`BATTLE_PET_STATE_PASSIVE_ELEMENTAL` is nonzero. Effect 169's caster mapping
also becomes battle-wide through the same bypass. Swaps need no application
hook because inactive pets were pre-populated.

The repository initializes state 47 as the elemental family passive, so the
condition is mechanically a family-passive test. No ability metadata branch,
build-18414 row, packet trace, or independent Flamehawk condition corroborates
that weather auras must be omitted for elementals. Moreover Andrew still emits
the battle-global weather state to the client. **`ELEMENTAL_EXCEPTION =
NEEDS_BUILD18414_EVIDENCE`.** `TARGET_ALL` and all-roster application likewise
remain source behavior, not retail-proven targeting.

## Weather-state delta model and correctness blocker

Andrew stores `stateId -> int32 value`, populated from the DB2 state's
repository-declared `uint32 Value`. The local map is signed, but source data is
unsigned and event serialization casts to `uint32`; negative-value intent is
therefore not proven end-to-end.

Despite its name and comment, the map is not cumulative and never applies a
delta relative to another value. Duplicate state rows overwrite the same key.
`ClearWeatherStates` ignores the stored `delta` and emits absolute zero for
every key. It therefore cannot restore a prior battle-global value, and can
erase an independent contributor. Replacement clears first and then applies
new values; distinct weather does not stack. Same-weather recasts neither
refresh duration nor reapply state. No arithmetic underflow occurs in the
helper because it writes absolute values, but information loss is possible.

**Correctness blocker:** exact reversal and coexistence with independent state
effects are not safe. The implementation does not satisfy the decision gate's
state-delta reversal requirement.

## Aura lifecycle, replacement walk, and round timing

Andrew's observable cases are:

1. **No weather -> A:** first generic handler invocation adds A to every
   non-elemental roster pet and their ordinary aura `OnApply` mutates per-pet
   states; it then emits null-pet `SET_STATE` values and records A. Further
   `TARGET_ALL` invocations are no-ops.
2. **A -> A:** complete no-op. No aura refresh, duration extension, event, or
   reapplication occurs, even if some per-pet aura expired or was dispelled.
3. **A -> B:** emits global zero states, synchronously expires every unexpired A
   aura found on all roster pets (aura-remove, removal proc, per-pet reversal),
   then applies B broadly and emits B global states. Old scalar identity is not
   reset between clear and assignment, but is overwritten afterward.
4. **Natural expiration:** each aura is processed through ordinary team aura
   processing, which iterates every roster pet despite the nearby "active"
   comment. Expiration removes that pet's aura and reverses its local
   state, but nothing clears battle-global state or `m_weatherAbility`.
   Consequently weather remains recorded forever, and a later same-weather
   cast is a no-op. Different roster pets can expire on different processing
   histories. This is a decisive lifecycle defect.
5. **Dead pet:** application includes it unless elemental. `Kill` expires its
   auras, so it loses weather locally while the global weather remains.
6. **Swap:** pre-applied inactive aura is present and team aura processing
   continues to tick every roster pet, so a swap itself does not reset or add
   weather. This still derives global duration from multiple ordinary aura
   instances rather than one battle-global clock.
7. **Battle end:** Andrew clears global states, then ordinary finalization
   expires all pet auras. Scalar weather IDs are not reset, but the battle is
   terminal. The aura objects remain owned by pet containers until ordinary
   destruction; `Expire` is guarded against double execution.

Ordinary round order is action phase, round-end procs, aura-processing begin,
all-roster aura processing, aura-processing end, then later result creation.
Final Andrew's A7 ancestor changed expiration to occur after an aura's effects
for that processing pass. Weather has no separate decrement despite its stored
duration. Flamehawk also relies on per-pet aura processing. No client trace
establishes the retail expiry boundary. **`ROUND_TIMING_EVIDENCE =
INSUFFICIENT`.** Multiple per-pet aura clocks stand in for the claimed global
model, with death/removal able to eliminate an individual instance, and this
is not safe to backport without runtime evidence.

## Packet and event contract

No packet layout or opcode change is needed merely to encode Andrew's events;
the current serializers support:

| Requirement | Finding |
| --- | --- |
| Aura apply/change/remove with ability, instance, duration, round, source and target indexes | `EXISTING_SERIALIZER_REUSED`; weather-specific order and source identity are `SEMANTICS_UNVERIFIED`. |
| Global `PET_BATTLE_EFFECT_SET_STATE` with source and target `PET_BATTLE_NULL_PET_INDEX`, effect ID zero, and unsigned value | `EXISTING_SERIALIZER_REUSED`; null-target meaning, signed values, and absolute-zero clearing `REQUIRE_BUILD18414_TRACE`. |
| Replacement ordering (state zero before old aura removals, then new aura applies, then new states) | Structurally queueable but `REQUIRES_BUILD18414_TRACE`. |
| Same-weather recast emitting nothing | `SEMANTICS_UNVERIFIED`. |
| Elemental receiving global state but no per-pet aura events | `SEMANTICS_UNVERIFIED`. |

This is an event-production uncertainty, not a serializer-layout absence.

## A7 dependency and EndBattle interaction

Final Andrew weather runs atop `867b69b0`'s A7 changes: aura effects process
before expiry, removal procs cast from the caster rather than target, and aura
state bookkeeping records only the modifier rather than base-plus-modifier.
Current preservation lacks those changes. A6 cannot safely be isolated by
silently selecting only the favorable A7 lines, and Task 028 is prohibited
from importing A7. **A7 is a required unresolved dependency.** Ordinary aura
stacking/max behavior is also different across the intermediate commits.

Task 025's exactly-once guard is compatible in placement: an adapted weather
clear inside `EndBattle`, after the guard and after the state becomes
`Finished`, would execute once. Andrew's clear is non-idempotent in event
production but a second direct call after its map is empty emits nothing.
There is no justification to weaken or change Task 025. The defect is that
weather's natural-expiry and reversal rules are wrong/unknown, not that Task
025 prevents cleanup.

## Dependency matrix

| Dependency | Grade | Reason |
| --- | --- | --- |
| A7 delayed/harmful aura lifecycle | `REQUIRED` | Final timing, removal caster, and correct local state reversal differ from preservation. |
| A8 direct effect families | `NONE` | Weather core does not require unrelated direct handlers. |
| Trainer code | `NONE` | Works in ordinary battles and has no trainer call. |
| SQL | `NONE` | Data comes from client DB2 stores. |
| Playerbots | `NONE` | No weather dependency. |
| Packet layout changes | `NONE` | Existing generic effect serializers can encode the shapes. |
| DBC/DB2 values | `REQUIRED` | Effect identity/properties, trigger abilities, and weather state rows control behavior. |
| New state fields | `REQUIRED` for Andrew's model | The four battle weather members do not exist in preservation. |
| Existing serializers | `REQUIRED` and present | Aura/state serializers exist; semantic use is unverified. |

## Decision and validation

**Final classification: `SEMANTICALLY_UNSUPPORTED`.** No source change is
authorized. Although final Andrew is fully reconstructed and generic packet
shapes exist, the gate fails independently because:

1. effect 169 directly conflicts with Flamehawk and lacks build-18414 data;
2. effect 80 targeting, elemental exclusion, event order, and recast semantics
   lack independent build-18414 evidence;
3. the purported delta map clears absolute zero and can destroy independent
   state;
4. natural expiry never clears global identity/state, making same-weather
   recasts permanently inert;
5. global duration is stored but unused while multiple independently removable
   per-pet aura instances act as its clock; and
6. final behavior depends on unbackported A7 lifecycle fixes.

This is stronger than a request for one missing row dump: the inspected final
implementation has internally unsafe lifecycle and restoration behavior. It
must not be preferred over Flamehawk merely because it is later or more
elaborate.

Only this report changed. No source build is applicable under the task's build
gate. Remaining work requires authenticated build-18414 effect/state rows and
client packet/runtime traces covering initial weather, same-weather refresh,
replacement, natural expiry, death, inactive swaps, elementals, and battle-end
event order; it also requires a separately reviewed A7 contract and a design
that restores prior global state exactly.
