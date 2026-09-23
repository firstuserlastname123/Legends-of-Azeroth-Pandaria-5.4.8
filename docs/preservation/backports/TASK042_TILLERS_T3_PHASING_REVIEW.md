# Task 042: Tillers T3 Personal Phasing and Visibility Review

## Starting State

- **FACT:** Review began on clean branch `work` at `c17217c153c4d9778351465b4362f463d7792639`, the merge of Task 041 (`cf5e498`). The local history also contains the merged T1 (`cd5ee92`) and T2A (`1d2db27`) work. The branch name is not literally `preservation/main`; the checked-out commit is its supplied Task 041 integration state.
- **FACT:** T1 is `sql/updates/characters/2026_09_22_00_characters_tillers_farm.sql`; T2A is `src/server/game/Tillers/TillersFarmPersistence.{h,cpp}`; T2B adds plots-then-state deletion to the established final character-delete transaction.
- **FACT:** Repository-wide source searches found no `TillersFarmMgr`, `TillersZoneHooks`, `TillersWorldHooks`, Tillers runtime phase setter, or `m_visibilityLayer`. Existing Tillers persistence has no gameplay registration. The starting tree was clean.
- **FACT:** Andrew's repository was freshly cloned to disposable `/tmp/andrew-loa-task042`; preservation remotes were not changed. Its master tip remains `f1abdd1a`; `bd8ad5515418d94abd35ba7bf71430979243a09c` remains the last runtime Tillers revision. Later master changes are tests/README, not a focused phasing correction.
- **FACT:** This task changes only this document. It does not compile, apply SQL, or execute gameplay.

## T2 Runtime-Inert Baseline

- **FACT:** Production matches for `TillersFarmPersistence::Load`, `Save`, and `Reset` occur only at their definitions. No login, zone, quest, world, command, Player, or script caller exists.
- **FACT:** Prepared-statement registration and hard-delete cleanup are infrastructure effects, not farm activation. T2 has no `Player*`, cache, hooks, automatic row creation, or automatic 16-plot materialization.
- **INFERENCE:** T3 can call `Load(guidLow)` on a future entry boundary and own the returned value without changing T2 semantics. Task 042 does not add that call.

## Andrew T3 Lineage

Path history, commit inspection, and diffs establish this final-line lineage. Earlier first-generation commits `897bbf8e` through `1bb75c28` were deleted by `dc22704b` and do not survive as T3 implementation.

| Commit / parent | Material T3 files | Introduced behavior | Supersession / final survival |
| --- | --- | --- | --- |
| `b4a0c20982f1c2d86ad61e20d1b74ff3abaf1991` / `dc22704bb60e00811a103250951af70c5f708d6f` | final-line manager/header, soil, ZoneHooks, initial character/world data | Reintroduced the global manager, dynamic soil and zone lifecycle foundation. | Foundation survives after extensive revision; its original masks/lifecycle are not independently final. |
| `6115dcae543887b54636a4fc03f1bb3fd8de93e5` / `41e67ba1fafd4f43ac13f529c7fa1577858b3802` | manager/header, soil, ZoneHooks; `05_31_03`, `05_31_04`, `06_01_00` world SQL | Added rewarded-30252 gate, GUID-derived custom mask, entry spawn, exit/logout teardown, dynamic Yoon/farm objects, and first static Yoon/rock phase correction. Initial formula used `(guidLow << 8) | (farmPhase + 1)` and claimed mask isolation. | Lifecycle survives. Formula is superseded to `| 1`; static rocks/Yoon are superseded by `06_02_04`; initial dynamic objects lacked final private ownership. |
| `1e138b1f1d7e1660eb28b647b7193afe961acb72` / `6115dcae543887b54636a4fc03f1bb3fd8de93e5` | manager/header/soil/ZoneHooks; `Object.{h,cpp}`; `06_01_00`, `06_02_02`, `06_02_04` and tutorial SQL | Converted farm state to obstacle bits, added public mask 128 and zone masks 129, made dynamic Yoon/soil/obstacles private-owned, fixed formula to `(guidLow << 8) | 1`, and added the visibility-layer field/accessors. | Core public/private architecture survives. `m_visibilityLayer` survives in Andrew but remains inert. Some `06_01_00` effects are superseded by `06_02_04`. |
| `867b69b0d142efaaea10b83970342900d3405f2e` / `1e138b1f1d7e1660eb28b647b7193afe961acb72` | manager/header; soil ScriptName and companion/state migrations | Refined tutorial/soil behavior and introduced persisted companion-unlock support. | Phasing foundation remains; no correction to mask, layer, entry coverage, or cleanup. |
| `bd8ad5515418d94abd35ba7bf71430979243a09c` / `aeef0c9d8e6484278ea98d07dde4cc608b077ff4` | manager/header, new WorldHooks, companion SQL | Finalized manager scene building and private companion spawns; retained GUID mask/private ownership and manual GUID tracking. | Final runtime state. No later master commit corrects phasing. |

- **FACT:** Commits `a008e984` and earlier materially affected an abandoned first-generation farm implementation, while `35f0c359` and later master commits do not change runtime T3.

## Current Phase APIs

- **FACT:** Every `Player` owns a by-value `PhaseMgr phaseMgr`; `GetPhaseMgr()` returns it. It is transient process/player state, not persisted in the character tables.
- **FACT:** `PhaseData` combines server masks from definitions and auras. With no resulting mask it returns `PHASEMASK_NORMAL` (1). A nonzero custom mask completely overrides that computed server mask; zero restores definition/aura computation.
- **FACT:** `SetCustomPhase(uint32)` is a raw 32-bit **mask**, not a dynamic phase-ID abstraction. It assigns `_CustomPhasemask`, marks the server-side phase dirty, calls `Update()`, and ultimately sets the player's mask.
- **FACT:** If the effective mask changed and the player is visible, `SendPhaseMaskToPlayer()` invokes `player->UpdateObjectVisibility()`. Therefore `SetCustomPhase` automatically refreshes nearby visibility for an in-world visible player; an unchanged mask returns without a refresh.
- **FACT:** `WorldObject::SetPhaseMask(mask, update)` refreshes only when `update` is true and the object is in world. Andrew creates objects with the desired mask, then in some creature paths redundantly sets it with update true.
- **FACT:** Mask equality is not required: `InSamePhase(uint32)` is `(GetPhaseMask() & otherMask) != 0`. Object-to-object phase also requires compatible phase-ID sets through `IsPhased`.
- **FACT:** Zone phase definitions are recalculated by zone updates, but a nonzero custom mask remains the effective mask. There is no automatic “farm” scope on custom masks.
- **FACT:** Destruction of the `Player` destroys its owned `PhaseMgr`, so custom state is not persisted across a genuinely new login object. A living player crossing zones/maps retains a custom mask until code explicitly clears it.

## Private Object Ownership API

- **FACT:** `SetPrivateObjectOwner` is declared on `WorldObject`; consequently creatures, gameobjects, players, corpses, dynamic objects, area triggers, and other subclasses can carry it. Practical creation paths decide which use it.
- **FACT:** Storage is a full `ObjectGuid _privateObjectOwner`, not a low GUID. Empty means public.
- **FACT:** `CheckPrivateObjectOwnerVisibility(seer)` admits (1) the exact owner GUID, (2) another private object carrying the same owner GUID, or (3) a player seer for whom `IsInGroup(ownerGuid)` is true. It otherwise denies.
- **FACT:** `CanSeeOrDetect` checks map/phase through `CanNeverSee` before private ownership. Ownership therefore does not make an out-of-map/out-of-phase object visible.
- **FACT:** Private ownership is checked before ordinary GM server-side visibility handling. A GM has no unconditional private-owner bypass; a GM must be the owner, share private owner, or qualify through the group test (unless a separate earlier always-visible override applies).
- **FACT:** `IsAlwaysVisibleFor`/`CanAlwaysSee` are evaluated before private ownership. Thus the API is not an absolute security boundary for specialized always-visible relationships.
- **FACT:** Controlled units do not generically inherit their controller GUID during this check. They can see an owner's private objects only if their own private-owner field was propagated to that full owner GUID. Current summon APIs accept/assign such a private owner, but control alone is insufficient.
- **FACT:** The check is directional: visibility of target `obj` asks `obj->CheckPrivateObjectOwnerVisibility(seer)`. A public target imposes no reciprocal private check. Two private objects with the same owner admit each other.
- **FACT:** The field is set before `AddToMap` in Andrew's direct GO creation paths and by `Map::SummonCreature` before map insertion when its private-owner argument is used. That ordering supports correct initial visibility. Andrew's later explicit creature setter is redundant.

## Private-Owner Isolation

For ordinary, in-range, phase-compatible, non-always-visible objects:

| Seer / target | Ungrouped result | Reason |
| --- | --- | --- |
| A / Object A owned by A | visible | exact full owner GUID |
| A / Object B owned by B | denied | no owner/private-owner/group match |
| B / Object A owned by A | denied | no match |
| B / Object B owned by B | visible | exact match |

- **FACT:** Overlapping masks—including bit 1—do not defeat the private-owner denial.
- **FACT:** If A and B are in the same group, each player passes `IsInGroup(otherOwnerGuid)` and can see the other's private objects, assuming all other visibility checks pass. Different groups behave like ungrouped players.
- **FACT:** A GM observer is denied unless owner/group/other earlier special visibility logic admits it. GM mode alone is not a bypass.
- **FACT:** Owner logout/map departure does not change the stored owner GUID. It only removes the seer; object destruction is a separate lifetime question.
- **FACT:** An owner is not guaranteed visibility if phase, map, distance, phase-ID, or other earlier checks fail.
- **CLASSIFICATION:** `PARTIAL`. The generic contract robustly separates ordinary ungrouped owners despite mask collision/overlap, but deliberately shares with group members, permits same-owner private seers, has earlier special-visibility exits, and does not manage lifetime.

## Public Phase Model

- **FACT:** Andrew defines `PUBLIC_FARM_MASK = 128` (bit 7). Final `06_02_04` replaces phase definitions `(zoneId, entry) = (1023,1), (5805,1), (5840,1)` with mask 129 (`1 | 128`), no phase ID, terrain swap, world-map swap, condition, or last-definition flag.
- **FACT:** Static ground Yoon GUID 516067 and farm obstacles use mask 128. Entrants whose resolved zone matches a definition see them through bit 128 while retaining ordinary bit-1 world visibility.
- **FACT:** Post-30252 Andrew sets a custom mask whose low byte is exactly 1. It therefore sees ordinary bit-1 world objects but does not intersect bit 128; custom override also bypasses the definition-derived 129.
- **FACT:** Bit 128 separates public farm presentation (public Yoon/obstacles) from farm owners. It does **not** separate farm players from the ordinary world or from one another.
- **FACT:** Plot-reference creatures and selected ranch animals are hidden with `4294901760` (`0xFFFF0000`). Static farmhouse/tools Yoon and rocks are also moved to that mask by the final public migration.

## Private Mask Formula

- **FACT:** Final code declares `uint32 guidLow = player->GetGUID().GetCounter();` and computes `uint32 phaseMask = static_cast<uint32>((guidLow << 8) | 1);` in `SpawnPlayerFarm` and the already-spawned soil upgrade path.
- **FACT:** `guidLow` is already unsigned 32-bit, so the shift is a 32-bit unsigned shift. The cast does not widen the intermediate. The result is stored in local `uint32`, passed to `PhaseMgr::SetCustomPhase`, and assigned to dynamic farm objects' phase mask.
- **FACT:** It is compared only by generic bitwise phase intersection. It is never persisted, registered as a phase ID, or compared for equality by Tillers.
- **FACT:** It is set after state load and before normal scene spawning. `DespawnPlayerFarm` clears it with `SetCustomPhase(0)`, but only if the manager's plot-map guard succeeds.

## Collision Analysis

The function is `f(g) = ((g mod 2^24) << 8) | 1` over a 32-bit low GUID. Exactly the upper eight input bits are discarded; only input bits 0–23 survive as output bits 8–31.

| `guidLow` | Result | Discarded input | Collision example |
| ---: | ---: | --- | --- |
| `0` | `0x00000001` (1) | none set | `0x01000000` |
| `1` | `0x00000101` (257) | none set | `0x01000001` |
| `0x0000FFFF` | `0x00FFFF01` | none set | `0x0100FFFF` |
| `0x00FFFFFF` | `0xFFFFFF01` | none set | `0x01FFFFFF` |
| `0x01000000` | `0x00000001` (1) | bit 24 | `0` |
| `0x12345678` | `0x34567801` | high byte `0x12` | `0x00345678`, `0x01345678`, … |
| `0xFFFFFFFF` | `0xFFFFFF01` | high byte `0xFF` | `0x00FFFFFF` |

- **FACT:** `f(a) = f(b)` exactly when `a ≡ b (mod 2^24)`. The collision period is `2^24 = 16,777,216`; there are 256 low-GUID values per output mask over the full 32-bit domain.
- **FACT:** All output masks have bit 0 set, bits 1–7 clear, and carry the retained GUID bits at 8–31.

## Phase-Overlap Analysis

- **FACT:** For any two Andrew private masks, `maskA & maskB` is at least 1. Current `InSamePhase` therefore returns true on mask grounds for every pair, regardless of GUID and regardless of a formula collision.
- **FACT:** The GUID bits add no player isolation. They can categorize/carry truncated identity and can overlap dynamic objects created with the same computed mask, but generic private ownership—not the GUID bits—provides ordinary cross-owner filtering.
- **INFERENCE:** The useful mask behavior is its low byte: bit 1 keeps normal-world visibility while absence of bit 128 hides the public farm set. A constant mask 1 would have the same relevant public/private phase intersections for final Andrew data. Unique-looking high bits are redundant when every private target is correctly private-owned.

## Visibility Layer Review

- **FACT:** `1e138b1f` adds `uint32 m_visibilityLayer`, constructor default 0, inline setter, and inline getter to `WorldObject`.
- **FACT:** At `bd8ad551`, repository-wide search finds only those three occurrences. Tillers never writes it; no visibility, phase, update, map, packet, or serialization path reads or compares it.
- **CLASSIFICATION:** `INERT_UNUSED_API` and `DO_NOT_PORT_m_visibilityLayer`.

## Generic Core Change Boundary

| Andrew/core mechanism | Classification | Finding |
| --- | --- | --- |
| Existing raw mask and phase-ID machinery | `ALREADY_EXISTS_IN_PRESERVATION` | No Andrew generic edit required. |
| Existing full-GUID private-owner field/check | `ALREADY_EXISTS_IN_PRESERVATION`, `REDUNDANT_EXISTING_API` | Sufficient for dynamic farm object ownership with documented group semantics. |
| `m_visibilityLayer` field/accessors | `INERT_EXPERIMENT` | No consumer; do not port. Activating it globally would be `GLOBAL_BEHAVIOR_RISK`. |
| Any equality-based/dynamic phase allocation | `UNRELATED` | Andrew does not implement it and T3 does not require it. |
| Generic Object visibility modification | `GLOBAL_BEHAVIOR_RISK` | Unnecessary before runtime evidence identifies a missing contract. |

## World SQL Phase Migrations

| Migration | Final effect and assumptions | Supersession / classification |
| --- | --- | --- |
| `2026_06_01_00_tillers_farm_phase_fix.sql` | Conditional update on map 870: entries 58646/58721 from `0xFFFF0000` to 1. Earlier revision also included 58719; `1e138b1f` removed it. | Yoon result is later overwritten selectively by `06_02_04`; `SUPERSEDED`, `ASSUMES_ANDREW_BASE`. |
| `2026_06_02_02_tillers_obstacle_phasing.sql` | Coordinate-bounded map-870 obstacle entries 210443–210448, 210462, 210451, 209572 -> 128; entry 55626 at z 164–166 -> `0xFFFF0000`; entries 59536/59599/59600 in farm bounds -> hidden. | Obstacle update is repeated by `06_02_04`; reference/ranch hiding survives. `DESTRUCTIVE_MUTATION`, `NEEDS_RUNTIME_EVIDENCE`. |
| `2026_06_02_04_tillers_public_farm_phase.sql` | GUID 516066 Yoon and every map-870 58721 -> hidden; GUID 516067 Yoon -> 128; bounded obstacles -> 128; REPLACE definitions 1023/5805/5840 entry 1 -> 129; every map-870 58719 -> hidden; deletes 58721 queststarter 30252 and REPLACEs 58646 starter. | Final public model. `GUID_SENSITIVE`, `DESTRUCTIVE_MUTATION`, `ASSUMES_ANDREW_BASE`, `QUEST_DATA_DEPENDENT`, `NEEDS_RUNTIME_EVIDENCE`. |
| `2026_06_20_01_tillers_companions.sql` | Adds templates and fixed coordinate/reference rows GUID 516068–516077 at phase 1; manager later reads positions and spawns copies. | Optional T11, not required by T3. Fixed GUIDs are collision-sensitive and static rows would be visible unless other treatment/data semantics intervene. |

- **FACT:** No later focused world migration changes the final public phase definitions or Yoon/obstacle masks.

## Current World Compatibility

- **FACT:** Preservation contains no base `world.sql`; repository updates do not establish Andrew's assumed farm spawn rows, GUIDs 516066/516067, phase definitions, or queststarter state. Text searches do not find Andrew's migrations or those GUIDs.
- **FACT:** Existing source-number matches for 5805/5840 include unrelated spell/zone comments and cannot prove AreaTable or world rows.
- **UNKNOWN:** Actual deployed world database row existence, pre-masks, coordinates, duplicate GUIDs, and `(zoneId,entry)` phase-definition occupancy cannot be established from this repository.
- **CLASSIFICATION:** None of these migrations is presently `CANONICAL_FINAL_DATA` for preservation. `06_01_00` is `SUPERSEDED`; `06_02_02` is coordinate-sensitive destructive mutation; `06_02_04` is GUID-sensitive, quest-data-dependent, assumes Andrew's base, and needs runtime evidence; companion data is optional/GUID-sensitive.

## Zone / Micro-Zone Model

- **FACT:** Andrew names 1023 Valley of the Four Winds, 5805 The Heartland, and 5840 Sunsong Ranch in SQL. The same SQL states the two farm micro-zones have `ParentAreaID=0`, causing `GetZoneId()` to resolve to 5805/5840. This DBC assertion is Andrew evidence, not independently available preservation DBC evidence.
- **FACT:** `OnUpdateZone(Player*, newZone, newArea)` ignores `newArea` and spawns only when `newZone == 1023`. It never accepts 5805 or 5840.
- **FACT:** SQL compensates visibility definitions for all three IDs, but code does not compensate entry/session creation. Thus a direct login/teleport or transition resolving as 5805/5840 cannot pass the spawn gate; entering outer 1023 can spawn the scene far beyond the ranch.
- **CLASSIFICATION:** `API_MISMATCH` with a demonstrable `MICROZONE_GAP` under Andrew's own stated DBC semantics. Exact client transition sequences still require build-18414 runtime proof.

## Farm Entry Lifecycle

Final Andrew order is:

1. **PHASING trigger:** `OnUpdateZone` receives a zone transition, accepts only 1023, then requires `IsQuestRewarded(30252)` and “not spawned.”
2. **PERSISTENCE/reference initialization:** `SpawnPlayerFarm` repeats world/reward checks; lazily loads global plot/Yoon/obstacle/companion positions; synchronously loads state and plots, or materializes 16 in-memory defaults.
3. **QUEST PROGRESSION:** reconciles companion unlocks from reputation/quest and computes unlocked plots; this is not T3 responsibility.
4. **PHASING:** computes the GUID mask and calls `SetCustomPhase` first, causing player visibility recalculation when changed.
5. **SPAWNING teardown/rebuild:** removes tracked old objects, then spawns dynamic Yoon and farm-state obstacles; after rewarded 30256 mutates initial plot states and creates soil; then spawns companions.
6. **VISIBILITY:** direct GOs receive owner/private owner before map insertion; Yoon/creature companions use summon ownership and/or explicit private owner and phase updates.

- **INFERENCE:** T3 owns entry detection, public/private mode selection, phase application, and visibility ownership contract—not plot materialization/mutation, obstacles' quest progression, soil, companions, or crop state.

## Farm Exit Lifecycle

- **FACT:** `OnMapChanged` calls despawn when the player's **new/current** zone is not 1023. `OnLogout` calls it only if manager state says spawned. There is no general `OnUpdateZone` exit branch, so walking from 1023 to a non-1023 zone on the same map does not teardown.
- **FACT:** Despawn returns immediately if player is null/not in world or if `_playerPlots` lacks the GUID. Otherwise it saves, erases state/plots, removes tracked soil/Yoon/obstacles/companions by looking on the player's current map, and clears custom phase.
- **FACT:** Because a map-change callback observes the new map, lookups for objects left on the old map can fail; tracking vectors are nevertheless erased. This can strand objects until map/grid lifetime removes them.
- **FACT:** Teleport/hearth/map transfer can hit `OnMapChanged`; ordinary zone exit does not. Logout attempts cleanup, but disconnect ordering and `IsInWorld` require runtime proof. Death has no hook. Stale/missing plot cache prevents even custom-phase clearing.
- **CLASSIFICATION:** Exit is explicit and incomplete, not an automatic scoped phase lifecycle.

## Login / Relog

- **FACT:** Andrew has no login hook. Login outside the farm does nothing and starts with a fresh `PhaseMgr` custom mask 0. Login inside 5805/5840 depends on whether core emits `OnUpdateZone` and on the 1023-only gate; under the stated resolved IDs it does not spawn.
- **FACT:** A successful logout teardown saves and clears; a later qualifying 1023 transition reloads. A failed teardown loses only runtime/cache certainty—the new Player object's PhaseMgr still starts without the old custom mask, while old dynamic objects may remain until map cleanup.
- **FACT:** Andrew creates in-memory defaults and 16 plots when no row exists; current T2 instead returns caller-owned `NotPersisted` defaults with zero plots and performs no write/materialization.
- **INFERENCE:** T3 can consume current T2 unchanged: treat `NotPersisted`, `Persisted`, and `InvalidRoot` as explicit entry outcomes, keep the returned value session-owned, and defer creation/mutation policy to later tasks.

## Quest 30252 Transition

- **FACT:** The exact condition is `Player::IsQuestRewarded(30252)`, not accepted, active, complete-but-unrewarded, or SQL condition state.
- **FACT:** Both ZoneHooks and manager enforce it. `OnQuestRewarded` immediately invokes spawn for quest 30252 (and 30256) only when `GetZoneId() == 1023`.
- **FACT:** SQL has unconditional public definitions; the custom mask, not a conditional definition, removes bit 128 after reward. The migration also moves quest 30252 starter from 58721 to 58646.
- **INFERENCE:** T3 needs a future transition input but should not own quest reward semantics until T4/runtime evidence. Task 042 adds no hook.

## Public / Private Yoon

| Representation | Final source/data | Purpose and visibility |
| --- | --- | --- |
| Farmer Yoon 58646 GUID 516066 | static, hidden `0xFFFF0000` | farmhouse/reference representation; not normal public/private presentation |
| Farmer Yoon 58646 GUID 516067 | static mask 128 | ground public Yoon for definition-mask entrants |
| Tools Yoon 58721 (all map 870) | static hidden `0xFFFF0000` | suppressed by final migration; removed as 30252 starter |
| Farmer Yoon 58646 | dynamically summoned at position loaded from a static 58646 row | post-30252 private Yoon, custom mask, full owner GUID |

- **FACT:** Correctly loaded, ungrouped owners see their own dynamic Yoon and not another owner's; grouped players are allowed to see each other's.
- **POSSIBLE DEFECT:** Missing GUID 516067 yields zero public Yoon; missing/ambiguous position data prevents private Yoon; failed SQL preconditions can leave an extra static Yoon. Group sharing can produce multiple private Yoons. Exact database cardinality is unknown.

## Obstacle Visibility

- **FACT:** Weeds 210443–210448/210462, wagon 210451, and boulder 209572 are static public GOs at mask 128 within coordinate bounds.
- **FACT:** Andrew also loads those static rows as coordinate records and dynamically creates owner/private-owner GOs according to farm-state bits: weeds 2, wagon 4, boulder 8. Their private object mask is the GUID-derived mask.
- **FACT:** Plot references 55626 and ranch animals 59536/59599/59600 are hidden static data, not private obstacles. Tutorial rocks 58719 are hidden static creatures and tutorial SAI spawning is T4, not T3.
- **INFERENCE:** T3 should establish only public/private visibility and ownership. Which obstacles exist after quest rewards is progression state and remains outside T3.

## T5 Soil Visibility Contract

- **FACT:** Andrew's persistent soil presentation is dynamically created GO 186314 at reference positions after rewarded quest 30256. It uses the player's private mask, `SetOwnerGUID(full player GUID)`, and `SetPrivateObjectOwner(full player GUID)` before `AddToMap`.
- **FACT:** Private ownership alone is sufficient to isolate soil between ordinary ungrouped players even with a common mask. A phase compatible with the owner remains necessary because ownership does not bypass phase checks.
- **INFERENCE:** T5 needs a T3 API that creates/registers a world object with a session-approved phase mask and full private owner. It does not require GUID-derived uniqueness or `m_visibilityLayer`.

## T11 Companion Visibility Contract

- **FACT:** Creature companions use the same private-owner mechanism and phase. Companion GOs are summoned through the player's GO summon path and set to the private mask; final Andrew does not explicitly call `SetPrivateObjectOwner` on those GOs, so inherited ownership/private behavior must be runtime-confirmed.
- **INFERENCE:** T11 can reuse T3 ownership/session policy but is optional and must not drive T3 design or import its data now.

## Player-to-Player Visibility

- **FACT:** Andrew never makes players private-owned and never consumes `m_visibilityLayer`. All private masks contain bit 1, so players A and B at the same coordinates pass mask intersection and remain subject to ordinary player visibility.
- **FACT:** Both can see normal bit-1 NPCs/objects. Public-mode players additionally see mask-128 public farm objects; private-mode players do not. Private dynamic farm objects are owner/group filtered.
- **INFERENCE:** Andrew's source establishes visible players plus isolated-or-group-shared farm objects—not isolated player avatars. Retail correctness is **UNKNOWN**.

## Group / Party Behavior

- **FACT:** Generic private visibility explicitly admits a player in the private owner's group. Andrew adds no host selection, raid/party distinction, distance override, session transfer, or visit-farm code.
- **CLASSIFICATION:** `GROUP_SHARE` at the raw visibility-check level; a coherent `HOSTED_FARM` experience is `NOT_IMPLEMENTED`. Retail visiting semantics remain **UNKNOWN**.

## Custom Phase Cleanup

- **FACT:** A living Player retains a nonzero custom mask across zone recalculation, teleport, and map change unless explicit code calls `SetCustomPhase(0)`. Andrew attempts this only at the end of successful manager despawn.
- **FACT:** Same-map zone exit, early guards, and callback/map ordering can leave the custom phase active outside the farm. That still includes normal bit 1 but suppresses definition masks such as public bit 128 and can affect unrelated phase visibility.
- **FACT:** Logout/destruction removes the transient `Player`/`PhaseMgr`; relog does not reload custom mask. That does not repair leaked objects.
- **CLASSIFICATION:** `PARTIAL`: automatic at object destruction only; explicit, reliable teardown is required for every live-player exit path.

## Private Object Lifetime

- **FACT:** Andrew tracks GUID vectors and manually despawns soil/obstacle/companion GOs and creatures. Soil and obstacles are direct map-owned allocations; ownership metadata alone does not bind their lifetime to Player destruction.
- **FACT:** Yoon/creature companions are manual-despawn temporary summons, and the player summon relationship may assist owner cleanup, but Andrew still explicitly tracks/removes them. This is not proof that every logout/map path removes them.
- **FACT:** Removal functions require an in-world player and generally look up objects on the player's current map. Old-map lookup failure plus erased tracking can leak until grid/map cleanup.
- **INFERENCE:** A future implementation needs a map-safe teardown handle/owner-scoped summon facility or session tracking for objects it actually spawns. T3 phasing-only need not track object GUIDs; T3D/T5 must solve lifetime before spawning.

## T2/T3 Ownership Boundary

| Model | Decision |
| --- | --- |
| A. Load a T2 value on entry and discard on exit | Safe and simplest if T3 only decides mode and does no long-lived spawning; repeated entry reload is acceptable. |
| B. Player/farm runtime session while inside | **Recommended** once a lifecycle exists: own one returned `PlayerFarmData` value and phase state for enter-to-exit; later spawning can attach explicit handles. |
| C. Global manager cache | Rejected. Andrew's global maps/references outlive locks and are unnecessary for T2 or phasing. |
| D. Stateless helper plus lifecycle-owned session | Preferred shape: pure mode/mask/ownership decisions, with storage owned by the player's bounded farm session rather than a singleton. |

- **FACT:** T2 returns values, exposes no references, and owns no Player. T3 must preserve those properties across the boundary.
- **INFERENCE:** Loading once per bounded entry session avoids escaped references and global map races while permitting later T5 to read a stable session snapshot.

## Runtime Session Model

Smallest phasing-only session:

- full player `ObjectGuid` (and low GUID only for T2 lookup/logging);
- caller-owned `PlayerFarmData` value plus its explicit load status;
- mode enum (`Public` or `PrivateEligible`), with the eligibility input supplied by a later integration task;
- prior/applied custom-mask state or a boolean proving T3 owns the override, so teardown is idempotent;
- no spawned-object GUIDs until a separate spawning unit actually creates objects.

Lifetime is `qualified farm enter -> load/decide/apply -> qualified farm exit/map/logout teardown -> destroy`. Exclude crop mutations, timers, rewards, commands, companions, quest progression, and global caches. Prefer mask 1 for a private session unless runtime/data evidence demonstrates a need for other bits; private object isolation must use full-GUID ownership.

## Build-18414 Evidence Boundary

- **Server-provable FACT:** current private-owner branches and group exception; full GUID storage; bitwise mask semantics; custom-mask override/update call graph; formula width/collision period; Andrew hook/cleanup call graphs; absence of visibility-layer consumers; static SQL statements.
- **Runtime/client-required UNKNOWN:** actual AreaTable resolution and transition callbacks with build-18414 data; client create/destroy update behavior; public/private Yoon presentation; deployed world-row preconditions; grouped farm visit experience; login/logout callback ordering; old-map object cleanup; quest-phase experience; GM observational tooling; models and obstacle/soil presentation.
- **INFERENCE:** Runtime tests should validate the proven server expectations rather than redefine them. Retail behavior not established by Andrew/source remains unknown.

## Two-Player Runtime Test Plan

Use two normal accounts A/B, instrumentation for current zone/area/mask/session/object owner, and optionally a GM observer.

| Step | Server-side assertion | Client-visible observation |
| --- | --- | --- |
| Both outside | no farm session/custom mask/private farm objects | ordinary mutual/world visibility |
| A enters public state | resolved zone+area recorded; mode public; effective 129 if SQL applies | A sees public Yoon/obstacles and ordinary world |
| B enters public state | independent public mode | A/B see each other and same public set |
| Reward/prepare A then enter | A private session; custom/private mask applied; no B ownership | A loses public set; B retains it; avatars remain mutually visible where in range |
| A and B both private, same coordinates | each object stores correct full owner; masks logged | each sees own Yoon/object/soil only while ungrouped |
| Inject one private A object and one B object | four `CanSeeOrDetect` outcomes match owner matrix | A sees A/not B; B sees B/not A |
| Group A/B | group exception returns true | each now sees the other's private objects; record whether this is desired |
| Ungroup A/B | exception ceases and visibility refresh is observed/forced | foreign private objects disappear |
| A exits same-map zone | session teardown and mask reset to definition/default; all A handles gone | A farm scene disappears; B remains unchanged |
| A teleports/maps/hearths | teardown runs against correct source map; no orphan GUIDs | no farm objects at destination; B scene unchanged |
| A logs out/relogs inside/outside | old session gone; fresh custom mask 0 before re-entry; T2 status unchanged | no duplicate A scene; correct public/private reconstruction |
| GM observes | GM without group fails private-owner check; grouped GM behavior logged | confirm no accidental GM bypass; use server diagnostics if client cannot see |

Also test A/B before and after 30252 independently, direct teleport/login to each of 1023/5805/5840, disconnect (not graceful logout), death/release, repeated enter/exit, colliding synthetic low-GUID masks if feasible, and bit-1 overlap. Gameplay is not executed in Task 042.

## Disposable World-DB Test Plan

1. Clone/import the exact candidate world baseline into a disposable database; record checksum and transaction/backup rollback point.
2. Assert exact cardinality, entry, map, coordinates, and pre-mask for GUIDs 516066/516067; enumerate every 58721/58719, bounded obstacle, 55626, and ranch-animal row.
3. Assert quest 30252 starter rows and all `(1023|5805|5840, entry=1)` phase definitions; detect entry/flag/condition collisions before any `REPLACE`.
4. Check proposed fixed GUIDs against both creature and gameobject namespaces and verify template entries exist.
5. Apply each reviewed migration separately in a transaction; capture `ROW_COUNT()` per statement and fail on unexpected zero/multiple counts rather than silently accepting drift.
6. Query exact post-state and prove rows outside map/coordinate/entry/GUID predicates are unchanged with before/after checksums or audit tables.
7. Reapply to measure idempotence. Conditional updates, `REPLACE`, deletes, and broad updates need explicit expected second-run counts; do not equate syntactic reapplication with semantic safety.
8. Roll back/restore and prove the baseline checksum/cardinalities return. Then test the eventual consolidated T3C migration—not Andrew's cumulative chain—on a fresh disposable copy.
9. Defer promotion until build-18414 two-player tests validate public definitions, Yoon, obstacles, transitions, and queststarter behavior.

## T3 Split Design

1. **T3A — private-owner/runtime proof, no production hook:** a tiny existing-harness test if one becomes available, or instrumented two-player baseline, confirming full-GUID owner/group/GM/map-insertion behavior. No Object changes.
2. **T3B — bounded phasing session and entry/exit lifecycle:** consume a caller-owned T2 value; explicit area coverage; idempotent apply/clear; no farm object spawning, quest mutation, or global cache. This should follow T3A evidence and be independently reversible.
3. **T3C — reviewed public farm world SQL:** reconstruct against the actual target world DB, use strict preconditions/counts, and keep it separate from code.
4. **T3D — private farm object substrate for later T5:** only after lifetime semantics are proven, add owner-before-insertion spawn/teardown handles. Do not spawn Yoon/soil/obstacles merely to establish phasing.

- **INFERENCE:** This split is smaller and safer than importing Andrew's manager. SQL and runtime lifecycle can be reviewed/rolled back independently.

## Core Change Decision

- **CLASSIFICATION:** `EXISTING_PRIVATE_OWNER_API_SUFFICIENT` for ordinary dynamic farm-object isolation, with its existing group-sharing semantics explicitly acknowledged.
- **DECISION:** `DO_NOT_PORT_m_visibilityLayer`. No `Object.cpp`/`Object.h` change is required for T3.

## Private Mask Decision

- **CLASSIFICATION:** `REDUNDANT_WITH_PRIVATE_OWNER`, `USEFUL_BUT_NOT_ISOLATING`, and `COLLISION_UNSAFE`.
- **DECISION:** Do not import the GUID-derived formula merely because Andrew used it. Its only demonstrated useful distinction is excluding public bit 128 while retaining normal bit 1; that does not require embedded GUID bits.

## Public Mask Decision

- **CLASSIFICATION:** `SUPPORTED_BY_FINAL_WORLD_DATA` in Andrew's repository, `QUEST_DATA_DEPENDENT`, and `NEEDS_BUILD18414_RUNTIME_EVIDENCE` for preservation. It is not yet proven structurally required or compatible with the current deployed world.
- **DECISION:** Keep public 128/129 data separate from private ownership and defer migration construction to T3C after row reconstruction.

## Static Defect Map

| Finding | Classification | Consequence |
| --- | --- | --- |
| GUID formula drops high 8 bits; period `2^24` | `STATIC_DEFECT` | 256-to-1 masks over uint32 domain. |
| Every private mask shares bit 1 | `STATIC_DEFECT` as isolation claim | All private players overlap; mask cannot isolate. |
| `m_visibilityLayer` has no consumer/writer | `STATIC_DEFECT` / inert experiment | Adds API/state without behavior. |
| Hook accepts 1023 while SQL says farm resolves 5805/5840 | `STATIC_DEFECT` under Andrew's own model; runtime manifestation needs proof | direct micro-zone entry/relog may not spawn. |
| Same-map zone exit has no teardown | `STATIC_DEFECT` | custom mask/session can leak. |
| Map-change removal looks in new map | `POSSIBLE_DEFECT` | old-map private objects may remain; tracking is erased. |
| Despawn plot-map guard can skip phase clear | `STATIC_DEFECT` | stale custom phase. |
| No login hook | `POSSIBLE_DEFECT` | relog inside farm depends on callback behavior and mismatched ID. |
| Public/private Yoon assumes exact static rows/coordinates | `POSSIBLE_DEFECT` | duplicate or zero Yoon. |
| Broad/conditional SQL and fixed GUID assumptions | `POSSIBLE_DEFECT` | no-op, unintended rows, or collisions on another base. |
| Group exception exposes other member's private scene | `RETAIL_SEMANTICS_UNKNOWN` | raw group sharing exists; hosted farm is absent. |
| Player avatars remain mutually visible | `RETAIL_SEMANTICS_UNKNOWN` | source proves implementation, not retail intent. |
| GM has no unconditional private bypass | `RETAIL_SEMANTICS_UNKNOWN` | operational observation may require grouping/instrumentation. |
| Companion GO explicit private owner is absent | `POSSIBLE_DEFECT` | depends on summon-path ownership semantics. |

## Final Classification

- **CLASSIFICATION:** `READY_FOR_SPLIT_T3`.
- **RATIONALE:** Current mask and private-owner semantics are known; group/GM/control caveats are explicit; GUID collision is quantified; the visibility layer is proven inert; public data and its compatibility gap are reconstructed; zone mismatch and cleanup defects are identified; T2 values can remain caller-owned; no global cache is needed; SQL remains separate; and two-player/DB plans exist.
- **BOUNDARY:** “Ready” means ready for bounded evidence and implementation units, not ready to enable Tillers or assert retail behavior. Public world data and client-visible results still require disposable-DB and build-18414 validation.

## Recommended Task 043 Unit

**Task 043: Tillers T3A private-owner visibility runtime proof (review/test only).** Use two normal players and an optional GM on a disposable runtime to validate the existing API's owner, non-owner, group, GM, controlled-unit, overlapping-mask, and owner-exit behavior, plus owner-before-map-insertion visibility. Do not modify generic Object code, do not add farm hooks, do not consume T2 from gameplay, do not apply Andrew SQL, and do not spawn actual Tillers content. The result should decide whether T3B may rely on the existing API exactly as documented.
