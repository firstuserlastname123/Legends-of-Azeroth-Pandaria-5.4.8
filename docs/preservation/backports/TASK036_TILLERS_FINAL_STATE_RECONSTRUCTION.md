# Task 036: Tillers Final-State Reconstruction

## Starting State

- **FACT:** Archaeology began on clean branch `work` at `63daba45b5d384b732e894ec23f542f7404bef5d`, the Task 035 merge and the locally available latest `preservation/main` integration state.
- **FACT:** `git status --short` was empty. No source named `TillersFarmMgr`, `TillersSoilScript`, `TillersZoneHooks`, `TillersWorldHooks`, or `cs_tillers` existed in preservation. Existing Tillers, Sunsong, Halfhill, and Farmer Yoon matches were historical/base world data, text, and prior preservation archaeology notes—not Andrew's farm implementation.
- **FACT:** This task changed only this document. It did not import commits, edit C++/SQL, run migrations, or compile.

## Repository / Branch

- **FACT:** A disposable clone at `/tmp/andrew-loa-task036` was made from `https://github.com/andrew-downey/Legends-of-Azeroth-Pandaria-5.4.8`; no preservation remote was changed.
- **FACT:** GitHub origin HEAD/default branch was `master`; inspected master tip was `f1abdd1a2754f4a39f9e5d2cd5112e63899a86c4` (`Refactor notable additions and clean up README`). Origin also exposed `feature/pandaria-fixes` at `bd8ad5515418d94abd35ba7bf71430979243a09c`.
- **FACT:** Candidate 13 is an ancestor of master. Later master commits through `f1abdd1a` change tests/README only; `155f6444` touches `tests/scripts/test_tillers.cpp` only to add includes/forward declarations and does not change runtime Tillers source. Therefore `f1abdd1a` is the final inspected repository tip and `bd8ad551` is the latest runtime Tillers revision.

## Complete Commit Timeline

The table includes every commit found by message searches (`Tiller`, `farm`, `Halfhill`, `Sunsong`), path-limited histories, and inspection of all changes between foundation and tip. “Survives” means runtime/data effect remains at the final inspected tip, not that it is correct.

| Date | Commit / parent | Subject | Tillers files/effect | Categories | Supersession / final presence |
|---|---|---|---|---|---|
| 2026-05-25 | `897bbf8e066eece1ce71e17910260f56e04e4828` / `987cdb972e1fc5ce3580fd8b6d8c545c4df8fd10` | Beginning Tillers work | `TILLERS.md`; first character SQL; `TillersFarm/tillers_farm.{h,cpp}`; CMake/loader | FOUNDATION, SCHEMA, FARM_STATE | Entire implementation and migrations deleted by `dc22704b`; does not survive. |
| 2026-05-25 | `6d1c8d310b42689c7e649653266f404752489e1f` / `897bbf8e…` | Update TILLERS.md with Phase 1/2 progress | Documentation only | FOUNDATION | Deleted by reset; no runtime effect. |
| 2026-05-25 | `319ab1fa5808c04beaed4292964d2a769ceb0909` / `6d1c8d31…` | Fix compilation errors in tillers_farm.cpp | First-generation manager compile repairs | FOUNDATION | Deleted by reset. |
| 2026-05-26 | `a008e984fd10df96a844a0ef03b917c4380f3fe9` / `319ab1fa…` | Playerbot fixes … tillers upgrades | First-generation farm, Yoon, shrine, friendship, voting, daily scripts; three character and five world migrations; Playerbots | FARM_STATE, CROP_GROWTH, DAILIES, FRIENDSHIP, TUTORIAL_QUEST, WORLD_DATA, PLAYERBOTS | Tillers implementation deleted by reset; Playerbots changes are mixed and unrelated to final normal-player farm. |
| 2026-05-26 | `1bb75c2864525ddab72ef68f082b9cdbe7f0a0e1` / `a008e984…` | Phase 4B refactor: Replace custom friendship/daily systems… | Deletes custom friendship/daily tables and `tillers_dailies.cpp`; adds refactor SQL; uses standard reputation/quest APIs | REPUTATION, FRIENDSHIP, DAILIES, SCHEMA | First-generation files deleted by reset. Design direction survives: final code uses reputation and standard daily quests, not old custom tables. |
| 2026-05-28 | `dc22704bb60e00811a103250951af70c5f708d6f` / `1bb75c28…` | Reset, new approach | Deletes all first-generation Tillers C++ and SQL; adds research | FOUNDATION, SCHEMA, WORLD_DATA | Explicit supersession boundary. Survives as deletion of old design. |
| 2026-05-31 | `b4a0c20982f1c2d86ad61e20d1b74ff3abaf1991` / `dc22704b…` | Cleanup | Introduces final-line manager/header, soil, workstation, ZoneHooks, command, character tables, crop templates, DB statements; mixed Playerbots/core gossip/player edits | FOUNDATION, SCHEMA, FARM_STATE, SOIL, PLANTING, CROP_GROWTH, CROP_PROBLEMS, HARVEST, REWARDS, ADMIN_COMMANDS, DATABASE_GLUE, PLAYERBOTS, UNRELATED_MIXED_CONTENT | Core of final implementation survives after later edits. |
| 2026-05-31 | `41e67ba1fafd4f43ac13f529c7fa1577858b3802` / `b4a0c209…` | battlepay cost migration | Battlepay SQL and Tillers documentation only | UNRELATED_MIXED_CONTENT | No Tillers runtime effect; exclude. |
| 2026-06-01 | `6115dcae543887b54636a4fc03f1bb3fd8de93e5` / `41e67ba1…` | …Tillers farm phasing…Playerbots… | Farm cleanup, quest gating, phase fix; manager/soil/hooks/DB edits; battle-pet and Playerbots hunks | PHASING, WORLD_DATA, TUTORIAL_QUEST, CORE_OBJECT-adjacent, PLAYERBOTS, UNRELATED_MIXED_CONTENT | Tillers portions survive or are refined by Candidate 12; mixed content excluded. |
| 2026-06-03 | `1e138b1f1d7e1660eb28b647b7193afe961acb72` / `6115dcae…` | Tillers content and fixes… | State bitmask/schema; tutorial SAI; obstacle/public phasing; dailies; Object visibility-layer API; manager/soil/hooks/commands | SCHEMA, WORLD_DATA, TUTORIAL_QUEST, PHASING, FARM_STATE, SOIL, ADMIN_COMMANDS, CORE_OBJECT_HOOK, DAILIES | Most survives; some SQL is cumulative/intermediate. Added visibility-layer field survives but is never consulted. |
| 2026-06-11 | `867b69b0d142efaaea10b83970342900d3405f2e` / `1e138b1f…` | Battle pet fixes… improved tillers intro quests | Companion columns/rename, daily/weed/Gina refinements, soil ScriptName, manager/header and DB edits; extensive battle pets/wild-pet SQL | TUTORIAL_QUEST, SCHEMA, COMPANIONS, SOIL, WORLD_DATA, BATTLE_PET/UNRELATED_MIXED_CONTENT | Tillers portions survive; battle-pet portions excluded. |
| 2026-06-21 | `bd8ad5515418d94abd35ba7bf71430979243a09c` / `aeef0c9d8e6484278ea98d07dde4cc608b077ff4` | Tillers farm: Phase 1 + Phase 2.2 planting→harvest retail cycle | Removes growth-tick column; adds companions SQL and WorldHooks; revises manager/header/registration/statements; mixed battle-pet, achievement, wild-pet and unrelated world SQL | SCHEMA, PLANTING, CROP_GROWTH, CROP_PROBLEMS, HARVEST, REWARDS, REPUTATION, COMPANIONS, WORLDSTATE, DATABASE_GLUE, UNRELATED_MIXED_CONTENT | Latest runtime Tillers state; survives at tip. Achievement hunk is battle-pet-only. |
| 2026-06-21 | `35f0c35938462d64399fe2e547419436a471800d` / `bd8ad551…` | README notable additions | README claims only | FOUNDATION/documentation | No runtime effect. |
| 2026-06-22 | `155f6444fe435529d38e46ee21ccc05d605e6031` / `7de670ec…` | Fix test suite compilation… | Include/forward-declaration edits in `tests/scripts/test_tillers.cpp` | FOUNDATION/test-only | Test source survives; runtime source unchanged. |

## Final Source File Set

- **FACT:** Runtime files: `src/server/scripts/Pandaria/TillersFarmMgr.{h,cpp}`, `TillersSoilScript.cpp`, `TillersWorkstation.cpp`, `TillersZoneHooks.cpp`, `TillersWorldHooks.cpp`, and `src/server/scripts/Commands/cs_tillers.cpp`.
- **FACT:** Supporting files: `src/server/database/Database/Implementation/CharacterDatabase.{h,cpp}`, `src/server/game/Entities/Object/Object.{h,cpp}`, Pandaria `CMakeLists.txt`, `ScriptLoader.cpp`, character/world migrations, and `tests/scripts/test_tillers.cpp`.
- **FACT:** The initial `Pandaria/TillersFarm/` directory and its `tillers_farm`, Yoon, shrine, friendship, vote, and daily scripts were deleted by `dc22704b`; they were superseded, not renamed in Git.
- **FACT:** `TillersZoneHooks.cpp` and `TillersWorldHooks.cpp` coexist. ZoneHooks owns player/quest lifecycle; WorldHooks owns reset-time bulk growth. Neither replaced the other.

## Manager Architecture

- **FACT:** `TillersFarmMgr` is a function-local-static singleton (`sTillersFarmMgr`). Global maps key by low player GUID: `PlayerFarmState`, `PlotMap`, and GUID vectors for dynamic soil, Yoon, obstacles, and companions. Sixty-four striped mutexes exist, but returned references outlive locks.
- **FACT:** Persistent farm state is `farmState`, redundant `plotsUnlocked`, and `bestFriendUnlocks`; each of 16 plots stores state, seed entry, water/pest booleans, and absolute maturity timestamp. Spawn GUID vectors and cached positions are transient.
- **FACT:** Farm states `14/12/8/0` encode weeds/wagon/boulder and imply `4/8/12/16` plots. Plot states are empty, prepared, seeded, growing, needs water, needs pest control, ready, and broken.
- **FACT:** Load uses synchronous raw SELECTs; save transactionally deletes/reinserts all plots and REPLACEs state. Despawn saves, erases memory, despawns objects, and clears custom phase. Reset only changes memory and tracked GUID maps; it neither deletes/saves DB rows nor despawns the actual objects.
- **Lifecycle (FACT):** zone entry after quest 30252 → lazy cache initialization → DB load/default construction → reputation/quest companion reconciliation → per-player phase → dynamic Yoon/obstacles, and after 30256 soil → tutorial/quest rewards alter farm state → soil spell tills/plants → random planting state and reset timestamp → water/pest action → daily WorldHook or re-entry timestamp check makes eligible crops ready → harvest grants items/reputation before plot reset → zone/map exit or logout saves and clears → relog/entry reloads.
- **UNKNOWN:** No runtime evidence establishes that this lifecycle matches build 18414.

## Character Schema History

| Migration | Introduced | Operation / dependency | Final status |
|---|---|---|---|
| `2026_05_24_00_characters_tillers_farm.sql` | `897bbf8e` | First-generation CREATE; deleted by reset | SUPERSEDED/absent. |
| `2026_05_25_00_characters_tillers_farm.sql` | `a008e984` | Replacement first-generation farm schema; deleted by reset | SUPERSEDED/absent. |
| `2026_05_25_02_characters_tillers_friendship.sql` | `a008e984` | Custom friendship table; deleted by `1bb75c28` | SUPERSEDED. |
| `2026_05_25_04_characters_tillers_daily_quests.sql` | `a008e984` | Custom daily table; deleted by `1bb75c28` | SUPERSEDED. |
| `2026_05_26_02_characters_tillers_refactor.sql` | `1bb75c28` | Removes/refactors old custom state; deleted by reset | INTERMEDIATE_ONLY. |
| `2026_05_28_00_tillers_farm_plots.sql` | `b4a0c209` | Destructive `DROP TABLE IF EXISTS`, then CREATE plots | Required base, but destructive. |
| `2026_05_28_01_tillers_farm_state.sql` | `b4a0c209` | Destructive drop/create state with `last_growth_tick` | Required base, later altered. |
| `2026_06_02_01_tillers_farm_state_bitmask.sql` | `1e138b1f` | Data transformation to states 14/12/8/0 | Required only for upgrading intermediate data; no-op conceptually on fresh defaults. |
| `2026_06_02_03_tillers_farm_state_schema.sql` | `1e138b1f` | Alters phase default/comment to 14 | Required after state CREATE. |
| `2026_06_03_07_tillers_companion_bitmask.sql` | `867b69b0` | Adds `companions SMALLINT UNSIGNED NOT NULL DEFAULT 0` | Required intermediate. |
| `2026_06_03_08_tillers_best_friend_unlocks_rename.sql` | `867b69b0` | Renames `companions`; backfills every character using `last_growth_tick` | Required after companion add; backfill is broad. |
| `2026_06_21_01_tillers_remove_last_growth_tick.sql` | `bd8ad551` | Drops obsolete tick | Required final alteration. |

## Final Character Schema

- **FACT:** `player_farm_state`: `guid INT UNSIGNED NOT NULL DEFAULT 0 PRIMARY KEY`; `farm_phase TINYINT UNSIGNED NOT NULL DEFAULT 14`; `plots_unlocked TINYINT UNSIGNED NOT NULL DEFAULT 4`; `best_friend_unlocks SMALLINT UNSIGNED NOT NULL DEFAULT 0`. No secondary indexes.
- **FACT:** `player_farm_plots`: `guid INT UNSIGNED NOT NULL DEFAULT 0`; `plot_id TINYINT UNSIGNED NOT NULL DEFAULT 0`; `state TINYINT UNSIGNED NOT NULL DEFAULT 0`; nullable `seed_entry INT UNSIGNED`; `needs_watering TINYINT(1) NOT NULL DEFAULT 0`; `has_pests TINYINT(1) NOT NULL DEFAULT 0`; nullable `maturity_timestamp INT UNSIGNED`; composite primary key `(guid, plot_id)`.
- **FACT:** `last_growth_tick`, old custom friendship, and custom daily state are obsolete. Absolute per-plot maturity plus the server daily reset replaces the farm-level tick.
- **INFERENCE:** A future clean migration should create this final schema directly rather than replay destructive intermediate drops, but Task 036 does not prescribe or implement that migration.

## Prepared Statements

| Enum | SQL purpose / parameter order | Caller / finding |
|---|---|---|
| `CHAR_DEL_PLAYER_FARM_PLOTS` | DELETE by `guid` | Full save; symmetric. |
| `CHAR_INS_PLAYER_FARM_STATE` | INSERT `(guid,farm_phase,plots_unlocked,last_growth_tick,best_friend_unlocks)` | **DEAD_STATEMENT + COLUMN_ORDER_MISMATCH:** retained after `last_growth_tick` was dropped and has no caller. |
| `CHAR_INS_PLAYER_FARM_PLOT` | INSERT seven final plot columns, values `guid,plot,state,seed,water,pests,maturity` | Full save; order/type broadly symmetric. |
| `CHAR_UPD_PLAYER_FARM_STATE` | REPLACE columns `(guid,farm_phase,plots_unlocked,best_friend_unlocks)` | **COLUMN_ORDER_MISMATCH:** caller binds `(farmState, plotsUnlocked, unlocks, guid)`, so GUID receives farm state and every value shifts. |
| `CHAR_UPD_PLAYER_FARM_PLOT_PLANT` | UPDATE state/seed/water/pests/maturity WHERE guid/plot | Plant; symmetric. |
| `CHAR_UPD_PLAYER_FARM_PLOT_RESET` | Same order as plant | Harvest; symmetric. |
| `CHAR_UPD_PLAYER_FARM_PLOT_WATER` | water,guid,plot | Water; symmetric. |
| `CHAR_UPD_PLAYER_FARM_PLOT_PESTS` | pests,guid,plot | Pest removal; symmetric. |
| `CHAR_UPD_PLAYER_FARM_PLOT_REPAIR` | state,seed,guid,plot | Repair; symmetric. |
| `CHAR_UPD_PLAYER_FARM_PLOT_STATE` | state,guid,plot | Water/pest transition; symmetric. |

- **FACT:** Loads are raw `PQuery`, not prepared statements: state result order is phase/plots/unlocks; plot order is plot/state/seed/water/pests/maturity and matches readers.
- **FACT:** `ResetPlayerFarm` has **MISSING_SAVE**; tilling mutates only memory and relies on later full save; companion update and force-grow use raw SQL. There is no final prepared load statement.

## World Migration Inventory

| Migration | Tables / function | Classification |
|---|---|---|
| `2026_05_29_00_tillers_crop_creatures.sql` | delete/reinsert `creature_template` custom crop entries | TILLERS_REQUIRED for the final crop/template assumptions; destructive/assumes schema. |
| `2026_05_31_03_tillers_farm_cleanup.sql` | `creature`, `creature_template`; hides references/debug entities, restores Yoon | TILLERS_REQUIRED foundation. |
| `2026_05_31_04_tillers_quest_gating.sql` | `quest_template_addon`; tutorial, farm, votes, final quest prerequisites | TILLERS_REQUIRED, QUEST_CHAIN_RISK. |
| `2026_06_01_00_tillers_farm_phase_fix.sql` | Yoon creature masks | SUPERSEDED in part by public-phase migration; INTERMEDIATE_ONLY when replaying chain. |
| `2026_06_01_01_tillers_quest_30256_harvest_sai.sql` | templates, spellclick, conditions, SAI | TILLERS_REQUIRED tutorial. |
| `2026_06_01_02_tillers_rock_spawn_sai.sql` | Yoon SAI IDs 7–14 | TILLERS_REQUIRED tutorial. |
| `2026_06_01_03_tillers_shrine_phase.sql` | shrine/bowl GO phase masks | TILLERS_OPTIONAL_CONTENT; not planting core. |
| `2026_06_02_02_tillers_obstacle_phasing.sql` | obstacle GOs/reference and ranch creatures | TILLERS_REQUIRED phasing; later public-phase overlaps it. |
| `2026_06_02_04_tillers_public_farm_phase.sql` | creatures, obstacles, `phase_definitions`, queststarter | TILLERS_REQUIRED phasing/tutorial; supersedes conflicting Yoon visibility assumptions. |
| `2026_06_02_05_tillers_rock_interaction_fix.sql` | templates/spellclick/conditions/SAI/creatures; quests 30252/30254/30256 | TILLERS_REQUIRED tutorial and replacement SAI. |
| `2026_06_03_00_tillers_quest_30257_dark_soil.sql` | creature cleanup, Yoon SAI, Dark Soil GO | TILLERS_REQUIRED tutorial. |
| `2026_06_03_01_tillers_cabbage_duplicate_fix.sql` | conditions/SAI coordinates | TILLERS_REQUIRED follow-up to `06_02_05`. |
| `2026_06_03_02_smart_script_fixes.sql` | unrelated Outland/Northrend entries and quests | UNRELATED; exclude despite neighboring sequence. |
| `2026_06_03_03_tillers_marsh_lily_fix.sql` | item and GO loot | TILLERS_REQUIRED for 30257. |
| `2026_06_03_04_tillers_daily_gate.sql` | quest-availability conditions for 15 dailies | TILLERS_OPTIONAL_CONTENT (required for claimed daily unlock, not planting cycle). |
| `2026_06_03_05_tillers_weed_expansion.sql` | fixed-GUID GO inserts 4000077–4000084 | TILLERS_OPTIONAL_CONTENT; GUID_COLLISION_RISK. |
| `2026_06_03_06_tillers_gina_spawn.sql` | SAI deletion, Gina spawn phase | TILLERS_OPTIONAL_CONTENT/tutorial market continuity. |
| `2026_06_04_00_tillers_soil_scriptname.sql` | GO template ScriptName | TILLERS_REQUIRED for final dynamic soil callback. |
| `2026_06_20_01_tillers_companions.sql` | companion templates/models/addons and fixed GUID spawns 516068–516077 | TILLERS_OPTIONAL_CONTENT; companions are not planting/harvest dependencies. |

## Migration Dependency Graph

**MINIMUM_FINAL_SCHEMA_CHAIN (literal historical chain):**

```text
05_28_00 plots CREATE -------------------------------> final plots
05_28_01 state CREATE
  -> 06_02_01 bitmask data conversion
  -> 06_02_03 defaults/comments
  -> 06_03_07 companions column
  -> 06_03_08 rename + broad backfill
  -> 06_21_01 DROP last_growth_tick ----------------> final state
```

**MINIMUM_WORLD_CONTENT_CHAIN:**

```text
05_29 crop templates
05_31_03 cleanup -> 06_01_00 phase correction -> 06_02_02 obstacle masks
                                           \-> 06_02_04 final public/private layout
05_31_04 quest prerequisites
06_01_02 rock spawns -> 06_01_01 spellclick base -> 06_02_05 replacement interaction
06_02_05 tutorial soil/cabbages -> 06_03_01 duplicate/coordinate correction
06_02_05 -> 06_03_00 Dark Soil -> 06_03_03 Marsh Lily correctness
06_04_00 soil ScriptName <-> compiled/registered TillersSoilScript
```

**OPTIONAL_CONTENT_CHAIN:** `06_01_03` shrine; `06_03_04` daily gating after tutorial prerequisites; `06_03_05` after obstacle/public phase setup; `06_03_06` after the prior Yoon/Gina SAI; `06_20_01` after final companion column and manager support. `06_03_02` is excluded.

## Phasing / Layering

- **FACT:** Public entrants receive mask 129 (`1|128`) through zone definitions for 1023, 5805, and 5840. Public farm objects use bit 128. After quest 30252, manager computes `(guidLow << 8) | 1`, calls `PhaseMgr::SetCustomPhase`, and creates player-owned/private Yoon, soil, obstacles, and companions with that mask; exit clears custom phase.
- **FACT:** Dynamic objects also use `SetPrivateObjectOwner(player GUID)`. Soil checks only mask overlap, while generic private-owner visibility supplies actual owner isolation.
- **FACT:** No dynamic phase IDs are used. A 32-bit left shift necessarily discards high GUID bits and masks may collide; every mask includes normal bit 1, so phase-mask intersection alone does not isolate players.
- **FACT:** Candidate 12 adds generic `WorldObject::m_visibilityLayer` accessors/initialization, but neither `InSamePhase` nor visibility code compares it and Tillers never sets it. Thus “layering” is inert; owner privacy is the operative mechanism.
- **FACT:** Static reference creatures are moved to `0xFFFF0000`; obstacles/public Yoon use 128; ordinary normal-world objects use bit 1. Farm players continue seeing normal world, including other players, subject to ordinary visibility. This is a **HIGH_RISK** global/core and phase-data design requiring runtime proof.

## Generic Core Changes

| Area | Change | Classification / necessity |
|---|---|---|
| `Object.{h,cpp}` | Adds initialized `uint32 m_visibilityLayer` plus getters/setter | GENERIC_BEHAVIOR/API_CHANGE, inert and not Tillers-required as written; omit unless a tested visibility consumer is designed. HIGH_RISK if activated globally. |
| Character DB | Adds farm statement enums/preparation | DATABASE_GLUE; required for manager writes, but contains defects. |
| Player | Earlier mixed edits around this line are not called by final farm manager | UNRELATED/needs hunk-level exclusion. |
| AchievementMgr | Candidate 13 implements pet-battle-against-tamer additional condition | UNRELATED battle-pet generic change; no Tillers achievement hook. |
| World | No core World edit; Tillers WorldScript polls public reset API | Script-only dependency. |

## Tutorial Quest Chain

**FACT dependency graph:**

```text
30252 A Helping Hand (Yoon 58646; 8 clickable 58719 rocks)
 -> 30535 Learn and Grow I: Seeds
 -> 30254 Learn and Grow II: Tilling and Planting
      (Yoon summons two 59985; click -> 59990; requires 80302; -> 59987)
 -> 30255 Learn and Grow III: Tending Crops
      (59987 quest-gated; duplicate set cleaned; water interaction inherited from SAI)
 -> 30256 Learn and Grow IV: Harvesting
      (two 59833; click gives 80314; completion enables manager soil)
 -> 30257 Learn and Grow V: Halfhill Market
      (Yoon summons GO 210565; loot 79269 Marsh Lily, unique and guaranteed quest drop)
 -> 31945 Learn and Grow VI: Gina's Vote
    +-> 30260 -> 30516 (weeds clear; 8 plots)
    +-> 30523 -> 30524 (wagon clear; 12 plots)
    +-> 30529 (boulder clear; 16 plots)
    +-> vote branches 30258->30259->31946;
        30517->30518->31947; 30519->31948->31949;
        30521->30522->{30525,30527}->30528
30526 Lost and Lonely is standalone (Lost Dog unlock).
30529 + faction 1272 standing 22000 -> 32682 Inherit the Earth.
```

- **FACT:** Daily quest conditions require rewarded 30257. The manager uses faction 1934 for harvest reputation, while final-quest SQL uses faction 1272; this inconsistency is a **POSSIBLE_DEFECT**.
- **UNKNOWN:** Quest rewards, all questgiver/end mappings, and retail sequencing were not independently validated against a build-18414 client.

## Soil / Plot Model

- **FACT:** Sixteen indexed plots (0–15) are persisted by character. Coordinates are derived at runtime by sorting world `creature` entry 55626 on map 870 with Z 164–166, not by an explicit plot-ID column. Only unlocked counts 4/8/12/16 are created.
- **FACT:** Soil is dynamic GO 186314 with `tillers_soil_patch`; plot ID is overloaded into GO `SpellId`. GO owner/private-owner and phase are assigned. No explicit distance/map/zone validation exists inside the soil callback beyond object validity, world state, phase overlap, and manager membership.
- **FACT:** Unlocks derive from rewarded quests via ZoneHooks and are persisted redundantly as mask/count. Occupancy/crop linkage is logical only (`seedEntry` and state); there is no spawned crop object per persistent plot and no visual-state change on the soil GO.

## Planting

- **FACT:** Dummy spell on owned soil validates phase, plot, unlock, and prepared state. The final script recognizes only spell/item IDs 79102, 110030, and 110031, checks inventory, invokes manager, updates state, and consumes one matching ID after DB commit.
- **FACT:** Manager's reward mapping instead recognizes seed items 79102, 80590, 80591, 80592, 80593, 80594, 80595, 89328, and 89329 mapping respectively to vegetables 74840, 74841, 74843, 74842, 74844, 74849, 74850, 74847, and 74848.
- **STATIC_DEFECT:** Script overwrites manager's randomly selected `READY`, water, or pest state with `SEEDED`, clears flags, and requests a new maturity time after `PlantSeed`; memory and committed DB diverge until a later save, which may overwrite the DB. IDs 110030/110031 have no vegetable mapping and fall back to returning the same ID; most mapped seeds cannot trigger the script. The comment calls 79102 wheat while mapping calls it Green Cabbage.
- **UNKNOWN:** These spell/item mappings and client-visible behavior are not proven build-18414 retail data.

## Growth Advancement

- **FACT:** Planting records the next standard daily quest reset as an absolute timestamp. WorldHooks polls `GetNextDailyQuestsResetTime`; whenever it increases (also once at startup), it bulk-updates only `SEEDED/GROWING` rows whose timestamp is due, then calls `SpawnPlayerFarm` for active farm players. Entry/rebuild independently converts due in-memory `SEEDED/GROWING` plots to ready.
- **FACT:** Offline characters advance through bulk SQL; relog loads state. Restart's first WorldHook pass is intended as catch-up. `.tillers grow` is a separate explicit path.
- **FACT:** `last_growth_tick` became unnecessary because each plot has an absolute maturity timestamp and global reset detection uses the world's next-reset value; Candidate 13 drops it.
- **STATIC_DEFECT:** `SpawnPlayerFarm` immediately returns for already spawned farms, so WorldHooks' “refresh” does not reload bulk-updated rows. Online farms can remain stale until despawn/reload. Problem states are excluded from both bulk maturity and local maturity transitions and can remain blocked until resolved.

## Crop Problems

- **FACT:** Planting independently rolls 1.1% instant “bursting”; otherwise 14% water, next 14% pests, otherwise normal. This is random at planting, not daily/deterministic. Water uses item 79104 and pest control item 80513; resolution clears the flag and moves the corresponding problem state to growing. Broken/repair exists with shovel 89880 but no generation path exists.
- **FACT:** Runty (186316), Wild (186317), “Smootherd” (186318), and Tangled (186319) callbacks only validate ready state and emit instructions. They neither set a persisted problem type nor resolve/change/reward it. They are stubs, not implemented crop problems.
- **UNKNOWN:** Percentages, IDs, tools, and visual/minigame semantics lack runtime evidence.

## Harvest / Rewards

- **FACT:** Harvest requires ready state. It maps seed to vegetable; gives 5, with 5% chance of 8; unknown mapping returns one seed-like item. Independently, 50% gives 1–3 planted seeds. Level 90 receives 50 reputation with faction 1934. Then a DB transaction resets plot, followed by memory reset. No quest credit or achievement criteria call exists.
- **STATIC_DEFECT:** Items and reputation are granted **before** plot reset commits and `AddItem` return values are ignored. A failure/crash/full inventory can lose or duplicate value; repeated interaction before durable state change is non-idempotent. There is no atomic transaction spanning character inventory/reputation and farm state.

## Reputation / Friendship

- **FACT:** The old custom friendship/daily tables and script were introduced in `a008e984`, replaced with standard reputation and quest APIs in `1bb75c28`, then deleted by reset. Final code checks `ReputationRank >= REP_EXALTED` for factions: Fung 1283, Haohan 1279, Hillpaw 1276, Chee Chee 1277, Ella 1275, Fish 1282, Sho 1278, Tina 1280, Gina 1281. Lost Dog checks rewarded quest 30526.
- **FACT:** A custom `best_friend_unlocks` bitmask remains only as a durable cache of unlocked farm decorations; ordinary friendship standing itself is not custom-persisted. Standard quest daily state controls daily completion.
- **POSSIBLE_DEFECT:** Treating “Best Friend” as generic Exalted and faction-ID inconsistencies require gameplay/data validation.

## Companions

- **FACT:** Ten bits represent Shaggy 85814, Fifi 85791, chickens 85820, sheep 85808, Luna 85818, pigs 85802, orange tree 237243, furniture 237244, mailbox 237242, and Lost Dog 85826. All newly satisfied bits accumulate; all unlocked companions/decorations spawn together as private farm objects.
- **FACT:** SQL supplies seven creature and three GO templates plus hidden/fixed spawn rows used as coordinate records. The manager reads these rows and dynamically summons private copies. Companions are optional phase content; planting/harvest does not call them.
- **UNKNOWN:** Models, coordinates, simultaneous visibility, and unlock semantics are not runtime-validated.

## Daily / Reset Behavior

- **FARM_GROWTH:** absolute next-reset timestamps; eligible seeded/growing rows mature globally, including offline characters.
- **DAILY_QUEST:** standard quest daily reset; SQL only gates offering behind 30257.
- **FRIENDSHIP:** standard persisted reputation; no daily custom reset.
- **COMPANION:** permanent accumulating bitmask; refreshed on farm spawn; no reset.
- **PLOT_UNLOCK:** permanent quest-derived mask/count; no reset.
- **FACT:** No persisted “last processed reset” exists. Startup always performs one idempotent conditional bulk update. Server restart should preserve timestamps, but active-player refresh is defective as noted.

## Achievement Integration

- **FACT:** Candidate 13's sole `AchievementMgr.cpp` hunk implements `ACHIEVEMENT_CRITERIA_ADDITIONAL_CONDITION_PET_BATTLE_AGAINST_TAMER` by examining the current pet-battle opponent. It has no Tillers IDs/event source.
- **FACT:** Final Tillers harvest never updates achievement criteria. Classification: **GENERIC_ACHIEVEMENT_FIX / EXCLUDED_FROM_TILLERS**, with global battle-pet risk; there is no isolated Tillers hook.

## Admin Commands

All are under `.tillers`, `SEC_ADMINISTRATOR`.

| Command | Console | Effect | Classification |
|---|---:|---|---|
| `status` | yes | Reads selected/self memory, displays farm and only plot IDs below 8 | USEFUL_ADMIN_RECOVERY, but incomplete display and may instantiate empty state. |
| `reset` | no | Resets memory/tracking only; claims farm reset | **UNSAFE/DEVELOPMENT_ONLY**: no DB write or actual despawn. |
| `setplots` | yes | Mutates mask/count or memory reset; calls Spawn | DEVELOPMENT_ONLY/UNSAFE: no reliable save and Spawn may early-return. |
| `grow` | no | Moves seeded/growing memory and DB rows to ready | DEVELOPMENT_ONLY; useful recovery only after hardening/audit. |

- **FACT:** No command is required at runtime. The command is declared/registered twice in ScriptLoader's startup paths, a **POSSIBLE_DEFECT** dependent on how those loaders are called.

## Script Registration

| Component | Compiled | Add function declared/called | SQL reference | Finding |
|---|---|---|---|---|
| FarmMgr | yes | no AddSC needed | tables only | Library substrate. |
| ZoneHooks | yes | `AddTillersZoneHooks` once | none | Consistent. |
| SoilScript | yes | `AddTillersSoilScript` once | `tillers_soil_patch` in `06_04_00` | Consistent only with migration. |
| Workstation | yes | `AddTillersWorkstation` once | Custom GO entries 400000/400001 are not supplied by Tillers SQL | **POSSIBLE_DEFECT:** compiled/registered but data absent. |
| WorldHooks | yes | `AddTillersWorldHooks` once | character table | Consistent. |
| Commands | command directory build mechanism | `AddTillersCommands` appears in two call sites | none | **POSSIBLE_DEFECT:** duplicate registration. |

## Playerbots Boundary

- **FACT:** Mixed hunks occur in `a008e984`, `b4a0c209`, and `6115dcae` under `modules/mod_playerbots` (manager/config/login/random bot behavior). Final Tillers runtime includes no Playerbots header, module call, bot farm logic, or bot quest logic.
- **Classification:** **FULLY_SEPARABLE** for ordinary-player Tillers. Exclude every Playerbots hunk.

## Battle-Pet Boundary

- **FACT:** Exclude `2026_05_22_00_battle_pet_trainer_spawns.sql`; pet supplies/wild-pet migrations; all `BattlePet*`, `PetBattle*`, weather, XP, trainer/tamer work; Merda positioning; and Candidate 13's `AchievementMgr` condition. These are **EXCLUDED_FROM_TILLERS**.

## Unrelated Content Boundary

- **FACT:** Candidate 13 migrations for Northrend wild pets, Taurast, general creature updates, Auchenai Crypts, flying fixes, two broad quest/creature correction files, undead start, and dungeon Pandaren reputation are **UNRELATED**. Candidate 12's `2026_06_03_02_smart_script_fixes.sql` and the preceding battlepay migration are also unrelated. None belongs in a future Tillers unit.

## Current Preservation Comparison

| Component | Status |
|---|---|
| Character schema/statements/manager/soil/ZoneHooks/WorldHooks/commands/Object layer field | ABSENT |
| Tutorial SQL, Andrew world migrations, private plot state, planting/growth/problems/harvest/reward, companions, daily gate, registrations | ABSENT |
| Reputation/friendship | PARTIALLY_PRESENT only in the sense that generic core reputation/quest APIs exist; Andrew integration absent |
| Achievements | ABSENT as Tillers integration (Andrew has none); Candidate 13 battle-pet change intentionally not Tillers |
| Existing base Halfhill/Sunsong/Yoon/world rows | PARTIALLY_PRESENT/UNKNOWN compatibility; Andrew migrations assume and mutate them |
- **FACT:** No Andrew Tillers preservation source had unexpectedly arrived before this task.

## Static Defects

| Severity | Finding |
|---|---|
| STATIC_DEFECT | Farm-state REPLACE SQL column order disagrees with bindings. |
| STATIC_DEFECT | Dead INSERT statement references dropped `last_growth_tick`. |
| STATIC_DEFECT | `LoadPlotPositions` first queries `creature_template` through **CharacterDatabase**, guaranteeing wrong-database behavior on normal split schemas before querying WorldDatabase. |
| STATIC_DEFECT | Soil script overwrites manager planting result/random problems and recomputes maturity; supported IDs and reward mapping disagree. |
| STATIC_DEFECT | Rewards/reputation precede durable reset; ignored item-add result and no cross-system transaction create loss/duplication hazards. |
| STATIC_DEFECT | WorldHook refresh calls a spawn function that returns early for active farms, leaving memory stale. |
| STATIC_DEFECT | Reset command changes no durable row and erases GUID tracking without despawning world objects. |
| POSSIBLE_DEFECT | Returned map/state references are used after striped locks release; several accesses are entirely unlocked. Singleton cache initialization boolean is unsynchronized. |
| POSSIBLE_DEFECT | GUID-derived 32-bit masks can collide; all contain bit 1; inert visibility layer does not isolate. Correctness rests on generic private-owner behavior. |
| POSSIBLE_DEFECT | Zone hook only accepts zone 1023 while SQL explicitly says farm micro-zones may resolve as 5805/5840. Farm may not spawn there. |
| POSSIBLE_DEFECT | Quest reward calls Spawn on an existing farm; early return can prevent obstacle rebuild and state persistence. |
| POSSIBLE_DEFECT | Only initially unlocked empty plots become prepared; newly unlocked plots can remain empty. |
| POSSIBLE_DEFECT | Plot IDs depend on coordinate sort and count is not capped while loading; no explicit stable world-data key. |
| POSSIBLE_DEFECT | Companion SQL uses fixed GUIDs and manager fallback co-locates missing entries; templates are synthetic copies. |
| POSSIBLE_DEFECT | Duplicate command registration; workstation lacks supplied world data; command status hides plots 8–15. |
| POSSIBLE_DEFECT | Faction 1934 harvest reputation versus 1272 quest requirement. |
| RETAIL_SEMANTICS_UNKNOWN | Quest order/rewards, phase masks, crop odds/timing/yields/reputation, all invented-looking spell/GO IDs, problem minigames, visuals, companions, and reset behavior. |

## Functional Units

| Unit | Source / SQL | Dependencies | 18414/runtime need | Independent/inert? | Risk |
|---|---|---|---|---|---|
| T1 final character schema + DB contract | final two tables; DB enum/SQL contract | none | disposable DB/static symmetry | yes if schema-only review; inert | medium (historical drops/backfill) |
| T2 manager persistence substrate | manager state/load/save only | T1 | compile + DB roundtrip | can be inert without hooks | medium/high; current defects |
| T3 personal phasing | manager spawning, ZoneHooks, world phase SQL; possibly private-owner API | T2, world rows | strong client/multiplayer evidence | no | very high |
| T4 tutorial | quest/SAI SQL | base world data; T3 for transition | quest gameplay | mostly independent but player-facing | high |
| T5 plot/soil | positions, soil GO/script/ScriptName | T2,T3,T4 completion | GO/spell client evidence | no | high |
| T6 planting | soil + manager writes/mappings | T5 | inventory/spell visuals | no | high |
| T7 growth | timestamps + WorldHooks | T1,T2,T6 | reset/offline/restart | hook can stage inert only after redesign | high |
| T8 problems | water/pest/repair/minigame | T6,T7 | spells/visuals | no | high/weak implementation |
| T9 harvest/rewards | manager/soil | T6,T7; optionally T8 | inventory/reputation/duplication tests | no | very high |
| T10 reputation/friendship | generic rep, SQL gates | T4/base factions | retail faction evidence | partially | medium/high |
| T11 companions | bitmask, manager spawn, `06_20_01` | T2,T3,T10 | models/visibility | optional and separable | high |
| T12 achievements | none exists for Tillers | evidence required | client/criteria | cannot stage from Andrew | unknown |
| T13 admin/recovery | `cs_tillers.cpp` | T2–T9 | command/runtime | optional; omit initially | high/unsafe |

## Unit Dependency DAG

```text
T1 schema/contract -> T2 inert persistence
T2 -> T3 personal phasing -> T5 persistent soil -> T6 planting -> T7 growth -> T9 harvest
                     \-> T4 tutorial -----------/                    \-> T8 problems -> T9
T4 -> T10 reputation/friendship -> T11 companions
T2 + relevant feature units -> T13 commands (optional, last)
T12 achievements: no Andrew Tillers implementation; evidence-only future branch
```

- **FIRST SAFE STAGING UNIT / NEXT_RECOMMENDED_UNIT:** **T1, a review-only reconstruction of the final character schema and corrected DB statement contract**, tested on a disposable database before implementation. It is smallest, inert, reversible with an explicit down plan, statically testable, and independent of unknown phase/client behavior. Task 037 should review/design it, not replay the destructive chain verbatim.

## SQL Risk

- Character base CREATE files: **DESTRUCTIVE**, not `SAFE_CREATE_ONLY`, because they drop tables. Bitmask conversion/rename/drop: **DESTRUCTIVE** and **ASSUMES_UPSTREAM_ROWS**. Backfill affects every character.
- Crop templates/tutorial SAI: **DESTRUCTIVE/ASSUMES_UPSTREAM_ROWS/QUEST_CHAIN_RISK** due to DELETE/REPLACE and hard-coded SAI IDs.
- Public farm/obstacle/shrine/Gina: **PHASE_DATA_RISK/ASSUMES_UPSTREAM_ROWS**.
- Weed and companion fixed spawns: **GUID_COLLISION_RISK**; `INSERT IGNORE` can silently retain a conflicting unrelated row.
- Daily conditions and some spellclick inserts are partly idempotent, but key compatibility and preexisting ElseGroups remain **UNKNOWN**.
- Every future SQL unit requires disposable database application, schema inspection, row-count assertions, restart, and rollback testing.

## Build-18414 Dependencies

- **Requires client/runtime evidence:** personal/cross-player visibility; public-to-private phase transition; Yoon/obstacle/companion visibility; quest offering/order/credit/reward; soil click and dummy-effect dispatch; stable plot positions; crop/GO visual states; seed spells/items; water/pest/minigames; harvest interaction and inventory-full behavior; reputation faction/amount; criteria packets if achievements are added; daily reset offline/relog/restart; all companion models/coordinates.
- **Provable statically:** Git lineage and final file presence; final DDL after ordered migrations; statement parameter/result symmetry (including the found mismatches); registration declarations/calls; explicit constants/control flow; absence of Playerbots dependencies; battle-pet/unrelated hunk boundaries.

## Final-State Confidence

| Area | Grade | Reason |
|---|---|---|
| Commit/file lineage | STRONG | Complete reachable history and tip inspected. |
| Final character DDL | STRONG | Ordered migrations are explicit. |
| DB/API integration | STRONG as defect finding; WEAK as usable implementation | Static mismatches are direct and blocking. |
| World migration inventory/order | PARTIAL | Files are complete, but assumptions against an actual base DB were not executed. |
| Manager lifecycle | STRONG as source description; WEAK as correctness | Control flow is explicit; multiple static defects and no runtime proof. |
| Phasing | WEAK | Inert layer API, mask collision risk, micro-zone inconsistency, no multiplayer test. |
| Tutorial | PARTIAL | IDs/SAI are explicit; no client/base-data validation. |
| Plant/growth/problems/harvest | WEAK | Internal contradictions and transaction hazards. |
| Reputation/companions/dailies | PARTIAL-to-WEAK | Standard API direction is clear; IDs/semantics/visibility unvalidated. |
| Achievement integration | STRONG | There is no Tillers change; Candidate 13 hunk is demonstrably battle-pet-only. |
| Playerbots/battle-pet boundary | STRONG | File/hunk dependencies are separable. |

## Overall Classification

**NEEDS_BUILD18414_RUNTIME_BASELINE**

**INFERENCE:** The final source state and migration lineage are now reconstructed, so neither final-state nor migration discovery is the primary remaining unknown. However, static defects mean it is not ready to stage wholesale, and the central phasing/quest/GO model depends on unverified build-18414 behavior. Unitized review may begin only with inert T1; player-facing units require a runtime baseline.

## Recommended Task 037 Unit

**T1 — final character schema + corrected database statement contract review.** Task 037 should remain review/design-first: specify direct final CREATEs, remove obsolete `last_growth_tick`/dead statement assumptions, correct parameter ordering, and define disposable-DB symmetry tests. It must not import the historical destructive migrations blindly and must not enable player-facing hooks.
