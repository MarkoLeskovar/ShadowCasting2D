#ifndef CUSTOM_FUNCTIONS_H
#define CUSTOM_FUNCTIONS_H

#include "olcPixelGameEngine.h"


// Squared euclidean distance between two points
float EuclideanDistanceSquared(const olc::vf2d& vA, const olc::vf2d& vB);

// Euclidean distance between two points
float EuclideanDistance(const olc::vf2d& vA, const olc::vf2d& vB);

// Rotation around a point
olc::vf2d RotatePoint(const olc::vf2d& vPoint, const float& fAngle, const olc::vf2d& vRotationCenter = { 0.0f, 0.0f });

// Squared euclidean distance from a point to a line segment
float EuclideanDistanceToLineSquared(const olc::vf2d& vStart, const olc::vf2d& vEnd, const olc::vf2d& vPoint);

// Euclidean distance from a point to a line segment
float EuclideanDistanceToLine(const olc::vf2d& vStart, const olc::vf2d& vEnd, const olc::vf2d& vPoint);

// Find the closest node to point
int FindClosestNode(const olc::vf2d& vPoint, const std::vector<olc::vf2d>& nodes, float* squaredDistance);

// Find the closest line segment to point
int FindClosestLineSegment(const olc::vf2d& vPoint, const std::vector<olc::vf2d>& nodes,
	                       const std::vector<std::array<int, 2>>& segments, float* squaredDistance);

// Find all the segments that are connected to the selected node
std::vector<int> FindConnectedSegments(const int& i_node, const std::vector<std::array<int, 2>>& segments);

// Check if the segment already exists
bool DoesSegmentExist(const std::array<int, 2>& segment, const std::vector<std::array<int, 2>>& segments);

// If two line segments intersect, function returns "true" and the intersection point. Otherwise "false" is returned.
bool SegmentToSegmentIntersection(const olc::vf2d& vA_start, const olc::vf2d& vA_end,
	                              const olc::vf2d& vB_start, const olc::vf2d& vB_end, olc::vf2d* vOutputPoint);

// If two line segments intersect, function returns "true" and the intersection point. Otherwise "false" is returned.
bool RayToSegmentIntersection(const olc::vf2d& vA_start, const olc::vf2d& vA_end,
                              const olc::vf2d& vB_start, const olc::vf2d& vB_end, olc::vf2d* vOutputPoint);


#endif // CUSTOM_FUNCTIONS_H