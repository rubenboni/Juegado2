//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "DynamicMesh/DynamicMesh3.h"

class FFlowVisLib {
public:
	struct FDAVertexInfo
	{
		FVector3d Position { FVector3d::Zero() };
		FVector3f Normal { FVector3f::Zero() };
		FVector4f Color { FVector4f::Zero() };
		FVector2f UV0 { FVector2f::Zero() };
		FVector2f UV1 { FVector2f::Zero() };
	};
	
	struct FGeometry {
		TArray<FDAVertexInfo> Vertices;
		TArray<UE::Geometry::FIndex3i> Triangles;
	};
	
	struct FLineInfo {
		FVector3f Start = {};
		FVector3f End = {};
		FVector4f Color;
	};

	struct FQuadInfo {
		FVector3f Location = {};
		FVector2f Size = {};
		FVector2f UVScale = {};
		FVector4f Color;
	};

	static void EmitGrid(const FIntPoint& NumCells, float CellSize, FGeometry& OutGeometry);
	static void EmitLine(const TArray<FLineInfo>& InLines, float InThickness, FGeometry& OutGeometry);
	static void EmitQuad(const TArray<FQuadInfo>& InQuads, FGeometry& OutGeometry);
	static void AddGeometry(UE::Geometry::FDynamicMesh3& InMesh, const FGeometry& InGeometry);
};
