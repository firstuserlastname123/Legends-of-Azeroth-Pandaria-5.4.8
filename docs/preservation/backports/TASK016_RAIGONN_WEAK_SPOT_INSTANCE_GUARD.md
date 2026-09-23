# Task 016: Raigonn Weak-Spot Instance Guard Review

## Review checkpoint

- **Starting commit:** `02f4cc891bd926f6a96b62ba3446ec8429a7f9c3`
  (`Merge pull request #15 from
  firstuserlastname123/codex/triage-remaining-candidate-9-hunks`). The supplied
  task branch is `work`, its tree was clean, and this commit is the current
  preservation integration history immediately after Task 015.
- **Candidate repository and branch:**
  `https://github.com/MityaFoxy/Legends-of-Azeroth-Pandaria-5.4.8.git`,
  `master`.
- **Candidate commit:**
  `481dd4bc5d82a7caed094ad175f54ed1c3c1d4a3`, `Fix multiple null pointer
  dereference issues and redundant checks`, authored and committed by
  google-labs-jules[bot] `<161369871+google-labs-jules[bot]@users.noreply.github.com>`
  on 2026-09-15 05:51:49 +0000, with MityaFoxy credited as co-author.
- **Exact scope:** only `npc_raigonn_weak_spotAI::JustDied` in
  `src/server/scripts/Pandaria/GateOfTheSettingSun/boss_raigonn.cpp`. No other
  Candidate 9 hunk is in scope.

## Exact candidate hunk

The candidate parent and current preservation tree both execute this code:

```cpp
void JustDied(Unit* /*killer*/) override
{
    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
    if (instance)
        if (Creature* raigonn = Unit::GetCreature(*me, instance->GetGuidData(DATA_RAIGONN)))
            if (raigonn->IsAIEnabled)
                raigonn->AI()->DoAction(ACTION_WEAK_SPOT_DEAD);
}
```

Candidate 9 changes it to:

```cpp
void JustDied(Unit* /*killer*/) override
{
    if (instance)
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        if (Creature* raigonn = Unit::GetCreature(*me, instance->GetGuidData(DATA_RAIGONN)))
            if (raigonn->IsAIEnabled)
                raigonn->AI()->DoAction(ACTION_WEAK_SPOT_DEAD);
    }
}
```

The hunk therefore remains byte-for-byte conceptually applicable to the
current tree.

## Creation and binding lifecycle

- **FACT:** `NPC_WEAK_SPOT` is creature entry `56895`.
- **FACT:** the only repository C++ creation site using that identifier is
  Raigonn's `EVENT_INITIALIZE`: Raigonn calls
  `me->SummonCreature(NPC_WEAK_SPOT, *me)`, puts the result into vehicle seat
  zero, and, when its own instance pointer exists, records the weak spot GUID
  in the Gate instance.
- **FACT:** the instance script does not create the weak spot. Its
  `OnCreatureCreate` switch records Raigonn and other encounter creatures but
  has no `NPC_WEAK_SPOT` case; its `SetGuidData`/`GetGuidData` methods merely
  store and return the GUID supplied by Raigonn.
- **FACT:** no repository `vehicle_template_accessory` row creates entry
  `56895`. The vehicle system is used after the explicit summon: the weak spot
  enters Raigonn, and artillery later transfers passengers into the weak
  spot's own vehicle kit.
- **FACT:** repository SQL contains no creature spawn, creature-template
  ScriptName binding, or vehicle accessory for entry `56895`. The checked-in
  SQL is therefore insufficient to prove the complete deployed world-data
  configuration. No SQL was executed.
- **FACT:** the CreatureScript name is globally registered as
  `npc_raigonn_weak_spot`. Core AI selection dispatches a creature's configured
  script ID without checking its map, and this script's `GetAI` has no
  Gate-map or instance predicate. A template bearing this ScriptName therefore
  receives this AI wherever core creature creation places it.
- **FACT:** generic `Map::SummonCreature` accepts the entry and the caller's
  map; it does not require an instance map. Static creature loading likewise
  has no script-local Gate restriction. Thus a valid core summon or content
  spawn of the configured entry on a non-dungeon map can instantiate this AI
  with no `InstanceScript`. Although that is not the intended Gate encounter
  placement, it is an API-supported path rather than an impossible callback
  state.
- **INFERENCE:** deployed production data probably binds the ScriptName even
  though the binding is absent from this repository, because otherwise this
  C++ AI would never be selected. Whether any deployed database also contains
  a static spawn is **UNKNOWN**.

## Instance initialization and lifetime

- **FACT:** the AI constructor caches exactly
  `creature->GetInstanceScript()` in a raw `InstanceScript* instance` member.
  It does not assert or otherwise require a non-null result.
- **FACT:** `WorldObject::GetInstanceScript()` returns null on maps that are
  neither dungeons nor scenarios. On a dungeon/scenario it returns the
  `InstanceMap`'s `i_data`, which is initialized to null and remains null when
  there is no instance template, no matching `InstanceMapScript`, or the
  script factory returns null. Neither the return type nor implementation is
  a formal non-null contract.
- **FACT:** the intended Raigonn summon inherits Raigonn's Gate map. With map
  962 correctly bound to `instance_gate_of_the_setting_sun`, instance data is
  created and initialized before ordinary encounter operation, so the normal
  configured Gate path supplies the Gate `InstanceScript`.
- **FACT:** `Reset`, `GetPassengerEnterPosition`, and
  `GetPassengerExitPosition` do not access `instance`. AI initialization calls
  `Reset`, so no earlier weak-spot callback necessarily fails when the cached
  pointer is null.
- **FACT:** the cache is assigned only in the constructor; it cannot later
  change from non-null to null. An `InstanceScript` is owned by its
  `InstanceMap`, as is the creature. No supported ordering was found that
  destroys or replaces the instance script, keeps the creature alive for
  gameplay, and then invokes `JustDied`.
- **INFERENCE:** normal map teardown removes objects rather than killing them,
  and therefore does not create a `JustDied` call on a dangling cached
  instance pointer. A teardown-induced death callback was not found.

## Death callback and semantics

- **FACT:** core death processing invokes the selected creature AI's
  `JustDied` for any ordinary creature death when an AI is available. The weak
  spot override has no killer-, summon-, vehicle-, map-, or encounter-state
  filter, so combat/scripted lethal damage to any live instance of entry
  `56895` using this AI can reach it.
- **FACT:** current evaluation order first evaluates `instance->` for
  `SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me)`. This is the first
  `instance` use in `JustDied`; only afterward does `if (instance)` guard the
  GUID lookup and action. A null pointer therefore fails before its local
  check.
- **FACT:** `SendEncounterUnit` is an `InstanceScript` operation and cannot be
  performed without one. The Raigonn lookup also requires the instance's
  `DATA_RAIGONN` GUID. Without an instance there is neither an encounter frame
  owned by this callback nor an instance GUID source for the action target.
- **FACT:** when `instance` is null, Candidate 9 skips both the disengage and
  the Raigonn lookup/action rather than dereferencing null. This matches the
  best-effort `if (instance)` policy already used for encounter-frame and state
  work elsewhere in this Raigonn script; it is defensive behavior for an
  out-of-encounter placement, not a claim that such placement is retail
  content.
- **FACT:** when `instance` is valid, Candidate 9 preserves the exact order:
  disengage the weak spot, fetch `DATA_RAIGONN`, resolve the creature, test
  `IsAIEnabled`, then send `ACTION_WEAK_SPOT_DEAD`. Lookup semantics,
  `ACTION_WEAK_SPOT_DEAD`, phases, states, vehicles, and combat behavior are
  unchanged.
- **INFERENCE:** a non-Gate instance with an unrelated `InstanceScript` is
  also safe under both versions at the first call, although its data-key
  semantics are content-invalid. Candidate 9 addresses nullability only; it
  does not validate instance identity.

## Explicit callback answers

1. **FACT:** yes, current code dereferences `instance` before its local null
   check.
2. **FACT:** yes, the encounter-frame call is its first dereference in
   `JustDied`.
3. **FACT:** no weak-spot lifecycle callback necessarily dereferences it
   earlier; initialization reaches `Reset`, which does not use it.
4. **FACT:** the correctly configured normal Gate/Raigonn summon supplies the
   Gate instance script.
5. **FACT:** the core API does not guarantee it; the API explicitly returns
   null outside instance maps and can return null for missing instance data.
6. **FACT:** yes, the generic core summon/static-spawn machinery can create a
   globally ScriptName-bound entry on a non-instance map; the AI has no map
   gate. The repository proves no deployed static spawn, which remains
   **UNKNOWN**, but a null construction/death path is supported by the API.
7. **FACT:** no supported teardown ordering that nulls the cached pointer while
   preserving the live creature was found; the member itself is never reset.
8. **FACT:** for null it changes a crash at the encounter-frame call into a
   no-op for both paired operations.
9. **FACT:** for valid instances it only adds a dominating branch and braces;
   operation order and effects are unchanged.

## Task 010 interaction

Task 010's accepted changes are in
`BatteringHeadbuttEffectTargetSelector::operator()` and
`spell_raigonn_battering_headbutt_SpellScript::HandleSpellEffectHit`, roughly
330 lines after this callback. They correct the headbutt player filter and add
an active-encounter check. Candidate 9's weak-spot hunk neither overlaps nor
depends on those changes, and applying it must leave Task 010 byte-for-byte
unchanged.

## Decision and planned validation

**Final classification: `STATICALLY_JUSTIFIED`.** The relevant API has an
explicit nullable result; globally bound creature AI and generic creation
machinery provide a supported null-instance route; no earlier weak-spot
lifecycle dereference blocks death; both skipped operations inherently need
the missing instance; and valid Gate behavior is structurally identical. The
intended content path's practical instance guarantee does not establish a
formal core contract or eliminate the separately supported creation route.

Only the exact Candidate 9 `JustDied` hunk will be applied. The persistent fast
development tree will then build `scripts` and link `worldserver`. Final static
review will confirm the guard dominates all local uses, valid-path ordering is
unchanged, Task 010 remains intact, and no other Candidate 9 hunk entered the
tree.

Compilation can prove syntax, API, and link compatibility only. It cannot
prove correct encounter-frame behavior, transition timing, or full Raigonn
encounter correctness. Runtime validation remains required with the deployed
database, extracted client data, and a build-18414 client.

## Final implementation and validation

- **Documentation checkpoint:** `8f17a4b` (`chore(preservation): record
  Raigonn weak-spot guard review`). The working tree was clean immediately
  after that commit and before the source edit.
- **Final classification:** `STATICALLY_JUSTIFIED`.
- **Source result:** changed only
  `npc_raigonn_weak_spotAI::JustDied`. The applied source hunk is
  byte-for-byte Candidate 9's isolated hunk: the unconditional frame call was
  moved into the existing instance condition, braces were added around the
  now-shared guarded block, and the Raigonn lookup/action stayed within it. No
  adaptation and no other Candidate 9 hunk were required.
- **Fast development configuration:**
  `/tmp/mop-preservation-dev/CMakeCache.txt` was absent, so the prescribed
  Release development profile was configured with GCC 13.3.0, ccache
  launchers, and Playerbots, modules, tools, and Eluna disabled. The build and
  install paths are outside the repository.
- **`scripts` build:**
  `cmake --build /tmp/mop-preservation-dev --target scripts --parallel 4`
  succeeded and linked `libscripts.a`, including the changed Gate translation
  unit.
- **`worldserver` build:**
  `cmake --build /tmp/mop-preservation-dev --target worldserver --parallel 4`
  succeeded and linked the `worldserver` executable.
- **Post-build static validation:** the instance condition now dominates the
  first and every later `instance` use in `JustDied`. For valid instances,
  encounter-frame disengage still precedes the unchanged `DATA_RAIGONN`
  lookup, `IsAIEnabled` test, and unchanged `ACTION_WEAK_SPOT_DEAD` action.
  Task 010's headbutt selector and encounter-state guard remain unchanged, as
  do all phase/state constants and surrounding vehicle/combat behavior.
- **Remaining runtime validation:** kill the weak spot during the live Gate of
  the Setting Sun encounter and verify encounter-frame removal, Raigonn's
  weak-spot-dead phase transition, player/vehicle state, reset and evade
  behavior, and full encounter completion. If practical, create and kill the
  ScriptName-bound entry on a non-instance map to demonstrate the guarded
  null-instance path. These checks require the persistent server's deployed
  database and extracted data plus a build-18414 client.

Compilation does not prove encounter correctness or build-18414 retail
accuracy; the runtime checks above remain outstanding.
