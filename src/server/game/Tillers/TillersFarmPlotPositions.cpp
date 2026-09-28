#include "TillersFarmPlotPositions.h"

#include "DatabaseEnv.h"
#include "Log.h"

#include <cmath>
#include <set>

namespace Tillers
{
FarmPlotPositions const* GetFarmPlotPositions()
{
    static FarmPlotPositions positions;
    static bool const valid = []
    {
        QueryResult result = WorldDatabase.Query(
            "SELECT guid, position_x, position_y, position_z, orientation FROM creature "
            "WHERE map = 870 AND id = 55626 AND position_z BETWEEN 164 AND 166 "
            "ORDER BY position_x, position_y, guid");
        if (!result || result->GetRowCount() != FarmPlotPositionCount)
        {
            TC_LOG_ERROR("server.loading", "Tillers soil: expected exactly 16 reference creatures, found " UI64FMTD,
                result ? result->GetRowCount() : uint64(0));
            return false;
        }

        std::set<uint32> guids;
        uint8 index = 0;
        do
        {
            Field* fields = result->Fetch();
            uint32 guid = fields[0].GetUInt32();
            float x = fields[1].GetFloat();
            float y = fields[2].GetFloat();
            float z = fields[3].GetFloat();
            float orientation = fields[4].GetFloat();
            if (!guids.insert(guid).second || !std::isfinite(x) || !std::isfinite(y) ||
                !std::isfinite(z) || !std::isfinite(orientation))
            {
                TC_LOG_ERROR("server.loading", "Tillers soil: invalid or duplicate reference creature GUID %u", guid);
                return false;
            }

            for (uint8 previous = 0; previous < index; ++previous)
                if (positions[previous].GetExactDistSq(x, y, z) < 0.0001f)
                {
                    TC_LOG_ERROR("server.loading", "Tillers soil: reference creature GUID %u has an ambiguous duplicate position", guid);
                    return false;
                }

            // This historical X/Y/GUID order defines persisted logical plot IDs; changing it remaps crops.
            positions[index++].Relocate(x, y, z, orientation);
        } while (result->NextRow());

        return index == FarmPlotPositionCount;
    }();

    return valid ? &positions : nullptr;
}
}
