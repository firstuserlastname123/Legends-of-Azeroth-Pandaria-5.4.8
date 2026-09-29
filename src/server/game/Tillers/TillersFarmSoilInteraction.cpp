/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#include "TillersFarmSoilInteraction.h"

#include "GameObject.h"
#include "Player.h"
#include "TillersFarmGameplay.h"
#include "TillersFarmSession.h"
#include "TillersFarmSoilPresentation.h"
#include "WorldSession.h"

namespace Tillers
{
namespace
{
constexpr uint32 SoilEntry = 186314;

void SendHarvestResult(Player& player, FarmPlayerHarvestResult result)
{
    char const* message;
    switch (result)
    {
        case FarmPlayerHarvestResult::Applied:
            message = "Harvest completed.";
            break;
        case FarmPlayerHarvestResult::WrongState:
            message = "This crop is not ready to harvest.";
            break;
        case FarmPlayerHarvestResult::InventoryRejected:
            message = "You do not have enough inventory space to harvest this crop.";
            break;
        case FarmPlayerHarvestResult::PersistenceBusy:
            message = "Farming is temporarily unavailable. Please try again later.";
            break;
        case FarmPlayerHarvestResult::AlreadyClaimed:
            message = "This harvest is already being processed.";
            break;
        case FarmPlayerHarvestResult::MissingPlot:
        case FarmPlayerHarvestResult::LockedPlot:
            message = "This farm plot is unavailable.";
            break;
        case FarmPlayerHarvestResult::NoFarmSession:
        case FarmPlayerHarvestResult::Unusable:
        case FarmPlayerHarvestResult::InconsistentFarm:
            message = "Your farm is currently unavailable.";
            break;
        case FarmPlayerHarvestResult::MissingSeed:
        case FarmPlayerHarvestResult::InvalidSeed:
        case FarmPlayerHarvestResult::InvalidPlumpRoll:
        case FarmPlayerHarvestResult::InvalidSeedReturnRoll:
        case FarmPlayerHarvestResult::InvalidSeedReturnCount:
            message = "This crop cannot be harvested.";
            break;
        case FarmPlayerHarvestResult::FinalizeRejected:
            message = "The harvest could not be completed.";
            break;
        case FarmPlayerHarvestResult::PersistenceRejected:
            message = "The farming operation was rejected. Please try again later.";
            break;
        case FarmPlayerHarvestResult::RewardDeliveryFailed:
            message = "The harvest reward could not be delivered.";
            break;
        default:
            message = "The harvest could not be completed.";
            break;
    }

    player.GetSession()->SendNotification("%s", message);
}
}

bool TryHandleRegisteredSoilUse(Player& player, GameObject& soil)
{
    if (soil.GetEntry() != SoilEntry)
        return false;

    TillersFarmSession* session = player.GetTillersFarmSession();
    if (!session || session->GetSoilBindings().find(soil.GetGUID()) == session->GetSoilBindings().end())
        return false;

    uint8 plotId;
    if (ResolvePlayerSoilObject(player, soil, plotId) != FarmSoilResolveResult::Resolved)
        return true;

    if (!soil.IsInWorld() || !soil.isSpawned() || soil.GetMap() != player.GetMap() ||
        !player.InSamePhase(&soil) || !player.CanSeeOrDetect(&soil) || !soil.IsAtInteractDistance(&player))
        return true;

    FarmPlotData const* plot = session->GetPlot(plotId);
    if (!plot || plot->state != FarmPlotState::ReadyToHarvest)
    {
        player.GetSession()->SendNotification("This crop is not ready to harvest.");
        return true;
    }

    SendHarvestResult(player, ExecutePlayerHarvestWithServerRolls(player, plotId));
    return true;
}
}
