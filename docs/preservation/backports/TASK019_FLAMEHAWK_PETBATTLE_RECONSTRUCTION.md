# Task 019: Flamehawk Battle-Pet Final-State Reconstruction

## Starting State

**FACT:** This analysis started with a clean working tree at
`10cfca37b95e982b2602c6becb6d5ab66febabc9`, the merge of Task 018 into the
current preservation integration history. The hosted task branch is named
`work`; as in earlier preservation tasks, that name is the task checkout rather
than evidence that the integration history is stale. No source, SQL, CMake, or
packet file was changed by this task.

**FACT:** The required preservation workflow, build workflow, architecture,
fork-archaeology, candidate-review, and Task 018 evidence-gate documents were
read before inspecting the external history. This is analysis only: no commit
from a fork was applied and no build or server was run.

## Fork and Candidate 7 Provenance

**FACT:** `https://github.com/Flamehawk147/LOACore_548` was cloned under
`/tmp`; no repository remote was added or changed. Candidate 7 resolves exactly
to `e91819a4c2acb6cc8eea09ed264d30aec6ed12b9`, parent
`0f46d20cce0e6b30957e9b062d1d88daf0ad4371`, authored and committed by
Flamehawk147 on 2024-11-15 20:11:38 -0500, subject `Update more PB`.

**FACT:** Candidate 7 changes exactly these four files:

- `src/server/game/BattlePet/BattlePetAbilityEffect.cpp`;
- `src/server/game/BattlePet/BattlePetAbilityEffect.h`;
- `src/server/game/BattlePet/PetBattle.cpp`;
- `src/server/game/BattlePet/PetBattle.h`.

It is not standalone. Its dispatch table invokes handlers introduced earlier,
its battle changes edit code repeatedly changed earlier in the series, and its
final `PetBattle.cpp` includes both live and commented-out remnants of earlier
swap/assistance experiments.

**FACT:** The complete effective final-state file set contributed by the seven
battle-pet commits is the four Candidate 7 files plus `BattlePet.cpp`,
`BattlePet.h`, and `src/server/game/Handlers/BattlePetHandler.cpp`. Candidate 7
does not modify the latter three, but their earlier changes survive at its tip.

## Commit Timeline

The first-parent path was inspected with full metadata, stats, summaries, and
patches. Seven commits on that path actually modify battle-pet implementation;
Candidate 6 was also reviewed and is recorded separately because it modifies no
battle-pet file or prerequisite API.

1. **`26e16ed4397a441bf877e2fe35c7a30bd739b5ba`**, parent
   `994f173d0b5faf272766d7e946549078465e27fe`, 2024-11-05 11:38:55 -0500,
   Flamehawk147, `Battle pet updates`.
   **FACT:** Changes `BattlePet.h`, both ability-effect files, and both
   `PetBattle` files (287 insertions/17 deletions). It adds state IDs 71/72,
   the write-only `BattlePet::isImmortal`, the pet-family enum and
   `m_immortalOneRound`; modifies hit, heal, damage, stun/passive-family and
   targeting logic; adds the first large handler set (effects 44, 52, 61, 62,
   65, 68, 80, 85, 111, 164, 170, 171, 172, and 197); clears damage modifier
   states at finish; and repeats player unroot/unpacify after finish packets.
   Later commits alter dispatch and handlers but do not integrate the orphan
   fields.
2. **`0c279a48030429f9256b7587cc633d9f1c961004`**, parent
   `ec27b519b5d9986d21af25e0a7995bdf91adbf98`, 2024-11-07 17:06:20 -0500,
   Flamehawk147, `PB update`.
   **FACT:** Changes the two ability-effect files and unrelated Elwynn content.
   It adds missing-effect logging, target-team expansion, aura proc traversal,
   and handlers for effects 28, 29, 49, 53, 59, 66, 67, 72, 74, 78, 79, 86,
   135, 136, 140, 141, and 158. It also changes effect targets/dispatch and
   adds critter control immunity. Later commits remap several entries and add
   the last handler group. The Elwynn script is unrelated.
3. **`e095c4940114344f68c3eda70ed3662b79158030`**, parent
   `0c279a48030429f9256b7587cc633d9f1c961004`, 2024-11-07 18:47:22 -0500,
   Flamehawk147, `Update PetBattle.cpp`.
   **FACT:** Adds a 35-percent nearby-wild-pet assist experiment in the PvE
   constructor. Candidate 7 leaves the whole block commented out and replaces
   the helper call inside it; the final state has no live assist behavior.
4. **`aec3e154069808e7ec62d89a273fc4841641a80c`**, parent Candidate 6,
   2024-11-10 17:24:37 -0500, Flamehawk147, `BP update`.
   **FACT:** Changes `BattlePet.cpp`, `BattlePetAbilityEffect.cpp`,
   `PetBattle.cpp`, and `BattlePetHandler.cpp`. It adds random treat bonus XP
   under aura 142205, an ability-349 accuracy special case, start-time health
   repair/death state setting, and handler diagnostics. The commit itself calls
   the work a `hackfix` and says the system needs a rewrite. All live changes
   survive Candidate 7; their semantics remain unproved.
5. **`61e1f477b3783b7b82364151bc6d86ec65cad8bd`**, parent `aec3e154...`,
   2024-11-10 17:49:27 -0500, Flamehawk147, `Update PetBattle.cpp`.
   **FACT:** Adds only empty `Passive: Humanoid` and `Passive: Dragonkin`
   comments after round-end casts. It has no runtime behavior and is exactly
   present only as comments in the final fork.
6. **`0f46d20cce0e6b30957e9b062d1d88daf0ad4371`**, parent `61e1f477...`,
   2024-11-12 10:19:22 -0500, Flamehawk147, `Update PB again`.
   **FACT:** Changes `PetBattle.cpp` and `BattlePetHandler.cpp`. Most proposed
   `TurnFinished` dead-pet code and handler repair are committed inside block
   comments. The live `Kill` change makes catch or active-pet death produce
   `CATCH_OR_KILL`, while a non-active death produces `NORMAL`; the handler now
   logs and returns rather than falsely marking a living active pet dead.
7. **`e91819a4c2acb6cc8eea09ed264d30aec6ed12b9`**, parent `0f46d20c...`,
   2024-11-15 20:11:38 -0500, Flamehawk147, `Update more PB`.
   **FACT:** Adds/remaps effects 22, 121, 128, 136, and 137; adds dummy/health
   consume/set-health-percent/cleansing/aura-state handlers and a declared NYI
   lock-active-ability handler; changes hit diagnostics; adds an assist helper;
   and greatly expands commented-out PvE selection/swap experiments. Its live
   PvE `TurnFinished` behavior remains the baseline random usable-ability or
   pass logic. The assist constructor block remains disabled.

**INFERENCE:** `e095c494` must be counted even though Candidate 7 ultimately
comments its behavior out: it explains the final dead code and the later helper.
Conversely, comment-only `61e1f477` is relevant history but not a backportable
behavior.

## Candidate 6 Battle-Pet Decomposition

**FACT:** Candidate 6 is
`2089a5470f9b981854e016ec8e5c2f81a5436712`, parent
`e095c4940114344f68c3eda70ed3662b79158030`, authored/committed 2024-11-10
14:54:35 -0500 by Flamehawk147, subject `Fixes`. Its 25-file patch contains LFG,
Creature, Player, Unit, custom Elwynn/NPC-bot, scenario, Timeless Isle, and
encounter work. It changes no file under `src/server/game/BattlePet`, no
`BattlePetHandler` file, and no API used by the final Flamehawk battle-pet diff.

**FACT:** The supposed Candidate 6 “battle-pet portion” decomposes to **zero
hunks**. Its only battle-pet-adjacent existing context is the unchanged
`Player::Regenerate` revive-pets cooldown code. The added `_hasBot`, `bot_Class`,
random creature damage overrides, LFG changes, economy caps, donation/NPC-bot
logic, and content scripts are not prerequisites.

**INFERENCE:** The earlier candidate inventory likely inferred a pet-battle
role from chronological placement or mixed subject matter. The actual fork
history is authoritative. The relevant pet-battle series crosses Candidate 6
by ancestry but has no dependency on its patch, so its pet unit is independently
reconstructable by synthesis rather than range cherry-picking.

## Functional Families

1. **Effect dispatch and targeting** — introduced in `26e16ed4`, greatly
   expanded in `0c279a48`, finalized in `e91819a4`; both ability-effect files.
   It depends on build-18414 effect-property rows and target semantics.
2. **Damage/heal/result computation and family passives** — `26e16ed4` plus
   `0c279a48` and the ability-349 special case in `aec3e154`; includes pet-type
   lookup, untargetable/cocoon misses, aquatic/magic damage caps, aura procs,
   and numerous percentage/state formulas. It depends on dispatch, state IDs,
   flags, and DBC properties.
3. **Aura, weather, cleanse, and state mutation** — `26e16ed4`, `0c279a48`,
   `e91819a4`; creates/removes/resets auras and state values. It depends on aura
   lifetime and event encoding; several precedence, targeting, and bounds
   questions remain.
4. **Health, death, resurrection, and round-result selection** — health
   handlers begin in `26e16ed4`/`0c279a48`, start-state repair in `aec3e154`,
   live active-versus-background death classification in `0f46d20c`, and health
   consume/set-percent in Candidate 7. It depends on `Kill`, `SetHealth`, and
   final-round client behavior.
5. **PvE move selection, swapping, and multi-wild assistance** — experiments
   in `e095c494`, `0f46d20c`, and Candidate 7. Most final code is commented out;
   only the baseline random AI remains live, plus an unused assist helper. This
   family is incomplete rather than an implementation unit.
6. **Battle start/end cleanup and client release** — `26e16ed4` and
   `aec3e154`; state clearing, start health repair, and duplicate post-packet
   player release. It touches persistent pet health/XP and packet ordering.
7. **XP bonus** — `aec3e154`; aura 142205 adds a random flat 8–20 XP and logs.
   It is narrow in code but data- and retail-semantics-dependent.

## Current Preservation Comparison

**FACT:** Direct final-file comparison and call-site tracing found no imported
Flamehawk series. Patch identity is not meaningful for this evolving mixed
series; current files retain the pre-series baseline. Classification by unit:

| Logical unit | Status | Exact current-versus-fork behavior |
| --- | --- | --- |
| Existing handlers/effect serializer | `PRESENT_EXACTLY` where untouched | Baseline effects and `PetBattleEffect` wire structures match; the new handler mappings do not. |
| Added handler/dispatch set | `ABSENT` | Current maps only effects 23, 24, 25, 26, 27, 31, 32, 50, 54, 63, 76, 96, 103, 104, 149, and 178 to live handlers. |
| Changed target selection and computation | `CONFLICTING_IMPLEMENTATION` | Flamehawk remaps targets and changes formulas/procs; current retains baseline behavior. |
| State IDs 71/72 and orphan fields | `ABSENT` | Current has neither IDs nor fields; Flamehawk does not complete their integration. |
| Treat aura XP | `ABSENT` | Current `SetXP` has no aura-142205 random bonus. |
| Start health/death repair | `ABSENT` | Current does not force zero-health pets to 1 or rewrite death state at start. |
| Death round result | `PARTIALLY_PRESENT` | Current always sets `CATCH_OR_KILL`; Flamehawk distinguishes inactive death. Common kill/event logic remains. |
| Living-pet front swap guard | `PARTIALLY_PRESENT` | Both return, but current first marks the living active pet dead and may queue a move; Flamehawk comments that mutation out and only logs/returns. |
| Multi-wild assistance | `SUPERSEDED_BY_CURRENT` only as non-behavior | Current has the same TODO and no assistance. Flamehawk's final constructor experiment is commented out; its unused helper is merely dead API. |
| End damage-state reset and post-packet release | `PARTIALLY_PRESENT` | Current already resets mechanics and releases the player in `EndBattle`; Flamehawk additionally zeroes two states and repeats release in both send routines. |
| Packet serializers | `PRESENT_EXACTLY` | Final/finished/round serialization layout is unchanged by the series; only state feeding it and post-send actions differ. |
| Comment-only passive placeholders | `ABSENT`, no semantic delta | They implement nothing. |

**UNKNOWN:** No runtime trace establishes that any conflicting implementation
is retail-correct. “More handlers” is not evidence of equivalence or accuracy.

## Data Structures and State

- **FACT:** `BATTLE_PET_STATE_ADD_FLATDAMAGETAKEN = 71` and
  `...ADD_FLATDAMAGEDEALT = 72` are added to `BattlePetStates`, explicitly
  marked NYI, and have no reader or writer elsewhere at Candidate 7. They are
  partially integrated declarations only.
- **FACT:** public `uint32 BattlePet::isImmortal = 0` is initialized in-class
  but never read or written. It has no persistence or packet path.
- **FACT:** private `bool BattlePetAbilityEffect::m_immortalOneRound = true` is
  initialized per effect object but never read or written. It is dead state.
- **FACT:** `PetBattlePetTypes` duplicates the ten family ordinal values for
  local calculation. `GetPetType()` reads ability-turn then ability DBC stores;
  there is no owned lifetime and no persistence.
- **FACT:** `AddWildBattlePetAssist` borrows the spawn manager's `BattlePet*`,
  assigns team/global indices, and appends it. The sole prospective caller is
  commented out. It performs no uniqueness, spawn engagement, active-pet, or
  rollback handling.
- **FACT:** no persistent schema or serialized `BattlePet` record layout is
  changed. The treat change alters XP subsequently saved through existing
  manager paths; the start repair and end health changes likewise affect
  existing persistent health updates.
- **FACT:** per-round condition states are reset before casts; round-start
  procs, ordinary casts, round-end procs, aura processing, stat effects,
  round-result send, `TurnFinished`, round increment, effect clear, and
  all-dead termination retain baseline order. The empty passive comments do
  not change that order.

## Ability Effect Dispatch

**FACT:** Relative to current preservation, the final fork activates effects
22, 28, 29, 44, 49, 52, 53, 59, 61, 62, 65–68, 72, 74–75, 77–80, 85–86, 111,
121, 128, 135–137, 140–141, 158, 164, 169–172, and 197. It also enables target
type 7 (`TARGET_TEAM`) and modifies target resolution. Effect 136 is ultimately
`Cleansing`; effect 22 is ultimately a no-op dummy. `HandleLockActiveAbility`
is declared and empty but is not dispatched.

Handler-family evidence classifications are deliberately conservative:

| Handler behavior | Inputs/mutation/result | Evidence |
| --- | --- | --- |
| Dummy (22), stop-chain (158), declared lock | no-op or local chain flag | `SUSPICIOUS_OR_INCOMPLETE` |
| Percent/state damage, ruthless/witching/vengeance, split/multistrike (29, 59, 62, 65–68, 141, 164, 169, 197) | DBC properties, health/state, damage flags; target/team variants | `NEEDS_DBC_EVIDENCE`, `NEEDS_RUNTIME_EVIDENCE`; split targeting also `NEEDS_PACKET_EVIDENCE` |
| Heal variants/equalize/last-hit (44, 53, 61, 67, 72, 78, 171) | DBC properties, cached last hit, aura/state tests, health effects | `NEEDS_DBC_EVIDENCE`, `NEEDS_RUNTIME_EVIDENCE` |
| Aura/control/remove/reset/cleanse (28, 49, 52, 74–80, 85–86, 136–137, 140, 172) | iterates aura vectors, expires/adds aura, modifies states and event flags | `NEEDS_DBC_EVIDENCE`, `NEEDS_PACKET_EVIDENCE`, `NEEDS_RUNTIME_EVIDENCE` |
| Resurrection, consume, set percent, kill (111, 121, 128, 135) | direct health/death mutation; special-cases ability 758 | `NEEDS_DBC_EVIDENCE`, `NEEDS_PACKET_EVIDENCE`, `NEEDS_RUNTIME_EVIDENCE` |
| Pet family/passive caps and effect-349 accuracy | hard-coded ability/aura IDs and family rules | `SUSPICIOUS_OR_INCOMPLETE` pending build-18414 data/runtime proof |

**FACT:** Added handlers generally call existing `CalculateHit`, `Damage`,
`Heal`, `SetHealth`, `AddAura`, `UpdatePetState`, or aura expiration APIs, so
their C++ call shapes are mostly statically recognizable. That is not enough to
mark them `STATICALLY_SELF_CONSISTENT`: unchecked property indices as state
indices, hard-coded ability/aura IDs, ambiguous boolean precedence in weather
aura filtering, target/caster inconsistencies, miss-flag tests, and empty NYI
paths make the aggregate unsafe.

**UNKNOWN:** Exact build-18414 property meanings, signs, units, target rows,
trigger abilities, family modifiers, and chain-failure rules were not available
as checked-in extracted data. An enum/comment match cannot establish them.

## Round / Death / Swap / Finish Semantics

**FACT:** Start initializes team active pets as before, then Flamehawk repairs
each pet: positive health clears `IS_DEAD`; non-positive health is forced to 1
and marked dead. This can mutate a persistent pet immediately and is a hackfix,
not a proven invariant.

**FACT:** Move selection/execution order is unchanged in live final code.
Voluntary swap uses `CanSwap`; forced dead swap uses `CanSwap(..., true)`.
Candidate 7 contains two extensive but disabled alternatives for NPC forced
swap/random casts. The active AI still chooses a random castable ability or
queues swap-or-pass. The unused assist helper and disabled nearby-pet scan do
not create a multi-pet wild battle.

**FACT:** On death, Flamehawk emits the normal death state effect. If no pet was
caged, only death of the active victim selects `CATCH_OR_KILL`; background-pet
death selects `NORMAL`. A catch still selects `CATCH_OR_KILL`. Current always
selects `CATCH_OR_KILL`. This directly changes the round-result value visible to
the client.

**FACT:** At the end of each round, effects are sent before `TurnFinished`, the
round counter increments afterward, and all-dead detection calls `EndBattle`.
No new humanoid/dragonkin passive runs despite comments claiming placeholders.

**FACT:** `EndBattle` still handles forced interruption off the world thread,
winner selection, aura cleanup, mechanic reset, forfeit health loss, PvE XP,
captured-pet creation, achievement win/loss, player release, finish packet
choice, and wild-spawn release. Player versus player follows these common team
paths; the series adds no separate PvP termination implementation. NPC/wild
termination still calls `LeftBattle` for the wild creature.

**FACT:** With a winner, `SMSG_PET_BATTLE_FINAL_ROUND` is sent; a force-ended
battle sends `SMSG_PET_BATTLE_FINISHED`. Candidate 26 additionally releases the
player after these sends, even though `EndBattle` already releases the player
before calling them. This does not change bytes but changes/repeats state-change
ordering around the send.

## Packet / Client Dependencies

| Contract | Classification | Finding |
| --- | --- | --- |
| `SMSG_PET_BATTLE_ROUND_RESULT` and `PetBattleEffect` target/event records | `CURRENT_API_COMPATIBLE` structurally; `REQUIRES_BUILD18414_TRACE` semantically | No serializer layout edit. New health/state/aura/death effects and `m_roundResult` choices change payload values/order. |
| `SMSG_PET_BATTLE_FINAL_ROUND` | `POSSIBLE_LAYOUT_DEPENDENCY` | Layout is unchanged, including the existing `Abandoned` hack bit; altered health, XP, seen-action, capture, and winner state feed it. |
| `SMSG_PET_BATTLE_FINISHED` | `CURRENT_API_COMPATIBLE` | Empty payload unchanged; repeated unroot/unpacify occurs after send. |
| `SMSG_PET_BATTLE_INITIAL_UPDATE` / `FIRST_ROUND` | `CURRENT_API_COMPATIBLE` for final live behavior | No layout edit. If disabled assistance were revived, pet counts/global indices would require client traces. |
| `SMSG_BATTLE_PET_UPDATE` | `POSSIBLE_LAYOUT_DEPENDENCY` | Existing manager serializer is unchanged, but altered XP/health is sent through it at finish. |
| `CMSG_PET_BATTLE_SET_FRONT_PET` handling | `REQUIRES_BUILD18414_TRACE` | Flamehawk refuses an anomalous living-active-pet forced swap without the current fake-death/pending-move repair; client recovery behavior is unknown. |

**FACT:** No opcode constant or packet definition is changed. Packet dependency
arises from changed state and event production, not a direct wire-layout patch.
Exact round event ordering, forced-swap acknowledgement, death result value,
and finish release behavior require a build-18414 client trace.

## DBC / DB2 Dependencies

**FACT:** `BattlePetAbilityEffect` rows supply `EffectProperty`, six properties,
trigger ability, turn association, and ordering; `BattlePetAbilityTurn` links to
ability and turn/proc data; `BattlePetAbility` supplies family and flags;
`BattlePetAbilityState` supplies state/value modifications; species/breed/quality
stores feed pet construction and family/stat behavior. Current source declares
and loads these contracts, and the Flamehawk handlers directly index them.

**FACT:** Weather, state, accuracy, damage/heal, aura trigger, cleanse, kill,
resurrection, and target-team behavior therefore cannot be reviewed independently
of the actual 5.4.8 rows. Ability 349, 654, 758, 1106 and aura IDs such as 174,
505, and 142205 are hard-coded additional dependencies.

**UNKNOWN:** This repository does not check in authoritative extracted
build-18414 rows sufficient to validate the new mappings or property semantics.
No generated DBC/DB2 data was imported. Public fork code was used only as
comparative implementation evidence, not as authoritative client data.

## Comparison With Later Battle-Pet Fork Work

**FACT:** The Andrew branch tip inspected for limited comparison was
`f1abdd1a2754f4a39f9e5d2cd5112e63899a86c4`. Its relevant lineage includes
Candidates 10 (`39205279...`) and 11 (`100303ca...`) plus `c6400a29`,
`1e338126`, `987cdb97`, `867b69b0`, `aeef0c9d`, and battle-pet portions of
`bd8ad551`. This task did not reconstruct or combine that series.

**FACT:** Andrew independently implements overlapping effect numbers but often
with different names, targets, and logic: for example effect 22 is
`NegativeAura` rather than Flamehawk `Dummy`; 29 is `StateBonusDamage`; 68 is
`Sacrifice`; 79 is `Damage`; 85 is `PositiveAura`; 135 is target-head
`KillActive`; 169 is caster `WeatherAura`; and 197 is `Vengeance`. Andrew also
implements additional effects (33, 100, 131, 139, 145, 150, 160, 165, 168, 177,
179, 204) while omitting or replacing multiple Flamehawk handlers.

**INFERENCE:** This substantial divergence is evidence that Flamehawk was not a
settled final specification. Andrew sometimes supersedes the same problem area
(weather, trainer death/swap, missing handlers), sometimes independently
implements an analogous effect, and sometimes conflicts outright. Its broader
trainer, world, Playerbots, map, persistence, and SQL dependencies make it
unsuitable as a drop-in correctness oracle. No Andrew code should be blended
with a Flamehawk unit without its own reconstruction task.

## Proposed Final-State Backport Units

Each unit targets the normal `game` library and final `worldserver` link when
eventually implemented; none is approved here.

1. **F1 — dispatch inventory and target contract (review-only precursor).**
   Files/functions: handler table, `AddTargets`, target enum, declarations in
   both ability-effect files. Ancestry `26e16ed4` → `0c279a48` → `e91819a4`.
   Prerequisite: build-18414 effect rows. Status `ABSENT`/`CONFLICTING_IMPLEMENTATION`;
   static confidence low; DBC high, packet medium, persistence none. Tests:
   table-driven effect-to-handler/target assertions and captured round payloads.
2. **F2 — formula/passive computation.** `GetPetType`, `CalculateHit/Heal/Damage`,
   `Damage`, `Heal`; ancestry `26e16ed4`, `0c279a48`, `aec3e154`, Candidate 7
   diagnostics. Requires F1 mappings plus DBC semantics. Status conflicting;
   confidence low; DBC/runtime high, packet medium, persistence health only.
   Tests: deterministic families, caps, misses, crits, aura procs, self/target.
3. **F3 — aura/weather/state handlers.** The aura/control/weather/cleanse/reset
   handlers and effects 28/49/52/74–86/136–140/169–172. Ancestry all three
   ability-effect commits. Requires F1/F2 and aura lifecycle. Status absent;
   confidence low; DBC/packet/runtime high, persistence none. Tests: each row,
   expiry, replacement, state rollback, weather transition, event ordering.
4. **F4 — direct health/death effect handlers.** Resurrection, kill, health
   consume, set-health-percent, equalize/nonlethal interactions; effects 67,
   111, 121, 128, 135. Ancestry `26e16ed4`, `0c279a48`, `e91819a4`. Requires F1/F2
   and proven death contract. Status absent; confidence low; DBC/packet/runtime
   high, persistence medium. Tests: lethal boundaries, death once, revive,
   mechanical/undead interactions, final saved health.
5. **F5 — inactive-death round-result classification.** `PetBattle::Kill` only,
   ancestry `0f46d20c`. No SQL/Playerbots/opcode prerequisite; current status
   partially present; static confidence medium but client semantics unknown;
   DBC none, packet high, persistence none. Tests: active death, background
   periodic death, catch, forced swap, final pet, exact round-result trace.
6. **F6 — front-pet anomaly guard.** The isolated live hunk in
   `WorldSession::HandlePetBattleSetFrontPet`, ancestry `0f46d20c`. Status
   conflicting; confidence medium for avoiding a false-death mutation, low for
   recovery; DBC none, packet high, persistence none. Tests: legal voluntary
   swap, dead forced swap, anomalous living active pet, next-round readiness.
7. **F7 — start/end state hygiene.** `StartBattle`, `EndBattle`, and removal of
   redundant release calls from the prospective synthesis; ancestry
   `26e16ed4`/`aec3e154`. Status partial; confidence low because forcing health
   to one changes durable state; DBC low, packet medium, persistence high.
   Tests: zero-health roster rejection/start, stale states, forfeit, forced end,
   logout, saved health, client release ordering.
8. **F8 — treat XP modifier.** `BattlePet::SetXP`, ancestry `aec3e154`. Status
   absent; code confidence medium, semantics low; DBC/spell-data high, packet
   low, persistence high. Tests: aura presence, cap/level-up, deterministic
   seeded bounds, save/reload, update payload. No SQL is in the fork unit.
9. **F9 — multi-wild and NPC swap experiment.** `AddWildBattlePetAssist`,
   constructor and `TurnFinished` blocks from `e095c494`/`0f46d20c`/Candidate 7.
   Status incomplete/commented and `SUSPICIOUS_OR_INCOMPLETE`; it is **not a
   backport candidate** until redesigned with spawn ownership, packet, AI, and
   runtime evidence. DBC, packet, persistence, and world-state dependencies are
   all high.
10. **F10 — orphan declarations/comments cleanup.** States 71/72,
    `isImmortal`, `m_immortalOneRound`, empty passive comments, NYI lock method,
    and unused assist API. These are provenance artifacts, not gameplay to
    backport. A future documentation/code-cleanup review may explicitly omit
    them from synthesized behavioral units.

## Suggested First Isolated Reviews

At most three units clear the *narrowness* threshold, but none is asserted
correct:

1. **F5, inactive-death round-result classification** — one function, no SQL,
   Playerbots, opcode, or DBC dependency. It is the best first evidence review,
   conditional on a build-18414 trace.
2. **F6, front-pet anomaly guard** — one handler hunk and no layout change. It
   should be reviewed separately from F5 and requires a reproducible stuck-swap
   sequence/client trace.
3. **F8, treat XP modifier** — mechanically narrow, but only after spell
   142205's build-18414 effect and intended XP formula are independently
   established. Without that evidence it must remain deferred.

**FACT:** No ability-effect family qualifies as a first implementation
candidate: each is coupled to unverified DBC semantics and several have
conflicting later-fork implementations.

## Unresolved Build-18414 Evidence

1. Exact effect-property, six-property, trigger, target, turn, proc, flag, and
   state rows for every newly dispatched effect.
2. Retail family-passive formulas, hard-coded ability/aura IDs, accuracy 349,
   cocoon/untargetable behavior, damage caps, and aura proc rules.
3. Round-result value and event ordering for active death, non-active periodic
   death, catch, resurrection, simultaneous deaths, and last-pet termination.
4. Client behavior after anomalous `SET_FRONT_PET`, forced/voluntary swaps,
   trainer/NPC multi-pet battles, and unavailable casts.
5. Exact final-round/finished ordering relative to unroot, unpacify, world
   thread interruption, forfeit health, updates, achievements, and persistence.
6. Spell 142205's identity and retail XP calculation; random flat XP is fork
   behavior only.
7. Whether states 71/72 or an immortality mechanism exist in build-18414 data;
   the Flamehawk fields are not integrated evidence.
8. Deployed wild-pet spawn ownership/lifetime and client packet expectations
   for nearby assistants. The final Flamehawk code deliberately leaves this
   disabled.

Until those artifacts exist, the reconstructed series is a map for isolated
future review, not a retail-correct patch set.
