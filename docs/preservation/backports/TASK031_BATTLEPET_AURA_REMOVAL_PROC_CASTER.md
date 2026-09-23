# Task 031: Battle-Pet Aura Removal Proc Caster Identity

## Starting State

- **FACT:** Review began from a clean `work` branch at
  `2bdda51c00d6ad16e31735fa4aead7d28d5fbbcf`, the integration merge of Task
  030. Although the checkout is named `work`, its head is the current
  preservation integration history requested by this task.
- **FACT:** Task 029's `7d37db0` one-line repair is present: aura reversal
  records each raw additive modifier rather than the resulting absolute state.
  Task 025's finalization guard is present: `EndBattle` returns when already
  finished and sets `Finished` before finalization side effects. Task 030's
  `091570f` is documentation-only and made no C++ change.
- **FACT:** `/tmp/mop-preservation-dev/CMakeCache.txt` was absent at the start.
  ccache reported 0.0 GiB used of 5.0 GiB. This review did not configure a
  build tree because its decision gate does not authorize a source change.
- This task changes neither aura timing nor stacking, delayed auras, weather,
  effect mappings, trainers, SQL, Playerbots, opcodes, or packet layout.

## Andrew Provenance

- **FACT:** The Andrew repository was cloned temporarily without adding a
  preservation remote. Commit
  `867b69b0d142efaaea10b83970342900d3405f2e` has the single parent
  `1e138b1f1d7e1660eb28b647b7193afe961acb72`.
- **FACT:** Its `BattlePetAura.cpp` patch contains two distinct changes. It
  defers duration expiry until after processing effects, and separately changes
  the removal-proc call from
  `Cast(m_target, m_ability, m_turn, ON_AURA_REMOVED)` to the same call with
  `m_caster`. The latter is mechanically a one-token identity substitution;
  it does not require the timing hunk or weather code.
- **FACT:** Path-limited ancestry inspection from `867b69b0` through final
  requested Andrew commit
  `bd8ad5515418d94abd35ba7bf71430979243a09c` finds no later
  `BattlePetAura.cpp` commit. The final file retains `m_caster`.
- **INFERENCE:** Mechanical independence makes an isolated backport possible,
  but says nothing about its build-18414 semantic correctness. No Andrew patch
  was applied.

## Aura Identity Contract

- **FACT:** `PetBattle::AddAura(source, target, ability, abilityEffect, ...)`
  constructs `BattlePetAura(..., source, target, ...)` and stores the resulting
  `unique_ptr` in `target->Auras`. Thus `m_caster` is the pet supplied as the
  source of the aura-producing effect; `m_target` is both the affected pet and
  the owner of the aura container; and `m_ability` is the triggered aura
  ability whose turn/effect/state rows are processed.
- **FACT:** `m_id` is an aura-instance identifier allocated as one more than
  the largest current ID in that target's aura list. It identifies client aura
  apply/change/remove events; it is not a pet identity.
- **FACT:** The constructor accepts the triggering effect ID as `trigger`, but
  its initializer list does not assign that argument to `m_trigger`.
  `m_trigger` consequently retains its in-class zero initializer and is unused
  elsewhere in `BattlePetAura`. This pre-existing issue does not choose a
  removal-proc source and is out of scope.
- **FACT:** Positive and toggle handlers can create self auras (`source ==
  target`). Negative and periodic target handlers can create an aura on the
  active opponent (`source != target`, opposing teams). The generic API also
  permits distinct same-team source/target pointers, although the currently
  implemented targeted handlers do not demonstrate such a row.
- **FACT:** Death only changes a state; it does not destroy or remove the
  `BattlePet`. Swaps replace the team's active pointer, not roster objects.
  Both aura pointers therefore remain roster-backed during ordinary battle
  lifetime. End-battle cleanup calls `OnExpire` before clearing aura owners.
- **UNKNOWN:** The source does not express a retail guarantee about firing a
  proc from a dead or inactive original caster. Pointer lifetime is safe, but
  gameplay permission is a different question.

## PetBattle::Cast Caster Semantics

The first `BattlePet*` argument is materially the acting/source pet:

| Dependency | Current-source behavior |
| --- | --- |
| Ability-state rows | On turn zero, additive ability states are written to the passed caster, with it as both event source and target. Removal expiry passes a nonzero aura turn, so this normally does not execute there. |
| Team and turn source | Effects and state/health events obtain their turn from the passed caster's team. |
| Target selection | `CASTER` selects the passed pet; `TARGET` selects the active pet of the opposite team; `HEAD` selects both of those; `CASTER_TEAM_UNCONDITIONAL` walks the passed pet's team; `ALL` walks both complete rosters. |
| Power and accuracy | Hit chance reads caster accuracy and level; damage/heal scaling reads caster power and dealt modifiers. |
| Family | Damage affinity uses the ability's family against the selected target's family, while caster-family passive state (for example beast) is read from the passed pet. |
| Critical and defenses | Current critical calculation reads the selected target's critical state; dodge, taken modifiers, magic passive, health, and level are also target-relative. Changing caster can still change which pet is selected. |
| Damage bookkeeping | Successful damage writes `DID_DAMAGE_THIS_ROUND` and `LAST_HIT_DEALT` to the passed caster and target-side counterparts to the selected target. |
| Healing | Healing-dealt power/modifiers come from the passed caster; healing-taken comes from the selected target. |
| Proc conditions | Implemented handlers read required caster states, first-attacking team, ramping state, and last-hit state relative to the passed caster. |
| Events | Aura, health, state, catch, and related effects use the passed caster's global index as source. |
| New aura ownership | Aura handlers pass effect `m_caster` into `AddAura`, so any aura created by a removal proc records the `Cast` argument as its source. |
| Cooldown | After effect execution, `Cast` searches the passed caster's equipped abilities and may update a matching cooldown. |

**FACT:** `Cast` itself has no `IsAlive`, active-pet, or `CanAttack` guard.
Those checks exist in `PetBattleTeam::DoCasts`, but `BattlePetAura::Expire`
calls `Cast` directly. Consequently either a dead/inactive `m_target` today or
a dead/inactive `m_caster` after Andrew's change can mechanically execute.

## ON_AURA_REMOVED Proc Contract

- **FACT:** `PET_BATTLE_ABILITY_PROC_ON_AURA_REMOVED` has numeric value **5**.
  It is passed only by `BattlePetAura::Expire` and consumed by `PetBattle::Cast`
  when matching `BattlePetAbilityTurnEntry::ProcType`; effect records are then
  joined by `AbilityTurnId`. It is turn metadata, not an effect property or a
  standalone callback record.
- **FACT:** Nearby enum values are apply 0, damage taken/dealt 1/2, heal
  taken/dealt 3/4, round start/end 6/7, turn 8, ability 9, and swap in/out
  10/11. Current runtime explicitly schedules round-start and round-end procs
  through the active pet. Most other enum values have no distinct scheduler in
  this implementation. There is no separate on-death enum here.
- **FACT:** Aura per-turn effects bypass `Cast`: `BattlePetAura::Process`
  explicitly uses stored `m_caster` and stored `m_target`. Aura apply and
  expiry state events likewise use stored `m_caster` as source. Only the
  removal-proc `Cast` substitutes stored `m_target` as acting pet.
- **INFERENCE:** That asymmetry makes Andrew's choice structurally plausible,
  but neither enum ordering nor neighboring incomplete proc paths establishes
  the retail owner of proc type 5.
- **UNKNOWN:** No current comment, DB2 row annotation, packet capture, or test
  states whether proc type 5 belongs to the aura applier, aura bearer, removal
  actor, or metadata-specific context.

## Expire Call Paths

| Path | Original caster / aura target | Removal actor | Current proc caster | Andrew difference |
| --- | --- | --- | --- | --- |
| Natural duration (`Process`) | Stored pair | Time/aura | target | original caster when distinct |
| Prowl 543 special removal | Stored pair | Aura processing after active pet damage state | target | original caster when distinct |
| Generic duration -1 next-attack removal | Stored pair | Aura processing after active pet damage state | target | original caster when distinct |
| Toggle effect 76 | Its lookup is on effect caster, so normally self/self | Current effect caster | target == caster | none |
| `maxAllowed` replacement | Old stored pair | A new application by possibly another source | old target | old original caster when distinct |
| Victim death (`Kill`) | Stored pair | Killer/effect causing death | dying target | original caster when distinct |
| Battle end | Stored pair | Battle finalization | no `Expire`; no removal proc | none |

- **FACT:** Repository-wide call search found no other battle-pet `Expire`
  caller. There is no general explicit dispel/removal handler beyond the
  implemented toggle and replacement mechanisms.
- **INFERENCE:** Neither implementation uses the actor that caused removal.
  Current always chooses aura bearer; Andrew always chooses original applier.
  The removal path therefore cannot make identity vary by remover without a
  broader API/model change.

## Self / Friendly / Hostile Cases

- **Self:** **FACT:** `m_caster == m_target`; current and Andrew are identical.
- **Friendly distinct target:** **FACT:** The generic model supports it.
  Current makes the recipient the proc source and resolves `TARGET` against
  the recipient's opponent; Andrew makes the applier the source and resolves
  relative to the applier's team. If both are on one team, the active opponent
  is normally the same, while `CASTER`, source index, stats, cooldown, and
  caster-state conditions still differ.
- **Hostile/debuff:** **FACT:** Implemented negative/periodic handlers establish
  this case. Current reverses perspective: the debuffed victim becomes caster,
  so `CASTER` means victim and `TARGET` means the victim's active opponent
  (usually the original applier's side). Andrew preserves the applier's
  perspective: `CASTER` means applier and `TARGET` means the opposing active
  pet (usually the aura bearer only while it remains active).
- **UNKNOWN:** Without actual proc-type-5 rows and expected outcomes, neither
  perspective can be declared the build-18414 contract.

## Proc Target Selection

- **FACT:** Removal-proc target selection does not receive the stored aura
  target, stored trigger, or removal actor. `Cast` supplies only the selected
  caster, ability ID, aura turn, and proc type; `AddTargets` uses the handler's
  hard-coded effect-property target class relative to that selected caster.
- `CASTER` selects the passed identity. For hostile aura A on B, current selects
  B while Andrew selects A.
- `TARGET` selects the active opponent of the passed identity. Current selects
  the active pet on A's team; Andrew selects the active pet on B's team. It is
  not guaranteed to be the stored aura target after a swap.
- `HEAD` selects the passed identity plus its active opponent, so changing the
  caster changes at least the same-team member and potentially both members.
- `CASTER_TEAM_UNCONDITIONAL` selects every roster pet on the passed identity's
  team. For a hostile aura the team is completely inverted.
- `ALL` selects both rosters and is target-set invariant, but source identity,
  source stats, event source, and caster-state conditions still differ.
- `NONE` selects nobody and `Execute` returns false. Other numeric target kinds
  shown in comments are not implemented by current `AddTargets`.

## Damage / Heal / State Consequences

| Result family | Observable consequence of choosing target versus original caster |
| --- | --- |
| Damage | May invert opponent selection; changes accuracy/level comparison, power, dealt modifiers, beast passive, source last-hit flags, team turn, source index, and downstream killer identity. Ability-family ID itself is unchanged. |
| Heal | `CASTER` heals a different pet; power and healing-dealt modifier come from a different pet; source index/team turn differ. |
| Set state | Hard-coded `CASTER` state effect writes a different pet and reports a different source. Target-oriented state effects can be team-inverted. |
| Apply aura | New aura's source becomes whichever pet was passed to `Cast`; its target is recomputed rather than inherited from the expiring aura. |
| Conditional/ramping effects | Reads/writes caster state, last-hit information, and first-attacking team for a different pet. |
| Accuracy/critical | Accuracy and caster level differ directly. Current critical formula is target-based, so it changes when target selection changes rather than merely because source changes. |
| Cooldown | A matching equipped ability, if any, is searched on a different pet. |

**FACT:** Identity is therefore semantically and client-observably material.
That proves the choice matters, not which choice is correct.

## Build-18414 Evidence

- **FACT:** Repository-local source contains DB2 structures and loaders but no
  extracted build-18414 `BattlePetAbilityTurn`/`BattlePetAbilityEffect` row
  dump, fixture, or test identifying a proc-type-5 ability. Documentation and
  SQL searches yielded no such rows.
- **FACT:** Internet source search was attempted, but the available web search
  service returned HTTP 401. The two accessible source forks provide
  implementations, not authenticated build-18414 row-level evidence.
- **UNKNOWN:** No independent row, 5.4.8 client packet trace, or retail outcome
  was recovered that joins proc type 5 to an effect property and aura use case.

`BUILD18414_REMOVAL_PROC_EVIDENCE = INSUFFICIENT`.

## Affected Ability Examples

**UNKNOWN:** No concrete build-18414 proc-type-5 ability row was recovered.
Accordingly this report does not guess ability IDs, names, effect targets, or
self/friendly/hostile classifications. Prowl 543 is a concrete special expiry
caller, but current local data does not prove that its aura ability has a
proc-type-5 turn, so it is not presented as an affected example.

## Flamehawk Comparison

- **FACT:** Flamehawk final relevant commit
  `e91819a4c2acb6cc8eea09ed264d30aec6ed12b9` uses
  `Cast(m_target, m_ability, m_turn, ON_AURA_REMOVED)` in `Expire`.
- Classification: **`AGREES_WITH_CURRENT`**.
- **INFERENCE:** Flamehawk and current share close lineage and identical nearby
  code, so this is corroboration of ancestry/current behavior rather than
  independent retail evidence.

## Other Lineage Comparison

- **FACT:** Andrew final through `bd8ad551` uses original `m_caster`; it is a
  MoP-derived fork but the change is the candidate under review and is not
  independent build-18414 evidence.
- **FACT:** Flamehawk uses `m_target` and is likewise a MoP 5.4.8-derived fork,
  but is closely related to the current implementation.
- **UNKNOWN:** No additional independently derived implementation with a
  demonstrated build-18414 data or packet basis was recovered. Newer-expansion
  semantics were not imported.

## Client Event Source Identity

- **FACT:** The aura-removal event itself always reports stored `m_caster` as
  source and stored `m_target` as aura target before invoking the proc. The
  proposed identity substitution does not alter that event.
- **FACT:** Every proc-produced health/state/aura/catch event constructed by
  implemented handlers uses effect `m_caster->GetGlobalIndex()` as its source
  and usually uses that caster's team turn. Thus current removal event source
  can say A while its immediately following proc event says B for hostile aura
  A-on-B; Andrew would normally keep A as both sources.
- **FACT:** Target indices depend on `AddTargets`, so hostile `TARGET`,
  `CASTER`, `HEAD`, and team-wide effects can report different target pets as
  well as a different source. Damage/heal source, state source, and newly
  created aura source all follow the passed caster.
- **INFERENCE:** Source continuity favors Andrew structurally, but no packet
  trace establishes that continuity as the retail requirement; deliberate
  bearer-owned expiration effects remain possible.

## Original Caster Death / Inactive Cases

- **FACT:** A dead, swapped-out, or otherwise inactive original caster remains
  a valid roster pointer. Direct `Cast` and `AddTargets` do not require it to be
  alive or active; `AddTarget` has its alive filter commented out. Andrew's
  call is therefore memory-safe under current normal lifetime and mechanically
  executable.
- **FACT:** `TARGET` means the opposite team's *current active pet*, not the
  stored aura target. A swapped-out original caster still targets relative to
  its team. Cooldown and caster states remain attached to the inactive/dead
  roster pet.
- **UNKNOWN:** Current source cannot establish whether retail permits a
  removal proc to originate from a dead or inactive original aura caster.
  Mechanical permissibility is not semantic proof. Substituting an alive aura
  target would likewise be an unsupported policy.

## Aura Target Death Case

- **FACT:** `Kill` first iterates and expires every victim aura, then calls
  `UpdatePetState(... IS_DEAD, 1 ...)`. During removal procs the victim still
  reports alive by the death state, although its health has already been set to
  zero by `SetHealth` before `Kill` is entered.
- **FACT:** Current therefore casts a hostile aura's removal proc as the
  zero-health, not-yet-`IS_DEAD` victim. The proc can select targets and can
  damage, heal, apply aura, or update state before the victim's death-state
  event. Andrew instead casts from the original applier, even if that pet is
  dead/inactive, and can select a different side.
- **INFERENCE:** The ordering makes the current substitution especially
  observable and unusual, but it does not prove retail ownership. Health-zero
  and death-state ordering itself may be part of a broader death/proc contract.

## EndBattle Behavior

- **FACT:** `EndBattle` directly calls `aura->OnExpire()` and clears each aura
  list. It does not call `Expire`, does not emit ordinary aura-remove events,
  and does not fire `ON_AURA_REMOVED` procs during normal final cleanup.
- This Task 029-relevant state reversal remains unchanged. Task 031 does not
  propose changing end-battle behavior.

## Semantic Invariant

Candidate evaluation:

- **A, original aura caster:** structurally consistent with aura process,
  apply/expire state-event ownership, and aura-remove event source; mechanically
  safe for dead/inactive roster pets. **Not established semantically** without
  rows/traces showing proc type 5 is applier-owned.
- **B, aura target:** implemented by current and Flamehawk, and could represent
  bearer-owned expiration behavior. **Not established semantically**; it is
  asymmetric with all other stored aura source usage and reverses hostile
  targeting perspective.
- **C, removal actor:** unsupported by either implementation and impossible to
  express through current `Expire()` because it receives no actor.
- **D, metadata-dependent:** no field or dispatch currently chooses the source;
  data could imply different conventions, but no rows were recovered.
- **E, insufficient evidence:** supported.

**INFERENCE:** Passing stored `m_caster` is the cleaner internal ownership
model, but the task's gate correctly requires more than internal elegance.
The deciding build-18414 proc rows and expected target/source outcomes are
absent, and dead/inactive-caster permission is unknown.

## Final Classification

**`NEEDS_BUILD18414_DATA`.** No C++ source was changed.

The roles are unambiguous and `Cast` identity is materially observable, but
decision-gate requirements 3, 4, 5, 6, 7, and 8 cannot all be certified.
Specifically, the build-18414 owner of proc type 5, concrete self/friendly/
hostile rows, dead/inactive permission, intended relative targets, and expected
client-visible source are missing. Andrew's mechanically isolated change is
therefore not justified for preservation merely because it makes the code more
internally consistent.

Remaining A7 work is unchanged: obtain authoritative build-18414 aura-removal
proc rows and packet/runtime outcomes; separately review delayed harmful auras,
expiration timing, and stacking policy; and retain Task 030's broader boolean
ownership blocker. Weather and A8 remain separate work.

## Validation Plan

- **FACT:** Static review covered self, hostile, generic friendly-target,
  natural expiry, special next-attack expiry, toggle removal, replacement,
  target death, original-caster death/inactivity/swap, `CASTER`, `TARGET`,
  `HEAD`, `ALL`, caster-team targeting, damage, healing, aura application,
  and state updates. Every distinct-identity case is materially changed, but
  none supplies the missing retail owner contract.
- **FACT:** `git diff --check` is the applicable repository validation. With no
  source change, the exact `BattlePetAura.cpp.o` compilation, aggregate `game`
  build, and implementation-only 14-case post-change matrix are not applicable.
- A future source patch must first recover authoritative proc-type-5 rows and
  expected outcomes, prove dead/inactive-caster behavior, then make only the
  one-token `m_target`/`m_caster` change if that evidence selects invariant A.
  It must compile the exact generated `BattlePetAura.cpp.o` before an
  implementation commit and must not import Andrew's timing hunk.
