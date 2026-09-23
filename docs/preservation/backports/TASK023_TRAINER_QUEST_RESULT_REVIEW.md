# Task 023: Trainer Quest / Achievement Result Review

## Continuation State

- **FACT:** Task 023 began with a clean working tree on the existing `work`
  branch at `4adb7a7a4fc73a0a397e51bbd396b078819b2e3f`.
- **FACT:** The immediately preceding commits are Task 022's
  `4adb7a7 chore(preservation): record battle-pet XP-rate review` and Task
  021's `10d435a fix(battlepet): restore full loadout slot GUIDs`.
- **FACT:** Task 021's raw `GetUInt64()` slot reconstruction remains present.
  This task changes no C++, SQL, configuration, opcode, packet, trainer,
  Playerbots, or build artifact. `/tmp/mop-preservation-dev` was not configured,
  built, cleaned, or otherwise touched.

## Andrew Provenance

- **FACT:** Trainer quest-result foundations first appear in
  `39205279cebebd1d2c83000a0e9043e68047e659` (2026-05-19). That commit adds
  trainer battles, extends the ordinary win-family mask to trainer PVE, and on
  a trainer victory calls `KilledMonsterCredit` for the trainer creature.
- **FACT:** Candidate 11,
  `100303ca22b08f04ee5620dc1622617d5a680cdd` (2026-05-22), introduces
  `Player::PetBattleCompleteQuest`, declares it, and calls it beside
  `KilledMonsterCredit`. Its subject is `petbattle: fix weather effects, quest
  credit, and various battle bugs`.
- **FACT:** No intermediate or later commit changes or removes the helper, its
  declaration, its EndBattle call, the trainer-inclusive family-mask call, or
  the trainer `KilledMonsterCredit` call.
- **FACT:** `bd8ad5515418d94abd35ba7bf71430979243a09c` (2026-06-21) adds the
  final achievement-side condition for
  `ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_PET_BATTLE_AGAINST_TAMER` in
  `AchievementMgr::AdditionalRequirementsSatisfied`.
- **FACT:** Direct comparison with the fetched final
  `feature/pandaria-fixes` tip shows Candidate 11's helper/calls unchanged and
  the `bd8ad551` tamer condition unchanged. Later EndBattle restructuring
  changes packet/reward order but retains the A5 statements and gates.

## Final A5 Source Scope

The final Andrew unit occupies four concrete locations:

| File / function | Ancestry | Role |
| --- | --- | --- |
| `src/server/game/Entities/Player/Player.h`, `Player::PetBattleCompleteQuest` declaration | `100303ca` | Quest-credit helper API |
| `src/server/game/Entities/Player/Player.cpp`, `Player::PetBattleCompleteQuest` | `100303ca` | Type-11 objective progress |
| `src/server/game/BattlePet/PetBattle.cpp`, `PetBattle::EndBattle` | trainer result from `39205279`; helper call from `100303ca`; later retained | Win gating, monster credit, type-11 credit, family-mask achievement update |
| `src/server/game/Achievements/AchievementMgr.cpp`, `AchievementMgr::AdditionalRequirementsSatisfied` | `bd8ad551` | Tamer-entry additional requirement |

- **FACT:** Quest credit and achievement filtering are logically separable.
  The helper does not call achievement APIs. EndBattle independently invokes
  both quest-credit paths and `ACHIEVEMENT_CRITERIA_TYPE_WIN_PET_BATTLE`.
- **FACT:** The broad commits also contain trainer lifecycle, weather, effects,
  world data, Tillers, and other work. None belongs to a future isolated A5
  patch merely because it shares ancestry.

## PetBattleCompleteQuest

The exact final signature is:

```cpp
void Player::PetBattleCompleteQuest(uint32 creatureEntry, ObjectGuid creatureGuid);
```

- **FACT:** `creatureEntry` is the defeated trainer's creature-template entry,
  not a quest ID. `creatureGuid` is used only for client credit context.
- **FACT:** The helper scans all `MAX_QUEST_LOG_SIZE` slots. Empty slots and
  missing quest templates are skipped.
- **FACT:** It prefilters templates with
  `HasQuestObjectiveType(QUEST_OBJECTIVE_WINPETBATTLEAGAINSTNPC)`, then accepts
  only `QUEST_STATUS_INCOMPLETE` quest status.
- **FACT:** It scans the quest's objectives and matches both objective type 11
  and `questObjective.ObjectID == creatureEntry`.
- **FACT:** It obtains progress by objective ID. If progress is less than the
  unsigned required `Amount`, it adds `min(1, Amount - currentCounter)`, updates
  `m_questObjectiveStatus[objective.ID]`, calls
  `MarkQuestObjectiveToSave(questId, objective.ID)`, and sends
  `SendQuestUpdateAddCredit` with the trainer GUID, old count, and increment.
- **FACT:** It calls `CanCompleteQuest`/`CompleteQuest` even when the matching
  objective was already at its required count, then breaks after the first
  matching objective in that quest. It continues scanning other quest-log
  entries.
- **FACT:** The cap test prevents progress exceeding `Amount` through this
  helper. Amounts greater than one require separate successful invocations and
  gain exactly one each time. Already-complete objectives gain nothing.
- **FACT:** Calling the helper twice for the same result can add twice when
  `Amount > 1`; the progress cap prevents overflow but does not make an
  individual battle result idempotent.

## Existing Quest Objective Framework

- **FACT:** Current preservation already provides the generic
  `Player::QuestObjectiveSatisfy(uint32 objectId, uint32 amount, uint8 type,
  uint64 guid)`. It performs the same quest-log scan, incomplete-status gate,
  type/ObjectID match, capped increment by objective ID, save marking,
  `SendQuestUpdateAddCredit`, and completion check.
- **FACT:** A call equivalent to Candidate 11's entire helper is:

```cpp
player->QuestObjectiveSatisfy(creature->GetEntry(), 1,
    QUEST_OBJECTIVE_WINPETBATTLEAGAINSTNPC, creature->GetGUID());
```

- **FACT:** Existing `KillCreditGO` and `TalkedToCreature` are compatibility
  wrappers around this generic API. Candidate 11 instead duplicates its
  implementation in a battle-pet-specific Player method.
- **FACT:** The helper follows the generic storage, persistence, packet, and
  completion conventions, but adds no semantics that the generic API lacks.
- **FACT:** Both implementations break after the first matching objective per
  quest. Multiple different quests can advance; multiple same-type/same-entry
  objectives inside one quest do not all advance in one call.
- **INFERENCE:** A future A5 should use `QuestObjectiveSatisfy` directly rather
  than preserve the redundant helper/declaration. This is an equivalent use of
  an existing API, not a change to objective semantics.

## WINPETBATTLEAGAINSTNPC Semantics

- **FACT:** `QUEST_OBJECTIVE_WINPETBATTLEAGAINSTNPC` is
  `QuestObjectiveType` value 11.
- **FACT:** Quest-template validation requires a storage index for this type
  and, when `ObjectID` is nonzero, validates it through
  `GetCreatureTemplate(ObjectID)`. This is direct source evidence that a
  nonzero `ObjectID` denotes a creature-template entry.
- **FACT:** The generic objective framework accepts arbitrary positive amounts
  and caps progress at `Amount`. The loader has no type-11 restriction forcing
  `Amount == 1`; therefore amounts greater than one are structurally supported.
- **FACT:** Current preservation contains no caller that advances type 11.
  `KilledMonsterCredit` processes `QUEST_OBJECTIVE_MONSTER` only and cannot
  satisfy type 11 merely because the ObjectID is the same creature entry.
- **UNKNOWN:** No checked-in trainer objective dataset or build-18414 trace was
  used to prove which retail trainer quests have amount 1 versus a larger
  amount, whether ObjectID zero has special wildcard meaning, or the exact
  client presentation. Candidate 11 requires exact nonzero entry equality and
  would not satisfy a zero/wildcard objective.

## EndBattle Gating

Andrew's final call remains inside `if (team == m_winningTeam)` and then
`if (GetType() == PET_BATTLE_TYPE_PVE_TRAINER)`. It iterates both battle teams,
finds a non-null `GetWildBattlePet()`, and passes that creature's entry and GUID
to both credit paths.

1. **FACT:** Ordinary wild battles cannot invoke the helper because their type
   is `PET_BATTLE_TYPE_PVE`, not trainer PVE. Capture handling is separately
   restricted to ordinary PVE.
2. **FACT:** A player loss cannot invoke it because the player's team is not
   `m_winningTeam`.
3. **FACT:** A player forfeit is a loss and cannot invoke it. The code would
   grant credit if the trainer team somehow forfeited and the player became the
   winner; whether that state is reachable is **UNKNOWN** without A4 runtime.
4. **FACT:** Forced/aborted `EndBattle(nullptr)` sets no winning team, so no
   team satisfies the win gate and no credit is issued.
5. **FACT:** In the expected trainer topology, only the opponent team exposes
   the trainer creature, so one normal player victory invokes each credit path
   once. **UNKNOWN:** `EndBattle` has no explicit already-finished guard; an
   erroneous repeated call before removal could award another count for an
   Amount-greater-than-one objective. Normal update flow marks the battle
   finished and removes it on the next system update, but A4 lifecycle runtime
   must prove single finalization.
6. **FACT:** The identifier passed is the trainer Creature's template entry;
   its live ObjectGuid accompanies the credit packet/API.
7. **FACT:** The call requires A4 to preserve valid teams, winner state,
   trainer type, and trainer creature lifetime through EndBattle.

## KilledMonsterCredit Relationship

- **FACT:** `KilledMonsterCredit(entry, guid)` updates kill-creature
  achievements and only `QUEST_OBJECTIVE_MONSTER` objectives. It may resolve
  the live GUID back to the creature's actual entry.
- **FACT:** Type-11 credit matches
  `QUEST_OBJECTIVE_WINPETBATTLEAGAINSTNPC`; it neither updates monster
  objectives nor invokes kill-creature achievements.
- **FACT:** The calls are therefore complementary for distinct objective
  types, not two implementations of the same objective. A quest containing
  both a monster objective and a type-11 objective for the trainer would
  intentionally advance both once under Andrew's code.
- **INFERENCE:** There is no direct duplicate credit to one objective because
  objective types differ. There is a semantic risk that treating a trainer
  battle victory as a creature kill could advance unrelated monster objectives
  or kill achievements for the same NPC. Repository source does not establish
  whether that broad side effect is intended retail behavior.
- **UNKNOWN:** Trainer quest data and a build-18414 result trace are required
  to decide whether `KilledMonsterCredit` should accompany type-11 credit in a
  future A5. It must not be silently removed or combined during extraction.

## Achievement Result Logic

Two achievement pieces form Andrew's final trainer result behavior:

1. **Trainer-inclusive win/family call (`39205279`).** On the winning player's
   team, EndBattle builds a bit mask with `1 << pet->GetFamilty()` for every pet
   on `Opponent()->BattlePets` when the type is ordinary PVE or trainer PVE,
   then calls `UpdateAchievementCriteria(
   ACHIEVEMENT_CRITERIA_TYPE_WIN_PET_BATTLE, 1, familyMask, 0, player)`.
   Current preservation already performs the same operation for ordinary wild
   PVE but excludes trainer PVE.
2. **Against-tamer additional condition (`bd8ad551`).** While evaluating an
   achievement modifier, it retrieves the player's live PetBattle from
   `sPetBattleSystem`, obtains `battle->Opponent()->GetWildBattlePet()`, and
   accepts only when that creature's entry equals the modifier's `reqValue`.
   The EndBattle achievement update occurs while the battle remains registered
   and before EndBattle sets `Finished`, so the lookup is available in the
   intended flow.

- **FACT:** The win/family update is win-only. Losses instead call
  `ResetCriterias(CRITERIA_RESET_TYPE_LOSE_PET_BATTLE, 0)`.
- **FACT:** Family-mask correctness depends on the opponent trainer team and
  its pets remaining alive as objects through reward processing. The tamer
  condition additionally depends on the trainer Creature pointer remaining
  valid and exposed as the opponent team's `GetWildBattlePet()`.
- **FACT:** Current preservation's
  `ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_PET_BATTLE_AGAINST_TAMER` always
  returns false. Thus the later hunk is not equivalent to existing behavior.
- **INFERENCE:** The tamer-entry comparison is statically coherent with the
  condition name and trainer team model, and the family mask follows the
  existing wild-battle convention. The meaning of `reqValue`, exact family-mask
  matching rules, multi-family trainer teams, and update/reset timing still
  require achievement data and runtime evidence.
- **Classification:** trainer-inclusive family update:
  `NEEDS_TRAINER_RUNTIME`; against-tamer condition:
  `NEEDS_BUILD18414_EVIDENCE`. Neither is redundant with the quest helper.

## Current Preservation Comparison

| A5 component | Current classification | Evidence |
| --- | --- | --- |
| Trainer-specific `PetBattleCompleteQuest` helper | `REDUNDANT_EXISTING_API` | Named helper is absent, but `QuestObjectiveSatisfy` already provides equivalent type/ObjectID progress semantics. |
| EndBattle type-11 quest call | `ABSENT` | No current caller advances objective type 11. |
| Trainer victory `KilledMonsterCredit` relationship | `ABSENT` | Current EndBattle has no trainer branch or trainer credit. |
| Trainer win/family achievement behavior | `ABSENT` | Current family mask is restricted to ordinary PVE. |
| Against-tamer additional achievement condition | `CONFLICTING_IMPLEMENTATION` | Current condition explicitly returns false; Andrew resolves the live trainer entry. |

- **FACT:** No current alternative path produces the complete final A5 result.
- **FACT:** The generic quest API makes the new Player helper unnecessary, but
  it does not make the missing EndBattle trigger redundant.

## Final Classification

**Final A5 classification: `REDUNDANT_EXISTING_API`.** The exact Andrew helper
should not be preserved: current `QuestObjectiveSatisfy` already implements its
logic. The intended type-11 EndBattle result remains worth preserving later as
a direct generic-API call, but the complete A5 unit is not
`READY_AFTER_TRAINER_CORE` because two semantic questions remain:

1. whether trainer victory should also call broad `KilledMonsterCredit`; and
2. the build-18414/data contract for tamer achievement `reqValue`, family masks,
   and lifecycle timing.

- **FACT:** No source is modified because any result hook is orphaned without
  validated trainer core.
- **UNKNOWN:** If authoritative trainer quest and achievement data resolve the
  two questions above, the reduced quest call may become ready after A4 without
  adding any Player helper.

## Future Dependency / Implementation Boundary

The safe ordering remains:

```text
A2 trainer data/lifetime
  -> A3 initiation/session
  -> A4 round/death/swap and exactly-once finalization
  -> A5 quest/achievement results
```

- **FACT:** A5 cannot be meaningfully invoked before A2 supplies a stable
  trainer creature/team, A3 creates a validated trainer session, and A4 proves
  winner/loser, forfeit, abort, swap, and finalization behavior.
- **INFERENCE:** Once A2-A4 are validated, the quest-only portion has a smaller
  implementation boundary: modify only
  `src/server/game/BattlePet/PetBattle.cpp::EndBattle` to call existing
  `Player::QuestObjectiveSatisfy` under the proven trainer-win gate.
- **INFERENCE:** If evidence independently approves the complete result unit,
  future scope is limited to `PetBattle.cpp::EndBattle` and
  `Achievements/AchievementMgr.cpp::AdditionalRequirementsSatisfied`. It
  should not add `Player::PetBattleCompleteQuest` to `Player.h/.cpp` because
  that API is redundant.
- **FACT:** A future A5 must not carry trainer SQL, trainer initiation/session,
  round/death/swap changes, weather, ability effects, XP rates, packets/opcodes,
  wild-pet SQL, Playerbots, or unrelated Andrew/Tillers code.
- **FACT:** `KilledMonsterCredit` must remain a separately reviewed line until
  objective and achievement data prove its intended side effects.

## Required Runtime Tests

After A2-A4 exist and before deployment, exercise all of the following with
authoritative trainer quest/achievement data and a build-18414 client:

1. win against the matching trainer and verify exactly one type-11 increment;
2. win against a nonmatching trainer and verify no increment;
3. objectives with Amount 1 and Amount greater than 1, including an already
   complete objective and repeated legitimate wins;
4. multiple active quests and, if data permits, multiple matching objectives
   in one quest;
5. player loss, player forfeit, forced abort, disconnect, and repeated
   finalization attempts, proving no result credit;
6. trainer-side forfeit or any equivalent terminal path, proving whether it is
   reachable and whether it should count;
7. quest progress packet, quest completion, save, logout/login, and server
   restart persistence;
8. monster-objective and kill-achievement effects with and without the proposed
   `KilledMonsterCredit` line;
9. trainer teams with one family, repeated families, and multiple families,
   checking the exact family mask and win criteria;
10. achievements with the against-tamer additional condition for matching and
    nonmatching trainer entries, plus loss/reset behavior; and
11. trainer creature/team lifetime through achievement evaluation and cleanup.

No compilation or runtime testing is performed in this review-only task.
