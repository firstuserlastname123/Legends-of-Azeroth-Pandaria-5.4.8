/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#ifndef TILLERS_FARM_PERSISTENCE_H
#define TILLERS_FARM_PERSISTENCE_H

#include "Define.h"
#include "Transaction.h"
#include <ctime>
#include <limits>
#include <map>
#include <optional>

namespace Tillers
{
constexpr std::size_t MaxFarmPlots = 16;

enum class FarmState : uint8
{
    Cleared = 0,
    WagonRemaining = 8,
    WeedsAndWagonRemaining = 12,
    Initial = 14
};

enum class FarmPlotState : uint8
{
    State0 = 0,
    State1 = 1,
    State2 = 2,
    State3 = 3,
    State4 = 4,
    State5 = 5,
    State6 = 6,
    State7 = 7
};

struct PlayerFarmState
{
    FarmState farmPhase = FarmState::Initial;
    uint8 plotsUnlocked = 4;
    uint16 bestFriendUnlocks = 0;
};

struct FarmPlotData
{
    uint8 plotId = 0;
    FarmPlotState state = FarmPlotState::State0;
    std::optional<uint32> seedEntry;
    bool needsWatering = false;
    bool hasPests = false;
    std::optional<time_t> maturity;
};

enum class FarmLoadStatus
{
    NotPersisted,
    Persisted,
    InvalidRoot
};

namespace FarmDataValidation
{
constexpr bool IsValidFarmState(FarmState state)
{
    switch (state)
    {
        case FarmState::Cleared:
        case FarmState::WagonRemaining:
        case FarmState::WeedsAndWagonRemaining:
        case FarmState::Initial:
            return true;
        default:
            return false;
    }
}

constexpr bool IsValidPlotCount(uint8 count)
{
    return count == 4 || count == 8 || count == 12 || count == 16;
}

constexpr bool IsValidPlotId(uint8 plotId)
{
    return plotId < MaxFarmPlots;
}

constexpr bool IsValidPlotState(FarmPlotState state)
{
    return static_cast<uint8>(state) <= static_cast<uint8>(FarmPlotState::State7);
}

constexpr bool IsValidMaturity(time_t maturity)
{
    if constexpr (std::numeric_limits<time_t>::is_signed)
        if (maturity < 0)
            return false;

    return static_cast<uintmax_t>(maturity) <= std::numeric_limits<uint32>::max();
}
}

struct PlayerFarmData
{
    FarmLoadStatus loadStatus = FarmLoadStatus::NotPersisted;
    PlayerFarmState state;
    std::map<uint8, FarmPlotData> plots;
};

struct FarmWriteResult
{
    bool accepted = false;
    std::optional<TransactionCallback> completion;
};

class TillersFarmPersistence
{
public:
    static PlayerFarmData Load(uint32 guidLow);
    static FarmWriteResult Save(uint32 guidLow, PlayerFarmData const& data);
    static FarmWriteResult Reset(uint32 guidLow);
};
}

#endif
