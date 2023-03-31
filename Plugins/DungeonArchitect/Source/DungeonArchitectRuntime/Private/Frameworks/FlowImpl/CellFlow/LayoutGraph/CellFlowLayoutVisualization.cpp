//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/FlowImpl/CellFlow/LayoutGraph/CellFlowLayoutVisualization.h"

#include "Frameworks/Flow/Domains/LayoutGraph/Utils/FlowLayoutGraphVisualization.h"
#include "Frameworks/Flow/Utils/FlowVisLib.h"
#include "Frameworks/FlowImpl/CellFlow/LayoutGraph/CellFlowLayoutGraph.h"

#include "Components/DynamicMeshComponent.h"
#include "Materials/Material.h"

ACellFlowLayoutVisualization::ACellFlowLayoutVisualization(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MeshComponent = CreateDefaultSubobject<UDynamicMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	UMaterialInterface* MeshMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
			TEXT("/DungeonArchitect/Core/Editors/FlowGraph/CellFlow/M_CellFlow_Quads_Inst")));
	
	MeshComponent->SetMaterial(0, MeshMaterial ? MeshMaterial : UMaterial::GetDefaultMaterial(MD_Surface));
	SetRootComponent(MeshComponent);
}

void ACellFlowLayoutVisualization::Generate(const UCellFlowLayoutGraph* InLayoutGraph, const UDAFlowCellGraph* InCellGraph, const FVector& InVisualizationScale) const {
	using namespace UE::Geometry;
	FFlowVisLib::FGeometry CellGeometry;

	GenerateGrid(InLayoutGraph, InCellGraph, InVisualizationScale, CellGeometry);
	
	FDynamicMesh3 Mesh(true, true, true, false);;
	Mesh.EnableTriangleGroups();
	Mesh.EnableAttributes();
	Mesh.Attributes()->EnablePrimaryColors();
	FFlowVisLib::AddGeometry(Mesh, CellGeometry);
	MeshComponent->SetMesh(MoveTemp(Mesh));
}

void ACellFlowLayoutVisualization::GenerateGrid(const UCellFlowLayoutGraph* InLayoutGraph, const UDAFlowCellGraph* InCellGraph, const FVector& InVisualizationScale, FFlowVisLib::FGeometry& OutCellGeometry) {
	
	const FVector3f GridSize = FVector3f(InVisualizationScale);
	constexpr float QuadZ = 0;

	// Build the tile quads
	{
		TMap<FGuid, const UFlowAbstractNode*> LayoutNodes;
		for (const UFlowAbstractNode* GraphNode : InLayoutGraph->GraphNodes) {
			const UFlowAbstractNode*& NodeRef = LayoutNodes.FindOrAdd(GraphNode->NodeId);
			NodeRef = GraphNode;
		}
		
		const FVector4f InActiveColor = FVector4f{ 0.25f, 0.25f, 0.25f, 1.0f };
		TArray<FFlowVisLib::FQuadInfo> RenderQuads;
		for (const FDAFlowCellGroupNode& GroupNode : InCellGraph->GroupNodes) {
			if (!GroupNode.IsActive()) continue;
			if (GroupNode.LeafNodes.Num() == 0) continue;
			
			FVector4f Color = GroupNode.GroupColor;
			const UFlowAbstractNode** LayoutNodePtr = LayoutNodes.Find(GroupNode.LayoutNodeID);
			const UFlowAbstractNode* LayoutNode = LayoutNodePtr ? *LayoutNodePtr : nullptr;
			if (!LayoutNode || !LayoutNode->bActive) {
				Color = InActiveColor;
			}
			
			for (const int LeafNodeId : GroupNode.LeafNodes) {
				if (const UDAFlowCellLeafNodeGrid* GridLeafNode = Cast<UDAFlowCellLeafNodeGrid>(InCellGraph->LeafNodes[LeafNodeId])) {
					const FVector2f NodeSize(GridLeafNode->Size.X, GridLeafNode->Size.Y);
					FFlowVisLib::FQuadInfo& Quad = RenderQuads.AddDefaulted_GetRef();
					Quad.Location = FVector3f(GridLeafNode->Location.X, GridLeafNode->Location.Y, QuadZ) * GridSize;
					Quad.Size = NodeSize * FVector2f(GridSize.X, GridSize.Y);
					Quad.UVScale = NodeSize;
					Quad.Color = Color;
				}
			}
		}
		FFlowVisLib::EmitQuad(RenderQuads, OutCellGeometry);
	}

	// Build the group borders
	{
		FIntPoint Min = { MAX_int32 - 2, MAX_int32 - 2 };
		FIntPoint Max = { 0, 0 };
		
		for (const FDAFlowCellGroupNode& GroupNode : InCellGraph->GroupNodes) {
			if (!GroupNode.IsActive()) continue;
			for (const int LeafNodeId : GroupNode.LeafNodes) {
				if (const UDAFlowCellLeafNodeGrid* GridLeafNode = Cast<UDAFlowCellLeafNodeGrid>(InCellGraph->LeafNodes[LeafNodeId])) {
					Min.X = FMath::Min(Min.X, GridLeafNode->Location.X);
					Min.Y = FMath::Min(Min.Y, GridLeafNode->Location.Y);
					Max.X = FMath::Max(Max.X, GridLeafNode->Location.X + GridLeafNode->Size.X);
					Max.Y = FMath::Max(Max.Y, GridLeafNode->Location.Y + GridLeafNode->Size.Y);
				}
			}
		}

		const int32 Width = FMath::Max(0, Max.X - Min.X + 1);
		const int32 Height = FMath::Max(0, Max.Y - Min.Y + 1);
		const int32 Size = Width * Height;
		if (Size > 0) {
			constexpr int32 CELL_EMPTY = -1;
			TArray<int32> CellMask;
			CellMask.SetNumZeroed(Width * Height);
			for (int i = 0; i < CellMask.Num(); i++) {
				CellMask[i] = CELL_EMPTY;
			}

			auto IdxValid = [&Min, &Max](int CoordX, int CoordY) {
				return CoordX >= Min.X && CoordX <= Max.X && CoordY >= Min.Y && CoordY <= Max.Y;
			};
			
			auto IdxFast = [&](int CoordX, int CoordY) {
				return (CoordY - Min.Y) * Width + (CoordX - Min.X);
			};

			auto Idx = [&](int CoordX, int CoordY) {
				check(IdxValid(CoordX, CoordY));
				return IdxFast(CoordX, CoordY);
			};

			auto GroupIdAtLoc = [&](int CoordX, int CoordY) {
				if (!IdxValid(CoordX, CoordY)) {
					return CELL_EMPTY;
				}
				return CellMask[Idx(CoordX, CoordY)];
			};

			
			for (const FDAFlowCellGroupNode& GroupNode : InCellGraph->GroupNodes) {
				if (!GroupNode.IsActive()) continue;
				for (const int LeafNodeId : GroupNode.LeafNodes) {
					if (const UDAFlowCellLeafNodeGrid* GridLeafNode = Cast<UDAFlowCellLeafNodeGrid>(InCellGraph->LeafNodes[LeafNodeId])) {
						const int32 X0 = GridLeafNode->Location.X;
						const int32 X1 = GridLeafNode->Location.X + GridLeafNode->Size.X;
						const int32 Y0 = GridLeafNode->Location.Y;
						const int32 Y1 = GridLeafNode->Location.Y + GridLeafNode->Size.Y;
						
						for (int y = Y0; y < Y1; y++) {
							for (int x = X0; x < X1; x++) {
								CellMask[Idx(x, y)] = GroupNode.GroupId;
							}
						}
					}
				}
			}


			constexpr float LineZOffset = 5;
			TArray<FFlowVisLib::FLineInfo> Lines;
			const FVector4f LineColor = FVector4f(0, 0, 0, 0.75f);
			for (int y = Min.Y - 1; y <= Max.Y; y++) {
				for (int x = Min.X - 1; x <= Max.X; x++) {
					const int32 C00 = GroupIdAtLoc(x, y);
					const int32 C10 = GroupIdAtLoc(x + 1, y);
					const int32 C01 = GroupIdAtLoc(x, y + 1);
					if (C00 != C10) {
						// Draw a vertical line (right side)
						FFlowVisLib::FLineInfo& Line = Lines.AddDefaulted_GetRef();
						Line.Start = FVector3f(x + 1, y, QuadZ) * GridSize + FVector3f(0, 0, LineZOffset);
						Line.End = FVector3f(x + 1, y + 1, QuadZ) * GridSize + FVector3f(0, 0, LineZOffset);
						Line.Color = LineColor;
					}

					if (C00 != C01) {
						// Draw a horizontal line (bottom)
						FFlowVisLib::FLineInfo& Line = Lines.AddDefaulted_GetRef();
						Line.Start = FVector3f(x, y + 1, QuadZ) * GridSize + FVector3f(0, 0, LineZOffset);
						Line.End = FVector3f(x + 1, y + 1, QuadZ) * GridSize + FVector3f(0, 0, LineZOffset);
						Line.Color = LineColor;
					}
				}
			}

			constexpr float LineWidth = 10;
			FFlowVisLib::EmitLine(Lines, LineWidth, OutCellGeometry);
		}
	}
}

