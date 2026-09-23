# Task 047 — Sunsong Ranch world-data baseline contract

## 1. Starting state

| Check | Recorded result |
|---|---|
| Initially checked-out branch and SHA | `work` at `37648e1cb8d4d3ddb86735e9ea4eea99ee049e65` |
| Latest fetched `origin/preservation/main` | `37648e1cb8d4d3ddb86735e9ea4eea99ee049e65` |
| Work tree before research | clean (`git status --porcelain` emitted nothing) |
| Merged Task 046 report | present at `docs/preservation/backports/TASK046_TILLERS_WORLD_DATA_PROVENANCE.md` |
| Task 044 fixture | absent (`find . -name '*.t3atest'` found nothing) |
| Task branch | `codex/task-047-sunsong-baseline-contract`, created directly from the fetched integration tip |

The checkout initially had no configured remote. `origin` was set to the preservation
fork before fetching and recording the integration SHA. **FACT — HISTORY:** the task
therefore begins at the requested merged Task 046 tip, not from Task 044, `master`,
or another transient branch.

## 2. Purpose and non-goals

This document and
[`TASK047_SUNSONG_BASELINE.tsv`](../data/TASK047_SUNSONG_BASELINE.tsv) define a
recognition contract for the world database LoA published before Andrew's changes.
They do not claim that the snapshot is retail-correct. They do not implement a
classifier, transition SQL, gameplay behavior, replacement phasing, or Tillers
runtime behavior. No production C++, production SQL, database update, server,
client, import, or gameplay test is part of Task 047. Task 045/T3B remains blocked
pending an explicit build-18414 runtime-validation decision.

The contract answers one narrow question: which rows, absences, and exact
cardinalities must a target satisfy before it may be called a match for the known
LoA-distributed pre-Andrew Sunsong subset? `BASELINE_MATCH` is a provenance label,
not a correctness endorsement.

## 3. Authoritative source and extraction boundary

**FACT — DB:** The immediate preservation basis is Task 046 and the same LoA release
asset: tag `WDB20231230`, displayed release name `WORLD_DB_2024-07-22`, archive
`world_548_20240722.7z` (75,143,384 bytes; SHA-256
`b14a1ebe7415f3135e771862af94139e7944406a636b9d107d2ab05e15905780`), containing
one 481,354,136-byte `world_548_20240722.sql`. The SQL was expanded under `/tmp` and
parsed as text; it was not imported into MySQL and no downloaded artifact is
committed.

The manifest is generated from complete snapshot-table scans, restricted as follows:

* creatures: entries 58646, 58721, 58719, 55626, 59669, and 66129 on map 870,
  zone 5805, area 6039;
* gameobjects: the nine required obstacle entries and the required
  214036–214056, 215705, 215706, 215719, 215720, and 210993 candidates on the same
  map/zone/area tuple;
* all matching tutorial quest tuples in the two complete relation tables; and
* the complete `phase_definitions` table for zone identifiers 1023, 5805, and 5840.

Historical migrations are used below only to name proven supersession. Snapshot
rows, not migration intent, define the baseline.

## 4. Manifest schema and interpretation

The TSV is UTF-8, LF-terminated, has one header and 105 data rows, and uses these
stable columns:

| Column | Contract meaning |
|---|---|
| `category` | `TUTORIAL_NPC`, `CREATURE_REFERENCE`, `OBSTACLE`, `PLOT_SOIL_REFERENCE`, `QUEST_RELATION`, or `PHASE_DEFINITION_ABSENCE` |
| `table` | Expected world table |
| `identity_key` | Unique manifest key: GUID, exact relation tuple, or zone-wide absence predicate |
| `entry`, `guid` | Snapshot identifiers; blank when inapplicable |
| `map`, `zone`, `area`, `phase_mask` | Exact integral identity fields |
| `position_x/y/z`, `orientation` | Snapshot numeric spellings, with no invented precision |
| `relation_type`, `quest_id` | Explicit `STARTER`/`ENDER` and quest identity |
| `expected_count` | Exact count for the entry/group or absence predicate, repeated on member rows intentionally |
| `source` | Direct source snapshot |
| `confidence` | Evidence classification; semicolon-delimited when multiple labels apply |
| `notes` | Template identity or a constraint against semantic overreach |

Blank means “not applicable,” never zero. Manifest keys are unique. Multiple rows
with one entry are legitimate distinct GUID records, and their repeated group count
makes cardinality independently consumable by future validation.

## 5. Yoon and adjacent-creature baseline

**FACT — DB:** The bounded creature set contains 36 rows:

| Entry | Exact count | GUIDs | Baseline phase | Classification |
|---:|---:|---|---:|---|
| 58646 | 2 | 516066, 516067 | 1 | Farmer Yoon snapshot identities |
| 58721 | 1 | 516061 | 1 | tutorial/reference NPC |
| 58719 | 8 | 516057–516060, 516062–516065 | 1 | rock/reference creatures; semantics not inferred |
| 55626 | 16 | 516075, 516076, 516089, 516091–516102, 516111 | 1 | adjacent reference candidates |
| 59669 | 1 | 516078 | 1 | adjacent reference candidate |
| 66129 | 8 | 516079–516085, 516110 | 1 | adjacent reference candidates |

The exact coordinates and orientations for every GUID are in the manifest. In
particular, GUID 516066 is `(-158.578,608.821,175.643,2.14675)`, GUID 516067 is
`(-180.844,628.358,165.493,1.85448)`, and GUID 516061 is
`(-159.463,633.603,165.409,5.19326)`.

**UNKNOWN:** Two snapshot Yoons do not prove that two simultaneously presented
Yoons are retail-correct. The 55626/59669/66129 rows are included because Task 046
established them as directly adjacent ranch/reference population; their gameplay
roles are not assigned here.

## 6. Quest-link baseline

**FACT — DB:** Eight exact tuples, each with cardinality one, form the relation
contract:

| Relation table | Entry | Quests | Tuple count |
|---|---:|---|---:|
| `creature_queststarter` | 58721 | 30252 | 1 |
| `creature_queststarter` | 58646 | 30254, 30255, 30256 | 3 |
| `creature_questender` | 58646 | 30252, 30254, 30255, 30256 | 4 |

The baseline contains no `(58646,30252)` starter tuple. For this contract, its
presence is a conflicting or specifically recognized Andrew addition, never a
baseline alternative. Static relationship rows do not prove client presentation or
quest functionality.

## 7. Obstacle baseline

**FACT — DB:** All 27 bounded obstacle spawns are phase 1 on map 870 / zone 5805 /
area 6039. Every GUID and coordinate is recorded in the manifest.

| Entry | Template identity | Exact count | GUIDs |
|---:|---|---:|---|
| 210443 | Weed | 2 | 514484, 514495 |
| 210444 | Weed | 5 | 514450, 514452, 514455, 514482, 514488 |
| 210445 | Weed | 3 | 544507–544509 |
| 210446 | Weed | 4 | 514453, 514454, 514486, 514494 |
| 210447 | Weed | 7 | 514448, 514470, 514485, 514487, 514492, 514493, 514498 |
| 210448 | Weed | 2 | 514447, 514449 |
| 210462 | Weed | 2 | 514496, 514497 |
| 210451 | Broken Wagon | 1 | 514483 |
| 209572 | Giant Rock | 1 | 514489 |

The names are direct `gameobject_template` identities. Presence proves identity and
location in the distributed snapshot, not intended retail visibility or obstacle
progression.

## 8. Plot, soil, and reference inventory

**FACT — DB:** The manifest contains 31 `PLOT_SOIL_REFERENCE` spawn rows, all phase
1 at the same exact map/zone/area tuple:

* one row apiece for every entry 214036 through 214056 (21 rows);
* entry 215705 once, entry 215706 five times, entry 215719 once, and entry 215720
  twice (nine rows); and
* entry 210993 once.

Their template names are preserved in `notes` (for example, `Tillers Shrine`,
`Offering Bowl`, `Dented Shovel`, `Incense`, and `Fixed Wagon`, plus the literal
Doodad names for 214036–214056). Those are database identities, not assigned farm
states.

**UNKNOWN:** The snapshot does not establish that any of these entries means tilled
soil, growing crop, unlocked plot, upgrade level, or another lifecycle state.
`IDENTITY_PROVEN;LOCATION_PROVEN;SEMANTICS_UNKNOWN` is deliberate. The adjacent
creature rows in section 5 receive the same semantic restraint.

## 9. Phase baseline

**FACT — DB:** All 94 contracted spawn rows (36 creatures plus 58 gameobjects) have
`phaseMask=1`. The complete snapshot `phase_definitions` table has exactly zero rows
for each `zoneId` 1023, 5805, and 5840. Three explicit zero-cardinality manifest rows
make those absences testable.

**FACT — HISTORY:** Andrew introduced mask 128 for a public-farm layer and three
entry-1 definitions using mask 129. Therefore:

* mask 128 is not part of this known pre-Andrew Sunsong baseline;
* mask 129 is not part of this baseline;
* absence of those masks/definitions is not corruption; and
* Andrew's 128/129 architecture must not be described as restored historical data.

No replacement phasing is proposed here. **UNKNOWN:** Correct runtime identifiers
among 1023, 5805, 5840, and area 6039 remain separate from the static absence fact.

## 10. Cardinality and matching rules

A future recognizer must enforce all of the following together, not “at least one”:

1. **Exact LoA identity:** each of 94 spawn GUIDs occurs exactly once and matches its
   entry, map, zone, area, phase, and row-specific position/orientation.
2. **Bounded entry cardinality:** within map 870 / zone 5805 / area 6039, every
   contracted entry has exactly its manifest count. A duplicate with a new GUID is
   a mismatch even if all named GUIDs remain.
3. **Exact relations:** each of the eight starter/ender tuples occurs once (the
   schema primary key normally enforces this), and the non-baseline
   `(58646,30252)` starter is absent.
4. **Exact phase absence:** there are zero `phase_definitions` rows for each of
   1023, 5805, and 5840, regardless of entry number.
5. **No substitution across groups:** counts cannot compensate for a missing GUID,
   wrong entry, wrong phase, or displaced row in LoA snapshot identity mode.

The expected entry counts are: creatures `58646=2`, `58721=1`, `58719=8`,
`55626=16`, `59669=1`, `66129=8`; obstacles `210443=2`, `210444=5`, `210445=3`,
`210446=4`, `210447=7`, `210448=2`, `210462=2`, `210451=1`, `209572=1`; plot/reference
`214036–214056=1 each`, `215705=1`, `215706=5`, `215719=1`, `215720=2`, and
`210993=1`. The relation total is eight and each phase-absence count is zero.

The area tuple is an additional guard, not a broad coordinate box. Counts elsewhere
in the world do not satisfy or invalidate this bounded group unless an exact GUID is
being checked and collides.

## 11. Coordinate and fingerprint strategy

### LoA snapshot identity

Use GUID as the primary key, then require exact integral fields and group
cardinality. Compare the parsed numeric values represented by the manifest, not
locale-dependent strings or insignificant formatting such as `0` versus `0.000000`.
An exact text comparison is appropriate only when validating this exact published
file byte-for-byte. GUID existence alone is never sufficient.

For a database round-trip, accept an absolute tolerance of at most **0.005 world
units** independently for x/y/z and **0.00001 radians** for orientation. These small
limits accommodate `FLOAT` storage/re-serialization of the snapshot's displayed
precision; they are not search radii. Integral IDs and masks have zero tolerance.
If more than one candidate falls within tolerance, classification fails closed.

### Portable semantic identity

GUIDs are not assumed universal across unrelated DB lineages. A portable candidate
may instead match entry + exact map/zone/area + a one-to-one row-specific coordinate
and orientation comparison using the same tolerances, followed by exact entry
cardinality. It must not use a single ranch-sized box as proof. Because the plot and
soil meanings remain unknown, this is more accurately a **portable positional
identity**, not proof of semantics. A later task must keep this weaker result
separate from `BASELINE_MATCH` in strict LoA identity mode.

## 12. Known Andrew mutations and supersession

These mutations are **FACT — HISTORY** from the Andrew migrations already traced by
Task 046 (with the affected baseline subset stated explicitly):

| Baseline datum | Proven later Andrew mutation |
|---|---|
| Yoon GUID 516066, phase 1 | set to unreachable mask 4294901760 |
| Yoon GUID 516067, phase 1 | set to mask 128 |
| entry 58721 / GUID 516061, phase 1 | map-870 entry set to 4294901760 |
| eight 58719 rows, phase 1 | map-870 entry set to 4294901760 |
| sixteen bounded 55626 rows, phase 1 | matching z-bounded entry set to 4294901760 |
| eight 66129 rows | Andrew's earlier cleanup deletes map-870 entry 66129 |
| 27 obstacle rows, phase 1 | coordinate-bounded entries moved to mask 128 |
| three baseline 210445 rows | later Andrew weed expansion also adds eight new mask-128 GUIDs 4000077–4000084; it does not turn 11 into the baseline count |
| starter `(58721,30252)` | deleted and starter `(58646,30252)` inserted |
| eight other contracted relation tuples | no superseding mutation established for this contract |
| zero definitions at 1023/5805/5840 | entry 1 at each zone replaced/created with phase mask 129 |
| 215705/215706 phase 1 | an Andrew migration globally ensures phase 1; no baseline value change is established |
| remaining plot/reference spawns and 59669 | no specific superseding row mutation established here |

This table records provenance, not approval. `KNOWN_ANDREW_MUTATION` may be emitted
only when the complete applicable contracted mutation pattern matches, including
cardinalities and Andrew's added/replacement rows. A lone phase 128 row or lone
58646 starter is insufficient.

## 13. Safe target-state classifications

A future read-only classifier should produce exactly one result and retain diagnostic
sub-results per group:

* **`BASELINE_MATCH`** — all 105 manifest predicates pass, the explicit non-baseline
  58646→starter-30252 tuple is absent, and the strict LoA GUID/cardinality contract
  passes. This says only that the contracted snapshot subset matches.
* **`KNOWN_ANDREW_MUTATION`** — every applicable row and count matches the complete,
  specifically proven Andrew pattern summarized above, including quest replacement,
  Yoon/rock/reference/obstacle phases, definitions, removals, and additions. Do not
  infer this from generic masks.
* **`PARTIAL_KNOWN_MUTATION`** — at least one specifically proven Andrew mutation is
  present, but the complete recognized pattern is not. This includes an interrupted
  or selectively applied migration set.
* **`CUSTOM_OR_CONFLICTING`** — relevant rows exist but a value/cardinality differs
  from both exact baseline and proven Andrew states, GUIDs collide with other
  identities, matching is ambiguous, or baseline and custom rows coexist in an
  unrecognized combination.
* **`MISSING_DATA`** — one or more required baseline records/relations are absent and
  the target does not completely match the proven Andrew pattern.
* **`UNSUPPORTED_SCHEMA`** — required tables, columns, numeric representation, keys,
  or readable metadata cannot be matched safely.

Future restoration defaults to aborting on `PARTIAL_KNOWN_MUTATION`,
`CUSTOM_OR_CONFLICTING`, `MISSING_DATA`, and `UNSUPPORTED_SCHEMA`. Even
`KNOWN_ANDREW_MUTATION` requires an explicitly designed transition in a later task;
recognition is not authorization to mutate.

## 14. Abort-on-mismatch rules

Any future production design must fail closed and must, at minimum, obey these rules:

1. Never update GUID 516067 merely because it exists; verify entry, complete location,
   phase, coordinate tolerance, and both-Yoon cardinality first.
2. Never globally rewrite entry 58646, 58721, 58719, a reference entry, or an
   obstacle entry. Every mutation must be bounded by a recognized state.
3. Never delete all 58721 quest links. Compare exact relation tuples, and touch only
   a tuple authorized by an explicit transition.
4. Never add or replace `(58646,30252)` as though Andrew's reassignment were baseline.
5. Never delete, replace, or normalize `phase_definitions` indiscriminately; unrelated
   zones/entries are outside this contract.
6. Never use “at least one,” a ranch-sized coordinate box, or entry count alone to
   authorize restoration.
7. Never assume two Yoons is retail-correct merely because it is LoA's snapshot.
8. Never treat absence of masks 128/129 or the three Andrew definitions as corruption.
9. Never infer soil/plot lifecycle meaning from the candidate inventory.
10. Never normalize a custom database automatically, reuse a colliding GUID, or
    choose arbitrarily between multiple coordinate candidates.
11. Abort if any expected count, identity field, relation, absence, schema feature,
    or one-to-one positional match cannot be proven before writes begin.
12. Re-check preconditions atomically in any future transition; a stale earlier audit
    is not permission to write.

## 15. Unresolved UNKNOWNs

* The migration or assembly step that re-added the two 58646 GUIDs after the old
  entry-wide deletion remains unlocated.
* Deployed databases may differ from the published archive.
* Retail-correct simultaneous visibility, role, and client presentation of the two
  Yoons is unknown.
* Exact lifecycle meanings of 214036–214056, 215705/215706/215719/215720, 210993,
  and adjacent creature references remain unknown.
* Correct runtime zone/area interpretation among 1023, 5805, 5840, and 6039 is not
  established by static rows.
* The retail-correct tutorial presenter and timing for 58721 versus 58646 remain
  unknown.
* GUID-portability across unrelated database lineages is unknown; the portable
  positional mode is intentionally weaker.
* No claim is made that this bounded inventory exhausts every object that could
  participate in ranch gameplay.

## 16. Runtime-gated claims

**BUILD-18414/RUNTIME-DEPENDENT:** The manifest cannot establish visibility,
spawn/despawn packets, interaction, quest presentation, crop/soil transitions,
personal ownership, solo/group behavior, map transfer, same-map exit, logout/relog,
cleanup, duplicate prevention, or retail gameplay correctness. No runtime test was
performed or is authorized by this task. T3B remains deliberately blocked.

## 17. Recommended next task

Unless contradictory evidence appears, proceed with **Task 048 — Bounded Sunsong
world-data restoration design**. It should design—but not assume authorization to
execute—an idempotent transition from strictly recognized states, using this TSV as
the precondition oracle, transactionally rechecking cardinality, and aborting on all
unrecognized states. Task 048 must not silently resolve the semantic or runtime
UNKNOWNs listed above.
