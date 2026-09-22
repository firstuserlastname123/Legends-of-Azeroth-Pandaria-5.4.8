# Task 025: PetBattle exactly-once finalization review

## Scope and starting state

Task 025 reviews the current preservation PetBattle finalization topology for
the Mists of Pandaria 5.4.8 build 18414 target. It does not import or implement
trainer support. The review started from clean commit
`291013e537755d4c16b0174271bbd9c0d2f041ed` on the workspace branch `work`.
That commit is the merge immediately following Task 024; Task 021's full
loadout-slot GUID repair and the Task 022--024 reports are present.

At entry, `/tmp/mop-preservation-dev/CMakeCache.txt` was absent
(`DEV_BUILD_TREE_MISSING`) and `ccache --show-stats` reported 0.0 GiB used of
5.0 GiB. No configuration was performed before the decision gate.

## Andrew provenance

A temporary, no-checkout clone at `/tmp/andrew-task025` was used without
altering repository remotes. The relevant full commit identities and subjects
are:

| Commit | Subject |
| --- | --- |
| `39205279cebebd1d2c83000a0e9043e68047e659` | Removing old migrations, adding new ones, battle pet trainers now battle you, but get stuck after you defeat their first pet. Also they stand right in the middle of the battle, but hey |
| `c6400a299c1fbf2df0b8654cbff001ad801e7f82` | Fix pet battle trainer pet death causing client to get stuck |
| `100303ca22b08f04ee5620dc1622617d5a680cdd` | petbattle: fix weather effects, quest credit, and various battle bugs |
| `1e338126a8f3e5381218dc90a50185119beeb592` | Trainer spawns, quests, quest chains and pet swapping mechanics. Also fixed weather effects further |
| `867b69b0d142efaaea10b83970342900d3405f2e` | Battle pet fixes, ability handlers, wild battle spawns. Also improved tillers intro quests. Tillers still needs the farm working, and pet battles still need more wild pet spawns |
| `bd8ad5515418d94abd35ba7bf71430979243a09c` | Tillers farm: Phase 1 + Phase 2.2 planting to harvest retail cycle |

The commits are not one linear parent chain after `c6400a29`; their actual
parents were inspected rather than inferred from abbreviated names. The final
Andrew `PetBattle::EndBattle`, system update/removal, and forfeit functions
retain the same relevant gap as preservation: `Finished` is assigned only
after all effects, while a battle stays discoverable until the system update
removes it. Andrew adds trainer results, increasing consequences, but no
Andrew trainer patch is required for the current-core finding.

## Current `EndBattle` behavior

On a non-world thread, `EndBattle` changes the state to `Interrupted` and
returns without results. On the world thread, it currently accepts any state,
including `Finished`. It computes `m_winningTeam` from the loser (or clears it
for a forced ending), expires and clears every pet aura, resets mechanic
states, and then for each player:

1. applies a ten-percent current-health penalty to every losing pet on a
   forfeit;
2. computes and adds wild-PVE winner XP for living pets in `SeenAction`;
3. sends per-pet collection updates;
4. creates a captured pet and updates both capture criteria when applicable;
5. updates win criteria or resets loss criteria;
6. removes pacified/immune flags and root control; and
7. sends `SMSG_PET_BATTLE_FINAL_ROUND` for an outcome or
   `SMSG_PET_BATTLE_FINISHED` for forced termination.

For each wild creature it also calls `BattlePetSpawnMgr::LeftBattle`. Only
after all of those operations does it assign `PetBattleState::Finished`.
Persistence is indirect: health/XP/capture mutations mark pet records for save
and logout subsequently saves the player. `EndBattle` does not directly queue
or perform deletion.

### Side-effect idempotence

| Side effect | Classification | Duplicate-call consequence |
| --- | --- | --- |
| assign `m_winningTeam` from arguments | `IDEMPOTENT` for identical arguments; `NON_IDEMPOTENT` across overlapping terminal reasons | A later forced call can replace a result with no winner, or another loser can reverse it. |
| aura `OnExpire()` callbacks | `UNKNOWN` | The vector is cleared on the first call, but callback effects are not globally proven idempotent. |
| clear aura vectors | `IDEMPOTENT` | Re-clearing is harmless. |
| reset mechanic states | `LIKELY_IDEMPOTENT` | Reassigns/reset states, but all effect-specific callbacks are not audited. |
| forfeit health reduction | `NON_IDEMPOTENT` | Each call subtracts ten percent of the then-current health. |
| `BattlePet::SetXP(xp)` | `NON_IDEMPOTENT` | `SetXP` adds its argument and can level a pet, so repeated finalization awards XP again. |
| per-pet client update | `LIKELY_IDEMPOTENT` in stored data, not on the wire | It redundantly transmits and can expose already-mutated values. |
| captured-pet `Create` | `NON_IDEMPOTENT` | It can create a second collection pet. |
| capture achievement criteria | `NON_IDEMPOTENT` | Criteria calls can advance twice. |
| win criteria update / loss criteria reset | `NON_IDEMPOTENT` / `UNKNOWN` | Win progress can advance twice; reset internals were not proven harmless. |
| remove player flags and root | `LIKELY_IDEMPOTENT` | Clearing an already-cleared state is normally harmless. |
| final-round or finished packet send | `NON_IDEMPOTENT` protocol effect | The client receives a duplicate terminal sequence. |
| wild-spawn `LeftBattle` | `UNKNOWN` | Manager cleanup was not established as repeat-safe. |
| final `Finished` assignment | `IDEMPOTENT` | Reassigning the enum is harmless but occurs too late to guard effects. |

## Complete current caller inventory

Repository-wide `EndBattle(` search found three PetBattle call sites (plus
unrelated Battlefield methods):

| Caller | Condition and guard | Registration/reentry assessment |
| --- | --- | --- |
| `PetBattle::HandleRound` | After round serialization/cleanup, scans teams in challenger-then-opponent array order; the first team with no living pet is the loser. It breaks after one call. No state guard inside this caller. | The battle remains registered. The same `HandleRound` is not invoked again by normal `Update` once state becomes `Finished`, but another already-supported terminal entry can overlap before removal. |
| `PetBattle::Update` | State `Interrupted` calls `EndBattle(nullptr)` on the world thread. | The cross-thread caller merely sets `Interrupted`; the next 300 ms system update performs results and then schedules/removes it. No finalization guard exists in `EndBattle`. |
| `PetBattleSystem::ForfietBattle(PetBattle*, PetBattleTeam*)` | Only checks both pointers for null, then calls `EndBattle(team, true)`. | It does not check battle state. Direct callers with a retained pointer can call repeatedly while the object lives. |
| `Unit::SetInCombatState` (upstream of `EndBattle`) | A player entering combat looks up its battle and calls `EndBattle(nullptr)`. Map-thread calls become `Interrupted`. | The registry entry remains. More combat/terminal activity can repeat or overlap before world-thread removal. |
| `WorldSession` logout (upstream of forfeit) | Calls `ForfietBattle(player GUID)` during logout. | Lookup succeeds until system removal; it supplies no state guard. A preceding completed terminal event can therefore be finalized again during logout. |
| `WorldSession::HandlePetBattleInput` (upstream of forfeit) | `CMSG_PET_BATTLE_INPUT` with `PET_BATTLE_MOVE_TYPE_REQUEST_LEAVE`; handler first requires `InProgress`, validates the team and readiness, then calls pointer overload. | A second packet processed after the first sees `Finished` and is guarded by the handler. This packet path alone is guarded, but the central API and logout path are not. |

There are no other PetBattle `EndBattle` calls in map cleanup, destructors,
GM/admin commands, explicit timeout code, capture code, or the unhandled
`CMSG_PET_BATTLE_QUIT_NOTIFY`. Capture kills the target and is completed by
the ordinary `HandleRound` terminal scan. `PetBattle::~PetBattle` only deletes
teams. The similarly named Battlefield calls are unrelated.

## `PetBattleSystem` lifetime timeline

1. `GetPlayerPetBattle` first resolves player GUID to battle ID in
   `m_playerPetBattles`, then ID to pointer in `m_petBattles`.
2. A terminal event calls `EndBattle`. A world-thread call performs results;
   a non-world-thread call only changes state to `Interrupted`.
3. Neither route erases either registry entry or queues removal directly.
4. At each 300 ms PetBattleSystem update, all registered battles are updated.
   `Interrupted` is finalized in that update; `Finished` does nothing.
5. After each battle's update returns, `Finished` battles are inserted into a
   pointer `std::set`, so repeated queue insertion is set-idempotent.
6. After iteration, `Remove` erases both player mappings and the battle ID,
   then deletes the battle. Only then does lookup stop finding it.

Thus the pointer remains alive and externally discoverable after an ordinary
world-thread `EndBattle` until the next system pass. Handlers can retrieve it;
most input is rejected by their `InProgress` check, but system/logout/combat
terminal APIs can still reach it. `Update` can still see it but its `Finished`
case does no work. `Remove` is not itself safe for two calls with the same stale
pointer, although the internal set prevents duplicate queue entries.

## Forfeit reentry

The client forfeit route is `CMSG_PET_BATTLE_INPUT` with move type
`PET_BATTLE_MOVE_TYPE_REQUEST_LEAVE`. It finds the battle through the player
registry and requires `InProgress`; therefore a second such packet processed
after the first returns is rejected because the first call eventually sets
`Finished`. This specific duplicate-packet sequence is `GUARDED` by the
handler.

The current, supported logout route calls the GUID overload without a state
check. A completed wild battle remains discoverable until the next 300 ms
system removal, so logout in that interval resolves the same object and calls
`EndBattle` again. This is `PROVEN_REENTRY` from current control flow. It can
repeat XP, capture creation/criteria, win/loss criteria, packets and wild-spawn
cleanup; if the first ending was also a forfeit, it repeats the health penalty.
The public pointer overload likewise explicitly permits a stale caller to
repeat finalization. This affects current wild PVE and is independent of
trainer support.

## Other reentry and overlap findings

| Combination | Classification | Reason |
| --- | --- | --- |
| normal terminal round, then logout before system removal | `PROVEN_REENTRY` | Finished object remains in both registries and logout forfeit has no state check. |
| first forfeit packet, then duplicate packet | `GUARDED` | Input handler rejects the second battle state as non-`InProgress`. |
| first forfeit, then logout before removal | `PROVEN_REENTRY` | Logout bypasses the input handler state check and repeats the forfeit call. |
| terminal round, then stale direct terminal request | `POSSIBLE_REENTRY` | Public system pointer overload and `EndBattle` accept `Finished`; actual external retained-pointer timing is not enumerated beyond current callers. |
| map-thread combat interrupt plus normal terminal work | `POSSIBLE_REENTRY` | Cross-thread code writes `Interrupted` without synchronization; ordering with world-thread terminal work is not expressed as a contract. This guard is not a broader thread-safety redesign. |
| logout/disconnect plus terminal round | `PROVEN_REENTRY` sequentially in the discoverability window; concurrent ordering `UNKNOWN` | Both supported routes exist, but thread scheduling beyond the sequential case is not proven. |
| capture plus terminal death | `GUARDED` within one round | `Catch` sets the caged pet and death state; the one terminal team scan calls once and breaks. Later logout remains a separate reentry. |
| two terminal teams/simultaneous death | `GUARDED` against two calls in one scan | `HandleRound` breaks after the first dead team. Winner correctness is separate. |
| subsequent `Update` after `Finished` | `GUARDED` | `Finished` switch case does nothing, then system removal is queued. |
| recursively triggered terminal work during finalization | `POSSIBLE_REENTRY` | No current callback was proven to recurse, but state remains nonterminal until all callbacks and sends complete. |

## Simultaneous final-pet death

Both teams can have zero living pets after casts, round-end procs, and aura
processing in the same round. After sending the round result and clearing
round state, `HandleRound` iterates `m_teams` from challenger index 0 to
opponent index 1. It calls `EndBattle` for the first dead team and breaks, so
there is one call. If both are dead, challenger is always selected as loser
and opponent as winner because of array order. The result therefore depends on
iteration order rather than an explicit draw/tie rule. Retail correctness is
`UNKNOWN`; this is documented for a later task and is not changed here.

## Existing state mechanisms and guard design

`PetBattleState` already has `Finished`, and the system already treats it as
the terminal/removable state. It is initialized to `Created`, handlers inspect
`InProgress`/`WaitingForFrontPets`, `Update` no-ops for `Finished`, and packet
writers do not serialize this internal enum. There is no separate finalized or
removal flag. `m_petBattlesToRemove` is system-owned, populated only after
`Update`, and cannot guard callbacks within `EndBattle`.

The chosen design is a central guard at the top of `PetBattle::EndBattle`:

1. return immediately if state is already `Finished`;
2. retain the existing non-world-thread transition to `Interrupted` and return;
3. on the world thread, assign `Finished` before any result mutation/callback;
4. execute the existing first-call body unchanged; and
5. let the existing PetBattleSystem scan remove the `Finished` object.

`Interrupted` cannot be treated as finalized because it intentionally defers
the real work to the world thread. Setting `Finished` only at the end would
not prevent recursive callback entry. Setting it early does not change winner
calculation, XP/capture eligibility, packets, actor cleanup, or removal:
none of those paths consumes the internal state, while the system continues
to observe the same terminal value after the function returns. No signature or
new member is needed, and ignored repeat calls need no return value because
all callers already use `void` and require no alternate action.

## Decision and applicability

The finding is `CURRENT_PRESERVATION_BUG` (and also a future trainer/A5
requirement), not a trainer-only concern. Current normal-round-to-logout and
forfeit-to-logout sequences can call non-idempotent finalization twice while a
wild battle is registered. A central early `Finished` transition establishes
the invariant without changing the first terminal result or packet layout and
without trainer code.

**Decision-gate classification: `STATICALLY_JUSTIFIED_CORE_FIX`.** All seven
gate conditions are met. The planned source scope is only
`src/server/game/BattlePet/PetBattle.cpp`.

## Planned validation

After this documentation checkpoint, add the minimal central guard, review the
source diff, configure the prescribed persistent development tree once, build
`game` and then `worldserver` with four jobs, and statically walk wild win,
wild loss, first/duplicate forfeit, stale terminal request, capture, and
simultaneous death. No broad unit framework will be introduced if the tree has
no existing focused PetBattle harness. Runtime build-18414 client validation
will remain outstanding, and compilation will not be represented as trainer
behavior proof.

## Implementation and validation result

The source changed only in `PetBattle::EndBattle` in
`src/server/game/BattlePet/PetBattle.cpp`. It now returns when the existing
state is `Finished`, preserves the off-world-thread `Interrupted` deferral, and
sets `Finished` immediately after reaching the world thread and before winner
or result side effects. The old assignment at the bottom was removed. No new
member, function signature, handler, packet, trainer behavior, or system
removal behavior was added.

The initially missing `/tmp/mop-preservation-dev` tree was configured once
with `PLAYERBOTS=0`, `USE_MODULES=0`, `TOOLS=0`, `ELUNA=0`, both ccache
launchers, and the requested temporary install prefix. The following checks
completed successfully:

* `git diff --check` before configuration;
* `cmake --build /tmp/mop-preservation-dev --target game --parallel 4`;
* `cmake --build /tmp/mop-preservation-dev --target worldserver --parallel 4`.

There is no existing focused PetBattle unit harness suitable for direct
`EndBattle` construction/reentry, so no new test framework was introduced.

### Static reentry matrix after the repair

| Scenario | First call | Later attempted call | Removal and packet/effect result |
| --- | --- | --- | --- |
| normal wild win | Sets `Finished` early, awards winner XP/criteria/capture if applicable, cleans up and sends the original final packet. | Any call before removal returns at the guard. | The next system update sees `Finished` and removes it; exactly one first-call result sequence. |
| normal wild loss | Sets `Finished`, resets loser criteria, cleans up and sends the original final packet. | Returns without mutation or packets. | Existing deferred removal remains intact. |
| first forfeit | Sets `Finished`, applies one ten-percent losing-team penalty, processes the normal outcome, and sends one final packet. | Duplicate direct/logout forfeit returns; a duplicate client input is also rejected by its existing state check. | One penalty and one result sequence; normal system removal. |
| terminal round then stale terminal request | Normal terminal processing is unchanged apart from the earlier internal state assignment. | Stale logout/combat/system terminal request returns. | No overwritten winner, repeated XP/criteria/capture or duplicate packet; removal proceeds. |
| capture terminal path | `Catch` still marks the caged/dead pet; `HandleRound` invokes finalization once and the captured pet is created once. | A stale request returns. | One capture and criteria sequence; removal proceeds. |
| simultaneous final-pet death | The first dead team in challenger-first order still causes one first call. | The loop still breaks; any later request returns. | Exactly-once is enforced, but the pre-existing iteration-order winner uncertainty is deliberately unchanged. |
| off-thread interruption | The first off-thread request still sets `Interrupted` and performs no results. The world update then passes the guard because state is not `Finished`, sets it early, and finalizes once. | Any call after that early transition returns. | The same update queues and removes the battle after finalization. |

First-call packet construction still sees the same winner, pet values, effects,
and result arguments. `SendFinalRound` and `SendFinished` do not inspect
`m_state`; XP, capture, achievements, player cleanup and wild-spawn cleanup do
not condition their work on it. Therefore moving the internal transition
earlier changes only reentry visibility, not the first call's serialized
contents or effects.

## Remaining evidence and readiness implications

The final classification remains **`STATICALLY_JUSTIFIED_CORE_FIX`** and source
did change. Compile validation establishes that the central state reuse is
well-formed and links into `worldserver`; it does not establish retail gameplay
or trainer correctness. A build-18414 client/server runtime should still verify
wild win/loss, capture, forfeit followed immediately by logout, terminal round
followed immediately by logout/combat, disconnect/interruption ordering, and
one final packet/result sequence. Existing unsynchronized cross-thread state
access is outside this narrowly scoped invariant and deserves separate runtime
or concurrency review.

For A4, the generic core now prevents supported terminal paths from applying
results more than once while a battle awaits removal. This removes the specific
finalization blocker identified by Task 024, but does not resolve trainer swap,
simultaneous-death winner policy, packet-trace, data, position, concurrency, or
lifetime blockers. For A5, future quest/achievement hooks would be protected by
the same central exactly-once boundary, but no A5 logic was imported and A5 is
not ready until the remaining A2--A4 requirements are satisfied.
