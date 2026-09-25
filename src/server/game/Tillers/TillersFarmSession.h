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
    bool IsUsable() const { return _data.loadStatus != FarmLoadStatus::InvalidRoot; }
    bool IsDirty() const { return _dirty; }
    PlayerFarmData const& GetFarmData() const { return _data; }
    PlayerFarmState const& GetFarmState() const { return _data.state; }
    std::map<uint8, FarmPlotData> const& GetPlots() const { return _data.plots; }
    FarmPlotData const* GetPlot(uint8 plotId) const;

    bool SetFarmPhase(FarmState state);
    bool SetPlotsUnlocked(uint8 count);
    bool SetBestFriendUnlocks(uint16 unlocks);

    bool EnsurePlot(uint8 plotId);
    bool RemovePlot(uint8 plotId);
    bool SetPlotState(uint8 plotId, FarmPlotState state);
    bool SetPlotSeed(uint8 plotId, std::optional<uint32> seedEntry);
    bool SetPlotNeedsWatering(uint8 plotId, bool needsWatering);
    bool SetPlotHasPests(uint8 plotId, bool hasPests);
    bool SetPlotMaturity(uint8 plotId, std::optional<time_t> maturity);

private:
    FarmPlotData* GetMutablePlot(uint8 plotId);

    uint32 const _ownerGuidLow;
    PlayerFarmData _data;
    bool _dirty = false;
};
}

#endif
