# Task 034: Battle-Pet Aura Container Reentrancy

## Starting State

- **FACT:** Review started from clean commit
  `8ae8c16307c623efffa0318fc21260ae0e281edf`, the merge of Task 033.
- **FACT:** Task 029's one-line numeric bookkeeping correction remains in
  `BattlePetAura::OnApply`: the aura records `stateEntry->Value`, rather than
  the target's post-application total.
- **FACT:** Tasks 030, 031, 032, and 033 added preservation reports but no
  battle-pet C++ changes. Task 025's early `Finished` guard and immediate
  transition to `Finished` remain at the start of `PetBattle::EndBattle`.
- **FACT:** `/tmp/mop-preservation-dev/CMakeCache.txt` was absent at entry and
  ccache used 0.0 GiB of its 5.0 GiB capacity.

## Aura Container Type

- **FACT:** `BattlePet.h` declares
  `typedef std::list<std::unique_ptr<BattlePetAura>> BattlePetAuraStore;` and
  each `BattlePet` owns a public `BattlePetAuraStore Auras` member.
- **FACT:** The sequence is therefore `std::list`, not the hypothesized
  `std::vector`. Each list node owns a separately allocated `BattlePetAura`
  through `std::unique_ptr`.
- **FACT:** Iterations occur in `PetBattleTeam::{HasAuras,ProcessAuras,
  RemoveExpiredAuras}`, `PetBattle::{EndBattle,AddAura,Kill}`, and
  `BattlePet::GetAura`. `AddAura` is the only append site and uses
  `emplace_back`. `RemoveExpiredAuras` is the only individual erase site.
  `EndBattle` is the only explicit clear site. No aura-container swap, move,
  assignment, splice, or other size-changing operation exists.

## ProcessAuras Traversal

**FACT:** `PetBattleTeam::ProcessAuras` visits every pet in that team's
`BattlePets` order, then uses a range-for over that pet's list:

```cpp
for (auto&& battlePet : BattlePets)
{
    for (auto&& aura : battlePet->Auras)
    {
        if (!aura->HasExpired())
            aura->Process();

        // Prowl and next-attack checks can then Expire() the aura.
    }
}
```

- **FACT:** `HasExpired()` is checked only before `Process`. The two special
  post-process expiration checks still inspect the current allocated object.
- **FACT:** No entry is erased here. After `Process` returns, the same aura is
  inspected for Prowl and indefinite next-attack expiry conditions.
- **FACT:** A C++ range-for evaluates and stores `begin` and `end` once. For a
  `std::list`, however, insertion does not invalidate either iterator, and a
  tail insertion places the new node before the same stable end sentinel.
  Advancing from the formerly last node can therefore reach newly appended
  nodes before reaching the stored end.

## Erase / Cleanup Timing

- **FACT:** `PetBattleTeam::TurnFinished` first calls
  `RemoveExpiredAuras`. That function erases only nodes whose aura reports
  expired, using `erase(itr++)` while walking each pet's list.
- **FACT:** In `HandleRound`, both teams complete `ProcessAuras`, stat updates
  and round-result construction/sending occur, and only then does each team
  run `TurnFinished`. Death/end-of-battle detection follows cleanup.
- **FACT:** `Expire`, maximum replacement, and `Kill` mark auras expired and
  run their expiry behavior; they do not erase list nodes.
- **FACT:** No call from `BattlePetAura::Process` or its effect dispatch reaches
  `TurnFinished`, `RemoveExpiredAuras`, or `EndBattle`.
- **FACT:** Classification: **`NO_ERASE_DURING_PROCESS_PHASE`**. An aura is not
  deleted while the normal synchronous `ProcessAuras` call remains active.

## AddAura Caller Inventory

All direct calls are in `BattlePetAbilityEffect.cpp`:

| Effect/handler | AddAura source | AddAura target | Ordinary cast | Aura `Process` | Current-aura target relationship |
|---|---|---|---|---|---|
| 26 `HandlePositiveAura` | `m_caster` | `m_target` | yes | yes | **SAME_CONTAINER_REACHABLE** |
| 50 `HandleNegativeAura` | `m_caster` | `m_target` | yes | yes | **SAME_CONTAINER_REACHABLE** |
| 54 `HandlePeriodicTrigger` | `m_caster` | `m_target` | yes | yes | **SAME_CONTAINER_REACHABLE** |
| 63 `HandlePeriodicPositiveTrigger` | `m_caster` | `m_target` | yes | yes | **SAME_CONTAINER_REACHABLE** |
| 76 `HandleDamageToggleAura` | `m_caster` | `m_caster` | yes | yes | **METADATA_DEPENDENT**: same container when aura caster and target coincide; otherwise caster container |
| 178 `HandlePowerlessAura` | `m_caster` | `m_target` | yes | yes | **SAME_CONTAINER_REACHABLE** |

- **FACT:** The table is the complete direct `PetBattle::AddAura` caller set.
  Hit/chain/state checks can suppress individual calls but do not change the
  generic target relationship.
- **FACT:** `BattlePetAura::Process` constructs an effect with the aura's
  original caster and explicitly adds the aura's `m_target`. `Execute` assigns
  that supplied pet to effect `m_target` before dispatch. Thus the five
  target-directed aura handlers can append directly to the list currently
  being traversed.
- **FACT:** Ordinary `PetBattle::Cast` can use the same handlers after
  `AddTargets`; this is separate from their proven aura-process reachability.

## Callback-to-Mutation Reachability

**FACT:** The complete generic source path is:

```text
PetBattleTeam::ProcessAuras
 -> BattlePetAura::Process
 -> BattlePetAbilityEffect::Execute
 -> handler 26 / 50 / 54 / 63 / 178
 -> PetBattle::AddAura(m_caster, m_target, ...)
 -> m_target->Auras.emplace_back(...)
```

The `m_target` supplied by `BattlePetAura::Process` is the pet whose aura is
being processed, so this is a supported same-container insertion path. Effect
76 instead inserts on `m_caster`, making same-container insertion conditional
on caster and aura target identity.

**FACT:** `Expire` also invokes `PetBattle::Cast` for
`ON_AURA_REMOVED`. If such a proc dispatches an aura-creating handler, it uses
the same generic `AddAura` path while an outer process traversal may still be
active. Task 031's unresolved caster identity affects which pet receives an
effect, not list iterator validity.

## Build-18414 Row Evidence

- **FACT:** Repository searches found runtime DB2 declarations and schema
  comments, but no extracted/authenticated build-18414 ability-effect and turn
  rows establishing a concrete recursive aura-creation example.
- **UNKNOWN:** No ability ID is asserted for a same-list runtime occurrence.
- **FACT:** The generic data-driven API nevertheless permits and directly
  implements the path above.

`SAME_VECTOR_RUNTIME_EVIDENCE = SOURCE_REACHABLE` (the historical label says
“vector”; the actual same-container sequence is a list).

## C++ Invalidation Rules

- **FACT:** The proposed `std::vector` hazard does not apply. A vector
  `push_back` that reallocates invalidates all vector iterators and references
  to vector elements; without reallocation it still invalidates `end()`.
  Relocating `unique_ptr` elements would not relocate their separately
  allocated pointees, so raw pointee pointers could remain valid absent erase.
- **FACT:** Current `std::list::emplace_back` invalidates no iterator or
  reference to existing elements. It allocates a new node; existing
  `unique_ptr` objects and `BattlePetAura` pointees do not move.
- **FACT:** The range-for iterator and cached list end remain valid across any
  number of appends or `Expire` calls. No iterator/reference undefined
  behavior results from current supported mutation.

## Replacement Path

- **FACT:** When `maxAllowed` is reached, `AddAura` calls `Expire` on enough
  active same-ability auras and then appends the new aura. It does not erase
  or shrink the list before appending.
- **FACT:** The currently processing aura can match and expire itself. It
  remains allocated, and the already-entered `BattlePetAura::Process`
  continues after nested `Expire` returns. The separate self-expiry
  continuation policy is outside this task.
- **FACT:** Append does not invalidate the outer list iterator. The new tail
  node is eligible to be reached later by the same list traversal under
  current list semantics. Replacement leaves size unchanged during expiry,
  then increases it by one during append.

## Death Path

- **FACT:** Damage can call `PetBattle::Kill`. `Kill` resets a dead active
  pet's active ability, range-iterates every victim aura and calls `Expire`,
  updates the dead state, and sets the round result.
- **FACT:** `Kill` neither erases nor clears the victim's aura list. Iteration
  may subsequently encounter expired nodes, which the pre-process check skips;
  the post-process special checks still inspect those allocated nodes.
- **FACT:** Death therefore creates no independent iterator invalidation.
  Removal-proc aura appends remain list-safe.

## EndBattle Path

- **FACT:** `EndBattle` calls `OnExpire` for each contained aura and then
  clears each pet's list. Its Task 025 guard makes the transition to
  `Finished` exactly once.
- **FACT:** Normal `HandleRound` invokes `EndBattle` only after both aura
  phases, round sending, both `TurnFinished` cleanup calls, round state reset,
  and the all-dead scan. No effect handler calls `EndBattle`.
- **FACT:** Other direct call sites are update/control paths, including leave
  processing, interruption, and explicit forfeit; none is a synchronous
  callback from aura processing. Normal battle processing is serialized on
  the world thread, and `EndBattle` refuses a non-world-thread finalization by
  setting `Interrupted` and returning.
- **FACT:** The current aura callback graph cannot synchronously clear the list
  traversed by `ProcessAuras`.

## Same-Container Scheduling

- **FACT:** Unlike the task's hypothesized vector behavior, appending to a
  list before its stable end sentinel makes the new tail node reachable by
  the existing traversal. Newly appended same-pet auras can therefore process
  in the same aura phase.
- **FACT:** This behavior is defined, not an artifact of invalidated
  iterators. Multiple appends are likewise encountered in insertion order if
  processing reaches them. Data capable of recursively creating an unbounded
  sequence could prolong or fail to terminate the phase; no authenticated row
  establishing that behavior was found.
- **INFERENCE:** Changing to an initial-size or pointer snapshot would defer
  these auras and select a new gameplay scheduling rule. Build-18414 evidence
  would be required before making that change.

## Cross-Pet Scheduling

- **FACT:** `HandleRound` processes the first-attacking team, then the second.
  Within a team, `ProcessAuras` visits every roster pet in `BattlePets` order,
  not only the active pet.
- **FACT:** An aura appended to a pet whose list traversal has not begun is
  eligible later in the same phase. An append to a previously traversed pet is
  deferred until the next phase. An append to the list currently being
  traversed is eligible later in the current traversal.
- **FACT:** A per-pet initial-size/snapshot repair would change only the last
  case, producing a hybrid policy. A global phase snapshot would additionally
  change later-pet and later-team scheduling. Neither policy is justified by
  current row evidence.

## Minimum Safety Invariant

**FACT:** Current code already meets the narrow memory invariant: callbacks
may append list nodes or mark any aura expired without invalidating the active
list traversal, and deletion is deferred until after both processing phases.

**INFERENCE:** Termination and first-tick scheduling are gameplay-policy
questions distinct from iterator/lifetime safety. They cannot be changed as a
surrogate for a nonexistent vector defect.

## Candidate Repair Designs

| Design | Iterator/lifetime safety | Scheduling and cost | Result |
|---|---|---|---|
| A: capture initial size, index loop | Lists have no random access; advancing from begin for every index is awkward/O(n²), while a moving iterator plus count is safe | Defers same-list appends and changes current defined behavior | Reject |
| B: snapshot `BattlePetAura*` | Pointees remain alive because erase/clear is deferred | Allocates; defers same-list appends; preserves original-object order | Safe mechanically, unjustified semantic change |
| C: snapshot IDs and re-find | Safe under deferred erase | Allocates and repeatedly scans; IDs are only container-local; same scheduling change | Reject |
| D: change container type | Could introduce the very invalidation hazard under review | Broad ownership/complexity/scheduling change | Reject |
| E: queue `AddAura` insertion | Can be engineered safely | Broad changes to apply events, `OnApply`, replacement, state timing, and visibility | Reject |

**FACT:** No repair is required for iterator safety. Designs A--C would impose
snapshot scheduling contrary to the current list traversal; D and E are much
broader than the established problem.

## Initial-Size Index Analysis

- **FACT:** Literal indexed access is unavailable for `std::list`.
- **FACT:** A count-bounded iterator traversal could retain validity because
  appends and expiry do not erase, and it could exclude all later appends.
- **FACT:** Self-expiry and expiry of earlier/later nodes preserve node
  lifetime until cleanup. Multiple appends remain safe.
- **INFERENCE:** Although mechanically sound, count-bounding would change
  same-container scheduling from same-phase eligibility to next-phase
  eligibility, so it is not a semantics-preserving safety repair.

## Raw-Pointer Snapshot Analysis

- **FACT:** A snapshot of `BattlePetAura*` would be lifetime-safe in the
  reviewed round path: list insertion does not move pointees, expiry does not
  delete them, cleanup is later, and `EndBattle` is not callback-reachable.
- **FACT:** Null `unique_ptr` entries are not inserted by current code, and the
  list is publicly accessible but has no other mutation site in the
  repository.
- **INFERENCE:** This extra allocation still changes defined scheduling and is
  unnecessary for safety.

## Expired / Null Entry Handling

- **FACT:** Expired but allocated entries are normal between `Expire` and
  `TurnFinished`; duplicate ability IDs are also permitted subject to the
  current Task 033 maximum policy. Aura instance IDs are recomputed to be
  unique among contained entries.
- **FACT:** No current insertion creates a null `unique_ptr`; current loops
  assume non-null and do not test it.
- **FACT:** No traversal change is made, so existing behavior remains: expired
  entries skip `Process`, allocated entries remain dereferenceable, and newly
  appended same-list entries retain current same-phase eligibility.

## Event-Order Preservation

- **FACT:** With no source change, ordinary processing order and all
  `AURA_APPLY`, `AURA_CHANGE`, `AURA_REMOVE`, and state-event order remain
  byte-for-byte as generated by current code.
- **FACT:** Same-list append remains apply/`OnApply` immediately, followed by
  possible later same-phase processing in list insertion order. Cross-list
  append retains its position-dependent eligibility described above.

## Task 029 Interaction

- **FACT:** No state bookkeeping changes. Each expired aura remains allocated
  until cleanup; guarded `Expire` runs `OnExpire` once, and Task 029's recorded
  numeric contribution is subtracted once.

## Task 031 Interaction

- **FACT:** Removal-proc casting remains target-as-caster. Such a proc may
  append through the reviewed handler path, but list insertion remains safe.
  No caster identity policy is selected.

## Task 032 Interaction

- **FACT:** Natural expiration remains before effect processing at the current
  `m_turn > m_maxDuration` boundary. No D/D+1 choice is changed.
- **FACT:** A newly appended list aura that is reached this phase receives its
  current first `Process` opportunity; this report records but does not alter
  that consequence.

## Task 033 Interaction

- **FACT:** `maxAllowed`, the target-wide same-ability counting key,
  replacement frequency/order, P3 versus literal-one arguments, failure-event
  ordering, and deferred erase are unchanged.
- **FACT:** Replacement can expire current or other nodes and append one node
  without invalidating the current list iterator.

## Data / Scheduling Requirement

- **FACT:** No build-18414 gameplay assumption is needed to conclude that
  current `std::list` insertion and mark-only expiry are memory-safe.
- **UNKNOWN:** Deferring newly added same-container auras, bounding recursive
  processing, or globally snapshotting an aura phase would change first-tick
  scheduling and requires build-18414 rows/traces or another authenticated
  contract. None is available locally.

## Final Classification

**`CURRENT_TRAVERSAL_SAFE`.** The review premise assumed a vector, but the
actual container is a node-based `std::list`. Supported appends and expirations
do not invalidate the range-for traversal, and deletion/clear is not reachable
during the process phase. The list also gives newly appended same-container
auras defined same-phase eligibility. A snapshot would change that established
behavior rather than merely remove undefined behavior. No C++ source is
modified.

Remaining A7 questions are the intended build-18414 first-tick/recursive aura
scheduling policy, authenticated rows exercising aura creation or removal-proc
creation during aura processing, delayed harmful activation, natural expiry
timing, removal-proc caster identity, boolean ownership, weather, and broader
effect semantics.

## Validation Plan

- **FACT:** Documentation validation uses focused source/history inspection,
  `git diff --check`, and the static matrix below. No translation-unit or
  aggregate compilation applies because no C++ source changed.
- **FACT:** Static mutation matrix: (1) zero auras is an empty safe traversal;
  (2--3) one/several unchanged auras retain list order; (4--6) expiring the
  current, later, or earlier aura marks nodes without invalidation; (7--9)
  same-target append with any allocator behavior and any count preserves all
  existing list iterators and appends in order; (10) replacement marks then
  appends safely; (11) kill marks victim nodes only; (12) a removal proc may
  append safely; (13) insertion into an already processed pet waits; (14)
  insertion into a not-yet-processed pet is eligible this phase; (15) expired
  nodes remain allocated until `TurnFinished`; (16) `EndBattle` follows the
  aura phase; (17) Task 029 numeric reversal is unchanged; (18) Task 031 caster
  semantics are unchanged; (19) Task 032 expiry semantics are unchanged; and
  (20) Task 033 stack semantics are unchanged.
- **FACT:** No lightweight unit fixture exists that can instantiate this
  data-store-driven battle/aura callback graph without broad infrastructure.
  No micro test is added for a nonexistent vector-growth defect.
