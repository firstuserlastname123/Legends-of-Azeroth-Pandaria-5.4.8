#include "Player.h"
#include "QuestDef.h"
#include "ScriptMgr.h"
#include "TillersFarmSoilPresentation.h"

namespace
{
constexpr uint32 FarmMap = 870;
constexpr uint32 FarmZone = 1023;
constexpr uint32 FarmTutorialQuest = 30256;

bool IsPresentationEligible(Player const* player)
{
    return player && player->GetMapId() == FarmMap && player->GetZoneId() == FarmZone &&
        player->GetQuestStatus(FarmTutorialQuest) == QUEST_STATUS_REWARDED;
}

class tillers_plot_player_script : public PlayerScript
{
public:
    tillers_plot_player_script() : PlayerScript("tillers_plot_player_script") { }

    void OnLogin(Player* player) override
    {
        if (IsPresentationEligible(player))
            Tillers::PresentPlayerSoil(*player);
    }

    void OnUpdateZone(Player* player, uint32 newZone, uint32 /*newArea*/) override
    {
        if (newZone == FarmZone && player->GetMapId() == FarmMap &&
            player->GetQuestStatus(FarmTutorialQuest) == QUEST_STATUS_REWARDED)
            Tillers::PresentPlayerSoil(*player);
        else
            Tillers::RemovePlayerSoil(*player, true);
    }

    void OnQuestRewarded(Player* player, Quest const* quest) override
    {
        if (quest && quest->GetQuestId() == FarmTutorialQuest &&
            player->GetMapId() == FarmMap && player->GetZoneId() == FarmZone)
            Tillers::PresentPlayerSoil(*player);
    }

    void OnMapChanged(Player* player) override
    {
        // This hook runs after transfer; old-map owned objects are handled by core map removal.
        Tillers::RemovePlayerSoil(*player, false);
        if (IsPresentationEligible(player))
            Tillers::PresentPlayerSoil(*player);
    }

    void OnLogout(Player* player) override
    {
        Tillers::RemovePlayerSoil(*player, true);
    }
};
}

void AddSC_tillers_plot_hooks()
{
    new tillers_plot_player_script();
}
