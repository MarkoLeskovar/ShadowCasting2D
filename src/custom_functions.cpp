#include "olcPixelGameEngine.h"


// Squared euclidean distance between two points
float EuclideanDistanceSquared(const olc::vf2d& vA, const olc::vf2d& vB)
{
	return (vB.x - vA.x) * (vB.x - vA.x) + (vB.y - vA.y) * (vB.y - vA.y);
}

// Euclidean distance between two points
float EuclideanDistance(const olc::vf2d& vA, const olc::vf2d& vB)
{
	return sqrt(EuclideanDistanceSquared(vA, vB));
}

// Rotation around a point
olc::vf2d RotatePoint(const olc::vf2d& vPoint, const float& fAngle, const olc::vf2d& vRotationCenter = { 0.0f, 0.0f })
{
	float cos_A = std::cos(fAngle);
	float sin_A = std::sin(fAngle);
	float x = cos_A * (vPoint.x - vRotationCenter.x) - sin_A * (vPoint.y - vRotationCenter.y) + vRotationCenter.x;
	float y = sin_A * (vPoint.x - vRotationCenter.x) + cos_A * (vPoint.y - vRotationCenter.y) + vRotationCenter.y;
	return olc::vf2d({ x,y });
}

// Squared euclidean distance from a point to a line segment
float EuclideanDistanceToLineSquared(const olc::vf2d& vStart, const olc::vf2d& vEnd, const olc::vf2d& vPoint)
{
	float l2 = EuclideanDistanceSquared(vStart, vEnd);
	if (l2 == 0.0f)
	{
		return EuclideanDistanceSquared(vPoint, vStart);
	}

	float distance = 0;
	float t = (vPoint - vStart).dot(vEnd - vStart) / l2;

	if (t >= 0.0f && t <= 1.0f)
	{
		olc::vf2d vProjection = vStart + t * (vEnd - vStart);
		distance = EuclideanDistanceSquared(vPoint, vProjection);
	}
	else if (t < 0.0f)
	{
		distance = EuclideanDistanceSquared(vPoint, vStart);
	}
	else if (t > 1.0f)
	{
		distance = EuclideanDistanceSquared(vPoint, vEnd);
	}

	return distance;
}

// Euclidean distance from a point to a line segment
float EuclideanDistanceToLine(const olc::vf2d& vStart, const olc::vf2d& vEnd, const olc::vf2d& vPoint)
{
	return sqrt(EuclideanDistanceToLineSquared(vStart, vEnd, vPoint));
}

// Find the closest node to point
int FindClosestNode(const olc::vf2d& vPoint, const std::vector<olc::vf2d>& nodes, float* squaredDistance)
{
	// Initialize a vector of lenghts
	std::vector<float> distances(nodes.size());

	// Loop over all nodes
	for (int i = 0; i < nodes.size(); i++)
	{
		distances[i] = EuclideanDistanceSquared(vPoint, nodes[i]);
	}
	int min_index = std::min_element(distances.begin(), distances.end()) - distances.begin();
	*squaredDistance = distances[min_index];
	return min_index;
}

// Find the closest line segment to point
int FindClosestLineSegment(const olc::vf2d& vPoint, const std::vector<olc::vf2d>& nodes,
	const std::vector<std::array<int, 2>>& segments, float* squaredDistance)
{
	// Initialize a vector of lenghts
	std::vector<float> distances(segments.size());

	// Loop over all nodes
	for (int i = 0; i < segments.size(); i++)
	{
		olc::vf2d vStart = nodes[segments[i][0]];
		olc::vf2d vEnd = nodes[segments[i][1]];

		distances[i] = EuclideanDistanceToLineSquared(vStart, vEnd, vPoint);
	}
	int min_index = std::min_element(distances.begin(), distances.end()) - distances.begin();
	*squaredDistance = distances[min_index];
	return min_index;
}

// Find all the segments that are connected to the selected node
std::vector<int> FindConnectedSegments(const int& i_node, const std::vector<std::array<int, 2>>& segments)
{
	std::vector<int> segment_index;
	segment_index.reserve(segments.size());
	for (int i = 0; i < segments.size(); i++)
	{
		if (i_node == segments[i][0] || i_node == segments[i][1])
		{
			segment_index.push_back(i);
		}
	}
	segment_index.resize(segment_index.size());
	return segment_index;
}

// Check if the segment already exists
bool DoesSegmentExist(const std::array<int, 2>& segment, const std::vector<std::array<int, 2>>& segments)
{
	for (int i = 0; i < segments.size(); i++)
	{
		// If start and end are the same
		if (segment[0] == segments[i][0] && segment[1] == segments[i][1])
		{
			return true;
		}
		// If start and end are the same,in the opposite direction
		if (segment[0] == segments[i][1] && segment[1] == segments[i][0])
		{
			return true;
		}
	}
	return false;
}

// If two line segments intersect, function returns "true" and the intersection point. Otherwise "false" is returned.
bool SegmentToSegmentIntersection(const olc::vf2d& vA_start, const olc::vf2d& vA_end,
	const olc::vf2d& vB_start, const olc::vf2d& vB_end, olc::vf2d* vOutputPoint)
{
	// Assign inputs
	float p0_x = vA_start.x;
	float p0_y = vA_start.y;
	float p1_x = vA_end.x;
	float p1_y = vA_end.y;
	float p2_x = vB_start.x;
	float p2_y = vB_start.y;
	float p3_x = vB_end.x;
	float p3_y = vB_end.y;

	// Create vectors from input points
	float s10_x = p1_x - p0_x;
	float s10_y = p1_y - p0_y;
	float s32_x = p3_x - p2_x;
	float s32_y = p3_y - p2_y;

	// Compute the denominator to check if the points are colinear
	float denom = s10_x * s32_y - s32_x * s10_y;
	if (abs(denom) < 1e-10f) { return false; }

	// Compute parametric length of intersection along each line
	float s = (-s10_y * (p0_x - p2_x) + s10_x * (p0_y - p2_y)) / (-s32_x * s10_y + s10_x * s32_y);
	float t = (s32_x * (p0_y - p2_y) - s32_y * (p0_x - p2_x)) / (-s32_x * s10_y + s10_x * s32_y);

	// Intersection detected
	if (s > 0.0f && s < 1.0f && t > 0.0f && t < 1.0f)
	{
		olc::vf2d vTemp;
		vTemp.x = p0_x + (t * s10_x);
		vTemp.y = p0_y + (t * s10_y);
		*vOutputPoint = vTemp;

		return true;
	}
	// No intersection
	return false;
}

// If two line segments intersect, function returns "true" and the intersection point. Otherwise "false" is returned.
bool RayToSegmentIntersection(const olc::vf2d& vA_start, const olc::vf2d& vA_end,
	const olc::vf2d& vB_start, const olc::vf2d& vB_end, olc::vf2d* vOutputPoint)
{
	// Assign inputs
	float p0_x = vA_start.x;
	float p0_y = vA_start.y;
	float p1_x = vA_end.x;
	float p1_y = vA_end.y;
	float p2_x = vB_start.x;
	float p2_y = vB_start.y;
	float p3_x = vB_end.x;
	float p3_y = vB_end.y;

	// Create vectors from input points
	float s10_x = p1_x - p0_x;
	float s10_y = p1_y - p0_y;
	float s32_x = p3_x - p2_x;
	float s32_y = p3_y - p2_y;

	// Compute the denominator to check if the points are colinear
	float denom = s10_x * s32_y - s32_x * s10_y;
	if (abs(denom) < 1e-10f) { return false; }

	// Compute parametric length of intersection along each line
	float s = (-s10_y * (p0_x - p2_x) + s10_x * (p0_y - p2_y)) / (-s32_x * s10_y + s10_x * s32_y);
	float t = (s32_x * (p0_y - p2_y) - s32_y * (p0_x - p2_x)) / (-s32_x * s10_y + s10_x * s32_y);

	// Intersection detected
	if (s >= 0.0f && s <= 1.0f && t >= 0.0f)
	{
		olc::vf2d vTemp;
		vTemp.x = p0_x + (t * s10_x);
		vTemp.y = p0_y + (t * s10_y);
		*vOutputPoint = vTemp;

		return true;
	}
	// No intersection
	return false;
}