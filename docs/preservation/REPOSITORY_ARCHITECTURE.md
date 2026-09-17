# Repository Architecture Map

## Scope and starting point

This document maps the repository as observed for Task 006. It is a navigation
guide for preservation work on Mists of Pandaria 5.4.8 client build 18414, not
a completeness or defect audit. A path's presence does not establish that its
behavior is correct, complete, or retail-accurate.

Task 006 started from branch `work` at commit
`02d29f1037104efe7bf06abcb6d224c7e722ae50`. The working tree was clean. That
commit is the merge of Task 005 into the current `preservation/main` history;
`master` was not checked out.

## Path-oriented quick reference

| If the work concerns... | Start here |
| --- | --- |
| Shared platform utilities | `src/common/` |
| Generic networking, packets, data stores, threads | `src/server/shared/` |
| MySQL access and automatic SQL updater | `src/server/database/` |
| Authentication service | `src/server/authserver/` |
| World service process and startup | `src/server/worldserver/` |
| Gameplay systems | `src/server/game/` |
| Client sessions and packet dispatch | `src/server/game/Server/`, `src/server/game/Handlers/` |
| Opcodes and packet serialization | `src/server/game/Server/Protocol/`, `src/server/game/Server/Packets/` |
| Maps, instances, grids, path generation | `src/server/game/Maps/`, `Instances/`, `Grids/`, `Movement/` |
| Players, creatures, objects, pets, vehicles | `src/server/game/Entities/` |
| Spells and auras | `src/server/game/Spells/` |
| Quests | `src/server/game/Quests/`, `src/server/game/Handlers/QuestHandler.cpp`, content scripts and world SQL |
| C++ content scripts | `src/server/scripts/` |
| Pandaria zones, dungeons, raids, scenarios | `src/server/scripts/Pandaria/` |
| Script interfaces and lifecycle | `src/server/game/Scripting/` |
| Playerbots | `modules/mod_playerbots/` |
| Eluna engine and live Lua | `src/server/game/LuaEngine/`, `dep/lualib/`, `contrib/lua_scripts/` |
| Other repository modules | `modules/` |
| Auth/character base schemas | `sql/base/` |
| Current SQL migrations | `sql/updates/` |
| Historical SQL | `sql/archive/`, `sql/old/` |
| Server configuration templates | `src/server/authserver/authserver.conf.dist`, `src/server/worldserver/worldserver.conf.dist` |
| Extractors and navigation tools | `src/tools/` |
| Build options and platform logic | `CMakeLists.txt`, `cmake/`, component `CMakeLists.txt` files |
| CI | `.github/workflows/` |
| Preservation records | `docs/preservation/` |
| General project documentation | `doc/`, root Markdown files |
| Vendored dependencies | `dep/` |

## Top-level repository map

- `.github/` contains GitHub issue metadata and workflows. The relevant build
  definitions are `.github/workflows/linux_gcc.yml` and
  `.github/workflows/windows-build-release.yml`.
- `cmake/` contains project options (`options.cmake`), option reporting,
  revision generation, compiler/platform settings, dependency finders, and
  build-directory checks. Root `CMakeLists.txt` selects C++20, adds `dep/` and
  `src/`, and conditionally adds `modules/`.
- `contrib/` contains operational and developer aids rather than core server
  targets: build scripts, configuration merge/cleanup helpers, debugger and
  Valgrind material, conversion/patching tools, and runtime Eluna scripts in
  `contrib/lua_scripts/`.
- `dep/` is the vendored dependency tree. It includes Boost, fmt, g3dlite,
  gSOAP, jemalloc, Lua, MySQL client material, OpenSSL, Recast/Detour,
  StormLib, SFMT, utf8cpp, zlib, bzip2, readline, and threading support.
- `doc/` contains project documentation such as `code_standards.md`; `docs/`
  contains the preservation workflow and baseline records.
- `modules/` is the statically compiled module tree. It currently contains
  `mod_exemple/` and the substantial `mod_playerbots/`, plus the generated
  module registration loader and its template.
- `sql/` contains database schemas, migrations, and historical SQL. Its actual
  current shape is detailed below; notably, this checkout has auth and
  characters base files but no `sql/base/world.sql`.
- `src/common/` builds the foundational `common` library (configuration,
  cryptography, logging, platform definitions, utilities, and process-level
  support).
- `src/server/` contains the database, shared, gameplay, scripts, authserver,
  and worldserver components.
- `src/tools/` contains `map_extractor`, `vmap4_extractor`,
  `vmap4_assembler`, and `mmaps_generator`; these are enabled by `TOOLS`.
- `src/genrev/` generates revision metadata used by binaries and libraries.
- `src/updater/` is an optional standalone updater target controlled by
  `UPDATER`; it is distinct from the database updater embedded in the
  `database` library.
- Root configuration and metadata (`CMakeLists.txt`, `README.md`, `THANKS`,
  `LICENSE`, and related files) describe and drive the whole project.

## Core source architecture

The primary core source path is `src/server/`. Most gameplay behavior is in
`src/server/game/`, while `src/server/shared/` and `src/common/` provide lower
layers. All paths below compile into `game` unless another target is named.
SQL associations indicate where data-driven behavior is likely to be found;
they do not imply a one-to-one file mapping.

| Subsystem | Primary path | Representative files/classes | Build/data relationship |
| --- | --- | --- | --- |
| Foundation | `src/common/` | `Common.h`, configuration, logging, cryptography, utilities | `common`; used broadly |
| Generic networking | `src/server/shared/Networking/` | `Socket`, `SocketMgr`, `NetworkThread`, `AsyncAcceptor` | `shared`; consumed by both servers |
| Shared packets/data | `src/server/shared/Packets/`, `DataStores/`, `Realms/` | byte buffers, DBC structures, realm definitions | `shared` |
| Authentication | `src/server/authserver/Authentication/`, `Server/` | `AuthSession`, `AuthSocketMgr`, `AuthCodes`, `TOTP` | `authserver`; auth DB |
| World connections | `src/server/game/Server/` | `WorldSocket`, `WorldSocketMgr`, `WorldSession`, `WorldPacket`, `GameClient` | `game`, then `worldserver` |
| Protocol/opcodes | `src/server/game/Server/Protocol/` | `Opcodes.h/.cpp`, `ServerPktHeader`, `PacketLog` | `game`; build-18414-sensitive |
| Typed packets | `src/server/game/Server/Packets/` | `AuthenticationPackets`, `CharacterPackets`, `MovementPackets`, `QuestPackets`, `SpellPackets` | `game` |
| Request handlers | `src/server/game/Handlers/` | `CharacterHandler.cpp`, `QuestHandler.cpp`, `SpellHandler.cpp`, `MovementHandler.cpp` and other feature handlers | `game`; routes session opcodes into systems |
| World lifecycle | `src/server/game/World/` | `World.h/.cpp` | `game`; world/auth/character DB use |
| Global data managers | `src/server/game/Globals/`, `DataStores/` | `ObjectMgr`, client-data loaders and global stores | `game`; mainly world DB plus client data |
| Maps and phases | `src/server/game/Maps/` | `Map`, `MapInstanced`, `MapManager`, `MapUpdater`, `PhaseMgr`, `TransportMgr` | `game`; world DB and extracted map data |
| Spatial grids | `src/server/game/Grids/` | cells and grid notifiers | `game` |
| Instances | `src/server/game/Instances/` | `InstanceScript`, `InstanceSaveMgr` | `game`; character/world data and content scripts |
| Entity base/unit | `src/server/game/Entities/Object/`, `Unit/` | `Object`, update-field machinery, `Unit` | `game` |
| Players | `src/server/game/Entities/Player/` | `Player`, `PlayerTaxi`, `SocialMgr`, `TradeData`, `SceneMgr` | `game`; character and world DB |
| Creatures | `src/server/game/Entities/Creature/` | `Creature`, `CreatureData`, `CreatureGroups`, `TemporarySummon` | `game`; world DB and scripts |
| Game objects/items | `src/server/game/Entities/GameObject/`, `Item/` | `GameObject`, `GameObjectData`, `Item`, containers | `game`; world/character DB |
| Pets/vehicles/transports | `src/server/game/Entities/Pet/`, `Vehicle/`, `Transport/` | `Pet`, `Vehicle`, transport types | `game`; world/character DB and scripts |
| AI | `src/server/game/AI/` | `CreatureAI`, `PlayerAI`, scripted AI, SmartScripts | `game`; SmartAI data is in world DB |
| Spells | `src/server/game/Spells/` | `Spell`, `SpellInfo`, `SpellMgr`, `SpellEffects`, `SpellScript`, `SpellHistory` | `game`; DBC/client data, world DB, C++ scripts |
| Auras | `src/server/game/Spells/Auras/` | `Aura`, `AuraEffect`, `SpellAuras`, `SpellAuraEffects` | `game`; spell data and scripts |
| Combat/threat | `src/server/game/Combat/` | `CombatManager`, `ThreatManager`; also `Unit` combat methods | `game` |
| Movement/pathfinding | `src/server/game/Movement/` | `MotionMaster`, `MovementGenerator`, `PathGenerator`, splines, waypoints | `game`; Detour/Recast, mmaps and world waypoint data |
| Quests | `src/server/game/Quests/` | `Quest`, `QuestStatusData` in `QuestDef`; related quest handlers | `game`; world/character DB and content scripts |
| Groups | `src/server/game/Groups/` | `Group`, `GroupMgr` | `game`; character DB |
| Guilds | `src/server/game/Guilds/` | `Guild`, `GuildMgr`, `GuildFinderMgr` | `game`; character DB |
| Battlegrounds/arenas | `src/server/game/Battlegrounds/`, `Battlefield/` | `Battleground`, `BattlegroundMgr`, `BattlegroundQueue`, `ArenaTeam`, battlefield zones | `game`; character/world DB and scripts |
| LFG | `src/server/game/DungeonFinding/` | `LFGMgr`, `LFGQueue`, `LFGGroupData`, `LFGPlayerData`, `LFGScripts` | `game`; character/world data |
| Scenarios | `src/server/game/Scenarios/` | scenario management | `game`; Pandaria scenario scripts/world data |
| Achievements | `src/server/game/Achievements/` | `AchievementMgr` | `game`; character/world/client data and scripts |
| Loot | `src/server/game/Loot/` | `LootMgr` | `game`; world DB loot templates |
| Mail | `src/server/game/Mails/` | `Mail` | `game`; character DB |
| Auction house | `src/server/game/AuctionHouse/`, `AuctionHouseBot/` | `AuctionHouseMgr`; auction bot logic | `game`; character/world DB and config |
| Chat/commands | `src/server/game/Chat/` | `ChatHandler`, channels, core command support | `game`; command scripts under `src/server/scripts/Commands/` |
| Skills/professions | `src/server/game/Skills/` | `SkillDiscovery`, `SkillExtraItems` | `game`; client/world spell and skill data |
| Script API | `src/server/game/Scripting/` | `ScriptMgr`, `ScriptSystem`, `MapScripts`, script base classes/macros | `game`; registrations supplied by `scripts` and modules |
| Conditions/events/pools | `src/server/game/Conditions/`, `Events/`, `Pools/` | condition, game-event, and spawn-pool managers | `game`; predominantly world DB |
| Additional systems | `Accounts/`, `BattlePet/`, `Calendar/`, `Reputation/`, `Services/`, `Tickets/`, `Weather/`, `Warden/`, `Vignette/` | feature-specific managers and handlers | `game`; applicable auth/character/world/client data |

The database abstraction resides in `src/server/database/`. `Database/`
contains connections, worker pools, prepared statements, transactions, and
implementations for Login, Character, World, and Playerbots databases;
`Logging/` provides database logging; `Updater/` contains the embedded update
engine. These compile into the `database` static library.

## C++ script architecture

### Organization

`src/server/scripts/` builds the static `scripts` library. Its top-level
content categories are:

- `EasternKingdoms/` and `Kalimdor/`: Classic locations plus later-expansion
  dungeons and raids located on those continents. Examples include Molten
  Core/Blackwing Lair, Karazhan/Sunwell, Blackwing Descent/Bastion of Twilight,
  Firelands, Dragon Soul, and zone files.
- `Outland/`: Burning Crusade zones, Auchindoun, Hellfire Citadel, Coilfang,
  Tempest Keep, Black Temple, and Gruul's Lair.
- `Northrend/`: Wrath zones and instances such as Naxxramas, Ulduar,
  Icecrown Citadel, Crusaders' Coliseum, Frozen Halls, and Utgarde Keep.
- `Maelstrom/`, plus Cataclysm instances within `EasternKingdoms/`,
  `Kalimdor/`, and `Kalimdor/CavernsOfTime/`: Cataclysm-era content including
  The Stonecore, Throne of the Tides, Grim Batol, Firelands, and Dragon Soul.
- `Pandaria/`: MoP zone files; Timeless Isle and Wandering Isle; scenarios;
  dungeons including Temple of the Jade Serpent, Stormstout Brewery,
  Shado-Pan Monastery, Mogu'shan Palace, Gate of the Setting Sun and Siege of
  Niuzao Temple; raids including Mogu'shan Vaults, Heart of Fear, Terrace of
  Endless Spring, Throne of Thunder and Siege of Orgrimmar.
- `Spells/`: class spell scripts, generic/item/quest/holiday/enchantment spell
  scripts. Encounter-local spell scripts also live beside their encounter.
- `Commands/`, `Battlegrounds/`, `OutdoorPvP/`, `Events/`, `World/`, `Pet/`,
  and `Custom/`: their named cross-content categories. `World/BrawlersGuild/`
  is a notable nested feature. `Examples/` and `Toys/` exist, but the main
  scripts CMake file does not add their sources to `scripts`.

Dungeon and raid code is generally grouped below its geographical/expansion
path rather than under global `Dungeon/` and `Raid/` folders. Typical instance
folders contain an instance script, shared header, encounter/boss sources,
and sometimes an area script.

### Registration and loading

- Content source files expose `AddSC_*` registration functions.
- `src/server/scripts/ScriptLoader/ScriptLoader.cpp` declares those functions
  and calls them from `AddScripts()`; `SCRIPTS` gates the optional content
  blocks while core spell, command, and battleground registrations remain in
  the library.
- `src/server/worldserver/Main.cpp` gives `AddScripts` to `ScriptMgr` during
  startup. `ScriptMgr` and the script base classes are in
  `src/server/game/Scripting/`.
- With `USE_MODULES`, the same loader calls `AddModulesScripts()`. CMake
  generates `modules/ModulesLoader.cpp` from
  `modules/ModulesLoader.cpp.in.cmake`, registering each detected module's
  `AddSC_<module>()` function.
- World database records such as `ScriptName`, SmartAI, spell-script-name,
  event, and instance data connect data rows to registered C++ scripts. Any
  future behavior investigation must inspect both code and the corresponding
  world database state.

## Playerbots

The exact module location is `modules/mod_playerbots/` (there is no top-level
`mod_playerbots/` directory in this checkout).

### Internal map

- `src/mod_playerbots.cpp` is the module entry point. `AddSC_mod_playerbots()`
  registers player, world, server, and Playerbot script hooks plus bot commands.
- `src/AI/` contains `PlayerbotAI`, bot state and AI support.
- `src/strategy/` contains the strategy engine and named-object contexts;
  concrete `actions/`, `triggers/`, and `value/` implementations; `generic/`,
  `RPG/`, `Dead/`, and `temporary/` strategies; and per-class strategies in
  `Classes/{deathknight,druid,hunter,mage,paladin,priest,rogue,shaman,warlock,warrior}/`.
- `src/Factory/` creates and randomizes bots and their character data.
- `src/Manager/` contains `PlayerbotMgr`, `PlayerbotsMgr`,
  `RandomPlayerbotMgr`, bracket management, performance monitoring, random
  item management, and related coordination.
- `src/Travel/` contains travel-node, path, destination, and travel strategy
  support; movement also appears in strategy actions and core `PathGenerator`.
- `src/Utils/` contains bot-specific utility support.
- Questing, combat, movement, chat, inventory/economy, loot, guild/group, and
  other behavior are expressed across the strategy actions/triggers/values
  rather than as independent top-level modules.

### Integration and data

- Root `PLAYERBOTS` defaults ON. `modules/CMakeLists.txt` removes
  `mod_playerbots` when it is OFF; otherwise its sources are globbed into the
  static `modules` library. `USE_MODULES` must also be ON for that library to
  be added and linked through `scripts`.
- The generated modules loader calls `AddSC_mod_playerbots()`. Playerbots uses
  core script hooks, entity/session/packet APIs, core databases, pathfinding,
  and world data; it is not an external process.
- `modules/mod_playerbots/config/playerbots.conf.dist` is the runtime
  configuration template and is loaded as the additional `playerbots.conf`.
- `modules/mod_playerbots/data/sql/characters/` and `world/` contain supporting
  character/name and world data. `data/sql/playerbots/` contains create/drop
  scripts, a base schema split into files (including `version_db_playerbots.sql`
  and `updates_include.sql`), and dated updates under
  `updates/db_playerbots/` for the separate Playerbots database.
- Core database implementations include
  `src/server/database/Database/Implementation/PlayerbotsDatabase.*`.
  `worldserver` conditionally opens this fourth connection under the
  `PLAYERBOTS` compile definition.

No Playerbots runtime or behavior validation was performed for this map.

## Eluna and the module system

### Eluna

- `ELUNA` controls compilation of `src/server/game/LuaEngine/` into `game`.
  That directory contains Eluna bindings, hooks, object methods, and extension
  code.
- `dep/lualib/` supplies the Lua library target. With Eluna enabled, `game`
  and `scripts` add the Lua dependency/include paths, and `worldserver` carries
  the relevant link dependency.
- `contrib/lua_scripts/` is the repository's live runtime script tree; its
  scripts are data deployed alongside a configured Eluna-enabled server, not
  C++ sources in the `scripts` library. Lua engine extensions also live under
  `src/server/game/LuaEngine/extensions/`.
- Eluna configuration is primarily in `worldserver.conf.dist` and CMake's
  `ELUNA` option. The Lua script directory is an operational/deployment input.

### Repository modules and `USE_MODULES`

- `USE_MODULES` controls whether root CMake adds `modules/`.
- `modules/CMakeLists.txt` discovers subdirectories with C++ sources, gathers
  them into one static `modules` library, and links that library publicly to
  `game`, `common`, and `database`.
- It generates `modules/ModulesLoader.cpp` from a template and copies it into
  the source tree. Consequently, module-enabled configuration can modify that
  tracked/generated source file and repository status must be checked.
- `scripts` depends on and privately links `modules`, defines `USE_MODULES`,
  and invokes the generated loader from its central `AddScripts()` flow.
- Modules are therefore compiled statically into the worldserver dependency
  graph; this repository does not load them as runtime shared-library plugins.
- Module-owned configuration and SQL live inside the module, exemplified by
  `modules/mod_playerbots/config/` and `modules/mod_playerbots/data/sql/`.

## Databases and SQL

### Logical databases

- **Auth/Login**: account, realm, access, build, uptime and authentication
  state. Base: `sql/base/auth.sql`; connection implementation:
  `LoginDatabase.*`; used by both server processes.
- **Characters**: characters and persistent player/social/group/guild/mail/
  auction/instance state. Base: `sql/base/characters.sql`; implementation:
  `CharacterDatabase.*`; used by worldserver.
- **World**: templates, spawns, quests, scripts, loot, conditions, events and
  other realm content. Implementation: `WorldDatabase.*`; current migrations:
  `sql/updates/world/`. This checkout does **not** contain a
  `sql/base/world.sql`, so establishing the provenance of a compatible full
  world base is a separate deployment/preservation concern, not something to
  infer from the update files.
- **Playerbots** (when compiled/configured): module-specific runtime and cache
  data under `modules/mod_playerbots/data/sql/playerbots/`, with supporting
  changes for character/world databases in sibling directories.

### SQL layout

- `sql/base/`: actual base dumps for auth and characters.
- `sql/updates/world/` and `sql/updates/characters/`: dated active updates.
- `sql/updates/master/auth/`, `master/characters/`, and `master/custom/`:
  additional tracked update/custom SQL areas. Which paths a deployed database
  includes is governed by its `updates_include` rows; directory placement
  alone does not prove automatic application.
- `sql/archive/`: archived auth/character/world update areas.
- `sql/old/`: large historical, merged, localization, test, work-in-progress,
  and other legacy SQL collections. Treat these as evidence/history unless a
  later task establishes an application path; do not bulk-apply them.
- Module SQL remains under its module rather than being merged into root SQL.

### Automatic update mechanism

`src/server/database/Updater/DBUpdater.*` and `UpdateFetcher.*` implement the
embedded updater. `DatabaseLoader` opens configured pools and, according to
update-mask/config settings, can create/populate/update them. The updater:

1. reads directories and states from each database's `updates_include` table;
2. recursively finds `.sql` files (with bounded depth), ordering/comparing by
   filename;
3. records applied filename, SHA-1 hash, state, timestamp and speed in
   `updates`;
4. applies pending files and performs configured redundancy/rehash checks.

The base auth and characters dumps define `updates` and `updates_include`.
Server templates expose `Updates.EnableDatabases`, `Updates.AutoSetup`,
redundancy, archived redundancy, rehash, and dead-reference cleanup controls;
the checked-in templates default automatic updates off for the normal server
DBs. Playerbots has its own update setting and update tables/files. Thus SQL
should be selected using both configuration and `updates_include`, not merely
all files visible under `sql/updates/`. No SQL was executed during Task 006.

## Build targets and relationships

The high-level static dependency graph observed in component CMake files is:

```text
vendored/platform libraries
        |
      common
        |
     database  <--------------------------+
        |                                  |
      shared                               |
        |                                  |
       game ---- Detour/Recast             |
        |                                  |
        +---- modules (USE_MODULES) --------+
        |          |
        +------ scripts <-------------------+
                   |
               worldserver

authserver --> common + shared + database
```

More precisely:

- `common` is the base project library and uses selected vendored/system
  libraries.
- `database` privately links `common`, MySQL, and Boost.
- `shared` privately uses `common` and publicly exposes `database`.
- `game` publicly links Detour, `common`, `database`, and `shared`, and
  privately links zlib; Eluna adds Lua.
- `modules`, when enabled, is a static library linked publicly to `game`,
  `common`, and `database`.
- `scripts` is a static library linked publicly to `game`; with modules it
  also depends on/links `modules`, and with Eluna it depends on Lua.
- `authserver` links `common`, `shared`, `database`, MySQL, OpenSSL and its
  runtime/support libraries. It does not depend on `game` or `scripts`.
- `worldserver` links `database`, `scripts`, `game`, gSOAP, readline and
  runtime/support libraries. The `scripts -> modules` edge brings modules and
  Playerbots into this final executable when enabled.
- `revision.h` is an explicit prerequisite across major targets.
- `TOOLS=1` adds extractor/assembler executable targets under `src/tools/`;
  these use the needed shared and vendored map/navigation libraries but are
  not runtime dependencies of the server executables.
- `UPDATER=1` adds the optional standalone `src/updater/` executable.

Task 005's configuration-only target discovery confirmed `common`, `database`,
`shared`, `game`, `scripts`, `authserver`, and `worldserver` in the fast
profile. That profile disabled modules and tools, so their targets were
intentionally absent. Task 006 performed no build and no CMake configuration.

## Configuration map

- `src/server/authserver/authserver.conf.dist`: auth listener, Login database,
  logging, update controls, and authserver runtime settings.
- `src/server/worldserver/worldserver.conf.dist`: realm/world behavior,
  Login/World/Character connections, data directory, networking, logging,
  update controls, and feature settings.
- `modules/mod_playerbots/config/playerbots.conf.dist`: bot AI, random bots,
  bot database connection/threading, updater, logging and behavior settings.
- `src/updater/updater.conf.dist`: optional standalone updater configuration.
- `cmake/options.cmake`: compile-time feature controls including `SERVERS`,
  `AUTH_SERVER`, `SCRIPTS`, `TOOLS`, `ELUNA`, `PLAYERBOTS`, `USE_MODULES`, PCH,
  warnings, sanitizers and updater selection.
- `.github/workflows/`: CI compiler, option, and platform invocations.
- `contrib/lua_scripts/`: runtime Lua content when Eluna is enabled.

Never commit live credentials or operational configuration derived from these
templates.

## Preservation hotspots and future evidence

These are high-impact investigation areas, not claims of defects.

| Hotspot | Relevant paths | Evidence to inspect later | Eventual validation |
| --- | --- | --- | --- |
| Client build 18414 protocol | `Server/Protocol/`, `Server/Packets/`, `Handlers/`, `WorldSession.*`, auth session code | build-specific opcode tables, packet captures, compatible fork history, client structures | serialization/unit checks where feasible; real 18414 login, character, movement and feature flows |
| Pandaria zones/quests | `scripts/Pandaria/zone_*.cpp`, Timeless/Wandering Isle, `Quests/`, world SQL | archived retail references, sniff data, quest/objective/spawn/script DB rows, verified fork commits | targeted quest chains and phase/event transitions with a 18414 client |
| Pandaria dungeons/scenarios/raids | named directories below `scripts/Pandaria/`, `Instances/`, `Scenarios/`, world SQL | encounter mechanics/timelines, instance bindings, spell IDs, doors, spawns, difficulties, achievements | per-difficulty instance runs, resets, wipes, achievements and multiplayer mechanics |
| Spell/aura engine | `game/Spells/`, `Spells/Auras/`, `scripts/Spells/`, encounter-local scripts | 5.4.8 spell data, proc/target/effect rules, combat logs, script-name bindings | focused automated calculations plus in-client class/encounter tests |
| Movement/pathfinding | `Movement/`, `Maps/`, `Grids/`, tool sources, waypoint SQL | map/vmap/mmap provenance, movement packets, path geometry, waypoint/spline data | extracted build-18414 data, NPC/vehicle/transport and anti-cheat movement tests |
| Core entities/combat | `Entities/`, `Combat/`, `AI/` | update fields, combat logs, client data and reproducible state transitions | focused unit/integration tests and controlled client combat |
| Data-driven world behavior | `sql/updates/world/`, database managers, SmartScripts/script bindings | exact deployed world-base provenance, migration history/hashes, schema and row diffs | disposable DB migration tests followed by server log and gameplay verification |
| Database updater/schema lineage | `database/Updater/`, `sql/base/`, `sql/updates/`, module SQL | `updates_include` contents, applied hashes, missing world-base lineage, archived migration intent | disposable clean and upgrade-path databases; never production first |
| Playerbots | `modules/mod_playerbots/`, Playerbots DB implementation | module history, config/SQL version alignment, core-hook adaptations, packet/AI behavior | module-enabled targeted build, disposable bot DB, long runtime and gameplay observation |
| Eluna | `game/LuaEngine/`, `dep/lualib/`, `contrib/lua_scripts/` | Eluna API/version compatibility, bindings, script expectations and module interaction | Eluna-enabled build plus isolated script and runtime-hook testing |
| Legacy expansion content | regional script trees outside Pandaria and historical world SQL | expansion-appropriate retail/fork evidence, later-port regressions, script/DB binding history | era-specific quest, dungeon and raid regression passes |
| Build/module boundaries | component CMake files, `modules/`, loader generation | option matrix, generated-loader diffs, CI configuration | targeted builds for affected libraries and final executable link; broader matrix for CMake changes |

## Navigation examples: where should I look if I am fixing X?

1. **A client packet is decoded incorrectly.** Start with
   `Server/Protocol/Opcodes.*`, find the opcode's handler registration in
   `WorldSession`, inspect the matching `Handlers/*Handler.cpp`, then inspect
   typed structures in `Server/Packets/`. Confirm all field widths/order with
   build-18414 evidence before changing code.
2. **A Pandaria quest stalls.** Locate its zone script under
   `scripts/Pandaria/`, its core quest flow in `QuestHandler.cpp`/`QuestDef`,
   and its world rows (quest template, objectives, creatures/gameobjects,
   SmartAI, conditions, phases, and script names). A C++ edit alone may not be
   the correct layer.
3. **A raid boss mechanic misbehaves.** Start in the named raid directory,
   then follow spell IDs into encounter-local SpellScripts and the spell/aura
   engine. Check instance script state, database bindings and difficulty data.
4. **An NPC chooses a bad route.** Check its script/AI, world waypoint or
   spline rows, `MotionMaster`/movement generators, `PathGenerator`, and the
   exact map/vmap/mmap data used by the runtime.
5. **Authentication fails before realm entry.** Start in
   `authserver/Server/AuthSession.*`, shared realm/network definitions, and the
   auth schema/config. If login reaches worldserver, continue into
   `WorldSocket`, authentication packets, and session initialization.
6. **A SQL update is not applied.** Inspect server update settings, the target
   DB's `updates_include` and `updates` tables, the update filename/path, and
   `database/Updater/`. Do not assume every root update directory is included.
7. **A bot strategy needs work.** Start in the class/generic strategy,
   identify its actions, triggers and values, then follow `PlayerbotAI` and
   manager integration. Include module config and Playerbots/character/world
   SQL in the evidence set.
8. **A Lua hook fails.** Determine whether `ELUNA` was enabled, inspect the
   relevant LuaEngine binding/hook and live script, then verify runtime script
   deployment and worldserver configuration. Do not treat the C++ `scripts`
   target as validation of Lua behavior.
9. **A new module does not register.** Inspect `modules/CMakeLists.txt`, its
   generated `ModulesLoader.cpp`, the module's `AddSC_*` name, the
   `scripts` module dependency, and `USE_MODULES`. Check Git status after
   configuration because loader generation writes into the source tree.

This map should narrow the first inspection point. Preservation changes must
still be evidence-backed, narrowly scoped, and validated at the appropriate
compile, database, runtime, and build-18414 client levels.
