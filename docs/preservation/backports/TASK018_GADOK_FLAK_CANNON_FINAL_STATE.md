# Task 018: Gadok Flak Cannon Final-State Review

## Scope and starting state

Task 018 started from clean commit
`597e012f43785a633f83f1932fa03771dfe2559b` (the Task 017 merge). The hosted
task branch is named `work`; its HEAD is the current `preservation/main`
integration history after Task 017. No unexpected changes were present.

This was an evidence gate, not authorization to import the fork. The exact
prospective three-file source unit is:

- `src/server/scripts/Pandaria/GateOfTheSettingSun/boss_striker_gadok.cpp`;
- `src/server/scripts/Pandaria/GateOfTheSettingSun/gate_of_the_setting_sun.h`;
- `src/server/scripts/Pandaria/GateOfTheSettingSun/instance_gate_of_the_setting_sun.cpp`.

Tasks 010 and 016 both affect `boss_raigonn.cpp`, not those cannon hunks.
Nevertheless, a range transplant could overwrite their preserved Raigonn
changes, so no historical commit was applied. Elevator, defender/corpse,
cannoneer-path, Playerbots, SQL, Gadok death-emote, and unrelated encounter
changes are explicitly outside this unit.

## Five-commit provenance

The fork was inspected in a temporary clone without changing repository
remotes. Full lineage and relevant roles are:

1. `566d274cc60f9af2b08eaa5610b9911a44fe76d1` — introduces
   `DATA_BOMBARDER_DEFEATED`, safe empty selection, consumption before target
   resolution, and bombardment cleanup/load behavior.
2. `3d478b71116b53a335780f7e1396ae2d6986ae15` — replaces the cannon's
   `SpellHit(116554)` activation with successful `OnSpellClick`; its elevator,
   path, door, corpse, and SQL changes are unrelated.
3. `3eb3c38bdbfec4281918b622573f242bdb40cea1` — keeps a cannon clickable until
   the shared bombarder pool becomes empty; its lift-defender changes are
   unrelated.
4. `67a6a17dcabc25d5a1cf29e59a89eced2f04b9e1` — replaces immediate spell
   116553/fall with raw visual 29216 and delayed fall/despawn using a
   distance-derived delay.
5. `00d1cd9976a874c65daf901e1a2740d4393e7be5` — replaces raw visual 29216 with
   triggered spell 133711 and changes the assumed speed from 40 to 20 units
   per second; its Raigonn change is unrelated and already addressed by Task
   010.

The relevant `--stat`, `--summary`, full metadata, and complete patches were
inspected for every commit. The review below describes their combined final
state, not any intermediate commit.

## Current preservation behavior

The following are **FACTS** from current source unless marked otherwise.

1. A player-visible spell-click interaction is expected to execute a
   database-defined click spell. If spell 116554 then hits the cannon, the
   cannon's `SpellHit` callback handles it. Thus interaction is indirect:
   CMSG spell-click/default click spell, followed by `SpellHit(116554)`.
2. The cannon callback is `SpellHit`, not `OnSpellClick`.
3. One accepted spell hit loops five times and can affect five successful
   creature resolutions.
4. Each iteration independently samples the same `std::list<ObjectGuid>`.
   It does not remove a selected GUID. Therefore the same bombarder can be
   selected repeatedly, including within one interaction.
5. Cannon code never removes its spell-click flag. **UNKNOWN:** the complete
   click lifecycle depends on deployed template/spell-click data; checked-in
   current source does not make the cannon clickable after Gadok.
6. Each successful target resolution triggers spell 116553 from cannon to
   bombarder; there is no explicit raw visual packet or flight delay.
7. The target immediately receives `MoveFall()` and is requested to despawn
   2000 ms later. The spell is feedback; C++ performs the fall/despawn.
8. Instance initialization clears and repopulates the bombarder and stalker
   GUID lists through creature creation. Creature removal erases a matching
   bombarder GUID. Save/load preserves boss and brazier state but not consumed
   bombarder identities. Current load does not clear bombarders/fire for an
   already completed brazier. **UNKNOWN:** grid loading and deployed spawn
   behavior determine the observable reload result.

The instance uses `std::list<ObjectGuid>` for bombarders and bomb stalkers.
Creation uses `push_back`; uniqueness therefore follows only if each creature
creation callback occurs once per live spawn (**INFERENCE**, not a container
invariant). Current random selection calls
`SelectRandomContainerElement` without an empty guard.

## Reconstructed fork final behavior

The final fork uses `OnSpellClick(Unit*, bool&)`, returns unless `result` is
true, an instance exists, and Gadok is `DONE`, then repeats at most five times.
Each iteration obtains a random GUID, stops on empty, removes all occurrences
of that GUID from the instance list before creature lookup, and, if resolution
succeeds, triggers spell 133711 from the cannon to the bombarder. It schedules
fall plus a 2000 ms despawn on the bombarder event processor after:

```text
uint32(distance(cannon, bombarder) / 20.0f * 1000), clamped to [500, 2500] ms
```

After the loop, the clicked cannon loses `UNIT_NPC_FLAG_SPELLCLICK` only when
the shared pool is empty. Both cannon instances query and consume the one
instance-wide pool, but only the cannon executing the final click explicitly
loses its flag. A later click on the other cannon can execute the default click
spell and callback, find no GUID, then remove its own flag.

The instance final state adds `DATA_BOMBARDER_DEFEATED`; safely returns an
empty GUID from empty bombarder/stalker pools; removes bombarders on creature
removal or selection; clears stalker aura 106875 when the pool empties; and,
when brazier state becomes or loads as `DONE`, clears the pool, despawns its
resolvable bombarders, and removes aura 106875 from resolvable stalkers. New
bombarders created after a loaded/completed brazier despawn immediately; new
stalkers have aura 106875 removed.

This state is mechanically isolatable from elevator and Playerbots work. It
does not add cannon SQL. Its comments assert 18 bombarders, five per shot, a
travelling/impact visual in 133711, and a 20-unit/second speed; those comments
are not independent evidence.

## `OnSpellClick` framework contract

These are **FACTS** from the current core:

- `CMSG_SPELLCLICK` resolves an in-world creature and calls
  `Unit::HandleSpellClick(player)`.
- `HandleSpellClick` looks up every `npc_spellclick_spells` record for the
  creature entry (or vehicle creature entry), checks relationship and database
  conditions, and casts every qualifying default click spell. `result` becomes
  true if at least one record qualifies.
- Only after processing all default spells does it invoke
  `creature->AI()->OnSpellClick(clicker, result)`. The AI callback therefore
  coexists with, and does not suppress, the default click spell.
- A usable client interaction normally requires
  `UNIT_NPC_FLAG_SPELLCLICK`; that flag tells the client to send the opcode.
  Loading removes the flag from a creature template that has no corresponding
  spell-click row. A row without a template flag is permitted for special
  creature/vehicle uses, but it does not by itself give an ordinary player the
  clickable UI.
- Consequently the fork callback requires at least one qualifying loaded
  `npc_spellclick_spells` row to receive `result == true`. One row is expected
  to cast 116554 under the current behavior, but the final callback would run
  after that cast and could coexist with `SpellHit(116554)`. Replacing
  `SpellHit` removes that source-level duplicate path; multiple qualifying rows
  still cause multiple default casts but only one AI callback per
  `HandleSpellClick` call.

Checked-in base/current update data do not establish the cannon entry's
template flag, ScriptName, `npc_spellclick_spells` row, conditions, cast flags,
or spell ID. Historical bulk data are not a reliable declaration of the
deployed database. Therefore whether the target server has the required row
and flag, and whether its default spell has side effects beyond calling the
old `SpellHit`, remains **UNKNOWN**. A single client action normally invokes
one handler and one AI callback (**FACT**); duplicate network actions or other
server-side calls were not established (**UNKNOWN**).

## Spell 133711 and 133710 evidence

Repository evidence found in two duplicate historical 5.4.8 SmartAI snapshots
names spell 133711 `Flak Fire` and spell 133710 `Flak Fire Impact`. The rows
belong to source entry 67885: one casts 133710 on spawn and another casts
133711 on evade. They do not identify Gate cannon/bombarder entries, projectile
speed, targeting, visuals, or an impact-chain relationship. Current live C++,
base/current SQL, spell-script bindings, and checked-in extracted spell data
contain no independent 133711/133710 metadata.

Public preservation checks:

- `ProjectSkyfire/SkyFire_548`, revision
  `aa8c59bab6bba1c0b9f1b15622da924576a4d8e8` (2026-09-17): no independent
  non-SQL occurrence was found. Its close lineage means it would in any event
  not be independent proof of retail behavior.
- `PandariaCore/PandariaCore`, revision
  `a959920207caf0b1d3205640e1efa9377e1ba2c7` (2014-11-07): no occurrence of
  133711, 133710, 116554, or this cannon script was found at the inspected
  revision. Absence is not contrary spell metadata.
- Web search through the provided browsing service was attempted but returned
  HTTP 401. GitHub repository discovery remained available, but unauthenticated
  GitHub code search did not expose further results. No modern spell page was
  accepted as build-18414 evidence.

`SPELL_133711_EVIDENCE = PARTIAL`. The IDs and names have MoP-era repository
corroboration, but Gate applicability, caster/target validity, travelling and
impact visuals, and build-18414 spell records are unproved. Spell 133710 has
the same naming-level evidence only; no evidence establishes that casting
133711 on a Gate bombarder automatically produces 133710 or that C++ should
cast 133710.

## Target pool and five-target analysis

The final pool is one instance-wide `std::list<ObjectGuid>` populated on
bombarder creation. Both cannons share it. Selection is random. Removal uses
`list::remove` before creature resolution, so a selected GUID cannot be chosen
again and duplicate list entries for that same GUID are all consumed. An
invalid or already despawned GUID is still consumed safely. With fewer than
five entries, selection continues until empty; with none, it immediately
stops. On normal creature removal its GUID is removed. No consumed set is
saved; reload rebuilds from loaded spawns, while completed brazier state clears
or rejects them.

These invariants support “up to five distinct GUIDs per click” internally.
They do not prove that five targets, random targeting, 18 total targets, four
clicks, or consumption shared between both cannons matches retail. Current
preservation already contains a five-iteration loop, which is limited legacy
support for the count, but its with-replacement semantics can hit fewer than
five distinct targets and no independent source was found.

`FIVE_TARGET_COUNT_EVIDENCE = PARTIAL`: five is pre-existing preserved
behavior, but distinct/random/without-replacement and the asserted 18-target
lifecycle remain fork design choices without build-18414 or contemporaneous
gameplay corroboration.

## Projectile, scheduler, and visual analysis

`67a6a17` attempted visible feedback with raw visual 29216 at 40 units/second,
then delayed C++ fall/despawn. Candidate 18 instead has the cannon cast spell
133711 on the bombarder with `triggered=true`, removes the raw visual, and
changes only the speed assumption to 20. The player is neither caster nor
target. C++ still causes impact gameplay by fall/despawn; an impact visual is
merely asserted to come from spell data. Thus this is replacement of one
unverified feedback mechanism by another, not demonstrated duplicate removal.

The delay uses three-dimensional `me->GetDistance(bombarder)`, truncates the
floating result to `uint32`, and clamps it to 500–2500 ms. The task is queued
on `bombarder->m_Events`; it captures the raw `Creature*`, but the event
processor is owned and updated by that creature, so it does not independently
outlive its owner under the core event model (**INFERENCE**). If removal/unload
destroys the creature, its owned event queue is destroyed/cancelled; if it
remains, the callback calls `MoveFall()` then schedules despawn 2000 ms later.
The GUID has already left the instance pool, so pool cleanup may occur before
visual impact. No code proves that the target remains in world until expiry.

Comparable repository scripts use creature-owned `m_Events.Schedule` and
distance-derived timings, supporting API/lifetime plausibility, not these
constants. No independent evidence was found for 20 units/second, visual
29216, the 500/2500 clamps, or correspondence between spell 133711 missile
travel and this timer.

`TIMING_EVIDENCE = INSUFFICIENT`. The timing changes when C++ executes the
target fall/despawn, so it is not safely characterized as feedback-only.

## Synthesized diff review

A prospective current-to-final diff was reconstructed but deliberately not
applied. It would:

1. add `SPELL_FLAK_FIRE = 133711`, replace `SpellHit` with successful
   `OnSpellClick`, consume up to five pool GUIDs before lookup, trigger that
   spell, schedule the clamped delayed fall/despawn, and conditionally remove
   the click flag in `boss_striker_gadok.cpp`;
2. replace the unused data slot with `DATA_BOMBARDER_DEFEATED = 9` in
   `gate_of_the_setting_sun.h`;
3. add aura 106875 cleanup, safe empty selection, removal/clear helpers,
   creation/removal hooks, and completed-brazier load cleanup in
   `instance_gate_of_the_setting_sun.cpp`.

No Raigonn, elevator, defender/corpse, path, Playerbots, SQL, or death-emote
hunk is needed. The source unit is mechanically synthesizable, but applying it
would encode unsupported behavior.

## Decision and validation disposition

**Final classification: `SEMANTICALLY_UNSUPPORTED`.**

The mechanical independence and pool invariants pass review, and
`OnSpellClick` exists in the core. The evidence gate nevertheless fails in
three precise ways:

1. deployed cannon template/spell-click rows and conditions are not present in
   checked-in authoritative data, so successful callback activation and
   default-spell coexistence cannot be established;
2. spell 133711 has only naming-level MoP evidence, not build-18414 Gate
   projectile/impact evidence; and
3. without-replacement targeting plus the 20-unit/second and 500–2500 ms
   gameplay delay remain fork-only semantics. Five iterations predate the fork
   but do not establish those new selection/timing semantics.

Accordingly no source or SQL was modified, no synthesized source diff was
applied, and the conditional `scripts` and `worldserver` builds were not
applicable. This avoids claiming compilation as evidence of gameplay
correctness and preserves Tasks 010 and 016 unchanged.

Before reconsideration, query the deployed database for the cannon creature
entry, `ScriptName`, NPC flags, all `npc_spellclick_spells` rows/cast flags,
and conditions; inspect build-18414 Spell/SpellEffect/missile/visual records
for 116553, 116554, 133710, and 133711; then record client/server runtime traces
for both cannons after Gadok, default spell and callback counts, four-click/18
target behavior, uniqueness, fewer-than-five and empty pools, simultaneous
players, invalid/despawned targets, projectile/impact timing and visuals,
fall/despawn timing, last-target click flags on both cannons, wipe/reset,
instance reload, completed brazier load, grid unload/reload, and stalker aura
106875 cleanup. These are exact evidence requirements, not a request for an
unbounded general test.
