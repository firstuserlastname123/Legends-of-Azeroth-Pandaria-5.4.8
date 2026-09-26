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
enum class FarmSaveRequestResult : uint8
{
    Queued,
    NoChanges,
    AlreadyPending,
    Rejected
};

enum class FarmSaveCompletionResult : uint8
{
    None,
    Success,
    Failure
};

enum class FarmProgressionResult : uint8
{
    Advanced,
    AtMaximum,
    InconsistentState,
    Unusable
};

std::optional<uint8> GetCanonicalPlotsUnlocked(FarmState state);

class TillersFarmSession
{
public:
    explicit TillersFarmSession(uint32 ownerGuidLow);

    uint32 GetOwnerGuidLow() const { return _ownerGuidLow; }
    FarmLoadStatus GetLoadStatus() const { return _data.loadStatus; }
    bool IsUsable() const { return _data.loadStatus != FarmLoadStatus::InvalidRoot; }
    bool IsDirty() const { return _dirty; }
    bool NeedsPersistence() const { return _dirty || _data.loadStatus == FarmLoadStatus::NotPersisted; }
    bool HasPendingSave() const { return _pendingSave.has_value(); }
    FarmSaveCompletionResult GetLastSaveResult() const { return _lastSaveResult; }
    PlayerFarmData const& GetFarmData() const { return _data; }
    PlayerFarmState const& GetFarmState() const { return _data.state; }
    std::map<uint8, FarmPlotData> const& GetPlots() const { return _data.plots; }
    FarmPlotData const* GetPlot(uint8 plotId) const;

    bool IsProgressionConsistent() const;
    FarmProgressionResult AdvanceFarmProgression();
    bool IsPlotUnlocked(uint8 plotId) const;

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

    FarmSaveRequestResult RequestSave();
    void ProcessPersistence();

private:
    FarmPlotData* GetMutablePlot(uint8 plotId);
    void MarkDirty();
    void HandleSaveCompletion(bool success, uint64 savedRevision);

    uint32 const _ownerGuidLow;
    PlayerFarmData _data;
    bool _dirty = false;
    uint64 _mutationRevision = 0;
    uint64 _pendingSaveRevision = 0;
    std::optional<TransactionCallback> _pendingSave;
    FarmSaveCompletionResult _lastSaveResult = FarmSaveCompletionResult::None;
};
}

#endif
