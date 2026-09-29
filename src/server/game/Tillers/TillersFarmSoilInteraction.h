/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#ifndef TILLERS_FARM_SOIL_INTERACTION_H
#define TILLERS_FARM_SOIL_INTERACTION_H

class GameObject;
class Player;

namespace Tillers
{
// Returns false only when normal GameObject use processing should continue.
bool TryHandleRegisteredSoilUse(Player& player, GameObject& soil);
}

#endif
