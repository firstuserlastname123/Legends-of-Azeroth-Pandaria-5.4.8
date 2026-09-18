# Task 009: ARC4 OpenSSL 3 Provider Handling

## Backport plan

- **Starting commit:** `2f67970f25d178184a4aebaf4b9fe29767fb284a`
  (`Merge pull request #8 from firstuserlastname123/codex/perform-detailed-review-of-fork-candidates`).
- **Source repository and branch:**
  `https://github.com/bondarenkoandriy/Legends-of-Azeroth-Pandaria-5.4.8.git`,
  `master`.
- **Source candidate:** `72e7fb1d492c45398630e9ea52cb695737d7865d`,
  `Update ARC4.cpp`, authored by Andrey B
  `<111489336+bondarenkoandriy@users.noreply.github.com>` on
  2026-09-11 16:34:50 +0300.
- **Affected source file:** `src/common/Cryptography/ARC4.cpp`.
- **Intended behavior:** on OpenSSL 3, load the default and legacy providers,
  retain their provider handles for process lifetime, and fetch RC4 explicitly
  from the legacy provider. Assert context, provider, and cipher acquisition;
  keep the pre-OpenSSL-3 `EVP_rc4()` path; and explicitly narrow `size_t`
  arguments at the OpenSSL APIs that accept `int`.
- **Dependency status:** Candidate 8's prerequisite/equivalent removal of
  obsolete OpenSSL initialization is already in the current tree. No other
  candidate dependency or new external library is required.
- **Compatibility findings:** the current `ARC4` class retains the structure
  expected by the candidate, including its OpenSSL 3 owned `EVP_CIPHER` member.
  The current global OpenSSL setup also loads the default and legacy providers;
  Candidate 14 takes additional provider references local to ARC4 initialization
  so they remain valid for the static handles' lifetime. Ubuntu's installed
  OpenSSL 3.0.13 exposes `OSSL_PROVIDER_load`, `EVP_CIPHER_fetch`, and the
  provider header used by the patch. The candidate's parent-to-commit diff is
  restricted to `ARC4.cpp` and introduces no configuration, credentials, SQL,
  Playerbots, Eluna, protocol, or gameplay changes. No post-Task-008 tree change
  supersedes it.
- **Intended validation:** review the applied diff, reuse or configure the fast
  development tree, build `authserver` and `worldserver`, run their bounded
  `--version` paths if binaries are produced, and run an existing local ARC4
  test only if one is already available.

Runtime authentication-handshake correctness is not yet proven. Successful
compilation and safe version checks cannot establish retail or client-build-18414
authentication correctness; actual handshake validation belongs on the later
persistent test server.

## Final implementation and validation

- **Exact source candidate:**
  `bondarenkoandriy/Legends-of-Azeroth-Pandaria-5.4.8` `master`, commit
  `72e7fb1d492c45398630e9ea52cb695737d7865d` (`Update ARC4.cpp`).
- **Exact patch behavior:** for OpenSSL 3, `ARC4.cpp` loads the `default` and
  `legacy` providers into function-static handles, asserts both handles, fetches
  `RC4` with the `provider=legacy` property, asserts the cipher and context,
  and frees each fetched cipher with the existing destructor path. Before
  OpenSSL 3 it retains `EVP_rc4()`. The patch also makes the existing `size_t`
  to `int` conversions explicit at the EVP key-length and update calls.
- **Final source file changed:** `src/common/Cryptography/ARC4.cpp` only.
  This report is the only other implementation-commit change.
- **Application:** exact parent-to-candidate source patch; no compatibility
  adaptation or additional source behavior was required.
- **Compatibility result:** Candidate 8-equivalent initialization cleanup and
  the expected ARC4 class layout were present. OpenSSL 3.0.13 and its provider
  APIs/header were available. Candidate 14's static provider references coexist
  with the current global provider setup and keep their own references alive;
  cipher ownership remains per ARC4 object. No unrelated configuration,
  credential, SQL, Playerbots, Eluna, protocol, or gameplay change was added.
- **Configure:** `/tmp/mop-preservation-dev` did not exist and was configured
  successfully with the documented fast profile: Playerbots, modules, tools,
  and Eluna disabled; ccache launchers enabled; Release build; GCC 13.3.0; and
  OpenSSL 3.0.13.
- **`authserver` build:** succeeded with
  `cmake --build /tmp/mop-preservation-dev --target authserver --parallel 4`.
- **`worldserver` build:** succeeded with
  `cmake --build /tmp/mop-preservation-dev --target worldserver --parallel 4`.
- **Safe binary checks:** both development binaries completed `--version`
  within the 30-second bound with status 0 and reported
  `Pandria 5.4.8 rev. Archive (branch) (Unix, Release, Static)`.
- **Existing crypto test:** repository searches found no existing ARC4/RC4
  test mechanism, so no local crypto test was available to run without
  inventing a new framework or protocol behavior.
- **ccache after builds:** 1,348 compiler calls; 89 cacheable calls, all misses;
  1,259 uncacheable calls; approximately 0.05 GiB stored.
- **First meaningful error:** none. Configuration, compilation, linking, and
  bounded version paths all succeeded.
- **Deferred validation:** legacy-provider-unavailable behavior, known-answer
  crypto regression coverage, and an actual client authentication handshake
  remain for the persistent test environment and supported OpenSSL matrix.

Successful compilation does not establish retail/build-18414 authentication
correctness. Actual handshake/runtime validation belongs on the later
persistent test server.
