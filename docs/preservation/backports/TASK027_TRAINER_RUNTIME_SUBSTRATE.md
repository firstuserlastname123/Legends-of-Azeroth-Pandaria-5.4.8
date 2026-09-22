# Task 027: Trainer Runtime Substrate Review

## Continuation State

- **FACT:** Task 027 started on clean branch `work` at
  `94081dac112f50f0ee527ba7eeab23778f6c5b5a`. That squashed commit contains
  the Task 025 exactly-once finalization guard and Task 026 A2A manager/schema.
- **FACT:** `/tmp/mop-preservation-dev/CMakeCache.txt` was unexpectedly absent
  (`DEV_BUILD_TREE_MISSING`) and ccache reported 0.0 GiB used of 5.0 GiB.
- **FACT:** This task did not clean the tree/cache. Because the decision below
  produces no source change, it did not configure or build a replacement tree.

## A2A Inertness Verification

- **FACT:** Excluding documentation, `BattlePetTrainerMgr`,
  `sBattlePetTrainerMgr`, and `GetTrainerTeam` occur only in the manager's own
  files and the World startup include/load call.
- **FACT:** No `PetBattle`, `PetBattleTeam`, `Player`, `WorldSession`, opcode,
  gossip, quest, or CreatureScript code calls `GetTrainerTeam`.
- **FACT:** The schema remains empty of content and the manager only loads and
  exposes const value definitions. A2A therefore remains player-inaccessible.

## Andrew A2B Provenance

- **FACT:** Candidate 10,
  `39205279cebebd1d2c83000a0e9043e68047e659`, introduced
  `AddTrainerBattlePets`, `CreateTrainerPets`, the trainer battle type,
  constructor routing, live `Creature*` storage, saved origin, and team-owned
  deletion.
- **FACT:** `c6400a299c1fbf2df0b8654cbff001ad801e7f82` changed active-pet effect and
  dead-trainer-pet progression. Later relevant battle commits are
  `100303ca22b08f04ee5620dc1622617d5a680cdd`,
  `1e338126a8f3e5381218dc90a50185119beeb592`,
  `867b69b0d142efaaea10b83970342900d3405f2e`, and final context
  `bd8ad5515418d94abd35ba7bf71430979243a09c`.
- **FACT:** The final manager factory still allocates every trainer pet with raw
  ID zero, assigns global indexes in definition-vector order, initializes
  default abilities, pushes raw pointers into `PetBattleTeam::BattlePets`, and
  selects vector element zero as active.
- **FACT:** Final Andrew adds `m_ownsPets`; `PetBattleTeam` deletes every raw
  pointer in `BattlePets` when this team-wide flag is true. Trainer addition
  also records the live creature and origin, so Andrew itself does not separate
  A2B1 from A2B2.

## A2B1 / A2B2 Boundary

- **FACT:** Pure construction can take a trainer entry, copy Task 026 values,
  and allocate ownerless `BattlePet` objects without a `Creature*` or `Player*`.
- **FACT:** A2B2 comprises live Creature identity, GUID/map lookup, saved
  origin, despawn/map-unload handling, reservation/concurrency, and interaction
  lifetime. None is required merely to run the `BattlePet` constructor.
- **INFERENCE:** A separate value-to-object factory can technically compile,
  but it is not yet a coherent *team* substrate. Connecting objects to
  `PetBattleTeam::BattlePets` immediately assigns slot/global-index semantics,
  and current `PetBattleTeam` has no per-pet ownership representation.
- **UNKNOWN:** No final-source contract shows how an unassigned owned
  collection would later be transferred into slots without revisiting
  ownership and ordering together.

## BattlePet Ownership Models

- **FACT:** Account pets are allocated and owned by `BattlePetMgr`, stored in
  its set, saved according to DB state, and deleted by that manager.
- **FACT:** Wild pets are freshly allocated with ID zero and null owner by
  `BattlePetSpawnMgr`, stored by creature GUID in spawn-template state, borrowed
  by `PetBattleTeam`, and deleted when the spawn-manager entry is removed.
- **FACT:** Capturing does not transfer the wild pointer; finalization creates a
  new account pet from its value fields.
- **FACT:** `BattlePet::~BattlePet` is empty; abilities use `unique_ptr`, while
  aura lifetime is managed separately by battle cleanup.
- **FACT:** Current `PetBattleTeam` only borrows raw pointers and has no explicit
  destructor. It can contain account or wild pointers that it must not delete.
- **FACT:** Andrew's team-wide `m_ownsPets` is safe only while team population
  modes are mutually exclusive. It cannot safely represent a mixed borrowed
  and owned team.
- **INFERENCE:** Temporary trainer ownership is `SAFE_WITH_ADAPTATION` using an
  explicit per-object owning container (for example, `unique_ptr`) plus derived
  non-owning battle views. Andrew's boolean/raw-pointer design should not be
  staged unchanged.

## Trainer Pet Initialization

- **FACT:** Andrew calls `new BattlePet(0, species, family, level, quality,
  breed, nullptr)`. This sets ID zero, null owner, empty nickname, XP/current
  health zero, flags zero, and DB state `SAVE`, then immediately calls
  `Initialise(true)`.
- **FACT:** Initialization dereferences the species DB2 row and its NPC
  CreatureTemplate/model, fills states, applies family/breed/species modifiers,
  indexes breed-quality at `7 + quality`, calculates power/speed/max health,
  fills current health to max, and leaves the pet alive when max health is
  nonzero.
- **FACT:** Task 026 validates species, the species NPC CreatureTemplate,
  level 1--25, quality and its breed-quality row, and globally known/zero breed.
- **UNKNOWN:** Task 026 does not prove `GetModelByIdx(0)` is non-null for every
  accepted species CreatureTemplate. The constructor dereferences that model.
- **UNKNOWN:** Task 026 also does not validate family range directly; it trusts
  build-18414 species DB2 integrity before `InitialiseStates` asserts the family
  is below `BATTLE_PET_FAMILY_COUNT`.
- **Conclusion:** Constructor safety is improved but not completely proven for
  every retained definition; A2B1 cannot claim the gate's crash-free invariant.

## Temporary Pet Identity

1. **FACT:** Andrew assigns raw/ObjectGuid ID zero to every trainer pet.
2. **FACT:** Zero is also the existing supported identity for a wild temporary
   pet.
3. **FACT:** Multiple trainer pets are therefore not ID-unique within a battle.
4. **FACT:** They are not ID-unique across simultaneous battles.
5. **FACT:** initial-update serialization emits each pet's ID/GUID fields, so
   every trainer-pet GUID serializes empty in Andrew's design.
6. **FACT:** round effects, swaps, and team lookup primarily use per-battle
   global indexes, not the persistent ID.
7. **UNKNOWN:** No build-18414 trace proves that multiple zero GUIDs are valid
   or distinguishable to the client when presented in one trainer team.
- **INFERENCE:** Identity is irrelevant to a disconnected owned collection,
  but becomes unresolved as soon as A2B1 claims usable team materialization.
  Inventing IDs is prohibited. This is an integration blocker, not evidence
  for a new ID scheme.

## Team Storage / Destruction

- **FACT:** `PetBattleTeam::BattlePets` is `std::vector<BattlePet*>` and
  `SeenAction` is a raw-pointer set. `PetBattle` deletes teams; current team
  destruction does not delete pets.
- **FACT:** Andrew deletes all `BattlePets` only when `m_ownsPets` is true.
- **FACT:** Pending moves, active pet, SeenAction, effects, and auras may hold
  pointers to the same objects during a battle, but team deletion occurs only
  when the containing battle is destroyed.
- **INFERENCE:** A dedicated owning container declared before/with the raw views
  could give deterministic teardown, but publishing into the views also chooses
  slot order. Adding ownership alone without a consumer would be dead machinery;
  adding views crosses the unresolved semantic boundary.

## Team Size

- **FACT:** Task 026 rejects more than `PET_BATTLE_MAX_TEAM_PETS` (3).
- **FACT:** global indexes support three local indexes per side; packet loops
  serialize vector size and vector order. One-, two-, and three-element vectors
  are structurally representable, while an empty definition is absent.
- **FACT:** Andrew calls `ConvertToGlobalIndex(BattlePets.size())` before each
  push, so a validated maximum of three prevents that assertion from receiving
  local index 3.
- **UNKNOWN:** Structural capacity does not prove gameplay correctness for
  trainer replacement or client presentation.

## Slot / Active-Pet Ordering

- **FACT:** Definition vector positions become local slots 0, 1, and 2;
  `ConvertToGlobalIndex` turns them into effect/packet indexes.
- **FACT:** Andrew selects `GetPet(0)` as the initial active pet. Available-pet
  scans and final trainer replacement choose by vector traversal; ability AI
  operates on the selected active pet. Thus order affects start, presentation,
  abilities, death replacement, and swaps.
- **FACT:** Task 026 deliberately orders definitions by species only for stable
  storage and explicitly does not claim retail slot order.
- **INFERENCE:** Constructing objects in a separate owned collection without
  populating `BattlePets` avoids slot meaning but is not a team substrate.
  Populating team storage or selecting an active pet necessarily promotes the
  species sort to gameplay semantics.
- **Conclusion:** No A2B1 team materializer can be staged without either being
  semantically incomplete or choosing an unsupported slot/active order.

## Ability Selection

- **FACT:** `InitialiseAbilities(false)` reads species ability pairs from DB2,
  applies level requirements, and selects each first-tier ability because a new
  trainer pet has flags zero. Passing `false` prevents the wild random
  second-tier choice.
- **FACT:** The trainer schema contains no ability-selection flags/columns.
- **FACT:** Andrew therefore implements deterministic first-tier/default
  selection, not trainer-specific choices.
- **Classification:** `DBC_DEFAULT` as Andrew source behavior;
  **UNKNOWN** as retail trainer-team behavior. Calling it now would encode an
  unverified ability policy, while omitting it would not produce a battle-ready
  team. This is another reason not to stage the runtime helper.

## Health / State Initialization

- **FACT:** Constructor initialization starts stored health at zero, builds
  family/breed/species/quality states, calculates stats using the configured
  level, then fills current health to calculated max health for a new pet.
- **FACT:** Each allocation owns independent state arrays, abilities, and aura
  storage. Repeating construction would not share mutable health or states.
- **UNKNOWN:** A zero/invalid derived max-health outcome for all possible DB2
  combinations was not runtime-tested; Task 026 validates representation
  safety, not retail stat content.

## Database Isolation

- **FACT:** The new-pet constructor sets DB state `SAVE` even with owner null.
- **FACT:** Persistence is driven only by iterating `BattlePetMgr::BattlePets`;
  a standalone or team-owned trainer object is not automatically registered.
- **FACT:** Andrew never inserts trainer objects into a player manager, so they
  are not saved to `account_battle_pet` or slot tables despite their DB state.
- **INFERENCE:** A2B1 could remain nonpersistent only if its API makes transfer
  into `BattlePetMgr` impossible/explicit. Current inaccessible code would meet
  that condition, but the misleading `SAVE` state merits adaptation before a
  reusable runtime ownership API is accepted.

## Capture Isolation

- **FACT:** Current catch eligibility is based on ordinary PVE battle/team
  state, and successful finalization creates a new account pet from the caged
  pointer's values.
- **FACT:** Andrew relies on a distinct `PET_BATTLE_TYPE_PVE_TRAINER` to make
  trainer battles non-capturable. That type belongs to runtime integration and
  is absent from Task 026/current preservation.
- **UNKNOWN:** A pure temporary trainer object carries no intrinsic
  non-capturable marker. If later inserted into an ordinary PVE opponent team,
  current capture logic could treat it as wild.
- **Conclusion:** Inaccessibility prevents capture today, but A2B1 alone cannot
  establish the required future invariant without A2B2/A3 battle typing or a
  separately justified ownership/category design.

## Concurrency

- **FACT:** Andrew allocates fresh objects per call, so hypothetical concurrent
  materializations share only immutable manager definitions, not pointers,
  health, auras, abilities, or pending moves.
- **FACT:** Every object still shares zero identity, and every call derives the
  same species-ordered slots/default abilities.
- **INFERENCE:** Object state independence is sound; identity/client semantics
  and live trainer reservation remain unresolved. Creature reservation and
  despawn/map lifetime are A2B2, not Task 027 concerns.

## Player-Inaccessibility Proof

- **FACT:** A newly declared but uncalled factory/helper could remain
  unreachable: current Player, WorldSession, gossip, quest, opcode,
  PetBattleSystem, and wild PetBattle creation have no trainer-manager lookup.
- **FACT:** This establishes dead-code inaccessibility, not runtime correctness.
- **INFERENCE:** Player-inaccessibility alone is insufficient justification for
  staging a helper that cannot safely publish a usable team without unresolved
  slot, ability, identity, capture, and ownership-category decisions.

## Staging Value

**Classification: `TOO_SEMANTICALLY_INCOMPLETE`.** A disconnected factory could
compile and allocate objects, but it would either:

* stop before team slots/active pet/abilities, providing little reusable A2B
  behavior; or
* reproduce Andrew's zero identities, species-as-slot order, default ability
  policy, coarse ownership flag, and reliance on a trainer battle type that is
  outside A2B1.

Compilation would not materially reduce the future integration uncertainty.
The useful next implementation must jointly define evidence-backed team slots,
identity/client expectations, explicit ownership/category, non-capture typing,
and construction preconditions.

## Final Classification

**Task 027 classification: `NO_VALUE_IN_STAGING`.** No source implementation is
authorized. Although fresh ownerless allocation can be isolated mechanically,
the result cannot become a coherent trainer-team runtime substrate without
unsupported slot/active ordering and ability semantics, and constructor,
identity, ownership-category, and capture invariants remain incomplete.

No source or SQL changed, no targeted build was applicable, and the
documentation checkpoint is the final Task 027 commit. Task 025 finalization
and Task 026 validation remain unchanged. Task 027 does not make trainer
battles playable.

## Future A2B2 / A3 Boundary

Before runtime staging, a later evidence-backed design must resolve:

1. explicit per-pet ownership rather than a team-wide raw-pointer flag;
2. model/family constructor preconditions and nonpersistent state semantics;
3. multi-pet temporary identity as observed by build 18414;
4. authoritative trainer slot, starting-active, replacement, and ability data;
5. a non-capturable trainer battle category;
6. trainer Creature identity, origin, despawn/map lifetime, and reservation;
7. A3 packet layout, validation, terrain/position, rollback, and initiation;
8. A4 replacement/swap, event ordering, simultaneous death, and client traces.

A2B2 must own live trainer identity/lifetime. A3 must be the first deliberate
player-accessible connection and must not be introduced merely to exercise a
dead factory.
