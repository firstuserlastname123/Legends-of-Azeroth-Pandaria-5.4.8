#include "TillersFarmSoilPresentation.h"

#include "GameObject.h"
#include "Log.h"
#include "Map.h"
#include "Player.h"
#include "TillersFarmPlotPositions.h"
#include "TillersFarmSession.h"

#include <vector>

namespace Tillers
{
namespace
{
constexpr uint32 FarmMap = 870;
constexpr uint32 FarmZone = 1023;
constexpr uint32 SoilEntry = 186314;

bool HasCompleteSoilPresentation(Player const& player, TillersFarmSession const& session)
{
    uint8 plotsUnlocked = session.GetFarmState().plotsUnlocked;
    if (session.GetSoilBindings().size() != plotsUnlocked)
        return false;

    std::vector<bool> resolvedPlots(plotsUnlocked, false);
    for (auto const& binding : session.GetSoilBindings())
    {
        GameObject* soil = player.GetMap()->GetGameObject(binding.first);
        if (!soil || binding.second >= plotsUnlocked || resolvedPlots[binding.second])
            return false;

        uint8 resolvedPlot;
        if (ResolvePlayerSoilObject(player, *soil, resolvedPlot) != FarmSoilResolveResult::Resolved ||
            resolvedPlot != binding.second)
            return false;

        resolvedPlots[resolvedPlot] = true;
    }

    for (bool resolved : resolvedPlots)
        if (!resolved)
            return false;

    return true;
}
}

FarmSoilResolveResult ResolvePlayerSoilObject(Player const& player, GameObject const& soil, uint8& plotId)
{
    if (!player.IsInWorld())
        return FarmSoilResolveResult::InvalidPlayer;
    if (!soil.IsInWorld() || soil.GetEntry() != SoilEntry)
        return FarmSoilResolveResult::InvalidObject;
    if (player.GetMapId() != FarmMap || player.GetZoneId() != FarmZone || soil.GetMap() != player.GetMap())
        return FarmSoilResolveResult::WrongContext;
    if (soil.GetOwnerGUID() != player.GetGUID() || !soil.IsPrivateObject() ||
        soil.GetPrivateObjectOwner() != player.GetGUID())
        return FarmSoilResolveResult::WrongOwner;

    TillersFarmSession const* session = player.GetTillersFarmSession();
    if (!session || !session->IsUsable())
        return FarmSoilResolveResult::UnusableSession;

    uint8 resolvedPlot;
    if (!session->ResolveSoilObject(soil.GetGUID(), resolvedPlot))
        return FarmSoilResolveResult::UnregisteredOrIneligible;

    plotId = resolvedPlot;
    return FarmSoilResolveResult::Resolved;
}

bool PresentPlayerSoil(Player& player)
{
    TillersFarmSession* session = player.GetTillersFarmSession();
    if (!player.IsInWorld() || player.GetMapId() != FarmMap || player.GetZoneId() != FarmZone ||
        !session || !session->IsUsable() || !session->IsProgressionConsistent())
        return false;
    if (HasCompleteSoilPresentation(player, *session))
        return true;

    if (!session->GetSoilBindings().empty())
        RemovePlayerSoil(player, true);

    FarmPlotPositions const* positions = GetFarmPlotPositions();
    if (!positions)
        return false;

    Map* map = player.GetMap();
    std::vector<GameObject*> created;
    for (uint8 plotId = 0; plotId < session->GetFarmState().plotsUnlocked; ++plotId)
    {
        if (!session->IsPlotUnlocked(plotId) || !session->GetPlot(plotId))
            goto fail;

        Position const& position = (*positions)[plotId];
        GameObject* soil = new GameObject();
        if (!soil->Create(map->GenerateLowGuid<HighGuid::GameObject>(), SoilEntry, map,
            player.GetPhaseMgr().GetPhaseMaskForSpawn(), position.GetPositionX(), position.GetPositionY(),
            position.GetPositionZ(), position.GetOrientation(), { }, 255, GO_STATE_READY))
        {
            delete soil;
            goto fail;
        }

        player.AddGameObject(soil);
        soil->SetPrivateObjectOwner(player.GetGUID());
        if (!map->AddToMap(soil))
        {
            player.RemoveGameObject(soil, false);
            delete soil;
            goto fail;
        }
        if (!session->RegisterSoilObject(plotId, soil->GetGUID()))
        {
            player.RemoveGameObject(soil, true);
            goto fail;
        }
        created.push_back(soil);
    }
    return true;

fail:
    TC_LOG_ERROR("entities.player", "Tillers soil: failed presentation for player %u; rolling back", player.GetGUID().GetCounter());
    for (GameObject* soil : created)
        player.RemoveGameObject(soil, true);
    session->ClearSoilBindings();
    return false;
}

void RemovePlayerSoil(Player& player, bool removeObjects)
{
    TillersFarmSession* session = player.GetTillersFarmSession();
    if (!session)
        return;

    if (removeObjects && player.IsInWorld())
    {
        std::vector<ObjectGuid> guids;
        for (auto const& binding : session->GetSoilBindings())
            guids.push_back(binding.first);
        for (ObjectGuid guid : guids)
            if (GameObject* soil = player.GetMap()->GetGameObject(guid))
                if (soil->GetOwnerGUID() == player.GetGUID())
                    player.RemoveGameObject(soil, true);
    }
    session->ClearSoilBindings();
}
}
