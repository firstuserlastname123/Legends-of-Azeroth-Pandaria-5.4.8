# Task 024: Trainer Battle Core Readiness

## Continuation State

- **FACT:** Review started clean on the existing `work` branch at
  `10cab0ef6681cfe74ac2fb10d1dfbfac4583f6e3`, after Tasks 021, 022, and 023.
  Task 021's raw slot-GUID load remains present and both later reports exist.
- **FACT:** `/tmp/mop-preservation-dev/CMakeCache.txt` was missing at both
  checks, so the warm development tree did not survive the environment. ccache
  reported an empty cache. Neither was configured, cleaned, or populated.
- **FACT:** This task changes documentation only. No CMake command, compiler,
  server, database, or runtime client was invoked.

## Andrew Provenance

The final trainer chain was reconstructed directly from the fetched Andrew
branch rather than treating any one mixed commit as a finished patch:

1. `39205279cebebd1d2c83000a0e9043e68047e659` (Candidate 10, 2026-05-19)
   introduces the manager/data, initiation paths, trainer team, and initial
   trainer round/result behavior. Its own subject admits the battle sticks
   after the trainer's first pet dies.
2. `c6400a299c1fbf2df0b8654cbff001ad801e7f82` (2026-05-22) moves trainer
   replacement before the round packet, removes a duplicate active-pet event,
   moves cooldown decrement, and restricts `CATCH_OR_KILL` to ordinary wild
   PVE.
3. `100303ca22b08f04ee5620dc1622617d5a680cdd` (Candidate 11, 2026-05-22)
   retains those state changes and adds the later-reviewed A5 helper among
   unrelated changes.
4. `1e338126a8f3e5381218dc90a50185119beeb592` (2026-05-24) revises swap-input
   locking and multi-turn behavior while adding trainer content/spawns.
5. `867b69b0d142efaaea10b83970342900d3405f2e` (2026-06-11) adds explicit
   skip-turn signaling for voluntary swaps and restructures final packet/reward
   order among broader effects and wild-spawn work.
6. `bd8ad5515418d94abd35ba7bf71430979243a09c` (2026-06-21) adds final passive,
   weather cleanup, and tamer-achievement behavior. It does not repair
   EndBattle idempotence.
7. `155f6444fe435529d38e46ee21ccc05d605e6031` only supplies include/forward
   declaration hygiene for PCH-independent compilation; it does not change
   trainer semantics.

- **FACT:** Final behavior below means the fetched
  `origin/feature/pandaria-fixes` tip, not Candidate 10 in isolation.

## A2 Final Scope

Final A2 consists of:

- new `src/server/game/BattlePet/BattlePetTrainerMgr.h/.cpp`;
- the BattlePet source glob already used by `src/server/game/CMakeLists.txt`, so
  no explicit source-list entry is required;
- `World.cpp` include and startup call to
  `sBattlePetTrainerMgr->LoadBattleTrainerPets()` after breed, quality, item,
  and spawn-pool battle-pet data load;
- `PetBattleTeam::AddTrainerBattlePets`, `m_ownsPets`, saved trainer origin,
  and the team destructor in `PetBattle.h/.cpp`; and
- the trainer branch in the PetBattle constructor that populates the opponent
  team from the request's Creature.

- **FACT:** Current preservation has none of the manager, startup load,
  owned-team mode, or trainer constructor branch (`ABSENT`). Existing generic
  BattlePet, team, DBC stores, WorldDatabase, and PetBattle allocation APIs are
  prerequisites (`PRESENT_EXACTLY` or `PARTIALLY_PRESENT`).
- **FACT:** A2 does not require an A3 handler merely to compile or load an empty
  definition store, but no player can use it without an initiation path.

## Trainer Data Model

- **FACT:** Manager key `entry` is the trainer NPC's creature-template entry.
  Its value is `std::vector<TrainerBattlePet>`; each record contains species
  (`uint32`), level, quality, and breed (`uint8`).
- **FACT:** There are no ability, nickname, display/model, GUID, or ordering
  fields. Ability selection is derived from species/level by
  `BattlePet::InitialiseAbilities(false)`. Display ID is derived indirectly
  from the species NPC's first CreatureTemplate model.
- **FACT:** No maximum team size is enforced. The schema's `(entry, species)`
  primary key forbids duplicate species for one trainer, but a trainer may have
  arbitrarily many distinct rows. The query has no `ORDER BY`; active-pet/team
  order therefore depends on database row order and is not contractual.
- **FACT:** Manager records are value objects owned by the singleton map.
  `CreateTrainerPets` allocates a new `BattlePet` for every row, with raw ID 0
  and owner `nullptr`, assigns team/global indices, derives abilities, and
  pushes the pointer into the trainer PetBattleTeam.
- **FACT:** Trainer pets are neither persistent account pets nor manager-owned
  BattlePets. They are transient copies owned by the team. `m_ownsPets=true`
  makes the team destructor delete every trainer BattlePet; player and wild
  teams remain non-owning.
- **FACT:** Missing trainer lookup returns `nullptr`; `AddTrainerBattlePets`
  logs and returns. PetBattle construction nevertheless continues with an
  empty opponent team, so callers must prove the team exists before Create.

## Minimum Trainer SQL Contract

### `A2_SCHEMA_MINIMUM`

Andrew queries the world database synchronously with:

```sql
SELECT entry, species, level, quality, breed FROM battle_pet_trainer
```

The minimum migration creates:

```sql
CREATE TABLE battle_pet_trainer (
  entry   MEDIUMINT(8) UNSIGNED NOT NULL,
  species INT(10) UNSIGNED NOT NULL,
  level   TINYINT(3) UNSIGNED NOT NULL DEFAULT 1,
  quality TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
  breed   TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (entry, species)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
```

- **FACT:** All five selected columns are required/non-null. The manager uses
  a raw query; there is no prepared statement.
- **FACT:** An existing empty table yields a null result, logs zero trainers,
  and returns. Startup continues. Thus A2 core can technically coexist safely
  with this empty schema and exposes no trainer through manager lookups.
- **FACT:** A missing table also returns no QueryResult and the loader returns,
  but the database layer emits a query error. Treating that as supported is not
  an acceptable migration contract; the schema is the minimum dependency.
- **FACT:** Creature spawn data is unnecessary for manager startup. Definitions
  do not dereference CreatureTemplates while loading.

### Excluded A11 data

- **FACT:** `2026_05_17_01_battle_pet_trainer_data.sql` is team content (102
  trainers/214 rows according to its header), not minimum schema.
- **FACT:** `2026_05_22_00_battle_pet_trainer_spawns.sql` and quest-chain/world
  edits are world content, not minimum schema. Neither belongs in an A2-only
  staging patch.
- **UNKNOWN:** The addon-derived rows, ordering, species, and trainer entries
  have not been verified against build 18414 or the deployed world database.

## Species / Breed / Quality Validation

| Field | Classification | Final behavior / failure |
| --- | --- | --- |
| trainer `entry` | `UNVALIDATED` | Loader accepts any integer and does not check CreatureTemplate; harmless until a matching creature initiates. |
| species | `DBC_DEPENDENT`, not safely validated | Loader accepts it; creation calls `sBattlePetSpeciesStore.LookupEntry` then `ASSERT`, so an invalid row can terminate the server when used. |
| level | `RANGE_ONLY` in SQL width, otherwise `UNVALIDATED` | 0–255 accepted; no 1–25 check. It changes stats and ability-tier selection and can create malformed pets. |
| quality | `DBC_DEPENDENT`, not safely validated | Constructor indexes quality data at `7 + quality` and asserts; invalid rows can terminate on use. |
| breed | `DBC_DEPENDENT`, effectively `UNVALIDATED` | No membership check; unmatched breed rows may leave base stats absent/malformed rather than being skipped. |
| ability IDs | `DBC_DEPENDENT` and derived | Not stored. Species/level select DB2 ability tiers; missing ability map returns no abilities rather than validating the trainer row. |
| team size/order | `UNVALIDATED` | More than three can assert in global-index conversion; row order is unspecified. |

- **FACT:** Malformed rows are never skipped by the loader. No diagnostics
  identify a bad field before battle creation.
- **INFERENCE:** A safe A2 implementation needs validation and deterministic
  ordering/team-size policy before real team content, rather than importing
  the exact assert-on-use behavior.

## Trainer Object Lifetime

- **FACT:** Manager definitions live for the process and are replaced by value
  on reload. Already-created trainer pets are copies and do not refer back to
  manager vectors, so manager reload does not invalidate them.
- **FACT:** PetBattle owns two teams; trainer team owns its allocated pets;
  PetBattle destruction deletes teams, then the trainer team deletes pets.
  This normal ownership path has no duplicate delete and no persistent save.
- **FACT:** PetBattleTeam stores a raw `Creature*` as `m_wildBattlePet`, plus a
  saved origin. EndBattle dereferences the raw pointer to restore flags,
  control, and position. The PetBattle request and teams also store raw
  Player/Unit pointers during the lifecycle.
- **UNKNOWN:** No guard reacquires the trainer by GUID after map unload,
  despawn, phase removal, scripted deletion, or disconnect. A trainer despawn
  can therefore leave a dangling pointer; runtime/map-lifecycle guarantees were
  not established.
- **FACT:** The trainer is not registered in `m_playerPetBattles`; only the
  player challenger is tracked for trainer PVE. Two players can target the same
  trainer concurrently, overwrite its target/control flags, and later restore
  it independently.
- **Classification:** `NEEDS_RUNTIME`, with a source-level concurrency and raw
  pointer concern. Normal allocation ownership is statically balanced, but the
  world-object lifetime contract is not proven.

## A3 Initiation Paths

Final Andrew exposes two independent ways to reach `PetBattleSystem::Create`:

1. **Client request path:** registered
   `CMSG_PET_BATTLE_REQUEST_UPDATE` ->
   `WorldSession::HandlePetBattleRequestUpdate` -> validation/position checks
   -> player/trainer mutation -> Create.
2. **Server gossip path:** NPC/quest hello adds
   `GOSSIP_OPTION_BATTLEPETF_TRAINER`; `Player::OnGossipSelect` independently
   repeats a subset of validation, fabricates positions from current player/
   trainer coordinates, mutates both actors, and calls Create directly.

- **FACT:** NPC gossip and questgiver hello only advertise the same gossip
  selection. They converge on `Player::OnGossipSelect`, not on the request
  packet handler.
- **FACT:** The two Create paths do not converge on a shared validator or
  position builder. Their checks and coordinate behavior differ.

## Request Packet Contract

- **FACT:** Opcode `CMSG_PET_BATTLE_REQUEST_UPDATE` is `0x0377` in both trees.
  Current preservation registers it `STATUS_UNHANDLED`, `PROCESS_INPLACE`,
  `Handle_NULL`; Andrew registers `STATUS_LOGGEDIN`, `PROCESS_THREADUNSAFE`,
  `HandlePetBattleRequestUpdate` and declares that handler in `WorldSession.h`.
- **FACT (Andrew-source behavior):** It first reads two team positions as
  floats in X, Z, Y order, then battle origin in Z, Y, X order. It reads GUID
  presence bits `0,6,3,5,2,7,1,4`, with inverted optional bits for facing and
  location-result, then GUID bytes `3,6,5,2,7,1,0,4`. Optional facing is a
  float; optional location result is a `uint32`.
- **FACT:** The parsed GUID selects the trainer. Failures use the existing
  `SMSG_PET_BATTLE_REQUEST_FAILED` writer and reasons for disabled, dead,
  combat, already battling, invalid target, too far, not trainer, empty slot,
  all pets dead, and terrain obstruction. Success mutates actors and creates a
  trainer PVE request.

Field evidence classification:

| Element | Classification |
| --- | --- |
| Numeric opcode in checked-in enum | `SOURCE_PROVEN`; wire correctness still lacks an independent trace |
| Handler status/thread mode | `SOURCE_PROVEN` as Andrew intent, not client evidence |
| XYZ permutations and float widths | `REQUIRES_BUILD18414_TRACE` |
| GUID bit/byte order | `REQUIRES_BUILD18414_TRACE` |
| inverted optional-bit meaning | `REQUIRES_BUILD18414_TRACE` and `SUSPICIOUS` |
| facing/location-result presence and meaning | `UNKNOWN` / `REQUIRES_BUILD18414_TRACE` |
| response reason values/wire timing | existing enums/writer are `LINEAGE_CORROBORATED`; trainer use requires trace |
| success transition and expected client follow-up | `REQUIRES_BUILD18414_TRACE` |

- **FACT:** No independent public lineage or capture was found in the local
  evidence. Andrew comments and copied forks would not prove the layout.

## Build-18414 Packet Evidence

- **UNKNOWN:** Required evidence is a build-18414 capture containing request
  and failure/success responses, exact bit/byte order, all coordinates, optional
  field cases, facing, location result, finalize location, initial update, first
  round, replacement round, final round, and finished packet.
- **INFERENCE:** Implementing the request decoder before that evidence risks
  consuming a valid client packet incorrectly even if the code compiles.

## Initiation Validation

| Check | packet path | gossip path |
| --- | --- | --- |
| pet battles enabled | yes | no explicit check |
| target exists / configured team | yes / yes | source Creature / yes |
| player alive / not in combat | yes / yes | yes / yes |
| player already in battle | yes | yes |
| trainer alive / in combat | no / no | no / no |
| trainer already used by another battle | no | no |
| distance | yes | yes |
| LOS | no | no |
| phase/map | accessor/map relation only | gossip source relation only |
| slot 0 populated / any living loadout pet | yes / yes | yes / yes |
| all configured pets valid and <= 3 | no | no |
| terrain/collision | ray test of client positions | none |
| finite/plausible coordinates | no | no |
| concurrency reservation | player only | player only |

- **FACT:** Both paths mutate player and trainer before `Create`; Create has no
  failure return or rollback if trainer population is empty/asserts.
- **Classification:** `NEEDS_VALIDATION_REWORK`. Packet evidence is an
  additional blocker, not a substitute for server-side checks.

## Position / Terrain Handling

- **FACT:** Packet initiation trusts client origin/team positions except for a
  line collision query from origin to each team position. It does not bound
  distance from player/trainer, reject non-finite values, check ground height,
  slope, LOS, phase, or navigation reachability.
- **FACT:** It faces the player toward the client opponent position and
  teleports the trainer two yards backward using the trainer's prior
  orientation; it does not recompute Z. EndBattle teleports the trainer back
  to the origin saved by `AddTrainerBattlePets`.
- **FACT:** Gossip initiation sets origin and opponent position to the
  trainer's exact location and challenger position to the player's current
  location, performs no terrain check, and does not move/facing-adjust the
  trainer as the packet path does.
- **Classification:** `SECURITY_OR_VALIDATION_CONCERN`, plus
  `NEEDS_PACKET_TRACE` and `NEEDS_TERRAIN_RUNTIME`.

## A4 Final-State History

Final A4 touches these existing/new behaviors:

- `PetBattleTeam::{CanSwap, GetAvaliablePets, SetActivePet,
  GetInputStatusFlags, ResetActiveAbility, TurnFinished, SetPendingMove}`;
- `PetBattle::{StartBattle, HandleRound, SwapActivePet, Kill, Catch,
  EndBattle, Update}`;
- `PetBattleSystem::{Update, Create, Remove, ForfietBattle}`; and
- existing packet writers `SendInitialUpdate`, `SendFirstRound`,
  `SendRoundResult`, `SendFinalRound`, `SendFinished`, and
  `SendFinalizeLocation`.

- **FACT:** Candidate 10's first-pet-death hang is not the final state;
  `c6400a29` is required to understand replacement timing, and `867b69b0` is
  required for final voluntary-swap skip-turn behavior.

## Trainer State Machine

The final source implies this state model:

1. **Create:** player loadout adds living pets; trainer rows allocate transient
   pets and select local pet 0. Both teams are ready because `SetActivePet`
   sets readiness. `SendFinalizeLocation` is sent.
2. **Created update:** sends initial update and enters
   `WaitingForFrontPets`. Trainer is already ready; player front-pet response
   confirms/selects the front pet.
3. **Start:** emits active-pet effects, sends first round, enters `InProgress`,
   increments round, and calls `TurnFinished`; trainer AI selects an ability.
4. **Player action:** packet handlers set a pending cast, swap/pass, catch, or
   dead-pet replacement and mark the player ready. When both teams are ready,
   `HandleRound` executes.
5. **Ordinary round:** pending moves prepare casts/swaps; voluntary swaps add a
   skip-turn active-pet effect and suppress all casts that round. Casts, procs,
   auras, passives, trainer replacement, stat/cooldown effects, and round packet
   follow; `TurnFinished` clears readiness and trainer AI chooses its next move.
6. **Player active death:** no automatic player replacement. The round packet's
   input flags request a new pet; a later `SWAP_DEAD_PET` uses `ignoreAlive`.
7. **Trainer active death:** final code finds an available trainer pet and calls
   `SwapActivePet(..., true)` before the same round result is serialized. The
   new active pet is ready; after TurnFinished trainer AI selects a move.
8. **Inactive/background death:** no active swap is required; death/aura
   semantics remain effect-dependent and need runtime coverage.
9. **No living pets:** after the round packet and TurnFinished, the first team
   found with zero living pets is passed to EndBattle as loser.
10. **Forfeit:** `PetBattleSystem::ForfietBattle` passes the forfeiting team to
    EndBattle with `forfeit=true`. Cross-thread EndBattle instead marks
    `Interrupted`; the world-thread update calls `EndBattle(nullptr)`.
11. **Finish:** EndBattle sends final/finished packets, restores actors, marks
    `Finished`; PetBattleSystem removes/deletes it on a subsequent update.

- **UNKNOWN:** Simultaneous death chooses the first zero-pet team encountered
  and breaks; it does not represent a draw or verify the nominal winner has a
  survivor. Retail outcome and passive ordering require evidence.
- **UNKNOWN:** Inactive death, multi-turn ability plus forced swap, disconnect,
  trainer despawn, and concurrent trainer challenges are not proven by static
  flow.

## Death Result Semantics

- **FACT:** Final `Kill` resets an active victim's ability, expires its auras,
  sets dead state, and sets round result `CATCH_OR_KILL` only when the victim
  team has a world creature **and** battle type is ordinary
  `PET_BATTLE_TYPE_PVE`. `Catch` explicitly sets `CATCH_OR_KILL`.
- **FACT:** Trainer deaths therefore retain `NORMAL`; this allows the active-pet
  replacement effect to travel in an ordinary round result rather than ending
  a wild catch/kill sequence.
- **INFERENCE:** The distinction is statically required by Andrew's internal
  multi-pet trainer continuation model and is likely client-required.
- **Classification:** `LIKELY_CLIENT_REQUIRED` and
  `REQUIRES_BUILD18414_TRACE`; it also `CONFLICTING_WITH_FLAMEHAWK`, whose
  active/background rule is different. No combined rule is justified.

## Swap / Replacement Semantics

- **FACT:** `c6400a29` fixes the admitted first-pet hang by moving trainer
  dead-pet replacement into HandleRound before `SendRoundResult`, using
  `ignoreAlive=true`, and avoiding `CATCH_OR_KILL` for trainer death.
- **FACT:** `SwapActivePet` resets multi-turn ability state, calls
  `SetActivePet` (ready and SeenAction), and emits one active-pet effect.
  c6400 removes the formerly duplicated effect from `SetActivePet`.
- **FACT:** `867b69b0` tracks `swappedThisRound`: voluntary swaps emit an
  additional active-pet effect with `SKIP_TURN` and skip round-start, ordinary,
  and round-end casts. Forced dead-pet swaps skip casts but do not add the
  voluntary skip-turn flag.
- **FACT:** Trainer auto-replacement happens after casts/auras/passives and
  before stat updates/cooldowns/round serialization. Trainer `TurnFinished`
  then selects a next action, preventing stale-move reuse.
- **UNKNOWN:** Whether the exact double active-pet event for voluntary swap
  (normal swap effect plus skip-turn effect), absence of that flag for forced
  swap, and same-round trainer replacement match build 18414 requires a trace.
- **INFERENCE:** The final flow removes the obvious Candidate-10 dead-trainer
  wait, but client non-stall and no-extra-turn claims remain runtime properties.

## Exactly-Once Finalization

All final calls are:

- HandleRound after it finds a zero-living-pet team;
- Update when state is `Interrupted`, using `EndBattle(nullptr)`; and
- `PetBattleSystem::ForfietBattle`, using the forfeiting team.

- **FACT:** PetBattle destructor does not call EndBattle or result hooks.
- **FACT:** HandleRound breaks after the first zero-pet team, so simultaneous
  death does not call EndBattle twice within that loop, although winner choice
  is unproven.
- **FACT:** EndBattle has no `Finished`/finalizing guard. The player remains in
  `m_playerPetBattles` until the next PetBattleSystem removal pass. A duplicate
  forfeit/result call in that window can invoke EndBattle and A5 result hooks
  again. A forfeit can also overlap other terminal work unless all calls are
  serialized by external behavior not expressed in this function.
- **FACT:** Cross-thread handling reduces one race by converting the call to
  `Interrupted`, but it is not an idempotence mechanism.
- **UNKNOWN:** Disconnect and trainer-despawn integrations provide no proven
  mutual exclusion with normal death/forfeit.
- **Classification:** `DEFECT_FOUND`. Exactly-once finalization is not merely
  untested; the final function lacks an idempotence gate while the battle
  remains externally discoverable.

## Packet / Client Dependencies

| Visible behavior | Classification |
| --- | --- |
| finalize location, initial update, first/normal/final round, finished writers | `EXISTING_SERIALIZER_REUSED` |
| trainer team with multiple pets in existing initial/round layouts | `LAYOUT_UNCHANGED_BUT_SEMANTICS_CHANGED` |
| death state/effects and active-pet swap effect | `EXISTING_SERIALIZER_REUSED` |
| trainer same-round replacement and NORMAL versus CATCH_OR_KILL | `LAYOUT_UNCHANGED_BUT_SEMANTICS_CHANGED`, `REQUIRES_BUILD18414_TRACE` |
| voluntary `SKIP_TURN` active-pet effect | `NEW_ANDREW_STRUCTURE` at event-sequence level, `REQUIRES_BUILD18414_TRACE` |
| final winner/final-round versus forced finished packet | existing writers, semantics `REQUIRES_BUILD18414_TRACE` |
| forfeit result ordering and HP reduction | `LAYOUT_UNCHANGED_BUT_SEMANTICS_CHANGED`, `UNKNOWN` |

- **FACT:** A4 can compile without changing opcode constants or packet writer
  layouts, but successful compilation cannot validate event order or client
  progression. It remains client-runtime blocked.

## Preservation Comparison

| Component | Current status |
| --- | --- |
| trainer manager/schema loader/startup | `ABSENT` |
| transient trainer pet/team ownership | `ABSENT` |
| generic BattlePet/team/PetBattle substrate | `PARTIALLY_PRESENT` |
| trainer gossip options/direct initiation | `ABSENT` |
| request opcode numeric constant | `PRESENT_EXACTLY` |
| request handler/registration | `ABSENT` / current explicitly unhandled |
| request-failure writer/reasons | `PARTIALLY_PRESENT` |
| server validation/terrain for wild battles | `PARTIALLY_PRESENT`, not equivalent trainer validation |
| trainer type and constructor branch | `ABSENT` |
| trainer AI move selection | baseline generic ownerless-PVE logic `PARTIALLY_PRESENT` |
| trainer death NORMAL/replacement behavior | `CONFLICTING_IMPLEMENTATION` / absent trainer distinction |
| voluntary/forced swap foundations | `PARTIALLY_PRESENT`; final Andrew event semantics absent |
| EndBattle exactly-once protection | `ABSENT` in both preservation and Andrew final |
| packet serializers | mostly `PRESENT_EXACTLY`; trainer event sequences absent |

## Staging Analysis

| Stage | Assessment | Rationale |
| --- | --- | --- |
| 1: A2 manager/lifetime + empty schema | `SAFE_STAGING_POINT` but operationally inert | World starts with zero rows; ordinary battles have no initiation path and no behavior change. Schema is required. It exposes internal APIs but no player path. Compile/startup test still required. |
| 2: A2 + validated team data, no initiation | `NOT_USEFUL` as a deployable feature | It can validate loader/data at startup, but no client can battle; rows must first gain safe validation/order/team-size rules. No packet trace required yet. |
| 3: A2 + A3 without final A4 | `UNSAFE_PARTIAL_STATE` | Players can reach Candidate-10-style trainer battles that hang or mis-finalize; packet, terrain, concurrency, and state blockers are live. |
| 4: A2 + A3 + A4 final | `UNKNOWN` / full experiment only | Technically coherent surface, but known exactly-once defect plus packet, data, terrain, simultaneous-death, and lifetime blockers prevent preservation deployment. |

- **INFERENCE:** A2 can be staged only as inaccessible infrastructure. Any
  accessible trainer feature needs A2/A3/A4 together after their blockers are
  repaired and traced.

## A2 Readiness

**Primary classification: `NEEDS_DATA_VALIDATION`. Secondary:
`NEEDS_RUNTIME`.** Empty-schema infrastructure is mechanically stageable, but
Andrew's loader must not ingest real content unchanged: invalid species/quality
assert, level/breed/team size/order are unchecked, and world Creature lifetime/
concurrency needs runtime work. It is not yet `READY_WITH_MINIMUM_SCHEMA` as a
useful feature.

## A3 Readiness

**Primary classification: `NEEDS_VALIDATION_REWORK`. Secondary:
`NEEDS_BUILD18414_PACKET_TRACE` and `NEEDS_POSITION_RUNTIME`.** Two divergent
creation paths, trusted coordinates, missing trainer reservation/alive/combat/
LOS checks, and absent rollback are source-level concerns independent of the
unknown wire layout.

## A4 Readiness

**Primary classification: `NEEDS_STATE_MACHINE_FIX`. Secondary:
`NEEDS_BUILD18414_CLIENT_TRACE` and `NEEDS_RUNTIME`.** The first-trainer-pet
hang has a coherent final repair, but exactly-once finalization has a concrete
gap and simultaneous death/outcome, passives, replacement, and event order are
not proven.

## Overall Trainer-Core Status

**Overall: `TRAINER_CORE_BLOCKED_ON_STATE_MACHINE`.** The primary blocker is
EndBattle idempotence/exactly-once result processing. Secondary blockers are
`TRAINER_CORE_BLOCKED_ON_PACKET_EVIDENCE` and
`TRAINER_CORE_BLOCKED_ON_DATA`, plus position, concurrency, Creature lifetime,
and client-runtime evidence. The final Andrew branch is suitable as comparative
implementation material, not a ready patch set.

## Recommended Task 025

**Choice D: review and conditionally repair one specific A4 defect before any
trainer implementation — exactly-once PetBattle finalization.**

- **Exact prospective source scope:**
  `src/server/game/BattlePet/PetBattle.cpp` (`EndBattle`, terminal callers, and
  PetBattleSystem removal/forfeit interaction) and, only if a minimal explicit
  finalizing state/API is necessary, `PetBattle.h`. No SQL, trainer manager,
  handlers, opcode, packets, effects, weather, A5, or Playerbots.
- **Source ancestry:** compare preservation baseline with Andrew final across
  `39205279 -> c6400a29 -> 100303ca -> 1e338126 -> 867b69b0 -> bd8ad551`; the
  defect remains at final tip, so Task 025 must be a reasoned repair rather than
  an Andrew cherry-pick.
- **SQL:** none.
- **Expected targets if a repair is justified:** `game`, then `worldserver`.
  `/tmp/mop-preservation-dev` may be reused if it exists in that future task;
  it is currently missing and must not be presumed warm.
- **Resulting test:** repeated forfeit/terminal calls, simultaneous terminal
  conditions, disconnect/interrupt, one result packet sequence, one cleanup,
  and one reward invocation in existing wild PVE. A trainer client test still
  cannot begin because A2/A3 are absent.
- **Why smallest:** idempotent finalization is required by current battles and
  by future A4/A5. Fixing or disproving it reduces a concrete core defect
  without exposing unverified trainer packets/data.

Task 025 must not implement A2, A3, or the rest of A4 as a side effect.

## Required Runtime / Client Evidence

Before accessible trainer support:

1. authoritative `battle_pet_trainer` rows: trainer entries, deterministic pet
   order, team size, species, breed, quality, level, and derived abilities;
2. loader rejection tests for every malformed field and empty-table startup;
3. trainer Creature despawn/map unload/phase change, manager reload, concurrent
   challengers, disconnect, and cleanup lifetime tests;
4. build-18414 request captures proving opcode, GUID mask/bytes, XYZ order,
   optional bits, facing, location result, failure reasons, and success flow;
5. server-owned position/height/LOS/collision design tested on flat, sloped,
   obstructed, indoor, transport, phased, and map-edge terrain;
6. one/two/three-pet trainers, first/middle/final death, voluntary and forced
   swaps, multi-turn abilities, auras/passives, inactive death, and trainer
   replacement event order;
7. simultaneous deaths, last-pet passive deaths, forfeit, duplicate forfeit,
   disconnect/abort, trainer despawn, and exactly-once finalization;
8. captured versus killed wild semantics and trainer NORMAL death compared with
   exact first/normal/final/finished packet traces; and
9. only after A2-A4 pass, the Task 023 A5 quest/achievement tests.

No source implementation, SQL, CMake configure, compilation, server start, or
runtime test was performed in Task 024.
