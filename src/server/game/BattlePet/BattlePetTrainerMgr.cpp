/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "BattlePetTrainerMgr.h"
#include "BattlePet.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "PetBattle.h"
#include "SharedDefines.h"
#include "Timer.h"

void BattlePetTrainerMgr::LoadBattleTrainerPets()
{
    uint32 oldMSTime = getMSTime();
    BattlePetTrainerContainer trainerPets;

    QueryResult result = WorldDatabase.Query("SELECT entry, species, level, quality, breed FROM battle_pet_trainer ORDER BY entry, species");
    if (!result)
    {
        m_trainerPets.swap(trainerPets);
        TC_LOG_INFO("server.loading", ">> Loaded 0 battle pet trainer definitions. DB table `battle_pet_trainer` is empty.");
        return;
    }

    std::set<uint32> invalidEntries;
    do
    {
        Field* fields = result->Fetch();
        uint32 entry = fields[0].GetUInt32();
        uint32 species = fields[1].GetUInt32();
        uint32 level = fields[2].GetUInt32();
        uint32 quality = fields[3].GetUInt32();
        uint32 breed = fields[4].GetUInt32();

        if (!sObjectMgr->GetCreatureTemplate(entry))
        {
            TC_LOG_ERROR("sql.sql", "Battle pet trainer entry %u has no CreatureTemplate; its team is skipped.", entry);
            invalidEntries.insert(entry);
            continue;
        }

        BattlePetSpeciesEntry const* speciesEntry = sBattlePetSpeciesStore.LookupEntry(species);
        if (!speciesEntry)
        {
            TC_LOG_ERROR("sql.sql", "Battle pet trainer entry %u uses unknown species %u; its team is skipped.", entry, species);
            invalidEntries.insert(entry);
            continue;
        }

        if (!sObjectMgr->GetCreatureTemplate(speciesEntry->NpcId))
        {
            TC_LOG_ERROR("sql.sql", "Battle pet trainer entry %u uses species %u with missing creature template %u; its team is skipped.",
                entry, species, speciesEntry->NpcId);
            invalidEntries.insert(entry);
            continue;
        }

        if (!level || level > BATTLE_PET_MAX_LEVEL)
        {
            TC_LOG_ERROR("sql.sql", "Battle pet trainer entry %u uses invalid level %u for species %u; its team is skipped.", entry, level, species);
            invalidEntries.insert(entry);
            continue;
        }

        if (quality > ITEM_QUALITY_LEGENDARY || !sBattlePetBreedQualityStore.LookupEntry(7 + quality))
        {
            TC_LOG_ERROR("sql.sql", "Battle pet trainer entry %u uses invalid quality %u for species %u; its team is skipped.", entry, quality, species);
            invalidEntries.insert(entry);
            continue;
        }

        if (breed && sBattlePetBreedSet.find(breed) == sBattlePetBreedSet.end())
        {
            TC_LOG_ERROR("sql.sql", "Battle pet trainer entry %u uses invalid breed %u for species %u; its team is skipped.", entry, breed, species);
            invalidEntries.insert(entry);
            continue;
        }

        trainerPets[entry].push_back({ species, uint8(level), uint8(quality), uint8(breed) });
    } while (result->NextRow());

    for (auto const& trainer : trainerPets)
    {
        if (trainer.second.size() > PET_BATTLE_MAX_TEAM_PETS)
        {
            TC_LOG_ERROR("sql.sql", "Battle pet trainer entry %u has %u pets; the maximum is %u and its team is skipped.",
                trainer.first, uint32(trainer.second.size()), uint32(PET_BATTLE_MAX_TEAM_PETS));
            invalidEntries.insert(trainer.first);
        }
    }

    for (uint32 entry : invalidEntries)
        trainerPets.erase(entry);

    uint32 petCount = 0;
    for (auto const& trainer : trainerPets)
        petCount += trainer.second.size();

    m_trainerPets.swap(trainerPets);
    TC_LOG_INFO("server.loading", ">> Loaded %u battle pet trainer pets in %u definitions in %u ms.",
        petCount, uint32(m_trainerPets.size()), GetMSTimeDiffToNow(oldMSTime));
}

TrainerBattlePetStore const* BattlePetTrainerMgr::GetTrainerTeam(uint32 entry) const
{
    auto itr = m_trainerPets.find(entry);
    if (itr != m_trainerPets.end())
        return &itr->second;

    return nullptr;
}
