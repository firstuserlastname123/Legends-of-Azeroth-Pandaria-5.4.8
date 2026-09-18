# Task 011: Jasper Chains Null-Guard Review

## Review scope and starting state

- **Starting commit:** `be2d5852b60ff58517811798f7437d638233cf44`
  (`Merge pull request #10 from
  firstuserlastname123/codex/review-and-backport-raigonn-state-guard`). The
  working tree was clean on the supplied task branch `work`. This is the merge
  of Task 010 into the current supplied `preservation/main` integration
  history; `master` was not checked out.
- **Scoped source:** only `spell_jasper_chains::spell_jasper_chains_AuraScript::HandlePeriodic`
  in
  `src/server/scripts/Pandaria/MogushanVault/boss_stone_guard.cpp` was reviewed.
  No other Candidate 9 function or file is approved by this review.
- **Decision:** no source change. The current expression dereferences the local
  `target` pointer before reaching its local null test, but this callback is a
  supported periodic aura hook for which the core API contract says
  `AuraScript::GetTarget()` is always valid. Candidate 9 therefore moves a
  redundant check earlier; it does not repair a statically justified null path
  in this callback.

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
- **Inspection:** the exact commit was fetched into the temporary ref
  `refs/task011/candidate9-commit`. Its 12-file aggregate was inspected with
  `git show --stat`, `git show --summary`, and `git show --format=fuller`, and
  its Stone Guard diff was inspected separately. It was not cherry-picked.

Candidate 9 contains two Stone Guard hunks. One removes the
`if (instance)` guard before tile-aura cleanup; that unrelated guard removal is
explicitly excluded. The Jasper Chains hunk is:

```diff
 Unit* caster = GetCaster();
 Unit* target = GetTarget();
 const SpellInfo* spell = GetSpellInfo();
+
+if (!caster || !target || !spell)
+{
+    Remove();
+    return;
+}
+
 Player* linkedPlayer = ObjectAccessor::GetPlayer(*target, playerLinkedGuid);

-if (!caster || !target || !spell || !linkedPlayer || !linkedPlayer->IsAlive() || !linkedPlayer->HasAura(spell->Id))
+if (!linkedPlayer || !linkedPlayer->IsAlive() || !linkedPlayer->HasAura(spell->Id))
```

## Current and candidate code

The current callback obtains `caster`, `target`, and `spell`, initializes
`linkedPlayer` with `ObjectAccessor::GetPlayer(*target, playerLinkedGuid)`, and
then tests `caster`, `target`, `spell`, and the linked-player conditions. On any
failed condition it invokes `Remove()` and returns. Its remaining logic checks
the distance to the linked player, updates or removes the Jasper Chains aura,
and casts the existing damage or visual spell.

The candidate adds an initial combined `caster`/`target`/`spell` test before
the lookup, calls `Remove()` and returns if that test fails, and leaves the
linked-player alive/aura validation immediately after the lookup. It does not
otherwise change the linked-player, distance, stack, aura-removal, damage, or
visual logic.

## Evaluation-order and API/lifecycle analysis

### Exact evaluation order

1. `GetCaster()` is called and its result is stored in `caster`.
2. `GetTarget()` is called and its result is stored in `target`.
3. `GetSpellInfo()` is called and its result is stored in `spell`.
4. To initialize `linkedPlayer`, the expression
   `ObjectAccessor::GetPlayer(*target, playerLinkedGuid)` evaluates the
   arguments required for the call. Evaluating `*target` dereferences
   `target`; this happens before the following `if` statement is evaluated.
5. The `if` operands are evaluated left-to-right with short-circuit `||`:
   `!caster`, `!target`, `!spell`, `!linkedPlayer`, and, only as needed, the
   linked player's alive and aura state. Thus the local `!target` check occurs
   after the dereference expression.

### Findings

1. **Can `target` be null according to the API contract or call path? — FACT:
   no for this registered hook.** `HandlePeriodic` is registered through
   `OnEffectPeriodic` and therefore runs as
   `AURA_SCRIPT_HOOK_EFFECT_PERIODIC`. The `AuraScript::GetTarget()` interface
   documents that its return does not need a null check: it is always valid in
   supported hooks and returns null only in an unsupported hook. The
   implementation lists `AURA_SCRIPT_HOOK_EFFECT_PERIODIC` among the supported
   states and returns `m_auraApplication->GetTarget()`. The periodic dispatcher
   prepares that hook with its `AuraApplication const* aurApp` before invoking
   the handler. The callback is therefore within the API's stated non-null
   contract.
2. **Is `*target` evaluated before the existing local null check? — FACT:
   yes.** It is evaluated as part of the `ObjectAccessor::GetPlayer` call before
   execution reaches the subsequent `if`.
3. **Does current code dereference `target` before proving it non-null? — FACT
   and distinction:** it dereferences before its *local* `!target` test, but it
   does not dereference before the API/lifecycle proof applies. Entry through
   the supported periodic hook already supplies the non-null guarantee. There
   is consequently no statically justified null-dereference path for `target`
   in this call path.
4. **Does Candidate 9 merely move the guard or alter encounter behavior? —
   FACT:** for contract-conforming periodic calls it merely moves the existing
   `caster`/`target`/`spell` guard ahead of the lookup and splits the condition;
   observable encounter behavior is unchanged. **INFERENCE:** if the callback
   could somehow be invoked in violation of the core hook contract, the early
   guard would avoid this dereference, but no such supported path was found.
5. **Is linked-player logic preserved? — FACT:** yes. The candidate retains the
   same lookup and the same null, alive, and aura checks, followed by the same
   distance, stack, removal, damage, and visual behavior.
6. **Does the hunk depend on another Candidate 9 change? — FACT:** no source or
   API dependency is visible. It only reorders checks already in the function.
   The other Stone Guard hunk and all other Candidate 9 files are independent
   and remain excluded.

### Uncertainty

- **UNKNOWN:** no runtime reproduction or build-18414 raid test was supplied
  for the alleged crash.
- **UNKNOWN:** this static review does not prove broader Jasper Chains gameplay
  correctness, retail accuracy, pairing lifecycle, distance thresholds, stack
  behavior, or reset/death behavior.
- **INFERENCE:** an invalid `m_auraApplication` would fail inside
  `AuraScript::GetTarget()` before an early local `!target` guard could help, so
  Candidate 9 is not a defense against violation of that separate lifecycle
  invariant.

## Dependency and validation result

- **Dependencies:** none for the isolated candidate rearrangement, but the
  rearrangement is not needed under the current `AuraScript` contract.
- **Exact hunk applied:** none. Candidate 9 was not cherry-picked or applied.
- **Intended validation if justified:** inspect the isolated diff, reuse the
  persistent fast development tree, build `scripts`, then link `worldserver`,
  followed by a source re-read and build-18414 runtime encounter testing.
- **Actual build result:** not run. The task directs the review to stop after a
  documentation commit when no statically justified null-dereference risk is
  exposed. With no C++ change, compilation would not validate a backport.
- **Post-review validation:** the final source remains unchanged. The core API
  contract and periodic call path were re-read, and no supported path returning
  a null target was found.
- **Runtime uncertainty:** persistent-server testing with a build-18414 client
  remains necessary for overall Jasper Chains encounter behavior, but there is
  no source modification from this task to runtime-validate. Compilation, had
  it been run, could prove only source/API/link compatibility and not gameplay
  correctness.

