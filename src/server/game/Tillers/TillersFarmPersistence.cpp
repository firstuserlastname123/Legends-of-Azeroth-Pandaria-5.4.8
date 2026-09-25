/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#include "TillersFarmPersistence.h"
#include "CharacterDatabase.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include <limits>

namespace Tillers
{
namespace
{
bool CanRepresentMaturity(uint32 maturity)
{
    return static_cast<uintmax_t>(maturity) <= static_cast<uintmax_t>(std::numeric_limits<time_t>::max());
}

bool ValidateSnapshot(uint32 guidLow, PlayerFarmData const& data)
{
    if (!FarmDataValidation::IsValidFarmState(data.state.farmPhase) || !FarmDataValidation::IsValidPlotCount(data.state.plotsUnlocked) || data.plots.size() > MaxFarmPlots)
    {
        TC_LOG_ERROR("sql.sql", "TillersFarmPersistence: invalid farm snapshot for GUID %u", guidLow);
        return false;
    }

    for (auto const& [key, plot] : data.plots)
    {
        if (!FarmDataValidation::IsValidPlotId(key) || key != plot.plotId || !FarmDataValidation::IsValidPlotState(plot.state) ||
            (plot.maturity && !FarmDataValidation::IsValidMaturity(*plot.maturity)))
        {
            TC_LOG_ERROR("sql.sql", "TillersFarmPersistence: invalid plot snapshot for GUID %u, map key %u, plot ID %u", guidLow, key, plot.plotId);
            return false;
        }
    }

    return true;
}

void AppendState(CharacterDatabaseTransaction const& transaction, uint32 guidLow, PlayerFarmState const& state)
{
    CharacterDatabasePreparedStatement* statement = CharacterDatabase.GetPreparedStatement(CHAR_REP_PLAYER_FARM_STATE);
    statement->setUInt32(0, guidLow);
    statement->setUInt8(1, static_cast<uint8>(state.farmPhase));
    statement->setUInt8(2, state.plotsUnlocked);
    statement->setUInt16(3, state.bestFriendUnlocks);
    transaction->Append(statement);
}

void AppendPlotDelete(CharacterDatabaseTransaction const& transaction, uint32 guidLow)
{
    CharacterDatabasePreparedStatement* statement = CharacterDatabase.GetPreparedStatement(CHAR_DEL_PLAYER_FARM_PLOTS);
    statement->setUInt32(0, guidLow);
    transaction->Append(statement);
}
}

PlayerFarmData TillersFarmPersistence::Load(uint32 guidLow)
{
    PlayerFarmData data;

    CharacterDatabasePreparedStatement* stateStatement = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PLAYER_FARM_STATE);
    stateStatement->setUInt32(0, guidLow);
    PreparedQueryResult stateResult = CharacterDatabase.Query(stateStatement);
    if (!stateResult)
        return data;

    Field* stateFields = stateResult->Fetch();
    FarmState farmPhase = static_cast<FarmState>(stateFields[0].GetUInt8());
    uint8 plotsUnlocked = stateFields[1].GetUInt8();
    if (!FarmDataValidation::IsValidFarmState(farmPhase) || !FarmDataValidation::IsValidPlotCount(plotsUnlocked))
    {
        TC_LOG_ERROR("sql.sql", "TillersFarmPersistence: invalid root row for GUID %u", guidLow);
        data.loadStatus = FarmLoadStatus::InvalidRoot;
        return data;
    }

    data.loadStatus = FarmLoadStatus::Persisted;
    data.state.farmPhase = farmPhase;
    data.state.plotsUnlocked = plotsUnlocked;
    data.state.bestFriendUnlocks = stateFields[2].GetUInt16();

    CharacterDatabasePreparedStatement* plotsStatement = CharacterDatabase.GetPreparedStatement(CHAR_SEL_PLAYER_FARM_PLOTS);
    plotsStatement->setUInt32(0, guidLow);
    if (PreparedQueryResult plotsResult = CharacterDatabase.Query(plotsStatement))
    {
        do
        {
            Field* fields = plotsResult->Fetch();
            uint8 plotId = fields[0].GetUInt8();
            uint8 plotState = fields[1].GetUInt8();
            uint8 watering = fields[3].GetUInt8();
            uint8 pests = fields[4].GetUInt8();
            bool invalid = !FarmDataValidation::IsValidPlotId(plotId) ||
                !FarmDataValidation::IsValidPlotState(static_cast<FarmPlotState>(plotState)) || watering > 1 || pests > 1 ||
                data.plots.size() >= MaxFarmPlots || data.plots.count(plotId) != 0;

            std::optional<uint32> maturityValue;
            if (!fields[5].IsNull())
            {
                maturityValue = fields[5].GetUInt32();
                invalid = invalid || !CanRepresentMaturity(*maturityValue);
            }

            if (invalid)
            {
                TC_LOG_ERROR("sql.sql", "TillersFarmPersistence: invalid plot row for GUID %u, plot ID %u", guidLow, plotId);
                continue;
            }

            FarmPlotData plot;
            plot.plotId = plotId;
            plot.state = static_cast<FarmPlotState>(plotState);
            if (!fields[2].IsNull())
                plot.seedEntry = fields[2].GetUInt32();
            plot.needsWatering = watering != 0;
            plot.hasPests = pests != 0;
            if (maturityValue)
                plot.maturity = static_cast<time_t>(*maturityValue);
            data.plots.emplace(plotId, std::move(plot));
        } while (plotsResult->NextRow());
    }

    return data;
}

FarmWriteResult TillersFarmPersistence::Save(uint32 guidLow, PlayerFarmData const& data)
{
    if (!ValidateSnapshot(guidLow, data))
        return {};

    CharacterDatabaseTransaction transaction = CharacterDatabase.BeginTransaction();
    AppendState(transaction, guidLow, data.state);
    AppendPlotDelete(transaction, guidLow);

    for (auto const& [plotId, plot] : data.plots)
    {
        CharacterDatabasePreparedStatement* statement = CharacterDatabase.GetPreparedStatement(CHAR_INS_PLAYER_FARM_PLOT);
        statement->setUInt32(0, guidLow);
        statement->setUInt8(1, plotId);
        statement->setUInt8(2, static_cast<uint8>(plot.state));
        if (plot.seedEntry)
            statement->setUInt32(3, *plot.seedEntry);
        else
            statement->setNull(3);
        statement->setBool(4, plot.needsWatering);
        statement->setBool(5, plot.hasPests);
        if (plot.maturity)
            statement->setUInt32(6, static_cast<uint32>(*plot.maturity));
        else
            statement->setNull(6);
        transaction->Append(statement);
    }

    FarmWriteResult result;
    result.accepted = true;
    result.completion.emplace(CharacterDatabase.AsyncCommitTransaction(transaction));
    return result;
}

FarmWriteResult TillersFarmPersistence::Reset(uint32 guidLow)
{
    CharacterDatabaseTransaction transaction = CharacterDatabase.BeginTransaction();
    AppendState(transaction, guidLow, PlayerFarmState{});
    AppendPlotDelete(transaction, guidLow);

    FarmWriteResult result;
    result.accepted = true;
    result.completion.emplace(CharacterDatabase.AsyncCommitTransaction(transaction));
    return result;
}
}
