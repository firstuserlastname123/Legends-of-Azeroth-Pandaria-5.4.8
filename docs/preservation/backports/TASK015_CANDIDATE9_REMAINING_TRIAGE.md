# Task 015: Candidate 9 Remaining-Hunk Triage

## Starting State

- **FACT:** this documentation-only review started from commit
  `e020e6ccb13eaa129b93b2c4e53205cbca9203b1`, the merge of Task 014 into the
  supplied preservation integration history. The supplied local branch is
  named `work`, as in the preceding preservation tasks, rather than literally
  `preservation/main`.
- **FACT:** `git status --short` was empty before review. The eight-commit log
  contained the Task 011 through Task 014 merges and their task commits, with
  Task 014 most recent.
- **FACT:** this task does not apply Candidate 9, modify C++ or SQL, configure
  CMake, compile, start a server, or perform runtime testing.

## Candidate Provenance

- Repository: `https://github.com/MityaFoxy/Legends-of-Azeroth-Pandaria-5.4.8.git`
- Documented branch: `master`
- Exact commit: `481dd4bc5d82a7caed094ad175f54ed1c3c1d4a3`
- Subject: `Fix multiple null pointer dereference issues and redundant checks`
- Author and committer: google-labs-jules[bot]
  `<161369871+google-labs-jules[bot]@users.noreply.github.com>`
- Author and commit date: 2026-09-15 05:51:49 +0000
- Co-author recorded by the commit: MityaFoxy
  `<134737088+MityaFoxy@users.noreply.github.com>`

**FACT:** the exact object was fetched into temporary ref
`refs/task015/candidate9-commit`. The commit's stat, summary, full metadata,
and complete patch were inspected with the required `git show` forms. It
changes 12 files with 56 insertions and 26 deletions. It was not applied.

## Previously Reviewed Hunks

These four micro-hunks retain the classifications and decisions established
by their dedicated tasks; this review does not reopen them.

| # | File and function | Candidate change | Prior result | State here |
|---:|---|---|---|---|
| 1 | `MogushanVault/boss_stone_guard.cpp`, `spell_jasper_chains_AuraScript::OnPeriodic` | Moves the caster/target/spell checks before the player lookup. | Task 011 rejected the guard as redundant under the AuraScript hook contract. | `ALREADY_REVIEWED` |
| 2 | `HeartOfFear/heart_of_fear.cpp`, `npc_wind_lord_meljarak_introAI::UpdateAI` | Wraps the Garalon instance operations in `if (instance)`. | Task 012 found that the guard is too late to make a null-instance lifecycle safe and classified it `NEEDS_CONTEXT`. | `ALREADY_REVIEWED` |
| 3 | `Scenarios/GreenstoneVillage/greenstone_village.cpp`, `AreaTrigger_at_behind_tzu::OnTrigger` | Adds an early null-player return and removes the later dominated player test. | Task 013 rejected it as redundant under the `AreaTriggerScript` dispatcher contract. | `ALREADY_REVIEWED` |
| 4 | `ThroneOfThunder/throne_of_thunder.cpp`, `RitualLightningPredicate::operator()` | Rejects invalid/non-player objects before evaluating player distance. | Task 014 proved the predicate defect, backported the exact hunk, and compile/link validated it. | `ALREADY_REVIEWED` |

## Remaining Hunk Inventory

Candidate 9 contains **17 logical micro-hunks**: the four above and **13
remaining** micro-hunks. Separate callbacks and unrelated edits in a shared
file are counted separately.

| # | File | Class/function | Kind | Pointer/state | Normal-valid-input effect |
|---:|---|---|---|---|---|
| 5 | `src/server/game/Battlegrounds/ArenaTeam.cpp` | `ArenaTeam::MemberWon` | REMOVED | `Player* player` | None when `player` is valid. |
| 6 | `src/server/scripts/Pandaria/GateOfTheSettingSun/boss_raigonn.cpp` | `npc_raigonn_weak_spotAI::JustDied` | ADDED/REORDERED | `InstanceScript* instance` | None when `instance` is valid. |
| 7 | `src/server/scripts/Pandaria/MogushanVault/boss_stone_guard.cpp` | `boss_stone_guard_controllerAI::DoAction`, `ACTION_POWER_DOWN` | REMOVED | `InstanceScript* instance` | None when `instance` is valid. |
| 8 | `src/server/scripts/Pandaria/ShadopanMonastery/boss_master_snowdrift.cpp` | `npc_snowdrift_noviceAI::DamageTaken` | REMOVED | `InstanceScript* instance` | None when `instance` is valid. |
| 9 | `src/server/scripts/Pandaria/SiegeOfOrgrimmar/boss_garrosh_hellscream.cpp` | `sat_garrosh_desecrated_weapon::CheckTriggering` | ADDED/REORDERED | `WorldObject* triggering` | None when `triggering` is valid. |
| 10 | `src/server/scripts/Pandaria/SiegeOfOrgrimmar/boss_korkron_dark_shamans.cpp` | `boss_wavebinder_kardrisAI::JustEngagedWith` | REMOVED | `InstanceScript* instance` | None when `instance` is valid. |
| 11 | `src/server/scripts/Pandaria/SiegeOfOrgrimmar/boss_norushen.cpp` | `spell_norushen_lingering_corruption::FilterTargets` lambda | ADDED/REORDERED | `WorldObject* target`; target-list semantics | None for non-null entries; null entries are newly removed. |
| 12 | `src/server/scripts/Pandaria/ThroneOfThunder/boss_council_of_elders.cpp` | `boss_council_of_elders_baseAI::JustEngagedWith` | ADDED/REORDERED | `InstanceScript* instance`; encounter engage/helper action | None when `instance` is valid. |
| 13 | same | `boss_council_of_elders_baseAI::JustDied` | ADDED | `InstanceScript* instance`; encounter disengage | None when `instance` is valid. |
| 14 | same | `npc_garajal_AI::BeginFight` | LOGIC_CHANGED | `InstanceScript* instance`; boss state | None when `instance` is valid; null now returns. |
| 15 | `src/server/scripts/Pandaria/zone_the_veiled_stair.cpp` | `npc_wrathion_suen_celestial::IsSummonedBy` | ADDED/REORDERED | `Unit* summoner` | None when `summoner` is valid. |
| 16 | same | `npc_wrathion_chiji_celestial::IsSummonedBy` | ADDED/REORDERED | `Unit* summoner` | None when `summoner` is valid. |
| 17 | same | `npc_wrathion_niuzao_celestial::IsSummonedBy` | ADDED/REORDERED | `Unit* summoner` | None when `summoner` is valid. |

## Detailed Static Triage

### Added Guards

#### 6. Raigonn weak spot death

- **Current-tree behavior:** `JustDied` unconditionally calls
  `instance->SendEncounterUnit`; only the subsequent Raigonn lookup is guarded.
- **Candidate behavior:** moves both the encounter-frame call and lookup/action
  beneath one `if (instance)` block.
- **FACT:** the constructor stores `creature->GetInstanceScript()` without an
  assertion. `JustDied` does not dereference `instance` before the affected
  statement, so this is not merely a late guard in this callback.
- **UNKNOWN:** content bindings and spawn restrictions may make the weak spot
  instance-only in supported play. Static inspection did not prove that
  deployment invariant.
- **State:** `PROMISING_STATIC_FIX`.
- **Rationale:** this is a narrow, behavior-preserving guard for valid dungeon
  instances, and unlike several other candidate edits it protects the first
  local dereference. A dedicated task can decide whether the framework/content
  contract permits a null instance.

#### 9. Garrosh desecrated-weapon area trigger

- **Current-tree behavior:** dereferences `triggering` to obtain its instance
  before the later `triggering &&` operand in the return expression.
- **Candidate behavior:** returns false for null first and removes that later,
  now-dominated operand.
- **FACT:** `IAreaTriggerAuraUpdater::operator()` dereferences the visited
  `WorldObject*` for world state, distance, height, and GUID around its call to
  `CheckTriggering`. The grid worker supplies a live object, not a nullable
  public callback argument.
- **INFERENCE:** null would violate the observed area-trigger visitor contract;
  the local guard cannot make a supported path safer.
- **State:** `REDUNDANT_GUARD`.
- **Rationale:** the candidate rearranges a path-insensitive warning without
  changing supported behavior.

#### 11. Norushen Lingering Corruption filter

- **Current-tree behavior:** the lambda immediately evaluates
  `target->GetEntry()`; its later `target &&` protects only a different
  conjunction arm and is too late for null.
- **Candidate behavior:** changes the predicate to `!target || (...)`, thereby
  removing a null entry, and removes the dominated later null test. Operator
  precedence and all non-null entry/GUID comparisons otherwise remain the
  same.
- **FACT:** the object-area target hook receives the core-built search list.
  The searcher appends live grid object sources; Task 014's review of this same
  target-list machinery found no supported null insertion path.
- **UNKNOWN:** external script mutation could violate that producer invariant,
  but no such producer was found for this local list.
- **State:** `REDUNDANT_GUARD`.
- **Rationale:** returning true would be the correct `remove_if` treatment of a
  hypothetical null, but null is outside the established list contract.

#### 12. Council base AI engagement

- **Current-tree behavior:** unconditionally sends `ENCOUNTER_FRAME_ENGAGE`,
  then conditionally supplies an empty GUID to the helper lookup if `instance`
  is null. The first statement already dereferences `instance`.
- **Candidate behavior:** places the frame send and Garajal lookup/action under
  `if (instance)` and removes the now-unneeded ternary from the GUID argument.
- **FACT:** the constructor stores `me->GetInstanceScript()`. The same base
  AI's `Reset` still unconditionally dereferences `instance`, so Candidate 9
  does not establish a generally null-safe councillor lifecycle.
- **UNKNOWN:** whether these councillors can legitimately initialize or engage
  without the Throne of Thunder instance script requires spawn/content and
  lifecycle evidence.
- **State:** `NEEDS_CONTEXT`.
- **Rationale:** the local ordering is safer, but it is partial lifecycle
  hardening of the same kind rejected as sufficient in Task 012.

#### 13. Council base AI death

- **Current-tree behavior:** unconditionally sends
  `ENCOUNTER_FRAME_DISENGAGE` before entry-specific death handling.
- **Candidate behavior:** guards only that encounter-frame call with
  `if (instance)`.
- **FACT:** `instance` comes from `me->GetInstanceScript()` in the base
  constructor. Other base lifecycle paths, notably `Reset`, remain
  unconditional users.
- **UNKNOWN:** no supported null-instance death path or crash reproduction is
  recorded.
- **State:** `NEEDS_CONTEXT`.
- **Rationale:** it prevents this one dereference but does not make the AI
  lifecycle null-safe or prove that null is legal.

#### 15--17. Veiled Stair celestial summon callbacks

The Suen, Chi-Ji, and Niuzao edits are three distinct micro-hunks with the
same contract finding.

- **Current-tree behavior:** each `IsSummonedBy` dereferences `summoner` to set
  private ownership and `ownerGUID`, then tests `summoner &&
  summoner->ToPlayer()` before awarding its respective credit.
- **Candidate behavior:** each returns early for null and simplifies the later
  player test to `summoner->ToPlayer()`.
- **FACT:** `TempSummon::InitSummon` obtains `owner = GetSummoner()` and calls
  `AI()->IsSummonedBy(owner)` only inside `if (owner)`. These overrides are
  reached through that non-null owner branch.
- **INFERENCE:** a null direct call would violate the observed callback
  contract. For valid summoners, private ownership, credit, scheduling, and
  later owner lookup are unchanged.
- **State:** each is `REDUNDANT_GUARD`.
- **Rationale:** all three move an already guaranteed precondition into the
  callback; duplication does not create preservation value.

### Removed Guards

#### 5. Arena team member win

- **Current-tree behavior:** obtains `player->GetGUID()` at function entry,
  then later updates the highest-personal-rating achievement only when
  `player &&` the slot is not rated battleground.
- **Candidate behavior:** removes `player &&` from that later condition.
- **FACT:** both known callers pass a live `Player*` already used extensively
  in `Battleground::RewardMark`; `MemberWon` itself dereferences it before the
  removed check.
- **INFERENCE:** the check is locally redundant, but deleting it neither fixes
  the unsupported null call nor improves preservation behavior.
- **State:** `NO_ACTION`.
- **Rationale:** static redundancy alone is not a reason to import cleanup,
  particularly when the task's rule forbids proposing guard removals merely
  for cleanup.

#### 7. Stone Guard power-down aura cleanup

- **Current-tree behavior:** the surrounding action uses `instance` to obtain
  guardian GUIDs, but separately guards the tiles-aura removal.
- **Candidate behavior:** removes that local `if (instance)`.
- **FACT:** the controller constructor stores
  `creature->GetInstanceScript()`. The branch has earlier unconditional
  instance use, which makes the guard appear path-redundant after successful
  execution, but no assertion or non-null type invariant was found.
- **UNKNOWN:** the guard may document intentional best-effort behavior or may
  merely be inconsistent with surrounding assumptions.
- **State:** `RISKY_GUARD_REMOVAL`.
- **Rationale:** removing a defensive check offers no behavioral fix and no
  sufficiently proven framework invariant supports the deletion.

#### 8. Snowdrift novice defeat state

- **Current-tree behavior:** in lethal `DamageTaken`, it uses `instance` for
  the referee GUID before conditionally setting `DATA_DEFEATED_NOVICE`.
- **Candidate behavior:** removes the condition and always invokes `SetData`.
- **FACT:** the constructor stores `me->GetInstanceScript()`; this path has an
  earlier unconditional instance dereference. That proves only that a null
  path would already fail, not that the framework permits removing defenses.
- **UNKNOWN:** content placement may guarantee the Shado-Pan instance, but no
  formal non-null contract was found.
- **State:** `RISKY_GUARD_REMOVAL`.
- **Rationale:** the edit is cleanup after an earlier assumption, not a fix,
  and weakens defensive handling without independent proof.

#### 10. Dark Shamans Kardris encounter engage

- **Current-tree behavior:** the callback uses `instance` to locate Haromm,
  then separately guards `SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me)`.
- **Candidate behavior:** removes the frame-send guard.
- **FACT:** `instance` is assigned from `me->GetInstanceScript()`; no
  assertion establishes it. Earlier use again makes a null call already
  unsafe but does not prove a non-null API invariant.
- **UNKNOWN:** raid database placement may supply the practical invariant.
- **State:** `RISKY_GUARD_REMOVAL`.
- **Rationale:** there is no preservation benefit in deleting the check, and
  encounter-frame state is not a suitable cleanup experiment.

### Logic/State Changes

#### 14. Garajal `BeginFight`

- **Current-tree behavior:** returns only when `instance` is non-null and the
  Council state is already `IN_PROGRESS`; a null instance falls through and
  is dereferenced by `SetBossState`, `FillBossGuids`, and subsequent work.
- **Candidate behavior:** changes the condition to return when `!instance` or
  when the state is already in progress.
- **FACT:** for a valid instance, boolean behavior and encounter state
  transitions are identical. For null, the candidate suppresses the entire
  fight-start sequence rather than crashing in this function.
- **FACT:** the AI constructor assigns `instance = me->GetInstanceScript()`;
  helpers and scheduled/update paths elsewhere also use instance state.
- **UNKNOWN:** whether Garajal can validly receive `ACTION_FIGHT_BEGIN` outside
  the intended instance, and whether silent return is the desired recovery,
  require lifecycle/content evidence.
- **State:** `NEEDS_CONTEXT`.
- **Rationale:** this is a coherent local predicate correction, but static
  inspection cannot distinguish a reachable defect from invalid content
  placement or determine the correct failure policy.

## Triage Classification Table

The table assigns exactly one workflow state to each micro-hunk. Historical
outcomes are not reassigned even where their prior decisions correspond to a
redundant or context-dependent result.

| # | Short name | State |
|---:|---|---|
| 1 | Jasper Chains | `ALREADY_REVIEWED` |
| 2 | Heart of Fear Mel'jarak/Garalon | `ALREADY_REVIEWED` |
| 3 | Greenstone player callback | `ALREADY_REVIEWED` |
| 4 | Ritual Lightning predicate | `ALREADY_REVIEWED` |
| 5 | Arena `MemberWon` guard removal | `NO_ACTION` |
| 6 | Raigonn weak-spot death | `PROMISING_STATIC_FIX` |
| 7 | Stone Guard aura guard removal | `RISKY_GUARD_REMOVAL` |
| 8 | Snowdrift novice guard removal | `RISKY_GUARD_REMOVAL` |
| 9 | Garrosh area-trigger object guard | `REDUNDANT_GUARD` |
| 10 | Kardris encounter-frame guard removal | `RISKY_GUARD_REMOVAL` |
| 11 | Norushen target-list null guard | `REDUNDANT_GUARD` |
| 12 | Council engagement instance block | `NEEDS_CONTEXT` |
| 13 | Council death instance guard | `NEEDS_CONTEXT` |
| 14 | Garajal `BeginFight` predicate | `NEEDS_CONTEXT` |
| 15 | Suen summon guard | `REDUNDANT_GUARD` |
| 16 | Chi-Ji summon guard | `REDUNDANT_GUARD` |
| 17 | Niuzao summon guard | `REDUNDANT_GUARD` |

Classification counts are: `ALREADY_REVIEWED` 4,
`PROMISING_STATIC_FIX` 1, `REDUNDANT_GUARD` 5, `NEEDS_CONTEXT` 3,
`RISKY_GUARD_REMOVAL` 3, `POSSIBLE_LOGIC_FIX` 0, and `NO_ACTION` 1.

## Suggested Future Isolated Reviews

Only these three remaining units merit priority. Selection is not approval to
implement them.

### 1. Raigonn weak-spot `JustDied`

- **Why:** it is the sole remaining `PROMISING_STATIC_FIX`, protects the first
  local instance dereference, and has narrow valid-input equivalence.
- **Exact scope:** `npc_raigonn_weak_spotAI::JustDied` in
  `GateOfTheSettingSun/boss_raigonn.cpp`.
- **Suspected defect:** death outside a bound instance dereferences null before
  the existing guard.
- **Proof/disproof evidence:** trace every creation/template binding of the
  weak spot, document `Creature::GetInstanceScript()` guarantees, and inspect
  whether supported summons can outlive or lack the instance script.
- **Expected build target if changed:** incremental `scripts`, then
  `worldserver` link.
- **Eventual runtime validation:** kill the weak spot in Gate of the Setting
  Sun, confirm encounter-frame removal and Raigonn phase action; separately
  exercise any proven supported missing-instance path.

### 2. Garajal `BeginFight` predicate

- **Why:** the candidate's boolean change is locally complete and preserves
  valid-instance state behavior, but requires a focused lifecycle decision.
- **Exact scope:** `npc_garajal_AI::BeginFight` in
  `ThroneOfThunder/boss_council_of_elders.cpp`.
- **Suspected defect:** null instance falls through a condition that visually
  resembles a guard and is then dereferenced multiple times.
- **Proof/disproof evidence:** enumerate `ACTION_FIGHT_BEGIN` senders, prove
  Garajal placement/instance binding, and determine whether fail-fast or
  silent no-op is the framework-consistent policy.
- **Expected build target if changed:** incremental `scripts`, then
  `worldserver` link.
- **Eventual runtime validation:** initiate and reset Council of Elders;
  confirm boss state, combat pull, GUID population, soul scheduling, and
  achievement polling with a build-18414 client and deployed content.

### 3. Council base `JustEngagedWith`

- **Why:** it isolates a first dereference and an encounter/helper transition,
  while exposing whether partial guards are appropriate in a lifecycle whose
  `Reset` still assumes an instance.
- **Exact scope:** `boss_council_of_elders_baseAI::JustEngagedWith` in
  `ThroneOfThunder/boss_council_of_elders.cpp`.
- **Suspected defect:** a missing instance crashes before the existing
  conditional GUID expression can have any protective effect.
- **Proof/disproof evidence:** establish `ScriptedAI`/instance-map creation
  invariants, trace councillor spawns, and determine whether `Reset` makes a
  null engagement categorically unsupported.
- **Expected build target if changed:** incremental `scripts`, then
  `worldserver` link.
- **Eventual runtime validation:** engage every councillor and verify encounter
  frames, Garajal action dispatch, aggro talk, event initialization, and reset
  behavior. A proven supported null-instance path would need its own targeted
  reproduction.

## Candidate 9 Aggregate Findings

- **Total logical micro-hunks:** 17.
- **Previously reviewed:** 4; **remaining triaged here:** 13.
- **Accepted/backported:** 1 (Task 014 Ritual Lightning).
- **Rejected/redundant:** 7 outcome-equivalent units: two prior rejections
  (Tasks 011 and 013) plus five new `REDUNDANT_GUARD` classifications.
- **Needs context:** 4 outcome-equivalent units: Task 012 plus three new
  `NEEDS_CONTEXT` classifications.
- **Risky guard removals:** 3.
- **Promising future static reviews:** 1.
- **Possible logic fixes:** 0. Garajal changes a predicate, but this review
  conservatively classifies it `NEEDS_CONTEXT`, not `POSSIBLE_LOGIC_FIX`.
- **No action:** 1 guard-removal cleanup.

The outcome counts deliberately cross-reference the four historical results,
whereas the classification table gives each hunk exactly one Task 015 state.
They therefore should not be summed as independent buckets.

**FACT:** Candidate 9 combines API-contract-redundant guards, partial instance
hardening, unproven guard removals, one proven predicate repair, and one
cleanup-only deletion. **INFERENCE:** the aggregate subject, “Fix multiple
null pointer dereference issues and redundant checks,” is not evidence that
every hunk is correct. Each micro-unit needs its own contract and behavior
case; Candidate 9 remains unsuitable for aggregate import. No overall quality
score is assigned.

## Runtime/Content Questions Still Open

- Are the Raigonn weak spot, Council members, Garajal helper, Stone Guard
  controller, Snowdrift novice, and Kardris guaranteed by instance maps and DB
  bindings to have a live `InstanceScript` throughout their callbacks?
- Can any supported despawn, unload, summon, or encounter-reset ordering make
  a cached instance pointer unavailable, or is null possible only after an
  invalid spawn/configuration?
- If Garajal lacks an instance, should `BeginFight` silently return, log a
  configuration error, or treat the state as an invariant violation?
- Do encounter-frame operations and helper actions remain paired correctly in
  all Council and Raigonn reset/death orderings?
- Does deployed spell data or another script ever mutate the Lingering
  Corruption target list to contain null? The core search producer does not.
- Runtime/content answers require the persistent server, its databases and
  extracted build-18414 data. No runtime claim is inferred from this static,
  documentation-only triage.
