# Task 048 — Bounded Sunsong world-data restoration design

## 1. Decision and scope

This is a static design record. It adds no runnable migration, production SQL, C++,
world-data update, or runtime behavior. No database was imported, no server or client
was started, and no gameplay test was performed. Task 045/T3B remains **BLOCKED /
RESERVED**.

The answer to the design question is deliberately narrow:

> A future restoration may recognize the exact Task 047 LoA snapshot baseline and
> one exact, complete Andrew end-state over that contracted subset. It may make one
> all-or-nothing transition from the latter to the former. Every partial, missing,
> ambiguous, custom, or schema-incompatible state must abort before any write.

This contract is **`LOA_SNAPSHOT_LINEAGE_ONLY`**. A portable restoration cannot be
proved safely. In particular, weakening GUID or cardinality checks to support an
unrelated database would exchange portability for destructive ambiguity. Task 047's
portable positional identity remains useful as a diagnostic label only; it cannot
authorize this transition.

“Baseline” throughout means the known LoA-distributed pre-Andrew database subset. It
does **not** mean retail correctness, correct simultaneous presentation of two
Yoons, correct callbacks, personal ownership, visibility, or gameplay correctness.
Andrew's custom-phase architecture is neither required nor preserved by this design.

## 2. Starting-state record

| Check | Recorded result |
|---|---|
| Initially checked-out branch and SHA | `work` at `8461fb160aca8a612559c1f735fd59e875b2d2df` |
| Latest fetched `origin/preservation/main` | `f150d675e041909fc3698737f3cdc0d2ec149c84` |
| Initial work tree | clean (`git status --porcelain` emitted nothing) |
| Task 046 report | present on `origin/preservation/main` at `docs/preservation/backports/TASK046_TILLERS_WORLD_DATA_PROVENANCE.md` |
| Task 047 report | present on `origin/preservation/main` at `docs/preservation/backports/TASK047_SUNSONG_BASELINE_CONTRACT.md` |
| Task 047 TSV | present on `origin/preservation/main` at `docs/preservation/data/TASK047_SUNSONG_BASELINE.tsv` |
| Task 047 integration proof | commit `aa3bc7e` is reachable from fetched `origin/preservation/main`; all three paths were read directly from that ref |
| Task 044 fixture | absent; neither the fetched tree nor a recursive working-tree name search contained `.t3atest` |
| Task branch | `codex/task-048-sunsong-restoration-design`, created directly from fetched `origin/preservation/main` |

The checkout initially had no remotes. `origin` was configured to the preservation
fork, `preservation/main` was fetched, and the checks above were completed before the
task branch was created. The branch does not derive from `master`; no local `master`
branch was created or changed.

## 3. Authorities, notation, and bounded population

The normative baseline oracle is
[`TASK047_SUNSONG_BASELINE.tsv`](../data/TASK047_SUNSONG_BASELINE.tsv). The Task 047
report defines how its 105 unique predicates are interpreted; Task 046 supplies
provenance and Andrew history. Task 048 does not add a row or silently enlarge that
manifest.

For compactness, this document uses:

* **M** — all 105 Task 047 predicates: 94 phase-1 spawn rows (36 creatures and 58
  gameobjects), eight quest tuples, and three zero-cardinality phase predicates.
* **A** — the complete Andrew end-state fingerprint in section 5, not merely one
  Andrew-looking value.
* **immutable context** — every manifest field other than a field explicitly changed
  by the proven Andrew statement. Entry, GUID, map, zone, area, position,
  orientation, and group cardinality remain mandatory context even when Andrew's SQL
  used a broader predicate.
* **exact tuple** — the complete primary-key relation, not every relation for an NPC.
* **contracted population** — only manifest identities plus the eight specifically
  identified Andrew-added weed rows and the three exactly identified Andrew phase
  rows. Same-entry rows elsewhere in the world are not restoration targets.

A recognizer must first calculate every component result without changing data, then
calculate exactly one overall result. Evidence from one component cannot compensate
for a mismatch in another.

## 4. Exact state model

| Overall state | Exact recognition rule | Disposition |
|---|---|---|
| `BASELINE_MATCH` | M passes in strict LoA GUID mode; `(58646,30252)` starter is absent; no Andrew-added weed or relevant phase row exists | `NO_OP` |
| `KNOWN_ANDREW_MUTATION` | every A predicate passes simultaneously, including unchanged M context, deleted 66129 rows, eight added weeds, quest swap, and three exact phase rows; no extra/conflicting bounded row exists | `TRANSITION_ALLOWED`, but only as the single atomic A→M operation described here |
| `PARTIAL_KNOWN_MUTATION` | at least one A-only predicate matches, while another required A predicate is baseline, missing, or inconsistent, and no stronger conflict classification applies | `ABORT`; never finish or partially revert Andrew |
| `CUSTOM_OR_CONFLICTING` | a relevant row exists with an identity/context/state not equal to M or A; a GUID collides; matching is ambiguous; an unexpected relevant relation/phase row exists; or recognized and custom data coexist | `ABORT` |
| `MISSING_DATA` | an M identity/relation is absent and the full A deletion/absence fingerprint does not explain it | `ABORT`; absence is not authority to recreate |
| `UNSUPPORTED_SCHEMA` | a required table, column, key, engine/transaction property, numeric representation, isolation behavior, or metadata check cannot be safely evaluated | `ABORT` |

Precedence is: `UNSUPPORTED` first; then `CONFLICT`; then `MISSING`; then complete
`MATCH` or complete `KNOWN_ANDREW`; otherwise `PARTIAL`. This precedence prevents an
Andrew-looking value from hiding an independently dangerous conflict. Diagnostics
must retain all lower-level facts even when the overall label has higher precedence.

There is no independently safe component transition. Although individual updates
can be bounded by GUID, applying them to a partial installation could create a hybrid
whose history and runtime assumptions are unknown. The only authorized future write
set is complete A→M. Separately approved, operator-directed forensic recovery could
later use manifest rows to reconstruct known identities, but that would be a new
contract—not an implicit `MISSING_DATA` path here.

## 5. Complete Andrew end-state fingerprint

Andrew's final relevant state is recognized only when all of the following hold:

1. GUID 516066/entry 58646 has all manifest context and phase `4294901760`.
2. GUID 516067/entry 58646 has all manifest context and phase `128`.
3. GUID 516061/entry 58721 has all manifest context and phase `4294901760`.
4. All eight manifest 58719 rows retain identity/context and have phase
   `4294901760`.
5. All sixteen manifest 55626 rows retain identity/context and have phase
   `4294901760`.
6. All eight manifest 66129 rows are absent. No replacement or unknown 66129 row may
   occupy a manifest GUID; the bounded entry count is zero.
7. The one manifest 59669 row and all 31 plot/soil/reference gameobjects still match
   M exactly.
8. All 27 manifest obstacles retain identity/context and have phase `128`.
9. Exactly eight additional entry-210445 objects exist at GUIDs 4000077–4000084,
   with every stored field equal to Andrew's insert (map 870, zone 5805, area 6039,
   spawn mask 1, phase 128, phase/group 0, recorded coordinates/orientation/rotations,
   spawn time 120, anim progress 255, state 1, empty script, build 0). They are the
   only additional contracted-entry objects in the bounded ranch population.
10. `(58721,30252)` starter is absent and `(58646,30252)` starter exists exactly once.
    The baseline ender and all six 30254–30256 relations remain exact M matches.
11. For each zone 1023, 5805, and 5840 there is exactly one `phase_definitions` row,
    precisely `(zoneId, entry=1, phasemask=129, phaseId=0, terrainswapmap=0,
    worldMapArea=0, flags=0)`, and no other row for that zone.
12. All schema, one-to-one identity, coordinate, cardinality, and conflict checks in
    sections 7–9 pass in the same consistent read.

This fingerprint deliberately strengthens Andrew's broad SQL predicates with the
manifest's exact identities. It recognizes the historical result without endorsing
the unsafe breadth of the historical statements.

## 6. Andrew world-data mutation ledger

The source commits were re-inspected only to make the already established Task
046/047 mutations exact. “Reversible” means reversible to the contracted Task 047
subset from static evidence; it does not mean that Andrew's original broad operation
preserved arbitrary custom rows.

| Source commit | SQL path | Table/effect | Baseline predicate → Andrew destination | Historical selector | Static reversibility / information loss / custom risk | Task 048 classification |
|---|---|---|---|---|---|---|
| `6115dcae543887b54636a4fc03f1bb3fd8de93e5` | `sql/updates/world/2026_05_31_03_tillers_farm_cleanup.sql` | `creature`, 66129 | eight manifest phase-1 rows → deleted | map 870 + entry | Manifest can recreate the eight contracted rows; DELETE lost row data and could have removed non-manifest/custom map rows, so their recovery is impossible | A member only when exact contracted absence and every other A predicate match; otherwise `MISSING`/`CONFLICT` |
| same | same | `creature`, 55626/58719 | manifest rows at phase 1 → `4294901760` | map + entries + source phase 1 (also 69098, outside M) | Phase value is reversible for exact manifest identities; original statement was broad and could affect custom rows | A member for the 24 exact manifest rows only; outside-M effects are not normalized |
| same | same | `creature_template`, 58646 | `unit_flags=32768` → 0 if present | exact entry/value | Baseline template value is not in M, so reversal is not proved | `OUT_OF_SCOPE`; never reverse |
| `1e138b1f1d7e1660eb28b647b7193afe961acb72` | `sql/updates/world/2026_06_02_02_tillers_obstacle_phasing.sql` | `gameobject`, obstacles | 27 manifest phase-1 rows → 128 | entry list + map + x/y box | Phase reversible after all 27 GUID/context checks; box could also touch custom rows | A member only for exact manifest GUIDs; any additional affected row is preserved and causes `CONFLICT` if in bounded population |
| same | same | `creature`, 55626 | phase 1 → `4294901760` | entry + map + z range | Same final value as cleanup; reversible only for the 16 manifest rows | A member; do not emulate range update |
| same | `sql/updates/world/2026_06_02_04_tillers_public_farm_phase.sql` | `creature`, Yoon 516066 | exact manifest row phase 1 → `4294901760` | entry + exact GUID | Reversible if all remaining manifest context and two-Yoon cardinality match; no row data lost | A member |
| same | same | `creature`, Yoon 516067 | exact manifest row phase 1 → 128 | entry + exact GUID | Reversible under the same checks; no row data lost | A member |
| same | same | `creature`, 58721 | GUID 516061 phase 1 → `4294901760` | entry + map, not GUID | Reversible for 516061 only after exact identity/cardinality checks; broad selector risks other/custom 58721 rows | A member for GUID 516061 only |
| same | same | `creature`, 58719 | eight manifest phase-1 rows → `4294901760` | entry + map | Reversible for the eight exact identities; broad selector risks custom rows | A member for exact manifest GUIDs only |
| same | same | `gameobject`, obstacles | 27 manifest phase-1 rows → 128 | entry list + map + x/y box | Repeats the obstacle result; reversible only by exact manifest identity | Same single A obstacle component, not a second transition |
| same | same | `creature_queststarter` | `(58721,30252)` present and `(58646,30252)` absent → inverse presence | exact tuples (`DELETE`, then `REPLACE`) | Reversible if the full relation-set fingerprint passes; tuple data is known, but unexpected presenters make normalization unsafe | A quest component |
| same | same | `phase_definitions` | no rows for zones 1023/5805/5840 → exact entry-1 rows with mask 129 and zeros elsewhere | `REPLACE` on each `(zoneId,entry)` | Known inserted values are removable only when byte-for-field exact and sole zone rows; `REPLACE` may have destroyed pre-existing custom entry-1 rows | A member, but prior custom content cannot be recovered |
| same | `sql/updates/world/2026_06_03_05_tillers_weed_expansion.sql` | `gameobject`, 210445 | no GUIDs 4000077–4000084 → eight explicit phase-128 rows | exact inserted GUIDs and full values | Removable when every field matches; GUID collision or edited row aborts | A member; exact-row deletion only |
| `867b69b0d142efaaea10b83970342900d3405f2e` | `sql/updates/world/2026_06_03_05_tillers_weed_expansion.sql` | insertion policy for the same eight 210445 rows | destination values unchanged | changes `INSERT` to `INSERT IGNORE` | No new recoverable state; a GUID collision can silently prevent or partially apply the historical insert | Recognition still requires all eight exact rows; otherwise `PARTIAL` or `CONFLICT` |
| `1e138b1f1d7e1660eb28b647b7193afe961acb72` | `sql/updates/world/2026_06_01_03_tillers_shrine_phase.sql` | `gameobject`, 215705/215706 | baseline already phase 1 → phase 1 | global entry | No baseline delta; broad write could normalize custom rows | `NO_BASELINE_DELTA`; never use to recognize or restore |

Andrew's quest gating, SAI, template, soil-script, conditions, character schema, and
runtime-manager changes are outside the 105-predicate world-data contract. Their
presence is neither required for recognition nor an invitation to reverse them.
Task 048 does not treat Andrew's branch as one reversible unit.

## 7. Component transition matrix

All rows below inherit the complete-state rule: an “allowed” component change is a
member of the atomic full A→M transition, never standalone authorization. `M row`
means exact GUID, entry, map 870, zone 5805, area 6039, coordinates/orientation,
phase, and relevant group count from the TSV. Confidence concerns static identity and
history only.

| Component | Recognized source and required preconditions | Identity fingerprint and cardinality | Allowed transition / expected destination | Forbidden transition | Idempotence and rollback/recovery | Confidence / evidence / runtime knowledge |
|---|---|---|---|---|---|---|
| Farmer Yoon 516066 | M row at phase 1 = `MATCH`; otherwise exact row at `4294901760` plus full A = `KNOWN_ANDREW` | exact GUID 516066, entry 58646, all M context; exactly two bounded 58646 rows, with 516067 also exact | In full A→M, phase only to 1; destination exact M | insert/delete/move/re-entry; update every 58646; transition from any other mask/context | M rerun no-op; forward audit permits rollback only from exact M back to captured exact A, never inferred | High, Tasks 046/047 + Andrew public-phase SQL; presentation is runtime-dependent, transition is not |
| Farmer Yoon 516067 | M row at phase 1, or exact row at 128 plus full A | exact GUID/entry/context and two-row group | phase only 128→1 in full transition | “make Yoon visible,” global entry update, altered coordinates | same phase-only rule; rollback requires exact before-image/A authorization | High static; simultaneous visibility remains unknown/runtime-dependent |
| NPC 58721 / 516061 | M phase 1, or exact phase `4294901760` plus full A | GUID 516061, entry 58721, M context; bounded count exactly one | phase only to 1 | map-wide/entry-wide update, insertion, or presenter inference | no-op at M; exact before-image required for rollback | High static; NPC role/presentation runtime-dependent |
| Quest 30252 starter | baseline tuple present/Andrew tuple absent, or exact inverse plus full A | independently count `(58721,30252)` and `(58646,30252)` in starter table; also enumerate every starter for quest 30252 | atomically remove exact Andrew tuple and restore exact baseline tuple | delete all 58721 links; accept both/neither/third presenter; decide gameplay presenter | baseline rerun no-op; rollback swaps only these exact tuples after full conflict recheck | High historical DB identity; correct presenter is explicitly runtime/retail unknown |
| Quest 30252 ender | exact `(58646,30252)` count 1 in both M and A | exact tuple plus enumeration of enders for quest 30252 | no write; remains M | recreate, replace, delete, or infer from starter | invariant must remain unchanged before/after/rollback | High; runtime presentation not required for recognition |
| Quest 30254 relations | exact 58646 starter and ender, each count 1, in M and A | two exact table tuples; enumerate relevant quest relations for conflicts | no write | normalize all 58646 relations or modify gating | invariant before/after | High for tuples; gameplay runtime-dependent |
| Quest 30255 relations | same as 30254 for quest 30255 | two exact tuples | no write | same | invariant before/after | High for tuples; gameplay runtime-dependent |
| Quest 30256 relations | same as 30254 for quest 30256 | two exact tuples | no write | same | invariant before/after | High for tuples; gameplay runtime-dependent |
| Obstacle population | all 27 exact M rows at phase 1, or all at phase 128 plus exact eight Andrew additions and full A | each manifest GUID/context; per-entry counts 2/5/3/4/7/2/2/1/1 at M; A has eleven 210445 total only because eight exact additions are present | set phase of the 27 exact GUIDs to 1 and remove only eight exact added rows; destination M counts | global entry/range update; tolerate mixed phases, changed GUID/location, missing rows, edited additions, or extra bounded objects | result matches M; rollback needs stored A before-image and cannot target changed rows | High for static rows/mutation; intended obstacle visibility/progression runtime-dependent |
| Ranch reference creatures | M: eight 58719, sixteen 55626, one 59669, eight 66129 at phase 1; A: first two groups hidden, 59669 unchanged, all eight 66129 absent | every TSV GUID/context and exact group counts; no substitution | phases of exact 58719/55626 to 1; insert the eight exact manifest 66129 rows; leave 59669 unchanged | map/entry/range normalization; recreate from partial/missing state; touch 69098 or other outside-M rows | poststate exact M; rollback may delete only reinserted exact rows and restore phases after proof; Andrew's broad deletion cannot be generally undone | High identities, medium recovery safety due historical information loss; semantics runtime/unknown |
| Plot/soil/reference objects | all 31 exact M rows at phase 1 in both M and A | every GUID/context; 214036–214056 once each, 215705 once, 215706 five, 215719 once, 215720 twice, 210993 once | no write; immutable corroborating population | assign lifecycle meaning, alter phase/script, infer ownership, or normalize shrine/bowls globally | invariant before/after; no behavioral rollback exists | High identity/location, semantics `UNKNOWN`; runtime knowledge required for behavior and intentionally absent |
| `phase_definitions` | M: zero rows for each zone; A: sole exact entry-1/mask-129 zero-filled row for each zone plus full A | enumerate all rows for 1023, 5805, 5840; A cardinality exactly 1 per zone and exact every field | remove only the three exact A rows; destination zero per zone | delete by zone broadly, replace custom definitions, or treat absence as corruption | rerun sees zero/no-op; rollback may recreate exact captured A rows only after zero-row check; overwritten pre-Andrew custom rows are unrecoverable | High for snapshot absence/Andrew creation; callback meaning runtime-dependent |

### 7.1 Quest 30252 truth table

| 58721 starter | 58646 starter | Any unexpected third starter for 30252 | Component label | Static action |
|---:|---:|---:|---|---|
| exactly 1 | 0 | no | `MATCH` | `NO_OP` |
| 0 | exactly 1 | no | `KNOWN_ANDREW` only if full A matches; otherwise `PARTIAL` | full A→M only, else `ABORT` |
| exactly 1 | exactly 1 | no | `PARTIAL` | `ABORT`; do not choose a presenter |
| 0 | 0 | no | `MISSING` | `ABORT`; no automatic creation |
| any | any | yes | `CONFLICT` | `ABORT`; preserve all tuples |
| duplicate/impossible cardinality | any | any | `CONFLICT` or `UNSUPPORTED` if schema permits ambiguity that cannot be isolated | `ABORT` |

The exact `(58646,30252)` ender must remain present in every recognized state. The
truth table decides only static row provenance, never which NPC should present the
quest to a build-18414 client.

### 7.2 Yoon field rule

For each Yoon, existence, GUID, entry, map, zone, area, x/y/z, orientation, and phase
are distinct predicates. Only phase differs between M and A. A missing row is
`MISSING`, an altered non-phase field is `CONFLICT`, and an unrecognized phase is
`CONFLICT`. Correct coordinates with a different GUID do not substitute; correct
GUID with moved coordinates does not authorize a phase write. Both-Yoon cardinality
is checked before either row can be changed.

### 7.3 Obstacles and additions

The 27 manifest GUIDs are evaluated one by one. All phase 1 is M; all phase 128 plus
the eight exact expansion rows is the obstacle part of A. Any mixture of phases is
`PARTIAL`; changed GUID/entry/context is `CONFLICT`; a missing manifest row is
`MISSING` unless full A explicitly calls for absence (it does not for obstacles).
Additional same-entry objects inside map 870/zone 5805/area 6039 are `CONFLICT`
unless they are precisely all eight Andrew expansion rows in full A. Same-entry
objects outside that bounded tuple are reported but not mutated; an exact-GUID
collision anywhere is a conflict.

### 7.4 Phase definitions

Baseline absence is healthy. Recognition of Andrew creation requires all seven
stored values of each of the three exact rows and exactly one total row for each
zone—not merely zone ID, entry 1, or mask 129. A custom entry, changed field, or
additional row in any of those zones is `CONFLICT`, and all rows are preserved on
abort. This design never proposes a zone-wide delete.

### 7.5 Plot/soil/reference restraint

These 31 rows corroborate lineage and state. They do not encode crop lifecycle,
unlocked plots, tilled/watered/growing/mature state, upgrades, or player ownership in
this contract. No behavioral transition or recovery is designed for them. A mismatch
therefore blocks the complete transition rather than prompting semantic “repair.”

## 8. Check-before-write protocol

A future implementation must perform these checks in a transaction-capable preflight
and repeat them after acquiring the write-protecting locks/isolation required by the
engine. A stale dry run is never permission to mutate.

### 8.1 Identity checks

* Resolve every one of the 94 manifest GUIDs exactly once in its declared table and
  require the manifest entry; detect a GUID used by another entry anywhere.
* Resolve each of the eight relation tuples by full table key and count it.
* Resolve Andrew GUIDs 4000077–4000084 individually and compare all inserted fields.
* Resolve phase rows by `(zoneId,entry)` and enumerate the entire three-zone sets.
* Reject key types, signedness, collations, or duplicate behavior that prevent an
  unambiguous one-to-one match.

### 8.2 Context checks

* Require exact integral map 870, zone 5805, area 6039, spawn/phase identifiers, and
  manifest group counts.
* Require row-specific x/y/z and orientation. For a database round-trip, compare
  parsed numeric values with Task 047's maximum absolute tolerance: **0.005 world
  units independently per axis and 0.00001 radians for orientation**. These are
  serialization tolerances, never search radii.
* Use zero tolerance for integral values. Use manifest textual equality only when
  checking the original snapshot bytes, not a database round-trip.
* Coordinate proximity can corroborate a known GUID; it can never independently
  authorize mutation. Multiple candidates within tolerance are `CONFLICT`.

### 8.3 State checks

* Compare every mutable phase to exactly M or its exact A value; no other mask is
  accepted.
* Enforce the complete quest 30252 truth table and preserve the 30252 ender and all
  30254–30256 tuples.
* Require every invariant component (59669, plot/reference rows, unchanged
  relations) to match M in A as well.
* Require all Andrew deletions/additions and exact phase definitions together. A
  transition is authorized only when the overall state is `KNOWN_ANDREW_MUTATION`.
* After writes, recompute all 105 M predicates and all explicit non-baseline
  absences. Any failure must roll back.

### 8.4 Conflict checks that force `ABORT`

Abort for any altered entry, exact-GUID collision, unexpected phase, moved coordinate
or orientation, wrong map/zone/area, duplicate or missing required row, unexpected
bounded same-entry row, edited Andrew addition, mixed phase population, both/neither/
third 30252 starter, missing invariant relation, custom phase definition, additional
row in one of the three phase zones, count mismatch, ambiguous numeric comparison,
schema mismatch, concurrent change, inability to lock/recheck, or non-transactional
table in the planned write set.

Rows outside the contract are never “cleaned up.” If an outside row was plausibly
caught by Andrew's historical broad predicate, the verifier reports it as unexpected;
it does not guess its former value.

## 9. Atomic transition and idempotence contract

A future implementation must use one all-or-nothing transaction if—and only if—all
involved tables use an engine and schema that provide the required atomicity and
isolation. It must begin a consistent transaction, acquire appropriate locks (or an
equivalent serializable guarantee), recompute the complete fingerprint, perform only
exact-key writes whose affected-row counts equal the precomputed counts, validate M,
and commit. Any discrepancy rolls back. If atomic cross-table behavior cannot be
proved, classify `UNSUPPORTED_SCHEMA`; do not offer best effort.

Normative equations:

* M → M = `NO_OP` with zero writes.
* complete A → M = one `TRANSITION_ALLOWED` atomic operation.
* transitioned M → M = `NO_OP` with zero writes.
* `PARTIAL` → unchanged = `ABORT` before writes.
* `CONFLICT` → unchanged = `ABORT` before writes.
* `MISSING` → unchanged = `ABORT` before writes.
* `UNSUPPORTED` → unchanged = `ABORT` before writes.

Affected-row counts are assertions, not success hints. A count of zero where one was
expected, or a count larger than the exact authorized set, forces rollback. Running
the migration twice cannot reinsert, re-delete, or re-update anything because the
second preflight recognizes M and exits before its write phase.

## 10. Rollback and recovery design

“Rollback” has two meanings and they must not be conflated:

1. **Failure rollback:** before commit, the database transaction restores its exact
   preflight state automatically. This is mandatory.
2. **Post-commit reversal:** returning M to Andrew's A is not ordinary restoration
   and must not be automatic. It would reintroduce an unsupported architecture. If
   an operator explicitly authorizes it, a separate audited operation must consume
   the immutable preflight report/before-image, prove the current database is still
   exact M, and recreate only the captured exact A values and rows atomically.

The future tool should record a tamper-evident preflight/before-image outside the
transaction: target/schema identity, timestamp, manifest version/hash, state,
component results, complete values of every row proposed for change, absent-row
assertions, and proposed affected counts. It must not store guesses.

Andrew's map-wide 66129 DELETE and `REPLACE` of phase definitions are information-
losing. The Task 047 manifest can reconstruct the eight contracted 66129 rows, but it
cannot recover unknown custom map rows or a custom entry-1 definition overwritten by
`REPLACE`. Full A recognition makes bounded restoration deterministic; it does not
make all historical data loss reversible. Discovery of evidence for such lost custom
data requires `ABORT` and operator-led recovery from backup.

`MISSING_DATA` might support a future, explicit recovery mode only when an operator
provides lineage proof and accepts exact manifest reconstruction. It is not part of
A→M, cannot be inferred from absence, and requires a new reviewed design.

## 11. Dry-run/preflight output contract

A future verifier should emit stable machine-readable records plus a human summary.
No executable verifier is added here.

### 11.1 Stable vocabulary

* Component states: `MATCH`, `KNOWN_ANDREW`, `PARTIAL`, `CONFLICT`, `MISSING`,
  `UNSUPPORTED`.
* Overall states: `BASELINE_MATCH`, `KNOWN_ANDREW_MUTATION`,
  `PARTIAL_KNOWN_MUTATION`, `CUSTOM_OR_CONFLICTING`, `MISSING_DATA`,
  `UNSUPPORTED_SCHEMA`.
* Decisions: `NO_OP`, `TRANSITION_ALLOWED`, `ABORT`.
* Predicate results: `PASS`, `MISMATCH`, `UNEXPECTED`, `ABSENT`, `AMBIGUOUS`.
* Runtime qualifier: `STATIC_ONLY`, `RUNTIME_KNOWLEDGE_REQUIRED`.

### 11.2 Required conceptual fields

Each run reports `contract_version`, manifest path and digest, lineage mode
(`LOA_SNAPSHOT_LINEAGE_ONLY`), target/schema identifier, transaction capability,
overall state, decision, and abort reasons. Each component record reports component
name, component state, expected predicate, observed value/count, missing identities,
unexpected rows, mismatch fields, evidence locator, confidence, runtime qualifier,
and proposed exact transition. The proposal must be empty for `NO_OP` and every
`ABORT`.

Output ordering should be stable: schema; Yoon 516066; Yoon 516067; 58721; quest
30252 starter; quest 30252 ender; quests 30254/55/56; obstacles; ranch references;
plot/reference objects; phase definitions; overall decision. GUIDs and tuples sort
numerically. Secrets and connection credentials must never appear.

Example conceptual summaries (not commands or SQL):

```text
OVERALL state=BASELINE_MATCH decision=NO_OP
OVERALL state=KNOWN_ANDREW_MUTATION decision=TRANSITION_ALLOWED
OVERALL state=CUSTOM_OR_CONFLICTING decision=ABORT reason=PHASE_ROW_UNEXPECTED
```

A dry run is observational. `TRANSITION_ALLOWED` says that the recorded state is
eligible under this design; it is not itself permission to execute a production
migration.

## 12. Explicitly prohibited normalization

No future implementation based on this design may:

* delete or update all entry 58646, 58721, 58719, 55626, 66129, or any obstacle entry;
* delete all 58721 quest links or choose a quest presenter from runtime assumptions;
* update obstacles by entry, coordinate box, or proximity alone;
* delete/replace all phase definitions for 1023, 5805, or 5840;
* delete and recreate a ranch area or use “at least one” as cardinality;
* treat a missing row as corruption or silently recreate it;
* assume Andrew is installed from one mask, tuple, GUID, filename, or comment;
* finish Andrew's partial migration or partially revert it;
* overwrite a custom mask, moved spawn, reused GUID, custom relation, or custom phase;
* infer crop, plot, upgrade, soil, ownership, or lifecycle semantics;
* claim M is retail-correct or that two phase-1 Yoons should be visibly simultaneous;
* use Andrew's custom masks, manager, callbacks, or private-phase architecture as a
  prerequisite for static restoration; or
* weaken identity checks to advertise portability.

## 13. Boundaries and final disposition

The transition contract is complete for a later design review, not authorized for
implementation. Its only positive path is a fully fingerprinted, transactional,
LoA-lineage A→M restoration. Everything else fails closed without mutation.

Client presentation, visibility, ownership, interaction, spawn/despawn behavior,
callback identifiers, plot meaning, crop lifecycle, quest gameplay, solo/group
behavior, cleanup, and retail correctness remain **BUILD-18414/RUNTIME-DEPENDENT** or
`UNKNOWN`. No conclusion here unblocks Task 045/T3B.
