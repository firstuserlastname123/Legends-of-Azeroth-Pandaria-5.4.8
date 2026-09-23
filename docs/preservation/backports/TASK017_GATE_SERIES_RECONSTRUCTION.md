# Task 017: Gate of the Setting Sun Series Reconstruction

## Starting State

- **FACT:** The supplied task branch was `work`, the working tree was clean, and
  `HEAD` was `accab660e94c66f4090c0ef9e132402cce4c932a` (`Merge pull request
  #16 from firstuserlastname123/codex/review-and-backport-raigonn-guard`). This
  is the preservation integration history immediately after Task 016, although
  the task checkout is named `work` rather than `preservation/main`.
- **FACT:** the last eight commits included Task 016's source commit
  `3954a6f`, Task 015, Task 014, and Task 013. There were no pre-existing
  changes to preserve.
- **FACT:** this task read the preservation workflow, development-build,
  architecture, fork-archaeology, candidate-review, Task 010, and Task 016
  records before reviewing the fork.
- **FACT:** no build, server, SQL execution, cherry-pick, merge, or source edit
  was performed. A temporary clone at `/tmp/task017-gate-fork` was used, so no
  preservation-repository remote was added or changed.

## Fork and Candidate 18 Provenance

- **FACT:** the reviewed repository is
  `https://github.com/ingussuveiks-dev/Legends-of-Azeroth-Pandaria-5.4.8`.
  Candidate 18 resolves exactly to
  `00d1cd9976a874c65daf901e1a2740d4393e7be5`, parent
  `59d06e966854609a9f595074fe3c3066df741f39`, subject `Fix Gate cannon
  feedback and stray headbutt stun`.
- **FACT:** all 14 commits inventoried below are a linear first-parent sequence
  from `566d274c` through Candidate 18. The two earliest Gate commits were not
  in the prompt's short known list but are necessary to explain the later
  source shape.
- **FACT:** every commit was authored and committed by ingussuveiks-dev
  `<ingussuveiks@gmail.com>` on 2026-09-17 at the displayed `+03:00` time.
- **INFERENCE:** adjacency establishes development order, not one indivisible
  backport. The diffs split into independent encounter, cannon, elevator,
  world-data, updater, and Playerbots concerns.

## Gate Commit Timeline

Classification abbreviations are `CORE_DUNGEON_BEHAVIOR` (CORE),
`PLAYERBOTS_ONLY` (PB), `MIXED_CORE_AND_PLAYERBOTS` (MIXED),
`DATABASE_OR_WORLD_DATA` (DB), and
`BUILD_OR_UPDATER_INFRASTRUCTURE` (INFRA).

| Commit (full SHA; parent) | Time / subject | Files and classification | Dependency and later modification |
| --- | --- | --- | --- |
| `566d274cc60f9af2b08eaa5610b9911a44fe76d1`; `015e2d36536c166a7257da773482dfbcbed1e318` | 12:48:58, `Fix Gate of the Setting Sun encounters` | Five Gate files: `boss_commander_rimok.cpp`, `boss_raigonn.cpp`, `boss_striker_gadok.cpp`, `gate_of_the_setting_sun.h`, `instance_gate_of_the_setting_sun.cpp`; CORE only; no SQL/PB. | First distinct-target/bombarder-pool and bombardment cleanup structure required by every later cannon commit. Also independent Rimok, Raigonn, and Gadok-emote changes. Cannon is revised by `3d478b7`, `3eb3c38`, `67a6a17`, `00d1cd9`.
| `3d478b71116b53a335780f7e1396ae2d6986ae15`; `566d274cc60f9af2b08eaa5610b9911a44fe76d1` | 14:11:53, `Fix Gate of the Setting Sun interactions` | Four Gate source files plus `sql/updates/world/2026_09_17_00_world_gate_setting_sun_elevator.sql`; CORE + DB, conceptually mixed. | Converts cannon from `SpellHit(116554)` to `OnSpellClick`; introduces elevator lever/data/SQL; adjusts cannoneer paths, a door registration, and defender handling. Elevator revised by `4d4d9d6`, corpse series, and `9f80fae`; cannon revised later.
| `56c140135d6c7d4f34d0301bee142ddd231dae6d`; `3d478b71116b53a335780f7e1396ae2d6986ae15` | 14:30:35, `Fix Gate of the Setting Sun cave-in template` | One new world update, `2026_09_17_01_world_gate_setting_sun_cave_in.sql`; DB only. | Independent of later source. Its installation in the fork is affected operationally by `394248d9`, but the data claim is separate.
| `394248d9aacf44be6e9f4e64f95209c92ebc28f4`; `56c140135d6c7d4f34d0301bee142ddd231dae6d` | 14:33:38, `Enable database updates for new installations` | `src/server/worldserver/worldserver.conf.dist`; INFRA only; no Gate source, SQL, or PB. | Changes global default `Updates.EnableDatabases` from 0 to 7. It is not a Gate behavior prerequisite and must not be bundled merely because adjacent.
| `3809418da433a5144375d2a9a4d4835235040cc3`; `394248d9aacf44be6e9f4e64f95209c92ebc28f4` | 14:57:28, `Fix Gate wall trash movement` | One new world update, `2026_09_17_02_world_gate_setting_sun_wall_trash_movement.sql`; DB only. | Independent final change to six spawn rows; no later commit changes it.
| `db9fe1420409169817051c84c9d0cd58a713c882`; `3809418da433a5144375d2a9a4d4835235040cc3` | 15:01:52, `Despawn Gate elevator defender corpses` | `instance_gate_of_the_setting_sun.cpp`; CORE only. | Extends `3d478b7` death handling from fall defenders to Serpent's Spine defenders. `3eb3c38` adds lift defender 58146; passenger attachment later changes the rationale but does not remove death despawn.
| `4d4d9d60a9d8e1bc8a289ad6e865dbcd8efed88a`; `db9fe1420409169817051c84c9d0cd58a713c882` | 15:06:03, `Prevent Gate elevator reversal in transit` | `gate_of_the_setting_sun.cpp`; CORE only. | Requires the lever/source/data/SQL introduced by `3d478b7`; endpoint-tests transport period/pause/timer before changing state. No later change replaces this guard.
| `3eb3c38bdbfec4281918b622573f242bdb40cea1`; `4d4d9d60a9d8e1bc8a289ad6e865dbcd8efed88a` | 15:13:59, `Fix Gate flak cannon sequence` | `boss_striker_gadok.cpp`, Gate header, instance source; CORE only. | Requires the `566d274c` cannon pool and `3d478b7` click form. Keeps cannon spell-click enabled until no random bombarder remains; also adds lift-defender 58146 to corpse despawn. Cannon revised by `67a6a17` and `00d1cd9`.
| `1cb74544651f53db5735cda6eae28247d0f0b983`; `3eb3c38bdbfec4281918b622573f242bdb40cea1` | 15:47:41, `Clean up Gate elevator corpses before movement` | `gate_of_the_setting_sun.cpp`; CORE only. | Requires elevator lever code; searches nearby entry 58146 corpses before movement. Its lever-side cleanup is removed by `3de7e80d` and is not part of final fork state.
| `3de7e80df7037c94f46879c1b6f5ee50ab250ff5`; `1cb74544651f53db5735cda6eae28247d0f0b983` | 16:03:21, `Attach Gate defender corpses to elevator` | Gate header-adjacent instance work in `instance_gate_of_the_setting_sun.cpp` and removal from `gate_of_the_setting_sun.cpp`; CORE only. | Supersedes `1cb7454` cleanup by tracking entry 58146 and attaching it as a transport passenger. Attachment is corrected by `9f80fae`.
| `67a6a17dcabc25d5a1cf29e59a89eced2f04b9e1`; `3de7e80df7037c94f46879c1b6f5ee50ab250ff5` | 16:14:57, `Show Gate flak cannon projectiles` | `boss_striker_gadok.cpp`; CORE only. | Requires the prior cannon click/pool sequence. Replaces immediate spell 116553/fall with visual 29216 and scheduled impact; superseded at the projectile operation and speed by `00d1cd9`.
| `9f80faebe41c6fcc0ad303a02f77fe08451f3ee7`; `67a6a17dcabc25d5a1cf29e59a89eced2f04b9e1` | 16:32:12, `Fix Gate elevator passengers and bot following` | Gate instance source plus `modules/mod_playerbots/src/AI/PlayerbotAI.cpp` and `.h`; MIXED. | Core half corrects `3de7e80d` attachment with a 500 ms retry and endpoint proximity; PB half boards/exits bots and adds follow recovery. Core can be reviewed independently. PB recovery is refined by `59d06e9`.
| `59d06e966854609a9f595074fe3c3066df741f39`; `9f80faebe41c6fcc0ad303a02f77fe08451f3ee7` | 16:50:13, `Recover bots stuck on Gate stairs` | Playerbot AI `.cpp`/`.h`; PB only; no Gate source or SQL. | Requires the PB block introduced by `9f80fae`; widens recovery and adds start-position progress tracking. Final PB state.
| `00d1cd9976a874c65daf901e1a2740d4393e7be5`; `59d06e966854609a9f595074fe3c3066df741f39` | 17:05:53, `Fix Gate cannon feedback and stray headbutt stun` | `boss_striker_gadok.cpp`, `boss_raigonn.cpp`; CORE only. | Cannon half requires `566d274c` → `3d478b7` → `3eb3c38` → `67a6a17`; it replaces visual 29216 with spell 133711. Raigonn half is independent and is already integrated by Task 010.

**FACT:** 14 commits were reviewed. Each was inspected with commit metadata,
summary/statistics, and its full patch; none was imported.

## Functional Families

- **FACT — Raigonn:** `566d274c` changes weak-spot GUID storage API and fixes
  the swarm-bringer reschedule event; `00d1cd9` independently fixes the
  Battering Headbutt filter and adds the `IN_PROGRESS` guard. The latter is the
  last fork touch. Task 016's weak-spot death guard does not occur in this fork.
- **FACT — Gadok flak cannon/bombardment lifecycle:** begins in `566d274c`,
  changes activation in `3d478b7`, click availability in `3eb3c38`, projectile
  feedback in `67a6a17`, and reaches its final fork state in `00d1cd9`. Final
  behavior depends on all of those intermediate source structures, although a
  future patch should be synthesized as a final-state diff.
- **FACT — elevator control:** `3d478b7` introduces lever script, object/data
  IDs, GUID capture, and its SQL binding; `4d4d9d6` adds the final in-transit
  reversal guard. It is separate from passenger/corpse behavior.
- **FACT — defender corpse/passenger state:** death cleanup begins in
  `3d478b7`, expands through `db9fe14` and `3eb3c38`; `1cb7454` adds a temporary
  pre-movement cleanup; `3de7e80` replaces that temporary approach with
  transport attachment; `9f80fae` supplies the final attachment timing and
  position gates.
- **FACT — cannoneer paths and door registration:** isolated hunks in
  `3d478b7`; no later changes.
- **FACT — wall trash:** only `3809418`; world-spawn data only.
- **FACT — cave-in template:** only `56c1401`; world-template data only.
- **FACT — Rimok/add-generator and Gadok emote:** independent source hunks in
  `566d274c`; no later changes.
- **FACT — Playerbots:** introduced as the module half of `9f80fae`, finalized
  by `59d06e9`; it is not required for normal-player elevator or cannon logic.
- **FACT — updater defaults:** only `394248d`; global infrastructure, not Gate
  dungeon behavior.

## Current Preservation Comparison

Comparison is against starting commit `accab660`, not original upstream.

| Logical final-fork change | Status | Direct comparison result |
| --- | --- | --- |
| Candidate 18 Battering Headbutt predicate and encounter guard | `PRESENT_EXACTLY` | Task 010 contains the candidate comment, outside-rectangle removal predicate, and null/non-`IN_PROGRESS` early return.
| Candidate 9 weak-spot `JustDied` instance guard | `SUPERSEDED_BY_PRESERVATION` relative to fork | Task 016 adds preservation-only safety absent from Candidate 18's fork; any future Gate diff must retain it.
| `566d274c` weak-spot `SetGuidData` and swarm-bringer reschedule | `ABSENT` | Current code still uses `SetData64` and reschedules `EVENT_SUMMON_ENGULFER` in the swarm-bringer case.
| `566d274c` Rimok/add-generator final fork state | `CONFLICTING_IMPLEMENTATION` | Current still schedules alternating `EVENT_SWARM`, while its add-generator actions already remove both periodic auras; the fork removes boss alternation and removes only the opposite aura in each action. This is not an exact or obviously equivalent change.
| `566d274c` Gadok death-emote removal | `ABSENT` | Current retains `SAY_STIKE_EMOTE` and the death `Talk` call.
| Final flak cannon/bombardment lifecycle | `ABSENT` | Current is the old `SpellHit(116554)` loop, repeatedly requests a random GUID, casts 116553, falls immediately, and despawns after 2 seconds. It lacks pool-removal data, safe empty selection, click sequencing, persistent click availability, delayed impact, and spell 133711.
| Elevator lever/control, including SQL binding | `ABSENT` | No lever script, elevator data/object IDs, GUID capture, or binding update is present.
| Cannoneer waypoint split and Raigonn door registration | `ABSENT` | Current retains loop starts 11/0/10, original right-path ordering, and does not add the Raigonn door in that hunk.
| Defender death cleanup and lift passenger attachment | `ABSENT` | No lift-defender ID, death special cases, tracking, attachment helper, or retry timer is present.
| Cave-in template row | `ABSENT` | Entry 211302 is not found in checked-in base/current update SQL.
| Wall-trash spawn updates | `ABSENT` | The six GUIDs and update are not found in checked-in base/current update SQL.
| Playerbot Gate elevator/follow recovery | `ABSENT` | No Gate-specific follow state or update block exists in current module code.
| Global updater default | `ABSENT` | Current `worldserver.conf.dist` retains `Updates.EnableDatabases = 0`.

- **INFERENCE:** absence means only that the fork's logical change is not in
  the current source/data tree; it is not evidence that the fork behavior is
  retail-correct.
- **UNKNOWN:** deployed world databases can differ from checked-in SQL, so SQL
  `ABSENT` does not prove that deployed rows have the old or missing values.

## Raigonn Changes Already Integrated

- **FACT:** Candidate 18's impossible conjunction was replaced by a predicate
  which removes null players or players outside `X [919, 997]`, `Y [2360,
  2372]`. This exact Candidate 18 hunk and comment are present from Task 010.
- **FACT:** Candidate 18's `caster->GetInstanceScript()` null check and
  `GetBossState(DATA_RAIGONN) != IN_PROGRESS` early return are also present
  exactly from Task 010.
- **FACT:** Task 016 is separate Candidate 9 provenance. It guards
  `npc_raigonn_weak_spotAI::JustDied`'s encounter-frame call and Raigonn lookup
  with `if (instance)`. Candidate 18 and its ancestors do not supply that
  change; a future range transplant would risk omitting or overwriting it.
- **FACT:** Candidate 18 does not depend on `566d274c`'s two other Raigonn
  edits. Those remain absent and unapproved.
- **UNKNOWN:** Task 010 documents remaining runtime uncertainty about the
  rectangle, vertical overlap, boundary edges, and delayed callback timing.

## Flak Cannon Development History

### Baseline and first structure

- **FACT:** current preservation behavior matches the fork before
  `566d274c`: `SpellHit` accepts only spell 116554 after Gadok is `DONE`; a
  five-iteration loop asks `DATA_RANDOM_BOMBARDER` each time, casts spell
  116553, immediately calls `MoveFall`, and requests despawn after 2000 ms.
  It does not remove a selected GUID before the next random selection.
- **FACT:** `566d274c` adds `DATA_BOMBARDER_DEFEATED = 9`, safe empty-list
  returns for random bombarder/stalker access, and removal/cleanup helpers. A
  selected bombarder is removed from the pool before lookup, making up to five
  distinct selections per activation. Empty pool or completed brazier state
  also clears bombardment fire spell 106875; completed state despawns remaining
  bombarders, including after instance-load restoration.

### Activation and repeated firing

- **FACT:** `3d478b7` changes the cannon from reacting to `SpellHit(116554)` to
  `OnSpellClick`. It requires a successful click result, a non-null instance,
  and Gadok `DONE`, then removes the spell-click flag and performs the loop.
  This source change uses the pool structure from `566d274c`.
- **FACT:** `3eb3c38` stops removing the click flag at the start. After each
  group of up to five, it removes the flag only if
  `DATA_RANDOM_BOMBARDER` is empty. Thus both cannons remain usable while any
  of the stated 18 bombarders remain.
- **INFERENCE:** switching to `OnSpellClick` avoids relying on a subsequent
  `SpellHit(116554)` callback, but the repository alone does not prove which
  callback order or spell-click data is correct for a build-18414 client.

### Projectile feedback and final source state

- **FACT:** `67a6a17` removes spell 116553 from this hit block. It sends raw
  spell visual 29216 at 40 units/second and schedules fall/despawn for a
  distance-derived 500–2500 ms delay.
- **FACT:** Candidate 18 replaces only that visual operation with triggered
  `CastSpell(bombarder, 133711, true)` and changes the locally assumed
  projectile speed to 20 units/second. It retains the same 500–2500 ms clamp,
  target, event scheduling, pool removal, five-target limit, and final
  click-flag check.
- **FACT:** spell 133711 first appears in this fork series in Candidate 18; no
  earlier reviewed Gate commit uses it. Visual 29216 exists only in the
  superseded `67a6a17` state. Spell 116554 exists only in the original
  `SpellHit` gate, and spell 116553 is the original/early impact cast removed by
  `67a6a17`.
- **FACT:** the final target is each randomly selected live creature resolved
  from `DATA_RANDOM_BOMBARDER`; its GUID is removed before resolution. There is
  no player target. A missing/deleted creature still consumes its pool entry.
- **FACT:** the final cannon code adds no `Vehicle`, passenger, seat, enter, or
  exit operation. The relevant interaction is creature spell-click state, not
  a vehicle ride. The separate elevator passenger work is not a cannon
  prerequisite.
- **FACT:** no cannon-specific SQL is added by this chain. It assumes existing
  creature instances, bombardier/stalker lists, spell-click configuration, and
  spell data. The SQL in `3d478b7` binds only elevator gameobject 211284.
- **INFERENCE:** a future cannon patch must represent the final combined state
  from `566d274c`, `3d478b7`, `3eb3c38`, `67a6a17`, and `00d1cd9`, rather than
  applying Candidate 18's small tip diff to current preservation: the tip's
  context and delayed impact do not exist in the current tree.

## Spell 133711 Findings

- **FACT:** current live C++ and current update/base SQL contain no reference to
  133711. Candidate 18 adds `SPELL_FLAK_FIRE = 133711` and casts it, triggered,
  from the cannon to a selected Krithik bombarder.
- **FACT:** two duplicate historical SQL snapshots under `sql/old/` contain a
  SmartAI row for source entry 67885 whose action casts 133711 and whose
  comment calls it `Flak Fire`. The same snapshots contain another row for the
  same source casting 133710 with comment `Flak Fire Impact`.
- **FACT:** no repository spell-script binding, C++ spell script, checked-in
  DBC/DB2 metadata, or extracted spell record for 133711 was found. The
  historical SmartAI comment is repository evidence of intended naming/use,
  not authoritative spell metadata for this dungeon.
- **UNKNOWN:** whether spell 133711 has the asserted travelling and impact
  visuals, accepts this caster/target pairing, travels at 20 units/second, or
  is retail-correct for Gate of the Setting Sun on client build 18414. The fork
  comment alone does not establish those claims.

## Elevator / Trash / Corpse Series

- **FACT — elevator control:** `3d478b7` creates
  `go_setting_sun_elevator_lever`, identifies platform 211013 and lever 211284,
  stores the platform GUID, and binds the lever ScriptName in SQL. Its initial
  toggle can reverse while travelling. `4d4d9d6` requires a real transport and
  permits state change only when timer equals `period - 1` or `pause`. The
  final control state needs both commits and the ScriptName row. It is absent
  in preservation and requires endpoint/click/runtime validation.
- **FACT — death cleanup:** `3d478b7` immediately despawns dead fall defenders
  and skips dead defenders before a scripted jump. `db9fe14` adds entry
  `NPC_SERPENTS_SPINE_DEFENDER`; `3eb3c38` defines/adds lift defender 58146.
  These changes also alter challenge-enemy counting by returning early for the
  named entries. They are absent and need challenge-mode as well as corpse
  validation.
- **FACT — temporary corpse cleanup:** `1cb7454` scans entry 58146 within 10
  units of the elevator immediately before movement and despawns dead units.
  `3de7e80` removes that entire scan; it is superseded and must not be a final
  backport unit.
- **FACT — passenger final state:** `3de7e80` records upper-platform entry
  58146 spawns, converts their world position to transport-local position,
  sets transport home, and adds them as passengers. `9f80fae` adds a 500 ms
  retry and attaches only when defender/platform horizontal distance is at
  most 10 and vertical difference at most 5, covering creation order and the
  platform being at the other endpoint. This final core state depends on
  `3d478b7` elevator GUID capture, `3eb3c38`'s ID, and `3de7e80`'s helper; it
  does not depend on the Playerbots half of `9f80fae`.
- **FACT — wall trash:** `3809418` updates `creature` on map 962 for GUIDs
  541801, 541802, and 542282–542285, limited to entries 58108/59801 and old
  `wander_distance = 10`, `MovementType = 1`; it sets both values to zero.
  No later source/state-machine commit assumes this data.
- **INFERENCE:** elevator control and lift-passenger attachment share elevator
  GUID infrastructure but are separable final-state reviews. Death cleanup is
  related operationally yet changes challenge counting and should be reviewed
  separately rather than treated as a mechanical prerequisite.
- **INFERENCE:** isolated review is practical for elevator control, attachment,
  and wall-trash data, but none is ready to call correct without runtime/world
  evidence. Transport timer endpoints, spawn load ordering/coordinates,
  corpse behavior, challenge count, and pathing must be observed.

## Cave-In and World-Data Findings

- **FACT:** `56c1401` performs `INSERT IGNORE` into `gameobject_template` for
  entry 211302 with type 5, displayId 2230, name `Cave In`, size 1, data1/data6
  zero, and `VerifiedBuild` 18414. Its comments say spell 115441 summons it and
  compare display 2230 to template 211305.
- **FACT:** neither entry 211302 nor the six wall-trash GUIDs occur in the
  checked-in base schema/data or current update set. The repository's base
  world SQL does not provide the deployed content rows needed to compare their
  full state. No SQL was executed.
- **FACT:** `3d478b7` separately updates `gameobject_template` entry 211284's
  `ScriptName` to `go_setting_sun_elevator_lever`. The relevant baseline row is
  likewise not present in checked-in base/current updates.
- **FACT:** `394248d9` merely changes the default global update mask from 0 to
  7. It neither supplies data nor prove that any Gate update ran, and it is not
  a legitimate prerequisite for reviewing the SQL contents.
- **INFERENCE:** all three world-data changes can be read syntactically, but
  their meaningful applicability requires a deployed-world query and runtime
  evidence. The cave-in provenance comment and `VerifiedBuild` value are
  assertions in the fork patch, not independent provenance.
- **UNKNOWN:** actual deployed rows for templates 211302/211284 and the six
  creature GUIDs; whether display 2230/type 5/fields are complete; whether the
  GUIDs are stable in the target database; and whether these changes reproduce
  retail build-18414 behavior.

## Playerbots Boundary

- **FACT:** `9f80fae` is mixed. Its one core Gate file implements corrected
  defender attachment timing. Its module files add map-962/LFG-controlled bot
  and pet boarding/exiting for transport 211013, teleport beside the real
  master, and a five-second post-lift follow recovery. Neither half calls into
  the other; the core half can be reviewed and built independently.
- **FACT:** `59d06e9` is Playerbots-only. It changes the follow threshold from a
  45-yard/vertical-or-LOS test to distance over 12 yards and detects progress
  using both distance and stored X/Y/Z movement before teleport recovery. It
  adds three state fields and resets all recovery state outside context.
- **FACT:** normal-player elevator control (`3d478b7`/`4d4d9d6`) and core
  defender passenger attachment do not require Playerbots. Conversely, the bot
  boarding block assumes the Gate map and elevator entry but not the lever AI.
- **INFERENCE:** no Playerbots hunk belongs in a normal fast-development build.
  Any future PB unit must use a separate `USE_MODULES=1`, `PLAYERBOTS=1`
  profile plus module compilation and real bot/elevator/navmesh validation.
- **UNKNOWN:** whether forced teleports, passenger offsets, 12-yard threshold,
  and five-second recovery are safe across the deployed navmesh and all LFG
  group states.

## Final-State Backport Units

These are review units, not approvals. “Core fast” means the documented
Playerbots/modules/tools/Eluna-disabled scripts/worldserver profile.

| Unit | Final-state source | Files / prerequisites | Preservation status; build/data impact | Required runtime evidence / missing evidence |
| --- | --- | --- | --- | --- |
| Raigonn headbutt filter/state guard | Exact Raigonn hunk of `00d1cd9` | `boss_raigonn.cpp`; none | `PRESENT_EXACTLY` from Task 010; already built there; no DB | Task 010's build-18414 rectangle, floors, reset/evade, and timing tests remain.
| Preserve weak-spot null-instance guard | Task 016 Candidate 9 provenance, not this fork | `boss_raigonn.cpp`; must dominate later Gate comparisons | `SUPERSEDED_BY_PRESERVATION`; already integrated/built; no DB | Task 016's normal encounter and optional non-instance path tests remain.
| Raigonn weak-spot GUID API + swarm scheduling | Isolated final hunks from `566d274c` | `boss_raigonn.cpp`; none established | `ABSENT`; Core fast; no DB patch | API semantics and summon cadence/target runtime evidence; retail provenance missing.
| Rimok generator state | Final `566d274c` logical diff against current, not raw commit | `boss_commander_rimok.cpp`; must reconcile current stronger aura removal | `CONFLICTING_IMPLEMENTATION`; Core fast | Add cadence, aura exclusivity, wipe/reset, challenge behavior; retail evidence missing.
| Gadok flak cannon and bombardment lifecycle | Synthesized final diff from `566d274c` + cannon hunk of `3d478b7` + `3eb3c38` + `67a6a17` + cannon hunk of `00d1cd9` | `boss_striker_gadok.cpp`, Gate header, instance source; retain preservation Raigonn/Task 016; no elevator/PB prerequisite | `ABSENT`; Core fast; no new DB patch, but existing spell-click/spawn/spell data required | Four clicks/18 targets, uniqueness, both cannons, empty pool, save/load and brazier cleanup, projectile/impact timing, disconnect/reset; authoritative 133711 and callback evidence missing.
| Elevator lever/control | Final source of `3d478b7` + `4d4d9d6`, plus only its lever binding SQL | Gate `.cpp`/header/instance + world update; requires target DB row 211284 | `ABSENT`; Core fast plus DB/runtime workflow | Both endpoints, double click in transit, reload, concurrent players, actual transport period/pause; deployed row and retail evidence missing.
| Cannoneer path split | Exact final path hunks of `3d478b7` | Gate `.cpp`/header; independent | `ABSENT`; Core fast; no DB | Observe both cannoneers before/jump/after routes; coordinate provenance missing.
| Raigonn door registration | Exact isolated `3d478b7` instance hunk | Gate instance; existing door metadata prerequisite | `ABSENT`; Core fast; no DB patch | Encounter transitions, wipe/reload, physical doors; retail state evidence missing.
| Defender death/challenge handling | Final logical state from `3d478b7` + `db9fe14` + `3eb3c38` | Gate header/instance; separate from superseded `1cb7454` | `ABSENT`; Core fast; no DB patch but spawn entries required | Three defender classes, corpses, scripted jumps, challenge enemy count; deployed spawn/runtime evidence missing.
| Lift defender passenger attachment | Final core-only diff from `3de7e80` + core half of `9f80fae` | Gate header/instance; elevator GUID capture from `3d478b7`; do not include PB | `ABSENT`; Core fast; no DB patch but entry 58146 spawns required | Both object creation orders, both endpoints, repeated trips, corpses/home position; coordinates/spawns and retail evidence missing.
| Cave-in template | `56c1401` SQL final state | one world migration; no updater-default prerequisite | `ABSENT`; DB/runtime workflow, no compilation | Query deployed template, trigger spell 115441, visual/collision/lifetime; authoritative template provenance missing.
| Wall trash stationary spawns | `3809418` SQL final state | one world migration; no source prerequisite | `ABSENT`; DB/runtime workflow | Verify exact GUID/entry/old-value predicates and observe idle/combat/evade pathing; target DB provenance missing.
| Playerbot Gate boarding/follow recovery | PB half of `9f80fae` finalized by `59d06e9` | Playerbot AI `.cpp`/`.h`; independent of core attachment | `ABSENT`; module-enabled Playerbots build; no SQL patch, deployed navmesh required | Bot/pet boarding/exiting, stairs/passage, combat/teleport/LFG cases; navmesh traces and safety evidence missing.
| Updater default | `394248d9` | `worldserver.conf.dist`; independent of Gate | `ABSENT`; configuration/infrastructure review, not a Gate unit | Installation/security/operations policy evidence; should be a separate non-content task.

**FACT:** `1cb7454` is deliberately not a unit because its only change is
removed in the final fork. The Gadok death-emote removal and miscellaneous
`566d274c` encounter changes should receive separate evidence reviews rather
than be hidden in the cannon unit.

## Suggested Next Isolated Reviews

1. **Gadok flak cannon final-state review (recommended, not approved).** It is
   pure core source with a clear five-commit final-state reconstruction, no new
   SQL or Playerbots dependency, and a focused three-file scope. The task must
   first establish spell/callback evidence for 133711 and `OnSpellClick`, then
   synthesize rather than cherry-pick the final diff, build scripts and
   worldserver, and require targeted dungeon runtime tests. **UNKNOWN:** until
   that evidence exists, suitability for review is not correctness.
2. **Elevator in-transit reversal review.** Review `3d478b7`'s minimal lever
   source/data foundation plus `4d4d9d6` as one final-state unit. It is narrow
   and Playerbots-free, but unlike the cannon it has an explicit world-template
   ScriptName dependency; deployed-row inspection and transport runtime traces
   are prerequisites. If a no-SQL implementation task is required, no second
   elevator unit is ready and this should remain analysis-only.

## Unresolved Retail / Build-18414 Evidence

- **UNKNOWN:** authoritative spell records/visual behavior for 133711, 133710,
  116553, 116554, and visual 29216; cannon spell-click DB flags/callback order;
  projectile speed and impact scheduling; and the retail number/selection of
  bombarders per click.
- **UNKNOWN:** the correct elevator endpoint timer semantics, lever template
  binding, spawn coordinates/order, transport passenger behavior, defender
  corpse policy, and challenge-count treatment.
- **UNKNOWN:** authoritative cannoneer waypoint splits, Raigonn door lifecycle,
  Rimok generator alternation, Gadok death talk, and `566d274c`'s other
  encounter behavior.
- **UNKNOWN:** deployed world rows and provenance for cave-in 211302, lever
  211284, wall-trash GUIDs, defender 58146 spawns, bombarders, and stalkers.
- **UNKNOWN:** retail correctness of the cave-in fields/display, stationary
  wall-trash policy, and all Playerbots recovery thresholds. Playerbots behavior
  is custom/module behavior rather than evidence of normal retail dungeon flow.
- **INFERENCE:** compilation in a later task could establish API and link
  compatibility only. Build-18414 client, deployed database, extracted client
  data, transport, dungeon, challenge-mode, and (separately) Playerbots runtime
  testing remain necessary.
