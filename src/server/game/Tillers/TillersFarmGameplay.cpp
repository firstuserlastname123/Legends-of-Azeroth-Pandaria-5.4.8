/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#include "TillersFarmGameplay.h"
#include "Random.h"

namespace Tillers
{
FarmPlayerHarvestResult ExecutePlayerHarvestWithServerRolls(Player& player, uint8 plotId)
{
    uint8 plumpRoll = urand(1, 100);
    uint8 seedReturnRoll = urand(0, 1);
    uint8 seedReturnCount = seedReturnRoll == 0 ? 0 : urand(1, 3);

    return ExecutePlayerHarvest(player, plotId, plumpRoll, seedReturnRoll, seedReturnCount);
}
}
