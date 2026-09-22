# Task 038: Tillers T1 Inert Character Schema

## Starting State

- **FACT:** Work started from clean commit `5afa10d1999e7fbadab3568d04c10c8d33d7980e` on task branch `work`. That commit merges Task 037 (`770fd46`) into the locally available preservation integration history.
- **FACT:** No Tillers runtime C++, CharacterDatabase farm statement, `player_farm_state` migration, or `player_farm_plots` migration existed at the starting commit.
- **FACT:** Task 038 changes only this report and one character-schema migration.

## Task 037 Contract

- **FACT:** Task 037 is classified `READY_FOR_INERT_T1_IMPLEMENTATION` and selects Option A: schema migration only.
- **FACT:** Its canonical DDL defines the four approved state columns and seven approved plot columns. It contains no `last_growth_tick`, custom friendship/daily table, foreign key, destructive operation, or backfill.
- **FACT:** A byte-for-byte comparison of the SQL fenced under Task 037's `Final Canonical DDL` heading with the Task 038 migration passed.

## Final Migration Filename

- **FACT:** The final filename is `sql/updates/characters/2026_09_22_00_characters_tillers_farm.sql`.
- **FACT:** Repository-wide filename inspection found this date/sequence unused before creation and found exactly one file with this name afterward. It sorts after the prior character updates, whose latest names are dated 2024-09-06.

## Migration Scope

- **FACT:** The migration contains exactly two plain `CREATE TABLE` statements, in state-then-plots order.
- **FACT:** It contains no `IF NOT EXISTS`, `DROP`, `ALTER`, `RENAME`, `UPDATE`, foreign key, secondary index, check constraint, trigger, seed row, or character backfill.

## Final DDL

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

## Historical Migrations Excluded

- **FACT:** None of Andrew's 2026-05-28 through 2026-06-21 historical Tillers migrations was imported.
- **FACT:** No obsolete `last_growth_tick`, destructive drop/alter, global update, rename, column removal, or legacy backfill was added.
- **INFERENCE:** Direct final-state creation is safer for preservation's fresh-install scope because an unknown pre-existing schema is rejected rather than guessed at or rewritten.

## Disposable Database Environment

- **FACT:** Validation used a fresh local-only datadir `/tmp/mop-task038-mariadb`, Unix socket `/tmp/mop-task038.sock`, disabled networking, and databases `mop_task038_characters` and `mop_task038_collision`. No configured preservation database was accessed.
- **FACT:** Server version was `10.11.14-MariaDB-0ubuntu0.24.04.1`. SQL mode was `STRICT_TRANS_TABLES,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,NO_ENGINE_SUBSTITUTION`.
- **FACT:** Server defaults were `utf8mb4` / `utf8mb4_general_ci`; the test databases and created tables reported `utf8mb3` / `utf8mb3_general_ci`, MariaDB's spelling of the requested `utf8` alias.
- **FACT:** Both disposable databases were dropped and the task-only server was stopped after validation.

## Actual Migration Application

- **FACT:** The repository file `sql/updates/characters/2026_09_22_00_characters_tillers_farm.sql` was supplied directly to the MariaDB CLI against the empty `mop_task038_characters` database.
- **FACT:** The application exited with status 0 and created both tables. No temporary SQL approximation was used.

## SHOW CREATE / DESCRIBE / Index Validation

- **FACT:** `SHOW CREATE TABLE` confirmed the exact columns, unsigned widths, defaults, nullability, comments, InnoDB engine, and utf8 charset selected by Task 037.
- **FACT:** `DESCRIBE` reported state defaults 14/4/0, nullable plot seed and maturity, and all other required defaults and nullability.
- **FACT:** `SHOW INDEX` reported only `PRIMARY (guid)` for state and only composite `PRIMARY (guid, plot_id)` for plots. No secondary index appeared.

## Roundtrip Smoke

- **FACT:** Inserting only `guid=1001` into state produced defaults `(farm_phase, plots_unlocked, best_friend_unlocks)=(14,4,0)`.
- **FACT:** An empty `(1001,0)` plot retained SQL NULL for seed and maturity and defaults state/watering/pests to zero.
- **FACT:** A planted `(1001,1)` plot round-tripped state 3, seed 79102, watering 1, pests 0, and maturity 1790000000 without alteration.

## Primary-Key Smoke

- **FACT:** A duplicate state GUID failed with CLI status 1 and MariaDB error 1062, `Duplicate entry '1001' for key 'PRIMARY'`.
- **FACT:** A duplicate `(guid, plot_id)` failed with CLI status 1 and MariaDB error 1062, `Duplicate entry '1001-0' for key 'PRIMARY'`.
- **FACT:** `(1002,0)` inserted successfully alongside `(1001,0)`, proving plot IDs are unique per character rather than globally.

## Collision Safety Test

- **FACT:** `mop_task038_collision` was given a stale Andrew-style state table containing `last_growth_tick` and sentinel `(77,12,8,123456)`.
- **FACT:** Applying the actual migration exited 1 at its first statement with MariaDB error 1050, `Table 'player_farm_state' already exists`.
- **FACT:** Post-failure `SHOW CREATE TABLE` still included the unchanged stale `last_growth_tick` structure. The sentinel SELECT still returned exactly `(77,12,8,123456)`.
- **FACT:** No destructive conversion occurred. Loud failure is the intended collision policy.

## Partial-DDL / Second-Table Behavior

- **FACT:** After the first-statement collision, an `information_schema.TABLES` query returned zero matching `player_farm_plots` tables in `mop_task038_collision`.
- **FACT:** For the tested CLI invocation, failure on the first CREATE prevented execution of the second CREATE; this result is observed behavior and not an assumption of transactional DDL.

## Updater Compatibility

- **FACT:** The character base schema includes `$/sql/updates/characters` as `RELEASED` in `updates_include`.
- **FACT:** `UpdateFetcher` recursively discovers `.sql` files, rejects duplicate basenames, and orders entries lexically by filename. The chosen unique filename satisfies that model.
- **FACT:** No manifest, CMake edit, or source registration is required. Static verification was used; no updater was pointed at a configured preservation database.

## Runtime Inertness

- **FACT:** These tables are currently unused at runtime. Task 038 adds no CharacterDatabase enum, prepared statement, manager, hook, script, or player-facing caller.
- **FACT:** Repository searches for `CHAR_INS_PLAYER_FARM_STATE`, `CHAR_UPD_PLAYER_FARM_STATE`, and `TillersFarmMgr` found no runtime implementation. New table-name references are confined to Task 037 documentation, this report, and this migration.

## Rollback / Legacy Upgrade Policy

- **FACT:** Disposable rollback was deletion of the disposable databases after inspection.
- **FACT:** There is no destructive production down migration and no legacy Andrew conversion. A production rollback after data exists requires stopped writers, backup/export, compatible code rollback, and a separately reviewed retention/conversion plan.
- **UNKNOWN:** The shape and provenance of any operator's manually installed Tillers tables cannot be inferred safely. Such a collision must remain unapplied pending operator diagnosis and backup-first conversion review.

## T2 Prerequisites

- **FACT:** T2 must add reviewed prepared SELECT and write contracts; none exist in T1.
- **FACT:** T2 must fix reset durability, handle character-deletion cleanup, and preserve nullable seed/maturity representation with NULL-aware binds and reads.
- **FACT:** T2 must retain Task 037's bind/result orders, atomic full-save expectations, maturity range checking, and loud duplicate-key behavior.

## Final Classification

**INERT_T1_IMPLEMENTED**

- **FACT:** The exact canonical DDL is present under a unique updater-compatible filename, applies cleanly to an empty database, matches schema inspection, passes roundtrip/key tests, and rejects a stale collision without changing its data.
- **FACT:** Only the migration and this report changed. No historical destructive SQL or runtime Tillers code was imported.

## Validation Results

- **FACT:** Starting-tree, filename, canonical-DDL, scope, updater-path, and runtime-inertness checks passed.
- **FACT:** `git diff --check` passed before final commit.
- **FACT:** Fresh migration application, `SHOW CREATE`, `DESCRIBE`, index inspection, default/NULL/value roundtrip, duplicate-key rejection, cross-character plot reuse, stale collision, sentinel preservation, and second-table absence all passed.
- **INFERENCE:** These results establish an inert T1 schema prerequisite only. They do not establish T2 persistence correctness or any gameplay behavior.
