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
