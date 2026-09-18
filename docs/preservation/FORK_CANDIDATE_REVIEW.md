# Fork Candidate Deep Review

## Scope

This Task 008 review is limited to the 18 substantive commits selected by Task
007. It is an archaeology and planning record for the Mists of Pandaria 5.4.8
client (build 18414), not an assertion that fork behavior is correct. No patch
was imported, no database was configured, and no build or runtime test was
performed.

Evidence labels have the following meaning:

- **FACT**: directly observed in Git metadata, a patch, or the current tree.
- **INFERENCE**: a technical conclusion supported by those observations but not
  demonstrated at build/runtime.
- **UNKNOWN**: evidence needed to decide is unavailable or outside this review.

Candidate numbers remain the Task 007 numbers. All repositories below use the
recorded default branch `master`.

## Starting Repository State

- **FACT:** the working tree was clean on branch `work` at
  `1c2aa24d8d3308fd7419b6e8efbe808457f5955d` (`Merge pull request #7 from
  firstuserlastname123/codex/conduct-fork-archaeology-inventory`).
- **FACT:** the preceding commit was Task 007's
  `a7c9dfa chore(preservation): inventory fork archaeology candidates`; the
  six-commit history also contained the merged Task 006 and Task 005 records.
- **INFERENCE:** although the local task branch is named `work`, its HEAD is the
  latest recorded `preservation/main` integration history supplied for this
  task. `master` was not checked out.

## Candidate Inventory

| # | Repository | SHA; author date; author | Subject | Architecture | Task 007 state |
|---:|---|---|---|---|---|
| 1 | `worldofd2/Legends-of-Azeroth-Pandaria-5.4.8` | `5b338780d9bf8184158b3f368cddf6f8e1c3ce87`; 2024-11-14; Syco `<62126578+worldofd2@users.noreply.github.com>` | `[Core/Custom] Fix Teleport Locations` | legacy/custom scripts | `LIKELY_RELEVANT` |
| 2 | `drikishuk/Legends-of-Azeroth-Pandaria-5.4.8` | `9d824dc77097190e3840f69e8b31ecb47c4a461c`; 2024-10-20; Ben Aldrich `<benjaminaldrich9@gmail.com>` | `Ben Commit Changes` | core, spells, DB API, custom scripts | `NEEDS_CONTEXT` |
| 3 | `drikishuk/Legends-of-Azeroth-Pandaria-5.4.8` | `7f658ec495e40c8480259c7ec946d28972af562c`; 2024-10-27; Ben Aldrich `<benjaminaldrich9@gmail.com>` | `Adding nightfall scripts` | custom scripts, DBC, extractor | `NEEDS_CONTEXT` |
| 4 | `TheYeti/Legends` | `6e35544f8016d54db74cefde4eb2206f268d333d`; 2024-10-19; The Yeti `<matt@theyeticave.net>` | `Make reputation standing account wide at exalted.` | reputation/auth DB | `NEEDS_CONTEXT` |
| 5 | `TheYeti/Legends` | `baa58b78ceea018e954db33dfb79b560851c9a75`; 2024-10-17; The Yeti `<matt@theyeticave.net>` | `Redo all the work on the reagent bank` | custom script/auth DB/build | `NEEDS_CONTEXT` |
| 6 | `Flamehawk147/LOACore_548` | `2089a5470f9b981854e016ec8e5c2f81a5436712`; 2024-11-10; Flamehawk147 `<edwardmccreary72@gmail.com>` | `Fixes` | core, LFG, scenarios/raid/zones | `NEEDS_CONTEXT` |
| 7 | `Flamehawk147/LOACore_548` | `e91819a4c2acb6cc8eea09ed264d30aec6ed12b9`; 2024-11-15; Flamehawk147 `<edwardmccreary72@gmail.com>` | `Update more PB` | battle pets/core | `LIKELY_RELEVANT` |
| 8 | `Dem0riaN/Legends-of-Azeroth-Pandaria-5.4.8` | `57fee55d98e62c0c706425f719c7536e23807bf8`; 2025-11-02; Dem0riaN `<90118628+Dem0riaN@users.noreply.github.com>` | `fix build with openssl 1.1.1w` | cryptography/build | `LIKELY_RELEVANT` |
| 9 | `MityaFoxy/Legends-of-Azeroth-Pandaria-5.4.8` | `481dd4bc5d82a7caed094ad175f54ed1c3c1d4a3`; 2026-09-15; google-labs-jules[bot] `<161369871+google-labs-jules[bot]@users.noreply.github.com>` | `Fix multiple null pointer dereference issues and redundant checks` | arena/Pandaria encounters | `LIKELY_RELEVANT` |
| 10 | `andrew-downey/Legends-of-Azeroth-Pandaria-5.4.8` | `39205279cebebd1d2c83000a0e9043e68047e659`; 2026-05-19; Andrew Downey `<andrew@anooserve.com>` | `Removing old migrations, adding new ones, battle pet trainers now battle you, but get stuck after you defeat their first pet. Also they stand right in the middle of the battle, but hey` | battle pets/opcodes/world SQL | `NEEDS_CONTEXT` |
| 11 | same | `100303ca22b08f04ee5620dc1622617d5a680cdd`; 2026-05-22; Andrew Downey `<andrew@anooserve.com>` | `petbattle: fix weather effects, quest credit, and various battle bugs` | battle pets/Playerbots/maps | `LIKELY_RELEVANT` |
| 12 | same | `1e138b1f1d7e1660eb28b647b7193afe961acb72`; 2026-06-03; Andrew Downey `<andrew@anooserve.com>` | `Tillers content and fixes, working tutorial quest chain, working on layering system` | Tillers/phasing/SQL | `LIKELY_RELEVANT` |
| 13 | same | `bd8ad5515418d94abd35ba7bf71430979243a09c`; 2026-06-21; Andrew Downey `<andrew@anooserve.com>` | `Tillers farm: Phase 1 + Phase 2.2 planting→harvest retail cycle` | Tillers/battle pets/SQL/achievements | `LIKELY_RELEVANT` |
| 14 | `bondarenkoandriy/Legends-of-Azeroth-Pandaria-5.4.8` | `72e7fb1d492c45398630e9ea52cb695737d7865d`; 2026-09-11; Andrey B `<111489336+bondarenkoandriy@users.noreply.github.com>` | `Update ARC4.cpp` | cryptography | `LIKELY_RELEVANT` |
| 15 | same | `35a6fbc07d713a864833d8b9b5cccd036b86f6c8`; 2026-09-11; Andrey B `<111489336+bondarenkoandriy@users.noreply.github.com>` | `Update default tolerance value in ChaseAngle constructor` | movement | `NEEDS_CONTEXT` |
| 16 | `HordeXL/Legends-of-Azeroth-Pandaria-5.4.8` | `c652a9d8ef75949337760497f121b10f7625898d`; 2026-09-17; hordexl `<wp0770@gmail.com>` | `修改配置文件：增加修改playerbots数据库连接配置` | Playerbots/config | `NEEDS_CONTEXT` |
| 17 | `ingussuveiks-dev/Legends-of-Azeroth-Pandaria-5.4.8` | `b134ab15a1643b74751e9dc8b9c576858aa86073`; 2026-06-17; ingussuveiks-dev `<ingussuveiks@gmail.com>` | `world struktūra bez datiem iekšā` | world schema | `LIKELY_RELEVANT` |
| 18 | same | `00d1cd9976a874c65daf901e1a2740d4393e7be5`; 2026-09-17; ingussuveiks-dev `<ingussuveiks@gmail.com>` | `Fix Gate cannon feedback and stray headbutt stun` | Gate dungeon/spells | `LIKELY_RELEVANT` |

## Detailed Candidate Reviews

### 1 — custom Cataclysm teleport destinations

- **Affected files (FACT):** `src/server/scripts/Custom/emo_teleporter.cpp`
  (23 insertions, 23 deletions).
- **Technical description (FACT):** enables 14 Cataclysm dungeon and six raid
  menu rows that were commented placeholders, supplying map IDs, coordinates,
  orientation, and a raid-menu back row. It does not alter teleporter control
  flow.
- **Current-tree comparison (FACT — D):** the current file retains the
  placeholders; the one-commit patch passes `git apply --check`. No equivalent
  destination table was found.
- **Dependencies/compatibility (INFERENCE):** no code dependency is apparent;
  semantic dependency is registration/use of the custom NPC and correctness of
  every coordinate. It uses the current row shape.
- **State:** `READY_FOR_ISOLATED_BACKPORT` — mechanically isolated and
  structurally compatible, but not proven accurate.
- **Later validation:** build `scripts`, link `worldserver`, confirm script-name
  and NPC registration/world data, then visit every destination with build
  18414 and verify safe placement and back navigation.

### 2 — Nightfall/custom-server bundle

- **Affected files (FACT):** `src/server/database/Database/Implementation/{CharacterDatabase.cpp,WorldDatabase.cpp}`;
  `src/server/game/BattlePay/{ServiceBoost.cpp,ServiceBoost.h}`;
  `src/server/game/Entities/Player/Player.cpp`;
  `src/server/game/Miscellaneous/SharedDefines.h`;
  `src/server/game/Scripting/{ScriptMgr.cpp,ScriptMgr.h}`;
  `src/server/game/Spells/SpellEffects.cpp`;
  `src/server/scripts/Commands/{cs_modify.cpp,cs_wp.cpp}`;
  `src/server/scripts/Custom/{custom_player_npcs.cpp,custom_player_scripts.cpp,nightfall_scripts.cpp}`;
  `src/server/scripts/ScriptLoader/ScriptLoader.cpp`; and
  `src/server/scripts/Spells/spell_shaman.cpp` (746 additions, 28 deletions).
- **Technical description (FACT):** combines a level-20 BattlePay boost and
  inventory/spell changes, specialization/glyph/talent behavior, a new player
  specialization hook, 36 custom “Fractured” aura IDs in the 600000 range,
  waypoint insertion/delay, display-ID widening, shaman behavior, wandering
  mage/quest NPC scripts, and loader registrations.
- **Current-tree comparison (FACT — D):** custom files, hook, IDs, and bundled
  behavior are absent; the patch conflicts in evolved core files.
- **Dependencies (UNKNOWN):** quests 900001/900002, custom creatures, spells
  600000–600035, DB rows, BattlePay policy, and configuration are not supplied.
- **Compatibility (INFERENCE):** mixed custom-server rules (including a changed
  talent-point formula) are not a coherent 18414 preservation fix and must be
  separated before evaluation.
- **State:** `NEEDS_CONTEXT`.
- **Later validation:** first produce per-subsystem patches and provenance for
  all custom IDs; then targeted `game`/`scripts` compile and `worldserver` link,
  disposable DB checks, boost rollback/security tests, and build-18414 quest,
  glyph, talent, and spell tests.

### 3 — Nightfall continuation and extractor/DBC edits

- **Affected files (FACT):** `src/server/game/DataStores/DBCStores.cpp`;
  `src/server/scripts/Custom/nightfall_scripts.cpp`;
  `src/server/shared/DataStores/DBCEnums.h`; `src/tools/map_extractor/System.cpp`
  (95 additions, 58 deletions).
- **Technical description (FACT):** continues Nightfall scripts (including a
  Temporal Shield aura accumulator/heal), changes currency constants/labels,
  changes DBC loading diagnostics/handling, and changes the extractor's MPQ
  list/liquid threshold behavior.
- **Current-tree comparison (FACT — D):** the custom script is absent and core
  hunks conflict; no equivalent full change was found.
- **Dependencies (FACT/UNKNOWN):** depends on candidate 2 and intervening
  `b179a560d9ed9c595ff892d39f1ee90906bd7fce` (“latest nightfall scripts”);
  required custom spell/DBC data remain **UNKNOWN**.
- **Compatibility (INFERENCE):** mixing runtime DBC enums, a game script, and
  extraction changes prevents an isolated backport; extractor assumptions must
  be verified specifically against build 18414 data.
- **State:** `READY_WITH_DEPENDENCIES`.
- **Later validation:** split into script, DBC, and tool units; compile
  `game`/`scripts` and map extractor separately; compare 18414 DBC layouts and
  extracted output; runtime-test the aura with spell data.

### 4 — account-wide exalted reputation

- **Affected files (FACT):** `src/server/database/Database/Implementation/{LoginDatabase.cpp,LoginDatabase.h}`;
  `src/server/game/Reputation/ReputationMgr.cpp` (37 additions, one deletion).
- **Technical description (FACT):** writes exalted faction/flags keyed by
  account into an `account_factions` auth table and, while loading a character,
  forces matching faction standing to 42000 and updates rank counters.
- **Current-tree comparison (FACT — D):** no `account_factions` table access or
  equivalent implementation exists.
- **Dependencies (FACT/UNKNOWN):** the patch supplies no `CREATE TABLE`; its
  load query selects `WHERE guid = character GUID` while writes bind account ID
  to column `id`, so the intended schema/column contract is **UNKNOWN**.
- **Compatibility (INFERENCE):** direct synchronous auth writes from reputation
  save and the key mismatch require design/security review; retail scope is not
  established.
- **State:** `REQUIRES_DATABASE_PROVENANCE`.
- **Later validation:** obtain schema/source and product requirement, review
  account isolation and faction exceptions, migrate a disposable auth DB,
  compile/link `worldserver`, and test multiple characters/accounts.

### 5 — custom reagent-bank NPC

- **Affected files (FACT):** `.gitignore`;
  `src/server/database/Database/Implementation/{LoginDatabase.cpp,LoginDatabase.h}`;
  `src/server/scripts/CMakeLists.txt`;
  `src/server/scripts/Custom/{ReagentBank.cpp,ReagentBank.h}`;
  `src/server/scripts/ScriptLoader/ScriptLoader.cpp` (342 additions, two
  deletions).
- **Technical description (FACT):** adds an account-level `reagent_bank` store,
  gossip UI with paging, bulk removal of trade goods/gems from bags, and stack
  withdrawal. It mixes async callbacks with player pointers and DB mutation.
- **Current-tree comparison (FACT — D):** files/statements are absent.
- **Dependencies (FACT/UNKNOWN):** no table migration or NPC/script-name world
  row is included. Later same-fork commit
  `a99957c0` is explicitly “Remove reagent bank entirely for now.”
- **Compatibility (FACT/INFERENCE):** loader declaration calls
  `AddSC_reagent_bank()` while the added definition is
  `AddSC_reagent_bankScripts()`, so the commit is internally unlink-incompatible;
  its CMake addition is MSVC/PCH-conditional and does not establish portable
  inclusion.
- **State:** `NEEDS_CONTEXT` — abandoned and incomplete, not a backport unit.
- **Later validation:** only after schema/world provenance and lifecycle design:
  fix naming/build integration in a future implementation, compile/link scripts,
  test transaction atomicity, disconnect races, inventory-full behavior,
  duplicate/loss prevention, and account concurrency.

### 6 — broad “Fixes” bundle

- **Affected files (FACT):**
  `src/server/database/Database/Implementation/LoginDatabase.cpp`;
  `src/server/game/DungeonFinding/{LFGMgr.cpp,LFGQueue.cpp}`;
  `src/server/game/Entities/{Creature/Creature.cpp,Player/Player.cpp,Player/Player.h,Unit/Unit.cpp,Unit/Unit.h}`;
  `src/server/scripts/EasternKingdoms/zone_elwynn_forest.cpp`;
  ten scenario files under `Pandaria/Scenarios/{AssaultOnZanvess,BattleOnTheHighSeas,BloodInTheSnow,GreenstoneVillage,SecretsOfRagefire,UngaIngoo}`;
  `Pandaria/SiegeOfOrgrimmar/boss_immerseus.cpp`; and five
  `Pandaria/TimelessIsle` files (`boss_chi_ji.cpp`, `boss_niuzao.cpp`,
  `boss_xuen.cpp`, `boss_yu_lon.cpp`, `zone_timeless_isle.cpp`) (2,105
  additions, 317 deletions).
- **Technical description (FACT):** combines donation-point auth updates,
  random battle-pet rewards, LFG role counts, random creature stat generation,
  player regeneration/currency changes, bespoke `_hasBot`/`bot_Class` fields,
  Elwynn custom NPC bots, and large scenario/encounter rewrites.
- **Current-tree comparison (FACT — C/D):** base encounters exist, but this
  bundled implementation is absent and core hunks conflict; no whole-patch
  equivalent exists.
- **Dependencies (FACT/UNKNOWN):** relies on earlier fork customization,
  including `994f173d` (NPC bots), `26e16ed4` and `0c279a48` (battle pets), plus
  custom auth/world tables and entries not supplied here.
- **Compatibility (INFERENCE):** custom bot/donation semantics overlap core and
  content changes and cannot be reviewed as one preservation change.
- **State:** `NEEDS_CONTEXT`.
- **Later validation:** decompose per function/encounter, establish DB and spell
  provenance, targeted `game`/`scripts` compiles and worldserver link, then LFG,
  economy/security, encounter, and build-18414 gameplay tests.

### 7 — Flamehawk battle-pet ability/state update

- **Affected files (FACT):** `src/server/game/BattlePet/{BattlePetAbilityEffect.cpp,BattlePetAbilityEffect.h,PetBattle.cpp,PetBattle.h}`
  (164 additions, 20 deletions).
- **Technical description (FACT):** extends ability effect dispatch/handling,
  aura/state updates, round processing and pet-battle state representation; it
  is an incremental patch, not a standalone implementation.
- **Current-tree comparison (FACT — D):** corresponding current files predate
  this fork series and the patch conflicts; no equivalent complete series was
  found.
- **Dependencies (FACT):** `26e16ed4`, `0c279a48`, candidate 6,
  `aec3e154`, `61e1f477`, and `0f46d20c` are direct earlier battle-pet series
  commits.
- **Compatibility (UNKNOWN):** packet/opcode, DBC effect semantics, persistence,
  and exact final-state behavior cannot be established from this tip commit.
- **State:** `READY_WITH_DEPENDENCIES`.
- **Later validation:** review the full range/final-state diff; targeted `game`
  compile and `worldserver` link; DBC effect comparison; deterministic round,
  aura, death/swap, packet, persistence, and 18414 client tests.

### 8 — OpenSSL 1.1.1 context initialization removal

- **Affected files (FACT):** `src/common/Cryptography/{AES.cpp,ARC4.cpp}` (two
  deletions).
- **Technical description (FACT):** removes redundant/deprecated
  `EVP_CIPHER_CTX_init` calls immediately after `EVP_CIPHER_CTX_new`.
- **Current-tree comparison (FACT — A):** exact behavior is already present in
  current commit `5b50aeb974a468ee5be428fc9652b4218dc5b7f9`; the current patch-ID is
  `18d7327fbd7c0d017e76c6a15ef8848f5a3b4d06` and current files contain neither
  call.
- **Dependencies/compatibility (FACT):** none beyond supported OpenSSL APIs.
- **State:** `LIKELY_ALREADY_PRESENT` (in fact patch-equivalent by content).
- **Later validation:** no backport; retain existing OpenSSL 1.1.1 and 3.x build
  matrix coverage.

### 9 — mixed null-check changes

- **Affected files (FACT):** `src/server/game/Battlegrounds/ArenaTeam.cpp` and
  11 scripts: `GateOfTheSettingSun/boss_raigonn.cpp`,
  `HeartOfFear/heart_of_fear.cpp`, `MogushanVault/boss_stone_guard.cpp`,
  `Scenarios/GreenstoneVillage/greenstone_village.cpp`,
  `ShadopanMonastery/boss_master_snowdrift.cpp`,
  `SiegeOfOrgrimmar/{boss_garrosh_hellscream.cpp,boss_korkron_dark_shamans.cpp,boss_norushen.cpp}`,
  `ThroneOfThunder/{boss_council_of_elders.cpp,throne_of_thunder.cpp}`, and
  `zone_the_veiled_stair.cpp` (56 additions, 26 deletions).
- **Technical description (FACT):** adds guards around several `instance`,
  target, player, spell, and summoner dereferences and repairs predicate
  grouping. It also removes existing guards in ArenaTeam, Stone Guard,
  Snowdrift, and Dark Shamans; therefore not every hunk is a null-safety gain.
- **Current-tree comparison (FACT — D):** all 12 source paths match sufficiently
  for the patch to pass `git apply --check`; the changes are absent.
- **Dependencies (FACT):** none apparent. **UNKNOWN:** reproduction evidence for
  each alleged null path.
- **Compatibility (INFERENCE):** structurally compatible, but bundling unrelated
  encounters and guard removals makes a direct backport unsafe.
- **State:** `REQUIRES_RUNTIME_EVIDENCE`.
- **Later validation:** split per function, statically prove constructor/lifecycle
  invariants, reproduce each crash, compile `game`/`scripts`, link worldserver,
  and regression-test every affected encounter/scenario with build 18414.

### 10 — incomplete battle-pet trainer battles

- **Affected files (FACT):** `AGENTS.md`; 14 deleted
  `sql/updates/master/auth/old/*` files; new
  `sql/updates/world/2026_05_17_0{0,1}_battle_pet_trainer*.sql`;
  `src/server/game/BattlePet/{BattlePet.cpp,BattlePetTrainerMgr.cpp,BattlePetTrainerMgr.h,PetBattle.cpp,PetBattle.h}`;
  `Entities/Creature/GossipDef.h`, `Entities/Player/Player.cpp`;
  `Handlers/{BattlePetHandler.cpp,NPCHandler.cpp,QuestHandler.cpp}`;
  `Server/Protocol/Opcodes.cpp`, `Server/WorldSession.h`, and `World/World.cpp`
  (1,063 additions, 297 deletions).
- **Technical description (FACT):** adds a trainer manager and SQL data, wires
  gossip/quest handlers and sessions into trainer battles, changes battle
  lifecycle, and registers/changes opcode handling. Its own subject records a
  stuck battle after the first opposing pet and incorrect trainer placement.
- **Current-tree comparison (FACT — D):** trainer manager and migrations are
  absent; current APIs are earlier and the raw patch also collides with the
  repository's existing `AGENTS.md`.
- **Dependencies (FACT):** `c440e10f` (weather handlers), `2da8a377` (login
  load), `77711e7a` (migrations), and merge `99801947`; later fix
  `c6400a29` and candidate 11 are required to assess final behavior.
- **Compatibility (UNKNOWN):** opcode value/direction, trainer SQL baseline,
  packet layouts, and migration deletions are not validated for canonical
  build 18414.
- **State:** `NEEDS_CONTEXT`.
- **Later validation:** final-state range review without metadata/deletion noise,
  schema and migration-order test in disposable DBs, targeted `game` build and
  worldserver link, packet capture review, multi-pet win/loss/forfeit/relog and
  quest-credit tests with client 18414.

### 11 — battle-pet weather/world/Playerbots integration

- **Affected files (FACT):** `AGENTS.md`, `PETBATTLES.md`, `build.sh`;
  `modules/mod_playerbots/src/{Manager/RandomPlayerbotMgr.cpp,Manager/RandomPlayerbotMgr.h,cs_playerbots.cpp}`;
  `src/server/game/BattlePet/{BattlePetAbilityEffect.cpp,BattlePetSpawnMgr.cpp,PetBattle.cpp}`;
  `Entities/{Creature/Creature.cpp,Creature/Creature.h,GameObject/GameObject.cpp,GameObject/GameObject.h,Player/Player.cpp,Player/Player.h}`;
  `Grids/ObjectGridLoader.cpp`; `Maps/{Map.cpp,Map.h}` (333 additions, 54
  deletions).
- **Technical description (FACT):** fixes battle weather/state effects and quest
  credit, changes battle results/spawns, exposes creature/gameobject grid
  bookkeeping, and adds Playerbots commands/manager behavior around battle pets.
- **Current-tree comparison (FACT — D):** implementation is absent and depends
  on APIs introduced earlier in the fork; patch conflicts.
- **Dependencies (FACT):** candidate 10, `c6400a29`, `ad520857`, and the earlier
  weather/login commits listed for candidate 10.
- **Compatibility (INFERENCE):** crosses `game`, maps/grids and optional
  Playerbots; cannot be validated by the normal modules-disabled profile.
- **State:** `READY_WITH_DEPENDENCIES`.
- **Later validation:** final series review, normal game/worldserver build plus a
  separate module-enabled `PLAYERBOTS=1` build, DB/config review, deterministic
  weather/credit/spawn/grid tests, bot regressions, persistence, and 18414 client
  packet/runtime tests.

### 12 — Tillers tutorial/farm/phasing iteration

- **Affected files (FACT):** `AGENTS.md`, `SAI.md`, `TODO.md`; character SQL
  `2026_06_02_01_tillers_farm_state_bitmask.sql` and
  `2026_06_02_03_tillers_farm_state_schema.sql`; 13 world migrations from
  `2026_06_01_00_tillers_farm_phase_fix.sql` through
  `2026_06_03_06_tillers_gina_spawn.sql`; `Entities/Object/{Object.cpp,Object.h}`;
  `scripts/Commands/cs_tillers.cpp`; and
  `scripts/Pandaria/{TillersFarmMgr.cpp,TillersFarmMgr.h,TillersSoilScript.cpp,TillersZoneHooks.cpp}`
  (1,237 additions, 351 deletions).
- **Technical description (FACT):** revises persistent farm state/plot unlocks,
  private phasing/layering, dynamic Farmer Yoon/obstacle spawn positions,
  tutorial quest SAI/data, soil interaction, daily gates, and admin commands.
- **Current-tree comparison (FACT — D):** no Tillers manager/scripts or these
  tables/migrations exist in the current tree.
- **Dependencies (FACT):** earlier series begins `897bbf8e`, followed by
  `319ab1fa`, `a008e984`, `1bb75c28`, `dc22704`, `b4a0c209`, `41e67ba1`, and
  `6115dcae`; candidate 13 later refactors/extends it.
- **Compatibility (UNKNOWN):** world content baseline, phase masks, existing
  object rows and migration ordering are unresolved.
- **State:** `READY_WITH_DEPENDENCIES`.
- **Later validation:** final-state diff first; disposable character/world DB
  migration and rollback test; `scripts` compile/worldserver link; script-name
  and SAI validation; per-character isolation, relog/reset/daily behavior, full
  quest chain and phasing with build 18414.

### 13 — later Tillers planting/harvest plus unrelated data

- **Affected files (FACT):** `AGENTS.md`, `TODO.md`, new `TILLERS-PLAN.md`, moves
  of `BATTLEPETS.md`/`SAI.md` to `research/`; character migration
  `2026_06_21_01_tillers_remove_last_growth_tick.sql`; 11 world migrations
  `2026_06_12_04_northrend_wild_pets_1.sql` through
  `2026_06_20_01_tillers_companions.sql` (including wild pets, Taurast,
  Auchindoun, flying, quest/creature, undead-start, reputation and companion
  data); `database/.../{CharacterDatabase.cpp,CharacterDatabase.h}`;
  `game/Achievements/AchievementMgr.cpp`;
  `game/BattlePet/{BattlePet.h,BattlePetAbilityEffect.cpp,BattlePetAbilityEffect.h,PetBattle.cpp,PetBattle.h}`;
  `scripts/Pandaria/{CMakeLists.txt,TillersFarmMgr.cpp,TillersFarmMgr.h,TillersWorldHooks.cpp}`;
  and `scripts/ScriptLoader/ScriptLoader.cpp` (2,904 additions, 131 deletions).
- **Technical description (FACT):** implements crop problems, watering/pest
  transitions, harvest rewards/seeds/reputation, daily world-state advancement,
  farm companions, achievement criteria, and more battle-pet effects/weather;
  most included world SQL is unrelated to Tillers.
- **Current-tree comparison (FACT — D):** all feature files/schema are absent;
  it cannot apply without earlier series.
- **Dependencies (FACT):** candidate 12 plus `867b69b0` and `aeef0c9d`; transitively
  the full candidate 10–12 battle-pet/Tillers lineage. Later fork history may
  further change this state (**UNKNOWN** within the frozen candidate set).
- **Compatibility (INFERENCE):** commit boundaries are unsuitable: Tillers,
  battle pets, achievements, and unrelated bulk world data require separate
  provenance and units.
- **State:** `READY_WITH_DEPENDENCIES`.
- **Later validation:** isolate final Tillers code/schema from battle-pet and
  bulk-data changes; disposable migrations; scripts/game builds and worldserver
  link; crop timing/restart/concurrency, rewards/reputation/achievement,
  companions/phasing, battle effects, and build-18414 gameplay tests.

### 14 — OpenSSL 3 legacy-provider ARC4

- **Affected files (FACT):** `src/common/Cryptography/ARC4.cpp` (59 additions,
  six deletions).
- **Technical description (FACT):** on OpenSSL 3 loads default and legacy
  providers, explicitly fetches RC4 from `provider=legacy`, asserts allocations,
  and casts `size_t` lengths to the OpenSSL `int` API. Pre-3 behavior remains
  `EVP_rc4()`.
- **Current-tree comparison (FACT — D):** current code fetches `RC4` without
  explicitly loading/selecting the legacy provider. The patch passes
  `git apply --check`.
- **Dependencies (FACT):** candidate 8's initialization deletion is already in
  current tree. No other source dependency is apparent.
- **Compatibility (INFERENCE):** structurally compatible, but provider lifetime,
  repeated initialization, integer narrowing and OpenSSL configuration require
  review; this is not gameplay behavior.
- **State:** `READY_FOR_ISOLATED_BACKPORT`.
- **Later validation:** targeted `common`, `authserver`, and `worldserver` builds
  against supported OpenSSL 1.1.1 and 3.0–3.1.1; startup/auth handshake and
  crypto known-answer/regression checks; test with legacy provider unavailable.

### 15 — ChaseAngle spelling of π/4

- **Affected files (FACT):** `src/server/game/Movement/MovementDefines.h` (one
  line changed).
- **Technical description (FACT):** replaces default `M_PI_4` with literal
  `0.7853981633974483f`.
- **Current-tree comparison (FACT — B):** current default is `M_PI_4`; the
  proposed float literal denotes the same π/4 tolerance after conversion to
  `float`. No behavior change is established.
- **Dependencies/compatibility (FACT):** no dependency; patch applies cleanly.
- **State:** `LIKELY_ALREADY_PRESENT` (semantic equivalence; no backport value).
- **Later validation:** none unless a compiler portability failure is reproduced;
  then compile `game`, link worldserver, and run movement/chase regressions.

### 16 — Playerbots connection defaults

- **Affected files (FACT):**
  `modules/mod_playerbots/config/playerbots.conf.dist` and
  `src/server/worldserver/worldserver.conf.dist` (five additions, one deletion).
- **Technical description (FACT):** changes the module template credentials to
  `root;root`, adds `PlayerbotsDatabaseInfo`, worker/synchronous thread counts,
  and logger to the worldserver template, naming database `acore_playerbots`.
- **Current-tree comparison (FACT — C):** the Playerbots template already owns
  the connection/thread keys; README and repository instructions document
  manual worldserver settings using `mop_playerbots`, but the worldserver
  template lacks them. Patch applies cleanly.
- **Dependencies (FACT):** external Playerbots DB/config. **UNKNOWN:** which
  configuration file has authoritative ownership at runtime.
- **Compatibility (FACT/INFERENCE):** `acore_playerbots` conflicts with this
  repository's documented `mop_playerbots`; committing root credentials as
  defaults is unsuitable and duplicates keys when both templates are deployed.
- **State:** `INCOMPATIBLE_WITH_CURRENT_TREE`.
- **Later validation:** a separate configuration-design task should trace config
  load/installation precedence, use non-secret placeholders, enable modules and
  Playerbots, validate startup logs and DB pool ownership, and test deployment
  without duplicate keys.

### 17 — schema-only world base

- **Affected files (FACT):** `sql/base/world.sql` (new, 4,874 lines).
- **Technical description (FACT):** adds table structure without world content;
  the subject translates approximately to “world structure without data
  inside.” It does not solve missing gameplay rows.
- **Current-tree comparison (FACT — D):** `sql/base/world.sql` is absent and the
  new-file patch passes `git apply --check`.
- **Dependencies (UNKNOWN):** generator/server version, source DB baseline,
  updater state, collation/version assumptions, license, and compatibility with
  current archived/dated migrations.
- **Compatibility (INFERENCE):** filename fits the base-schema location, but
  applying an unproven empty schema could establish the wrong updater baseline.
- **State:** `REQUIRES_DATABASE_PROVENANCE`.
- **Later validation:** provenance/license and schema diff first; compare updater
  expected tables/columns and MySQL 5.7/8.0 syntax; create/migrate disposable
  databases; validate updater/server logs and demonstrate content-loading plan.

### 18 — Gate cannon feedback/headbutt selection

- **Affected files (FACT):**
  `src/server/scripts/Pandaria/GateOfTheSettingSun/{boss_raigonn.cpp,boss_striker_gadok.cpp}`
  (20 additions, 11 deletions).
- **Technical description (FACT):** fixes impossible coordinate comparisons in
  the headbutt removal predicate, gates stun processing on Raigonn being in
  progress, and replaces a raw spell-visual packet with triggered spell 133711
  plus a delayed fall based on assumed projectile speed.
- **Current-tree comparison (FACT — C/D):** current Raigonn retains the impossible
  comparisons and lacks the encounter-state guard. Current Gadok has a simpler
  immediate cast/fall loop, whereas the candidate hunk is based on a prior
  delayed cannon implementation; the whole patch does not apply.
- **Dependencies (FACT):** at minimum Gate series `3eb3c38b` and `67a6a17d`;
  preceding `3809418d`, `db9fe142`, `4d4d9d60`, `1cb74544`, `3de7e80d`,
  `9f80faeb`, and `59d06e96` alter the same dungeon context.
- **Compatibility (UNKNOWN):** build-18414 data/evidence for spell 133711,
  effect target semantics, exact rectangle, projectile speed and delayed target
  lifetime.
- **State:** `READY_WITH_DEPENDENCIES`.
- **Later validation:** split Raigonn and cannon units after final Gate-series
  review; compile scripts/link worldserver; inspect Spell/DBC 133711 and target
  filtering; test evade/reset, vertical floors, cannon clicks, projectile/fall,
  despawn and full encounter with build 18414.

## Current-Tree Equivalence Checks

| Candidate | Result | Evidence |
|---:|---|---|
| 1 | D — absent | placeholders remain; patch check succeeds |
| 2 | D — absent | custom hooks/files/IDs absent; core conflicts |
| 3 | D — absent | Nightfall file absent; DBC/tool hunks conflict |
| 4 | D — absent | no `account_factions` implementation |
| 5 | D — absent | no reagent bank files/statements |
| 6 | C/D — base content only | current encounters exist; bundled fork behavior does not |
| 7 | D — absent | battle-pet series differs and patch conflicts |
| 8 | A — same patch | canonical `5b50aeb...` has the same two deletions |
| 9 | D — absent | patch applies to current files |
| 10 | D — absent | no trainer manager/schema |
| 11 | D — absent | fork APIs/integration absent |
| 12 | D — absent | no Tillers subsystem/schema |
| 13 | D — absent | no prerequisite Tillers/battle-pet state |
| 14 | D — absent | current ARC4 does not load/select providers |
| 15 | B — equivalent | `M_PI_4` and rounded float literal yield π/4 float |
| 16 | C — partially documented | module keys exist; world template additions/database name differ |
| 17 | D — absent | no world base schema |
| 18 | C/D — one defect visible, implementation divergent | Raigonn expression matches defect; cannon ancestry differs |

**FACT:** different SHAs were not treated as proof of novelty. **FACT:** only 8
was found patch-equivalent and 15 semantically equivalent. **UNKNOWN:** runtime
equivalence cannot be established for the remaining gameplay candidates without
the validations above.

## Dependency Analysis

- **Nightfall family:** 3 requires 2 and `b179a560...`; both require unknown
  custom DB/spell/creature data.
- **Flamehawk battle-pet/content family:** 6 rests on earlier custom NPC-bot and
  pet-battle commits; 7 requires the explicit six-commit battle-pet chain listed
  in its review.
- **Andrew battle-pet family:** 10 requires pre-candidate weather/login/migration
  work; 11 requires 10 and two intervening commits. These are prerequisites, not
  endorsements.
- **Andrew Tillers family:** 12 requires its eight-commit development/refactor
  chain; 13 requires 12 plus two intervening mixed battle-pet commits and the
  earlier trainer lineage for its battle-pet portions.
- **Crypto family:** 14 assumes candidate 8 behavior, already canonical.
- **Gate family:** 18 is the tip of a long same-file dungeon series and cannot be
  interpreted as a standalone cannon implementation.
- **Database dependencies:** 4 and 5 omit required auth tables; 10, 12, and 13
  require ordered fork-local migrations; 16 requires a separately provisioned
  Playerbots DB; 17's baseline/provenance is unknown.
- **External libraries:** only 8/14 directly depend on OpenSSL version behavior.
  No candidate adds a new external library.

**Count:** 13 candidates have unresolved or required dependencies (2–7, 10–13,
16–18). Candidates 1, 8, 9, 14, and 15 have no identified prerequisite commit,
although gameplay evidence remains unresolved for 1 and 9.

## Compatibility Analysis

- **FACT:** clean conceptual/mechanical application was observed only for 1, 9,
  14, 15, 16, and 17; this is not correctness evidence.
- **FACT:** 2–7 and 10–13 were authored across changed APIs or missing new files;
  their raw patches are not useful backport boundaries.
- **FACT:** candidate 5 contains a loader/definition symbol mismatch. Candidate
  16 uses an AzerothCore-style DB name inconsistent with this tree. Candidate 18
  expects a prior cannon implementation absent here.
- **INFERENCE:** packet/opcode changes in 10 and battle-state changes in 7,
  10–11, and 13 are high-risk for client build 18414 and require trace/data
  evidence, not just compilation.
- **INFERENCE:** Playerbots changes in 11/16 require a separate module-enabled
  build and database/config validation; the normal fast profile is insufficient.
- **INFERENCE:** SQL in 4–5, 10, 12–13, and 17 cannot be assessed without exact
  schema provenance and migration ordering.
- **UNKNOWN:** none of the gameplay claims was accompanied here by authoritative
  retail captures or reproducible runtime evidence.

## Overlapping Candidate Families

1. **Custom-server/Nightfall:** 2 and 3 are sequential and share
   `nightfall_scripts.cpp`; 4 and 5 are separate account-wide custom features
   touching the same LoginDatabase statement list. These should not be merged as
   one unit merely because they share a fork era.
2. **Pandaria encounter safety/content:** 6 and 9 both edit Greenstone Village;
   9 and 18 both edit Raigonn. Candidate 6 is a content rewrite, 9 a mixed guard
   set, and 18 a later Gate behavior series—alternate context, not duplicates.
3. **Battle pets:** 7 is the Flamehawk implementation line; 10–11 are a separate
   trainer/weather/world line; 13 adds further effects on top of the latter.
   They modify the same battle functions and are alternate/overlapping
   implementations requiring range comparison before combination.
4. **Tillers:** 12 and 13 are one sequential subsystem. Candidate 13 supersedes
   portions of 12 but also includes unrelated world data.
5. **Cryptography:** 8 and 14 are sequential-compatible ARC4 work. Candidate 8
   is already canonical; 14 is the remaining isolated OpenSSL 3 proposal.
6. **Database/configuration:** 4, 5, 10, 12, 13, 16, and 17 require database
   provenance, but affect different databases/tables and are not one backport.

## Technical Review States

| State | Candidates | Evidence/next step |
|---|---|---|
| `READY_FOR_ISOLATED_BACKPORT` | 1, 14 | isolated, current structure matches, no missing source prerequisite; still needs functional validation |
| `READY_WITH_DEPENDENCIES` | 3, 7, 11, 12, 13, 18 | exact earlier series/schema/context identified |
| `NEEDS_CONTEXT` | 2, 5, 6, 10 | mixed, incomplete, abandoned, or internally broken bundles |
| `LIKELY_ALREADY_PRESENT` | 8, 15 | exact patch content (8) or semantic constant equivalence (15) |
| `LIKELY_DUPLICATE` | none | no additional candidate is demonstrably duplicate |
| `REQUIRES_DATABASE_PROVENANCE` | 4, 17 | missing table contract or unknown base-schema origin/baseline |
| `REQUIRES_RUNTIME_EVIDENCE` | 9 | compatible patch mixes added and removed guards across unrelated encounters |
| `INCOMPATIBLE_WITH_CURRENT_TREE` | 16 | wrong documented DB name, credential defaults, duplicated config ownership |
| `NONFUNCTIONAL_OR_METADATA_ONLY` | none of the 18 | Task 007's separate SQL-deletion example remains excluded |

Two candidates (1 and 14) are technically suitable as later isolated backport
reviews. “Ready” means bounded enough to implement and test, not correct.

## Validation Requirements

The per-candidate reviews specify exact checks. At the family level:

- **Core/C++:** targeted owning-library compile followed by `worldserver` link;
  crypto also requires `authserver` and the supported OpenSSL matrix.
- **Scripts/quests/encounters:** `scripts` compile, worldserver final link,
  script-name/SQL binding review, then build-18414 gameplay and reset/edge-case
  tests.
- **SQL/schema:** provenance and ordered schema diff, disposable database create
  and migration, then updater/server-log validation. Never infer data correctness
  from successful import.
- **Battle pets/packets:** DBC and packet-structure review, deterministic battle
  tests, persistence/quest-credit tests, and actual build-18414 client traces.
- **Playerbots:** separate `USE_MODULES=1`, `PLAYERBOTS=1` development build,
  external DB/config compatibility and later bot runtime/regression testing.
- **Extractor/DBC:** build the relevant tool, verify 18414 layouts and compare
  extracted output; do not use generated client data as repository content.

## Proposed Backport Units

These are future review boundaries; none was performed.

| Unit | Included candidates/commits | Subsystem and prerequisites | Expected files/build | SQL impact/runtime validation |
|---|---|---|---|---|
| U1 Teleporter data | 1 / `5b338780...` | custom script; registered NPC/world data must be confirmed | `emo_teleporter.cpp`; `scripts`, `worldserver` | none in patch; visit all destinations/menu paths |
| U2 ARC4 provider handling | 14 / `72e7fb1d...` (8 already present) | common crypto; OpenSSL 3 legacy provider | `ARC4.cpp`; `common`, `authserver`, `worldserver`, multi-OpenSSL builds | none; auth handshake/provider-unavailable tests |
| U3 Null-safety micro-units | selected hunks of 9, one function/encounter per task | only reproduced/proven hunks; never the aggregate commit | one source each; `game` or `scripts`, then worldserver | none; crash reproduction and encounter regression |
| U4 Account reputation experiment | 4 / `6e35544f...` | first obtain schema and requirements | LoginDatabase + ReputationMgr; `game`/worldserver | new auth table; multi-account/character semantics/security |
| U5 Nightfall script series | 2–3 plus `b179a560...`, after separating core rules | custom content, DBC, extractor in separate subunits | listed custom/core/tool files; scripts/game/tool | unknown custom data; quest/spell/DBC runtime tests |
| U6 Flamehawk pet-battle final state | 7 plus `26e16ed4`, `0c279a48`, 6's relevant pet hunk, `aec3e154`, `61e1f477`, `0f46d20c` | battle-pet core | four battle-pet files; game/worldserver | none identified; packet/DBC/round tests |
| U7 Andrew trainer/weather series | 10–11 plus `c440e10f`, `2da8a377`, `77711e7a`, `c6400a29` | battle pets, maps, optional Playerbots; split trainer, weather, grid, bots | game + module files; worldserver and module-enabled builds | trainer world SQL; DB, packet, quest and bot runtime tests |
| U8 Tillers final state | 12–13 plus `897bbf8e`…`6115dcae`, `867b69b0`, `aeef0c9d` | split schema/core scripts/SAI/companions from unrelated SQL and battle pets | Tillers, DB statements, loader, Object/Achievement as justified; scripts/game/worldserver | character/world migrations; full chain/phasing/crop persistence |
| U9 World schema provenance | 17 / `b134ab15...` | database-only investigation before adoption | `sql/base/world.sql`; no C++ target | disposable MySQL 5.7/8.0 create/updater test; provenance/license |
| U10 Gate micro-series | Raigonn portion of 18; cannon portion only with `3eb3c38b`, `67a6a17d` and relevant Gate ancestry | Gate scripts/spell data | one Gate file per subunit; scripts/worldserver | none in tip; encounter, rectangle, projectile/client tests |

Candidates 5, 6, 15, and 16 deliberately have no proposed direct backport unit:
5 is abandoned and internally incomplete; 6 requires prior decomposition before
a unit can be named; 15 is equivalent; 16 conflicts with current configuration
conventions.

## Suggested First Implementation Units

1. **U2 — ARC4 provider handling:** smallest functional diff, no DB/gameplay
   coupling, clear supported-library matrix, and candidate 8 prerequisite is
   already present. **UNKNOWN:** whether explicit provider loading is needed in
   every supported deployment; the future task must prove this.
2. **U1 — teleporter data:** one data table in one existing custom script and a
   clear compile/runtime path. Coordinate provenance remains **UNKNOWN**, so it
   should follow or require independent coordinate verification.
3. **U3 — one proven null-safety hunk:** only after selecting a hunk that adds a
   guard (not one that removes it) and obtaining a reproduction/static lifecycle
   proof. Candidate 9 as a whole is not a first safe unit.
4. **U10a — Raigonn predicate/state guard:** bounded and visibly repairs an
   impossible predicate, but only after confirming `remove_if` semantics,
   rectangle coordinates, and encounter lifecycle. The cannon change is not a
   first unit because it depends on the prior Gate series.

No database, packet, Tillers, Playerbots, or broad mixed bundle should be the
first implementation task.

## Unresolved Questions

1. What authoritative retail/build-18414 evidence supports candidates 1, 7,
   9–13, 15, and 18 (coordinates, spells, packets, encounter behavior, movement
   and farm/battle rules)?
2. Where are the schemas and migrations for `account_factions`, `reagent_bank`,
   Nightfall IDs, Flamehawk custom bot/donation data, and the precise trainer and
   Tillers baselines?
3. Which final commits in each active fork supersede candidate snapshots, and
   should final-state range diffs replace isolated commit review for 3, 7,
   10–13, and 18?
4. Why does candidate 4 read column `guid` but write an account ID into `id`, and
   what factions/flags are intentionally account-wide?
5. Was candidate 5 removed because of correctness, security, build, or product
   concerns, and does any table/world binding survive outside Git?
6. Which candidate 6 hunks are genuine encounter fixes versus custom-server NPC
   bot, donation, economy, and random-stat behavior?
7. For candidate 9, which removed guards are backed by constructor invariants,
   and can every added guard's crash path be reproduced?
8. What are the exact opcode/packet captures and DBC effect mappings for the
   alternate battle-pet implementations (7 versus 10–11/13)?
9. Who generated candidate 17, from which licensed database/version, and which
   updater marker/baseline does it represent?
10. For candidate 18, is spell 133711 correct for build 18414, and what evidence
    supports its target, timing, coordinate rectangle, and vertical-floor
    behavior?
11. Should Playerbots connection settings be installed from the module template
    or exposed in the worldserver template, and what non-secret database name is
    canonical?
