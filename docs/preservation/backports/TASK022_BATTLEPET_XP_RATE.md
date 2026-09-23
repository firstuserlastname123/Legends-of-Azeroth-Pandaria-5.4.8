# Task 022 — Battle-pet XP-rate review

## Scope and starting state

Task 022 began with a clean tree at
`10d435a3e5e9eb368b958529b0bcb29fa4b0fb5e`, the post-Task-021 preservation
integration state on the checkout's local `work` branch. This review is limited
to Andrew's configurable battle-pet XP multiplier. Ability effects, auras,
wild-pet SQL, trainers, weather, packets/opcodes, and Playerbots are excluded.

## Andrew provenance and isolated hunks

- **FACT:** Source commit
  `aeef0c9d8e6484278ea98d07dde4cc608b077ff4`, parent
  `867b69b0d142efaaea10b83970342900d3405f2e`, was authored and committed by
  Andrew Downey on 2026-06-13 21:25:22 +0100. Its subject is `Add Cataclysm
  wild battle pets, implement 20+ missing ability effects, add pet XP rate`.
- **FACT:** Its complete file list is `BATTLEPETS.md`, `progress.txt`,
  `research/DBC-PARSING.md`, four Cataclysm wild-pet SQL migrations,
  `BattlePetAbilityEffect.cpp`, `BattlePetAbilityEffect.h`, `PetBattle.cpp`,
  `World.cpp`, `World.h`, and `worldserver.conf.dist`.
- **FACT:** Only four files contain the isolated rate feature:
  `src/server/game/World/World.h`, `src/server/game/World/World.cpp`,
  `src/server/game/BattlePet/PetBattle.cpp`, and
  `src/server/worldserver/worldserver.conf.dist`. The same `PetBattle.cpp`
  commit also changes `StartBattle`; that unrelated hunk is excluded.

The exact isolated change is:

```diff
+    RATE_BATTLE_PET_XP,
     MAX_RATES
```

```diff
+        { RATE_BATTLE_PET_XP, "Rate.BattlePet.XP", 1.0f, false },
```

```diff
                         }
 
+                        xp = uint16(xp * sWorld->getRate(RATE_BATTLE_PET_XP));
                         battlePet->SetXP(xp);
```

```ini
#
#    Rate.BattlePet.XP
#        Description: Experience rate for battle pet (pet battle) XP gains.
#        Default:     1
#

Rate.BattlePet.XP = 1
```

No underlying XP term is replaced. The candidate multiplies the completed
per-pet, per-battle `uint16 xp` immediately before `BattlePet::SetXP`.

## Current XP path and formula

### Eligibility

A player-owned battle pet receives XP only when all of these current
conditions hold:

1. it appears in that team's `SeenAction` set;
2. battle type is `PET_BATTLE_TYPE_PVE`;
3. its team won;
4. its level is below the level-25 `BATTLE_PET_MAX_LEVEL`; and
5. it is alive at battle end.

The current tree has wild PVE battles. Trainer support is absent, so no current
trainer-specific XP behavior can be validated. The eligibility test is generic
PVE rather than explicitly wild; a future trainer implementation using PVE
would therefore enter the same calculation unless it changes the contract.
Captured-pet creation occurs after participant XP and does not modify this XP
calculation. Forfeit health reduction applies to the losing team and likewise
does not change winner XP.

### Calculation

For each eligible pet `p`, current code starts `uint16 xp = 0` and visits every
opposing battle pet `o`. Let:

- `L_p` be the eligible pet's current level;
- `L_o` be the opposing pet's level;
- `D = clamp(L_o - L_p, -4, +2)`; and
- `N = team->SeenAction.size()`.

For each opponent it performs integer arithmetic and adds:

```text
((L_o + 9) * (D + 5)) / N
```

to the `uint16` accumulator. Division therefore truncates each opponent's
contribution before addition; the sum is not divided once at the end. No other
XP bonus or modifier is present in this path.

The exact current expression after the loop is:

```cpp
battlePet->SetXP(xp);
```

The exact candidate expression is:

```cpp
xp = uint16(xp * sWorld->getRate(RATE_BATTLE_PET_XP));
battlePet->SetXP(xp);
```

Usual arithmetic conversions promote `xp` to `float`, multiplication occurs
as `float`, and the explicit conversion truncates toward zero when the result
is finite and representable by `uint16`. Floating-to-integer conversion is not
defined by C++ when the truncated value is outside the destination type's
representable range.

### Applying and persisting XP

`BattlePet::SetXP(uint16)` returns for zero or a level-25 pet. Otherwise it
reads the current level's base and multiplier from `gtBattlePetXP.dbc`, computes
a `uint16 maxXpForLevel`, and either adds the gain or performs exactly one
level transition. On a transition it carries the remainder and calls
`SetLevel(current + 1)`. It does not loop across multiple thresholds.

`SetLevel` enforces levels 1 through 25, recalculates stats with full current
health, updates a currently summoned pet, and updates achievements.
`SetXP` marks the pet for database save. `EndBattle` then sends the existing
battle-pet update to the client. The normal Player save calls
`BattlePetMgr::SaveToDb`, which persists XP, level, health, and statistics.
Compilation cannot establish the retail XP formula or client-visible timing;
the source comment cites an external wiki rather than checked-in build-18414
evidence.

## Existing rate architecture

- **FACT:** `enum Rates` indexes the `World::rate_values` float array, and
  `getRate` returns a `float`.
- **FACT:** `World::LoadRates` uses a table of enum, configuration key,
  float default, and `PositiveOnly`. It loads each value through
  `ConfigMgr::GetFloatDefault`; active bonus-rate records may multiply it.
- **FACT:** Player kill, quest, explore, and gather XP rates use the same enum,
  float table, `Rate.XP.*` naming family, `1.0f` default, and
  `PositiveOnly = false`. Reputation, honor, drops, skills, and other settings
  use the same architecture.
- **FACT:** `reload config` calls `World::LoadConfigSettings(true)`, which calls
  `LoadRates(true)`. The candidate would therefore update without restart in
  the same way as adjacent rates. World-thread reads of the array follow the
  established project design.
- **FACT:** The candidate's enum placement, float type, configuration load,
  naming, default, documentation form, and reload behavior are structurally
  consistent with the existing system.
- **FACT:** Its `PositiveOnly = false` flag performs no validation. Despite the
  field name, the shared check for `true` rejects zero as well as negative
  values and restores the default. With `false`, zero, negative values,
  infinities if parsed, and arbitrarily large finite values reach the XP cast.
- **FACT:** Bonus-rate database multipliers can additionally enlarge or negate
  the configured value if `Rate.BattlePet.XP` is listed, so the distributed
  configuration file is not the only input affecting the effective rate.

## Default-behavior equivalence

For every possible current `uint16 xp`, conversion to `float` is exact because
IEEE-754 binary32 exactly represents all integers through 16,777,216.
Multiplication by exactly represented `1.0f` returns the same value, which is
representable by `uint16`; the explicit cast therefore returns the original
integer. The candidate does not move the participant division, opponent sum,
level-cap check, or `SetXP` call.

Consequently default `1.0` is exactly behavior-neutral for zero, one, odd
values, non-divisible participant results, and every other value that can
already inhabit `xp`. This conclusion depends on the effective rate remaining
exactly 1.0; an active bonus-rate multiplier can intentionally change it.

## Zero, fractional, normal, and large-rate semantics

For representative completed base gains, the candidate's cast gives:

| base XP | 0.0 | 0.5 | 1.0 | 2.0 | 10.0 |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 0 | 0 | 0 | 0 | 0 | 0 |
| 1 | 0 | 0 | 1 | 2 | 10 |
| 3 (odd) | 0 | 1 | 3 | 6 | 30 |
| 101 (mid-range) | 0 | 50 | 101 | 202 | 1010 |
| 714 (upper bound with three level-25 opponents, one participant) | 0 | 357 | 714 | 1428 | 7140 |

- **FACT:** Rate 0 is arithmetically safe and suppresses XP because `SetXP(0)`
  returns immediately.
- **FACT:** Positive fractional results truncate toward zero. For example,
  `1 * 0.5` becomes zero and `3 * 0.5` becomes one.
- **FACT:** Under current level bounds and a maximum three-pet opposing team,
  each contribution is at most `(25 + 9) * (2 + 5) = 238`; the ordinary base
  gain is at most 714 and rates through 10 produce a representable uint16.
- **FACT:** A gain large enough to cross multiple DBC thresholds is not fully
  normalized: `SetXP` processes only one level-up and can leave XP exceeding
  the next level's threshold. A rate of 10 can exercise that pre-existing
  single-transition assumption for sufficiently low thresholds.
- **FACT:** The candidate declares no supported maximum. Any effective rate
  above approximately `65535 / xp` makes the float-to-`uint16` conversion out
  of range; C++ does not define that conversion. NaN/infinity, if accepted by
  the parser or produced by multiplication, have the same issue.
- **FACT:** Negative effective rates also produce an out-of-range conversion
  to unsigned `uint16`, with undefined C++ behavior. The candidate explicitly
  permits negative values by selecting `PositiveOnly = false`.
- **INFERENCE:** The setting would require a deliberately specified validation
  policy and upper bound, plus a decision about multi-level gains, before its
  non-default range can be called safe. Inventing those semantics would no
  longer be the isolated Andrew feature and is outside this task.

## History and preservation assessment

- **FACT:** Searches of all current refs and source/config history found no
  prior `RATE_BATTLE_PET_XP`, `Rate.BattlePet.XP`, removed equivalent key, or
  TODO requesting this setting. Current preservation contains no analogous
  battle-pet rate.
- **FACT:** Andrew retains the rate line through the fetched final
  `feature/pandaria-fixes` tip. That establishes Andrew's intent, not safety or
  preservation correctness.
- **FACT:** No local evidence identifies a required MoP mechanic that a
  configurable server multiplier repairs. The underlying formula is unchanged.
- **INFERENCE:** The administrative rationale is recognizable—operators often
  configure progression rates—and the feature fits the broad World rate
  architecture. Nevertheless, architectural familiarity is insufficient when
  the proposed configuration contract accepts values that make its arithmetic
  undefined or violate the single-level transition assumption.
- **FACT:** External corroboration was not needed. Presence in another core
  could show popularity but could not resolve the candidate's local arithmetic
  and `SetXP` behavior.

## Decision

**Final classification: `UNSAFE_CONFIGURATION_CHANGE`.** The default value is
proven exactly behavior-neutral and the integration shape is conventional, but
the proposal fails the Task 022 implementation gate that arithmetic remain
safe for accepted configuration values. It accepts negative and unbounded
float inputs before an unchecked conversion to `uint16`, and high gains can
outrun `SetXP`'s one-level-only transition logic.

No source or configuration file is modified. Only this documentation
checkpoint is committed. A future proposal would need an evidence-backed
effective-rate domain, explicit finite/nonnegative/upper-bound validation, and
a decision on whether large gains are rejected or whether multi-level XP logic
is separately repaired. Those are policy and gameplay changes, not safe
adaptations to synthesize in this evidence-gated review.

## Validation status and future runtime procedure

Because the decision gate failed, no C++ source changed and the conditional
`game`/`worldserver` builds were not applicable. Static review and the numeric
table above are the validation for this documentation-only result.

If a later bounded design is approved, runtime validation must compare rate
1.0 against baseline; test 0.0, 0.5, and 2.0; cover multiple participants,
pets near a level-up, the level-25 cap, wild battles, logout/login persistence,
server-restart persistence, and client XP/level display. Trainer tests remain
deferred until trainer support independently exists.
