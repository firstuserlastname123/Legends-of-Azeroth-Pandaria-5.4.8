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

enum class FarmPlotLifecycleResult : uint8
{
    Applied,
    NoChange,
    MissingPlot,
    LockedPlot,
    InconsistentFarm,
    Unusable,
    WrongState
};

enum class FarmPlantingOutcome : uint8
{
    Seeded,
    NeedsWater,
    NeedsPestControl,
    ReadyToHarvest
};

enum class FarmPlantingResult : uint8
{
    Applied,
    MissingPlot,
    LockedPlot,
    InconsistentFarm,
    Unusable,
    WrongState,
    InvalidSeed,
    InvalidMaturity,
    InvalidBurstRoll,
    InvalidProblemRoll,
    InvalidResetTime
};

struct FarmPlantingPlan
{
    FarmPlantingOutcome outcome;
    std::optional<time_t> maturity;
};

enum class FarmPlantingPolicyResult : uint8
{
    Ready,
    InvalidBurstRoll,
    InvalidProblemRoll,
    InvalidResetTime
};

// Logical reward data preserved from the historical custom Tillers implementation.
// Applying these rewards and finalizing the plot are deliberately separate operations.
struct FarmHarvestPlan
{
    uint32 plantedSeedEntry = 0;
    uint32 primaryItemEntry = 0;
    uint8 primaryItemCount = 0;
    uint8 returnedSeedCount = 0;
    bool plumpBonus = false;
    bool legacySeedFallback = false;
};

enum class FarmHarvestPolicyResult : uint8
{
    Ready,
    InvalidSeed,
    InvalidPlumpRoll,
    InvalidSeedReturnRoll,
    InvalidSeedReturnCount
};

enum class FarmHarvestResult : uint8
{
    Ready,
    MissingPlot,
    LockedPlot,
    InconsistentFarm,
    Unusable,
    WrongState,
    MissingSeed,
    InvalidSeed,
    InvalidPlumpRoll,
    InvalidSeedReturnRoll,
    InvalidSeedReturnCount
};

std::optional<uint8> GetCanonicalPlotsUnlocked(FarmState state);
FarmPlantingPolicyResult BuildPlantingPlan(uint16 burstRoll, uint16 problemRoll, time_t nextReset,
    FarmPlantingPlan& plan);
std::optional<uint32> GetPreservedHarvestItemForSeed(uint32 seedEntry);
FarmHarvestPolicyResult BuildHarvestRewardPlan(uint32 seedEntry, uint8 plumpRoll,
    uint8 seedReturnRoll, uint8 seedReturnCount, FarmHarvestPlan& plan);

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

    FarmPlotLifecycleResult MaturePlotIfDue(uint8 plotId, time_t now);
    FarmPlotLifecycleResult ResolvePlotWatering(uint8 plotId);
    FarmPlotLifecycleResult ResolvePlotPests(uint8 plotId);
    FarmPlotLifecycleResult ResetHarvestedPlot(uint8 plotId);
    bool IsPlotReadyToHarvest(uint8 plotId) const;
    FarmHarvestResult PrepareHarvest(uint8 plotId, uint8 plumpRoll, uint8 seedReturnRoll,
        uint8 seedReturnCount, FarmHarvestPlan& plan) const;

    FarmPlantingResult PlantCrop(uint8 plotId, uint32 seedEntry, FarmPlantingOutcome outcome,
        std::optional<time_t> maturity);
    FarmPlantingResult PlantCropWithPolicy(uint8 plotId, uint32 seedEntry, uint16 burstRoll,
        uint16 problemRoll, time_t nextReset);
    bool IsPlotPlantable(uint8 plotId) const;

    FarmSaveRequestResult RequestSave();
    void ProcessPersistence();

private:
    FarmPlotLifecycleResult GetLifecyclePlot(uint8 plotId, FarmPlotData*& plot);
    FarmPlotLifecycleResult GetLifecyclePlot(uint8 plotId, FarmPlotData const*& plot) const;
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
