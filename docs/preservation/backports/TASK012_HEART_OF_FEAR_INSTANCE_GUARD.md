# Task 012: Heart of Fear Instance-Guard Review

## Review scope and starting state

- **Starting commit:** `cf630f7d9716fb8954f64e48a25036b5c0f08fbd`
  (`Merge pull request #11 from
  firstuserlastname123/codex/review-and-backport-jasper-chains-null-safety`).
  The working tree was clean on the supplied task branch `work`. This is the
  merge of Task 011 into the supplied `preservation/main` integration history;
  `master` was not checked out.
- **Scoped source:** only the `npc_wind_lord_meljarak_introAI` initialization
  and the Garalon state/action logic in
  `EVENT_MELJARAK_TERRACE_EMPTY`, in
  `src/server/scripts/Pandaria/HeartOfFear/heart_of_fear.cpp`, were reviewed.
  No other Candidate 9 hunk is approved by this review.
- **Decision:** `NEEDS_CONTEXT`; no source change. The API can return null in
  general, but the supported placement of this encounter-specific creature
  outside the scripted Heart of Fear instance was not established. More
  importantly, this AI already unconditionally dereferences `instance` in
  `InitializeAI()` before it can schedule the reviewed event. Candidate 9
  guards only the later event and therefore cannot make a null-instance AI
  survive initialization.

## Candidate 9 provenance

- **Repository:**
  `https://github.com/MityaFoxy/Legends-of-Azeroth-Pandaria-5.4.8.git`.
- **Documented branch:** `master`.
- **Full commit:** `481dd4bc5d82a7caed094ad175f54ed1c3c1d4a3`.
- **Subject:** `Fix multiple null pointer dereference issues and redundant checks`.
- **Author and author date:** google-labs-jules[bot]
  `<161369871+google-labs-jules[bot]@users.noreply.github.com>`,
  2026-09-15 05:51:49 +0000. The committer and commit date are identical;
  the message also credits MityaFoxy as co-author.
- **Inspection:** the exact commit was fetched into temporary ref
  `refs/task012/candidate9-commit`. Its aggregate was inspected with
  `git show --stat`, `git show --summary`, and `git show --format=fuller`; only
  its Heart of Fear diff was then inspected. It was not cherry-picked.

The exact scoped hunk wraps the existing Garalon state block in
`if (instance)`, replaces the conditional expression
`instance ? instance->GetGuidData(DATA_GARALON) : ObjectGuid::Empty` with the
direct call `instance->GetGuidData(DATA_GARALON)`, and otherwise retains the
state write and Garalon AI action:

```diff
-if (instance->GetData(DATA_GARALON) != SPECIAL)
+if (instance)
 {
-    instance->SetData(DATA_GARALON, SPECIAL);
-    if (Creature* Garalon = ObjectAccessor::GetCreature(*me, instance ? instance->GetGuidData(DATA_GARALON) : ObjectGuid::Empty))
-        Garalon->AI()->DoAction(ACTION_GARALON_INITIALIZE);
+    if (instance->GetData(DATA_GARALON) != SPECIAL)
+    {
+        instance->SetData(DATA_GARALON, SPECIAL);
+        if (Creature* Garalon = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_GARALON)))
+            Garalon->AI()->DoAction(ACTION_GARALON_INITIALIZE);
+    }
 }
```

## Instance initialization and lifecycle

### Facts

1. `instance` is an uninitialized `InstanceScript*` data member of
   `npc_wind_lord_meljarak_introAI`. The AI constructor invokes only the
   `ScriptedAI` base constructor; it does not initialize this member.
2. The core invokes `InitializeAI()` after creating the creature AI.
   `InitializeAI()` first assigns `instance = me->GetInstanceScript()`.
3. `WorldObject::GetInstanceScript()` returns an `InstanceMap`'s stored
   `InstanceScript*` only when the object's map is a dungeon or scenario; it
   returns null on other map types. The stored pointer itself begins as null.
4. `InstanceMap::CreateInstanceData()` leaves that pointer null when there is
   no instance template or when `ScriptMgr::CreateInstanceData()` cannot
   create a matching script. The core API therefore does **not** provide a
   universal non-null contract for `Creature::GetInstanceScript()`.
5. The registered Heart of Fear map script covers map 1009 and its factory
   returns a newly allocated `instance_heart_of_fear_InstanceMapScript`. The
   script is registered by `ScriptLoader` and included by the Pandaria script
   build.
6. Heart of Fear's instance script explicitly handles entry 65501 in
   `OnCreatureCreate`; this establishes that the NPC is encounter-specific,
   but it is not a core assertion forbidding the entry or its CreatureScript
   on another map or in a misconfigured instance.
7. In `InitializeAI()`, immediately after the assignment, current code calls
   `instance->GetData(DATA_GARALON)` without a local check. Only after that
   call does it schedule either `EVENT_MELJARAK_TERRACE_EMPTY` or
   `EVENT_MELJARAK_TERRACE_IN_COMBAT`.
8. The reviewed empty-terrace event talks, moves Mel'jarak and the familiars,
   and then evaluates `instance->GetData(DATA_GARALON)`. If the state is not
   `SPECIAL`, it writes `SPECIAL`, conditionally obtains Garalon's GUID, looks
   up Garalon, and calls `ACTION_GARALON_INITIALIZE` when the creature exists.
9. In the current conditional GUID expression, the `instance` test is reached
   only after earlier unconditional calls to `GetData` and `SetData`. It is
   therefore later defensive syntax, not protection for a null pointer on
   entry to the state block.

### Inferences

- In a correctly configured map-1009 Heart of Fear instance, map instance data
  is created from the registered factory before encounter creatures use it, so
  this encounter AI is expected to receive the Heart of Fear `InstanceScript`.
- A null result would indicate an unsupported placement or missing/mismatched
  instance-script configuration. If such a result occurs, the initialization
  dereference fails before the reviewed event can be scheduled. Consequently,
  Candidate 9's later guard is unreachable as a recovery for that condition.
- For a valid `instance`, Candidate 9 preserves evaluation of the Garalon
  state, the `SPECIAL` write, GUID lookup, creature lookup, and AI action. It
  changes no valid-instance encounter behavior, timing, movement, or dialogue.

### Unknowns

- The repository review did not establish a supported product requirement for
  spawning entry 65501 with this CreatureScript outside a correctly scripted
  Heart of Fear instance.
- No crash reproduction, server log, malformed-instance fixture, or
  build-18414 raid runtime result accompanied Candidate 9.
- Static analysis cannot prove the deployed world's instance-template and
  creature/script bindings, nor overall Mel'jarak/Garalon encounter accuracy.

## Explicit review questions

1. **Is `instance` dereferenced before any local validity check? — FACT: yes.**
   The first dereference is in `InitializeAI()`. It occurs before event
   scheduling and before the candidate's proposed event-local check.
2. **Can `instance` legally be null under the core API contract? — FACT: yes,
   in general.** `GetInstanceScript()` is nullable for non-instance maps and
   for an `InstanceMap` with no created instance data. **UNKNOWN:** no supported
   placement/configuration of this particular encounter NPC with a null
   instance was established.
3. **Is the NPC guaranteed to exist only inside a valid Heart of Fear
   `InstanceScript`? — INFERENCE for intended content, not a core guarantee.**
   Its entry is handled by the map-1009 instance script, but no assertion in
   the CreatureScript enforces placement.
4. **Does the core guarantee non-null instance data for this CreatureScript? —
   FACT: no universal guarantee.** The core's creation and accessor paths both
   admit null; correctness depends on map/template/script configuration.
5. **Does later defensive handling imply null was considered possible? —
   FACT:** the GUID lookup uses a conditional `instance` expression. However,
   it follows unconditional dereferences and cannot handle null. Its intent is
   **UNKNOWN** and it does not establish a supported null lifecycle.
6. **What does Candidate 9 change? — FACT:** it adds an outer null guard only
   around the event's state/action block and simplifies the now-dominated GUID
   lookup. It does not guard initialization.
7. **Does it change behavior for a valid instance? — FACT: no.** Nesting and
   lookup syntax change, but state/action semantics remain the same.
8. **Does it depend on another Candidate 9 change? — FACT: no.** The hunk is
   mechanically independent of the other 11 files and unrelated hunks.

## Decision and validation

- **Decision:** `NEEDS_CONTEXT`; the Candidate 9 hunk is not statically
  justified as an isolated fix. Although the accessor is nullable in general,
  no legitimate supported null-instance placement was demonstrated, and this
  exact hunk cannot protect such a path because initialization has already
  dereferenced the same pointer.
- **Source changed:** no.
- **Exact hunk applied:** none; neither Candidate 9 nor any portion of it was
  applied.
- **Dependencies:** no code dependency on another Candidate 9 hunk. A complete
  design for deliberately supporting a null instance would require broader
  initialization and event-lifecycle decisions outside this task's approved
  hunk, so this task stops rather than inventing them.
- **Intended validation if a source change had been justified:** inspect the
  isolated diff, reuse/configure the persistent development tree, compile
  `scripts`, link `worldserver`, re-read the function, and retain build-18414
  runtime encounter testing for the persistent server.
- **Actual scripts build:** not run; no C++ source changed, so the requested
  static review was sufficient and a build would not validate a backport.
- **Actual worldserver build:** not run for the same reason.
- **Post-review validation:** the scoped source remains unchanged; no other
  Candidate 9 hunk entered the task change set.
- **Remaining runtime uncertainty:** persistent-server testing with client
  build 18414 remains necessary to establish overall encounter behavior and
  deployed DB configuration. Compilation, if it had been needed, would prove
  only compile/link compatibility and would not prove encounter correctness.
