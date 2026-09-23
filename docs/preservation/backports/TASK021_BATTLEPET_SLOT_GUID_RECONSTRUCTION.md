# Task 021 — Battle-pet slot GUID reconstruction

## Scope and starting state

Task 021 reviewed only `BattlePetMgr::LoadSlotsFromDb` for the Mists of
Pandaria 5.4.8 build-18414 preservation target. The working tree was clean at
starting commit `775909c6b70b877d58a778765c92471ef40c8273`, the merge of Task
020. The checkout's local branch name was `work`; its HEAD was the current
post-Task-020 preservation integration history. No SQL, packet/opcode,
trainer, weather, ability-effect, Playerbots, or generated-module code is in
scope.

## Andrew source provenance and candidate

- **FACT:** The source is Andrew Downey's
  `andrew-downey/Legends-of-Azeroth-Pandaria-5.4.8` commit
  `2da8a377a4c2b15de30c2cad501914301e15c6af`, authored and committed on
  2026-05-17 with subject `Fix loading battlepets on login`. Its parent is
  `c440e10f49cb6d67dea66c3916f1d594fcd7f03d`.
- **FACT:** The commit also changes `build.sh` and generated
  `modules/ModulesLoader.cpp`. Those unrelated changes are excluded.
- **FACT:** The isolated candidate hunk is:

```diff
-    ObjectGuid slot1(HighGuid::BattlePet, fields[0].GetUInt32());
-    ObjectGuid slot2(HighGuid::BattlePet, fields[1].GetUInt32());
-    ObjectGuid slot3(HighGuid::BattlePet, fields[2].GetUInt32());
+    ObjectGuid slot1(fields[0].GetUInt64());
+    ObjectGuid slot2(fields[1].GetUInt64());
+    ObjectGuid slot3(fields[2].GetUInt64());
```

- **FACT:** Both Andrew remote branches containing the commit retain the
  candidate at their fetched tips. Direct comparison with Andrew's final
  `feature/pandaria-fixes` version found the same three `GetUInt64()` raw-value
  constructions.
- **FACT:** Preservation history has no equivalent later fix: the current
  function is the original counter-reconstruction implementation introduced
  with the imported battle-pet manager.

## Login query and slot load path

1. **FACT:** `LoginQueryHolder::Initialize` obtains
   `CHAR_SEL_ACCOUNT_BATTLE_PET_SLOTS`, binds the account ID, and stores it as
   `PLAYER_LOGIN_QUERY_LOAD_BATTLE_PET_SLOTS`.
2. **FACT:** `CharacterDatabaseConnection::DoPrepareStatements` defines that
   prepared statement as `SELECT slot1, slot2, slot3, flags FROM
   account_battle_pet_slots WHERE accountId = ?` on the character database,
   with an asynchronous prepared result.
3. **FACT:** `Player::LoadFromDB` first calls `BattlePetMgr::LoadFromDb` with
   `PLAYER_LOGIN_QUERY_LOAD_BATTLE_PETS`, then calls `LoadSlotsFromDb` with the
   slot result. Thus pet objects exist before slot validation.
4. **FACT:** Result columns `fields[0]`, `[1]`, and `[2]` are respectively
   slot 0 (`slot1`), slot 1 (`slot2`), and slot 2 (`slot3`); `fields[3]` is the
   flags byte.
5. **FACT:** Current preservation reads each slot Field with `GetUInt32()` and
   constructs `ObjectGuid(HighGuid::BattlePet, counter)`. All three slots use
   the same expression apart from the Field index.
6. **FACT:** This interprets each database value as a 32-bit counter and then
   synthesizes BattlePet high bits; it does not preserve the stored unsigned
   64-bit raw value.
7. **FACT:** A stored zero produces `ObjectGuid::Empty`, because the
   `(HighGuid, counter)` constructor stores zero when its counter is zero.
   Validation also explicitly skips lookup errors for `slot == 0`.
8. **FACT:** `GetBattlePet(slot)` linearly compares the slot's raw `uint64`
   value against each loaded `BattlePet::GetId()` ObjectGuid. ObjectGuid
   equality compares `GetRawValue()` exactly. If any nonempty slot is locked
   or has no exact pet match, one shared `hasError` condition clears all three
   slots and marks the loadout for saving; flags and training-spell logic are
   otherwise unchanged.

## Save contract

- **FACT:** `SetLoadoutSlot` stores an `ObjectGuid` in the three-element
  `m_loadout` array. Normal slot assignment receives the full eight-byte
  ObjectGuid decoded from `CMSG_BATTLE_PET_SET_BATTLE_SLOT`, verifies it by
  exact `GetBattlePet` lookup, and stores that same value.
- **FACT:** `SaveSlotsToDb` deletes the account row and inserts account ID,
  `GetLoadoutSlot(0)`, `(1)`, and `(2)` through `setUInt64`, followed by flags.
  ObjectGuid's `operator uint64()` returns its raw value. This is equivalent
  to writing `GetRawValue()`, not `GetCounter()`.
- **FACT:** Empty ObjectGuids serialize as unsigned 64-bit zero.
- **FACT:** Battle-pet rows use the same identifier representation:
  `SaveToDb` writes `BattlePet::GetId()` through `setUInt64`, while
  `LoadFromDb` reads `account_battle_pet.id` through `GetUInt64()` and passes
  it directly into the BattlePet constructor.
- **FACT:** Therefore the save path is not symmetric with the current slot
  load when a nonzero raw ID is read: current load injects BattlePet high bits
  and truncates to 32 bits. It is symmetric with Andrew's proposed raw
  `GetUInt64()` construction.
- **FACT:** No checked-in migration converts slot counters to high-bit-packed
  ObjectGuids or vice versa. Repository history shows only the base-schema
  introduction of this table.
- **INFERENCE:** Rows produced by this code and legacy rows matching the base
  seed are compatible with Andrew's raw load because both pet IDs and slot IDs
  use the same raw numeric identifier. They are not two distinct deployed
  formats merely because small raw identifiers can also be described as
  counters.
- **UNKNOWN:** The contents of an independently deployed persistent database
  have not been sampled. Repository code/schema establish the expected
  contract, but cannot prove that operators never manually inserted or
  transformed values.

## Database contract

- **FACT:** The authoritative checked-in character base schema is
  `sql/base/characters.sql`. Table `account_battle_pet_slots` has primary key
  `accountId`; `slot1`, `slot2`, and `slot3` are each `bigint(20) unsigned NOT
  NULL DEFAULT '0'`; `flags` is unsigned `tinyint`.
- **FACT:** The base seed row uses `(accountId, slot1, slot2, slot3, flags) =
  (1, 2, 0, 0, 1)`. Its referenced `account_battle_pet.id` is also unsigned
  `bigint`, and the base data contains pet ID 2. This is direct evidence that
  the expected raw pet identity may be numerically small; a BIGINT declaration
  by itself is not used as semantic proof.
- **FACT:** Searches across all checked-in base and update SQL found no other
  statement or migration affecting `account_battle_pet_slots` and no
  representation conversion.

## ObjectGuid and loaded-pet representation

- **FACT:** `HighGuid::BattlePet` is numeric 21. The two-argument ObjectGuid
  constructor stores `counter | (uint64(HighGuid::BattlePet) << 52)` for a
  nonzero counter, and stores zero for counter zero. Its counter parameter and
  low portion are 32-bit.
- **FACT:** `ObjectGuid(uint64)` stores the supplied raw 64-bit value without
  adding bits or truncating it. `GetRawValue()` and implicit conversion to
  `uint64` return that value; `GetCounter()` returns only the low 32 bits;
  equality is exact raw-value equality.
- **FACT:** `ObjectMgr::GenerateBattlePetId` returns a monotonically increasing
  raw `uint64`, initialized from `SELECT MAX(id) FROM account_battle_pet`; it
  does not call an ObjectGuid high-part constructor. A newly created BattlePet
  receives that raw ID. A loaded BattlePet receives the database `id` fetched
  with `GetUInt64()`. `BattlePet::m_id` is an ObjectGuid direct-initialized
  from that raw uint64.
- **FACT:** Consequently a loaded pet with database ID 2 has in-memory raw
  ObjectGuid value 2. Current slot reconstruction turns stored slot value 2
  into `(21 << 52) | 2`, which cannot compare equal. Andrew's construction
  preserves raw value 2 and compares equal.
- **FACT:** Current code fails for every expected nonzero raw slot ID whose pet
  object uses that same database raw ID. Andrew's code would fail only if a
  slot row stored a separately high-bit-packed value while the referenced pet
  row stored only its low counter (or otherwise used a different raw value).
  No local save, load, schema seed, or migration produces that split format.
- **INFERENCE:** In this codebase a battle-pet identifier is represented by an
  ObjectGuid container but its persistent/in-memory identity is the database's
  raw unsigned 64-bit ID, not a conventional high-part-packed BattlePet GUID.
  The candidate restores raw-value identity rather than introducing a schema
  migration.

## Decision gate and classification

All six implementation gates are established locally:

1. Slots persist the full raw uint64 ObjectGuid value via ObjectGuid's uint64
   conversion.
2. Loaded BattlePet objects retain the same raw uint64 database identifier.
3. Current slot load truncates through `GetUInt32()` and injects high bits.
4. Andrew's direct uint64 construction restores save/load and pet/slot
   symmetry.
5. Raw zero still constructs an empty ObjectGuid and remains exempt from
   invalid-slot lookup.
6. The base schema and history require no migration; its explicit seed is
   already in the raw-ID form expected by the candidate.

**Final classification: `ANDREW_FIX_REQUIRED`.** The implementation will be
the exact isolated three-line Andrew hunk, not a cherry-pick. No external
corroboration was needed because the local query, save, schema seed,
identifier generator, loaded-object construction, and equality paths form a
complete and mutually consistent contract.

## Intended validation and runtime follow-up

After the documentation checkpoint, apply only the three-line candidate,
check the diff, build target `game`, and then link target `worldserver` in the
persistent fast-development tree. Compilation establishes only source/link
compatibility, not login correctness.

Later validation against a copy of the deployed database and a build-18414
client must:

1. inspect representative `account_battle_pet.id` and all three corresponding
   slot columns before startup, confirming exact raw-value equality;
2. test all three populated slots and arrangements with one or more zero/empty
   slots;
3. verify slot pet identity before and after logout/login;
4. verify again after a server restart/login;
5. repeat on multiple characters of the same account;
6. confirm pet health, XP, and level are unchanged;
7. confirm no slot is cleared or accidentally reassigned; and
8. compare the persisted values again after a legitimate slot change/save.

Runtime/deployed-database testing is deliberately deferred. It must not be
claimed from the targeted compile and link checks.

## Implementation and validation result

- **Final classification:** `ANDREW_FIX_REQUIRED`.
- **Source changed:** yes, only
  `src/server/game/BattlePet/BattlePetMgr.cpp::LoadSlotsFromDb`.
- **Applied hunk:** the exact three-line Andrew hunk recorded above; it was
  isolated manually rather than cherry-picked. All three slots now construct
  ObjectGuid directly from `Field::GetUInt64()`.
- **Unchanged behavior:** zero still constructs `ObjectGuid::Empty`; loadout
  flags, invalid-slot handling, training spells, health, XP, level, and all
  packet code are untouched. No unrelated Andrew change entered the tree.
- **Fast build configuration:** `/tmp/mop-preservation-dev` did not exist, so
  it was configured once with `PLAYERBOTS=0`, `USE_MODULES=0`, `TOOLS=0`,
  `ELUNA=0`, ccache launchers, and the documented temporary install prefix.
- **Game build:** `cmake --build /tmp/mop-preservation-dev --target game
  --parallel 4` passed and produced `libgame.a`, including compilation of the
  modified `BattlePetMgr.cpp`.
- **Worldserver build:** `cmake --build /tmp/mop-preservation-dev --target
  worldserver --parallel 4` passed and linked the `worldserver` executable.
- **Static post-build result:** save now writes the raw slot ObjectGuid as
  uint64, slot load reads that uint64 without transformation, loaded battle
  pets retain their raw uint64 database ID, and lookup compares those exact
  raw values. Save/load/object lookup are symmetrical.
- **Remaining evidence:** a deployed-database sample and the exact later
  login/relog procedure above remain required. The compile and link results do
  not establish persistent-server or client-visible correctness.
