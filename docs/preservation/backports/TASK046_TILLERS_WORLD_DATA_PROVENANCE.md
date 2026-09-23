# Task 046 — Tillers world-data provenance and cross-fork archaeology

## Executive decision record

This is a documentation-only archaeology result. It changes no C++, SQL, schema, or
runtime behavior. The labels used below are deliberately strict:

* **FACT — CODE** means directly observed source behavior.
* **FACT — DB** means a row directly observed in an SQL snapshot or migration.
* **FACT — HISTORY** means a Git object, parent relationship, ref, release, or file
  history was directly observed.
* **INFERENCE** means the stated conclusion best explains multiple facts but is not
  itself a stored row or executable fact.
* **UNKNOWN** means the available chain does not prove the proposition.
* **BUILD-18414/RUNTIME-DEPENDENT** means static evidence cannot decide it.

The principal new result is that the published LoA database archive
`world_548_20240722.7z` is inspectable and contains the pre-Andrew farm rows. It
proves both Yoon GUIDs, the obstacle population, and the tutorial quest links in the
database distributed by LoA. It does **not** prove that those rows produced correct
retail behavior.

## Starting state

| Check | Recorded result |
|---|---|
| Initially checked-out branch/commit | `work` at `b9b49f107734ac31e163bf622a027b46f83ab43c` |
| Latest fetched `origin/preservation/main` | `b9b49f107734ac31e163bf622a027b46f83ab43c` |
| Task branch | `codex/task-046-tillers-world-data-archaeology`, created directly from that SHA |
| Work tree before research | clean (`git status --porcelain` emitted nothing) |
| Task 044 fixture | absent: a recursive name search of the starting tree found no `TASK044`, T3A fixture, or private-owner fixture path |

**FACT — HISTORY:** Task 046 therefore starts from the requested integration tip,
not `master`, Andrew's branch, or the disposable Task 044 branch.

## Scope and method

Research was read-only and offline except for Git/GitHub downloads. No database was
imported, no server or client was started, and no gameplay was tested.

1. Fetched `preservation/main`, enumerated refs and trees, and searched the complete
   locally available LoA history rather than only its tip.
2. Enumerated the GitHub fork network and the specifically named forks; fetched all
   useful branch refs into a bare archaeology clone; tested ancestry with
   `merge-base --is-ancestor`; inspected trees and patches; and queried exact commit
   objects when GitHub exposed an otherwise unreferenced network object.
3. Inspected Andrew's commits, parents, dates, patches, later path history, and tip.
4. Cloned and searched cooler-SAI, SkyFire, PandariaCore, alexkulya, JadeCore548,
   WoWSCore548, PandaCore, and the separately classified torghast lineage.
5. Downloaded LoA release asset `world_548_20240722.7z` (75,143,384 bytes), expanded
   its single 481,354,136-byte `world_548_20240722.sql` file with `py7zr`, and searched
   it as text. No generated/downloaded artifact entered Git.
6. Compared migrations against the snapshot. A row in a migration is reported as
   historical intent unless the snapshot independently contains it.

Useful reproducibility locators are the [LoA release](https://github.com/Legends-of-Azeroth/Legends-of-Azeroth-Pandaria-5.4.8/releases/tag/WDB20231230),
[Andrew repository](https://github.com/andrew-downey/Legends-of-Azeroth-Pandaria-5.4.8),
and [SkyFire repository](https://github.com/ProjectSkyfire/SkyFire_548).

## Repository and lineage inventory

### Direct LoA forks

All twelve required repositories were reachable by Git at research time. The SHA is
the inspected `master` tip. “Andrew object” distinguishes an API-resolvable object
from ancestry: an object exposed by GitHub's shared fork network is not a reachable
branch commit.

| Fork | Inspected tip | Andrew reachable from tip? | Tip has Andrew manager/hooks? | Classification |
|---|---|---:|---:|---|
| `andrew-downey` | Andrew `master` containing `bd8ad55…` and later commits | yes | yes | ANDREW ORIGINAL |
| `ingussuveiks-dev` | `62447321c3af9be715d98ee73e8cf98b95e13d03` | no | no | PRE-/PARALLEL LoA; `6115dca…` API object only |
| `d23monkey` | `7944bf57bab9a86dc56ba7b3a21a50e4b1d23aa7` | no | no | PRE-ANDREW fork |
| `leelf00` | `e3267eda705effafffe9acabaeb4953eed508793` | no | no | PRE-/PARALLEL LoA; `6115dca…` API object only |
| `nbyaya` | `1a50f5c45541d2849f8bfe0a16a65db96207896b` | no | no | PRE-ANDREW fork |
| `lingxDEV` | `2c4e66fd0168342e1786dbfb0db9ac62ccd836c7` | no | no | PRE-ANDREW fork |
| `zitengzela` | `5cf544d85ff332aa41769c04f6dbca847207836c` | no | no | PRE-ANDREW fork |
| `Garona0125` | `6dbd283eae7f55008186da5ce63cf0b52aed6241` | no | no | PRE-ANDREW fork |
| `ShadoriaWow` | `b3712fb5d3d1b0e055208e384cffefe4bfd0579d` | no | no | PRE-ANDREW fork |
| `Dude100` | `72a02a21283802ad837e68456eea31ca2b756eff` | no | no | PRE-ANDREW fork |
| `Apheleos` | `7eb03e9b0a887402db142a0d0d92d95597f161be` | no | no | PARALLEL/current LoA |
| `NeverlandWOW` | `7703b15ed48f5a343922f13aa0114a32dc8e0e07` | no | no | PARALLEL LoA; `6115dca…` API object only |

Newer network forks (`bondarenkoandriy`, `hilch1981-prog`, `mrcool72`, `ketsmen`,
`MityaFoxy`, `Yuukiring`, and `mudyx`) were also sampled because their push dates are
post-Andrew. None had `6115dca…` as an ancestor or Andrew's manager/hooks at its tip.

**FACT — HISTORY:** The starting lead is confirmed in its important qualified form:
GitHub can return Andrew's exact commit object through some fork repository APIs even
though no fetched branch contains it and the fork tip lacks the files. Branch-tip
search alone is insufficient, but object visibility alone is equally insufficient.
No inspected fork is an Andrew descendant that removed or repaired his work.

### Independent and ancestral sources

| Source | Inspected tip | Finding/classification |
|---|---|---|
| `cooler-SAI/pandaria_5.4.8` | `40abe3d8327d6d1759cce71d7739557091cd80a9` | LoA ancestor; contains 2021 Yoon deletion comment; no personal-farm implementation |
| `ProjectSkyfire/SkyFire_548` | `ad88c54ab41b7859f36227a3ab9a4c1f5c79bfd2` | Independent/ancestral SFDB world-data lineage; historical Yoon spawn and quest links |
| `PandariaCore/PandariaCore` | `a959920207caf0b1d3205640e1efa9377e1ba2c7` | Shares early SFDB-era data; no superior personal-farm architecture located |
| `alexkulya/pandaria_5.4.8` | `e0a20613d73e2b9324ac13a4783c724fec1d4559` | Related 5.4.8 code; no relevant implementation/data hit at tip |
| `dufernst/JadeCore548` | `f7b83aa95c113cc180f273f82c79832424714315` | Related lineage; no relevant implementation/data hit |
| `dufernst/WoWSCore548` | `46f487b2c091e38a611d920d0ac11d504917bc43` | Related lineage; no relevant implementation/data hit |
| `CrossUCI/PandaCore` | `bf3dd380ea693ac432a324eb3c69f1ec79cc8d1f` | Related lineage; no relevant implementation/data hit |
| `Legends-of-Azeroth/torghast-project-5.4.8` | `60c57ce752338ceb592a8a6f202ac922ae066657` | Separate corroborating LoA-derived lineage; repeats 2021 deletion/SAI files, not an automatically compatible implementation |

Absence claims here mean “not located after tip, history/path, and term searches”; they
do not claim that every unreachable private branch or external DB was searched.

## Significant commit table

| Repository/ref | Full SHA; parent(s) | Author date; subject | Relevant files | Relationship |
|---|---|---|---|---|
| Andrew `feature/pandaria-fixes`, `master` | `6115dcae543887b54636a4fc03f1bb3fd8de93e5`; parent `41e67ba1fafd4f43ac13f529c7fa1577858b3802` | 2026-06-01; “Fixing Merda… adding Tillers farm phasing…” | `TILLERS.md`, manager/header, soil/zone hooks, cleanup/gating/phase SQL | ANDREW ORIGINAL |
| same | `1e138b1f1d7e1660eb28b647b7193afe961acb72`; parent `6115dcae…` | 2026-06-03; “Tillers content and fixes…” | manager/hooks/command plus public farm, obstacles, quests, soil SQL | ANDREW ORIGINAL |
| same | `867b69b0d142efaaea10b83970342900d3405f2e`; parent `1e138b1f…` | 2026-06-11; “Battle pet fixes… improved tillers intro quests” | manager/header, `Player.cpp`, companion/daily/soil SQL | ANDREW ORIGINAL |
| same | `bd8ad5515418d94abd35ba7bf71430979243a09c`; parent `aeef0c9d8e6484278ea98d07dde4cc608b077ff4` | 2026-06-21; “Tillers farm: Phase 1 + Phase 2.2…” | manager/header/world hooks, plan, companion/schema SQL | ANDREW ORIGINAL (later branch after intervening commits) |
| same | `155f6444fe435529d38e46ee21ccc05d605e6031`; parent `7de670eccf6d01c8be4c38d00506b53815f7c658` | 2026-06-22; “Fix test suite compilation…” | manager include changes and `tests/scripts/test_tillers.cpp` | ANDREW DESCENDANT MODIFIED, compile-only |
| LoA imported tree | `5006264db873914125f92626252c87bec4b9af92`; shallow-history boundary in this checkout | 2024-09-30; “[Core/Packet] Split SMSG_OVERRIDE_LIGHT…” | imports `2021_04_01_Disables_logros.sql` and 2022 Tending Crops SQL | PRE-ANDREW data import; original upstream commit identities unavailable here |

**FACT — HISTORY:** Parent relationships above were verified with `git show`/object
inspection. No post-Andrew Tillers behavior change was found after the compile-only
commit. No REVERT/REMOVAL commit was found; forks without files simply do not descend
from Andrew's commit chain.

## World-database source inventory

| Source | Release/date/file | Compatibility | Direct observations |
|---|---|---|---|
| LoA release `WDB20231230`, displayed name `WORLD_DB_2024-07-22` | GitHub release record has `published_at=2022-01-25`; asset updated 2024-07-22; `world_548_20240722.7z` | Best located snapshot of the DB LoA itself distributed before Andrew | Both Yoon rows, ranch NPCs/GOs, obstacles, quest relations; no ranch phase definitions |
| Repository SQL imported into LoA/cooler-SAI | files named 2021 and 2022 | Same lineage, migration evidence | global deletion of entry 58646; later SAI content; cannot alone establish final rows |
| SkyFire SFDB release 2→3 | `2014_05_24_14_world_creature.sql` | Independent ancestor/corroborator, schema differs | Yoon at `-180.8438,628.3577,165.4926,1.854483` |
| SkyFire SFDB release 10→11 | `2015_07_17_01_Quest_end_pandaria.sql`, `...02_Quest_start_pandaria.sql` | Independent quest-data corroborator | 58721 starts 30252; 58646 ends 30252 and starts/ends 30254–30256 |
| PandariaCore early releases | release 2/3 copies of early SFDB files | Shared ancestry, not a second independent observation | corroborates data lineage, not independent implementation |

The LoA archive is a snapshot, not merely a migration implication. The confusing
release tag/name/date combination is recorded rather than normalized: tag
`WDB20231230`, displayed name `WORLD_DB_2024-07-22`, old release publication timestamp,
and a 2024-07-22 asset update.

## Farmer Yoon reconstruction and supersession

### Located chain

1. **FACT — DB:** SkyFire's 2014 migration inserts entry 58646 at the same lower-ranch
   coordinates later used by LoA GUID 516067 (rounded differences only).
2. **FACT — DB:** LoA/cooler-SAI's file named `2021_04_01_Disables_logros.sql` says
   `516066,516067 de la creatura id 58646` immediately before
   `DELETE FROM creature WHERE entry IN (64231,58646)`. This proves both GUID/entry
   identities existed before Andrew. It deletes by **entry**, not those GUIDs alone.
3. **FACT — DB:** The LoA 2024 snapshot nevertheless directly contains both rows:
   * 516066: entry 58646, map 870, zone 5805, area 6039, phase mask 1,
     `(-158.578,608.821,175.643,2.14675)`;
   * 516067: same identity fields/mask,
     `(-180.844,628.358,165.493,1.85448)`.
4. **FACT — DB:** That snapshot also has one 58721 row, GUID 516061, phase 1 at
   `(-159.463,633.603,165.409,5.19326)`, and eight 58719 rock rows, GUIDs
   516057–516065 excluding 516061.
5. **FACT — HISTORY:** Andrew's 2026 public-farm migration hides 516066 and promotes
   516067 to mask 128; it did not originate the GUIDs, entries, or coordinates.

### What is and is not final

**FACT — DB:** The final reconstructable **pre-Andrew distributed snapshot state** is
two phase-1 Yoons at the coordinates above. Thus the older global DELETE was
superseded by a re-add or snapshot construction, even though the exact re-add
migration was not located.

**UNKNOWN:** There is no proof that every operator applied exactly that archive plus
every later update, nor that two simultaneously visible Yoons was intended retail
presentation. “Expected” can therefore mean only LoA's distributed pre-Andrew state,
not correct runtime state. The missing migration link remains unknown.

## Quest 30252 and Learn and Grow provenance

* **FACT — DB:** SkyFire's dated 2015 quest-link files establish 58721 as starter and
  58646 as ender for 30252. They also establish 58646 as starter and ender for
  30254, 30255, and 30256.
* **FACT — DB:** The LoA 2024 snapshot has `(58721,30252)` in
  `creature_queststarter`, `(58646,30252)` in `creature_questender`, and 58646 links
  for 30254–30256 on both sides.
* **FACT — DB:** The repository's 2022 Tending Crops migration assigns SmartAI to
  58646, reacts to acceptance of 30254/30255, and summons two 59987 parched cabbages
  at the ranch. This is tutorial scripting evidence, not personal ownership.
* **FACT — HISTORY:** Andrew's migration deletes `(58721,30252)` and replaces it with
  `(58646,30252)`. That is an Andrew-local relationship change, not restoration of
  the independently attested pre-Andrew relationship.
* **INFERENCE:** Duplicate/missing-Yoon risk is partly a world-data supersession
  problem: two static Yoons plus Andrew's hide/promote/dynamic-summon scheme must be
  considered together. It cannot be attributed solely to generic phasing.

## Obstacle, soil, and plot provenance

### Obstacles

**FACT — DB:** The LoA snapshot directly contains farm-bounded, map-870/zone-5805/
area-6039, phase-1 spawns for every questioned entry. Located GUIDs include:

| Entry | Identity | Located farm GUIDs (snapshot) |
|---|---|---|
| 210443 | Weed | 514484, 514495 |
| 210444 | Weed | 514450, 514452, 514455, 514482, 514488 |
| 210445 | Weed | 544507–544509 (additional ranch set) |
| 210446 | Weed | 514453, 514454, 514486, 514494 |
| 210447 | Weed | 514448, 514470, 514485, 514487, 514492, 514493, 514498 |
| 210448 | Weed | 514447, 514449 |
| 210462 | Weed | 514496, 514497 |
| 210451 | Broken Wagon | 514483 |
| 209572 | Giant Rock | 514489 |

The snapshot's `gameobject_template` names independently identify 210443–210448 and
210462 as Weed, 210451 as Broken Wagon, and 209572 as Giant Rock. This converts entry
identity, farm placement, coordinates, and pre-Andrew phase 1 from UNKNOWN to
**FACT — DB**. Andrew did not invent those entries or their farm coordinates.

### Soil/plot/reference population

**FACT — DB:** The same ranch block contains static objects 214036–214056 around the
upper farmhouse; 215705/215706/215719/215720 around the plot; and 210993. It contains
static creatures 58719 (rocks), 58721, 55626, 59669, 66129, and two Yoons. These are
reference candidates, not yet proven semantic plot-state mappings.

**UNKNOWN:** Static SQL alone does not establish which of 214036–214056 represents
each client-visible farm upgrade, whether 215705-series objects are the canonical
soil lifecycle, or which must be static versus dynamically player-owned. Entry names,
spell relations, and build-18414 display behavior need a bounded T5 archaeology pass.

## Phase 128/129 and `phase_definitions`

* **FACT — DB:** Every located ranch obstacle and both Yoons use phase mask 1 in the
  pre-Andrew LoA snapshot—not 128 or 129.
* **FACT — DB:** Searching the snapshot's complete `phase_definitions` INSERT found no
  definition keyed by 1023, 5805, or 5840. Numeric hits elsewhere were unrelated
  tables and were rejected.
* **FACT — HISTORY:** Andrew's `2026_06_02_04_tillers_public_farm_phase.sql` creates
  definitions for 1023/5805/5840 with mask 129 and moves the public objects to 128.
* **INFERENCE:** 128 is an Andrew-local choice for a new public-farm layer, and 129 is
  its mask-1-plus-mask-128 composite. Generic use of bit 128 elsewhere does not give
  these masks Sunsong provenance.
* **UNKNOWN:** Why Andrew included 1023 is not resolved by historical data. In the
  snapshot, Yoon and ranch objects consistently say zone 5805/area 6039; Andrew's
  callback also discusses 5840, while 1023 is not evidenced as the ranch identifier.
  This strengthens the mismatch finding but cannot prove the correct callback tuple
  without runtime/DBC confirmation.

## Personal-phasing/private-owner architecture comparison

| Candidate | Isolation/lifecycle model | Result |
|---|---|---|
| Preserved generic core API | Full `ObjectGuid` private owner; generic visibility intentionally also admits owner group | Best bounded primitive located; no global core change required |
| Andrew | private owner plus colliding `(guidLow << 8) | 1` custom masks, global manager/cache, zone hooks | Not an improvement; mask is non-isolating and lifecycle gaps remain |
| Historical LoA/SFDB SQL | static phase-1 shared objects and quest links | World-data baseline only, not personal-farm architecture |
| Other inspected 5.4.8 lineages | no explainable independent player-owned Tillers system located | No candidate to backport |

**FACT — CODE:** No independent implementation located improves on full-`ObjectGuid`
private ownership while staying local. The established conclusions remain: Andrew's
`m_visibilityLayer` is inert, his masks share bit 1, same-group admission is generic
policy, T2 remains stateless, and any future session owns its returned data.

**BUILD-18414/RUNTIME-DEPENDENT:** Whether private-owner objects render, interact,
phase, and clean up correctly for solo/grouped clients still needs the deliberately
reserved runtime decision. Static archaeology cannot complete T3B.

## Andrew defect/fix matrix

| Concern | Independent evidence | Descendant fix? | Disposition |
|---|---|---|---|
| GUID-derived mask collisions | source arithmetic still shares bit 1 | none | DO_NOT PORT |
| 1023 vs 5805/5840 | snapshot supports zone 5805, not definitions; area rows use 6039 | none | unresolved; runtime/DBC gated |
| same-map zone exit teardown | no alternative/descendant logic found | none | BLOCKED_ON_BUILD_18414_RUNTIME |
| stale custom phase | no reset repair found | none | DO_NOT PORT Andrew mask model |
| old-map object cleanup | no reliable repair found | none | future local session design only |
| duplicate/missing Yoon | two pre-Andrew rows and old delete/re-add history now proven | none | bounded DB design possible, presentation runtime-gated |
| unsafe global cache | no independent validation/fix | none | DO_NOT PORT; retain session ownership |
| fixed GUID assumptions | GUIDs and coordinates proven in LoA snapshot, but installation cardinality is not universal | none | use snapshot fingerprint/preconditions, never blind UPDATE |

## Supersession chains

### Yoon

`2014 SFDB lower Yoon` → `pre-2021 LoA has GUIDs 516066/516067` → `2021 global
DELETE entry 58646` → **unlocated re-add/snapshot assembly** → `2024 LoA snapshot has
both phase-1 rows` → `Andrew 2026 hides 516066, makes 516067 public-128, and uses a
static row to position private summons`.

The chain closes for the distributed snapshot, but not for the missing re-add
migration or retail correctness.

### Quest 30252

`2015 SFDB: 58721 starter / 58646 ender` → `2024 LoA snapshot: same` → `Andrew 2026:
delete 58721 starter / add 58646 starter`. The Andrew mutation supersedes historical
LoA if applied; no independent descendant reverses or validates it.

### Obstacles

`2024 LoA snapshot: all questioned entries, farm coordinates, phase 1` → `Andrew
2026: coordinate-bounded move to 128` (repeated in his later public-phase migration).
Identity and baseline are closed; correctness of phase 128 is not.

### Phase definitions

`2024 snapshot: no 1023/5805/5840 row` → `Andrew 2026 REPLACE three entry-1 rows at
129`. This is creation, not restoration.

## Consolidated provenance matrix

| Datum | Earliest located source/date | Later/final pre-Andrew state | Andrew/post-Andrew | Confidence and implication |
|---|---|---|---|---|
| Farmer Yoon 58646 | SFDB 2014 spawn | two LoA 2024 phase-1 rows | hide/promote/summon; no later fix | high; identity factual, presentation not |
| NPC 58721 | SFDB quest link by 2015 | GUID 516061 plus starter 30252 in LoA snapshot | starter removed | high; do not silently adopt removal |
| GUID 516066 | named in LoA 2021 deletion comment | present at upper coordinates in snapshot | hidden/reference | high identity; runtime role inference |
| GUID 516067 | named in same comment | present at lower SFDB-matching coordinates | phase 128/public | high identity; 128 Andrew-local |
| 30252 starter | 58721 in SFDB 2015 | 58721 in LoA snapshot | changed to 58646 | high; Andrew change needs evidence |
| 30252 ender | 58646 in SFDB 2015 | 58646 in LoA snapshot | retained | high |
| 30254–30256 links | 58646 start/end in SFDB 2015 | same in LoA snapshot; 2022 SAI uses 30254/55 | Andrew expands scripts | high for links, not behavior |
| obstacles 210443–448, 210462, 210451, 209572 | directly located in LoA 2024 snapshot | farm phase-1 spawns and identities | moved to 128; no fix | high DB provenance |
| 214036–214056 / 215705-series | LoA 2024 snapshot ranch block | static phase-1 objects | selectively used by Andrew soil work | medium identity/location; semantic mapping UNKNOWN |
| public mask 128 | no pre-Andrew ranch evidence | obstacles/Yoon are mask 1 | introduced by Andrew | high: Andrew-local |
| composite/zone mask 129 | no pre-Andrew ranch evidence | no relevant definitions | introduced by Andrew | high: Andrew-local |
| definitions 1023/5805/5840 | absent from snapshot | absent | three rows created by Andrew | high for absence/creation; runtime meaning UNKNOWN |

Identity, GUID, coordinates, mask, quest relation, and runtime meaning are intentionally
separate columns/conclusions: agreement on one never proves the others.

## Decision questions

1. **Post-Andrew fixed fork?** **No. FACT — HISTORY:** none inspected descends from
   Andrew with a Tillers behavior repair; his own later change is compile-only.
2. **Genuinely independent implementation?** **No located implementation.** SFDB is
   independent world-data evidence, not a personal-farm system.
3. **Do 516066/516067 predate Andrew?** **Yes. FACT — DB:** both are named in the
   2021 deletion comment and directly present in the 2024 snapshot.
4. **Final expected row state?** The final distributed **pre-Andrew snapshot** is
   proven: both phase-1 rows. Retail-correct or universal deployed state is UNKNOWN.
5. **Obstacle IDs proven?** **Yes. FACT — DB:** templates and bounded farm spawns for
   every requested ID exist in LoA's published snapshot.
6. **Masks 128/129?** Andrew-local Sunsong design choices; no pre-Andrew ranch or
   phase-definition grounding was found.
7. **Better alternative to full-GUID ownership?** No. Nothing located improves it
   without global changes; retain the bounded existing primitive.
8. **1023 vs 5805/5840 resolved?** Not fully. DB evidence strongly supports 5805 for
   ranch rows and does not support 1023, but callback/runtime semantics remain gated.
9. **Descendant lifecycle/cache/Yoon fixes?** No fixes located for same-map exit,
   stale phase, old-map cleanup, duplicate Yoon, or global cache.
10. **What follows?** A **bounded world-data reconstruction design**, not T3B. It
    should fingerprint the published snapshot and specify idempotent preconditions for
    Yoon, 58721, rocks, obstacles, and quest links without changing production SQL.
    T5 can follow once plot-entry semantics receive focused archaeology; T4 remains
    possible but mixes more runtime-dependent quest behavior.

## Candidate future work

| Candidate | Classification | Boundary |
|---|---|---|
| Snapshot-fingerprinted Sunsong baseline contract | `EVIDENCE_SUFFICIENT_FOR_STATIC_DESIGN` | document cardinality/coordinates/links and safe failure conditions only |
| Idempotent restoration of proven Yoon/58721/obstacle rows | `CANDIDATE_BOUNDED_BACKPORT` | only after comparing the target DB; never hard-coded blind mutation |
| T5 plot/soil entry semantics | `NEEDS_MORE_ARCHAEOLOGY` | trace 214036–214056, 215705-series, spells, and later migrations |
| Full dynamic soil/crop world data | `BLOCKED_ON_WORLD_DATA` | semantic state mapping not proven |
| T3B private-owner runtime integration | `BLOCKED_ON_BUILD_18414_RUNTIME` | remains deliberately reserved |
| Andrew masks, `m_visibilityLayer`, global manager/cache | `DO_NOT_PORT` | contradicted or unsupported architecture |
| Andrew's 58721→58646 starter reassignment | `NEEDS_MORE_ARCHAEOLOGY` | historical lineages agree on 58721; requires stronger evidence |

## Unresolved UNKNOWNs

* The exact migration/loader step that re-added GUIDs 516066/516067 after the 2021
  entry-wide deletion.
* Whether any real deployment diverged from the 2024 release snapshot.
* Intended simultaneous visibility and role of the two Yoons for build 18414.
* Exact semantic names and transitions of the ranch's 214036–214056 and
  215705/215706/215719/215720 objects.
* Correct zone/area callback contract among 5805, 5840, 6039, and Andrew's unexplained
  1023, including DBC-vs-core identifier interpretation.
* Retail-correct behavior of 58721 versus 58646 at each tutorial step.

## Explicit runtime gates

The following remain **BUILD-18414/RUNTIME-DEPENDENT**: solo and group visibility;
spawn/despawn packet behavior; interaction with owned soil/crops; map transfer,
same-map exit, logout, relog, and re-entry cleanup; phase reset timing; client display
of static versus summoned Yoon; duplicate prevention; and all gameplay correctness.
No static database fact in this report is proof of any of those outcomes.

## Recommended next preservation task

Open a documentation-only **bounded Sunsong world-data reconstruction design** based
on a machine-readable inventory extracted from `world_548_20240722.sql`, with explicit
row cardinalities, coordinate tolerances, migration supersession, and abort-on-mismatch
rules. This is narrower and better evidenced than T4, prepares T5 without pretending
plot semantics are known, and does not cross the T3B runtime gate.
