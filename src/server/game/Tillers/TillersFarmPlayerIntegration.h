/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#ifndef TILLERS_FARM_PLAYER_INTEGRATION_H
#define TILLERS_FARM_PLAYER_INTEGRATION_H

#include "Define.h"

class Player;

namespace Tillers
{
enum class FarmPlayerHarvestResult : uint8
{
    Applied,
    NoFarmSession,
    PersistenceBusy,
    AlreadyClaimed,
    MissingPlot,
    LockedPlot,
    InconsistentFarm,
    Unusable,
    WrongState,
    MissingSeed,
    InvalidSeed,
    InvalidPlumpRoll,
    InvalidSeedReturnRoll,
    InvalidSeedReturnCount,
    InventoryRejected,
    FinalizeRejected,
    PersistenceRejected,
    RewardDeliveryFailed
};

FarmPlayerHarvestResult ExecutePlayerHarvest(Player& player, uint8 plotId, uint8 plumpRoll,
    uint8 seedReturnRoll, uint8 seedReturnCount);
}

#endif
