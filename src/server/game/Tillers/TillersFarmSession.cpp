/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#include "TillersFarmSession.h"

namespace Tillers
{
TillersFarmSession::TillersFarmSession(uint32 ownerGuidLow)
    : _ownerGuidLow(ownerGuidLow), _data(TillersFarmPersistence::Load(ownerGuidLow))
{
}

FarmPlotData const* TillersFarmSession::GetPlot(uint8 plotId) const
{
    auto itr = _data.plots.find(plotId);
    return itr != _data.plots.end() ? &itr->second : nullptr;
}
}
