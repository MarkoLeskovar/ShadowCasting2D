#ifndef VISIBILITY_POLYGON_H
#define VISIBILITY_POLYGON_H

#include "olcPixelGameEngine.h"
#include "custom_functions.h"

std::vector<olc::vf2d> VisibilityPolygon(const olc::vf2d& vMP_W, const olc::vf2d& vTL_W, const olc::vf2d& vTR_W, const olc::vf2d& vBR_W, const olc::vf2d& vBL_W,
	const std::vector<olc::vf2d>& nodes, const std::vector<std::array<int, 2>>& segments, const std::vector<olc::vf2d>& intersections);

#endif // VISIBILITY_POLYGON_H