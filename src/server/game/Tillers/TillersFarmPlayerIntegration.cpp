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
#include <algorithm>
#include <array>
#include <cstdlib>
#include <limits>
#include <memory>
#include <vector>

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

    std::vector<std::unique_ptr<Item>> rewardItems;
    for (uint8 i = 0; i < rewardCount; ++i)
    {
        InventoryResult ownershipResult = player.CanTakeMoreSimilarItems(rewards[i].itemEntry, rewards[i].count);
        if (ownershipResult != EQUIP_ERR_OK)
        {
            session->CancelHarvestClaim(claim.claimId);
            player.SendEquipError(ownershipResult, nullptr, nullptr);
            return FarmPlayerHarvestResult::InventoryRejected;
        }

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(rewards[i].itemEntry);
        if (!itemTemplate || rewards[i].count == 0 || itemTemplate->GetMaxStackSize() == 0)
        {
            session->CancelHarvestClaim(claim.claimId);
            return FarmPlayerHarvestResult::InventoryRejected;
        }

        uint32 remaining = rewards[i].count;
        while (remaining != 0)
        {
            uint32 stackCount = std::min(remaining, itemTemplate->GetMaxStackSize());
            std::unique_ptr<Item> item(Item::CreateItem(rewards[i].itemEntry, stackCount, &player));
            if (!item)
            {
                session->CancelHarvestClaim(claim.claimId);
                return FarmPlayerHarvestResult::InventoryRejected;
            }

            if (uint32 randomPropertyId = Item::GenerateItemRandomPropertyId(rewards[i].itemEntry))
                item->SetItemRandomProperties(randomPropertyId);
            rewardItems.push_back(std::move(item));
            remaining -= stackCount;
        }
    }

    if (rewardItems.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
        session->CancelHarvestClaim(claim.claimId);
        return FarmPlayerHarvestResult::InventoryRejected;
    }

    std::vector<Item*> preflightItems;
    preflightItems.reserve(rewardItems.size());
    for (std::unique_ptr<Item> const& item : rewardItems)
        preflightItems.push_back(item.get());

    InventoryResult inventoryResult = player.CanStoreItems(preflightItems.data(), static_cast<int>(preflightItems.size()));
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

    CharacterDatabaseTransaction transaction = CharacterDatabase.BeginTransaction();
    uint64 savedRevision = 0;
    if (session->AppendCurrentStateToTransaction(transaction, savedRevision) != FarmTransactionSaveResult::Appended)
    {
        TC_LOG_ERROR("sql.sql", "Tillers harvest failed to append farm snapshot for player %u", player.GetGUID().GetCounter());
        return FarmPlayerHarvestResult::PersistenceRejected;
    }

    for (std::unique_ptr<Item>& rewardItem : rewardItems)
    {
        ItemPosCountVec destination;
        InventoryResult storeResult = player.CanStoreItem(NULL_BAG, NULL_SLOT, destination, rewardItem.get(), false);
        if (storeResult != EQUIP_ERR_OK)
        {
            TC_LOG_ERROR("entities.player.items", "Tillers harvest reward storage invariant failed for player %u, item %u count %u, error %u",
                player.GetGUID().GetCounter(), rewardItem->GetEntry(), rewardItem->GetCount(), storeResult);
            return FarmPlayerHarvestResult::RewardDeliveryFailed;
        }

        uint32 storedCount = rewardItem->GetCount();
        Item* storedItem = player.StoreItem(destination, rewardItem.release(), true);
        if (!storedItem)
        {
            TC_LOG_ERROR("entities.player.items", "Tillers harvest reward insertion failed for player %u", player.GetGUID().GetCounter());
            return FarmPlayerHarvestResult::RewardDeliveryFailed;
        }
        player.SendNewItem(storedItem, storedCount, true, false);
    }

    player.SaveInventoryAndGoldToDB(transaction);
    if (!CharacterDatabase.DirectCommitTransaction(transaction))
    {
        TC_LOG_FATAL("sql.sql", "Tillers harvest transaction failed for player %u; terminating to preserve inventory persistence bookkeeping",
            player.GetGUID().GetCounter());
        std::abort();
    }
    session->CompleteTransactionSave(true, savedRevision);
    return FarmPlayerHarvestResult::Applied;
}
}
