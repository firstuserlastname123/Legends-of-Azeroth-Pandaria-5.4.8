# Task 040: Tillers T2A Inert Persistence Implementation

## Starting State

- **FACT:** Work began on clean branch `work` at `b1141b42ee91a674b3b82a653f36d7a4600b1c24`, which merges Task 039 (`659c564`) after Task 038 (`cd5ee92`).
- **FACT:** The T1 migration existed and repository searches found no Tillers persistence service, farm prepared statement, or production farm-table caller.

## Task 039 Contract

- **FACT:** Task 039 classified the work `READY_FOR_SPLIT_T2A` and prescribed a stateless value service, two synchronous reads, atomic asynchronous full replacement/reset, explicit SQL NULL preservation, no gameplay caller, and T2B deletion deferral.
- **FACT:** Task 040 implements only that T2A boundary and does not import Andrew's manager.

## Database API Findings

- **FACT:** `CharacterDatabase.Query(prepared)` is the blocking prepared-query API and requires `CONNECTION_SYNCH`; `PreparedQueryResult` is `std::shared_ptr<PreparedResultSet>`, and a null pointer represents no result. This API does not distinguish “no rows” from query failure to its caller.
- **FACT:** `Field::IsNull()` directly tests the stored result pointer. `PreparedStatementBase::setNull(uint8)` binds SQL NULL.
- **FACT:** `BeginTransaction()` returns a shared `Transaction<CharacterDatabaseConnection>`; `Append` transfers prepared statements into an ordered operation vector.
- **FACT:** `AsyncCommitTransaction` posts `TransactionTask::Execute` on the database pool's Boost.Asio executor and returns `TransactionCallback` holding `std::future<bool>`. The database worker produces `true` only after commit and `false` after transaction failure/deadlock retry exhaustion and rollback handling.
- **FACT:** `TransactionCallback::AfterComplete(std::function<void(bool)>)` installs a value-safe completion handler; `InvokeIfReady()` runs that handler on whichever caller thread polls it, not on the DB worker. Callers must retain and poll/register the returned callback.
- **INFERENCE:** Since the service returns the repository callback and captures nothing, later callers can safely capture value data while avoiding `Player*` lifetime coupling.

## CharacterDatabase Statements

- **FACT:** Exactly five enum entries and preparations were added: `CHAR_SEL_PLAYER_FARM_STATE`, `CHAR_SEL_PLAYER_FARM_PLOTS`, `CHAR_REP_PLAYER_FARM_STATE`, `CHAR_DEL_PLAYER_FARM_PLOTS`, and `CHAR_INS_PLAYER_FARM_PLOT`.
- **FACT:** Both SELECTs use `CONNECTION_SYNCH`; all three writes use `CONNECTION_ASYNC`. SQL column order and parameter order exactly match the T1/Task 039 contract.
- **FACT:** No state DELETE, obsolete state INSERT, or narrow gameplay mutation was added.

## Source Location

- **FACT:** The service is in `src/server/game/Tillers/`, a game-core-owned directory with no script or Player dependency.
- **INFERENCE:** A dedicated directory keeps this reusable persistence boundary visible without inventing a module or registering a script.

## Persistence DTOs

- **FACT:** `FarmState : uint8` defines persisted values 0, 8, 12, and 14. `FarmPlotState : uint8` defines values 0 through 7. `MaxFarmPlots` is 16.
- **FACT:** `PlayerFarmState` defaults to 14/4/0. `FarmPlotData` contains plot ID/state, optional `uint32` seed, two booleans, and optional `time_t` maturity.
- **FACT:** `PlayerFarmData` contains explicit `FarmLoadStatus`, state, and deterministic `std::map<uint8, FarmPlotData>` plots. All returned data is caller-owned.

## Load API

- **FACT:** `static PlayerFarmData Load(uint32 guidLow)` performs synchronous prepared queries and returns a value.
- **FACT:** Missing state returns `NotPersisted` defaults 14/4/0 with no plots and performs no plots query or write. This intentionally ignores orphan plots.
- **FACT:** A valid state sets `Persisted`; no plot rows remains a valid persisted empty map. Invalid root returns default values with `InvalidRoot`.
- **UNKNOWN:** The current synchronous query API cannot separately expose query failure versus no rows; Task 040 does not invent an error signal.

## Load Validation

- **FACT:** Root phase must be 0/8/12/14 and unlocked count 4/8/12/16. Invalid root is logged and never repaired.
- **FACT:** Each row checks ID 0..15, state 0..7, raw booleans 0..1, duplicate ID, capacity 16, and maturity representability. A bad plot is logged/skipped while valid plots remain.

## NULL Handling

- **FACT:** Load calls `IsNull()` before both nullable numeric getters. SQL NULL becomes `std::nullopt`; present values remain engaged even when numerically zero.
- **FACT:** Save calls `setNull()` for absent seed/maturity and typed `setUInt32()` for present values.

## Save Validation

- **FACT:** `Save` validates the whole snapshot before beginning a transaction: root domains, at most 16 plots, map key/plot ID equality, ID/state ranges, and maturity 0..`UINT32_MAX`.
- **FACT:** `std::map` enforces unique keys. No gameplay correspondence or crop semantics are inferred.
- **FACT:** Invalid input returns `FarmWriteResult{accepted=false, no completion}` and queues nothing.

## Full Save Transaction

- **FACT:** A valid save appends one state REPLACE, one plot DELETE, then zero through sixteen ordered plot INSERTs.
- **FACT:** State binding is GUID, phase, unlocked count, friend mask at indexes 0..3. Plot binding is GUID, ID, state, optional seed, watering, pests, optional maturity at indexes 0..6.
- **FACT:** Zero plots still replaces the root and deletes old rows atomically.

## Async Completion Semantics

- **FACT:** `Save` and `Reset` return `FarmWriteResult`, whose `accepted` reports only validation/submission acceptance and whose `completion` is the actual `TransactionCallback` commit result.
- **FACT:** Durable success is observable only from the callback's boolean after `InvokeIfReady`; enqueue is not reported as durable success.
- **FACT:** The service captures no object or player pointer and maintains no cache.

## Reset

- **FACT:** `Reset(uint32)` constructs one transaction containing state REPLACE 14/4/0 followed by plot DELETE, with no INSERTs.

## Logging

- **FACT:** Invalid roots, malformed loaded plots, and invalid save snapshots use `TC_LOG_ERROR("sql.sql", ...)` with `TillersFarmPersistence` and GUID/plot context. Routine success is silent.
- **FACT:** Database execution errors remain logged by the database layer; the returned bool lets the eventual caller add domain-specific failure handling without a service-owned callback processor.

## Deferred T2B Deletion

- **FACT:** `CHAR_DEL_PLAYER_FARM_STATE`, `Player::DeleteFromDB` integration, and all hard-deletion behavior remain deferred to T2B. `CHAR_DEL_PLAYER_FARM_PLOTS` exists only for full save/reset.

## Runtime Inertness

- **FACT:** Repository-wide searches found the service only in its declaration/definition and this report. No login, logout, save, zone, quest, world, script-loader, or command caller was added.
- **FACT:** Prepared statement registration is the only normal initialization effect. With no production invocation, the service performs no query or write.

## Build Registration

- **FACT:** `src/server/game/CMakeLists.txt` now globs/groups `Tillers/*.cpp` and `Tillers/*.h`, adds them to `game`, and exports the directory as a game include path. No script target/loader changed.

## Exact Object Compile

- **FACT:** The generated Make rules compiled the exact `CharacterDatabase.cpp.o` and `TillersFarmPersistence.cpp.o` successfully with GCC 13.3.0 after one diagnosed type-name correction.

## Game Build

- **FACT:** `cmake --build /tmp/mop-preservation-dev --target game --parallel 4` was attempted once from a newly configured cold build tree.
- **FACT:** The cold build progressed through dependencies, `common`, `database`, `shared`, and into game sources (38%) without compiler/linker error before the bounded worker execution window was ended. Result: `AGGREGATE_VALIDATION_DEFERRED_ENVIRONMENT_LIMIT`; completed artifacts remain in the persistent tree.

## Worldserver Build

- **FACT:** Worldserver was not attempted because the game target did not complete in the same worker window. This follows the task gate and is an environment limitation, not a link failure.

## Disposable Database

- **FACT:** A local socket-only disposable MariaDB 10.11.14 instance used `/tmp/mop-task040-mariadb`, socket `/tmp/mop-task040.sock`, and database `mop_task040_characters`.
- **FACT:** The actual T1 migration was applied unchanged. No user/production database was used.

## SQL Contract Tests

- **FACT:** MariaDB PREPARE/EXECUTE exercised the exact five SQL strings and binding order.
- **FACT:** Default 14/4/0 and advanced 0/16/65535 roots passed; NULL and non-NULL seed/maturity (including `UINT32_MAX`) read distinctly; zero and sixteen plots passed; delete-before-reinsert replacement removed all old plots.

## Service Harness

- **FACT:** Repository inspection found no core unit/integration framework or lightweight executable for initializing the full CharacterDatabase pool and game library.
- **INFERENCE:** Creating a permanent command/hook or a large standalone linker scaffold would violate the bounded/inert test strategy. Direct service execution is therefore `DEFERRED_NO_EXISTING_HARNESS`.

## Service Load Tests

- **UNKNOWN:** Direct service-level DB execution was deferred. Exact-object compilation, SQL execution, and the static matrix validate the available layers independently.

## Service Save Tests

- **UNKNOWN:** Direct service-level save execution was deferred for lack of a lightweight harness. Prepared SQL replacement semantics passed on the disposable database.

## Service Reset Tests

- **UNKNOWN:** Direct service-level reset execution was deferred. The static transaction is exactly REPLACE 14/4/0 then DELETE, and both SQL operations passed independently.

## Invalid-Save Tests

- **UNKNOWN:** Runtime invalid DTO calls were deferred with the harness. Source inspection confirms ID 16, state 8, mismatched key, negative maturity, and maturity above `UINT32_MAX` return before `BeginTransaction()`.

## Async Failure Test

- **UNKNOWN:** `ASYNC_FAILURE_INJECTION_NOT_PRACTICAL` without a service harness. The verified database implementation returns `TransactionTask::Execute`'s durable boolean through the future/callback; no fake success path was introduced.

## Static Load Matrix

- **FACT:** No root gives unpersisted default and no child query; valid root/no plots gives persisted empty; NULL and non-NULL optionals are preserved; 16 ordered rows fit.
- **FACT:** Invalid phase/count gives `InvalidRoot`; bad ID/state/time, excess rows, and duplicate rows are skipped/logged. Orphans are ignored because child query is not issued without a valid root. No load branch writes.

## Static Save Matrix

- **FACT:** Default or advanced roots with zero plots, one plot, 16 plots, and independently nullable seed/maturity are accepted.
- **FACT:** Invalid ID/state, key mismatch, excess count, or out-of-range time is rejected before transaction creation. Every valid save binds GUID at state index 0, always deletes prior plots, and inserts the complete ordered snapshot atomically, regardless of old row count.

## Static Reset Matrix

- **FACT:** Every reset appends only root REPLACE 14/4/0 and plot DELETE. It creates no default plots and touches no gameplay/cache state.

## Final Changed Files

- `src/server/database/Database/Implementation/CharacterDatabase.h`
- `src/server/database/Database/Implementation/CharacterDatabase.cpp`
- `src/server/game/CMakeLists.txt`
- `src/server/game/Tillers/TillersFarmPersistence.h`
- `src/server/game/Tillers/TillersFarmPersistence.cpp`
- `docs/preservation/backports/TASK040_TILLERS_T2A_PERSISTENCE_IMPLEMENTATION.md`

## Remaining T2B Work

- **FACT:** T2B must add `CHAR_DEL_PLAYER_FARM_STATE`, reuse plot deletion, and integrate plots-first/state-second cleanup into final character hard deletion. It must be reviewed separately.

## Final Classification

- **FACT:** `IMPLEMENTATION_COMPILES_RUNTIME_DB_TEST_DEFERRED` because exact objects and SQL contracts pass, while actual service-level database execution is deferred for lack of an existing lightweight harness. Gameplay remains runtime-inert.
