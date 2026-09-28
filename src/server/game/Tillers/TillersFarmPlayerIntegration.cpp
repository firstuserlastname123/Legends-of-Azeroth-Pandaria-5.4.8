/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#include "TillersFarmPlayerIntegration.h"
#include "CharacterDatabase.h"
#include "Item.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "TillersFarmSession.h"
#include <array>
#include <limits>
#include <memory>

namespace Tillers
{
namespace
{
struct RewardStack
{
    uint32 itemEntry = 0;
    uint32 count = 0;
};

FarmPlayerHarvestResult TranslateClaimResult(FarmHarvestClaimResult result)
{
    switch (result)
    {
        case FarmHarvestClaimResult::AlreadyClaimed: return FarmPlayerHarvestResult::AlreadyClaimed;
        case FarmHarvestClaimResult::MissingPlot: return FarmPlayerHarvestResult::MissingPlot;
        case FarmHarvestClaimResult::LockedPlot: return FarmPlayerHarvestResult::LockedPlot;
        case FarmHarvestClaimResult::InconsistentFarm: return FarmPlayerHarvestResult::InconsistentFarm;
        case FarmHarvestClaimResult::Unusable: return FarmPlayerHarvestResult::Unusable;
        case FarmHarvestClaimResult::WrongState: return FarmPlayerHarvestResult::WrongState;
        case FarmHarvestClaimResult::MissingSeed: return FarmPlayerHarvestResult::MissingSeed;
        case FarmHarvestClaimResult::InvalidSeed: return FarmPlayerHarvestResult::InvalidSeed;
        case FarmHarvestClaimResult::InvalidPlumpRoll: return FarmPlayerHarvestResult::InvalidPlumpRoll;
        case FarmHarvestClaimResult::InvalidSeedReturnRoll: return FarmPlayerHarvestResult::InvalidSeedReturnRoll;
        case FarmHarvestClaimResult::InvalidSeedReturnCount: return FarmPlayerHarvestResult::InvalidSeedReturnCount;
        case FarmHarvestClaimResult::Ready: break;
    }
    return FarmPlayerHarvestResult::FinalizeRejected;
}
}

FarmPlayerHarvestResult ExecutePlayerHarvest(Player& player, uint8 plotId, uint8 plumpRoll,
    uint8 seedReturnRoll, uint8 seedReturnCount)
{
    TillersFarmSession* session = player.GetTillersFarmSession();
    if (!session)
        return FarmPlayerHarvestResult::NoFarmSession;
    if (session->HasPendingSave())
        return FarmPlayerHarvestResult::PersistenceBusy;

    FarmHarvestClaim claim;
    FarmHarvestClaimResult claimResult = session->BeginHarvestClaim(plotId, plumpRoll, seedReturnRoll,
        seedReturnCount, claim);
    if (claimResult != FarmHarvestClaimResult::Ready)
        return TranslateClaimResult(claimResult);

    std::array<RewardStack, 2> rewards{};
    uint8 rewardCount = 1;
    rewards[0] = { claim.plan.primaryItemEntry, claim.plan.primaryItemCount };
    if (claim.plan.returnedSeedCount != 0)
    {
        if (claim.plan.plantedSeedEntry == rewards[0].itemEntry)
        {
            if (rewards[0].count > std::numeric_limits<uint32>::max() - claim.plan.returnedSeedCount)
            {
                session->CancelHarvestClaim(claim.claimId);
                return FarmPlayerHarvestResult::InventoryRejected;
            }
            rewards[0].count += claim.plan.returnedSeedCount;
        }
        else
            rewards[rewardCount++] = { claim.plan.plantedSeedEntry, claim.plan.returnedSeedCount };
    }

    std::array<std::unique_ptr<Item>, 2> preflightItems;
    std::array<Item*, 2> preflightPointers{};
    for (uint8 i = 0; i < rewardCount; ++i)
    {
        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(rewards[i].itemEntry);
        if (!itemTemplate || rewards[i].count == 0 || rewards[i].count > itemTemplate->GetMaxStackSize())
        {
            session->CancelHarvestClaim(claim.claimId);
            return FarmPlayerHarvestResult::InventoryRejected;
        }

        preflightItems[i].reset(Item::CreateItem(rewards[i].itemEntry, rewards[i].count, &player, true));
        if (!preflightItems[i])
        {
            session->CancelHarvestClaim(claim.claimId);
            return FarmPlayerHarvestResult::InventoryRejected;
        }
        preflightPointers[i] = preflightItems[i].get();
    }

    InventoryResult inventoryResult = player.CanStoreItems(preflightPointers.data(), rewardCount);
    if (inventoryResult != EQUIP_ERR_OK)
    {
        session->CancelHarvestClaim(claim.claimId);
        player.SendEquipError(inventoryResult, nullptr, nullptr);
        return FarmPlayerHarvestResult::InventoryRejected;
    }

    if (session->FinalizeHarvestClaim(claim.claimId) != FarmHarvestFinalizeResult::Finalized)
    {
        TC_LOG_ERROR("entities.player.items", "Tillers harvest claim " UI64FMTD " failed finalization for player %u",
            claim.claimId, player.GetGUID().GetCounter());
        return FarmPlayerHarvestResult::FinalizeRejected;
    }

    for (uint8 i = 0; i < rewardCount; ++i)
    {
        ItemPosCountVec destination;
        if (player.CanStoreNewItem(NULL_BAG, NULL_SLOT, destination, rewards[i].itemEntry, rewards[i].count) != EQUIP_ERR_OK)
        {
            TC_LOG_ERROR("entities.player.items", "Tillers harvest reward storage invariant failed for player %u, item %u count %u",
                player.GetGUID().GetCounter(), rewards[i].itemEntry, rewards[i].count);
            return FarmPlayerHarvestResult::RewardDeliveryFailed;
        }

        Item* item = player.StoreNewItem(destination, rewards[i].itemEntry, true,
            Item::GenerateItemRandomPropertyId(rewards[i].itemEntry));
        if (!item)
        {
            TC_LOG_ERROR("entities.player.items", "Tillers harvest reward creation failed for player %u, item %u count %u",
                player.GetGUID().GetCounter(), rewards[i].itemEntry, rewards[i].count);
            return FarmPlayerHarvestResult::RewardDeliveryFailed;
        }
        player.SendNewItem(item, rewards[i].count, true, false);
    }

    CharacterDatabaseTransaction transaction = CharacterDatabase.BeginTransaction();
    player.SaveInventoryAndGoldToDB(transaction);
    uint64 savedRevision = 0;
    if (session->AppendCurrentStateToTransaction(transaction, savedRevision) != FarmTransactionSaveResult::Appended)
    {
        TC_LOG_ERROR("sql.sql", "Tillers harvest failed to append farm snapshot for player %u", player.GetGUID().GetCounter());
        return FarmPlayerHarvestResult::PersistenceRejected;
    }

    CharacterDatabase.CommitTransaction(transaction);
    session->CompleteTransactionSave(true, savedRevision);
    return FarmPlayerHarvestResult::Applied;
}
}
