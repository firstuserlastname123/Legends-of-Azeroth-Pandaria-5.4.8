/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#include "TillersFarmPlantingIntegration.h"
#include "CharacterDatabase.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "TillersFarmSession.h"
#include <cstdlib>
#include <ctime>

namespace Tillers
{
namespace
{
FarmPlayerPlantingResult TranslatePlantingResult(FarmPlantingResult result)
{
    switch (result)
    {
        case FarmPlantingResult::Applied: return FarmPlayerPlantingResult::Applied;
        case FarmPlantingResult::MissingPlot: return FarmPlayerPlantingResult::MissingPlot;
        case FarmPlantingResult::LockedPlot: return FarmPlayerPlantingResult::LockedPlot;
        case FarmPlantingResult::InconsistentFarm: return FarmPlayerPlantingResult::InconsistentFarm;
        case FarmPlantingResult::Unusable: return FarmPlayerPlantingResult::Unusable;
        case FarmPlantingResult::WrongState: return FarmPlayerPlantingResult::WrongState;
        case FarmPlantingResult::InvalidSeed: return FarmPlayerPlantingResult::InvalidSeed;
        case FarmPlantingResult::InvalidBurstRoll: return FarmPlayerPlantingResult::InvalidBurstRoll;
        case FarmPlantingResult::InvalidProblemRoll: return FarmPlayerPlantingResult::InvalidProblemRoll;
        case FarmPlantingResult::InvalidResetTime:
        case FarmPlantingResult::InvalidMaturity: return FarmPlayerPlantingResult::InvalidResetTime;
        case FarmPlantingResult::PersistenceBusy: return FarmPlayerPlantingResult::PersistenceBusy;
        case FarmPlantingResult::PersistenceRejected: return FarmPlayerPlantingResult::PersistenceRejected;
    }
    return FarmPlayerPlantingResult::PersistenceRejected;
}
}

FarmPlayerPlantingResult ExecutePlayerPlanting(Player& player, uint8 plotId, uint32 seedEntry,
    uint16 burstRoll, uint16 problemRoll, time_t nextReset)
{
    if (!player.IsInWorld())
        return FarmPlayerPlantingResult::PlayerNotInWorld;

    TillersFarmSession* session = player.GetTillersFarmSession();
    if (!session)
        return FarmPlayerPlantingResult::NoFarmSession;
    if (session->GetOwnerGuidLow() != player.GetGUID().GetCounter())
        return FarmPlayerPlantingResult::OwnerMismatch;
    if (!session->IsUsable())
        return FarmPlayerPlantingResult::Unusable;
    if (!session->IsProgressionConsistent())
        return FarmPlayerPlantingResult::InconsistentFarm;
    if (session->HasPendingSave())
        return FarmPlayerPlantingResult::PersistenceBusy;
    if (session->HasPendingHarvestClaim(plotId))
        return FarmPlayerPlantingResult::HarvestClaimPending;
    if (!GetPreservedHarvestItemForSeed(seedEntry) || !sObjectMgr->GetItemTemplate(seedEntry))
        return FarmPlayerPlantingResult::InvalidSeed;
    if (!player.HasItemCount(seedEntry, 1, false))
        return FarmPlayerPlantingResult::MissingSeedItem;

    FarmPlantingPlan plan;
    switch (BuildPlantingPlan(burstRoll, problemRoll, nextReset, plan))
    {
        case FarmPlantingPolicyResult::InvalidBurstRoll: return FarmPlayerPlantingResult::InvalidBurstRoll;
        case FarmPlantingPolicyResult::InvalidProblemRoll: return FarmPlayerPlantingResult::InvalidProblemRoll;
        case FarmPlantingPolicyResult::InvalidResetTime: return FarmPlayerPlantingResult::InvalidResetTime;
        case FarmPlantingPolicyResult::Ready: break;
    }
    if (plan.outcome != FarmPlantingOutcome::ReadyToHarvest && nextReset <= std::time(nullptr))
        return FarmPlayerPlantingResult::InvalidResetTime;

    CharacterDatabaseTransaction transaction = CharacterDatabase.BeginTransaction();
    uint64 savedRevision = 0;
    FarmPlantingResult const plantingResult = session->PlantCropInTransaction(plotId, seedEntry,
        plan, transaction, savedRevision);
    if (plantingResult != FarmPlantingResult::Applied)
        return TranslatePlantingResult(plantingResult);

    uint32 const countBefore = player.GetItemCount(seedEntry, false);
    player.DestroyItemCount(seedEntry, 1, true);
    uint32 const countAfter = player.GetItemCount(seedEntry, false);
    if (countBefore == 0 || countAfter != countBefore - 1)
    {
        TC_LOG_FATAL("entities.player.items", "Tillers planting seed-consumption invariant failed for player %u, seed %u, before %u, after %u; terminating",
            player.GetGUID().GetCounter(), seedEntry, countBefore, countAfter);
        std::abort();
    }

    player.SaveInventoryAndGoldToDB(transaction);
    if (!CharacterDatabase.DirectCommitTransaction(transaction))
    {
        TC_LOG_FATAL("sql.sql", "Tillers planting transaction failed for player %u; terminating to preserve farm and inventory persistence bookkeeping",
            player.GetGUID().GetCounter());
        std::abort();
    }

    session->CompleteTransactionSave(true, savedRevision);
    return FarmPlayerPlantingResult::Applied;
}
}
