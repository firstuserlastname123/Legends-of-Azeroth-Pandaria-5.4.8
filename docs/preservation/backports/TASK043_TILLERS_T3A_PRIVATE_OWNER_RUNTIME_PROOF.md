# Task 043: Tillers T3A Private-Owner Runtime Proof

## Starting State

- **FACT:** Review began on branch `work` at `1b13c2843c22d9e0bd5cbf749faaeb55627d0a2e`, the merge of Task 042. `git status --short` was empty.
- **FACT:** The immediately preceding history contains Task 042 (`bfba9bd`), Task 041/T2B (`cf5e498`), and Task 040/T2A (`1d2db27`). T2 remains an inert persistence service under `src/server/game/Tillers`; repository searches found no gameplay caller of `TillersFarmPersistence::Load`, `Save`, or `Reset`.
- **FACT:** Searches found no T3 farm runtime hook and no `m_visibilityLayer` in production source. No Andrew Tillers phasing migration is present in `sql`; existing Tillers-related source matches are the inert T2 service and unrelated pre-existing Pandaria scripts.
- **FACT:** This task changes documentation only. It does not change C++, SQL, gameplay registration, or Tillers behavior.

## Current Source Contract

- **FACT:** `WorldObject` stores `_privateObjectOwner` as an `ObjectGuid`. `SetPrivateObjectOwner` assigns that value without truncation, `GetPrivateObjectOwner` returns it, and `IsPrivateObject` tests it for non-empty (`src/server/game/Entities/Object/Object.h:651-654,711`).
- **FACT:** `ObjectGuid::operator==` compares `GetRawValue()`, not `GetCounter()` (`src/server/game/Entities/Object/ObjectGuid.h:252`). Thus both direct-owner and same-private-owner comparisons use the complete encoded GUID.
- **FACT:** `CheckPrivateObjectOwnerVisibility` admits a non-private target, the exact owner, a private seer with the same owner GUID, or a player whose current/original group GUID equals the private-owner GUID. It denies every other seer (`src/server/game/Entities/Object/Object.cpp:2219-2237`).
- **FACT:** `Player::IsInGroup(ObjectGuid)` compares the supplied GUID with the full GUID of `GetGroup()` and `GetOriginalGroup()` (`src/server/game/Entities/Player/Player.cpp:25203-25214`). The exception is membership in the owner's group, not merely being in any group.
- **FACT:** `CanSeeOrDetect` first rejects `IsNeverVisibleFor`/`CanNeverSee`, then returns for `IsAlwaysVisibleFor`/`CanAlwaysSee`, and only then applies private-owner filtering (`src/server/game/Entities/Object/Object.cpp:2239-2252`). GM detection logic occurs later, so GM mode is not an unconditional bypass of the private-owner denial.
- **FACT:** `CanNeverSee` rejects different maps and incompatible phases before private-owner filtering can admit a target (`src/server/game/Entities/Object/Object.cpp:2338-2368`). `InSamePhase(uint32)` is nonzero mask intersection, and `InSamePhase(WorldObject)` additionally checks phase IDs (`src/server/game/Entities/Object/Object.h:445-446`; `Object.cpp:3689-3692`). Ownership therefore does not override map or phase compatibility.
- **FACT:** Controller substitution occurs later in `CanDetect`; it does not rewrite the seer passed to `CheckPrivateObjectOwnerVisibility` (`src/server/game/Entities/Object/Object.cpp:2253-2336,2370-2382`). A pet/controlled unit does not generically inherit its controller's private-owner identity.
- **FACT:** `Map::SummonCreature` creates and phases a summon, sets its private owner, and only then calls `AddToMap` (`src/server/game/Entities/Object/Object.cpp:2648-2763`). The spell personal-gameobject path likewise creates/phases the GO, calls `SetPrivateObjectOwner`, then calls `AddToMap` (`src/server/game/Spells/SpellEffects.cpp:7329-7353`).
- **FACT:** `Map::AddToMap` adds the object to the grid/world and ends by calling `UpdateObjectVisibility(true)` (`src/server/game/Maps/Map.cpp:551-607`). Consequently ownership must already be present to constrain the initial visibility pass.
- **FACT:** This contract is materially identical to Task 042. The `SOURCE_CONTRACT_CHANGED` stop condition did not occur.

## Existing Test Infrastructure

- **FACT:** Repository searches for GoogleTest, gtest, Boost.Test, `enable_testing`, `add_test`, WorldObject/Map/visibility/ObjectGuid/Group tests, and test directories found no core test target capable of constructing mapped players, groups, creatures, or gameobjects. `dep/StormLib/test` is dependency-local and unrelated.
- **FACT:** No existing test target can exercise private ownership, group membership, phase compatibility, and map insertion together.
- **CLASSIFICATION:** `NO_LIGHTWEIGHT_TEST_HARNESS`.

## Runtime Options

1. **A — existing harness:** unavailable; no applicable core target exists.
2. **B — `/tmp` C++ harness:** not practical as a lightweight proof. The relevant method is coupled to polymorphic `WorldObject`, real `Player`/`Group`, maps, object stores, DBC-backed creation, and visibility update machinery. Reproducing only the branch would test a copy, while linking enough of `game` to instantiate valid objects would approximate a worldserver fixture rather than a small harness.
3. **C — disposable worldserver plus clients:** unavailable in this worker. No configured world/auth/characters databases, extracted build-18414 maps/DBC/vmaps, runnable realm, test accounts, or build-18414 clients were supplied.
4. **D — persistent host:** practical and required for direct client evidence, provided a temporary nonproduction private-object mechanism is available outside the canonical patch.

## Testability Decision

- **CLASSIFICATION:** `RUNTIME_TEST_REQUIRES_EXTERNAL_CLIENTS`.
- **FACT:** Source behavior can be fully traced, but no source-only result is reported as runtime proof.
- **UNKNOWN:** Client create/destroy packet behavior, automatic visibility refresh after group changes, and object survival across owner exit remain unobserved.

## Test Object Strategy

- **FACT:** No Tillers ID, Yoon, soil, obstacle, Andrew migration, or farm hook was used.
- **REQUIRED EXTERNAL FIXTURE:** On a disposable persistent host, create generic non-Tillers Creature A/Creature B and GameObject A/GameObject B in the same map, range, and compatible phase. Assign A objects to Player A's full `ObjectGuid` and B objects to Player B's full `ObjectGuid` before insertion.
- **UNKNOWN / BLOCKER:** Existing commands can spawn ordinary objects but cannot assign `SetPrivateObjectOwner`. Therefore the required fixture needs a temporary, noncommitted test binary/instrumentation or an already-existing spell path that creates a suitable generic personal GO; neither is available in this worker. It must not become production source or SQL.

## Full-GUID Ownership

- **FACT:** Storage, getter, setter, and equality are full-`ObjectGuid`; the comparison never calls `GetCounter()`.
- **SOURCE RESULT:** Exact full owner is admitted and a different raw GUID is denied, even if a deliberately constructed identity shares a counter component. This is source-proven, not harness-proven.
- **EXTERNAL TEST:** Log both raw GUIDs and counters; use distinct full GUIDs designed to expose counter-only comparison where the runtime's valid GUID construction permits it. Assert exact raw owner passes and the alternate raw GUID fails.

## Ungrouped Owner Matrix

| Seer | Target | Expected from source | Actual evidence | Result |
| --- | --- | --- | --- | --- |
| Player A | Object A, owner A | admitted | branch/equality trace | `SOURCE_PROVEN`; client deferred |
| Player A | Object B, owner B | denied | fall-through trace | `SOURCE_PROVEN`; client deferred |
| Player B | Object A, owner A | denied | fall-through trace | `SOURCE_PROVEN`; client deferred |
| Player B | Object B, owner B | admitted | branch/equality trace | `SOURCE_PROVEN`; client deferred |

All expectations assume same map, range, compatible phase, in-world state, no always-visible relationship, and otherwise ordinary visibility.

## Phase Overlap

- **FACT:** Mask compatibility is bitwise intersection. Masks `1` and `1` overlap; Andrew-like masks `(x << 8) | 1` and `(y << 8) | 1` also always intersect at bit 1.
- **FACT:** After map/phase admission, private-owner filtering still denies a non-owner. Overlap does not defeat private ownership, but it also does not isolate players or objects by itself.
- **RESULT:** `SOURCE_PROVEN`; client proof deferred. Architecture remains **private owner + ordinary compatible phase**, not GUID-derived private phases.

## Phase Mismatch

- **FACT:** `CanNeverSee` rejects incompatible phases before the owner check. Exact owner plus disjoint masks (for example 1 and 2), absent another shared phase-ID relationship, is hidden.
- **RESULT:** `SOURCE_PROVEN`; client proof deferred. T3B must preserve phase compatibility.

## Group Sharing

- **FACT:** A player in the private owner's current or original group is admitted by the explicit group branch.
- **RESULT:** `GROUP_SHARE_SOURCE_ONLY` and `GROUP_REFRESH_UNPROVEN`.
- **UNKNOWN:** No client/runtime evidence establishes whether group join automatically schedules the necessary create update or whether `UpdateObjectVisibility` must be invoked by the test fixture.

## Different Groups

- **FACT:** `IsInGroup` compares exact group GUIDs. Player B in Group 2 is denied an A-owned target whose owner GUID resolves through Group 1; merely having a group is insufficient.
- **RESULT:** `SOURCE_PROVEN`; runtime deferred.

## GM Behavior

- **FACT:** The private-owner check precedes GM detection shortcuts. An ungrouped GM who is neither the owner nor a same-owner private seer is denied.
- **FACT:** A GM grouped with A qualifies through the ordinary group exception, not through GM mode.
- **CLASSIFICATION:** `CLIENT_TEST_DEFERRED`; no live GM was available.

## Controlled Units

- **FACT:** The private-owner check sees the controlled unit's own GUID/private-owner field. Controller substitution is limited to later detection logic.
- **FACT:** Controller relationship alone therefore does not admit A's pet to an A-owned target. Explicitly setting the controlled unit's private owner to A would satisfy the same-private-owner branch.
- **CLASSIFICATION:** Source requires explicit propagation if such object-to-object visibility is later needed; runtime remains unproven.

## Same-Owner Private Seers

- **FACT:** Object X private-owned by A is admitted as seer of Object Y private-owned by A because Y compares its owner to X's private-owner value.
- **RESULT:** `SOURCE_PROVEN`; runtime deferred.

## Owner-Before-Insertion

- **FACT:** Both inspected private Creature and personal GO paths assign private owner before `AddToMap`; `AddToMap` performs the initial visibility update after insertion.
- **INFERENCE:** `OWNER_BEFORE_INSERTION` is the safe mandatory ordering for T3D/T5 and avoids an initial public visibility pass.
- **UNKNOWN:** Without packet/client instrumentation, this task cannot prove that a non-owner receives no create packet at runtime. Result is source-only.

## Owner-After-Insertion

- **FACT:** The inline setter only assigns `_privateObjectOwner`; it does not call `UpdateObjectVisibility`.
- **INFERENCE:** Setting ownership after insertion allows the initial `AddToMap` update to run without privacy and requires an explicit later visibility refresh to repair known-client state.
- **RESULT:** Not runtime-tested because no safe fixture existed. This comparison does not weaken `OWNER_BEFORE_INSERTION`.

## Owner Exit

- **FACT:** `_privateObjectOwner` is metadata on the target. The setter/check contains no owner lookup, logout observer, despawn, or map-removal behavior.
- **INFERENCE:** Leaving range, changing map, or logging out changes visibility opportunities but does not by itself establish object deletion. Object lifetime depends on the creation/summon owner and explicit teardown mechanisms.
- **UNKNOWN:** Actual lifetime for the proposed generic Creature and GO fixture, including grid unload and summon-specific cleanup, was not observed.

## Owner Reentry

- **INFERENCE:** If an object survives and retains the same full owner GUID, a re-entered player with that same GUID will again satisfy the owner branch once ordinary map/phase/range constraints hold.
- **UNKNOWN:** Survival, stale-map validity, relog update timing, and duplicate prevention require live testing. Persistence of a stale object must not be treated as desired behavior.

## Map Constraints

- **FACT:** Different maps and incompatible phases are rejected before private ownership. `IsNeverVisibleFor` also rejects targets that are not in world or are destroyed.
- **RESULT:** Map mismatch and not-in-world constraints are `SOURCE_PROVEN`; runtime deferred.

## Always-Visible Paths

- **FACT:** Always-visible checks precede private-owner filtering and can bypass it. Concrete paths include a player's mover/self or farsight target (`Player::CanAlwaysSee`), unit owner/charmer and owner-group relationships (`Unit::IsAlwaysVisibleFor`), GO transports/destructible buildings and GO owner/friendly-unit relationships (`GameObject::IsAlwaysVisibleFor`), and creature AI whose `CanSeeAlways` returns true (`Creature::CanAlwaysSee`).
- **INFERENCE:** Ordinary future Tillers farm creatures and ordinary, unowned non-transport/non-destructible farm GOs do not naturally match those relationships. The fixture must avoid setting conventional owner/charmer relations that independently trigger an early path.
- **CLASSIFICATION:** `NOT_RELEVANT_TO_TILLERS_OBJECTS` for the bounded ordinary Creature/GO design. Reclassify if a future farm object is a transport, destructible building, pet/owned unit, farsight target, conventionally owned GO, or uses AI `CanSeeAlways`.

## Creature / GameObject Parity

- **FACT:** The private-owner field and check live on `WorldObject`, so ordinary Creature and GO targets share the same private filter.
- **FACT:** Both have owner-before-insertion production examples. Their subclass early always-visible rules differ and must be avoided in the fixture.
- **RESULT:** Owner, non-owner, group, phase, and insertion expectations are source-equivalent. Neither type received runtime/client proof; this is an explicit gap.
- **INFERENCE:** Later Tillers units currently require Creature and GameObject only. DynamicObject and other subclasses need no exhaustive T3A testing.

## m_visibilityLayer Confirmation

- **FACT:** Final preservation search found no `m_visibilityLayer` or `visibilityLayer` symbol. Task 036/042 record Andrew's field as unwritten/unconsumed by visibility logic.
- **FACT:** Every current conclusion follows existing owner, map, phase, and subclass visibility paths independently of such a layer. Lack of runtime infrastructure does not motivate adding it.
- **DECISION:** `DO_NOT_PORT_m_visibilityLayer`.

## Existing Debug Facilities

- **FACT:** Existing facilities include temporary GO spawning (`gobject add temp`), persisted GO addition, developer-tool creature/GO selection, player/unit phase mask changes (`modify phase`), GO phase changes, and visibility-distance/update commands (`src/server/scripts/Commands/cs_gobject.cpp`, `cs_modify.cpp`, and `cs_misc.cpp`). Normal player group operations can establish/remove groups.
- **FACT:** No existing GM/debug command found by source search assigns a private owner to a selected Creature or GO. Visibility commands manage distance/activity/reload/update, not private identity.
- **INFERENCE:** Existing commands can prepare phase and request a visibility refresh, but cannot create the essential private-owned pair. This is the exact operational blocker.

## Temporary Instrumentation

- **FACT:** None was created. No temporary patch, binary, or `/tmp` source fixture was used.
- **RATIONALE:** A faithful fixture would require substantial worldserver/map/player/group/data initialization or a disposable server patch. A small standalone copy of the boolean branch would not constitute runtime proof of the repository implementation.

## Packet / Client Observation

- **UNKNOWN:** No server packet instrumentation or build-18414 client was available. Owner create, non-owner suppression, group-join create, group-leave destroy, phase-mismatch destroy, and insertion-time create behavior were not observed.

## Evidence Grades

- `SOURCE_ONLY`: direct current-source control-flow/data-type proof, with no executed object runtime.
- `TEMP_HARNESS`: none.
- `LIVE_SERVER_NO_CLIENT`: none.
- `LIVE_TWO_CLIENT`: none.
- `LIVE_TWO_CLIENT_PLUS_GM`: none.

## Final T3A Matrix

| # | Matrix item | EXPECTED_FROM_SOURCE | ACTUAL_EVIDENCE | RESULT | EVIDENCE_GRADE |
| ---: | --- | --- | --- | --- | --- |
| 1 | owner / private Creature | admitted unless an earlier ordinary rejection applies | shared WorldObject branch; Creature summon orders owner before insertion | consistent, client deferred | `SOURCE_ONLY` |
| 2 | non-owner / private Creature | denied | private check fall-through | consistent, client deferred | `SOURCE_ONLY` |
| 3 | owner / private GO | admitted unless an earlier ordinary rejection applies | shared branch; personal-GO path orders owner before insertion | consistent, client deferred | `SOURCE_ONLY` |
| 4 | non-owner / private GO | denied absent an early GO always-visible relation | private check and GO early-path review | consistent, client deferred | `SOURCE_ONLY` |
| 5 | overlapping phase masks | phase passes; private owner still separates | bitwise-mask and subsequent private-check trace | consistent, client deferred | `SOURCE_ONLY` |
| 6 | phase-incompatible owner | hidden before owner check | `CanNeverSee` order | consistent, client deferred | `SOURCE_ONLY` |
| 7 | same group | non-owner group member admitted | explicit `IsInGroup(ownerGuid)` branch | `GROUP_SHARE_SOURCE_ONLY` | `SOURCE_ONLY` |
| 8 | group removal | admission ceases after visibility reevaluation | membership branch ceases; refresh trigger not established | `GROUP_REFRESH_UNPROVEN` | `SOURCE_ONLY` |
| 9 | different group | denied | exact group-GUID comparisons | consistent, client deferred | `SOURCE_ONLY` |
| 10 | GM non-owner | denied unless another early/group relation admits | private check precedes GM shortcut | client deferred | `SOURCE_ONLY` |
| 11 | controlled unit | controller relation alone denied; explicit same private owner admitted | check uses unit identity before controller substitution | runtime unproven | `SOURCE_ONLY` |
| 12 | same-owner private seer | admitted | explicit same-private-owner branch | consistent, client deferred | `SOURCE_ONLY` |
| 13 | owner before insertion | initial update is owner-filtered | owner set before `AddToMap`; update at end of insertion | required; packets unobserved | `SOURCE_ONLY` |
| 14 | owner after insertion | initial public exposure possible; setter alone does not refresh | setter and insertion control flow | not runtime-tested; do not use | `SOURCE_ONLY` |
| 15 | owner leaves range | target remains private; lifetime separately determined | no lifetime coupling in private field/check | runtime unproven | `SOURCE_ONLY` |
| 16 | owner changes map | map visibility rejects; target lifetime separately determined | map check plus no cleanup coupling | runtime unproven | `SOURCE_ONLY` |
| 17 | owner logout | no generic private-owner cleanup promised | no logout/despawn coupling in API | runtime unproven | `SOURCE_ONLY` |
| 18 | owner reentry if object survives | same full GUID readmitted when ordinary constraints pass | raw GUID equality | survival/update runtime unproven | `SOURCE_ONLY` |
| 19 | map mismatch | hidden | map inequality rejected first | consistent, client deferred | `SOURCE_ONLY` |
| 20 | always-visible relevance | early paths bypass privacy, but ordinary farm Creature/GO should avoid them | concrete subclass path review | `NOT_RELEVANT_TO_TILLERS_OBJECTS` within stated design | `SOURCE_ONLY` |

## T3B Reliance Decision

**Can T3B rely on current private-owner behavior for private farm-object visibility without modifying Object.cpp/Object.h?**

- **CLASSIFICATION:** `PRIVATE_OWNER_RUNTIME_PROOF_DEFERRED`.
- **FACT:** No source contradiction or need for generic Object modification was found.
- **DECISION:** The API remains the intended design, but Task 043 does not supply the required runtime evidence to open the Task 044 gate.

## Group Semantics Decision

- **CLASSIFICATION:** `GROUP_SHARE_RUNTIME_UNPROVEN`.
- **INFERENCE:** Source clearly defines group sharing as current core behavior, and T3B must not override it. Acceptance for implementation gating awaits the external two-client test, not a speculative retail change.

## GM Decision

- **CLASSIFICATION:** `GM_BEHAVIOR_SOURCE_ONLY`.
- **DECISION:** Do not alter GM visibility.

## Controlled-Unit Decision

- **CLASSIFICATION:** `RUNTIME_UNPROVEN`.
- **INFERENCE:** Source indicates `EXPLICIT_OWNER_PROPAGATION_REQUIRED` if T3D/T11 later requires controlled-unit private visibility; it is not needed for T3B's phase lifecycle.

## Object-Lifetime Decision

- **CLASSIFICATION:** `RUNTIME_UNPROVEN`.
- **INFERENCE:** Source indicates `OBJECT_LIFETIME_NEEDS_T3D_DESIGN`; private visibility metadata is not an owner-scoped lifetime facility.

## Task 044 Gate

- **RESULT:** **CLOSED pending external runtime proof.** Source establishes no generic core modification, owner-before-insertion ordering, coherent owner/non-owner filtering, explicit group semantics, and no visibility-layer requirement. However, the task's gate demands trustworthy runtime behavior, including group refresh and initial visibility for both Creature and GO; those points remain unexecuted.
- **DECISION:** Do not implement T3B yet. Run the protocol below on the persistent host and append evidence in a follow-up evidence task.

## Final Classification

- **CLASSIFICATION:** `T3A_SOURCE_PROVEN_RUNTIME_DEFERRED`.
- **RATIONALE:** The current contract is internally consistent and unchanged from Task 042, but no lightweight harness or two-client environment exists here. None of the minimum live cases required for `T3A_RUNTIME_PROVEN` was executed.

## Recommended Next Action

Perform an external, disposable build-18414 validation before Task 044:

### Server requirements

1. Check out exactly `1b13c2843c22d9e0bd5cbf749faaeb55627d0a2e` plus only a separately recorded, disposable test patch; record `git diff` and never merge that patch.
2. Use matching world/auth/characters databases and build-18414-compatible DBC/maps/vmaps/mmaps. Do not apply Andrew Tillers SQL and do not enable farm hooks.
3. Use generic non-Tillers Creature and GO templates already present in the disposable database. The patch may expose a one-off mechanism that creates A/B objects, assigns the requested full owner and phase before insertion, logs raw GUID/counter/mask/map/in-world state, forces `UpdateObjectVisibility` on request, and removes every object at test end. It must not use farm entries or persist rows.
4. Capture server logs and, if already supported, outbound create/destroy update observations. Do not add production commands or reverse engineer packet layouts.

### Client requirements

1. Two separate normal accounts and two unmodified MoP 5.4.8 build-18414 clients; put Players A and B at the same coordinates/map/range.
2. Optional third GM account/client for the GM matrix. Test GM mode off/on while ungrouped, then grouped with A.

### Execution protocol

1. Spawn private Creature A/GO A owned by A and Creature B/GO B owned by B, phase-compatible, setting ownership before insertion. Confirm the four-way owner matrix for each type and that no non-owner receives an initial create.
2. Log full raw owner/seer GUIDs and counters. Where valid GUID construction permits, include identities that would expose a counter-only comparison.
3. Repeat with mask 1 on all participants, then with two Andrew-like masks that share bit 1. Confirm private separation remains.
4. Give A an object with a mask disjoint from A's and confirm even A loses it; restore compatibility and confirm create/update.
5. Group A/B and record whether foreign objects appear automatically. If not, invoke one ordinary visibility update and record that distinction. Remove B and record automatic versus forced destroy. Put A/B in different groups and confirm denial.
6. Exercise GM off/on ungrouped and grouped. Record whether any early always-visible relation, rather than GM mode, explains visibility.
7. With A's controlled unit, test controller relationship alone; if the disposable fixture supports it, assign the unit private owner A and repeat. Test a synthetic A-private seer against another A-private target.
8. In a deliberately separate comparison, insert a generic object without a private owner, observe initial visibility, set owner afterward, and determine whether an explicit update is required. Delete it immediately; retain owner-before-insertion as the production rule.
9. Move A out of range, transfer maps, log out/disconnect, and return/relog if the object survives. Record target lifetime, map presence, owner field, create/destroy transitions, and cleanup separately for Creature and GO.
10. Test a map-mismatched/not-in-world target where fixture-safe. Verify no owner override.
11. Remove all disposable objects, revert/delete the test patch and binary, verify the canonical working tree is clean, and archive the exact patch/log/client observations outside the PR.

**Task 043 complete. The private-owner visibility contract remains source-level consistent, but direct runtime proof was deferred because no lightweight harness/two-client environment is available. No core or Tillers behavior was changed; T3B remains gated on the documented external runtime proof.**
