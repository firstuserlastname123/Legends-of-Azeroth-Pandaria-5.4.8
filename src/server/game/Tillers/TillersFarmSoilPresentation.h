#ifndef TILLERS_FARM_SOIL_PRESENTATION_H
#define TILLERS_FARM_SOIL_PRESENTATION_H

#include "Define.h"

class GameObject;
class Player;

namespace Tillers
{
enum class FarmSoilResolveResult : uint8
{
    Resolved,
    InvalidPlayer,
    InvalidObject,
    WrongContext,
    WrongOwner,
    UnusableSession,
    UnregisteredOrIneligible
};

FarmSoilResolveResult ResolvePlayerSoilObject(Player const& player, GameObject const& soil, uint8& plotId);
bool PresentPlayerSoil(Player& player);
void RemovePlayerSoil(Player& player, bool removeObjects);
}

#endif
