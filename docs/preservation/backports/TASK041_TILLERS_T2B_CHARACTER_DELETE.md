# Task 041: Tillers T2B Character Hard-Deletion Cleanup

## Starting State

- **FACT:** Work began on clean branch `work` at `2aa903836e8d721b18164b6e53ffe471191293b6`, the merge of Task 040.
- **FACT:** The T1 migration and T2A service were present. The five T2A statements existed, `CHAR_DEL_PLAYER_FARM_STATE` did not exist, and `Player::DeleteFromDB` did not reference either farm table.

## Task 039 / 040 Contract

- **FACT:** Task 039 requires plot-then-state deletion in the established final character-delete transaction, without a foreign-key cascade, independent transaction, or gameplay hook.
- **FACT:** Task 040 owns the existing plot delete and inert Load/Save/Reset service; this task reuses that statement and does not alter the service.

## Character Deletion Architecture

- **FACT:** `Player::DeleteFromDB(ObjectGuid, uint32, bool, bool)` selects `CHAR_DELETE_REMOVE` or `CHAR_DELETE_UNLINK`. `deleteFinally=true` forces remove; otherwise configuration selects the mode, with configured minimum-level handling able to force remove.
- **FACT:** The normal character-list handler calls the method with defaults. The erase command and deleted-character purge call it with `deleteFinally=true`. Account deletion calls it with `deleteFinally=false` after logging out a connected player.
- **FACT:** Repository-wide inspection found `CHAR_DEL_CHARACTER` prepared and appended only in the remove branch of `Player::DeleteFromDB`; no independent production SQL path directly deletes a character row.

## Soft Delete Path

- **FACT:** `CHAR_DELETE_UNLINK` executes only `CHAR_UPD_DELETE_INFO`, which saves the prior name/account, clears live name/account, and sets `deleteDate`. It never enters the remove transaction and therefore preserves both farm tables.
- **FACT:** The deleted-character restore command reverses that metadata update. Retaining auxiliary rows makes the soft deletion reversible.

## Final Hard Delete Path

- **FACT:** The `CHAR_DELETE_REMOVE` case creates the large `CharacterDatabaseTransaction`, appends character-owned cleanup, and calls `CharacterDatabase.CommitTransaction(trans)` once.
- **FACT:** `CommitTransaction` posts the transaction to the asynchronous database executor and returns no completion handle; final deletion is fire-and-forget at this call site.
- **FACT:** Control flow is: `SOFT DELETE -> CHAR_UPD_DELETE_INFO -> Execute`; `FINAL DELETE -> BeginTransaction -> existing deletes -> farm plots delete -> farm state delete -> corpse deletes -> CommitTransaction`.

## Old Character Cleanup

- **FACT:** Both startup and non-startup branches of `DeleteOldCharacters` select expired soft-deleted rows and call `Player::DeleteFromDB(..., true, true)`. They are covered by the same forced final-delete branch.

## Other Hard-Delete Entry Points

- **FACT:** Character erase, deleted-character explicit purge, and retention cleanup are `COVERED_BY_PLAYER_DELETE` with `deleteFinally=true`.
- **FACT:** Normal client deletion and account deletion are `COVERED_BY_PLAYER_DELETE`, but intentionally honor configured remove/unlink semantics because they pass `deleteFinally=false`.
- **FACT:** Restoration is `NOT_RELEVANT`: it updates a soft-deleted character and performs no hard delete.
- **FACT:** No `ADDITIONAL_HARD_DELETE_PATH` was found; the only `CHAR_DEL_CHARACTER` production use is the central remove transaction.

## Character GUID Contract

- **FACT:** `ObjectGuid::LowType` is `uint32`. `DeleteFromDB` assigns `playerguid.GetCounter()` to that type, and neighboring character cleanup statements bind it with `setUInt32(0, guid)`.
- **FACT:** Both T1 tables define `guid` as unsigned 32-bit SQL `INT`; no full GUID conversion, account ID, realm ID, or pointer value is required.

## CharacterDatabase Statement

- **FACT:** This task adds only `CHAR_DEL_PLAYER_FARM_STATE`, registered on `CONNECTION_ASYNC` as exactly `DELETE FROM player_farm_state WHERE guid = ?`.

## Plot-Delete Reuse

- **FACT:** Final deletion reuses the existing `CHAR_DEL_PLAYER_FARM_PLOTS` statement. T2A Save and Reset still use the same statement unchanged.

## Transaction Integration

- **FACT:** The remove branch appends `CHAR_DEL_PLAYER_FARM_PLOTS` first and `CHAR_DEL_PLAYER_FARM_STATE` second, binding parameter 0 of each to the function's low GUID.
- **FACT:** Both are part of the already-open character transaction and occur before its single existing commit. No standalone Tillers transaction or service call was added.

## Exactly-Once Append

- **FACT:** Each farm statement appears once in the straight-line remove-branch append sequence, outside all mail, pet, and social loops. The unlink branch bypasses them.
- **INFERENCE:** Database-layer transaction retry may re-execute the transaction, but source construction appends each operation exactly once per invocation.

## T2A Non-Interaction

- **FACT:** No T2A header/source, DTO, validation, NULL handling, async completion, CMake registration, Load, Save, or Reset behavior changed.
- **FACT:** No `TillersFarmPersistence::DeleteCharacter` API or persistence-service call was added.

## Exact Object Compile

- **FACT:** GCC 13.3.0 compiled the exact generated objects for `CharacterDatabase.cpp` and `Player.cpp` successfully in `/tmp/mop-preservation-dev`.

## Game Build

- **FACT:** `cmake --build /tmp/mop-preservation-dev --target game --parallel 4` was attempted once from the cold tree. It completed dependencies through `common`, `database`, and `shared`, then reached 35% while compiling game sources without a compiler/linker error before the bounded cloud work window ended: `AGGREGATE_VALIDATION_DEFERRED_ENVIRONMENT_LIMIT`.

## Worldserver Build

- **FACT:** Worldserver was not attempted because the game target did not finish cheaply in the same worker. This follows the task gate and is not a link failure.

## Disposable Database

- **FACT:** A socket-only disposable MariaDB 10.11.14 instance used `/tmp/mop-task041-mariadb`, socket `/tmp/mop-task041.sock`, and database `mop_task041_characters`.
- **FACT:** The unchanged T1 migration `sql/updates/characters/2026_09_22_00_characters_tillers_farm.sql` created the tested tables. No configured user database was touched.

## Delete Isolation Test

- **FACT:** GUID 4101 began with one root and three plots; GUID 4102 began with one root and two plots. MariaDB prepared and executed the exact plots delete followed by the exact state delete for 4101.
- **FACT:** The result was 0 plots/0 roots for 4101 and unchanged 2 plots/1 root for 4102.

## Transaction Atomicity Test

- **FACT:** Executing both prepared deletes in one InnoDB transaction and committing removed the target's root and plots while leaving the other GUID unchanged.
- **FACT:** Failure injection issued the plot delete and then deliberately referenced a nonexistent table before rollback. After rollback, both the target root and both target plots remained, as did all other-GUID rows.

## Runtime Delete Harness

- **UNKNOWN:** `DIRECT_PLAYER_DELETE_RUNTIME_TEST_DEFERRED_NO_HARNESS`. Repository inspection found no lightweight test that initializes the full world/game/database context needed to invoke `Player::DeleteFromDB`; no permanent command or test-only production API was introduced.

## Soft-Delete Static Matrix

- **FACT:** Configured unlink, including the ordinary client/account call sites when unlink applies, runs only the reversible metadata update and preserves root and plot rows.
- **FACT:** Restoration updates name/account/delete metadata and can therefore recover the character with its farm data intact.

## Final-Delete Static Matrix

- **FACT:** Normal remove mode and minimum-level-forced remove append both farm deletes once.
- **FACT:** Forced erase, explicit deleted-character purge, and both retention-cleanup branches append both once through `deleteFinally=true`.
- **FACT:** Offline character handling reaches the same method; online account characters are logged out before it. No farm operation depends on a `Player*`.
- **FACT:** A nonexistent GUID passed with forced final deletion still builds idempotent GUID-scoped deletes; zero matching rows is harmless.
- **FACT:** Both statements share the existing transaction's commit and retry behavior and have no separate commit.

## Cross-GUID Safety

- **FACT:** Both new append blocks bind the local `guid` low counter with `setUInt32(0, guid)`. The disposable test proved rows for GUID 4102 remain unchanged when GUID 4101 is deleted.

## Orphan Prevention Invariant

- **FACT:** After a successful final transaction, counts in both farm tables for the deleted low GUID are zero, while other GUIDs remain unchanged. Soft unlink deliberately does not promise this invariant.

## Runtime Scope

- **FACT:** `CHAR_DEL_PLAYER_FARM_STATE` has production references only in statement registration and final character deletion. Existing plot-delete production references are T2A Save/Reset and final deletion.
- **FACT:** No login/logout, gameplay, script, command, cache, phasing, quest, soil, crop, world-data, Playerbots, or Eluna path was added.

## Final Changed Files

- `src/server/database/Database/Implementation/CharacterDatabase.h`
- `src/server/database/Database/Implementation/CharacterDatabase.cpp`
- `src/server/game/Entities/Player/Player.cpp`
- `docs/preservation/backports/TASK041_TILLERS_T2B_CHARACTER_DELETE.md`

## Remaining T2 Work

- **FACT:** T2A persistence and T2B hard-deletion cleanup are implemented. Direct full-core runtime execution remains deferred; gameplay lifecycle/cache/mutation work is explicitly outside T2 and requires a separately evidenced task.

## Final Classification

- **FACT:** `IMPLEMENTATION_COMPILES_RUNTIME_DELETE_TEST_DEFERRED` because exact objects and exact SQL/transaction behavior pass, while direct `Player::DeleteFromDB` execution is deferred for lack of a lightweight integration harness.
