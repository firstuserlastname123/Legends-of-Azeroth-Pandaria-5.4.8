# Task 026: validated battle-pet trainer data substrate (A2A)

## Starting state

Task 026 started from clean commit
`f3cd970720a7acd920452aa8b9e182bbf10f92cc` on branch `work`. That squashed
Task 025 commit contains both its report and the central `EndBattle` guard.
Task 021 and Tasks 022--024 are also present in the ancestry.

Contrary to the workflow expectation, the continued cloud environment did not
contain `/tmp/mop-preservation-dev/CMakeCache.txt`; it reported
`DEV_BUILD_TREE_MISSING`. ccache also reported 0.0 GiB used of 5.0 GiB. Nothing
was cleaned or cleared by this task. If implementation passes the gate, the
development tree must therefore be configured once rather than reused, then
preserved.

## Andrew A2 ancestry and final state

A fresh temporary no-checkout clone was made at `/tmp/andrew-task026` because
the Task 025 temporary clone was absent. No preservation remote was changed.
Repository history shows exactly one commit modifying
`BattlePetTrainerMgr.h`/`.cpp`:

* Candidate 10,
  `39205279cebebd1d2c83000a0e9043e68047e659` (parent
  `99801947ccebbe1653ba6d3aa444559bc3efddd2`), introduced the manager, schema,
  runtime trainer-pet factory, startup call, and trainer battle integration.

The manager files and schema are unchanged at Andrew final commit
`bd8ad5515418d94abd35ba7bf71430979243a09c`. Later commits add content/spawns
and modify runtime battles, not the A2 loader/API. This task uses the final
files only as provenance and adapts rather than cherry-picks Candidate 10.

## A2A / A2B split

### A2A: allowed inert data substrate

* value-only `TrainerBattlePet` records;
* a manager-owned entry-to-team container;
* world-database schema and startup loading;
* validation, logging, deterministic retrieval, and a read-only lookup API.

### A2B: explicitly deferred runtime ownership

* `BattlePet` construction and `CreateTrainerPets`;
* `PetBattleTeam::AddTrainerBattlePets` and ownership/destruction;
* retained trainer `Creature*` state;
* PetBattle constructor/type integration and teardown;
* trainer Creature despawn/map lifetime and concurrent challengers.

Andrew's loader/factory can be separated cleanly: A2A needs no `PetBattle`,
`PetBattleTeam`, `BattlePet`, `Player`, or `Creature` object. Only numeric
definitions are stored. No current code has a lookup or initiation path for the
new manager, so A2B can remain wholly absent.

## Exact schema and update destination

Andrew's final world schema is exactly:

```sql
CREATE TABLE IF NOT EXISTS `battle_pet_trainer` (
    `entry` MEDIUMINT(8) UNSIGNED NOT NULL COMMENT 'NPC entry of the trainer',
    `species` INT(10) UNSIGNED NOT NULL COMMENT 'Battle pet species ID (BattlePetSpecies.db2)',
    `level` TINYINT(3) UNSIGNED NOT NULL DEFAULT 1,
    `quality` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
    `breed` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`entry`, `species`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
```

There are no secondary indexes, nullable columns, or trainer rows in this
definition. The composite primary key prevents the same species from appearing
twice for one trainer, but permits more than three distinct species.

No equivalent table exists in preservation. The repository places incremental
world updates in `sql/updates/world/` using date/sequence names. The new empty
schema belongs in `sql/updates/world/2026_09_21_00_world.sql`. It will contain
only this `CREATE TABLE IF NOT EXISTS`; no content, spawn, quest, gossip,
destructive statement, or row modification is permitted.

## Empty-schema safety

`WorldDatabase.Query` returns an empty result for a table with no rows. The
loader clears its value container, logs zero definitions, and returns normally.
There is no assertion and startup continues. The manager has no player/session,
gossip, opcode, PetBattle, or PetBattleTeam consumer in A2A. Therefore an empty
table changes no wild battle, account pet, packet, creature interaction, or
current database row, and no trainer content row is needed.

## Startup ordering

Client DBC/DB2 stores, including `BattlePetSpecies`, `BattlePetBreedState`, and
`BattlePetBreedQuality`, are loaded before `World::SetInitialWorldSettings`.
During world content initialization, creature templates load substantially
earlier than battle-pet SQL data. The existing order near the selected load
point is:

1. creature templates (and subsequent creature content);
2. scripts and other ObjectMgr content;
3. `LoadBattlePetBreedData`;
4. `LoadBattlePetQualityData`;
5. `LoadBattlePetItemToSpeciesData`;
6. `BattlePetSpawnMgr::Initialise`;
7. proposed `BattlePetTrainerMgr::LoadBattleTrainerPets`.

At this point `GetCreatureTemplate`, `sBattlePetSpeciesStore`, the global breed
set derived from `BattlePetBreedState.db2`, the breed-quality DB2 store, and
item quality constants are all available. This is Andrew's relative location
and is safe for the stronger validations; it is not copied before its backing
stores.

## Validation policy

Validation is team-atomic: rows are first grouped by entry in deterministic
`entry, species` query order. If any row is malformed or the team exceeds the
runtime maximum, the entire entry is rejected with actionable `sql.sql`
logging. A partially valid team is never published.

### Trainer entry

`entry` represents the NPC trainer CreatureTemplate in Andrew's schema and
runtime lookup. `sObjectMgr->GetCreatureTemplate(entry)` must succeed. Missing
entries reject the entire definition; no world object pointer is retained.

### Species

The authoritative current store is `sBattlePetSpeciesStore`. A row is valid
only when `LookupEntry(species)` succeeds. Because the future `BattlePet`
initializer also dereferences the species' `NpcId` CreatureTemplate, that
template must exist as a compatibility prerequisite. Either failure rejects
the team rather than allowing a future assertion/null dereference.

### Level

`BATTLE_PET_MAX_LEVEL` is 25. Unlike the current account loader, which fails to
reject zero, trainer data must satisfy `1 <= level <= BATTLE_PET_MAX_LEVEL`.
The check occurs on the database integer before narrowing to `uint8`.

### Quality

Current account-pet and `battle_pet_quality` loaders accept numeric qualities
through `ITEM_QUALITY_LEGENDARY` (5), inclusive. `BattlePet::InitialiseStates`
indexes `sBattlePetBreedQualityStore` at `7 + quality` and asserts the result.
Trainer quality must therefore be at most Legendary and the exact DB2 entry
`7 + quality` must exist before it is narrowed/stored. Poor through Legendary
are only admitted when backed by that DB2 row; Artifact/Heirloom and larger
values are rejected. This establishes a safe bound without inventing retail
trainer-team rarity rules.

### Breed

Breed IDs are sparse: `sBattlePetBreedSet` is derived from actual
`BattlePetBreedState.db2` records, so no numeric interval is invented. Current
account-pet loading treats zero as a special allowed value, and a zero breed is
safe in `InitialiseStates` (it simply matches no breed modifier). Trainer breed
must be zero or a member of the authoritative global set.

The current ObjectMgr also loads species-to-breed SQL associations but exposes
no read-only membership API. Whether a globally valid breed is retail-correct
for a particular trainer species is therefore `DEFERRED_TO_A2B`. That does not
make A2A unsafe: A2A allocates no BattlePet, all retained values are safe for
the existing constructor, and A2B must decide species/team semantics before it
is allowed to consume them.

### Team size and duplicates

`PET_BATTLE_MAX_TEAM_PETS` is 3. Zero rows means no definition; one, two, or
three valid rows produce a definition. Four or more rows reject the entire
trainer team rather than silently truncating it. The SQL primary key prevents
duplicate `(entry, species)` rows at the database layer; the loader nevertheless
does not rely on duplicate rows to establish cardinality safety.

## Ordering contract

Andrew queried without `ORDER BY` and later selected vector element zero as the
active pet, so its slot/presentation behavior was database-order dependent.
The A2A query will use `ORDER BY entry, species`, yielding deterministic
internal storage by species ID. This is not asserted to be retail slot order
and has no gameplay effect while A2B is absent. A2B remains blocked on an
evidence-backed slot/active-pet ordering contract; Task 026 does not add an
invented slot column.

## Ownership, API, reload and lifetime

The manager value-owns `std::map<uint32, std::vector<TrainerBattlePet>>`.
`TrainerBattlePet` contains only `uint32 Species` and three `uint8` values.
`GetTrainerTeam(uint32 entry) const` returns a pointer-to-const vector or null;
there is no mutable accessor or runtime factory. No world-object pointer is
stored.

Loading is once during startup. The loader builds a temporary container and
swaps it into place after validation, so an empty table is a valid empty state
and a future direct reload replaces the complete container at once. No GM reload
command is added. Because A2B is absent, nothing retains pointers into manager storage;
future reload support must reconsider reference lifetime once consumers exist.

## Current-behavior isolation

The proposed files do not modify `PetBattle.cpp`, `PetBattle.h`,
`BattlePetMgr.cpp`, handlers, gossip, opcodes, achievements, or Playerbots.
Task 021 GUID reconstruction and Task 025 finalization remain byte-for-byte
unchanged. The only call loads inert data at startup. Nothing queries the
manager after loading; consequently no wild battle/account pet changes, no
gossip or packet becomes reachable, no trainer Creature becomes interactable,
and no PetBattle can consume a trainer definition.

## Decision and planned validation

**Classification: `SAFE_INERT_SUBSTRATE`.** The final schema/update path are
known; empty startup is safe; required stores are initialized; entry, species,
level, quality, breed safety, and team cardinality can be validated without
inventing retail behavior; no player path is exposed; and A2B is cleanly
excluded.

The implementation will add only the value manager `.h/.cpp`, one World include
and startup call, one empty-schema world migration, and this report. Andrew's
factory and runtime includes will be omitted, while validation, deterministic
ordering, and team-atomic publication will be added. The intended checks are
SQL/source diff review, `git diff --check`, `game`, and `worldserver`. Since the
expected build tree is unexpectedly absent, it will be configured once using
the Task 025 fast profile and preserved; no clean build directory operation is
performed.

## Implementation and validation result

The final classification remains **`SAFE_INERT_SUBSTRATE`**. Source and SQL
changed. The implementation consists of:

* `src/server/game/BattlePet/BattlePetTrainerMgr.h`: value definition,
  value-owned container, singleton, load method, and const lookup;
* `src/server/game/BattlePet/BattlePetTrainerMgr.cpp`: deterministic query,
  team-atomic validation/publication, safe empty result, and logging;
* `src/server/game/World/World.cpp`: one include and one startup load after the
  existing battle-pet data/spawn loaders;
* `sql/updates/world/2026_09_21_00_world.sql`: the empty table schema only; and
* this report.

Compared with Andrew Candidate 10, the runtime `CreateTrainerPets` factory,
`BattlePet` allocations, `PetBattleTeam` dependency, and mutable runtime
integration are omitted. The adapted loader adds `ORDER BY entry, species`,
temporary-container publication, CreatureTemplate entry validation, species
and species-creature validation, nonzero level bounds, quality bounds plus the
exact breed-quality DB2 lookup, sparse breed validation, and whole-team
rejection for any invalid row or more than three pets.

Post-build static review confirms:

* an empty query swaps in an empty container and returns normally;
* invalid entry, species, species creature, level, quality, or breed marks the
  whole trainer entry invalid and prevents publication;
* a fourth distinct pet rejects the whole definition;
* accepted numeric values are narrowed only after validation;
* the manager stores only values and allocates no `BattlePet`;
* it retains no `Creature*`, player, battle, or team pointer;
* repository references to the manager are limited to its implementation and
  the startup load—no player/session/gameplay path consumes it; and
* `PetBattle.cpp`, including Task 025's finalization guard, is unchanged from
  the starting commit.

Both requested targets passed:

* `cmake --build /tmp/mop-preservation-dev --target game --parallel 4`;
* `cmake --build /tmp/mop-preservation-dev --target worldserver --parallel 4`.

The workflow expected a warm tree, but the tree and ccache were absent at task
entry. Consequently the existing tree could not be reused. The prescribed fast
profile was configured once, including the new globbed source from the outset;
CMake did not need an automatic regeneration during either build. The newly
created tree is preserved for Task 027.

No SQL was executed, so startup with the migrated empty schema still requires
runtime confirmation. **A2A does not make trainer battles playable.** A2B must
still establish transient BattlePet/team ownership, species-specific breed and
team-slot semantics, trainer Creature despawn/map lifetime, concurrency, and
safe construction/teardown. A3 initiation remains absent and blocked on packet,
validation, and position evidence. Remaining A4 trainer swaps, simultaneous-
death policy, event ordering, and client traces also remain deferred.
