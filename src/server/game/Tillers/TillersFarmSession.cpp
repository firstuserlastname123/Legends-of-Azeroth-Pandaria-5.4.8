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
std::optional<uint8> GetCanonicalPlotsUnlocked(FarmState state)
{
    switch (state)
    {
        case FarmState::Initial:
            return 4;
        case FarmState::WeedsAndWagonRemaining:
            return 8;
        case FarmState::WagonRemaining:
            return 12;
        case FarmState::Cleared:
            return 16;
        default:
            return std::nullopt;
    }
}

FarmPlantingPolicyResult BuildPlantingPlan(uint16 burstRoll, uint16 problemRoll, time_t nextReset,
    FarmPlantingPlan& plan)
{
    if (burstRoll < 1 || burstRoll > 1000)
        return FarmPlantingPolicyResult::InvalidBurstRoll;

    if (burstRoll <= 11)
    {
        plan = { FarmPlantingOutcome::ReadyToHarvest, std::nullopt };
        return FarmPlantingPolicyResult::Ready;
    }

    if (problemRoll < 1 || problemRoll > 1000)
        return FarmPlantingPolicyResult::InvalidProblemRoll;
    if (nextReset <= 0 || !FarmDataValidation::IsValidMaturity(nextReset))
        return FarmPlantingPolicyResult::InvalidResetTime;

    FarmPlantingOutcome outcome = FarmPlantingOutcome::Seeded;
    if (problemRoll <= 140)
        outcome = FarmPlantingOutcome::NeedsWater;
    else if (problemRoll <= 280)
        outcome = FarmPlantingOutcome::NeedsPestControl;

    plan = { outcome, nextReset };
    return FarmPlantingPolicyResult::Ready;
}

std::optional<uint32> GetPreservedHarvestItemForSeed(uint32 seedEntry)
{
    // This table preserves historical custom-server policy; it is not asserted as
    // independently verified retail 5.4.8 behavior.
    switch (seedEntry)
    {
        case 79102: return 74840;
        case 80590: return 74841;
        case 80591: return 74843;
        case 80592: return 74842;
        case 80593: return 74844;
        case 80594: return 74849;
        case 80595: return 74850;
        case 89328: return 74847;
        case 89329: return 74848;
        default: return std::nullopt;
    }
}

FarmHarvestPolicyResult BuildHarvestRewardPlan(uint32 seedEntry, uint8 plumpRoll,
    uint8 seedReturnRoll, uint8 seedReturnCount, FarmHarvestPlan& plan)
{
    if (seedEntry == 0)
        return FarmHarvestPolicyResult::InvalidSeed;
    if (plumpRoll < 1 || plumpRoll > 100)
        return FarmHarvestPolicyResult::InvalidPlumpRoll;
    if (seedReturnRoll > 1)
        return FarmHarvestPolicyResult::InvalidSeedReturnRoll;
    if (seedReturnRoll == 1 && (seedReturnCount < 1 || seedReturnCount > 3))
        return FarmHarvestPolicyResult::InvalidSeedReturnCount;

    std::optional<uint32> const harvestItem = GetPreservedHarvestItemForSeed(seedEntry);
    bool const plumpBonus = plumpRoll <= 5;

    FarmHarvestPlan completePlan;
    completePlan.plantedSeedEntry = seedEntry;
    completePlan.primaryItemEntry = harvestItem.value_or(seedEntry);
    completePlan.primaryItemCount = harvestItem ? 5 : 1;
    completePlan.returnedSeedCount = seedReturnRoll == 1 ? seedReturnCount : 0;
    completePlan.plumpBonus = plumpBonus;
    completePlan.legacySeedFallback = !harvestItem;
    if (plumpBonus)
        completePlan.primaryItemCount += 3;

    plan = completePlan;
    return FarmHarvestPolicyResult::Ready;
}

TillersFarmSession::TillersFarmSession(uint32 ownerGuidLow)
    : _ownerGuidLow(ownerGuidLow), _data(TillersFarmPersistence::Load(ownerGuidLow))
{
}

FarmPlotData const* TillersFarmSession::GetPlot(uint8 plotId) const
{
    auto itr = _data.plots.find(plotId);
    return itr != _data.plots.end() ? &itr->second : nullptr;
}

bool TillersFarmSession::IsProgressionConsistent() const
{
    if (!IsUsable())
        return false;

    std::optional<uint8> canonicalPlots = GetCanonicalPlotsUnlocked(_data.state.farmPhase);
    return canonicalPlots && _data.state.plotsUnlocked == *canonicalPlots;
}

FarmProgressionResult TillersFarmSession::AdvanceFarmProgression()
{
    if (!IsUsable())
        return FarmProgressionResult::Unusable;

    if (!IsProgressionConsistent())
        return FarmProgressionResult::InconsistentState;

    FarmState nextState;
    uint8 nextPlotsUnlocked;
    switch (_data.state.farmPhase)
    {
        case FarmState::Initial:
            nextState = FarmState::WeedsAndWagonRemaining;
            nextPlotsUnlocked = 8;
            break;
        case FarmState::WeedsAndWagonRemaining:
            nextState = FarmState::WagonRemaining;
            nextPlotsUnlocked = 12;
            break;
        case FarmState::WagonRemaining:
            nextState = FarmState::Cleared;
            nextPlotsUnlocked = 16;
            break;
        case FarmState::Cleared:
            return FarmProgressionResult::AtMaximum;
        default:
            return FarmProgressionResult::InconsistentState;
    }

    _data.state.farmPhase = nextState;
    _data.state.plotsUnlocked = nextPlotsUnlocked;
    MarkDirty();
    return FarmProgressionResult::Advanced;
}

bool TillersFarmSession::IsPlotUnlocked(uint8 plotId) const
{
    return FarmDataValidation::IsValidPlotId(plotId) && IsProgressionConsistent() &&
        plotId < _data.state.plotsUnlocked;
}

bool TillersFarmSession::SetFarmPhase(FarmState state)
{
    if (!IsUsable() || !FarmDataValidation::IsValidFarmState(state))
        return false;

    if (_data.state.farmPhase != state)
    {
        _data.state.farmPhase = state;
        MarkDirty();
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
        MarkDirty();
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
        MarkDirty();
    }
    return true;
}

bool TillersFarmSession::EnsurePlot(uint8 plotId)
{
    if (!IsPlotUnlocked(plotId))
        return false;

    if (_data.plots.count(plotId) == 0)
    {
        FarmPlotData plot;
        plot.plotId = plotId;
        _data.plots.emplace(plotId, std::move(plot));
        MarkDirty();
    }

    return true;
}

bool TillersFarmSession::RemovePlot(uint8 plotId)
{
    if (!IsUsable() || !FarmDataValidation::IsValidPlotId(plotId))
        return false;

    if (_data.plots.erase(plotId) != 0)
    {
        MarkDirty();
    }
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
        MarkDirty();
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
        MarkDirty();
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
        MarkDirty();
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
        MarkDirty();
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
        MarkDirty();
    }
    return true;
}

FarmPlotLifecycleResult TillersFarmSession::MaturePlotIfDue(uint8 plotId, time_t now)
{
    FarmPlotData* plot = nullptr;
    FarmPlotLifecycleResult eligibility = GetLifecyclePlot(plotId, plot);
    if (eligibility != FarmPlotLifecycleResult::Applied)
        return eligibility;

    if (plot->state != FarmPlotState::Seeded && plot->state != FarmPlotState::Growing)
        return FarmPlotLifecycleResult::WrongState;

    if (!plot->maturity || now < *plot->maturity)
        return FarmPlotLifecycleResult::NoChange;

    plot->state = FarmPlotState::ReadyToHarvest;
    plot->maturity = std::nullopt;
    MarkDirty();
    return FarmPlotLifecycleResult::Applied;
}

FarmPlotLifecycleResult TillersFarmSession::ResolvePlotWatering(uint8 plotId)
{
    FarmPlotData* plot = nullptr;
    FarmPlotLifecycleResult eligibility = GetLifecyclePlot(plotId, plot);
    if (eligibility != FarmPlotLifecycleResult::Applied)
        return eligibility;

    if (plot->state != FarmPlotState::NeedsWater)
        return FarmPlotLifecycleResult::WrongState;

    plot->state = FarmPlotState::Growing;
    plot->needsWatering = false;
    MarkDirty();
    return FarmPlotLifecycleResult::Applied;
}

FarmPlotLifecycleResult TillersFarmSession::ResolvePlotPests(uint8 plotId)
{
    FarmPlotData* plot = nullptr;
    FarmPlotLifecycleResult eligibility = GetLifecyclePlot(plotId, plot);
    if (eligibility != FarmPlotLifecycleResult::Applied)
        return eligibility;

    if (plot->state != FarmPlotState::NeedsPestControl)
        return FarmPlotLifecycleResult::WrongState;

    plot->state = FarmPlotState::Growing;
    plot->hasPests = false;
    MarkDirty();
    return FarmPlotLifecycleResult::Applied;
}

FarmPlotLifecycleResult TillersFarmSession::ResetHarvestedPlot(uint8 plotId)
{
    FarmPlotData* plot = nullptr;
    FarmPlotLifecycleResult eligibility = GetLifecyclePlot(plotId, plot);
    if (eligibility != FarmPlotLifecycleResult::Applied)
        return eligibility;

    if (plot->state != FarmPlotState::ReadyToHarvest)
        return FarmPlotLifecycleResult::WrongState;

    plot->state = FarmPlotState::SoilPrepared;
    plot->seedEntry = std::nullopt;
    plot->needsWatering = false;
    plot->hasPests = false;
    plot->maturity = std::nullopt;
    MarkDirty();
    return FarmPlotLifecycleResult::Applied;
}

bool TillersFarmSession::IsPlotReadyToHarvest(uint8 plotId) const
{
    FarmPlotData const* plot = nullptr;
    if (GetLifecyclePlot(plotId, plot) != FarmPlotLifecycleResult::Applied)
        return false;

    return plot->state == FarmPlotState::ReadyToHarvest;
}

FarmHarvestResult TillersFarmSession::PrepareHarvest(uint8 plotId, uint8 plumpRoll,
    uint8 seedReturnRoll, uint8 seedReturnCount, FarmHarvestPlan& plan) const
{
    FarmPlotData const* plot = nullptr;
    switch (GetLifecyclePlot(plotId, plot))
    {
        case FarmPlotLifecycleResult::Applied:
            break;
        case FarmPlotLifecycleResult::MissingPlot:
            return FarmHarvestResult::MissingPlot;
        case FarmPlotLifecycleResult::LockedPlot:
            return FarmHarvestResult::LockedPlot;
        case FarmPlotLifecycleResult::InconsistentFarm:
            return FarmHarvestResult::InconsistentFarm;
        case FarmPlotLifecycleResult::Unusable:
            return FarmHarvestResult::Unusable;
        default:
            return FarmHarvestResult::WrongState;
    }

    if (plot->state != FarmPlotState::ReadyToHarvest)
        return FarmHarvestResult::WrongState;
    if (!plot->seedEntry)
        return FarmHarvestResult::MissingSeed;

    switch (BuildHarvestRewardPlan(*plot->seedEntry, plumpRoll, seedReturnRoll, seedReturnCount, plan))
    {
        case FarmHarvestPolicyResult::Ready:
            return FarmHarvestResult::Ready;
        case FarmHarvestPolicyResult::InvalidSeed:
            return FarmHarvestResult::InvalidSeed;
        case FarmHarvestPolicyResult::InvalidPlumpRoll:
            return FarmHarvestResult::InvalidPlumpRoll;
        case FarmHarvestPolicyResult::InvalidSeedReturnRoll:
            return FarmHarvestResult::InvalidSeedReturnRoll;
        case FarmHarvestPolicyResult::InvalidSeedReturnCount:
            return FarmHarvestResult::InvalidSeedReturnCount;
    }

    return FarmHarvestResult::InvalidSeed;
}

FarmHarvestClaimResult TillersFarmSession::BeginHarvestClaim(uint8 plotId, uint8 plumpRoll,
    uint8 seedReturnRoll, uint8 seedReturnCount, FarmHarvestClaim& claim)
{
    if (HasPendingHarvestClaim(plotId))
        return FarmHarvestClaimResult::AlreadyClaimed;

    FarmHarvestPlan plan;
    switch (PrepareHarvest(plotId, plumpRoll, seedReturnRoll, seedReturnCount, plan))
    {
        case FarmHarvestResult::Ready:
            break;
        case FarmHarvestResult::MissingPlot:
            return FarmHarvestClaimResult::MissingPlot;
        case FarmHarvestResult::LockedPlot:
            return FarmHarvestClaimResult::LockedPlot;
        case FarmHarvestResult::InconsistentFarm:
            return FarmHarvestClaimResult::InconsistentFarm;
        case FarmHarvestResult::Unusable:
            return FarmHarvestClaimResult::Unusable;
        case FarmHarvestResult::WrongState:
            return FarmHarvestClaimResult::WrongState;
        case FarmHarvestResult::MissingSeed:
            return FarmHarvestClaimResult::MissingSeed;
        case FarmHarvestResult::InvalidSeed:
            return FarmHarvestClaimResult::InvalidSeed;
        case FarmHarvestResult::InvalidPlumpRoll:
            return FarmHarvestClaimResult::InvalidPlumpRoll;
        case FarmHarvestResult::InvalidSeedReturnRoll:
            return FarmHarvestClaimResult::InvalidSeedReturnRoll;
        case FarmHarvestResult::InvalidSeedReturnCount:
            return FarmHarvestClaimResult::InvalidSeedReturnCount;
    }

    FarmPlotData const* plot = GetPlot(plotId);
    if (!plot)
        return FarmHarvestClaimResult::MissingPlot;

    FarmHarvestClaimId const claimId = _nextHarvestClaimId++;
    if (_nextHarvestClaimId == 0)
        _nextHarvestClaimId = 1;

    FarmHarvestClaim completeClaim { claimId, plotId, plan };
    PendingHarvestClaim pendingClaim { claimId, plotId, plan, *plot };
    _pendingHarvestClaims.emplace(plotId, std::move(pendingClaim));
    claim = completeClaim;
    return FarmHarvestClaimResult::Ready;
}

FarmHarvestCancelResult TillersFarmSession::CancelHarvestClaim(FarmHarvestClaimId claimId)
{
    for (auto itr = _pendingHarvestClaims.begin(); itr != _pendingHarvestClaims.end(); ++itr)
    {
        if (itr->second.claimId == claimId)
        {
            _pendingHarvestClaims.erase(itr);
            return FarmHarvestCancelResult::Cancelled;
        }
    }

    return FarmHarvestCancelResult::NotFound;
}

FarmHarvestFinalizeResult TillersFarmSession::FinalizeHarvestClaim(FarmHarvestClaimId claimId)
{
    auto claimItr = _pendingHarvestClaims.end();
    for (auto itr = _pendingHarvestClaims.begin(); itr != _pendingHarvestClaims.end(); ++itr)
    {
        if (itr->second.claimId == claimId)
        {
            claimItr = itr;
            break;
        }
    }

    if (claimItr == _pendingHarvestClaims.end())
        return FarmHarvestFinalizeResult::NotFound;

    PendingHarvestClaim const& claim = claimItr->second;
    FarmPlotData const* plot = nullptr;
    FarmPlotLifecycleResult const eligibility = GetLifecyclePlot(claim.plotId, plot);
    FarmPlotData const& claimedPlot = claim.claimedPlot;
    bool const stale = eligibility != FarmPlotLifecycleResult::Applied ||
        plot->plotId != claimedPlot.plotId ||
        plot->state != FarmPlotState::ReadyToHarvest ||
        plot->state != claimedPlot.state ||
        plot->seedEntry != claimedPlot.seedEntry ||
        plot->needsWatering != claimedPlot.needsWatering ||
        plot->hasPests != claimedPlot.hasPests ||
        plot->maturity != claimedPlot.maturity;
    if (stale)
    {
        _pendingHarvestClaims.erase(claimItr);
        return FarmHarvestFinalizeResult::StaleClaim;
    }

    FarmPlotLifecycleResult const resetResult = ResetHarvestedPlot(claim.plotId);
    _pendingHarvestClaims.erase(claimItr);
    return resetResult == FarmPlotLifecycleResult::Applied ?
        FarmHarvestFinalizeResult::Finalized : FarmHarvestFinalizeResult::ResetRejected;
}

bool TillersFarmSession::HasPendingHarvestClaim(uint8 plotId) const
{
    return _pendingHarvestClaims.count(plotId) != 0;
}

FarmPlantingResult TillersFarmSession::PlantCrop(uint8 plotId, uint32 seedEntry,
    FarmPlantingOutcome outcome, std::optional<time_t> maturity)
{
    FarmPlotData* plot = nullptr;
    switch (GetLifecyclePlot(plotId, plot))
    {
        case FarmPlotLifecycleResult::Applied:
            break;
        case FarmPlotLifecycleResult::MissingPlot:
            return FarmPlantingResult::MissingPlot;
        case FarmPlotLifecycleResult::LockedPlot:
            return FarmPlantingResult::LockedPlot;
        case FarmPlotLifecycleResult::InconsistentFarm:
            return FarmPlantingResult::InconsistentFarm;
        case FarmPlotLifecycleResult::Unusable:
            return FarmPlantingResult::Unusable;
        default:
            return FarmPlantingResult::WrongState;
    }

    if (plot->state != FarmPlotState::SoilPrepared)
        return FarmPlantingResult::WrongState;
    if (seedEntry == 0)
        return FarmPlantingResult::InvalidSeed;

    bool const timedOutcome = outcome != FarmPlantingOutcome::ReadyToHarvest;
    if ((timedOutcome && (!maturity || *maturity <= 0 || !FarmDataValidation::IsValidMaturity(*maturity))) ||
        (!timedOutcome && maturity))
        return FarmPlantingResult::InvalidMaturity;

    switch (outcome)
    {
        case FarmPlantingOutcome::Seeded:
            plot->state = FarmPlotState::Seeded;
            plot->needsWatering = false;
            plot->hasPests = false;
            break;
        case FarmPlantingOutcome::NeedsWater:
            plot->state = FarmPlotState::NeedsWater;
            plot->needsWatering = true;
            plot->hasPests = false;
            break;
        case FarmPlantingOutcome::NeedsPestControl:
            plot->state = FarmPlotState::NeedsPestControl;
            plot->needsWatering = false;
            plot->hasPests = true;
            break;
        case FarmPlantingOutcome::ReadyToHarvest:
            plot->state = FarmPlotState::ReadyToHarvest;
            plot->needsWatering = false;
            plot->hasPests = false;
            break;
        default:
            return FarmPlantingResult::InvalidMaturity;
    }

    plot->seedEntry = seedEntry;
    plot->maturity = maturity;
    MarkDirty();
    return FarmPlantingResult::Applied;
}

FarmPlantingResult TillersFarmSession::PlantCropWithPolicy(uint8 plotId, uint32 seedEntry,
    uint16 burstRoll, uint16 problemRoll, time_t nextReset)
{
    FarmPlantingPlan plan;
    switch (BuildPlantingPlan(burstRoll, problemRoll, nextReset, plan))
    {
        case FarmPlantingPolicyResult::Ready:
            break;
        case FarmPlantingPolicyResult::InvalidBurstRoll:
            return FarmPlantingResult::InvalidBurstRoll;
        case FarmPlantingPolicyResult::InvalidProblemRoll:
            return FarmPlantingResult::InvalidProblemRoll;
        case FarmPlantingPolicyResult::InvalidResetTime:
            return FarmPlantingResult::InvalidResetTime;
    }

    return PlantCrop(plotId, seedEntry, plan.outcome, plan.maturity);
}

bool TillersFarmSession::IsPlotPlantable(uint8 plotId) const
{
    FarmPlotData const* plot = nullptr;
    return GetLifecyclePlot(plotId, plot) == FarmPlotLifecycleResult::Applied &&
        plot->state == FarmPlotState::SoilPrepared;
}

FarmPlotLifecycleResult TillersFarmSession::GetLifecyclePlot(uint8 plotId, FarmPlotData*& plot)
{
    FarmPlotData const* constPlot = nullptr;
    FarmPlotLifecycleResult const result = static_cast<TillersFarmSession const*>(this)->GetLifecyclePlot(plotId, constPlot);
    plot = const_cast<FarmPlotData*>(constPlot);
    return result;
}

FarmPlotLifecycleResult TillersFarmSession::GetLifecyclePlot(uint8 plotId, FarmPlotData const*& plot) const
{
    plot = nullptr;
    if (!IsUsable())
        return FarmPlotLifecycleResult::Unusable;
    if (!IsProgressionConsistent())
        return FarmPlotLifecycleResult::InconsistentFarm;
    if (!FarmDataValidation::IsValidPlotId(plotId) || plotId >= _data.state.plotsUnlocked)
        return FarmPlotLifecycleResult::LockedPlot;

    auto itr = _data.plots.find(plotId);
    if (itr == _data.plots.end())
        return FarmPlotLifecycleResult::MissingPlot;

    plot = &itr->second;
    return FarmPlotLifecycleResult::Applied;
}

FarmPlotData* TillersFarmSession::GetMutablePlot(uint8 plotId)
{
    if (!IsUsable() || !FarmDataValidation::IsValidPlotId(plotId))
        return nullptr;

    auto itr = _data.plots.find(plotId);
    return itr != _data.plots.end() ? &itr->second : nullptr;
}

FarmSaveRequestResult TillersFarmSession::RequestSave()
{
    if (HasPendingSave())
        return FarmSaveRequestResult::AlreadyPending;

    if (!IsUsable())
        return FarmSaveRequestResult::Rejected;

    if (!NeedsPersistence())
        return FarmSaveRequestResult::NoChanges;

    PlayerFarmData const snapshot = _data;
    FarmWriteResult result = TillersFarmPersistence::Save(_ownerGuidLow, snapshot);
    if (!result.accepted || !result.completion)
        return FarmSaveRequestResult::Rejected;

    _pendingSaveRevision = _mutationRevision;
    result.completion->AfterComplete([this, savedRevision = _pendingSaveRevision](bool success)
    {
        HandleSaveCompletion(success, savedRevision);
    });
    _pendingSave.emplace(std::move(*result.completion));
    return FarmSaveRequestResult::Queued;
}

void TillersFarmSession::ProcessPersistence()
{
    if (_pendingSave && _pendingSave->InvokeIfReady())
        _pendingSave.reset();
}

void TillersFarmSession::MarkDirty()
{
    ++_mutationRevision;
    _dirty = true;
}

void TillersFarmSession::HandleSaveCompletion(bool success, uint64 savedRevision)
{
    _lastSaveResult = success ? FarmSaveCompletionResult::Success : FarmSaveCompletionResult::Failure;
    if (!success)
        return;

    if (_data.loadStatus == FarmLoadStatus::NotPersisted)
        _data.loadStatus = FarmLoadStatus::Persisted;

    if (_mutationRevision == savedRevision)
        _dirty = false;
}
}
