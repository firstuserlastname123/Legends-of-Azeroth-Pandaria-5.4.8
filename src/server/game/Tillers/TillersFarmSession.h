/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#ifndef TILLERS_FARM_SESSION_H
#define TILLERS_FARM_SESSION_H

#include "TillersFarmPersistence.h"

namespace Tillers
{
class TillersFarmSession
{
public:
    explicit TillersFarmSession(uint32 ownerGuidLow);

    uint32 GetOwnerGuidLow() const { return _ownerGuidLow; }
    FarmLoadStatus GetLoadStatus() const { return _data.loadStatus; }
    PlayerFarmData const& GetFarmData() const { return _data; }
    PlayerFarmState const& GetFarmState() const { return _data.state; }
    FarmPlotData const* GetPlot(uint8 plotId) const;

private:
    uint32 const _ownerGuidLow;
    PlayerFarmData _data;
};
}

#endif
