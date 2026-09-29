/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#ifndef TILLERS_FARM_PLANTING_INTEGRATION_H
#define TILLERS_FARM_PLANTING_INTEGRATION_H

#include "Define.h"
#include <ctime>

class Player;

namespace Tillers
{
enum class FarmPlayerPlantingResult : uint8
{
    Applied,
    PlayerNotInWorld,
    NoFarmSession,
    OwnerMismatch,
    PersistenceBusy,
    MissingPlot,
    LockedPlot,
    InconsistentFarm,
    Unusable,
    WrongState,
    HarvestClaimPending,
    InvalidSeed,
    MissingSeedItem,
    InvalidBurstRoll,
    InvalidProblemRoll,
    InvalidResetTime,
    PersistenceRejected
};

FarmPlayerPlantingResult ExecutePlayerPlanting(Player& player, uint8 plotId, uint32 seedEntry,
    uint16 burstRoll, uint16 problemRoll, time_t nextReset);
}

#endif
