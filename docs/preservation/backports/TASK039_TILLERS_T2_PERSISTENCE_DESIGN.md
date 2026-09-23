# Task 039: Tillers T2 Inert Persistence Design

## Starting State

- **FACT:** Review began on clean branch `work` at `f5ea8fd6d3c0aa6ae7f49496fe7627b18f861c0b`, the merge of Task 038 into the locally available preservation integration history. `git status --short` was empty.
- **FACT:** The Task 038 migration is present. Repository searches found no `TillersFarmMgr`, farm prepared-statement enum, or runtime read/write of either farm table; table-name references were limited to SQL and preservation documentation.
- **FACT:** Task 039 changes documentation only. It does not change C++, SQL, registration, or runtime behavior.

## T1 Schema Verification

- **FACT:** `player_farm_state` is exactly: `guid INT UNSIGNED NOT NULL DEFAULT 0` primary key; `farm_phase TINYINT UNSIGNED NOT NULL DEFAULT 14`; `plots_unlocked TINYINT UNSIGNED NOT NULL DEFAULT 4`; `best_friend_unlocks SMALLINT UNSIGNED NOT NULL DEFAULT 0`.
- **FACT:** `player_farm_plots` is exactly: `guid INT UNSIGNED NOT NULL DEFAULT 0`; `plot_id TINYINT UNSIGNED NOT NULL DEFAULT 0`; `state TINYINT UNSIGNED NOT NULL DEFAULT 0`; nullable `seed_entry INT UNSIGNED`; `needs_watering TINYINT NOT NULL DEFAULT 0`; `has_pests TINYINT NOT NULL DEFAULT 0`; nullable `maturity_timestamp INT UNSIGNED`; primary key `(guid, plot_id)`.
- **FACT:** Both are InnoDB/utf8. There are no foreign keys or secondary keys. The migration matches Task 037/038 and has no schema drift.

## Andrew Final Source

- **FACT:** A disposable repository at `/tmp/andrew-loa-task039` was fetched from Andrew Downey's GitHub repository and checked out at final runtime revision `bd8ad5515418d94abd35ba7bf71430979243a09c`. No preservation remote was altered.
- **FACT:** Inspected files were `src/server/scripts/Pandaria/TillersFarmMgr.{h,cpp}`, `TillersZoneHooks.cpp`, `TillersWorldHooks.cpp`, `TillersSoilScript.cpp`, `TillersWorkstation.cpp`, `src/server/scripts/Commands/cs_tillers.cpp`, and `src/server/database/Database/Implementation/CharacterDatabase.{h,cpp}`.
- **FACT:** Later master tip `f1abdd1a2754f4a39f9e5d2cd5112e63899a86c4` differs in the focused runtime files only by changing `player->getLevel()` to `player->GetLevel()` in harvest reputation code. It changes neither persistence nor Tillers behavior under review; no later persistence/runtime-farm design exists.

## Persistent Data Model

| Field / collection | Andrew C++ | T1 SQL | Default | Ownership | Classification |
| --- | --- | --- | --- | --- | --- |
| farm state/phase | `uint8` (`FarmState`) | unsigned tinyint | 14 | per character | `PERSISTENT_T2` |
| plots unlocked | `uint8` | unsigned tinyint | 4 | per character | `PERSISTENT_T2` |
| friend unlock mask | `uint16` | unsigned smallint | 0 | per character | `PERSISTENT_T2` |
| plot ID | `uint8` | unsigned tinyint | logical 0 | per plot | `PERSISTENT_T2` |
| plot state | `FarmPlotState : uint8` | unsigned tinyint | 0 | per plot | `PERSISTENT_T2` |
| seed | Andrew `uint32`; T2 `optional<uint32>` | nullable unsigned int | absent | per plot | `PERSISTENT_T2` |
| watering | `bool` | tinyint | false | per plot | `PERSISTENT_T2` |
| pests | `bool` | tinyint | false | per plot | `PERSISTENT_T2` |
| maturity | Andrew `time_t`; T2 `optional<time_t>` | nullable unsigned int | absent | per plot | `PERSISTENT_T2` |
| plot collection | `map<uint8, FarmPlotData>` | rows keyed by `(guid,plot_id)` | empty in inert DTO | per returned aggregate | `PERSISTENT_T2` |

- **FACT:** Soil/obstacle/creature/best-friend GUID maps are `SPAWN_TRACKING`; custom phase selection and phase helpers are `PHASING`; cached plot/Yoon/obstacle/companion positions are `WORLD_DATA_CACHE`; commands and forced growth are `DEBUG/ADMIN`; inventory, random crop condition, maturity calculation, planting, growth, repair, harvest and rewards are `TRANSIENT_GAMEPLAY`.
- **INFERENCE:** T2 needs one value aggregate (`PlayerFarmData`) containing state-presence information, `PlayerFarmState`, and an ordered plot container. It needs no `Player*`, `Map`, `ObjectGuid`, spawn GUID, position, quest, item, spell, or phase field.

## Required Enums / Constants

| Contract | Classification | T2 decision |
| --- | --- | --- |
| maximum plot count `16`, valid IDs `0..15` | `T2_REQUIRED_DATA_CONTRACT` | minimal persistence header constant |
| plot states `0..7` | `T2_REQUIRED_DATA_CONTRACT` | preserve a scoped/minimal `uint8` enum for validation and readable DTOs |
| stored farm-state values `14,12,8,0` | `T2_REQUIRED_DATA_CONTRACT` only as the known persisted domain | minimal `uint8` enum; do not add progression helpers |
| default state `14/4/0` | `T2_REQUIRED_DATA_CONTRACT` | DTO defaults/constants |
| best-friend bit meanings, plot-to-phase calculation | `GAMEPLAY_ONLY` | exclude; T2 stores the opaque `uint16` mask |
| NPC/GO/item/spell/quest/zone/map IDs and positions | `WORLD_DATA_ONLY` or `GAMEPLAY_ONLY` | exclude |

- **INFERENCE:** Minimal typed enums are preferable to wholly raw DTOs because state range validation is persistence corruption defense. The header must not import Andrew's broad constants or attach retail semantics beyond the staged schema.

## Andrew Load Path

- **FACT:** `SpawnPlayerFarm(Player*)` called private synchronous `LoadPlayerState`; it derived the low GUID, issued an ad-hoc state SELECT on `CharacterDatabase`, validated phase as one of `14/12/8/0` and plot count as one of `4/8/12/16`, then inserted state in singleton maps.
- **FACT:** Only after a valid state row, Andrew issued an ad-hoc plot SELECT ordered by `plot_id`. It skipped IDs above 15, performed no plot-state validation, overwrote duplicate map keys, read nullable integers without `IsNull()`, converted maturity through `GetUInt64()` to `time_t`, and inserted plots into the cache. The primary key normally makes duplicates impossible.
- **FACT:** If the state row was absent, Andrew created state `14/4/0` and all 16 empty plot objects without querying orphan plots. If state existed but no plot rows existed, its map remained absent/empty. Invalid state returned `false`; the caller then overwrote state with defaults and materialized 16 plots. No data was repaired durably during load.
- **FACT:** Load and initial cache insertion occurred without the striped mutex, while other APIs could access the same maps. SQL was synchronous; no callback lifetime existed.
- **INFERENCE:** T2 load should synchronously execute both prepared queries into a local aggregate, validate it, and return by value. It should not mutate a shared cache.

## World-Data Boundary

- **FACT:** `LoadPlotPositions` is unrelated to character persistence. It incorrectly queried `creature_template` through `CharacterDatabase` before querying plot creatures through `WorldDatabase`.
- **FACT:** Excluded methods are `LoadPlotPositions`, `GetPlotPosition(s)`, `LoadYoonPosition`, `LoadObstaclePositions`, `LoadBestFriendUnlockPositions`, `GetBestFriendUnlockPosition(s)`, `CreateSoilGos`, `RemoveSoilGos`, `SpawnYoon`, `SpawnObstacles`, `SpawnPlayerFarmCompanions`, both companion spawn helpers, all spawned-object removal methods, `SpawnPlayerFarm`, `DespawnPlayerFarm`, and `IsPlayerFarmSpawned`.
- **FACT:** Plant/water/pests/repair/harvest/maturity/reputation/quest/force-growth APIs are also excluded because they touch gameplay, Player state, or world objects even when they issue character writes.

## Full Save Path

- **FACT:** Andrew returned without saving if its plot map was absent or empty. Otherwise it began `CharacterDatabaseTransaction`, appended delete-all plots, appended one complete insert per valid map entry, appended state REPLACE, and called fire-and-forget `CommitTransaction`.
- **FACT:** Andrew bound the REPLACE values as phase, plot count, friend mask, GUID despite SQL requiring GUID first. It bound nullable values as numeric zero and maturity as `uint64`, despite the SQL `INT UNSIGNED` range.
- **FACT:** Andrew read maps/references without holding a lock during snapshot/transaction construction, assumed state existed through `operator[]`, logged success immediately after queueing, and could not observe commit outcome.
- **Decision:** Canonical T2 save order is **state upsert, delete old plots, complete plot inserts**, all in one transaction. InnoDB rollback makes the two sensible orders equivalent on statement failure, but state-first matches Task 037, makes the aggregate's root explicit, and avoids a transient child-first mental model. Plot deletion/insertion is never committed separately.

## Farm-State Upsert

- **Decision:** Use repository convention `CHAR_REP_PLAYER_FARM_STATE`, not Andrew's misleading `CHAR_UPD_PLAYER_FARM_STATE`.
- **Contract:** `REPLACE INTO player_farm_state (guid, farm_phase, plots_unlocked, best_friend_unlocks) VALUES (?, ?, ?, ?)` with `setUInt32(0, guid)`, `setUInt8(1, farmState)`, `setUInt8(2, plotsUnlocked)`, `setUInt16(3, bestFriendUnlocks)`.
- **FACT:** This deliberately does not reuse Andrew's broken caller order.

## Prepared Load Contracts

- `CHAR_SEL_PLAYER_FARM_STATE`: `SELECT farm_phase, plots_unlocked, best_friend_unlocks FROM player_farm_state WHERE guid = ?`; bind GUID with `setUInt32(0, guid)`; read `GetUInt8`, `GetUInt8`, `GetUInt16` at result indexes 0, 1, 2.
- `CHAR_SEL_PLAYER_FARM_PLOTS`: `SELECT plot_id, state, seed_entry, needs_watering, has_pests, maturity_timestamp FROM player_farm_plots WHERE guid = ? ORDER BY plot_id`; bind `setUInt32(0, guid)`; read `GetUInt8`, `GetUInt8`, NULL-aware `GetUInt32`, `GetBool`, `GetBool`, NULL-aware `GetUInt32` at indexes 0..5.
- **Decision:** Call `Field::IsNull()` before both nullable getters. No state and no plots returns an unpersisted default aggregate. State without plots returns persisted state and zero plot rows. Plots without state are diagnosed as orphans and ignored; return an unpersisted default aggregate rather than adopting unrooted children.

## NULL Representation

- **Decision:** Use `std::optional<uint32>` for seed and `std::optional<time_t>` for maturity. Absence binds with `setNull`; presence binds the checked unsigned value. Load preserves SQL NULL as disengaged optional.
- **INFERENCE:** C++20 is already required and the core already uses `std::optional`; explicit absence prevents a legitimate numeric value from being conflated with SQL NULL and remains straightforward for later gameplay.

## Maturity Range Safety

- **Decision:** Before constructing any transaction, reject the entire save/reset input if a present maturity is negative or exceeds `UINT32_MAX`; log the GUID/plot and return failure. Never clamp, truncate, or translate an invalid present value to NULL.
- **FACT:** On platforms where `time_t` cannot represent the loaded `uint32`, the load must range-check before conversion; reject/skip that plot with an error rather than overflow. On the supported 64-bit environment all SQL values fit.
- **INFERENCE:** Rejecting before enqueue preserves the prior durable aggregate and avoids partial replacement. It is safer than silently changing persistence meaning.

## Plot Validation

- **Database corruption defense:** accept at most one row for each ID `0..15`; reject IDs above 15 and states above 7; enforce at most 16 rows; verify the DTO map key equals `plot.plotId` on save; reject duplicate IDs even though the PK should prevent them; reject non-boolean watering/pest raw values if the result API exposes them before bool conversion.
- **Decision:** Load logs and skips a malformed plot row while retaining other valid rows. A duplicate is logged and the later row is skipped (first ordered occurrence wins). Save validates the entire aggregate first and fails without queueing anything if any plot is malformed.
- **Database corruption defense:** known persisted farm phase is exactly `0/8/12/14`; known plot count is exactly `4/8/12/16`. Invalid state-root values make the persisted aggregate unusable: log and return an unpersisted default; do not normalize or auto-repair.
- **Gameplay enforcement excluded:** T2 does not require phase/count correspondence, constrain friend-mask bits, require seed/maturity for particular plot states, or infer quest/progression/crop rules.

## Default / No-Row Behavior

- **FACT:** Andrew gameplay expected all 16 map entries after its no-state/default path, while an existing state with no plot rows produced zero entries. This was inconsistent and tied materialization to spawning code.
- **Decision:** `LOAD_ONLY_PERSISTED` for T2: the default aggregate is `14/4/0`, `persisted=false`, with zero plot rows. T2 must not manufacture 16 durable or in-memory plot records. A later gameplay initialization unit may explicitly materialize logical plots when evidence and callers require them.
- **INFERENCE:** This preserves database truth, distinguishes “no row” from “16 empty rows,” and avoids embedding gameplay initialization in persistence.

## Initial Persistence Rule

- **Decision:** Load is read-only. Missing rows never cause a write. First durable state is created only by an explicit successful `Save` or `Reset` request from a future caller; T2 adds no login/logout caller. Consequently merely logging in cannot populate every character.

## Cache Ownership

- **Decision:** Choose **C, standalone persistence service returning values**. `Load(guid)` returns a value aggregate; `Save(guid, const data&)`, `Reset(guid)`, and deletion helpers operate on explicit values/transactions. T2 owns no global per-player cache.
- **INFERENCE:** This is the smallest testable substrate, avoids premature Player coupling and global lifetime, and allows later gameplay to choose stable per-Player or manager ownership without changing database symmetry.

## Locking Model

- **FACT:** Andrew's `GetPlayerState` and `GetPlayerPlots` returned references after their local striped lock was released. Soil/commands retained those references; save and multiple manager methods accessed maps without the same lock. `LoadPlayerState` initialized cache maps without locking. References could therefore race with erase, rehash, or mutation.
- **Decision:** `LOCKED_COPY_IN_OUT` degenerates to **no shared lock needed** in T2: DTOs are caller-owned values and the service has no mutable cache. Database pool synchronization is its own subsystem. No service API returns references to internal storage.
- **UNKNOWN:** A future gameplay cache's thread-affinity requirements depend on its eventual hook/callback design and must be reviewed then; they are not a reason to add an unsafe cache now.

## Player Lifetime

- **Decision:** T2 APIs take `uint32 guidLow`, not `Player*`. Load is synchronous and completes before returning; no DB callback captures Player. The returned value may outlive a Player because it owns no Player/world pointers.
- **Decision:** Intended future lifecycle is explicit request → caller-owned value → validate/save → caller releases it. Relog performs a fresh load. A fire-and-forget save cannot truthfully report durable success; reset requiring cache publication must use observable async completion or synchronous disposable-test execution.
- **FACT:** Character GUID low is the schema key and is sufficient within the character database contract.

## Reset Durability

- **FACT:** Andrew's `ResetPlayerFarm` only erased/rebuilt singleton maps and spawn trackers. It issued no SQL, so old rows returned after relog; it also materialized 16 plots.
- **Decision:** Durable reset is one transaction: state upsert `guid,14,4,0`, then delete all plot rows, and insert **no** default plots. This makes reset equal to the inert default persisted root with zero stored plots.
- **Decision:** Validate/build first, submit with `AsyncCommitTransaction`, and publish/reset any future caller cache only in a completion callback receiving `true`. On `false`, retain the old in-memory value and log failure. In T2A, which has no cache, return/forward completion rather than claiming success at enqueue time.

## Character Deletion Cleanup

- **FACT:** There are no foreign keys. Final hard deletion is assembled in `Player::DeleteFromDB` as one character-database transaction containing prepared deletes for auxiliary tables, followed by `CommitTransaction`. Soft unlink intentionally retains character-owned rows until final deletion.
- **Decision:** Add `CHAR_DEL_PLAYER_FARM_PLOTS` and `CHAR_DEL_PLAYER_FARM_STATE`, bind `guid` at index 0, and append both to the `CHAR_DELETE_REMOVE`/final-delete transaction in `Player::DeleteFromDB`; use plots-first then state for conceptual parent/child order. Old-character cleanup reaches the same method. Do not use cascade or an independent service transaction.
- **INFERENCE:** Service `Erase/DeleteCharacter` is useful for isolated testing/manual lifecycle only if it reuses a caller-supplied transaction; production deletion integration must remain atomic with the established hard-delete list.

## Narrow Mutation Boundary

- **Decision:** T2 exposes only `Load`, full `Save`, durable `Reset`, and append-to-character-delete/erase support, plus immutable/value data access. Plant/reset-plot/repair, water, pests, and state-only semantic methods belong to gameplay units.
- **Decision:** Although Task 037 defines their eventual SQL shapes, their statements must not be registered in Task 040 without callers. Reset uses full replacement, not the gameplay plot-reset statement.

## Prepared Statement Inventory

| Proposed statement | Immediate classification |
| --- | --- |
| `CHAR_SEL_PLAYER_FARM_STATE` | `USED_BY_T2_LOAD` |
| `CHAR_SEL_PLAYER_FARM_PLOTS` | `USED_BY_T2_LOAD` |
| `CHAR_REP_PLAYER_FARM_STATE` | `USED_BY_T2_SAVE`, `USED_BY_T2_RESET` |
| `CHAR_DEL_PLAYER_FARM_PLOTS` | `USED_BY_T2_SAVE`, `USED_BY_T2_RESET`, `USED_BY_CHARACTER_DELETE` |
| `CHAR_INS_PLAYER_FARM_PLOT` | `USED_BY_T2_SAVE` |
| `CHAR_DEL_PLAYER_FARM_STATE` | `USED_BY_CHARACTER_DELETE` |
| full plot mutation; watering; pests; state-only update | `DEFER_TO_GAMEPLAY_UNIT` |
| Andrew `CHAR_INS_PLAYER_FARM_STATE`, duplicate PLANT/RESET SQL, incomplete REPAIR | `DO_NOT_PORT` |

- **Contract:** plot insert SQL is `INSERT INTO player_farm_plots (guid, plot_id, state, seed_entry, needs_watering, has_pests, maturity_timestamp) VALUES (?, ?, ?, ?, ?, ?, ?)` with setters `UInt32, UInt8, UInt8, Null/UInt32, Bool, Bool, Null/UInt32` at indexes 0..6.
- **Contract:** delete SQL is respectively `DELETE FROM player_farm_plots WHERE guid = ?` and `DELETE FROM player_farm_state WHERE guid = ?`, each `setUInt32(0, guid)`.

## Failure Semantics

- **FACT:** `CommitTransaction` returns `void`, queues work, and only the SQL layer logs execution errors. Andrew's immediate success log was not a durability result. `AsyncCommitTransaction` returns `TransactionCallback`; `TransactionTask::Execute` returns `true` on commit and `false` after failure/deadlock retry exhaustion. `DirectCommitTransaction` also returns `void`.
- **FACT:** A failed REPLACE, delete, insert, duplicate key, or unavailable connection causes transaction execution failure/rollback as handled by the database layer; append order is preserved. Ordinary fire-and-forget callers cannot observe it.
- **Decision:** `Load` can return a synchronous status. Save/reset API must expose the existing callback completion boolean (or a future used by test code), never return “durable success” on enqueue. Log domain context on callback failure while `sql.sql` supplies the database error. No automatic speculative retry beyond the database layer's deadlock policy.

## Corruption Handling

- Invalid root phase/count: log error, ignore the whole persisted aggregate, return unpersisted defaults; never rewrite during load.
- State missing: return unpersisted defaults. Orphan plots are detected by the optional second query (or explicitly queried for diagnosis), logged, and ignored.
- No plots with state: valid persisted state plus empty collection.
- Plot ID/state invalid, duplicate, excess row, or maturity unrepresentable by `time_t`: log and skip only that row. Primary-key duplicates are normally impossible and indicate query/database anomaly.
- NULL seed/maturity: preserve absence. No cross-field crop normalization is allowed.
- Save-side invalidity: reject the entire operation before transaction construction. These are corruption/representation rules, not retail progression rules.

## Logging

- **Decision:** Use `TC_LOG_ERROR("sql.sql", ...)` for invalid persisted data, narrowing failures, orphan rows, and completion failure, prefixed `TillersFarmPersistence` and including GUID/plot where applicable. Do not log routine loads/saves at info level.
- **INFERENCE:** `sql.sql` matches database execution diagnostics and avoids inventing a configuration-dependent category. Optional debug-only summaries may be added later but are unnecessary for T2.

## Disposable DB Test Plan

Use a local disposable character database only. Apply T1, register/execute the real statements/service, and verify:

1. no rows → unpersisted `14/4/0`, zero plots, no writes;
2. explicit default state and advanced `0/16/nonzero-mask` state;
3. persisted state with zero plots and with exactly 16 ordered plots;
4. NULL seed/maturity and non-NULL boundary values round-trip distinctly;
5. save/load, modify/full-replacement save/load, including removal of old plots;
6. reset/load yields persisted `14/4/0` and zero plots;
7. hard-delete sentinel: create character, state and plots; execute reviewed deletion path/transaction; both farm tables empty for target and another GUID untouched;
8. invalid root values, plot IDs/states, orphan plots, and (where a synthetic result can bypass the PK) duplicates follow documented policy;
9. maturity `0`, `UINT32_MAX`, negative, and `UINT32_MAX+1`: valid boundaries round-trip and out-of-range saves enqueue nothing;
10. inject a failing statement/closed disposable connection if practical; callback reports false, transaction leaves old rows, and reset caller state is not published.

No client, gameplay, maps, vmaps, mmaps, phase, spawn, quest, or crop test is part of this matrix.

## Compile / Registration Boundary

- **Decision:** Task 040 needs `CharacterDatabase.h/.cpp` for the five immediately used T2A statements, a minimal game persistence DTO/service header/source, and its owning game target's existing source discovery/CMake entry if files are not globbed. T2B separately adds the state-delete statement and appends both prepared farm deletes in `Player::DeleteFromDB`.
- **FACT:** It does not require ScriptLoader, Pandaria script registration, WorldDatabase, Object changes, PlayerScript, WorldScript, zone hooks, phasing, commands, or Andrew scripts. Prepared registration at character database initialization is the only unavoidable startup effect.
- **INFERENCE:** A focused `game` then `worldserver` incremental build is the relevant future compile boundary; Task 039 itself is documentation-only and requires no compilation.

## Manager File Strategy

- **Decision:** **OPTION B:** write a preservation-native minimal persistence DTO/service from the final contract. Andrew's manager is tightly coupled to Player, quests, inventory, random gameplay, world data, spawns, phases, commands, unsafe cache references, and ad-hoc SQL; stripping it is riskier than expressing the small contract directly.
- **Rejected C:** DB-only statements would be callerless, repeating the dead-API problem Task 038 avoided. The service supplies immediate callers.

## T2 Subdivision

- **Decision:** Split T2. **T2A/Task 040** is prepared load/full-save/reset statements plus minimal inert value service. **T2B** is character hard-deletion statement integration and its orphan test (small but touches high-risk `Player::DeleteFromDB`). A later gameplay unit, not T2, chooses cache/hooks and narrow mutations.
- **INFERENCE:** This separation makes the main round-trip substrate easy to compile/recover while giving deletion integration its own review. T2A should not register `CHAR_DEL_PLAYER_FARM_STATE` unless it provides an actually called standalone erase test API; preferably defer that statement with T2B.

## Runtime Inertness

- **FACT:** A value service with no ScriptLoader, login/logout/zone/quest/world hook, soil/workstation script, command, spawn, or phase call has no player-facing entry point. If never invoked it performs no query or write.
- **FACT:** Only prepared SQL registration occurs at database initialization. Load never auto-saves, and there is no automatic row creation for logins.

## Build-18414 Independence

- **Decision:** No T2 decision requires client-build evidence. Schema symmetry, NULL preservation, range checks, transaction atomicity, deletion, and value lifetime are server-internal.
- **Decision:** Quest progression, phase masks, visuals, plot materialization, crop state semantics, and farm-state-to-unlock correspondence are explicitly outside T2 and require separate evidence before gameplay implementation.

## Known Defect Resolution Map

| Defect | Classification | Resolution |
| --- | --- | --- |
| broken state REPLACE bind order | `FIX_IN_T2` | GUID-first typed contract |
| dead INSERT references `last_growth_tick` | `NO_LONGER_APPLICABLE` | do not port statement/column |
| NULL collapsed to zero | `FIX_IN_T2` | optionals + `IsNull`/`setNull` |
| incomplete REPAIR retained watering/pests/maturity | `DEFER_TO_T3+` | eventual canonical full mutation; no repair API in T2 |
| reset only changed memory | `FIX_IN_T2` | observable atomic root upsert + plot delete |
| character deletion orphaned farm rows | `FIX_IN_T2` (T2B) | append two deletes to final hard-delete transaction |
| striped-lock references escaped | `FIX_IN_T2` | stateless value service; no internal references |
| cache initialization unsynchronized | `NO_LONGER_APPLICABLE` | no T2 cache |
| wrong DB in plot-position lookup | `EXCLUDED_FROM_T2` | all world-position loading excluded |
| stale online growth/no timer | `DEFER_TO_T3+` | gameplay lifecycle, not persistence |
| empty plot map prevented state save | `FIX_IN_T2` | state plus zero plots is valid full save |
| unchecked plot state/time narrowing | `FIX_IN_T2` | explicit validation before transaction |
| immediate “saved” log after queueing | `FIX_IN_T2` | callback truth; no normal success log |
| gameplay/spawn mixed with persistence | `FIX_IN_T2` | preservation-native isolated service |

## Canonical T2 Contract

- **DATA TYPES:** minimal `FarmState:uint8`, `FarmPlotState:uint8`, state DTO, plot DTO with optional seed/maturity, ordered plot map, aggregate with explicit persisted/root-presence status; max 16.
- **OWNERSHIP:** service stateless; aggregates caller-owned; GUID-low key; no Player/world pointers or cache.
- **LOAD API:** synchronous `Load(uint32 guid) -> result/status + PlayerFarmData`; two exact prepared SELECTs; read NULL explicitly; never writes/materializes rows.
- **SAVE API:** validate complete caller snapshot, then async atomic state upsert → plot delete → complete inserts; permit zero plots; expose commit boolean asynchronously.
- **RESET API:** async atomic upsert `14/4/0` → delete plots; no inserts; caller publishes default only after successful completion.
- **DELETE API:** append plots/state deletes to established final character deletion transaction; no cascade. Standalone erase, if provided for tests, has identical order and observable completion.
- **DB STATEMENTS:** T2A uses state/plot SELECT, state REPLACE, plot delete and plot insert. T2B uses state delete and reuses plot delete. Narrow gameplay statements are deferred.
- **NULL RULES:** `optional`; `Field::IsNull`; `setNull`; NULL round-trips as NULL.
- **RANGE RULES:** IDs 0..15, plot state 0..7, at most 16 unique plots; known root domains; checked maturity 0..`UINT32_MAX`; reject invalid save snapshot.
- **TRANSACTION RULES:** full replacement and reset are each one character transaction; validate before append; never split state/plots; append order is deterministic.
- **LOCKING RULES:** no shared mutable storage, no escaped references, no service mutex. Future gameplay ownership is a separate review.
- **ERROR RULES:** conservative load fallback/row skipping without repair; save/reset completion uses real callback boolean; failed reset cannot update caller cache; SQL errors remain logged by database layer.
- **LOGGING:** `sql.sql`, contextual errors only, no routine chatter or premature success.
- **NO-RUNTIME-HOOK RULE:** no automatic invocation, row creation, Player hook, script, world access, spawn, phase, quest, crop, or command.

## Task 040 Implementation Boundary

- **Decision:** `T2A_DB_API_PLUS_INERT_SERVICE`.
- Implement the five immediately used T2A statements, minimal DTO/service, synchronous value load, validated callback-observable full save/reset, and focused compile/disposable round-trip tests. Do not add the character deletion statement/hook until T2B, and do not add gameplay narrow writes.
- **INFERENCE:** This is the smallest boundary with real callers for every registered statement, a compile and DB-test surface, no runtime hook, no global cache, and no dead API.

## Final Classification

**READY_FOR_SPLIT_T2A**

- **FACT:** T1 is exact; data, load/result ordering, full-save transaction, NULL handling, range policy, default/no-row semantics, reset, deletion integration, ownership, failure semantics, logging, and tests are bounded.
- **FACT:** Gameplay/world data and build-18414-dependent behavior are excluded. Task 040 has an explicit inert implementation boundary; character deletion remains a separately bounded T2B review rather than an architecture blocker.
