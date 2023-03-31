//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/Utils/FlowVisLib.h"

#include "DynamicMesh/DynamicMeshAttributeSet.h"

////////////////////////////////////////// FFlowVisLib //////////////////////////////////////////
void FFlowVisLib::AddGeometry(UE::Geometry::FDynamicMesh3& InMesh, const FGeometry& InGeometry) {
	using namespace UE::Geometry;
	
	InMesh.Attributes()->SetNumUVLayers(2);
	FDynamicMeshUVOverlay* UV0Overlay = InMesh.Attributes()->GetUVLayer(0);
	FDynamicMeshUVOverlay* UV1Overlay = InMesh.Attributes()->GetUVLayer(1);
	FDynamicMeshNormalOverlay* NormalOverlay = InMesh.Attributes()->PrimaryNormals();
	FDynamicMeshColorOverlay* ColorOverlay = InMesh.Attributes()->PrimaryColors();
	
	auto AppendVertex = [&](const FDAVertexInfo& VertexInfo) {
		const int32 Index = InMesh.AppendVertex(VertexInfo.Position);
		UV0Overlay->AppendElement(VertexInfo.UV0);
		UV1Overlay->AppendElement(VertexInfo.UV1);
		NormalOverlay->AppendElement(VertexInfo.Normal);
		ColorOverlay->AppendElement(VertexInfo.Color);
		return Index;
	};

	constexpr int32 PolyID = 0;
	auto AppendTriangle = [&](const FIndex3i& Tri) {
		const int32 TriId = InMesh.AppendTriangle(Tri, PolyID);
		UV0Overlay->SetTriangle(TriId, Tri);
		UV1Overlay->SetTriangle(TriId, Tri);
		NormalOverlay->SetTriangle(TriId, Tri);
		ColorOverlay->SetTriangle(TriId, Tri);
		//return TriId;
	};

	TArray<int32> VertexLookup;
	VertexLookup.SetNum(InGeometry.Vertices.Num());
	for (int i = 0; i < InGeometry.Vertices.Num(); i++) {
		VertexLookup[i] = AppendVertex(InGeometry.Vertices[i]);
	}
	for (const FIndex3i& LocalTri : InGeometry.Triangles) {
		FIndex3i Tri(
			VertexLookup[LocalTri.A],
			VertexLookup[LocalTri.B],
			VertexLookup[LocalTri.C]);
		AppendTriangle(Tri);
	}
}

void FFlowVisLib::EmitLine(const TArray<FLineInfo>& InLines, float InThickness, FGeometry& OutGeometry) {
	using namespace UE::Geometry;
	auto CreateVertex = [](const FVector3f& InPosition, const FVector2f& InUV, const FVector4f& InColor) {
		FDAVertexInfo Vertex;
		Vertex.Position = FVector3d(InPosition);
		Vertex.Normal = FVector3f::UnitZ();
		Vertex.UV0 = InUV;
		Vertex.UV1 = {1, 1};
		Vertex.Color = InColor;
		return Vertex;
	};

	for (const FLineInfo& LineInfo : InLines) {
		FVector3f Direction = (LineInfo.End - LineInfo.Start).GetSafeNormal();
		FVector3f Side = FVector3f::CrossProduct(Direction, FVector3f::UnitZ()) * InThickness;

		const int32 I00 = OutGeometry.Vertices.Add(CreateVertex(LineInfo.Start - Side, FVector2f(0, 0), LineInfo.Color));
		const int32 I10 = OutGeometry.Vertices.Add(CreateVertex(LineInfo.Start + Side, FVector2f(1, 0), LineInfo.Color));
		const int32 I11 = OutGeometry.Vertices.Add(CreateVertex(LineInfo.End + Side, FVector2f(1, 1), LineInfo.Color));
		const int32 I01 = OutGeometry.Vertices.Add(CreateVertex(LineInfo.End - Side, FVector2f(0, 1), LineInfo.Color));
			
		OutGeometry.Triangles.Add(FIndex3i(I00, I11, I10));
		OutGeometry.Triangles.Add(FIndex3i(I00, I01, I11));
	}
}

void FFlowVisLib::EmitQuad(const TArray<FQuadInfo>& InQuads, FGeometry& OutGeometry) {
	using namespace UE::Geometry;
	auto CreateVertex = [](const FVector3f& InPosition, const FVector2f& InUV, const FVector2f& InUVScale, const FVector4f& InColor) {
		FDAVertexInfo Vertex;
		Vertex.Position = FVector3d(InPosition);
		Vertex.Normal = FVector3f::UnitZ();
		Vertex.UV0 = InUV * InUVScale;
		Vertex.UV1 = InUVScale;
		Vertex.Color = InColor;
		return Vertex;
	};

	for (const FQuadInfo& Quad : InQuads) {
		FVector3f V00 = Quad.Location;
		FVector3f V10 = Quad.Location + FVector3f(Quad.Size.X, 0, 0);
		FVector3f V11 = Quad.Location + FVector3f(Quad.Size.X, Quad.Size.Y, 0);
		FVector3f V01 = Quad.Location + FVector3f(0, Quad.Size.Y, 0);
		
		const int32 I00 = OutGeometry.Vertices.Add(CreateVertex(V00, FVector2f(0, 0), Quad.UVScale, Quad.Color));
		const int32 I10 = OutGeometry.Vertices.Add(CreateVertex(V10, FVector2f(1, 0), Quad.UVScale, Quad.Color));
		const int32 I11 = OutGeometry.Vertices.Add(CreateVertex(V11, FVector2f(1, 1), Quad.UVScale, Quad.Color));
		const int32 I01 = OutGeometry.Vertices.Add(CreateVertex(V01, FVector2f(0, 1), Quad.UVScale, Quad.Color));
			
		OutGeometry.Triangles.Add(FIndex3i(I00, I11, I10));
		OutGeometry.Triangles.Add(FIndex3i(I00, I01, I11));
	}
}


void FFlowVisLib::EmitGrid(const FIntPoint& NumCells, float CellSize, FGeometry& OutGeometry) {
	using namespace UE::Geometry;

	constexpr float LineThickness = 20.0f;
	const FVector3f Color = FVector3f(0, 0, 0);
	TArray<FLineInfo> Lines;
	for (int y = 0; y <= NumCells.Y; y++) {
		Lines.Add({
			FVector3f(0, y, 0) * CellSize,
			FVector3f(NumCells.X, y, 0) * CellSize,
			Color
		});
	}

	for (int x = 0; x <= NumCells.X; x++) {
		Lines.Add({
			FVector3f(x, 0, 0) * CellSize,
			FVector3f(x, NumCells.Y, 0) * CellSize,
			Color
		});
	}
	
	EmitLine(Lines, LineThickness, OutGeometry);
}


