# Task 037: Tillers T1 Character Schema and DB Contract

## Starting State

- **FACT:** Investigation started on branch `work` at `268a29a20aed6d58ccd83c51b238097ede687a6f` (merge of Task 036 / PR 32). The preceding Task 036 documentation commit is `9f5edd7`.
- **FACT:** `git status --short` was empty. Searches outside Task 036 found no `player_farm_state`, `player_farm_plots`, Tillers prepared statements, or `TillersFarmMgr` implementation in preservation. Thus Task 036 is integrated and no Andrew Tillers C++ or schema has been imported.
- **FACT:** Task 037 changes documentation only. It does not add a migration, statements, manager, hooks, phasing, world data, or gameplay content.

## Preservation Character-Migration Conventions

- **FACT:** `sql/updates/characters/` contains `.gitkeep` and two updates: `2024_09_06_00_characters_auctionbidders.sql` and `2024_09_06_01_characters_auctionhouse_houseid.sql`. Names sort as `YYYY_MM_DD_sequence_description.sql`; the latest is `2024_09_06_01_characters_auctionhouse_houseid.sql`.
- **FACT:** The one current character update that creates a table uses backtick quoting, uppercase SQL types, explicit widths/unsigned/defaults, `ENGINE=InnoDB DEFAULT CHARSET=utf8`, and no file header. It uses `DROP TABLE IF EXISTS`, but that destructive pattern is inappropriate for this new preservation-owned table.
- **FACT:** `CREATE TABLE IF NOT EXISTS` is not the current create style. A plain `CREATE TABLE` makes a collision fail visibly.
- **FACT:** The base schema commonly includes dump-style comments and table comments, uses InnoDB, and mixes `utf8` and `utf8mb4`. The Andrew farm tables use `utf8`; retaining `utf8` conforms to the directly relevant current migration and changes no data-bearing text behavior (these tables have no string columns).
- **FACT:** The updater obtains included directories from each database's `updates_include`, recursively discovers `.sql` files, and rejects duplicate filenames because ordering is filename-based. Character base population comes from `sql/base/characters.sql`; incremental placement is `sql/updates/characters/`.
- **INFERENCE:** The future migration should be a single direct final-state file named `2026_09_22_00_characters_tillers_farm.sql`, provided that Task 038 rechecks HEAD and confirms that name remains unused immediately before staging.

## Character GUID Contract

- **FACT:** `characters.guid` is `int(10) unsigned NOT NULL DEFAULT '0'` and its primary key. Representative character-owned auxiliary tables use the same 32-bit unsigned storage shape.
- **FACT:** `ObjectGuid::LowType` is `uint32`; `ObjectGuid::GetCounter()` returns `uint32`; Andrew stores the manager key and `guidLow` as `uint32` and binds it with `setUInt32`.
- **FACT:** Both proposed farm `guid` columns are `INT(10) UNSIGNED NOT NULL DEFAULT '0'`.
- **Classification:** **GUID_TYPE_MATCH**.

## Andrew Schema Lineage

A disposable clone, `/tmp/andrew-loa-task037`, was checked out at final runtime commit `bd8ad5515418d94abd35ba7bf71430979243a09c`. No preservation remote was changed.

| Commit | Character migration | Effect |
|---|---|---|
| `b4a0c209` | `2026_05_28_00_tillers_farm_plots.sql` | Destructively drops/recreates the seven-column plots table. |
| `b4a0c209` | `2026_05_28_01_tillers_farm_state.sql` | Destructively drops/recreates state with `farm_phase` default 0 and `last_growth_tick`. |
| `1e138b1f` | `2026_06_02_01_tillers_farm_state_bitmask.sql` | Rewrites all state rows to the 14/12/8/0 model and synchronizes plot counts. |
| `1e138b1f` | `2026_06_02_03_tillers_farm_state_schema.sql` | Changes state defaults/comments to phase 14 and plots 4. |
| `867b69b0` | `2026_06_03_07_tillers_companion_bitmask.sql` | Adds unsigned-smallint `companions`. |
| `867b69b0` | `2026_06_03_08_tillers_best_friend_unlocks_rename.sql` | Renames it to `best_friend_unlocks` and globally backfills missing character rows. |
| `bd8ad551` | `2026_06_21_01_tillers_remove_last_growth_tick.sql` | Drops obsolete `last_growth_tick`. |

- **FACT:** The ordered final state has only the two tables described below. `last_growth_tick` is absent. Earlier custom friendship and daily-quest tables/scripts had already been removed in the reconstructed lineage and are not dependencies of final persistence.
- **FACT:** Andrew's comments say `MPS_GUID`, an unexplained label; canonical preservation wording uses “Player GUID” without importing that typo/ambiguity.
- **DO_NOT_REPLAY:** The historical drops, global updates/backfill, rename, and final drop exist only because Andrew evolved an already-shipped fork schema. They are not a safe preservation install path.

## Final Canonical DDL

This is the exact direct-create design tested from `/tmp/task037_final_create.sql`:

```sql
CREATE TABLE `player_farm_state` (
  `guid` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Player GUID',
  `farm_phase` TINYINT(3) UNSIGNED NOT NULL DEFAULT '14' COMMENT 'Farm state bitmask: 14=all obstacles, 12=weeds cleared, 8=wagon cleared, 0=all cleared',
  `plots_unlocked` TINYINT(3) UNSIGNED NOT NULL DEFAULT '4' COMMENT 'Number of unlocked plots (4, 8, 12, or 16)',
  `best_friend_unlocks` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Bitmask of unlocked best-friend farm decorations',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Tracks per-player farm-wide state for Sunsong Ranch farming system';

CREATE TABLE `player_farm_plots` (
  `guid` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Player GUID',
  `plot_id` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Plot number 0-15',
  `state` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'Plot state (0=EMPTY, 1=SOIL_PREPARED, 2=SEEDED, 3=GROWING, 4=NEEDS_WATER, 5=NEEDS_PEST_CONTROL, 6=READY_TO_HARVEST, 7=BROKEN)',
  `seed_entry` INT(10) UNSIGNED DEFAULT NULL COMMENT 'Seed item entry ID',
  `needs_watering` TINYINT(1) NOT NULL DEFAULT '0' COMMENT 'Whether plot needs watering',
  `has_pests` TINYINT(1) NOT NULL DEFAULT '0' COMMENT 'Whether plot has pests to spray',
  `maturity_timestamp` INT(10) UNSIGNED DEFAULT NULL COMMENT 'Unix timestamp when crop will be ready (NULL = not planted)',
  PRIMARY KEY (`guid`, `plot_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='Tracks per-player farm plot states for Sunsong Ranch farming system';
```

- **FACT:** This matches Andrew's final types, defaults, nullability, primary keys, InnoDB engine, utf8 charset, and substantive comments. It directly incorporates the final altered state rather than the intermediate schema.
- **FACT:** It contains no drop, rename, obsolete column/table, backfill, or foreign key.

## Fresh Preservation vs Andrew Legacy Upgrade

- **Decision:** Support **A: fresh/current preservation installations where both tables are absent**.
- **FACT:** Preservation has never shipped either table. Supporting arbitrary Andrew snapshots would require recognizing multiple intermediate shapes, deciding how to translate possibly modified data, and performing ALTER/DROP operations that preservation cannot safely infer.
- **INFERENCE:** Exact-final Andrew installations and stale/manual installations should use a separately reviewed, backup-first manual conversion/import path. Broad compatibility in the normal migration would risk destroying unknown user data for an unshipped fork.

## NULL Semantics

| Field | Final DDL | Final Andrew behavior | Classification |
|---|---|---|---|
| `seed_entry` | nullable unsigned int | `Field::GetUInt32()` collapses SQL NULL to 0; memory uses 0 for none; full save/reset/repair bind integer 0, never NULL. | **ZERO_EQUIVALENT** semantically, **CURRENT_CODE_MIXED** representationally. |
| `maturity_timestamp` | nullable unsigned int | `Field::GetUInt64()` collapses SQL NULL to 0; memory uses time 0 for no maturity; full save/reset bind 0, never NULL. | **ZERO_EQUIVALENT** semantically, **CURRENT_CODE_MIXED** representationally. |

- **FACT:** NULL carries no distinct gameplay meaning in Andrew's final code, but the final schema deliberately permits it and describes NULL as “not planted.” The DB API supports `PreparedStatement::setNull`.
- **Decision:** Preserve nullable DDL. T2 should deliberately bind NULL for absent seed/maturity and test `Field::IsNull()` before numeric reads if representation is to round-trip. It may normalize legacy zero to absence on the next save. Do not silently make the columns `NOT NULL` in T1.

## Type / Range Review

| Value | C++/known domain | SQL capacity | Finding |
|---|---|---|---|
| character low GUID | `uint32` | `INT UNSIGNED` (0..4294967295) | Exact width; tested 0 and 4294967295. |
| farm phase | `uint8`; known 0, 8, 12, 14 | `TINYINT UNSIGNED` | Sufficient; all four tested. |
| plots unlocked | `uint8`; 4, 8, 12, 16 | `TINYINT UNSIGNED` | Sufficient; 4 and 16 tested. |
| friend unlocks | `uint16`; ten known bits, mask accumulation | `SMALLINT UNSIGNED` | Exact width; multi-bit 37 and 65535 stored. |
| plot ID | `uint8`; valid 0..15 | `TINYINT UNSIGNED` | Sufficient; 0 and 15 tested. DDL intentionally does not add a check constraint; manager validates. |
| plot state | `enum : uint8`; 0..7 | `TINYINT UNSIGNED` | Sufficient. |
| seed entry | `uint32` | nullable `INT UNSIGNED` | Exact width. |
| maturity | memory `time_t`, Andrew binds/reads 64-bit | nullable `INT UNSIGNED` | Stores Unix seconds through 2106; realistic `1790000000` tested. T2 must range-check before binding to avoid 64-to-32 narrowing beyond SQL range. |

## Primary Keys / Indexes

- **FACT:** `PRIMARY KEY (guid)` enforces one state row per character.
- **FACT:** `PRIMARY KEY (guid, plot_id)` enforces at most one row per character/plot, permits the same plot ID for different characters, and supports delete/select by the leftmost `guid` prefix.
- **Decision:** No additional index is statically useful to the known contract. There is no query by `plot_id` alone and no speculative global uniqueness.

## Character Deletion / FK Behavior

- **FACT:** The character schema and current updates contain no foreign keys/`ON DELETE` cascades. Ownership cleanup is explicit prepared-statement work in `Player::DeleteFromDB`.
- **FACT:** Andrew adds no farm-table statements to `Player::DeleteFromDB`; its only delete removes plots during a full farm save, and no state-row delete exists.
- **Classification:** **ROW_LEAK_POSSIBLE** after permanent character deletion.
- **Decision:** Do not invent a foreign key inconsistent with the repository. A later manager/runtime task must add explicit deletion cleanup (or separately establish a repository-wide cascade policy). Task 037 implements neither.

## Andrew Prepared-Statement Inventory

There are exactly ten final Andrew farm enums/statements and no additional prepared farm statement.

| Enum | SQL placeholder order | Final caller/bind order | Frequency / use | Finding |
|---|---|---|---|---|
| `CHAR_DEL_PLAYER_FARM_PLOTS` | `DELETE ... WHERE guid=?` → guid | `SavePlayerFarm`: `uint32 guid` | once per full save | Used; symmetric. |
| `CHAR_INS_PLAYER_FARM_STATE` | INSERT guid, phase, plots, **last tick**, unlocks | none | never | Dead and invalid against final DDL. |
| `CHAR_INS_PLAYER_FARM_PLOT` | guid, plot, state, seed, water, pests, maturity | full-save loop: `uint32,uint8,uint8,uint32,bool,bool,uint64` | once per valid plot/full save | Used; positional symmetry, but absence becomes 0. |
| `CHAR_UPD_PLAYER_FARM_STATE` | REPLACE guid, phase, plots, unlocks | phase, plots, unlocks, guid | once per full save | Used; completely shifted, corrupting/failing data. |
| `CHAR_UPD_PLAYER_FARM_PLOT_PLANT` | state, seed, water, pests, maturity, guid, plot | same typed sequence | once per plant | Used; symmetric, non-null values. |
| `CHAR_UPD_PLAYER_FARM_PLOT_RESET` | state, seed, water, pests, maturity, guid, plot | same sequence, zeros for absent | once per harvest | Used; symmetric, but stores 0 rather than NULL. |
| `CHAR_UPD_PLAYER_FARM_PLOT_WATER` | water, guid, plot | false, guid, plot | once per water action | Used; symmetric. A separate STATE update can share its transaction. |
| `CHAR_UPD_PLAYER_FARM_PLOT_PESTS` | pests, guid, plot | false, guid, plot | once per pest action | Used; symmetric. A separate STATE update can share its transaction. |
| `CHAR_UPD_PLAYER_FARM_PLOT_REPAIR` | state, seed, guid, plot | prepared, 0, guid, plot | once per repair | Used; symmetric but does not clear flags/maturity. |
| `CHAR_UPD_PLAYER_FARM_PLOT_STATE` | state, guid, plot | state, guid, plot | water/pests transition | Used; symmetric. |

All update keys use SQL `guid INT UNSIGNED`/`plot_id TINYINT UNSIGNED` and caller `setUInt32`/`setUInt8`. State, phase, and plot count are `TINYINT UNSIGNED`; unlock mask is `SMALLINT UNSIGNED`; seed is `INT UNSIGNED`; booleans are signed `TINYINT(1)` but bound as bool; maturity is `INT UNSIGNED` despite Andrew's 64-bit bind.

## Dead Statement

- **FACT:** Repository-wide final-tip searches find `CHAR_INS_PLAYER_FARM_STATE` only in the enum and registration, not a caller. It names the dropped `last_growth_tick` column.
- **Classification:** **REMOVE_FROM_FINAL_CONTRACT / DO_NOT_PORT**. Repairing and retaining an unused API would expand T1 without a consumer.

## Corrected Farm-State Upsert

- **Decision:** Retain Andrew's established, minimal `REPLACE INTO` model. This repository already uses REPLACE extensively. With only a stable single-column primary key and all final columns explicitly supplied, it handles absent and present rows without an extra round trip. Future schema additions must be consciously reviewed because omitted future columns would take defaults.

```sql
REPLACE INTO player_farm_state
    (guid, farm_phase, plots_unlocked, best_friend_unlocks)
VALUES (?, ?, ?, ?)
```

| Placeholder / setter | Column | SQL type | Required T2 bind |
|---:|---|---|---|
| 0 | `guid` | `INT UNSIGNED` | `setUInt32(0, guidLow)` |
| 1 | `farm_phase` | `TINYINT UNSIGNED` | `setUInt8(1, state.farmState)` |
| 2 | `plots_unlocked` | `TINYINT UNSIGNED` | `setUInt8(2, state.plotsUnlocked)` |
| 3 | `best_friend_unlocks` | `SMALLINT UNSIGNED` | `setUInt16(3, state.bestFriendUnlocks)` |

This SQL-order/caller-order identity makes the Andrew shift impossible when followed. Test insertion and replacement retained GUID 1003 while changing only its state values; GUID 4294967295 also round-tripped.

## Plot Insert Contract

```sql
INSERT INTO player_farm_plots
    (guid, plot_id, state, seed_entry, needs_watering, has_pests, maturity_timestamp)
VALUES (?, ?, ?, ?, ?, ?, ?)
```

| Index | Column | T2 bind |
|---:|---|---|
| 0 | guid | `setUInt32` |
| 1 | plot_id | `setUInt8` |
| 2 | state | `setUInt8` |
| 3 | seed_entry | `setNull` when absent, otherwise `setUInt32` |
| 4 | needs_watering | `setBool` |
| 5 | has_pests | `setBool` |
| 6 | maturity_timestamp | `setNull` when absent, otherwise range-checked `setUInt32` |

- **FACT:** Andrew binds the same order but always binds numeric values at indexes 3 and 6, so SQL NULL cannot survive full save. That is a T2 caller correction, not a DDL reason to remove nullability.
- **FACT:** Plain INSERT is correct after the transaction deletes all rows. A duplicate key indicates a duplicate map/contract error and must fail visibly.

## Plot Update Contracts

Every key order is `WHERE guid = ? AND plot_id = ?`; every retained Andrew caller matches the SQL order. The canonical NULL-aware forms are:

| Contract | SET order, then key order | Required T2 bind order | Nullable handling | Expected row scope |
|---|---|---|---|---|
| PLANT | state, seed, water, pests, maturity; guid, plot | `u8,u32,bool,bool,u32;u32,u8` | planted seed/maturity non-NULL | at most one; normally one existing row |
| RESET | state, seed, water, pests, maturity; guid, plot | `u8,NULL,bool,bool,NULL;u32,u8` | clear seed/maturity with `setNull` | at most one; normally one |
| WATER | water; guid, plot | `bool,u32,u8` | none | at most one; may be followed by STATE in same transaction |
| PESTS | pests; guid, plot | `bool,u32,u8` | none | at most one; may be followed by STATE in same transaction |
| REPAIR | state, seed, water, pests, maturity; guid, plot | `u8,NULL,false,false,NULL;u32,u8` | clear all crop/problem fields | at most one; normally one |
| STATE | state; guid, plot | `u8,u32,u8` | none | at most one |

Canonical SQL:

```sql
UPDATE player_farm_plots SET state=?, seed_entry=?, needs_watering=?, has_pests=?, maturity_timestamp=? WHERE guid=? AND plot_id=?;
UPDATE player_farm_plots SET needs_watering=? WHERE guid=? AND plot_id=?;
UPDATE player_farm_plots SET has_pests=? WHERE guid=? AND plot_id=?;
UPDATE player_farm_plots SET state=? WHERE guid=? AND plot_id=?;
```

- **FACT:** PLANT and RESET share the first SQL shape; T1 needs one enum per distinct SQL contract, not two identical registrations. If T2 benefits from semantic names, aliases/helpers should not duplicate prepared SQL unnecessarily.
- **FACT (new hidden mismatch):** Andrew REPAIR changes only state and seed in DB and memory; it leaves watering, pests, and maturity untouched. Repair-to-prepared should clear all crop/problem fields like RESET. The canonical contract therefore uses the full reset shape and does not retain `CHAR_UPD_PLAYER_FARM_PLOT_REPAIR` as a distinct SQL statement.
- **FACT:** SQL affected-row reporting can be zero for a missing or unchanged row and one for a changed row (and depends on client flags); the invariant is never more than one row because of the primary key. T2 should diagnose a missing expected plot rather than equate all zero counts with failure.

## Load Contract

| Result index | State SELECT | Reader |
|---:|---|---|
| 0 | `farm_phase` | `GetUInt8` |
| 1 | `plots_unlocked` | `GetUInt8` |
| 2 | `best_friend_unlocks` | `GetUInt16` |

| Result index | Plot SELECT | Reader |
|---:|---|---|
| 0 | `plot_id` | `GetUInt8` |
| 1 | `state` | `GetUInt8` → enum |
| 2 | `seed_entry` | `IsNull` then `GetUInt32` |
| 3 | `needs_watering` | `GetBool` |
| 4 | `has_pests` | `GetBool` |
| 5 | `maturity_timestamp` | `IsNull` then `GetUInt32`/checked `time_t` conversion |

- **FACT:** Andrew's final indexes and numeric widths match these SELECT orders, except it does not inspect NULL and reads maturity as UInt64 from a 32-bit SQL column.
- **Decision:** T2 should add prepared SELECT statements keyed by `guid`, rather than retain formatted `PQuery`. A numeric `%u` is not presently injection-prone, but prepared statements establish typed, reviewable result contracts and match persistence conventions.

## Default-Row Semantics

- **FACT:** With no state row, Andrew constructs memory defaults `farmState=14`, `plotsUnlocked=4`, `bestFriendUnlocks=0`, initializes all 16 plots empty, and later prepares the initial four during spawn. With a state row but no plot rows, load succeeds with an empty plot map; later lifecycle code may populate it.
- **FACT:** DDL defaults 14/4/0 agree with the state object's in-memory defaults; plot defaults agree with empty/false/absent values. Defaults do not create a row.
- **Decision:** T2's first full save (after safe initialization and only when plots exist) must create the state row and replace the complete plot set. There is no global migration backfill.

## Full-Save Transaction Contract

- **FACT:** Andrew begins one character transaction, deletes all plots, appends every valid plot INSERT, appends the state REPLACE, then commits once. The intended full save is atomic at InnoDB transaction level.
- **Required contract:** state upsert, delete-all, and every replacement INSERT must commit together. A duplicate `(guid,plot_id)` must produce a visible transaction failure; partial deletion/replacement must not commit. T2 must check/report commit behavior using the database layer rather than claim gameplay durability from compilation.

## Reset Durability Boundary

- **FACT:** `ResetPlayerFarm(uint32)` erases cache/spawn tracking and reconstructs default memory but issues no SQL and calls no save. Persisted rows can reload the old farm.
- **Classification:** **T2_MANAGER_DEFECT**, not a T1 schema defect. Do not add reset-specific schema/API to mask it; Task 038 must record it for the manager stage.

## Collision / Stale-Schema Policy

| Existing condition | Desired direct CREATE behavior |
|---|---|
| both absent | both CREATEs succeed |
| exact final table already present | first colliding CREATE fails loudly |
| stale state with `last_growth_tick` | state CREATE fails loudly; no alter/drop/data change |
| wrong columns/manual table | CREATE fails loudly; no guessing |
| any pre-existing user data | remains untouched; operator backs up and uses separately reviewed conversion |

- **Decision:** No `IF NOT EXISTS`: silently accepting an incompatible structure would mark the update applied while leaving runtime expectations false. No automatic DROP/ALTER: collisions require operator diagnosis.
- **INFERENCE:** Because MySQL/MariaDB DDL commits implicitly, a pathological collision on the second table could leave the first newly created. Operators must resolve/restore before marking the update applied. This is preferable to silently destroying either table, and the updater will report the failed file.

## Disposable Database Environment

- **FACT:** Validation used a newly initialized local-only datadir `/tmp/mop-task037-mariadb`, socket `/tmp/mop-task037.sock`, and unmistakable databases `mop_task037_characters` and `mop_task037_stale`. Networking was disabled. No configured/user database or credentials were used.
- **FACT:** Server: `10.11.14-MariaDB-0ubuntu0.24.04.1`. SQL mode: `STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION`. Server default: `utf8mb4_general_ci`; test databases: `utf8_general_ci`.
- **FACT:** MariaDB reports the requested `utf8` alias as `utf8mb3/utf8mb3_general_ci`, which is conceptually the requested repository contract.

## CREATE Validation

- **FACT:** Applying only `/tmp/task037_final_create.sql` to the empty disposable database succeeded.
- **FACT:** `SHOW CREATE`, `DESCRIBE`, and `SHOW INDEX` confirmed every intended type, unsigned attribute, default, NULL flag, table comment, InnoDB engine, utf8 charset, and only the intended primary keys.

## Roundtrip Tests

- **FACT:** Character A (`guid=1001`, phase 14, four plots, mask 0) and character B (`guid=1002`, phase 0, sixteen plots, mask 37) round-tripped in the intended SELECT order.
- **FACT:** Empty, seeded, watering, pest, ready, plot IDs 0/15, NULL/non-NULL seed, and NULL/non-NULL maturity rows round-tripped exactly.

## State-Upsert Test

- **FACT:** Correct-order REPLACE inserted GUID 1003 as 14/4/0, then replaced it as 0/16/1023 without moving phase into GUID. A boundary row with GUID 4294967295, phase 8, plots 12, and mask 65535 also round-tripped exactly.

## Plot-Update Tests

- **FACT:** INSERT, PLANT, WATER, PESTS, STATE, canonical REPAIR, canonical RESET, and delete-all-by-GUID were simulated. Each update reported one changed row, keys remained stable, and columns outside each narrow update remained unchanged.
- **FACT:** Delete-all removed exactly the one test row for GUID 2000 and left rows owned by other GUIDs.

## Primary-Key Tests

- **FACT:** Duplicate state INSERT failed with error 1062 on `PRIMARY`; duplicate `(guid,plot_id)` INSERT failed with error 1062 on `PRIMARY`.
- **FACT:** Plot ID 7 simultaneously existed for GUIDs 2000 and 2001. Plot IDs 0 and 15 were accepted. No global plot uniqueness or additional index appeared.

## NULL Tests

- **FACT:** Explicit NULL seed/maturity inserted and selected as NULL. The same row transitioned NULL → values → NULL using the canonical statement model; both final `IS NULL` assertions returned 1.
- **T2 requirement:** use `setNull`/`Field::IsNull` for representational preservation; Andrew's integer-zero bindings do not meet this requirement even though its gameplay model treats zero and NULL equivalently.

## Stale-Schema Test

- **FACT:** The second database contained a stale state table with `last_growth_tick` and row `(77,12,8,123456)`. Applying the clean CREATE returned exit status 1 / error 1050 “Table already exists.”
- **FACT:** `SHOW CREATE` and SELECT afterward confirmed the stale structure and row were unchanged; no plot table was created because failure occurred at the first statement.
- **Decision:** This is the intended loud, non-destructive result. A future manual Andrew conversion must start with backup/export, positively identify a supported schema, transform into separately named tables, validate, and only then swap under explicit operator control.

## Rollback Plan

- **Disposable validation only:** `DROP TABLE player_farm_plots; DROP TABLE player_farm_state;`, followed by dropping the two disposable databases.
- **Production:** no destructive down migration. Once player data exists, rollback requires stopping writers, backup/export, version-compatible code rollback, and an explicitly reviewed data retention/conversion plan. Blind drops are forbidden.

## DB Updater Compatibility

- **FACT:** Character update discovery is driven by `updates_include`, recursive `.sql` discovery, global filename uniqueness, and lexical filename ordering. Plain CREATE failure propagates through the updater rather than being silently accepted.
- **Proposed Task 038 placement/name:** `sql/updates/characters/2026_09_22_00_characters_tillers_farm.sql`, subject to rechecking current HEAD/name availability. It should contain only the two direct CREATEs.

## Canonical T1 Contract

### A. Final DDL

The two direct CREATE statements in “Final Canonical DDL”; historical migrations are **DO_NOT_REPLAY**.

### B. Required prepared-statement enums

For the eventual persistence manager contract:

1. state SELECT by guid;
2. plots SELECT by guid ordered by plot ID;
3. state REPLACE/upsert;
4. delete all plots by guid;
5. full plot INSERT;
6. plot full mutation (PLANT/RESET/REPAIR shape);
7. watering flag update;
8. pest flag update;
9. state-only update.

`CHAR_INS_PLAYER_FARM_STATE` is **DO_NOT_PORT**. Separate identical PLANT/RESET SQL registrations and the incomplete REPAIR registration are unnecessary API duplication.

### C–E. SQL, placeholders, future binds

The exact SQL/order tables above are normative. Columns are listed in bind order; setters at indexes 0..N must follow that order. State begins with `guid`, never ends with it.

### F. Load result order

State: phase, plot count, friend mask. Plots: plot ID, state, seed, watering, pests, maturity.

### G. NULL handling

Preserve nullable DDL and bind/read actual NULL for absent seed/maturity. Range-check non-NULL `time_t` before the 32-bit SQL bind.

### H. Transaction expectations

A full save is one atomic state-plus-complete-plot transaction. Narrow multi-statement transitions (WATER/PESTS plus STATE) are also one transaction. Duplicate plot keys fail visibly; missing expected update rows are diagnosed. This specification enables no runtime by itself.

## Task 038 Implementation Boundary

- **Decision: OPTION A — schema migration only.** It is the smallest useful inert unit, independently database-testable, and creates no dead production C++ API.
- **Rejected B/C:** prepared registrations without the T2 manager would be unused; helpers would add more compile-only surface. The dead historical enum demonstrates the cost of staging callerless APIs.
- **Rejected D:** the schema alone is useful and inert, validates updater/collision behavior, and gives the later manager a stable prerequisite.
- **Bounded scope:** Task 038 should add only the direct final character migration and validate it on an empty disposable DB plus collision DB. It must not add statements, manager code, deletion hooks, reset fixes, runtime hooks, phasing, world SQL, or content.

## Final Classification

**READY_FOR_INERT_T1_IMPLEMENTATION**

- **FACT:** Final schema and GUID contract are unambiguous; obsolete fields/tables are excluded; normal preservation installation needs no destructive conversion/backfill.
- **FACT:** Correct state order is explicit, dead API is excluded, retained plot contracts are symmetric, repair's hidden stale-field issue is corrected in the specification, and NULL is safely preserved.
- **FACT:** Fresh CREATE, schema inspection, roundtrip, upsert, every plot mutation, key behavior, NULL transitions, boundaries, and non-destructive stale collision were validated on disposable MariaDB.
- **FACT:** Updater placement is understood, no runtime was enabled, and Task 038 is limited to a schema-only inert migration.
