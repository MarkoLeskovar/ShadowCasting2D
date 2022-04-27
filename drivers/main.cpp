// olcPixelGameEngine Tutorial
// https://github.com/OneLoneCoder/olcPixelGameEngine/wiki
//
#define OLC_PGE_APPLICATION

#include "custom_functions.h"
#include "visibility_polygon.h"


// Use "vf2d" and "vi2d" where appropriate
// Add bounding box method.
// Add convex polygon method.
// Change container from a vector to list (if it is better).
// Add posibility to select multiple nodes at once.
// Show node numbering and show line numbering


// Global variables
const int32_t mainScreenWidth = 1000;
const int32_t mainScreenHeight = 500;
const int32_t mainToolbarWidth = 185;
const int32_t mainPixelSize = 2;




class Plane2D : public olc::PixelGameEngine
{
public:
	Plane2D()
	{
		sAppName = "SIMPLE CAD APPLICATION";
	}

private: // Drawing rutine coordinate transformations

	// Transform from SCREEN space to WORLD space
	olc::vf2d s2w(const olc::vf2d &vInputPointWorld)
	{
		return { (vInputPointWorld.x - vShift.x + mainToolbarWidth) / fScale ,
			     (mainScreenHeight + vShift.y - vInputPointWorld.y) / fScale };
	}

	// Transform from WORLD space to SCREEN space
	olc::vf2d w2s(const olc::vf2d &vInputPointWorld)
	{
		return { vShift.x - mainToolbarWidth + vInputPointWorld.x * fScale,
				 mainScreenHeight + vShift.y - vInputPointWorld.y * fScale};
	}


private: // Private variables
	
	// Panning and scaling default variables
	float fScale;
	olc::vf2d vShift;
	float fScaleDefault = 1.0f;
	float fScaleStep = 1.01f;
	const olc::vf2d rShiftDefault = {0.5f * (mainScreenWidth + mainToolbarWidth) + mainToolbarWidth, 
		                            -0.5f * mainScreenHeight};

	// Display information flags
	bool bDisplayCoordinateSystem = true;
	bool bDisplayLineSegmentInfo = false;
	bool bDisplaySelfIntersections = false;

	// Other flags
	bool bIsSelected = false;
	int32_t nSelectionSize = 10;
	int32_t nSelectionSizeSquared = nSelectionSize * nSelectionSize;

	// Drawing selection
	bool bToolbarMode = false;

	// Drawing modes
	int nMode = 0;

	// Mouse positions in [S] screen and [W] world space
	olc::vf2d vMP_S; 
	olc::vf2d vMP_W;
	olc::vf2d vMP_S_previous;
	olc::vf2d vMP_W_previous;

	// Bounding box in screen space
	olc::vf2d vTL_S = { float(mainToolbarWidth), 0.0f};
	olc::vf2d vTR_S = { float(mainScreenWidth),  0.0f };
	olc::vf2d vBL_S = { float(mainToolbarWidth), float(mainScreenHeight) };
	olc::vf2d vBR_S = { float(mainScreenWidth),  float(mainScreenHeight) };

	// Bounding box in world space
	olc::vf2d vTL_W, vTR_W, vBL_W, vBR_W;

	// Currently node and segment
	int i_segment = -1;
	int i_node = -1;
	int i_node_start = -1;
	int i_node_end = -1;
	olc::vf2d vDifferenceStart, vDifferenceEnd;

	// Geometry
	std::vector<olc::vf2d> nodes;
	std::vector<std::array<int, 2>> segments;



	// DEBUG - ball geometry
	float fBallRadius = 10.0f;
	olc::vf2d vBallPosition;





	// Colors
	olc::Pixel color_ToolbarBackground = olc::BLACK;
	olc::Pixel color_Background = olc::VERY_DARK_BLUE;
	olc::Pixel color_Line = olc::WHITE;
	olc::Pixel color_Node = olc::WHITE;
	olc::Pixel color_NodeInfo = olc::RED;
	olc::Pixel color_Axis = olc::GREY;
	olc::Pixel color_Selection = olc::GREEN;
	olc::Pixel color_TempLine = olc::YELLOW;
	olc::Pixel color_TempNode = olc::YELLOW;
	olc::Pixel color_Intersection = olc::RED;
	olc::Pixel color_VisibilityPolygon = olc::DARK_YELLOW;

	// Layers
	int nLayerBouncingBall = 0;
	int nLayerVisibilityPolygon = 0;
	int nLayerToolbar = 0;
	int nLayerBackground = 0;
	int nLayerGeometry = 0;
	int nLayerAxis = 0;
	int nLayerCursor = 0;


public:

	// Run only at the start of the program
	bool OnUserCreate() override
	{
		// Initialize panning and zooming variables
		vShift = rShiftDefault;
		fScale = fScaleDefault;
		
		// Reserve the number of nodes and segments
		// TODO : Optimize this
		nodes.reserve(16);
		segments.reserve(16);

		// Create layers in order from top to bottom
		nLayerToolbar = CreateLayer();
		nLayerCursor = CreateLayer();
		nLayerGeometry = CreateLayer();
		nLayerVisibilityPolygon = CreateLayer();
		nLayerAxis = CreateLayer();
		nLayerBackground = CreateLayer();

		// Enable layers
		EnableLayer(nLayerToolbar, true);
		EnableLayer(nLayerCursor, true);
		EnableLayer(nLayerGeometry, true);
		EnableLayer(nLayerVisibilityPolygon, true);
		EnableLayer(nLayerBouncingBall, true);
		EnableLayer(nLayerAxis, true);
		EnableLayer(nLayerBackground, true);

		
		return true;
	}

	// Run every frame
	bool OnUserUpdate(float fElapsedTime) override
	{	
		// O------------------------------------------------------------------------------O
		// | CLEAR ALL LAYERS                                                             |
		// O------------------------------------------------------------------------------O
		SetDrawTarget(nullptr);
		Clear(olc::BLANK);

		SetDrawTarget(nLayerBackground);
		Clear(color_Background);

		SetDrawTarget(nLayerCursor);
		Clear(olc::BLANK);

		SetDrawTarget(nLayerGeometry);
		Clear(olc::BLANK);

		SetDrawTarget(nLayerAxis);
		Clear(olc::BLANK);

		SetDrawTarget(nLayerToolbar);
		Clear(olc::BLANK);

		SetDrawTarget(nLayerVisibilityPolygon);
		Clear(olc::BLANK);

		SetDrawTarget(nLayerBouncingBall);
		Clear(olc::BLANK);

		SetDrawTarget(nullptr);


		// O------------------------------------------------------------------------------O
		// | GRAB MOUSE POSITION AND CREATE A BOUNDING BOX                                |
		// O------------------------------------------------------------------------------O
		vMP_S = olc::vf2d{ float(GetMouseX()), float(GetMouseY()) };
		vMP_W = s2w(vMP_S);
		
		// Bounding box
		vTL_W = s2w(vTL_S);
		vTR_W = s2w(vTR_S);
		vBL_W = s2w(vBL_S);
		vBR_W = s2w(vBR_S);

		
		// O------------------------------------------------------------------------------O
		// | DISPLAY ALL GEOMETRY	                                                      |
		// O------------------------------------------------------------------------------O
		SetDrawTarget(nLayerGeometry);
		for (int i = 0; i < segments.size(); i++)
		{
			DrawLine(w2s(nodes[segments[i][0]]), w2s(nodes[segments[i][1]]), color_Line);
		}
		for (int i = 0; i < nodes.size(); i++)
		{
			FillCircle(w2s(nodes[i]), 2, color_Node);
		}
		SetDrawTarget(nullptr);

		
		// O------------------------------------------------------------------------------O
		// | DISPLAY NODES AND THEIR INFO                                                 |
		// O------------------------------------------------------------------------------O
		if (GetKey(olc::Key::N).bPressed)
		{
			bDisplayLineSegmentInfo = !bDisplayLineSegmentInfo;
		}
		if (bDisplayLineSegmentInfo == true)
		{	
			SetDrawTarget(nLayerGeometry);
			for (int i = 0; i < nodes.size(); i++)
			{	
				olc::vf2d vNode = w2s(nodes[i]);
				FillCircle(vNode, 2, color_NodeInfo);
				DrawString(vNode + olc::vi2d{ 5, 5 }, std::to_string(i), color_NodeInfo);
			}
			SetDrawTarget(nullptr);
		}


		// O------------------------------------------------------------------------------O
		// | PANNING AND ZOOMING                                                          |
		// O------------------------------------------------------------------------------O
		SetDrawTarget(nLayerCursor);
		// Pan the screen
		if (!bToolbarMode && GetMouse(2).bHeld)
		{
			vShift += vMP_S - vMP_S_previous;
			DrawCircle(vMP_S, nSelectionSize, color_Selection);
		}
		// Zoom-in the screen
		if (!bToolbarMode && (GetKey(olc::Key::NP_ADD).bHeld || GetMouseWheel() > 0))
		{
			olc::vf2d vMousePointer_temp_start = s2w(vMP_S); // Get starting position for the mouse
			fScale *= fScaleStep; // Scale also changes "s2w" and "w2s" functions
			if (fScale > 100.0f)
			{
				fScale = 100.0f; // Max zoom
			}
			vShift += w2s(s2w(vMP_S)) - w2s(vMousePointer_temp_start); // Correct position by panning
			DrawCircle(vMP_S, nSelectionSize, color_Selection);
		}
		// Zoom-out the screen
		if (!bToolbarMode && (GetKey(olc::Key::NP_SUB).bHeld || GetMouseWheel() < 0))
		{
			olc::vf2d vMousePointer_temp_start = s2w(vMP_S); // Get starting position for the mouse
			fScale *= 1.0f / fScaleStep; // Scale also changes "s2w" and "w2s" functions
			if (fScale < 0.01f)
			{
				fScale = 0.01f; // Min zoom
			}
			vShift += w2s(s2w(vMP_S)) - w2s(vMousePointer_temp_start); // Correct position by panning
			DrawCircle(vMP_S, nSelectionSize, color_Selection);
		}
		// Check if the position is inside drawing area
		if (GetMouseX() > mainToolbarWidth)
		{
			bToolbarMode = false;
			vMP_W_previous = vMP_W;
			vMP_S_previous = vMP_S;
		}
		else
		{
			bToolbarMode = true;
			vMP_W = vMP_W_previous;
			vMP_S = vMP_S_previous;
		}
		// Save the current mouse position for the next frame
		vMP_S_previous = vMP_S;
		// Reset pan and zoom to default value
		if (GetKey(olc::Key::R).bHeld)
		{
			fScale = fScaleDefault;
			vShift = rShiftDefault;
		}
		SetDrawTarget(nullptr);


		// O------------------------------------------------------------------------------O
		// | DISPLAY AXIS                                                                 |
		// O------------------------------------------------------------------------------O
		if (GetKey(olc::Key::A).bPressed)
		{
			bDisplayCoordinateSystem = !bDisplayCoordinateSystem;
		}
		// Draw the coordiante system
		if (bDisplayCoordinateSystem)
		{	
			SetDrawTarget(nLayerAxis);

			// Screen center point
			olc::vf2d vCP = w2s(olc::vf2d{ 0.0f, 0.0f });
			// Draw the coordinate axis
			DrawLine(vCP.x, 0.0,   vCP.x, vBR_S.y, color_Axis, 0xF0F0F0F0);
			DrawLine(vTL_S.x, vCP.y, vTR_S.x, vCP.y, color_Axis, 0xF0F0F0F0);
			// Draw the coordinate axis markings
			DrawString(olc::vf2d{ vCP.x + 5.0f   , 5.0f                   }, "Y",  color_Axis);
			DrawString(olc::vf2d{ vCP.x + 5.0f   , ScreenHeight() - 10.0f }, "-Y", color_Axis);
			DrawString(olc::vf2d{ vBL_S.x + 5.0f , vCP.y + 5.0f           }, "-X", color_Axis);
			DrawString(olc::vf2d{ vBR_S.x - 10.0f, vCP.y + 5.0f           }, "X",  color_Axis);

			SetDrawTarget(nullptr);
		}


		// O------------------------------------------------------------------------------O
		// | ADD NODES                                                                    |
		// O------------------------------------------------------------------------------O
		SetDrawTarget(nLayerCursor);
		// Enter the "add nodes" mode
		if (nMode == 0 && GetKey(olc::Key::K1).bPressed)
		{
			nMode = 1;
		}
		// Exit the "add nodes" mode
		else if (nMode == 1  && (GetKey(olc::Key::K1).bPressed || GetKey(olc::Key::ESCAPE).bPressed))
		{
			nMode = 0;
		}
		// Add the node
		if (nMode == 1)
		{	
			olc::vf2d temp_MP = vMP_W;
			float squaredDistance;
			bool bNodeExists = false;

			// Snap mouse pointer to the nearest existing node
			if (!nodes.empty())
			{
				i_node = FindClosestNode(temp_MP, nodes, &squaredDistance);
				if (squaredDistance * fScale * fScale <= nSelectionSizeSquared)
				{
					temp_MP = nodes[i_node];
					bNodeExists = true;
					DrawCircle(w2s(temp_MP), nSelectionSize, color_Selection);
				}
			}
			if (GetMouse(0).bPressed && !bNodeExists)
			{
				nodes.push_back(temp_MP);
			}
			FillCircle(w2s(temp_MP), 2, color_TempNode);
		}
		SetDrawTarget(nullptr);

		
		// O------------------------------------------------------------------------------O
		// | MOVE NODES                                                                   |
		// O------------------------------------------------------------------------------O
		SetDrawTarget(nLayerCursor);
		// Enter the "move nodes" mode
		if (nMode == 0 && GetKey(olc::Key::K2).bPressed)
		{
			i_node = -1;
			nMode = 2;
		}
		// Exit the "move nodes" mode
		else if (nMode == 2 && (GetKey(olc::Key::K2).bPressed || GetKey(olc::Key::ESCAPE).bPressed))
		{
			nMode = 0;
		}
		// Select the node
		if (nMode == 2 && i_node == -1)
		{
			olc::vf2d temp_MP = vMP_W;
			float squaredDistance;

			// Snap mouse pointer to the nearest existing node
			if (!nodes.empty())
			{
				int temp_i_node = FindClosestNode(temp_MP, nodes, &squaredDistance);
				if (squaredDistance * fScale * fScale <= nSelectionSizeSquared)
				{
					temp_MP = nodes[temp_i_node];
					DrawCircle(w2s(temp_MP), nSelectionSize, color_Selection);

					if (GetMouse(0).bPressed)
					{
						i_node = temp_i_node;
					}
				}
			}
		}
		// Move the node
		if (nMode == 2 && i_node != -1 && GetMouse(0).bHeld)
		{
			DrawCircle(vMP_S, nSelectionSize, color_Selection);
			nodes[i_node] = vMP_W;
		}
		// De-select the node
		if (nMode == 2 && i_node != -1 && GetMouse(0).bReleased)
		{
			i_node = -1;
		}
		SetDrawTarget(nullptr);

		
		// O------------------------------------------------------------------------------O
		// | DELETE NODES                                                                 |
		// O------------------------------------------------------------------------------O
		SetDrawTarget(nLayerCursor);
		// Enter the "delete node" mode
		if (nMode == 0 && GetKey(olc::Key::K3).bPressed)
		{
			nMode = 3;
		}
		// Exit the "delete node" mode
		else if (nMode == 3 && (GetKey(olc::Key::K3).bPressed || GetKey(olc::Key::ESCAPE).bPressed))
		{
			nMode = 0;
		}
		// Delete the node
		if (nMode == 3)
		{
			olc::vf2d temp_MP = vMP_W;
			float squaredDistance;
			bool bNodeExists = false;

			// Snap mouse pointer to the nearest existing node
			if (!nodes.empty())
			{
				i_node = FindClosestNode(temp_MP, nodes, &squaredDistance);
				if (squaredDistance * fScale * fScale <= nSelectionSizeSquared)
				{
					temp_MP = nodes[i_node];
					bNodeExists = true;
					DrawCircle(w2s(temp_MP), nSelectionSize, olc::GREEN);
				}
			}
			// Delete the selected node
			if (GetMouse(0).bPressed && bNodeExists)
			{	
				// Find and delete connected segments
				std::vector<int> connectedSegments = FindConnectedSegments(i_node, segments);
				for (int i = connectedSegments.size()-1; i >= 0; i--)
				{
					segments.erase(segments.begin() + connectedSegments[i]);
				}
				// Re-number the remaining segments
				for (int i = 0; i < segments.size(); i++)
				{
					if (segments[i][0] >= i_node) { segments[i][0] -= 1; }
					if (segments[i][1] >= i_node) { segments[i][1] -= 1; }
				}
				// Delete the selected node
				nodes.erase(nodes.begin() + i_node);
			}
		}
		SetDrawTarget(nullptr);

		
		// O------------------------------------------------------------------------------O
		// | ADD LINE SEGMENTS                                                            |
		// O------------------------------------------------------------------------------O
		SetDrawTarget(nLayerCursor);
		// Enter the "add line segment" mode
		if (nMode == 0 && GetKey(olc::Key::K4).bPressed)
		{
			nMode = 4;
		}
		// Exit the "add line segment" mode
		else if (nMode == 4 && (GetKey(olc::Key::K4).bPressed || GetKey(olc::Key::ESCAPE).bPressed))
		{	
			i_node_start = -1;
			i_node_end = -1;
			nMode = 0;
		}
		// Add a line segment
		if (nMode == 4)
		{
			olc::vf2d vMP_W_temp = vMP_W;
			float squaredDistance;
			bool bNodeExists = false;

			// Check if any nodes exist
			if (nodes.empty())
			{
				FillCircle(vMP_S, 2, color_TempNode);
				if (i_node_start != -1) { DrawLine(w2s(nodes[i_node_start]), vMP_S, color_TempLine); }
			}
			else
			{
				i_node = FindClosestNode(vMP_W, nodes, &squaredDistance);
				if (squaredDistance * fScale * fScale <= nSelectionSizeSquared)
				{
					vMP_W_temp = nodes[i_node];
					bNodeExists = true;
					DrawCircle(w2s(vMP_W_temp), nSelectionSize, color_Selection);
					FillCircle(w2s(vMP_W_temp), 2, color_TempNode);
					if (i_node_start != -1) { DrawLine(w2s(nodes[i_node_start]), w2s(vMP_W_temp), color_TempLine); }
				}
				else
				{
					FillCircle(vMP_S, 2, color_TempNode);
					if (i_node_start != -1) { DrawLine(w2s(nodes[i_node_start]), vMP_S, color_TempLine); }
				}
			}
			// First selection - push back a new node and assign it to "start"
			if (GetMouse(0).bPressed && !bNodeExists && i_node_start == -1)
			{
				nodes.push_back(vMP_W_temp);
				i_node_start = nodes.size() - 1;
			}
			// First selection - selected node is the "start"
			else if (GetMouse(0).bPressed && bNodeExists && i_node_start == -1)
			{
				i_node_start = i_node;
			}
			// Second selection - push back a new node and assign it to "end"
			else if (GetMouse(0).bPressed && !bNodeExists && i_node_start != -1)
			{
				nodes.push_back(vMP_W_temp);
				i_node_end = nodes.size() - 1;
				segments.push_back({i_node_start, i_node_end});
				i_node_start = i_node_end;

			}
			// Second selection - selected node is the "end"
			else if (GetMouse(0).bPressed && bNodeExists && i_node_start != -1)
			{
				i_node_end = i_node;
				std::array<int, 2> temp_segment = {i_node_start, i_node_end};
				if (!DoesSegmentExist(temp_segment, segments) && i_node_start != i_node_end)
				{
					segments.push_back({ i_node_start, i_node_end });
					i_node_start = i_node_end;
				}
			}
		}
		SetDrawTarget(nullptr);


		// O------------------------------------------------------------------------------O
		// | MOVE LINE SEGMENTS                                                           |
		// O------------------------------------------------------------------------------O
		SetDrawTarget(nLayerCursor);
		// Enter the "move line segments" mode
		if (nMode == 0 && GetKey(olc::Key::K5).bPressed)
		{
			i_segment = -1;
			nMode = 5;
		}
		// Exit the "move line segments" mode
		else if (nMode == 5 && (GetKey(olc::Key::K5).bPressed || GetKey(olc::Key::ESCAPE).bPressed))
		{
			nMode = 0;
		}
		// Select the line segment
		if (nMode == 5 && i_segment == -1)
		{	
			olc::vf2d temp_Start, temp_End;
			float squaredDistance;

			// Snap mouse pointer to the nearest existing line segment
			if (!segments.empty())
			{
				int temp_i_segment = FindClosestLineSegment(vMP_W, nodes, segments, &squaredDistance);
				if (squaredDistance * fScale * fScale <= nSelectionSizeSquared)
				{
					// Show selected segment
					temp_Start = w2s(nodes[segments[temp_i_segment][0]]);
					temp_End = w2s(nodes[segments[temp_i_segment][1]]);
					DrawCircle(temp_Start, nSelectionSize, color_Selection);
					DrawCircle(temp_End, nSelectionSize, color_Selection);
					DrawLine(temp_Start, temp_End, color_Selection);

					// Select the segment
					if (GetMouse(0).bPressed)
					{
						i_segment = temp_i_segment;
						vDifferenceStart = nodes[segments[i_segment][0]] - vMP_W;
						vDifferenceEnd = nodes[segments[i_segment][1]] - vMP_W;
					}
				}
			}
		}
		// Move the segment (with the nodes)
		if (nMode == 5 && i_segment != -1 && GetMouse(0).bHeld)
		{	
			// Show selected segment
			olc::vf2d temp_Start = w2s(nodes[segments[i_segment][0]]);
			olc::vf2d temp_End = w2s(nodes[segments[i_segment][1]]);
			DrawCircle(temp_Start, nSelectionSize, color_Selection);
			DrawCircle(temp_End, nSelectionSize, color_Selection);
			DrawLine(temp_Start, temp_End, color_Selection);

			// Move the segment (with the nodes)
			nodes[segments[i_segment][0]] = vMP_W + vDifferenceStart;
			nodes[segments[i_segment][1]] = vMP_W + vDifferenceEnd;
		}
		// De-select the segment
		if (nMode == 5 && i_segment != -1 && GetMouse(0).bReleased)
		{
			i_segment = -1;
		}
		SetDrawTarget(nullptr);


		// O------------------------------------------------------------------------------O
		// | DELETE LINE SEGMENT                                                          |
		// O------------------------------------------------------------------------------O
		SetDrawTarget(nLayerCursor);
		// Enter the "delete line segment" mode
		if (nMode == 0 && GetKey(olc::Key::K6).bPressed)
		{	
			i_segment = -1;
			nMode = 6;
		}
		// Exit the "delete line segment" mode
		else if (nMode == 6 && (GetKey(olc::Key::K6).bPressed || GetKey(olc::Key::ESCAPE).bPressed))
		{
			nMode = 0;
		}
		// Delete the line segment
		if (nMode == 6)
		{
			olc::vf2d temp_Start, temp_End;
			float squaredDistance;
			bool bSegmentExists = false;

			// Snap mouse pointer to the nearest existing line segment
			if (!segments.empty())
			{
				i_segment = FindClosestLineSegment(vMP_W, nodes, segments, &squaredDistance);
				if (squaredDistance * fScale * fScale <= nSelectionSizeSquared)
				{
					temp_Start = w2s(nodes[segments[i_segment][0]]);
					temp_End   = w2s(nodes[segments[i_segment][1]]);
					bSegmentExists = true;

					DrawCircle(temp_Start, nSelectionSize, color_Selection);
					DrawCircle(temp_End, nSelectionSize, color_Selection);
					DrawLine(temp_Start, temp_End, color_Selection);
				}
			}
			// Delete the selected segment
			if (GetMouse(0).bPressed && bSegmentExists)
			{
				segments.erase(segments.begin() + i_segment);
			}
		}
		SetDrawTarget(nullptr);


		// O------------------------------------------------------------------------------O
		// | CLEAR GEOMETRY                                                               |
		// O------------------------------------------------------------------------------O
		if (GetKey(olc::Key::C).bHeld)
		{
			nodes.clear();
			segments.clear();
		}		


		// O------------------------------------------------------------------------------O
		// | CHECK FOR SELF-INTERSECTIONS                                                 |
		// O------------------------------------------------------------------------------O
		std::vector<olc::vf2d> intersections;
		intersections.reserve(16);
		// Find all line segment intersections
		for (int i = 0; i < segments.size(); i++)
		{	
			for (int j = 0; j < segments.size(); j++)
			{	
				// Check for intersection
				olc::vf2d vIntersectionPoint;
				if (SegmentToSegmentIntersection(nodes[segments[i][0]], nodes[segments[i][1]],
					                             nodes[segments[j][0]], nodes[segments[j][1]], &vIntersectionPoint))
				{
					intersections.push_back(vIntersectionPoint);
				}
			}
		}
		// Draw all intersections
		if (GetKey(olc::Key::I).bPressed)
		{
			bDisplaySelfIntersections = !bDisplaySelfIntersections;
		}
		if (bDisplaySelfIntersections == true)
		{
			SetDrawTarget(nLayerGeometry);
			for (int i = 0; i < intersections.size(); i++)
			{
				FillCircle(w2s(intersections[i]), 2, color_Intersection);
			}
			SetDrawTarget(nullptr);
		}
		

		// O------------------------------------------------------------------------------O
		// | VISIBILITY POLYGON                                                           |
		// O------------------------------------------------------------------------------O
		// Select the "visibility polygon" mode
		if (nMode == 0 && GetKey(olc::Key::V).bPressed)
		{
			nMode = 7;
		}
		// Exit the "visibility polygon" mode
		else if (nMode == 7 && ( GetKey(olc::Key::V).bPressed || GetKey(olc::Key::ESCAPE).bPressed))
		{
			nMode = 0;
		}
		// Find intersections to each line
		if (nMode == 7 && GetMouse(0).bHeld)
		{	
			SetDrawTarget(nLayerVisibilityPolygon);

			// Compute The visibility polygon
			std::vector<olc::vf2d> visibilityPolygon = VisibilityPolygon(vMP_W, vTL_W, vTR_W, vBR_W, vBL_W, nodes, segments, intersections);

			// Compute screen coordinates of the nodes
			for (int i = 0; i < visibilityPolygon.size(); i++)
			{
				visibilityPolygon[i] = w2s(visibilityPolygon[i]);
			}
			// Draw visibility polygon
			for (int i = 0; i < visibilityPolygon.size()-1; i++)
			{
				FillTriangle(vMP_S, visibilityPolygon[i], visibilityPolygon[i + 1], color_VisibilityPolygon);
			}
			FillTriangle(vMP_S, visibilityPolygon.back(), visibilityPolygon.front(), color_VisibilityPolygon);
			FillCircle(vMP_S, 3, olc::RED);
			
			SetDrawTarget(nullptr);
		}
		

		// O------------------------------------------------------------------------------O
		// | BOUNCING BALL                                                               |
		// O------------------------------------------------------------------------------O
		// Select the "bouncing ball" mode
		if (nMode == 0 && GetKey(olc::Key::B).bPressed)
		{
			nMode = 8;
		}
		// Exit the "bouncing ball" mode
		else if (nMode == 8 && (GetKey(olc::Key::B).bPressed || GetKey(olc::Key::ESCAPE).bPressed))
		{
			nMode = 0;
		}
		if (nMode == 8 && GetMouse(0).bHeld)
		{
			SetDrawTarget(nLayerBouncingBall);

			
			FillCircle(vMP_S, 3, olc::RED);


			vBallPosition = vMP_W;

			SetDrawTarget(nullptr);
		}
		if (nMode == 8)
		{	
			SetDrawTarget(nLayerBouncingBall);

			// Draw the ball
			FillCircle(w2s(vBallPosition), int(fBallRadius), olc::WHITE);

			vBallPosition.y -= 1;

			SetDrawTarget(nullptr);

		}
		


		









		// O------------------------------------------------------------------------------O
		// | TOOLBAR INFORMATION                                                          |
		// O------------------------------------------------------------------------------O
		SetDrawTarget(nLayerToolbar);

		// Draw a toolbar
		FillRect(olc::vi2d{ 0, 0 }, olc::vi2d{ mainToolbarWidth, ScreenHeight() }, olc::BLACK);

		// Draw the mouse pointer position
		std::string sMousePositionX = "X = " + std::to_string(vMP_W.x);
		std::string sMousePositionY = "Y = " + std::to_string(vMP_W.y);
		std::string sScale =          "S = " + std::to_string(fScale);
		DrawString(olc::vi2d{ 5, 5  }, sMousePositionX, olc::WHITE);
		DrawString(olc::vi2d{ 5, 15 }, sMousePositionY, olc::WHITE);
		DrawString(olc::vi2d{ 5, 25 }, sScale, olc::WHITE);

		// Draw divider line
		DrawLine(olc::vi2d{ 0, 35 }, olc::vi2d{ mainToolbarWidth - 1, 35 }, olc::WHITE);

		// Number of shapes, nodes, segments
		std::string sNumNodes = "# NODES    = " + std::to_string(nodes.size());
		std::string sNumLines = "# SEGMENTS = " + std::to_string(segments.size());
		DrawString(olc::vi2d{ 5, 40 }, sNumNodes, olc::WHITE);
		DrawString(olc::vi2d{ 5, 50 }, sNumLines, olc::WHITE);

		// Divider line
		DrawLine(olc::vi2d{ 0, 60 }, olc::vi2d{ mainToolbarWidth - 1, 60 }, olc::WHITE);

		// Display selection info
		DrawString(olc::vi2d{ 5, 65  }, "[ESC] CLEAR SELECTION ", olc::WHITE);
		DrawString(olc::vi2d{ 5, 75  }, "[1]   NODE - ADD      ", olc::WHITE);
		DrawString(olc::vi2d{ 5, 85  }, "[2]   NODE - MOVE     ", olc::WHITE);
		DrawString(olc::vi2d{ 5, 95  }, "[3]   NODE - DELETE   ", olc::WHITE);
		DrawString(olc::vi2d{ 5, 105 }, "[4]   LINE - ADD      ", olc::WHITE);
		DrawString(olc::vi2d{ 5, 115 }, "[5]   LINE - MOVE     ", olc::WHITE);
		DrawString(olc::vi2d{ 5, 125 }, "[6]   LINE - DELETE   ", olc::WHITE);
		
		// Highlight the selected mode
		if      (nMode == 0) { DrawString(olc::vi2d{ 5, 65  }, "[ESC] CLEAR SELECTION ", olc::RED); }
		else if (nMode == 1) { DrawString(olc::vi2d{ 5, 75  }, "[1]   NODE - ADD      ", olc::RED); }
		else if (nMode == 2) { DrawString(olc::vi2d{ 5, 85  }, "[2]   NODE - MOVE     ", olc::RED); }
		else if (nMode == 3) { DrawString(olc::vi2d{ 5, 95  }, "[3]   NODE - DELETE   ", olc::RED); }
		else if (nMode == 4) { DrawString(olc::vi2d{ 5, 105 }, "[4]   LINE - ADD      ", olc::RED); }
		else if (nMode == 5) { DrawString(olc::vi2d{ 5, 115 }, "[5]   LINE - MOVE     ", olc::RED); }
		else if (nMode == 6) { DrawString(olc::vi2d{ 5, 125 }, "[6]   LINE - DELETE   ", olc::RED); }

		// Divider line
		DrawLine(olc::vi2d{ 0, 135 }, olc::vi2d{ mainToolbarWidth - 1, 135 }, olc::WHITE);

		// Display addition options
		DrawString(olc::vi2d{ 5, 140 }, "[R] RESET PAN & ZOOM  ", olc::WHITE);
		DrawString(olc::vi2d{ 5, 150 }, "[A] SHOW AXIS         ", olc::WHITE);
		DrawString(olc::vi2d{ 5, 160 }, "[N] SHOW NODE INFO    ", olc::WHITE);
		DrawString(olc::vi2d{ 5, 170 }, "[I] SHOW INTERSECTIONS", olc::WHITE);
		DrawString(olc::vi2d{ 5, 180 }, "[C] CLEAR GEOMETRY    ", olc::WHITE);

		// Highlight the selected mode
		if (GetKey(olc::Key::R).bHeld) { DrawString(olc::vi2d{ 5, 140 }, "[R] RESET PAN & ZOOM  ", olc::GREEN); }
		if (bDisplayCoordinateSystem)  { DrawString(olc::vi2d{ 5, 150 }, "[A] SHOW AXIS         ", olc::GREEN); }
		if (bDisplayLineSegmentInfo)   { DrawString(olc::vi2d{ 5, 160 }, "[N] SHOW NODE INFO    ", olc::GREEN); }
		if (bDisplaySelfIntersections) { DrawString(olc::vi2d{ 5, 170 }, "[I] SHOW INTERSECTIONS", olc::GREEN); }
		if (GetKey(olc::Key::C).bHeld) { DrawString(olc::vi2d{ 5, 180 }, "[C] CLEAR GEOMETRY    ", olc::GREEN); }

		// Divider line
		DrawLine(olc::vi2d{ 0, 190 }, olc::vi2d{ mainToolbarWidth - 1, 190 }, olc::WHITE);

		// Display selection info
		DrawString(olc::vi2d{ 5, 195 }, "[V] VISIBILITY POLYGON", olc::WHITE);
		DrawString(olc::vi2d{ 5, 205 }, "[B] BOUNCING BALL     ", olc::WHITE);

		// Highlight selected mode
		if (nMode == 7) { DrawString(olc::vi2d{ 5, 195 }, "[V] VISIBILITY POLYGON", olc::RED); }
		if (nMode == 8) { DrawString(olc::vi2d{ 5, 205 }, "[B] BOUNCING BALL     ", olc::RED); }


		// Default draw target
		SetDrawTarget(nullptr);

		
		return true;

	}
};


int main()
{
	Plane2D demo;
	if (demo.Construct(mainScreenWidth, mainScreenHeight, mainPixelSize, mainPixelSize, false))
		demo.Start();
	return 0;
}