# Task 020: Andrew Battle-Pet Final-State Reconstruction

## Starting State

- **FACT:** The repository was clean. The task branch was `work` at
  `4751b870f59d3d8a4bd52a64cc23b6b3c571a83e`, the merge of Task 019
  (`c3c98a2`), and therefore contains the current `preservation/main` history
  after the Flamehawk reconstruction. No source, SQL, opcode, module, or build
  file was changed by this task.
- **FACT:** The prescribed workflow, build workflow, architecture, archaeology,
  candidate-review, and Task 019 documents were read before archaeology.
- **FACT:** This is historical/static analysis only. No commit was applied, no
  SQL was executed, and no compilation or server process was run.

## Repository and Candidate Provenance

- **FACT:** A temporary clone of
  `https://github.com/andrew-downey/Legends-of-Azeroth-Pandaria-5.4.8` was used;
  the preservation repository's remotes were not changed. The inspected Andrew
  tip resolves to `f1abdd1a2754f4a39f9e5d2cd5112e63899a86c4` (parent
  `155f6444fe435529d38e46ee21ccc05d605e6031`, 2026-07-01, `Refactor notable
  additions and clean up README`).
- **FACT:** Candidate 10 is
  `39205279cebebd1d2c83000a0e9043e68047e659`, parent
  `99801947ccebbe1653ba6d3aa444559bc3efddd2`. Candidate 11 is
  `100303ca22b08f04ee5620dc1622617d5a680cdd`, parent
  `ad52085706c01060667e2219eb994942ac85657b`. Both are on the inspected
  first-parent line.
- **FACT:** The weather prerequisite came through merge
  `99801947ccebbe1653ba6d3aa444559bc3efddd2`. Its second-parent chain is
  `c440e10f49cb6d67dea66c3916f1d594fcd7f03d` ->
  `2da8a377a4c2b15de30c2cad501914301e15c6af` ->
  `77711e7ae24fbeda408efd080052aebb211f81ac`.
- **FACT:** Fifteen battle-pet-relevant commits were reviewed through their
  metadata, statistics, complete patch, and resulting files: the 14 behavioral,
  data, merge, or documentation commits in the table below plus the final
  PCH-independent test/header repair `155f6444`. Tillers-only and Playerbots-only
  commits were inspected for boundary purposes but are not counted as battle-pet
  commits.

## Battle-Pet Commit Timeline

`SQL`, `wire`, `bots`, and `mixed` identify changed concerns, not endorsements.

| Commit (parent) | Author date; subject | Battle-pet files/subsystem | SQL / wire / bots / mixed | Dependency and final disposition |
| --- | --- | --- | --- | --- |
| `c440e10f49cb6d67dea66c3916f1d594fcd7f03d` (`556566a6`) | Andrew Downey, 2026-05-17; weather handlers | `BattlePetAbilityEffect.{h,cpp}`, `BattlePetAura.cpp` | unrelated auth deletions; no wire/bots | Introduces effect 80 weather aura and aura duration work; corrected repeatedly by `100303ca`, `1e338126`, and finally `bd8ad551`. |
| `2da8a377a4c2b15de30c2cad501914301e15c6af` (`c440e10f`) | 2026-05-17; login load fix | `BattlePetMgr.cpp` | no pet SQL/wire; incidental generated `ModulesLoader.cpp` | Reconstructs saved slot GUIDs directly from stored `uint64`; retained at tip. |
| `77711e7ae24fbeda408efd080052aebb211f81ac` (`2da8a377`) | 2026-05-17; restore migrations | no pet code | restores unrelated archived auth SQL | Cancels `c440e10f`'s accidental auth deletions only; Candidate 10 deletes them again. Not a pet schema prerequisite. |
| `99801947ccebbe1653ba6d3aa444559bc3efddd2` (`556566a6`, `77711e7a`) | 2026-05-17; merge weather work | merge result includes effect, aura, manager | merge; no opcode | Makes the prerequisite chain first-parent-visible; no independent semantic patch beyond its merged result. |
| `39205279cebebd1d2c83000a0e9043e68047e659` (`99801947`) | 2026-05-19; Candidate 10 | trainer manager, battle/team, gossip/quest/player request paths, session/opcode registration, world load | two trainer SQL files; opcode registration; unrelated auth deletion | First runnable trainer architecture, explicitly stuck after pet one. Superseded as a standalone unit by `c6400a29`, Candidate 11, `1e338126`, `6115dcae`, `867b69b0`, and `bd8ad551`. |
| `c6400a299c1fbf2df0b8654cbff001ad801e7f82` (`39205279`) | 2026-05-22; trainer death fix | `PetBattle.{h,cpp}` | no | Adds same-round forced trainer swap, PvE-only catch/kill result, removes duplicate active-pet event, and adjusts cooldown/ready flow. Retained and then extended. |
| `ad52085706c01060667e2219eb994942ac85657b` (`c6400a29`) | 2026-05-22; `PETBATTLES.md` | documentation | no | Records the intermediate implementation and known issues; later replaced by `BATTLEPETS.md` and then moved to `research/`. |
| `100303ca22b08f04ee5620dc1622617d5a680cdd` (`ad520857`) | 2026-05-22; Candidate 11 | effect 80, quest credit, spawn bookkeeping, duplicate object loading across Creature/GameObject/Map/Grid, Player | no pet SQL/opcode; substantial unrelated Playerbots commands | Changes weather target to all, adds objective-type quest credit, fixes spawn relation erase, and introduces broad grid duplication controls. It is an intermediate mixed patch, not final weather or battle behavior. |
| `1e338126a8f3e5381218dc90a50185119beeb592` (`2e4a6f7f`) | 2026-05-24; trainers/swaps/weather | effects, battle, battle handler, Hyjal script | trainer spawn SQL; unrelated binary/build artifacts | Adds trainer spawns/quest chains, voluntary/dead swap corrections and more aura changes. Later spawn position/data and battle logic modify it. |
| `987cdb972e1fc5ce3580fd8b6d8c545c4df8fd10` (`1e338126`) | 2026-05-24; harmful stacking guard | ability effects | no | Reuses an existing harmful aura instead of stacking blindly; retained, while delayed activation is refined in `867b69b0`. |
| `6115dcae543887b54636a4fc03f1bb3fd8de93e5` (`41e67ba1`) | 2026-06-01; Merda position/Tillers/bots | modifies trainer spawn SQL only for pet scope | SQL; bots and Tillers mixed | Corrects one trainer placement. Only that SQL hunk belongs to pet final state. |
| `867b69b0d142efaaea10b83970342900d3405f2e` (`1e138b1f`) | 2026-06-11; battle fixes/effects/wild spawns | effects/aura/battle/player and battle-pet docs | loot, trainer, Feralas/Kalimdor/EK SQL; Tillers and DB layer mixed | Adds delayed-aura handling, aura expiry, swap-round suppression, family/passive and battle-finish fixes, and first wild-data batches. Retained but effect/weather/result code is extended later. |
| `aeef0c9d8e6484278ea98d07dde4cc608b077ff4` (`867b69b0`) | 2026-06-13; effects/Cataclysm wild pets/rate | effects, battle, World config | four wild SQL files; no opcode/bots | Adds 20+ handlers, pet-XP rate, and four regions. This is the principal final effect dispatch ancestor, but `bd8ad551` still changes effects 61/75/77 and weather. |
| `bd8ad5515418d94abd35ba7bf71430979243a09c` (`aeef0c9d`) | 2026-06-21; mixed Tillers farm | `BattlePet.h`, effect files, `PetBattle.{h,cpp}`, achievement code | two Northrend pet SQL files amid extensive unrelated SQL/Tillers | Final semantic pet commit: weather becomes explicit battle state, adds three handlers, and adjusts passives/death/achievement behavior. Pet hunks are separable; Tillers is not a prerequisite. |
| `155f6444fe435529d38e46ee21ccc05d605e6031` (`7de670ec`) | 2026-06-22; test/PCH compilation | forward declarations/includes in `BattlePet.h` and `PetBattle.h`, pet test include | no SQL/wire/bots; build/tests mixed | Build-hygiene only; retained at tip, no gameplay semantics. |

**FACT:** Commits `897bbf8`, `a008e984`, `1bb75c28`, `dc22704b`, `b4a0c209`,
`41e67ba1`, and `1e138b1f` touched some broad candidate paths, but their inspected
hunks are Tillers, battlepay, database, or Playerbots work rather than a change
to the final battle-pet subsystem. Chronology alone does not make them pet work.

## Trainer-Battle Development

### Entry, data, and construction

- **FACT:** Candidate 10 adds `BattlePetTrainerMgr`, loaded during world startup.
  It queries world table `battle_pet_trainer` into an entry-keyed vector of
  `{species, level, quality, breed}`. `CreateTrainerPets` resolves the species
  DB2 row, allocates non-owned-guid `BattlePet` objects, assigns team/global
  indexes, initializes abilities, and selects local pet zero. The opponent team
  marks `m_ownsPets`; its destructor deletes these transient pets.
- **FACT:** Trainer initiation has two entry paths in the final tree. NPC
  gossip/quest interaction in `Player::OnGossipSelect` constructs a request
  directly. `CMSG_PET_BATTLE_REQUEST_UPDATE` parses client positions, origin,
  optional facing/location, and opponent GUID; validates enablement, life,
  combat, existing battle, range, trainer data, loadout, living pets, and
  terrain; roots/pacifies both actors; backs the NPC up two yards; and creates
  `PET_BATTLE_TYPE_PVE_TRAINER`.
- **FACT:** Candidate 10 registers the already-declared build-18414 value
  `CMSG_PET_BATTLE_REQUEST_UPDATE = 0x0377` to a newly declared session handler.
  It does not add a new serializer class. Gossip bypasses that incoming packet
  and fills location fields server-side.
- **UNKNOWN:** Neither path proves its client-provided coordinate order,
  opcode value, facing semantics, or gossip-generated positions against a
  captured 18414 session. The gossip path initially places the trainer at the
  battle origin/team position; the packet path's later two-yard teleport is a
  fork workaround, not retail evidence.

### State machine and Candidate 10 correction

- **FACT:** Candidate 10 initially allowed trainer AI readiness/auto-pass and
  used the ordinary death result in ways its own subject identified as stuck.
  `c6400a29` is the direct correction: after aura processing and before the
  round packet, it finds an alive trainer reserve and calls
  `SwapActivePet(..., true)` so a dead outgoing pet does not fail `CanSwap`.
  It also restricts `CATCH_OR_KILL` to wild `PET_BATTLE_TYPE_PVE`, removes a
  duplicate active-pet event, moves cooldown decrement before serialization,
  and removes asymmetric ready/auto-swap logic.
- **FACT:** Final `HandleRound` tracks whether either side swapped, emits a
  `SKIP_TURN` active-pet effect for voluntary swaps, accepts forced dead swaps,
  suppresses that side's round-start/ordinary/round-end casts, processes auras
  and passives, then performs the trainer same-round forced swap before stat,
  cooldown, and round-result serialization. `867b69b0` is therefore part of
  the final swap behavior, not merely Candidate 10 plus `c6400a29`.
- **FACT:** Win/loss is still driven by all-dead detection and common
  `EndBattle`. A winning player receives normal pet-battle achievement credit,
  trainer kill credit, and explicit `QUEST_OBJECTIVE_WINPETBATTLEAGAINSTNPC`
  credit. The losing side resets loss criteria. Trainer battles award
  participation XP but never enter wild capture storage.
- **FACT:** Forfeit follows common request-leave handling and the common 10%
  current-health reduction for the losing player team. A forced end has no
  winner and sends `SMSG_PET_BATTLE_FINISHED`; a completed battle sends the
  final-round packet. On cleanup, player root/pacify flags are removed; a wild
  NPC is returned to the spawn manager, while a trainer is unrooted/unpacified
  and teleported to its saved original transform.
- **INFERENCE:** Logout/disconnect relies on the existing PetBattleSystem/session
  cleanup rather than a trainer-specific persistence record. No trainer battle,
  transient trainer pet, current round, or weather is persisted for relog.
- **UNKNOWN:** Client correctness for simultaneous deaths, a player pet dying
  while the trainer swaps, disconnect during finalization, forfeit timing, and
  trainer NPC removal/map unload was not demonstrated. Candidate 10 is
  `SUPERSEDED_INTERMEDIATE`, not an independently backportable patch.

## Weather Development

- **FACT:** `c440e10f` maps effect 80 to `HandleWeatherAura`, initially targeted
  at the head pet. Candidate 11 changes effect 80 to `TARGET_ALL` so all six
  pets receive the aura. `1e338126`/`987cdb97` adjust triggered aura and stacking
  behavior. `aeef0c9d` also maps effect 169 to weather, but with caster targeting.
- **FACT:** `bd8ad551` is the final weather implementation. A battle owns
  `m_weatherAbility`, duration, originating effect ID, and a map of state
  deltas. On a different weather, it emits state clears and expires the old
  weather ability aura on every pet. On the first invocation of new weather,
  it applies the triggered aura to every non-elemental pet on both teams,
  emits no-target `SET_STATE` events for all matching
  `BattlePetAbilityState` rows, and records the new weather. Common battle end
  clears weather states before ordinary aura expiration and mechanic reset.
- **FACT:** Weather is battle-local and memory-only. It is not stored in the
  character database. Ordinary per-pet auras still carry duration and expiry;
  global weather state is separately represented by null-pet state events.
  Replacement explicitly expires old per-pet auras and clears the tracked
  state map. The handler only applies when the recorded weather ability changes,
  preventing its `TARGET_ALL` calls from multiplying the weather.
- **INFERENCE:** Because all roster pets receive the aura up front, incoming
  swapped pets inherit weather without a swap hook. Elemental immunity is
  implemented by omitting their per-pet aura, while the global state event is
  still emitted; whether that matches the client/state evaluator is uncertain.
- **FACT:** Against Flamehawk F3, this is `CONFLICTING`, not a safe superset.
  Both use triggered ability, properties `[2]` (duration) and `[3]`
  (max/stack argument), and aura/event APIs. Flamehawk's effect 169 is a
  damage-family handler and its weather/aura logic does not have Andrew's
  explicit single-weather battle state and DB2 state-delta rollback. Andrew is
  structurally more complete for replacement, but no 18414 data/trace proves
  `SUPERSEDES_FLAMEHAWK`.
- **UNKNOWN:** Round-boundary decrement, exact replacement event ordering,
  negative state-value encoding through `uint32`, elemental exemptions, and
  repeated casting of the same weather all require 18414 DBC and packet traces.

## Login / Persistence

- **FACT:** Before `2da8a377`, `LoadSlotsFromDb` read each saved slot as a
  32-bit counter and reconstructed `ObjectGuid(HighGuid::BattlePet, counter)`.
  The fix reads each database field as its stored `uint64` `ObjectGuid`. This
  repairs identity reconstruction; it does not change pet-row loading.
- **FACT:** The existing player login holder loads account-owned battle-pet
  rows and the three-slot record, then calls `BattlePetMgr::LoadSlotsFromDb`.
  The slot row contains three GUIDs plus loadout flags. Pet rows continue to
  own species, nickname, timestamp, level, XP, health, max health, power,
  speed, quality, breed, flags, and account ownership through the pre-existing
  manager/database path.
- **FACT:** No schema migration accompanies `2da8a377`; the issue is C++ GUID
  interpretation. The line remains unchanged at Andrew tip. No later commit
  supersedes it. Trainer pets are transient and do not enter this account
  collection or saved slots.
- **FACT:** Current preservation still constructs the three slot GUIDs from
  `HighGuid::BattlePet` plus `GetUInt32`; therefore Andrew's exact repair is
  `ABSENT`, subject to checking the deployed column representation and the
  repository's current `ObjectGuid` contract.
- **UNKNOWN:** No saved database sample establishes whether all deployed rows
  contain packed/full GUIDs rather than counters, and no login/relog test proves
  slot, health, XP, or level behavior. The fix has no direct health/XP formula
  effect.

## SQL / Migration Final State

The final pet-specific ordering is by filename within the **world** database:

| Order/file | Tables/actions | Family and assumptions |
| --- | --- | --- |
| `2026_05_17_00_battle_pet_trainer.sql` | creates `battle_pet_trainer(entry,species,level,quality,breed)`, PK `(entry,species)` | Trainer prerequisite; assumes species/breed/quality DB2 IDs. Must precede data. |
| `2026_05_17_01_battle_pet_trainer_data.sql` | `REPLACE` rows for 102 NPCs/214 pets | Trainer teams; generated from addon `RivalInfo`, not independently verified retail data. Depends on table. |
| `2026_05_22_00_battle_pet_trainer_spawns.sql` | deletes/reinserts 16 `creature` entries, deriving GUID base from `MAX(guid)+1`; later edits include Merda placement | Trainer world data; destructive/idempotent-by-entry assumption, coordinate/duplicate/concurrency risk. Depends on existing templates and quest/gossip data. |
| `2026_06_09_00_pet_supplies_loot.sql` | item flags and `item_loot_template` replacement/inserts | Reward content, not core trainer state. Sources are public sites; independent review needed. |
| `2026_06_09_01_feralas_wild_pets.sql` | `battle_pet_wild_pool`, direct `creature` inserts | Wild spawn only; assumes pre-existing pool schema/templates. |
| `2026_06_10_01_kalimdor_wild_pets.sql` | pool rows and direct spawns | Wild spawn only. |
| `2026_06_10_02_eastern_kingdoms_wild_pets.sql` | pool rows and direct spawns | Wild spawn only. |
| `2026_06_12_00_deepholm_wild_pets.sql` | pools/direct spawns | Wild spawn only. |
| `2026_06_12_01_twilight_highlands_wild_pets.sql` | pools/direct spawns | Wild spawn only. |
| `2026_06_12_02_mount_hyjal_wild_pets.sql` | `REPLACE` pools/direct spawns | Wild spawn only. |
| `2026_06_12_03_uldum_wild_pets.sql` | `REPLACE` pools/direct spawns | Wild spawn only. |
| `2026_06_12_04_northrend_wild_pets_1.sql` | pool repair plus direct spawns | Wild spawn only; mixed insert/replace assumptions. |
| `2026_06_12_05_northrend_wild_pets_2.sql` | direct spawns/pools | Wild spawn only. |

- **FACT:** `77711e7a` restores archived **auth** migrations accidentally deleted
  on the topic branch; Candidate 10 deletes them again. Those deletions persist
  at Andrew tip but are unrelated and must not be carried with pet work.
- **FACT:** Weather, login, effect handlers, battle results, and XP-rate code
  add no pet-specific migration. Wild scripts rely on the baseline
  `battle_pet_wild_pool`; trainer code uniquely requires the new trainer table.
- **FACT:** None of the pet migrations is deleted later. Trainer spawn SQL is
  amended, while later wild files append geographic batches. Final-state review
  must use the resulting files, not every historical version.
- **UNKNOWN:** Deployed world data, GUID allocation safety, duplicate keys,
  addon-derived teams, coordinates, templates, quest chains, and loot rates
  were not validated. Tillers/character migrations and unrelated June content
  in mixed commits are excluded.

## Packet / Opcode Dependencies

| Contract | Preservation comparison | Final finding |
| --- | --- | --- |
| `CMSG_PET_BATTLE_REQUEST_UPDATE` value `0x0377`, registration, session declaration/handler | `ABSENT` registration/handler; constant is present in current enum | Candidate 10 enables the receive path. Exact value/bit order/floats are `REQUIRES_BUILD18414_TRACE`. |
| request-failed send | `PRESENT_EQUIVALENTLY` baseline API, trainer uses it in new paths | Existing `SMSG_PET_BATTLE_REQUEST_FAILED` writer is reused; reason sequencing needs trace. |
| initial/finalize/first-round trainer creation | `PARTIALLY_PRESENT` for wild/PvP, trainer type absent | Reuses existing packets with a new battle type/team composition; no serializer layout edit. |
| round result: swap, skip-turn, weather state/aura, death | `CONFLICTING` values/events | Serializers remain structurally unchanged; event production/order changes. Trace is required. |
| final-round vs finished | `PRESENT_EQUIVALENTLY` serializer, changed state feeding it | Winner sends final round; interruption sends finished. Trainer result and release timing require trace. |
| set-front-pet receive path | `CONFLICTING` final battle logic | Existing opcode/handler; final Andrew changes dead/voluntary swap validation and emitted events, not its numeric opcode. |

**FACT:** No Andrew pet commit changes `Opcodes.h` numeric values or packet
classes. Candidate 10 changes `Opcodes.cpp` registration and `WorldSession.h`;
all later packet dependencies arise from battle state/event production. No
later commit removes the registration or reverses its direction.

## Quest Credit and Battle Results

- **FACT:** Candidate 10 awards `KilledMonsterCredit` on trainer victory.
  Candidate 11 adds `Player::PetBattleCompleteQuest`, which scans incomplete
  quest-log entries, finds objectives of
  `QUEST_OBJECTIVE_WINPETBATTLEAGAINSTNPC` whose `ObjectID` equals the trainer
  entry, increments by one up to the requested amount, marks the objective for
  saving, sends objective credit, and completes the quest when eligible.
- **FACT:** Common win achievement credit now includes trainer opponent family
  masks; loss criteria reset applies to the loser. Wild capture remains limited
  to type `PVE`, including level loss and capture achievements. Trainer type
  gets XP and win criteria but cannot cage its transient pets.
- **FACT:** `c6400a29` changes death semantics so only a killed wild pet selects
  `CATCH_OR_KILL`; trainer death remains `NORMAL`, permitting the same-round
  replacement pet. Catch explicitly selects `CATCH_OR_KILL`. `867b69b0` and
  `bd8ad551` add swap suppression and passive/death processing, so those later
  hunks are part of the final result.
- **FACT:** Compared with Flamehawk F5, both reject the baseline rule that every
  death is `CATCH_OR_KILL`, but on different axes: Flamehawk distinguishes
  active from background death; Andrew distinguishes wild battle type from
  trainer. Andrew supplies evidence that Candidate 10 was internally broken,
  not proof that Flamehawk is retail-wrong. A combined active/type rule would
  be a new invention and is prohibited.
- **FACT:** Compared with Flamehawk F6, Andrew's later swap state machine is
  broader (voluntary skip-turn plus explicit forced-dead path and trainer
  auto-swap). It does not independently validate Flamehawk's narrow living-pet
  anomaly guard.
- **UNKNOWN:** Simultaneous deaths, background periodic deaths, achievement
  family masks, exact quest-objective packets, forfeit outcomes, and final
  reward ordering lack a build-18414 trace.

## Grid / World / Spawn Integration

- **FACT:** Candidate 11's `BattlePetSpawnMgr` fix erases a relation by GUID
  key rather than indexing the map then erasing the mapped value. This is a
  `WILD_SPAWN_REQUIREMENT` and remains at tip.
- **FACT:** Candidate 11 adds `allowDuplicate` arguments to Creature and
  GameObject DB loaders, map-level spawn-ID lookup/bookkeeping, and grid-loader
  checks. These are `GRID_BOOKKEEPING` changes used to avoid duplicate DB
  objects while permitting deliberate replacement/loading flows. Player/map
  changes connect spawned wild pets and objects to that bookkeeping.
- **FACT:** Trainer manager startup and trainer request/gossip changes are
  `TRAINER_REQUIREMENT`; direct-spawn/pool SQL and spawn-manager relations are
  `WILD_SPAWN_REQUIREMENT`; Playerbots manager/command hunks are
  `PLAYERBOTS_REQUIREMENT`; Tillers phasing, farm managers, and unrelated
  creature/quest corrections are `UNRELATED`.
- **FACT:** The structural Creature/GameObject/Map/Grid signatures remain in
  Andrew final state. They are global core changes even when pet battles are
  disabled, so their preservation value and regression risk cannot be justified
  solely by enabling trainer battles.
- **INFERENCE:** The broad duplicate-load API can be reviewed independently of
  trainer state, but the wild replacement system may rely on it. Backporting
  direct wild SQL without spawn ownership/bookkeeping review risks duplicate or
  orphaned creatures.
- **UNKNOWN:** Grid unload/reload, map transfer, respawn timers, event spawns,
  replacement restoration, and direct type-14 spawn behavior were not runtime
  tested.

## Playerbots Boundary

- **FACT:** Candidate 11's Playerbots hunks add/restructure console commands
  (`bot`, `pmon`, `rndbot`) and random-bot management; they do not implement
  trainer/weather mechanics. Other later mixed commits change bot login/config
  behavior. No ordinary-player battle path calls those APIs.
- **FACT:** `2da8a377` also edits generated `ModulesLoader.cpp` to register
  Playerbots. That generated/module integration change is not a prerequisite
  of its one-line slot-GUID fix.
- **FACT:** No core battle-pet API in the reconstructed final state exists only
  for bots. Trainer manager, quest credit, request handler, weather, and wild
  spawn APIs are used by ordinary-player/core paths. Playerbots hunks can be
  excluded from a normal core backport.
- **FACT:** Any future Playerbots review must use a separate module-enabled,
  `PLAYERBOTS=1` build. The normal core review units below require no Playerbots.

## Later Battle-Pet Evolution

- **FACT:** `1e338126` makes Candidate 10's trainer data usable in the world,
  adjusts swap mechanics, and further changes weather; `987cdb97` guards aura
  stacking. Neither freezes the final state.
- **FACT:** `867b69b0` adds delayed-trigger logic, explicit aura expiration,
  active-pet swap turn suppression, battle finish/passive fixes, and first
  large wild spawn/loot batches. It also mixes Tillers work, which is excluded.
- **FACT:** `aeef0c9d` adds the XP-rate configuration and handlers for effects
  22, 28, 29, 33, 44, 52, 53, 59, 62, 66, 68, 79, 85, 100, 131, 135, 136,
  139, 145, 150, 160, 164, 165, 168, 169, 177, 179, 197, and 204, plus four
  Cataclysm wild-data files.
- **FACT:** `bd8ad551` is mixed predominantly with Tillers but its pet hunks are
  final: effects 61, 75, and 77 become live; weather gains explicit replacement
  and state tracking; passive/death/achievement behavior changes; two Northrend
  wild-data files are added. These hunks belong to Andrew final state; its
  Tillers code and unrelated SQL do not.
- **FACT:** No semantic battle-pet commit follows `bd8ad551`. `155f6444` adds
  declarations/includes for PCH-independent tests only. README refactors do not
  alter runtime behavior.

## Andrew vs Flamehawk Effect Comparison

Only IDs live-dispatched on both final branches are compared. `P[n]` means
`BattlePetAbilityEffectEntry::Properties[n]`. Both sides ultimately generate
existing health/state/aura events; “packet” notes describe produced effects,
not serializer changes.

| ID | Flamehawk final | Andrew final | Target/properties/state and packet-visible difference | Classification/evidence |
| --- | --- | --- | --- | --- |
| 22 | `Dummy`, no-op | `NegativeAura` | Flamehawk none; Andrew target, trigger aura, `P1` accuracy unless delayed, `P2` duration | `DIRECT_CONFLICT`; Andrew is functional but DBC/trigger evidence is missing. |
| 28 | positive aura family | `PositiveAura` | Both caster aura using trigger, `P1` accuracy, `P2` duration; stack/max conventions differ in family implementation | `SIMILAR_BUT_DIFFERENT`; neither has stronger client evidence. |
| 29 | state-bonus damage | `StateBonusDamage` | Both target damage; Andrew uses `P0` base, `P1` accuracy, caster state `P2` else target state `P3` to double | `SIMILAR_BUT_DIFFERENT`; formulas/conditional precedence require DBC. |
| 44 | heal from last hit | `HealWithLastHit` | caster; Andrew heals `P0` percent of `LAST_HIT_DEALT`, accuracy `P1` | `AGREES` in intent, formula/event details unproven. |
| 52 | negative aura/control family | `NegativeAura` | Andrew target trigger aura with delayed-hit special case; Flamehawk aura/control flags differ | `SIMILAR_BUT_DIFFERENT`; Andrew has later delayed-activation rationale only. |
| 53 | percentage heal | `HealPct` | caster; Andrew `P0` percent of target max, `P1` accuracy | `SIMILAR_BUT_DIFFERENT`; target alias makes it caster in Andrew. |
| 59 | low-health damage | `LowHpDamage` | target; Andrew `P0` damage doubled when caster health percent < `P2`, accuracy `P1` | `SIMILAR_BUT_DIFFERENT`; threshold side/formula conflict needs rows. |
| 61 | conditional percent heal | `HealCasterPercentNotState` | caster; Andrew requires caster state `P2` and target state `P3`, then heals caster `P0` percent max with `P1` accuracy | `SIMILAR_BUT_DIFFERENT`; handler name says “NotState” but code requires states, weakening evidence. |
| 62 | percent-health damage | `PctHealthDamage` | target; Andrew damages `P0` percent target max, `P1` accuracy, periodic if `P2` | `SIMILAR_BUT_DIFFERENT`; packet damage/periodic flags may differ. |
| 66 | execute/ruthless damage | `ExecuteDamage` | target; Andrew doubles `P0` if target health percent < `P2`, accuracy `P1` | `SIMILAR_BUT_DIFFERENT`; threshold semantics need DBC. |
| 68 | damage-family handler | `Sacrifice` | Andrew immediately `SetHealth(caster,0)`, then target damage `P0`/accuracy `P1`; Flamehawk does not share this self-kill contract | `DIRECT_CONFLICT`; death ordering requires trace. |
| 75 | aura/damage toggle family | `DamageToggleAura` | target mapping, but Andrew searches caster aura for trigger; if present expires and damages `P0`, else adds to caster for `P2` | `SIMILAR_BUT_DIFFERENT`; target/caster asymmetry is suspicious. |
| 77 | aura/damage toggle family | `DamageToggleAura` | same Andrew logic as 75 | `SIMILAR_BUT_DIFFERENT`; requires ability rows and two-cast trace. |
| 79 | aura/control family | `Damage` | Andrew plain target damage `P0`, accuracy `P1`, periodic `P2`; Flamehawk treats ID as aura behavior | `DIRECT_CONFLICT`. |
| 80 | weather/aura family | `WeatherAura` | Andrew `TARGET_ALL`, trigger weather, `P2` duration/`P3` max, all-roster auras plus null-pet state events/replacement; Flamehawk lacks this battle-wide state model | `DIRECT_CONFLICT`; Andrew is more complete structurally, not proven. |
| 85 | aura/control family | `PositiveAura` | Andrew caster trigger aura (`P1`, `P2`); Flamehawk's effect-specific aura semantics differ | `SIMILAR_BUT_DIFFERENT`. |
| 135 | kill/direct-health family | `KillActive` | both head/active target; Andrew actually performs ordinary calculated `P0` damage (`P1` accuracy, `P2` periodic), not unconditional kill | `DIRECT_CONFLICT`; Andrew's name contradicts operation. |
| 136 | `Cleansing` | `Cleanse` | caster; both expire auras, but Andrew expires every target aura without polarity/filter and emits normal expiry effects | `SIMILAR_BUT_DIFFERENT`; Andrew likely over-broad without DBC proof. |
| 164 | multistrike | `MultiStrike` | target; Andrew rolls `P1` chance and deals calculated `P0`, without an explicit accuracy calculation | `SIMILAR_BUT_DIFFERENT`; proc/chance and event count need trace. |
| 169 | damage/witching-or-weather conflict | `WeatherAura` | Flamehawk treats 169 as target damage-family behavior; Andrew maps caster then applies battle-wide trigger weather/state | `DIRECT_CONFLICT`; strongest example that neither fork is a specification. |
| 178 | baseline powerless aura modified by series | `PowerlessAura` | both target aura/state behavior descended from baseline; later surrounding computation differs | `AGREES` at dispatch level; exact state/aura lifecycle remains uncertain. |
| 197 | vengeance | `Vengeance` | target; Andrew uses `P0` percent of caster `LAST_HIT_TAKEN`, accuracy `P1`; Flamehawk has independently implemented vengeance semantics | `SIMILAR_BUT_DIFFERENT`; state timing requires trace. |

- **FACT:** Flamehawk additionally live-dispatches several IDs that Andrew
  leaves `HandleNull` (49, 65, 67, 72, 74, 78, 86, 111, 121, 128, 137, 140,
  141, 158, 170, 171, 172). Those are `ONE_SIDE_INCOMPLETE` in Andrew, not
  overlapping implementations. Andrew additionally handles 33, 100, 131, 139,
  145, 150, 160, 165, 168, 177, 179, and 204, which Task 019's final dispatch
  does not establish.
- **UNKNOWN:** No checked-in build-18414 extracted rows or packet captures
  establish targets, property meanings, signedness, trigger IDs, event order,
  or retail formulas. Commit date and handler count are not correctness evidence.

## Current Preservation Comparison

| Family | Classification | Finding |
| --- | --- | --- |
| Trainer battles | `ABSENT` | No trainer manager/type/data construction or complete request/gossip path. |
| Weather | `ABSENT` / `CONFLICTING_IMPLEMENTATION` | Baseline lacks Andrew's battle-wide weather state/replacement; existing aura machinery remains. |
| Login persistence | `PARTIALLY_PRESENT` | Load path exists, but slot GUIDs retain counter reconstruction rather than Andrew's `uint64` fix. |
| Battle result/death/swap | `CONFLICTING_IMPLEMENTATION` | Current baseline lacks final trainer state machine, swap suppression, passives, and wild-only death result. |
| Quest credit | `ABSENT` | No trainer-specific objective-type completion helper/call. |
| Ability effects | `PARTIALLY_PRESENT` / `CONFLICTING_IMPLEMENTATION` | Baseline handlers exist; Andrew's added/remapped set and formulas do not. |
| Wild spawn/world integration | `PARTIALLY_PRESENT` | Baseline spawn manager/pools exist; relation fix, duplicate-load structure, and geographic data batches differ/are absent. |
| Packet/session | `PARTIALLY_PRESENT` | Constants and common packet writers exist; trainer request registration/handler and event semantics are absent. |
| Migrations | `ABSENT` | Andrew trainer table/data/spawns and later wild batches are not present. |
| Playerbots boundary | `SUPERSEDED_BY_CURRENT` as out-of-scope boundary | Current has its own module history; Andrew bot commands are not required by core pet behavior and should not be imported. |

**FACT:** No family is classified `PRESENT_EXACTLY` as a complete Andrew unit.
Some unchanged baseline serializers/utilities are exact, but that does not make
the new behavior present.

## Proposed Final-State Backport Units

These are review units, not approved patches. “Build” means a future targeted
`game` build followed by `worldserver`, unless noted.

1. **A1 — slot GUID reconstruction.** Files/functions:
   `BattlePetMgr.cpp::LoadSlotsFromDb`. Source `2da8a377`, unchanged at tip.
   Prerequisite: prove DB column/full-GUID contract. SQL/opcode/DBC/Playerbots:
   none; persistence: high. Current: `PARTIALLY_PRESENT`. Tests: account pets
   with all three slots across logout/login, empty slots, multiple characters,
   saved health/XP/level unchanged. Build: `game`, `worldserver`.
2. **A2 — trainer data manager and lifetime.** `BattlePetTrainerMgr.{h,cpp}`,
   world startup, `PetBattleTeam::AddTrainerBattlePets`, owned-team destructor.
   Ancestry Candidate 10 plus final header hygiene. Requires trainer table and
   build-18414 species/breed/quality data; no opcode; persistence none; no bots.
   Current: `ABSENT`. Tests: 1/2/3-pet teams, invalid rows, reload, teardown.
3. **A3 — trainer initiation/session contract.** `BattlePetHandler.cpp`,
   `NPCHandler.cpp`, `QuestHandler.cpp`, `Player.cpp` gossip path,
   `Opcodes.cpp`, `WorldSession.h`, request structures. Candidate 10 through
   `1e338126`. Requires A2, trainer data/spawns, exact 18414 request trace and
   terrain/position evidence. SQL medium; packet high; DBC low; no bots.
   Current: `ABSENT`/`REQUIRES_BUILD18414_TRACE`. Tests: every failure reason,
   gossip and client request, position/facing, concurrent request, cleanup.
4. **A4 — trainer round/death/swap state machine.** `PetBattleTeam` move/ready
   methods and `PetBattle::{HandleRound,SwapActivePet,Kill,EndBattle}`. Ancestry
   Candidate 10 -> `c6400a29` -> `1e338126` -> `867b69b0` -> `bd8ad551`.
   Requires A2/A3; SQL trainer teams; packet/event high; DBC ability/passive
   medium; persistence health/XP; no bots. Current: `CONFLICTING_IMPLEMENTATION`.
   Tests: each pet death, voluntary/forced swap, skip turn, simultaneous death,
   win/loss/forfeit/disconnect, exact round/final packets.
5. **A5 — trainer quest/achievement result.**
   `Player::PetBattleCompleteQuest`, declaration, `PetBattle::EndBattle`, later
   achievement hunk. Candidate 11 -> `bd8ad551`. Requires A4 and objective data;
   no new schema/opcode; DBC none; persistence quest objective; no bots. Current
   `ABSENT`. Tests: matching/nonmatching/multi-count objectives, duplicate win,
   loss/forfeit, achievement family mask, relog.
6. **A6 — weather final state.** Effect mappings 80/169 and handlers,
   `BattlePetAura`, weather fields/accessors and
   `PetBattle::{ApplyWeatherStates,ClearWeatherStates,EndBattle}`. Ancestry
   `c440e10f` -> Candidate 11 -> `1e338126`/`987cdb97` -> `aeef0c9d` ->
   `bd8ad551`. No SQL/bots/persistence; packet and DBC dependencies high.
   Current `ABSENT`/conflicting with Flamehawk. Tests: same/different weather,
   expiry/replacement, every roster slot, elemental pet, swap, battle end,
   signed state and exact events.
7. **A7 — delayed/harmful aura lifecycle.** `BattlePetAbilityEffect` negative
   aura/delayed trigger/stack guard and `BattlePetAura` expiry. `987cdb97` ->
   `867b69b0` -> final. No SQL/bots/persistence; DBC and packet high. Current
   `PARTIALLY_PRESENT`. Tests: untargetable at cast/expiry, duplicate harmful
   cast, dispel, duration, proc-on-remove.
8. **A8 — selected direct effect families.** Split into table-driven subreviews:
   heal/damage (29/33/44/53/59/61/62/66/68/100/160/164/197), aura/control
   (22/28/52/75/77/85/131/136/139/145/150/165/168/177/178/179/204), and
   effect 135/death. Files are ability-effect pair plus battle primitives.
   `aeef0c9d`/`bd8ad551`; no SQL/bots, DBC/packet/runtime high, health persistence
   medium. Current absent/conflicting. Tests require authoritative row fixtures
   and captured event sequences. Do not merge with Flamehawk.
9. **A9 — wild relation/grid core.** `BattlePetSpawnMgr`, Creature/GameObject,
   Map, ObjectGridLoader and Player integration from Candidate 11/follow-ups.
   Requires baseline pool schema; SQL optional for code review; no trainer or
   bots, but global core/regression risk high. Current partial. Build broadly
   through `worldserver`; tests: map/grid load/unload, replacement, respawn,
   events, duplicates, pet battles disabled/enabled.
10. **A10 — geographic wild data.** The nine wild-region SQL files, separately
    from code and from `pet_supplies` loot. Requires A9, deployed-world audit,
    templates and DBC species. SQL high; packets/persistence/bots none. Current
    absent. Runtime tests per zone/pool/direct spawn; never batch with trainer SQL.
11. **A11 — trainer schema/team data/spawns.** The three ordered trainer SQL
    files, reviewed independently: schema, addon-derived teams, then destructive
    spawn file. Requires A2/A3 and deployed DB diff. SQL high; DBC high;
    packet indirectly A3; no persistence/bots. Current absent. Tests: migration
    on a copy, referential/data audit, NPC gossip, coordinates and respawn.
12. **A12 — pet XP rate.** `World.{h,cpp}`, `worldserver.conf.dist`, and the
    multiplication in `EndBattle`, from `aeef0c9d`. No SQL/opcode/bots; DBC
    none; persistence high. Current absent. Tests: 0/1/fractional/high rate,
    level caps, participant division, trainer/wild distinction, save/relog.

## Suggested First Isolated Reviews

At most three units meet the narrowness/no-bot/no-broad-migration preference;
none is claimed correct:

1. **A1, slot GUID reconstruction** — one three-line functional change, no SQL,
   opcode, Playerbots, or DBC dependency. First establish the deployed column
   and `ObjectGuid` representation, then test login/relog.
2. **A12, pet XP rate** — small configuration/computation unit with no opcode
   or schema dependency. It still needs deterministic XP and persistence tests
   and evidence that the rate belongs in this project.
3. **A5, quest objective helper (review-only extraction)** — narrow core code
   and no packet-value or schema change, but meaningful runtime validation
   requires a functioning trainer harness/A4. Review the helper independently;
   do not deploy it as an orphan.

**FACT:** Trainer initiation, trainer death/swap, weather, ability effects, and
world/grid changes do not qualify as first implementation units: all retain
build-18414 packet/DBC/world-data uncertainty or broad regression scope.

## Unresolved Build-18414 Evidence

1. A capture proving `CMSG_PET_BATTLE_REQUEST_UPDATE` opcode `0x0377`, GUID bit/
   byte sequence, XYZ order, optional bits, facing/location values, and response
   reasons.
2. Initial, first-round, normal-round, forced-swap, final-round, finished, and
   failure packets for trainer battles, including exact NPC/player placement.
3. Death/result behavior for active/background, trainer/wild, simultaneous,
   periodic, passive-delayed, capture, forfeit, and last-pet cases.
4. Authoritative effect/turn/state/species/breed/quality rows for every added or
   conflicting handler, especially effects 22, 61, 68, 75, 77, 80, 135, 136,
   164, 169, 178, and 197.
5. Weather duration, same-weather recast, replacement/expiry order, state
   signedness, elemental immunity, incoming pet behavior, and final cleanup.
6. Account battle-pet and loadout slot database samples proving full GUID versus
   counter storage, plus multi-character logout/login persistence.
7. Retail trainer teams, selected abilities, levels/quality/breeds, quest chains,
   objective counts, achievements, reward loot, coordinates, orientation, and
   NPC movement.
8. Grid unload/reload and spawn ownership traces for type-8 replacement and
   type-14 direct wild pets, including duplicates, respawn, events, and maps.
9. XP formula/rate behavior, participant sharing, level-up/cap packets, saved
   health/XP/level, and trainer-versus-wild distinctions.
10. Disconnect/relog/map unload during request, active battle, forced swap, and
    finalization. No fork source proves those lifecycle edges.

Until these artifacts exist, Andrew's final branch is comparative implementation
evidence and a source of bounded review units, not a retail-correct patch set.
Candidates 10 and 11 must not be cherry-picked or treated as standalone fixes.
