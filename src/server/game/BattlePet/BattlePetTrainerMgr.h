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

#ifndef BATTLE_PET_TRAINER_MGR_H
#define BATTLE_PET_TRAINER_MGR_H

#include "Common.h"

struct TrainerBattlePet
{
    uint32 Species;
    uint8 Level;
    uint8 Quality;
    uint8 Breed;
};

using TrainerBattlePetStore = std::vector<TrainerBattlePet>;
using BattlePetTrainerContainer = std::map<uint32, TrainerBattlePetStore>;

class BattlePetTrainerMgr
{
public:
    static BattlePetTrainerMgr* instance()
    {
        static BattlePetTrainerMgr instance;
        return &instance;
    }

    void LoadBattleTrainerPets();

    TrainerBattlePetStore const* GetTrainerTeam(uint32 entry) const;

private:
    BattlePetTrainerContainer m_trainerPets;
};

#define sBattlePetTrainerMgr BattlePetTrainerMgr::instance()

#endif
