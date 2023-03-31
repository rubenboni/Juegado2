//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"
#include "CellFlowLib.generated.h"

UCLASS(Abstract)
class DUNGEONARCHITECTRUNTIME_API UDAFlowCellLeafNode : public UObject {
	GENERATED_BODY()
public:
	UPROPERTY()
	int CellId = -1;

public:
	virtual float GetArea() const { return 0; }
	virtual FVector2D GetCenter() const { return FVector2D::ZeroVector; }
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UDAFlowCellLeafNodeGrid : public UDAFlowCellLeafNode {
	GENERATED_BODY()
public:
	UPROPERTY()
	FIntPoint Location;
	
	UPROPERTY()
	FIntPoint Size;

	virtual FVector2D GetCenter() const override { return FVector2D(Location) + FVector2D(Size) * 0.5f; }
	virtual float GetArea() const override { return Size.X * Size.Y; }
};

USTRUCT()
struct DUNGEONARCHITECTRUNTIME_API FDAFlowCellGroupNode {
	GENERATED_BODY()
	
	UPROPERTY()
	int GroupId = -1;

	UPROPERTY()
	FGuid LayoutNodeID = {};

	UPROPERTY()
	FLinearColor GroupColor = {};

	UPROPERTY()
	TSet<int> LeafNodes;
	
	UPROPERTY()
	TSet<int> Connections;  // Connections to the other groups

	UPROPERTY()
	FVector2D PreviewLocation = {};

	FORCEINLINE bool IsActive() const { return LeafNodes.Num() > 0; }
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UDAFlowCellGraph : public UObject, public IFlowExecCloneableState {
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<UDAFlowCellLeafNode*> LeafNodes;
	
	UPROPERTY()
	TArray<FDAFlowCellGroupNode> GroupNodes;

	static const FName StateTypeID;
public:
	template<typename TNode>
	TNode* CreateLeafNode() {
		TNode* LeafNode = NewObject<TNode>(this);
		LeafNodes.Add(LeafNode);
		return LeafNode;
	}

	virtual void CloneFromStateObject(const UObject* SourceObject) override;
};

namespace DA {
	class FCellAreaLookup {
	public:
		void Init(UDAFlowCellGraph* InGraph);
		FORCEINLINE float GetLeafArea(int LeafId) const { return LeafAreas[LeafId]; }
		FORCEINLINE float GetGroupArea(int GroupId) const { return GroupAreas[GroupId]; }
		void SetGroupArea(int GroupId, float NewArea);
		bool GetGroupWithLeastArea(const FRandomStream& InRandom, int& OutGroupId) const;
		
	private:
		TArray<float> LeafAreas;
		TArray<float> GroupAreas;

		TMap<float, TArray<int>> ActiveGroupIdsByArea;
	};
	
	class FCellGraphBuilder {
	public:
		static void CollapseEdges(UDAFlowCellGraph* InGraph, int MinGroupArea, const FRandomStream& Random, FCellAreaLookup& AreaLookup);
		static void AssignGroupColors(UDAFlowCellGraph* InGraph);
		static void AssignGroupPreviewLocations(UDAFlowCellGraph* InGraph, const FCellAreaLookup& InAreaLookup);
		
	private:
		static bool CollapsedBestGroupEdge(UDAFlowCellGraph* InGraph, int MinGroupArea, const FRandomStream& Random, FCellAreaLookup& AreaLookup);
		static void MergeGroups(UDAFlowCellGraph* InGraph, int GroupA, int GroupB, FCellAreaLookup& AreaLookup);
	};
	
}
