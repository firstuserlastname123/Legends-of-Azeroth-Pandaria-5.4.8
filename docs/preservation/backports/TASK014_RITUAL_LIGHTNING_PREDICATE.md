# Task 014: Ritual Lightning Predicate Review

## Scope and starting state

- **Starting commit:** `4c7322bfc6f847d09e4852b81afa1e918f16476a`
  (`Merge pull request #13 from
  firstuserlastname123/codex/review-greenstone-village-player-guard`). The
  working tree was clean on the supplied task branch `work`; this is the merge
  of Task 013 into the supplied `preservation/main` integration history.
- **Scoped function:** only
  `RitualLightningPredicate::operator()(WorldObject* object)` in
  `src/server/scripts/Pandaria/ThroneOfThunder/throne_of_thunder.cpp` was
  reviewed. No other Candidate 9 hunk is approved by this review.
- **Decision:** `STATICALLY_JUSTIFIED`. The current conditional expression is
  parsed so that a failed object/player guard selects a distance branch; it
  therefore passes a null `Player*` to a distance overload that dereferences
  it. The spell target hook receives a general unit-area target list, and the
  core search mask admits hostile creatures as well as players. Candidate 9
  removes invalid/non-player entries while preserving both player distance
  tests.

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
  `refs/task014/candidate9-commit`. Its aggregate was inspected with
  `git show --stat`, `git show --summary`, and `git show --format=fuller`;
  only its Throne of Thunder file diff was then inspected. It was not
  cherry-picked.

## Exact expressions and C++ parse

The current expression is:

```cpp
return object && object->ToPlayer() && _spell_id == SPELL_LONG_RANGE_RITUAL_LIGHTNING ? _caster->GetExactDist2d(object->ToPlayer()) < 30.0f : _caster->GetExactDist2d(object->ToPlayer()) > 30.0f;
```

Because equality `==` has higher precedence than logical AND `&&`, and `&&`
has higher precedence than conditional `?:`, the fully parenthesized parse is:

```cpp
return (((object) && (object->ToPlayer())) &&
        ((_spell_id) == (SPELL_LONG_RANGE_RITUAL_LIGHTNING)))
    ? ((_caster->GetExactDist2d(object->ToPlayer())) < (30.0f))
    : ((_caster->GetExactDist2d(object->ToPlayer())) > (30.0f));
```

It is **not** parsed as a guard followed by a parenthesized conditional. A
false guard selects the `:` expression.

The exact Candidate 9 replacement is:

```cpp
if (!object || !object->ToPlayer())
    return true;
return _spell_id == SPELL_LONG_RANGE_RITUAL_LIGHTNING ? _caster->GetExactDist2d(object->ToPlayer()) < 30.0f : _caster->GetExactDist2d(object->ToPlayer()) > 30.0f;
```

## Evaluation table

The table describes the current expression. Merely testing a pointer value is
listed as evaluating that operand; `GetExactDist2d` evaluates `_caster` and
the selected branch's second `object->ToPlayer()` call.

| Case | Guard evaluation | Selected branch | `object` | `object->ToPlayer()` | `_caster` | `GetExactDist2d(...)` | Result/risk |
|---|---|---|---|---|---|---|---|
| `object == nullptr` | first `object` is false; both later `&&` operands short-circuit | `:` | yes | yes, in `:` argument | yes | yes | Null `Player*` is passed to `Position::GetExactDist2d(Position const*)`, which dereferences it. |
| non-null, non-player object | `object` true; guard conversion returns null; equality short-circuits | `:` | yes | yes in guard and again in `:` argument | yes | yes | Same invalid null-pointer dereference. |
| Player, long-range spell | all guard operands true | `?` | yes | yes in guard and `?` argument | yes | yes | Returns true when distance is below 30.0f. |
| Player, other spell | object/player operands true; equality false | `:` | yes | yes in guard and `:` argument | yes | yes | Returns true when distance is above 30.0f. |

Under Candidate 9, null stops after `!object`; a non-player stops after
`!object->ToPlayer()`; neither case evaluates `_caster` or distance. Both
return true. Valid players still select exactly the original long-range or
other-spell distance expression.

## Predicate caller and container analysis

### Facts

1. `spell_range_ritual_lightning::FilterTargets` receives
   `std::list<WorldObject*>& targets` from an
   `OnObjectAreaTargetSelect` hook registered for effect 0 and
   `TARGET_UNIT_SRC_AREA_ENEMY`.
2. When `GetCaster()` returns non-null, `FilterTargets` calls
   `targets.remove_if(RitualLightningPredicate(caster, GetSpellInfo()->Id))`.
   For `std::list::remove_if`, predicate true erases the entry and false
   retains it. Invalid/null/non-player entries are not valid Ritual Lightning
   player targets, so the correct predicate result for them is true.
3. `TARGET_UNIT_SRC_AREA_ENEMY` is a unit, source-referenced, area target with
   an enemy check. `Spell::SelectImplicitAreaTargets` builds a local
   `std::list<WorldObject*>`, calls `SearchAreaTargets`, and passes that list
   to script area-target hooks before converting retained entries to final
   targets.
4. For a unit object type, `Spell::GetSearcherTypeMask` includes player,
   creature, and (subject to spell restrictions) corpse containers. It narrows
   to players only if the spell has the specific only-player attribute or
   conditions impose a narrower mask. The target kind alone therefore does
   not guarantee Player objects.
5. `WorldObjectListSearcher` appends `itr->GetSource()` after its target check.
   These grid-reference sources are live object pointers; no supported null
   insertion path into this locally built list was found.
6. The target check accepts a `WorldObject*`, converts it with `ToUnit()`, and
   applies the enemy test to unit targets. Hostile creatures are a supported
   unit-area search category; the hook's static container type remains
   `WorldObject*` rather than `Player*`.
7. The predicate is constructed only inside `if (Unit* caster = GetCaster())`.
   Its `_caster` member receives that exact non-null pointer by value. The
   temporary predicate is used synchronously by `remove_if`, so `_caster` is
   non-null and remains alive for the call.
8. `Position::GetExactDist2d(Position const*)` immediately delegates through
   `GetExactDist2d(*pos)`. Passing the null result of `ToPlayer()` therefore
   dereferences a null pointer; it is not a null-tolerant distance API.

### Inferences

- Null list entries are not legal products of the observed core search path.
  Candidate 9's null-object arm is defensive but has the same correct filter
  semantic as its necessary non-player arm.
- A non-player enemy Unit is within the supported search/container domain
  unless data attributes or conditions happen to narrow these two spells at
  runtime. The predicate itself is required to reject such entries safely;
  it cannot rely on the general target enum to establish `Player`.
- Candidate 9 is semantically correct for `remove_if`: entries that cannot be
  player Ritual Lightning targets are removed rather than retained for the
  later final-target conversion.

### Unknowns

- The extracted build-18414 spell data and deployed condition rows were not
  inspected here, so whether spells 137994/137995 happen to exclude creatures
  before the script hook in a particular runtime remains unknown. That does
  not repair the malformed predicate or change the hook/container contract.
- No live Throne of Thunder cast, encounter reproduction, or retail-reference
  comparison accompanied Candidate 9. Runtime encounter correctness remains
  unproved.

## Explicit findings and dependency review

1. **Can `object` legally be null? — FACT: no supported producer was found.**
   The core list searcher appends live grid-source pointers. If an external
   hook violated that invariant, current code would nevertheless dereference
   null through the false conditional branch, while Candidate 9 would remove
   it safely.
2. **Can `object` legally be a non-player? — FACT: yes at the hook/container
   contract level.** Unit-area search includes creatures as well as players;
   only runtime spell attributes/conditions can narrow it further.
3. **Is `_caster` guaranteed non-null? — FACT: yes for every predicate
   construction.** The caller constructs it only within a successful
   `if (Unit* caster = GetCaster())` and uses it synchronously.
4. **What does true mean? — FACT:** `std::list::remove_if` erases that object
   from the target list.
5. **Should invalid/null/non-player objects be removed? — INFERENCE: yes.**
   This script distinguishes Ritual Lightning ranges for player targets;
   retaining an invalid entry cannot produce a valid player target and would
   contradict the explicit `ToPlayer()` requirement already present.
6. **Does Candidate 9 implement that meaning? — FACT: yes.** It returns true
   before distance evaluation for invalid/non-player entries.
7. **Valid-player behavior — FACT:** Candidate 9 retains `< 30.0f` removal for
   the long-range spell and `> 30.0f` removal for the other spell, including
   the exact boundary behavior. Spell IDs, caster, and distance are unchanged.
8. **Dependencies — FACT:** the hunk is mechanically and semantically
   independent of the other Candidate 9 hunks. No other candidate change is
   required.

## Decision and validation plan

- **Classification:** `STATICALLY_JUSTIFIED`.
- **Reason:** the current parse is logically malformed as a guarded predicate
  and reaches a proven null dereference for the supported non-player unit
  category. Candidate 9 removes invalid entries and leaves both valid-player
  distance branches unchanged.
- **Checkpoint source status:** unchanged; this review is committed before the
  isolated source hunk is applied.
- **Planned exactness:** apply only Candidate 9's exact
  `RitualLightningPredicate` hunk, then incrementally compile `scripts` and
  link `worldserver` in the persistent fast-development build tree.
- **Runtime limitation:** compilation can establish source/link compatibility,
  not retail encounter behavior for client build 18414.

## Final implementation and validation record

- **Final classification:** `STATICALLY_JUSTIFIED`.
- **Source changed:** yes. Only
  `RitualLightningPredicate::operator()(WorldObject*)` was changed.
- **Applied hunk:** exact Candidate 9 hunk, with no adaptation:

  ```diff
  -return object && object->ToPlayer() && _spell_id == SPELL_LONG_RANGE_RITUAL_LIGHTNING ? _caster->GetExactDist2d(object->ToPlayer()) < 30.0f : _caster->GetExactDist2d(object->ToPlayer()) > 30.0f;
  +if (!object || !object->ToPlayer())
  +    return true;
  +return _spell_id == SPELL_LONG_RANGE_RITUAL_LIGHTNING ? _caster->GetExactDist2d(object->ToPlayer()) < 30.0f : _caster->GetExactDist2d(object->ToPlayer()) > 30.0f;
  ```

- **Scripts build:** passed with
  `cmake --build /tmp/mop-preservation-dev --target scripts --parallel 4`.
  The persistent build directory did not exist, so it was first configured
  with the documented Playerbots-, modules-, tools-, and Eluna-disabled fast
  development profile. The modified Throne of Thunder translation unit and
  final `scripts` static library both compiled successfully.
- **Worldserver build:** passed with
  `cmake --build /tmp/mop-preservation-dev --target worldserver --parallel 4`;
  the final executable linked successfully.
- **Post-build static validation:** null stops at `!object`; non-player stops
  at `!object->ToPlayer()`; both return true and are removed without evaluating
  `_caster` or distance. A valid player with the long-range spell still
  returns true only below 30.0f, while a valid player with the other spell
  still returns true only above 30.0f. No other Candidate 9 code entered the
  source diff.
- **Remaining runtime uncertainty:** no persistent-server Throne of Thunder
  encounter cast was run with a build-18414 client, extracted data, or the
  deployed databases. Compilation proves compile/link compatibility, not
  retail spell selection or encounter correctness; that runtime validation
  remains outstanding.
