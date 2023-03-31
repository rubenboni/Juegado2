//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Utils/FlowVisLib.h"

#include "GameFramework/Actor.h"
#include "CellFlowLayoutVisualization.generated.h"

class UDAFlowCellGraph;
class UCellFlowLayoutGraph;
class UDynamicMeshComponent;

UCLASS()
class DUNGEONARCHITECTRUNTIME_API ACellFlowLayoutVisualization : public AActor {
	GENERATED_UCLASS_BODY()
	
public:
	/**
	 * @brief Generate cell flow visualization
	 * @param InLayoutGraph The flow layout graph
	 * @param InCellGraph The cell flow graph
	 * @param InVisualizationScale The node separation distance 
	 */
	void Generate(const UCellFlowLayoutGraph* InLayoutGraph, const UDAFlowCellGraph* InCellGraph, const FVector& InVisualizationScale) const;

private:
	static void GenerateGrid(const UCellFlowLayoutGraph* InLayoutGraph, const UDAFlowCellGraph* InCellGraph, const FVector& InVisualizationScale, FFlowVisLib::FGeometry& OutCellGeometry);
	
public:
	UPROPERTY()
	FGuid DungeonID;
	
private:
	UPROPERTY()
	TObjectPtr<UDynamicMeshComponent> MeshComponent;
};


