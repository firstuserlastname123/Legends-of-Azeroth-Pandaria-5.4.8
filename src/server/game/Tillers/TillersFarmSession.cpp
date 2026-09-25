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

bool TillersFarmSession::SetFarmPhase(FarmState state)
{
    if (!IsUsable() || !FarmDataValidation::IsValidFarmState(state))
        return false;

    if (_data.state.farmPhase != state)
    {
        _data.state.farmPhase = state;
        _dirty = true;
    }
    return true;
}

bool TillersFarmSession::SetPlotsUnlocked(uint8 count)
{
    if (!IsUsable() || !FarmDataValidation::IsValidPlotCount(count))
        return false;

    if (_data.state.plotsUnlocked != count)
    {
        _data.state.plotsUnlocked = count;
        _dirty = true;
    }
    return true;
}

bool TillersFarmSession::SetBestFriendUnlocks(uint16 unlocks)
{
    if (!IsUsable())
        return false;

    if (_data.state.bestFriendUnlocks != unlocks)
    {
        _data.state.bestFriendUnlocks = unlocks;
        _dirty = true;
    }
    return true;
}

bool TillersFarmSession::EnsurePlot(uint8 plotId)
{
    if (!IsUsable() || !FarmDataValidation::IsValidPlotId(plotId))
        return false;

    if (_data.plots.count(plotId) == 0)
    {
        FarmPlotData plot;
        plot.plotId = plotId;
        _data.plots.emplace(plotId, std::move(plot));
        _dirty = true;
    }
    return true;
}

bool TillersFarmSession::RemovePlot(uint8 plotId)
{
    if (!IsUsable() || !FarmDataValidation::IsValidPlotId(plotId))
        return false;

    if (_data.plots.erase(plotId) != 0)
        _dirty = true;
    return true;
}

bool TillersFarmSession::SetPlotState(uint8 plotId, FarmPlotState state)
{
    if (!FarmDataValidation::IsValidPlotState(state))
        return false;

    FarmPlotData* plot = GetMutablePlot(plotId);
    if (!plot)
        return false;

    if (plot->state != state)
    {
        plot->state = state;
        _dirty = true;
    }
    return true;
}

bool TillersFarmSession::SetPlotSeed(uint8 plotId, std::optional<uint32> seedEntry)
{
    FarmPlotData* plot = GetMutablePlot(plotId);
    if (!plot)
        return false;

    if (plot->seedEntry != seedEntry)
    {
        plot->seedEntry = seedEntry;
        _dirty = true;
    }
    return true;
}

bool TillersFarmSession::SetPlotNeedsWatering(uint8 plotId, bool needsWatering)
{
    FarmPlotData* plot = GetMutablePlot(plotId);
    if (!plot)
        return false;

    if (plot->needsWatering != needsWatering)
    {
        plot->needsWatering = needsWatering;
        _dirty = true;
    }
    return true;
}

bool TillersFarmSession::SetPlotHasPests(uint8 plotId, bool hasPests)
{
    FarmPlotData* plot = GetMutablePlot(plotId);
    if (!plot)
        return false;

    if (plot->hasPests != hasPests)
    {
        plot->hasPests = hasPests;
        _dirty = true;
    }
    return true;
}

bool TillersFarmSession::SetPlotMaturity(uint8 plotId, std::optional<time_t> maturity)
{
    if (maturity && !FarmDataValidation::IsValidMaturity(*maturity))
        return false;

    FarmPlotData* plot = GetMutablePlot(plotId);
    if (!plot)
        return false;

    if (plot->maturity != maturity)
    {
        plot->maturity = maturity;
        _dirty = true;
    }
    return true;
}

FarmPlotData* TillersFarmSession::GetMutablePlot(uint8 plotId)
{
    if (!IsUsable() || !FarmDataValidation::IsValidPlotId(plotId))
        return nullptr;

    auto itr = _data.plots.find(plotId);
    return itr != _data.plots.end() ? &itr->second : nullptr;
}
}
