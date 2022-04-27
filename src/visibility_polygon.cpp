#include "olcPixelGameEngine.h"
#include "visibility_polygon.h"

std::vector<olc::vf2d> VisibilityPolygon(const olc::vf2d& vMP_W, const olc::vf2d& vTL_W, const olc::vf2d& vTR_W, const olc::vf2d& vBR_W, const olc::vf2d& vBL_W,
	const std::vector<olc::vf2d>& nodes, const std::vector<std::array<int, 2>>& segments, const std::vector<olc::vf2d>& intersections)
{
	// Add screen edges to the list of nodes
	std::vector<olc::vf2d> nodes_edg;
	nodes_edg.reserve(4);
	nodes_edg.push_back(vTL_W);
	nodes_edg.push_back(vTR_W);
	nodes_edg.push_back(vBR_W);
	nodes_edg.push_back(vBL_W);

	// Add screen edges to the list of segments
	std::vector<std::array<int, 2>> segments_edg;
	segments_edg.reserve(4);
	segments_edg.push_back({ 0,1 });
	segments_edg.push_back({ 1,2 });
	segments_edg.push_back({ 2,3 });
	segments_edg.push_back({ 3,0 });

	// Find intersections of line segments with screen edges
	std::vector<olc::vf2d> intersections_edg;
	intersections_edg.reserve(16);
	for (int i = 0; i < 4; i++)
	{
		olc::vf2d vIntersectionPoint;
		for (int j = 0; j < segments.size(); j++)
		{
			// Check for intersection
			if (SegmentToSegmentIntersection(nodes_edg[segments_edg[i][0]], nodes_edg[segments_edg[i][1]],
				nodes[segments[j][0]], nodes[segments[j][1]], &vIntersectionPoint))
			{
				intersections_edg.push_back(vIntersectionPoint);
			}
		}
	}

	// Create a list of rays all rays
	std::vector<olc::vf2d> rays_all;
	rays_all.reserve(4 + intersections_edg.size() + 3 * nodes.size() + 3 * intersections.size());
	for (int i = 0; i < 4; i++)
	{
		rays_all.push_back(nodes_edg[i]);
	}
	for (int i = 0; i < intersections_edg.size(); i++)
	{
		rays_all.push_back(intersections_edg[i]);
	}
	for (int i = 0; i < nodes.size(); i++)
	{
		rays_all.push_back(RotatePoint(nodes[i], -0.000001f, vMP_W));
		rays_all.push_back(nodes[i]);
		rays_all.push_back(RotatePoint(nodes[i], 0.000001f, vMP_W));
	}
	for (int i = 0; i < intersections.size(); i++)
	{
		rays_all.push_back(RotatePoint(intersections[i], -0.000001f, vMP_W));
		rays_all.push_back(intersections[i]);
		rays_all.push_back(RotatePoint(intersections[i], 0.000001f, vMP_W));
	}

	// Check if ray endpoints are within screen boundaries
	std::vector<olc::vf2d> rays_active;
	rays_active.reserve(rays_all.size());
	for (int i = 0; i < rays_all.size(); i++)
	{
		olc::vf2d vPoint = rays_all[i];
		if (vPoint.x >= vBL_W.x && vPoint.y >= vBL_W.y && vPoint.x <= vTR_W.x && vPoint.y <= vTR_W.y)
		{
			rays_active.push_back(rays_all[i]);
		}
	}

	// Sort the rays by their angle relative to the mouse pointer
	float angle = 0.0f;
	std::vector<std::pair<olc::vf2d, float>> node_angle_pair;
	node_angle_pair.reserve(rays_active.size());
	for (int i = 0; i < rays_active.size(); i++)
	{
		angle = std::atan2(rays_active[i].y - vMP_W.y, rays_active[i].x - vMP_W.x);
		node_angle_pair.push_back({ rays_active[i], angle });
	}
	std::sort(node_angle_pair.begin(), node_angle_pair.end(),
		[](const std::pair<olc::vf2d, float> a, const std::pair<olc::vf2d, float> b) {return a.second > b.second; });

	// Write the data back to rays
	for (int i = 0; i < rays_active.size(); i++)
	{
		rays_active[i] = node_angle_pair[i].first;
	}

	// Loop over each ray
	std::vector<olc::vf2d> vClosestIntersectionPoints;
	vClosestIntersectionPoints.reserve(rays_active.size());
	for (int i = 0; i < rays_active.size(); i++)
	{
		// Closes intersection
		std::vector<olc::vf2d> vRayIntersections;
		std::vector<float> vRayIntersectionDistances;
		olc::vf2d vIntersectionPoint;

		vRayIntersections.reserve(1000);
		vRayIntersectionDistances.reserve(1000);

		// Loop over each line segment_edg to check for intersection
		for (int j = 0; j < segments_edg.size(); j++)
		{

			if (RayToSegmentIntersection(vMP_W, rays_active[i],
				nodes_edg[segments_edg[j][0]], nodes_edg[segments_edg[j][1]], &vIntersectionPoint))
			{
				vRayIntersectionDistances.push_back(EuclideanDistanceSquared(vMP_W, vIntersectionPoint));
				vRayIntersections.push_back(vIntersectionPoint);
			}
		}
		// Loop over each line segment to check for intersection
		for (int j = 0; j < segments.size(); j++)
		{
			// Check for intersection
			if (RayToSegmentIntersection(vMP_W, rays_active[i], nodes[segments[j][0]], nodes[segments[j][1]], &vIntersectionPoint))
			{
				vRayIntersectionDistances.push_back(EuclideanDistanceSquared(vMP_W, vIntersectionPoint));
				vRayIntersections.push_back(vIntersectionPoint);
			}
		}
		// Find closest intersection
		if (vRayIntersections.size() == 1)
		{
			vClosestIntersectionPoints.push_back(vRayIntersections[0]);
		}
		if (vRayIntersections.size() > 1)
		{
			auto min = std::min_element(vRayIntersectionDistances.begin(), vRayIntersectionDistances.end());
			vClosestIntersectionPoints.push_back(vRayIntersections[min - vRayIntersectionDistances.begin()]);
		}
	}
	return vClosestIntersectionPoints;
}