/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#include "ScriptMgr.h"
#include "AccountMgr.h"
#include "Chat.h"
#include "Player.h"
#include "TillersFarmGameplay.h"
#include <charconv>
#include <string_view>

namespace
{
bool ParsePlotId(char const* args, uint8& plotId)
{
    std::string_view input(args ? args : "");
    while (!input.empty() && (input.front() == ' ' || input.front() == '\t'))
        input.remove_prefix(1);

    if (input.empty())
        return false;

    uint32 parsedPlotId = 0;
    char const* begin = input.data();
    char const* end = begin + input.size();
    std::from_chars_result result = std::from_chars(begin, end, parsedPlotId);
    if (result.ec != std::errc() || result.ptr == begin || parsedPlotId > 15)
        return false;

    for (char const* current = result.ptr; current != end; ++current)
        if (*current != ' ' && *current != '\t')
            return false;

    plotId = static_cast<uint8>(parsedPlotId);
    return true;
}

char const* GetHarvestResultMessage(Tillers::FarmPlayerHarvestResult result)
{
    switch (result)
    {
        case Tillers::FarmPlayerHarvestResult::Applied: return "Harvest completed";
        case Tillers::FarmPlayerHarvestResult::NoFarmSession: return "Farm session unavailable";
        case Tillers::FarmPlayerHarvestResult::PersistenceBusy: return "Previous farm save still pending";
        case Tillers::FarmPlayerHarvestResult::AlreadyClaimed: return "Plot already has a harvest claim";
        case Tillers::FarmPlayerHarvestResult::MissingPlot: return "Plot record does not exist";
        case Tillers::FarmPlayerHarvestResult::LockedPlot: return "Plot is not unlocked";
        case Tillers::FarmPlayerHarvestResult::InconsistentFarm: return "Farm progression state is inconsistent";
        case Tillers::FarmPlayerHarvestResult::Unusable: return "Farm session is invalid";
        case Tillers::FarmPlayerHarvestResult::WrongState: return "Plot is not ready to harvest";
        case Tillers::FarmPlayerHarvestResult::MissingSeed: return "Ready plot has no recorded seed";
        case Tillers::FarmPlayerHarvestResult::InvalidSeed: return "Recorded seed is invalid";
        case Tillers::FarmPlayerHarvestResult::InvalidPlumpRoll: return "Generated plump roll is invalid";
        case Tillers::FarmPlayerHarvestResult::InvalidSeedReturnRoll: return "Generated seed-return roll is invalid";
        case Tillers::FarmPlayerHarvestResult::InvalidSeedReturnCount: return "Generated seed-return count is invalid";
        case Tillers::FarmPlayerHarvestResult::InventoryRejected: return "Inventory cannot accept the complete reward";
        case Tillers::FarmPlayerHarvestResult::FinalizeRejected: return "Harvest finalization was rejected";
        case Tillers::FarmPlayerHarvestResult::PersistenceRejected: return "Farm persistence preparation was rejected";
        case Tillers::FarmPlayerHarvestResult::RewardDeliveryFailed: return "Reward insertion failed unexpectedly";
    }

    return "Harvest failed with an unknown result";
}
}

class tillers_commandscript : public CommandScript
{
public:
    tillers_commandscript() : CommandScript("tillers_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> tillersCommandTable =
        {
            { "harvest", &HandleHarvestCommand, rbac::RBAC_PERM_COMMAND_GM, Trinity::ChatCommands::Console::No },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "tillers", tillersCommandTable, rbac::RBAC_PERM_COMMAND_GM, Trinity::ChatCommands::Console::No },
        };
        return commandTable;
    }

    static bool HandleHarvestCommand(ChatHandler* handler, char const* args)
    {
        WorldSession* session = handler->GetSession();
        if (!session || session->GetSecurity() < SEC_GAMEMASTER)
        {
            handler->SendSysMessage("This command requires an in-game GM session.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = session->GetPlayer();
        if (!player)
        {
            handler->SendSysMessage("Farm player unavailable.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint8 plotId = 0;
        if (!ParsePlotId(args, plotId))
        {
            handler->SendSysMessage("Usage: .tillers harvest <plotId> (plotId must be an integer from 0 through 15)");
            handler->SetSentErrorMessage(true);
            return false;
        }

        Tillers::FarmPlayerHarvestResult result = Tillers::ExecutePlayerHarvestWithServerRolls(*player, plotId);
        handler->SendSysMessage(GetHarvestResultMessage(result));
        return true;
    }
};

void AddSC_tillers_commandscript()
{
    new tillers_commandscript();
}
