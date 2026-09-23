# Task 013: Greenstone Village Player-Guard Review

## Review scope and starting state

- **Starting commit:** `e4d47ceae8681ac19c18ee25605e86d279bce2ab`
  (`Merge pull request #12 from
  firstuserlastname123/codex/review-heart-of-fear-instance-guard`). The working
  tree was clean on the supplied task branch `work`. This is the merge of Task
  012 into the supplied `preservation/main` integration history; `master` was
  not checked out.
- **Scoped function:** only
  `AreaTrigger_at_behind_tzu::OnTrigger(Player*, AreaTriggerEntry const*)` in
  `src/server/scripts/Pandaria/Scenarios/GreenstoneVillage/greenstone_village.cpp`
  was reviewed. No other Candidate 9 change is approved by this review.
- **Decision:** no source change. The callback contract and every actual core
  invocation establish a valid `Player*` before the script callback runs.
  Candidate 9 moves a redundant local test; it does not protect a supported
  null-player path.

## Candidate 9 provenance

- **Repository:**
  `https://github.com/MityaFoxy/Legends-of-Azeroth-Pandaria-5.4.8.git`.
- **Documented branch:** `master`.
- **Full commit:** `481dd4bc5d82a7caed094ad175f54ed1c3c1d4a3`.
- **Subject:** `Fix multiple null pointer dereference issues and redundant checks`.
- **Author and author date:** google-labs-jules[bot]
  `<161369871+google-labs-jules[bot]@users.noreply.github.com>`,
  2026-09-15 05:51:49 +0000. The committer and commit date are identical;
  the message also credits MityaFoxy as co-author.
- **Inspection:** the exact commit was fetched into temporary Git ref
  `refs/task013/candidate9-commit`. Its 12-file aggregate was inspected using
  `git show --stat`, `git show --summary`, and `git show --format=fuller`.
  Only its Greenstone Village file diff was then inspected. The commit was not
  cherry-picked.

The exact candidate hunk is:

```diff
 bool OnTrigger(Player* player, AreaTriggerEntry const* trigger) override
 {
+    if (!player)
+        return false;
+
     if (InstanceScript* instance = player->GetInstanceScript())
     {
-        if (player && player->IsOnVehicle() && player->GetVehicleBase() && player->GetVehicleBase()->ToCreature())
+        if (player->IsOnVehicle() && player->GetVehicleBase() && player->GetVehicleBase()->ToCreature())
```

No spell ID, instance lookup, creature lookup, credit cast, AI action, or
return value is otherwise changed by this hunk.

## Existing code, candidate code, and evaluation order

Current code first evaluates `player->GetInstanceScript()` to initialize the
condition's local `instance`. If the result is non-null, it evaluates the
vehicle condition left-to-right: `player`, `player->IsOnVehicle()`,
`player->GetVehicleBase()`, and `player->GetVehicleBase()->ToCreature()` with
short-circuit `&&`. It then casts `SPELL_KEG_DELIVERY_CREDIT`, looks up
`NPC_BREWMASTER_TZU` from the instance GUID, and invokes `ACTION_SPECIAL_1`
when the owner and its AI exist. It always returns `false`.

Thus **FACT:** `player->GetInstanceScript()` dereferences `player` before the
function's existing local `player` truth test. That ordering alone does not
show that a null value is permitted by the callback contract.

Candidate 9 would test `!player` first and return `false`, then perform the
same instance lookup. It removes only the now-dominated `player &&` operand
from the vehicle condition. **FACT:** for a valid player, the candidate does
not change the instance, vehicle, creature-conversion, credit-spell, owner-AI,
or return behavior.

## AreaTrigger API contract and actual call path

### Facts

1. `AreaTriggerScript` describes `OnTrigger` as called when an area trigger is
   activated **by a player** and declares its argument as `Player*`.
2. The only core invocation of an `AreaTriggerScript::OnTrigger` override is
   in `ScriptMgr::OnAreaTrigger`. That dispatcher begins with
   `ASSERT(player)` and `ASSERT(trigger)`, optionally offers the already-valid
   arguments to Eluna, resolves the registered script, and calls `OnTrigger`
   only for entry events (or `OnExit` for exit events).
3. The only core caller of `ScriptMgr::OnAreaTrigger` is
   `WorldSession::HandleAreaTriggerOpcode`. It obtains `Player* player` from
   the logged-in session with `GetPlayer()`, immediately dereferences it for
   the flight check, resolves the DBC area-trigger entry, verifies that the
   packet's entry/exit state matches the player's actual position, checks
   conditions, and then passes that same pointer to `ScriptMgr`.
4. `CMSG_AREATRIGGER` registers this handler with `STATUS_LOGGEDIN`. The
   handler therefore runs in the session state that supplies a player.
5. A repository-wide search found no second core call to
   `ScriptMgr::OnAreaTrigger` and no second invocation of an
   `AreaTriggerScript::OnTrigger` override. The similarly named
   `SpellAreaTriggerScript`/`IAreaTrigger` callbacks use `WorldObject*` or
   `Unit*` and are a separate API; they cannot invoke this override.
6. `AreaTrigger_at_behind_tzu` is registered once by
   `AddSC_greenstone_village`. No direct or alternate invocation of its
   `OnTrigger` method exists.

### Inferences

- The API comment, logged-in session origin, pre-callback dereferences, and
  dispatcher assertions jointly define a non-null-player callback contract.
  Passing null directly to the public dispatcher from hypothetical new code
  would violate that contract rather than create a supported callback path.
- Because the only normal call path already requires a live `Player` object,
  Candidate 9's early test is redundant. It may silence a path-insensitive
  static-analysis warning, but it does not repair a reachable supported null
  dereference.

### Unknowns

- No crash reproduction or build-18414 scenario runtime result accompanied
  Candidate 9.
- Static inspection cannot prove the overall scenario's gameplay accuracy,
  database binding, vehicle-credit behavior, or deployed runtime health.
- Future code could violate the current dispatcher contract by introducing a
  new direct call with null. Such hypothetical unsupported code is not a basis
  for changing preservation behavior now.

## Explicit review answers

1. **Does current code dereference before its local test? — FACT: yes.** The
   instance lookup is evaluated before the later `player &&` operand.
2. **Does the API contract guarantee non-null? — FACT: yes.** The interface is
   player-activation-specific, and `ScriptMgr::OnAreaTrigger` asserts both
   callback arguments before dispatch.
3. **Do all actual core call sites establish a valid player? — FACT: yes.**
   There is one dispatcher invocation, from the logged-in session opcode
   handler; that handler already dereferences the same session player before
   dispatch, and the dispatcher asserts it.
4. **Is there a supported null-player callback path? — FACT: no path was
   found.** A direct null call would violate dispatcher preconditions.
5. **What does Candidate 9 protect? — FACT:** it moves a redundant local guard;
   it does not protect a supported null path.
6. **Does it alter valid-player behavior? — FACT: no.** The removed `player &&`
   is true under the contract, and all subsequent logic is retained.
7. **Does it depend on another Candidate 9 change? — FACT: no.** The rearrangement
   is mechanically independent of all other Candidate 9 hunks.

## Decision, dependency, and validation record

- **Decision:** reject the isolated source backport because static analysis
  establishes a non-null `Player*` contract and no supported null path.
- **Source modification status:** unchanged. The scoped source blob remains
  the starting blob `2110e507e338e2595b5b7954d1a557122c84c27f`.
- **Exact hunk applied:** none. Candidate 9 was neither cherry-picked nor
  applied, exactly or in adapted form.
- **Dependencies:** none on another Candidate 9 hunk. Independence does not
  make the redundant guard necessary.
- **Intended validation if justified:** review the isolated source diff, build
  the `scripts` target, link `worldserver`, re-read the scoped function, and
  retain scenario runtime testing for the persistent build-18414 environment.
- **Scripts build:** not run because no C++ source changed.
- **Worldserver build:** not run for the same reason. A build would not validate
  a documentation-only rejection.
- **Final static validation:** the scoped function and its source blob were
  rechecked against the starting commit; no source or other Candidate 9 hunk
  entered the task.
- **Remaining runtime uncertainty:** the persistent build-18414 environment is
  still required to validate overall Greenstone Village scenario behavior,
  database bindings, vehicle state, credit delivery, and owner AI behavior.
  There is no source modification from this task to runtime-validate.
