/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#ifndef TILLERS_FARM_GAMEPLAY_H
#define TILLERS_FARM_GAMEPLAY_H

#include "TillersFarmPlayerIntegration.h"
#include "TillersFarmPlantingIntegration.h"

namespace Tillers
{
FarmPlayerHarvestResult ExecutePlayerHarvestWithServerRolls(Player& player, uint8 plotId);
FarmPlayerPlantingResult ExecutePlayerPlantingWithServerRolls(Player& player, uint8 plotId,
    uint32 seedEntry);
}

#endif
