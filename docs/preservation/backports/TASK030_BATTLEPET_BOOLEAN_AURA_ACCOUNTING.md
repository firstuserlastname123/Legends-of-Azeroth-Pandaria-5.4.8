# Task 030: Battle-Pet Boolean Aura Contribution Accounting

## Starting State

- **FACT:** Review began with a clean working tree on task branch `work` at
  `56c8ae0c780adc6f6e92d110812a857e3edced0a`. The history contains Task 029's
  `7d37db0` implementation and the subsequent Task 028 pull-request merge.
- **FACT:** Task 025's `PetBattle::EndBattle` guard still returns when the
  battle is already `Finished` and sets `Finished` before finalization effects.
- **FACT:** `/tmp/mop-preservation-dev/CMakeCache.txt` was absent. Initial
  ccache use was 0.0 GiB of 5.0 GiB. No build tree was configured because this
  review does not authorize a source change.
- This task is limited to A7B1 boolean/clamped aura-state accounting. It does
  not change weather, effect 80/169, delayed auras, expiry timing, removal-proc
  identity, general stacking policy, trainers, direct-effect handlers, SQL,
  Playerbots, or opcodes.

## Task 029 Baseline

**FACT:** `BattlePetAura::OnApply` visits every `BattlePetAbilityState` row for
the aura ability. For each row it computes `requested = before + Value`, adds
raw `Value` to the aura-private state map, and calls `UpdatePetState` with the
requested absolute value. `OnExpire` computes `current - recorded` and passes
that absolute value to `UpdatePetState`, then clears the map.

This is Task 029's correct additive invariant for ordinary integer states:
each aura removes its raw additive modifier rather than the post-application
total. The map remains private reversal bookkeeping and is not serialized.

## State Modify Types

**FACT:** Current source defines exactly three modify types:

| Value | Type | `UpdatePetState` behavior |
| ---: | --- | --- |
| 0 | `NONE` | Rejects the update before reading/writing the target state or queuing an event. |
| 1 | `BOOL` | Normalizes the requested value to zero or one, then applies unchanged suppression. |
| 2 | `INT` | Uses the requested `int32` value unchanged, then applies unchanged suppression. |

**FACT:** The fixed 163-entry source table is the only modify-type dispatch in
`UpdatePetState`. A missing `BattlePetState.db2` row also rejects the update.
There is no non-boolean lower bound, upper bound, percentage clamp, saturation,
or enum normalization in this function.

## Boolean States

**FACT:** The current table marks the following 59 IDs `BOOL`. Names are shown
only where the current enum supplies one; `StateNNN` is the source table's own
label, not an inferred DB2 name. Every row has the same normalization behavior:
requested `< 1` becomes 0, requested `>= 1` becomes 1, and an unchanged result
is stored neither again nor emitted.

| State ID | Current-source name | Type |
| ---: | --- | --- |
| 1 | `BATTLE_PET_STATE_IS_DEAD` | `BOOL` |
| 21 | `BATTLE_PET_STATE_MECHANIC_POISONED` | `BOOL` |
| 22 | `BATTLE_PET_STATE_MECHANIC_STUNNED` | `BOOL` |
| 28 | `BATTLE_PET_STATE_CONDITION_WAS_DAMAGED_THIS_ROUND` | `BOOL` |
| 29 | `BATTLE_PET_STATE_UNTARGETABLE` | `BOOL` |
| 30 | `BATTLE_PET_STATE_UNDERGROUND` | `BOOL` |
| 33 | `BATTLE_PET_STATE_MECHANIC_FLYING` | `BOOL` |
| 34 | `BATTLE_PET_STATE_MECHANIC_BURNING` | `BOOL` |
| 35 | `BATTLE_PET_STATE_TURN_LOCK` | `BOOL` |
| 36 | `BATTLE_PET_STATE_SWAP_OUT_LOCK` | `BOOL` |
| 42 | `BATTLE_PET_STATE_PASSIVE_CRITTER` | `BOOL` |
| 43 | `BATTLE_PET_STATE_PASSIVE_BEAST` | `BOOL` |
| 44 | `BATTLE_PET_STATE_PASSIVE_HUMANOID` | `BOOL` |
| 45 | `BATTLE_PET_STATE_PASSIVE_FLYING` | `BOOL` |
| 46 | `BATTLE_PET_STATE_PASSIVE_DRAGON` | `BOOL` |
| 47 | `BATTLE_PET_STATE_PASSIVE_ELEMENTAL` | `BOOL` |
| 48 | `BATTLE_PET_STATE_PASSIVE_MECHANICAL` | `BOOL` |
| 49 | `BATTLE_PET_STATE_PASSIVE_MAGIC` | `BOOL` |
| 50 | `BATTLE_PET_STATE_PASSIVE_UNDEAD` | `BOOL` |
| 51 | `BATTLE_PET_STATE_PASSIVE_AQUATIC` | `BOOL` |
| 52 | `BATTLE_PET_STATE_MECHANIC_CHILLED` | `BOOL` |
| 53 | `State053` | `BOOL` |
| 54 | `State054` | `BOOL` |
| 55 | `State055` | `BOOL` |
| 56 | `State056` | `BOOL` |
| 57 | `State057` | `BOOL` |
| 58 | `State058` | `BOOL` |
| 59 | `State059` | `BOOL` |
| 60 | `State060` | `BOOL` |
| 61 | `State061` | `BOOL` |
| 62 | `State062` | `BOOL` |
| 63 | `State063` | `BOOL` |
| 64 | `BATTLE_PET_STATE_MECHANIC_WEBBED` | `BOOL` |
| 67 | `BATTLE_PET_STATE_MECHANIC_INVISIBLE` | `BOOL` |
| 68 | `BATTLE_PET_STATE_UNKILLABLE` | `BOOL` |
| 69 | `State069` | `BOOL` |
| 70 | `State070` | `BOOL` |
| 77 | `BATTLE_PET_STATE_MECHANIC_BLEEDING` | `BOOL` |
| 82 | `BATTLE_PET_STATE_MECHANIC_BLIND` | `BOOL` |
| 84 | `State084` | `BOOL` |
| 85 | `State085` | `BOOL` |
| 91 | `State091` | `BOOL` |
| 93 | `State093` | `BOOL` |
| 98 | `BATTLE_PET_STATE_SWAP_IN_LOCK` | `BOOL` |
| 113 | `State113` | `BOOL` |
| 114 | `State114` | `BOOL` |
| 120 | `State120` | `BOOL` |
| 122 | `State122` | `BOOL` |
| 123 | `State123` | `BOOL` |
| 126 | `State126` | `BOOL` |
| 127 | `BATTLE_PET_STATE_CONDITION_DID_DAMAGE_THIS_ROUND` | `BOOL` |
| 128 | `State128` | `BOOL` |
| 129 | `State129` | `BOOL` |
| 136 | `BATTLE_PET_STATE_MECHANIC_BOMB` | `BOOL` |
| 144 | `State144` | `BOOL` |
| 149 | `BATTLE_PET_STATE_RESILITANT` | `BOOL` |
| 153 | `State153` | `BOOL` |
| 158 | `State158` | `BOOL` |
| 162 | `State162` | `BOOL` |

All remaining non-`NONE` table entries are `INT` and receive no normalization.

## Clamp Semantics

**FACT:** The exact boolean transformation is
`value = value >= 1 ? 1 : 0`. It occurs after DB2-row and modify-type
validation, and before the unchanged-value check. Thus zero and every negative
request normalize to false; every positive request normalizes to true.

## Symbolic Boolean Matrix

The expiry column starts from the just-stored value, subtracts Task 029's raw
record, then applies the same boolean clamp.

| Base | Raw modifier | Requested | Stored | Actual delta | Task 029 record | Task 029 expiry | Exact isolated restore? |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 0 | +1 | 1 | 1 | +1 | +1 | 0 | yes |
| 1 | +1 | 2 | 1 | 0 | +1 | 0 | **no** |
| 0 | -1 | -1 | 0 | 0 | -1 | 1 | **no** |
| 1 | -1 | 0 | 0 | -1 | -1 | 1 | yes |
| 0 | +2 | 2 | 1 | +1 | +2 | 0 (request -1) | yes only by clamp, but record overstates ownership |
| 1 | -2 | -1 | 0 | -1 | -2 | 1 (request 2) | yes only by clamp, but record overstates ownership |

**FACT:** Task 029 can therefore subtract a raw contribution that was never
applied. The two highlighted single-aura cases prove the arithmetic defect if
such a row is supported. The apparently restoring magnitude-two cases do not
make raw ownership valid; the second clamp merely hides the oversized record.

## Aura-to-BOOL Evidence

- **FACT:** `BattlePetAbilityStateEntry` contains unrestricted `AbilityId`,
  `StateId`, and `Value` fields. `BattlePetAura::OnApply` does not filter state
  IDs by modify type. It can mechanically reach any valid state row.
- **FACT:** `AddAura` constructs an aura and immediately calls `OnApply`; no
  state metadata is consulted before that call.
- **FACT:** This repository does not contain extracted build-18414 DB2 files or
  a row dump that joins a triggered aura ability to a boolean state. Prior
  preservation research found only the DB2 shapes, not authoritative rows.
- **UNKNOWN:** No authenticated build-18414 `BattlePetAbilityState` row was
  recovered during this review to prove one concrete aura-to-BOOL pairing.

Classification: **`SUPPORTED_BY_DATA_MODEL`**, but not `PROVEN_CURRENT_ROW`.
The path is expressly accepted by the generic loader/runtime contract; its
actual build-18414 population remains a data-evidence gap.

## Non-Boolean Normalization

**FACT:** `INT` is passed through unchanged. `NONE`, an invalid/missing state
row, and an unchanged normalized value return without a write or event. Only
`BOOL` transforms a valid requested value. The structure's `Value` field is
`uint32` despite signed DB2 format metadata; that existing conversion concern
does not create another normalization rule in `UpdatePetState`.

## Actual-Contribution Model

For one attempted mutation, define `before`, call `UpdatePetState` with
`requested = before + raw`, then read `after`; `after - before` exactly measures
the mutation that this call caused. It gives the raw modifier for ordinary
non-overflowing integer updates, zero for unchanged/clamped no-ops, and the
actual signed transition for a boolean update. It also gives zero when a
valid request is unchanged.

However, **INFERENCE:** a historical delta is not necessarily what a boolean
aura *owns*. It captures a transition, not continuing demand. If validation
rejects an invalid state or `NONE`, reading the array afterward would happen to
produce zero delta only when the index is safe; an out-of-range state is
already indexed by `OnApply` before `UpdatePetState`, so a generic read-back
patch would not repair that separate trust boundary.

Consequently actual-delta recording is locally accurate but not a sufficient
boolean ownership model.

## Multiple Rows

For ordinary integers, accumulating each `afterN - beforeN` equals the sum of
the raw modifiers and preserves Task 029. For repeated boolean true requests
from one aura, the first row may contribute +1 and subsequent rows contribute
zero. Reversing total actual delta restores the isolated starting value.

**INFERENCE:** This works only while that aura is the sole boolean claimant.
It does not prove that duplicate rows mean independent claims, nor does it
protect a claim introduced by another aura between apply and expiry.

## Multiple Aura Instances

For base false, Aura A requests true, then Aura B requests true:

| Step | State | Actual-delta ownership proposal |
| --- | ---: | --- |
| Base | 0 | none |
| Apply A | 1 | A records +1 |
| Apply B | 1 | B records 0 |
| Expire A first | 0 | subtract A's +1: **B's continuing request is lost** |
| Then expire B | 0 | subtract zero |

If B expires first, its zero subtraction is harmless and A later restores
false. The result is therefore expiration-order dependent. Raw Task 029
accounting also fails overlapping flags, though differently: each claimant
records +1 even where the clamp applied no second increment.

**FACT:** Current `AddAura` supports multiple aura objects; its `maxAllowed`
policy is ability-specific and does not establish uniqueness across different
abilities that address the same state.

## Boolean Ownership Semantics

The current core stores only one absolute integer per pet/state and a private
delta map per aura. It has no per-state source list, reference count, boolean
claim registry, or recomputation of a state from active auras. Active auras
can be enumerated, but there is no existing routine that derives a boolean
state from all aura, direct, initial, and lifecycle contributors.

**INFERENCE:** A flag that remains true while any aura requests true requires
reference-counting or recomputation, while negative/false-setting modifiers
and non-aura contributors require an even more explicit precedence/base
contract. Current source does not establish whether retail semantics are
reference-counted flags, last-writer state, restoration, or another model.

Therefore a one-line actual-delta patch is unsafe. Correctness requires a
broader boolean-state ownership model plus build-18414 semantic evidence; this
task is prohibited from introducing that subsystem.

## Direct-Cast Interaction

**FACT:** `PetBattle::Cast`, on turn zero, applies every matching
`BattlePetAbilityState` row directly to the caster through the same additive
request and `UpdatePetState`. It records no owner and has no later reversal.
`HandleSetState` is another direct absolute writer through `UpdatePetState`.

Thus a direct contributor can make a boolean true before an aura applies. An
actual-delta aura records zero in that case, which preserves the current true
value on expiry. But if a direct writer changes the flag after aura application,
subtracting the aura's historical transition can still erase that newer
writer. Whether direct ability-state changes are battle-permanent or temporary
cannot be recovered from ownership metadata because none exists.

## Other State Writers

**FACT:** Current contributors include:

- pet initialization: family passives, death, breed and species state rows;
- direct ability-state rows in `Cast`;
- direct `HandleSetState` and ramping-state effect handlers;
- aura `OnApply`/`OnExpire`;
- damage bookkeeping and round reset for the two damage-condition flags;
- `Kill` for the death flag;
- resurrection-style direct death-state writes in `BattlePet`;
- end-battle mechanic reset, which directly clears named mechanic flags.

Swap checks consume lock flags but do not establish their ownership. Capture
uses `Kill`. Initial family passive flags demonstrate a concrete independent
boolean base, although no build-18414 aura row targeting them is proven.
Several direct assignments bypass `UpdatePetState` and therefore emit no
`SET_STATE` from that write. Aura expiration cannot distinguish any of these
sources from its own contribution by inspecting the one absolute state value.

## Client Event Contract

**FACT:** A successful `UpdatePetState` queues an absolute
`PET_BATTLE_EFFECT_SET_STATE` value. Invalid, `NONE`, and unchanged normalized
updates queue nothing. Private bookkeeping itself has no packet representation.

An actual-delta-only change would preserve first-application calls and event
ordering, and would suppress/emit exactly the same apply event because the
normalization remains in `UpdatePetState`; it would change some expiry absolute
values and potentially whether expiry is a no-op. But because overlap/direct
ownership is unresolved, those changed expiry values cannot be certified as
correct. No packet layout or opcode change is needed; event-production
semantics, not serialization, are the blocker.

## Andrew Comparison

**FACT:** Final Andrew at `bd8ad5515418d94abd35ba7bf71430979243a09c`
retains the Task 029 formula: record raw `stateEntry->Value`, call the unchanged
clamping `UpdatePetState`, and subtract the raw record on expiry. It has no
boolean special case, read-back actual delta, or broader per-state ownership
model. Classification: **`RAW_MODIFIER_ONLY`**.

## Flamehawk Comparison

**FACT:** Flamehawk final relevant state at
`e91819a4c2acb6cc8eea09ed264d30aec6ed12b9` records the post-application
`newValue`, the pre-Task-029 defect, then subtracts it on expiry. It has no
boolean special case or broader ownership model. Classification:
**`UNRESOLVED`** (and independently incorrect for nonzero additive bases).

## Candidate Fix Designs

| Option | Evaluation |
| --- | --- |
| A: actual applied delta for all types | Locally correct for isolated mutations and preserves integer behavior, but expiration-order dependent for overlapping boolean claimants and unsafe against later direct writers. Not sufficient. |
| B: actual applied delta only for `BOOL` | Has the same ownership failure as A, adds type-special bookkeeping, and still lacks retail semantics. Not sufficient. |
| C: leave Task 029 because BOOL aura rows are impossible | Rejected as a conclusion: the data model/runtime explicitly permit the path, while repository data is insufficient to prove either population or impossibility. |
| D: broader boolean ownership/reference model | The only design family capable of preserving overlapping continuing claims, but negative writers, direct writers, initial bases, reset policy, and event semantics need build-18414 evidence. Out of scope for an isolated fix. |
| E: original-value restoration | Erases later contributors and is expiration-order dependent; incorrect as a generic model. |

## Final Classification

**`NEEDS_BROADER_BOOLEAN_STATE_MODEL`.** No core source was changed.

The current raw-modifier bookkeeping is demonstrably not the actual applied
delta for clamped requests, and the generic data model supports aura-to-boolean
rows. Nevertheless the decision gate for an isolated repair fails: recording
actual delta cannot preserve overlapping boolean auras when the first claimant
expires first, and it cannot reliably distinguish initial, direct-cast,
direct-effect, round-reset, kill/death, and aura contributors. Current source
does not establish the correct retail ownership/precedence semantics.

Task 029 remains valid unchanged for additive `INT` states. Its report
correctly deferred boolean clamping; this review does not revert or weaken its
numeric repair. Remaining A7 work includes a source-supported boolean ownership
contract and build-18414 rows/traces, plus the separately excluded delayed-aura,
expiration-timing, removal-proc-caster, and stacking-policy reviews.

## Validation Plan

- **FACT:** Documentation was checked with `git diff --check`; no C++ source
  changed, so translation-unit and aggregate compilation are not applicable.
- **FACT:** Static matrix results: numeric positive/negative cases remain valid
  under Task 029; isolated boolean transitions expose raw-record defects;
  duplicate same-aura rows can be measured locally; two boolean auras fail
  actual-delta accounting when the first claimant expires first; direct and
  later writers remain indistinguishable; death and `EndBattle` use the same
  reversal map; client values remain absolute.
- A future implementation must first establish build-18414 boolean rows and
  semantics, represent all relevant owners (including direct/base sources),
  pass both aura expiration orders and direct-writer interleavings, preserve
  numeric Task 029 behavior, and retain existing absolute `SET_STATE` layout.
