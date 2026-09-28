#ifndef TILLERS_FARM_PLOT_POSITIONS_H
#define TILLERS_FARM_PLOT_POSITIONS_H

#include "Define.h"
#include "Position.h"

#include <array>

namespace Tillers
{
constexpr uint8 FarmPlotPositionCount = 16;
using FarmPlotPositions = std::array<Position, FarmPlotPositionCount>;

FarmPlotPositions const* GetFarmPlotPositions();
}

#endif
