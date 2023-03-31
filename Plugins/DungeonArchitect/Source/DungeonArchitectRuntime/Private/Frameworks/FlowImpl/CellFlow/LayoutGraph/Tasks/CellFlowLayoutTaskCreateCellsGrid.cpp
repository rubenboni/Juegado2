//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/FlowImpl/CellFlow/LayoutGraph/Tasks/CellFlowLayoutTaskCreateCellsGrid.h"

#include "Core/Utils/MathUtils.h"
#include "Frameworks/FlowImpl/CellFlow/Common/CellFlowStats.h"

void UCellFlowLayoutTaskCreateCellsGrid::GenerateCellsImpl(UDAFlowCellGraph* InGraph, const FRandomStream& Random) {
	const FIntPoint WorldLoc {0, 0};
	
	const int NumTiles = WorldSize.X * WorldSize.Y;
	constexpr int CELL_EMPTY = -1;
	constexpr int CELL_INVALID = -2;
	TArray<int> CellTileMap;

	auto FnGetCellIdAtTile = [&](int x, int y) {
		if (x < 0 || x >= WorldSize.X || y < 0 || y >= WorldSize.Y) {
			return CELL_INVALID;
		}
		const int Idx = y * WorldSize.X + x;
		return CellTileMap[Idx];
	};

	auto FnSetCellIdAtTile = [&](int x, int y, int Value) {
		if (x < 0 || x >= WorldSize.X || y < 0 || y >= WorldSize.Y) {
			return;
		}
		const int Idx = y * WorldSize.X + x;
		CellTileMap[Idx] = Value;
	};

	CellTileMap.SetNum(NumTiles);
	for (int i = 0; i < NumTiles; i++) {
		CellTileMap[i] = CELL_EMPTY;
	}
	int CellIdCounter = 0;

	// Generate a shuffled tile coord list
	TArray<FIntPoint> TileCoords;
	{
		TileCoords.Reserve(NumTiles);

		for (int y = WorldLoc.Y; y < WorldLoc.Y + WorldSize.Y; y++) {
			for (int x = WorldLoc.X; x < WorldLoc.X + WorldSize.X; x++) {
				TileCoords.Add({x, y});
			}
		}
		FMathUtils::Shuffle(TileCoords, Random);
	}

	// Fit tiles in the tilemap
	{
		SCOPE_CYCLE_COUNTER(STAT_CFBuild_FitTiles);
		for (const FIntPoint& Coord : TileCoords) {
			for (int t = 0; t < FitIterations; t++) {
				int CellWidth = Random.RandRange(MinCellSize, MaxCellSize);
				int CellHeight;
				{
					int CellHeightMin = static_cast<int>(round(CellWidth * MinAspectRatio));
					int CellHeightMax = static_cast<int>(round(CellWidth * MaxAspectRatio));
					CellHeightMin = FMath::Clamp(CellHeightMin, MinCellSize, MaxCellSize);
					CellHeightMax = FMath::Clamp(CellHeightMax, MinCellSize, MaxCellSize);
					CellHeight = Random.RandRange(CellHeightMin, CellHeightMax);
				}

				bool bCanFit = true;
				// Check if we can fit a cell of this size
				for (int cy = 0; cy < CellHeight; cy++) {
					for (int cx = 0; cx < CellWidth; cx++) {
						const int CellIdAtTile = FnGetCellIdAtTile(Coord.X + cx, Coord.Y + cy);
						if (CellIdAtTile != CELL_EMPTY || CellIdAtTile == CELL_INVALID) {
							bCanFit = false;
							break;

						}
					}
					if (!bCanFit) break;
				}

				if (bCanFit) {
					UDAFlowCellLeafNodeGrid* Node = InGraph->CreateLeafNode<UDAFlowCellLeafNodeGrid>();
					Node->CellId = CellIdCounter++;
					Node->Location = Coord;
					Node->Size = {CellWidth, CellHeight};

					// Tag the cell tiles
					for (int cy = 0; cy < CellHeight; cy++) {
						for (int cx = 0; cx < CellWidth; cx++) {
							FnSetCellIdAtTile(Coord.X + cx, Coord.Y + cy, Node->CellId);
						}
					}

					break;
				}
			}
		}
	}

	// Build the cell graph
	{
		SCOPE_CYCLE_COUNTER(STAT_CFBuild_BuildCellGraph);
		const int NumCells = InGraph->LeafNodes.Num();
		TArray<TSet<int>> Connections;
		Connections.SetNum(NumCells);

		static const TArray<FIntPoint> NCoords = {
			{-1, 0 },
			{ 1, 0 },
			{ 0, -1 },
			{ 0, 1 }
		};

		for (int y = WorldLoc.Y; y < WorldLoc.Y + WorldSize.Y; y++) {
			for (int x = WorldLoc.X; x < WorldLoc.X + WorldSize.X; x++) {
				const int GroupId = FnGetCellIdAtTile(x, y);
				if (GroupId == CELL_EMPTY || GroupId == CELL_INVALID) {
					continue;
				}
				for (const FIntPoint& NCoord : NCoords) {
					const int dx = NCoord.X;
					const int dy = NCoord.Y;
					const int NGroupID = FnGetCellIdAtTile(x + dx, y + dy);
					if (NGroupID != GroupId && NGroupID != CELL_EMPTY && NGroupID != CELL_INVALID) {
						Connections[GroupId].Add(NGroupID);
						Connections[NGroupID].Add(GroupId);
					}
				}
			}
		}

		InGraph->GroupNodes.Reset();
		InGraph->GroupNodes.SetNum(NumCells);
		for (int i = 0; i < NumCells; i++) {
			FDAFlowCellGroupNode& GroupNode = InGraph->GroupNodes[i];
			GroupNode.GroupId = i;
			GroupNode.LeafNodes.Add(i);    // each node belongs to its own group initially
			GroupNode.Connections = Connections[i];
			GroupNode.GroupColor = FColorUtils::GetRandomColor(Random, 0.3f);
		}
	}
}

