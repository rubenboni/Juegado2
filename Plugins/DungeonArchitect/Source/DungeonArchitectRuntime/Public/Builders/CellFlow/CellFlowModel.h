//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonModel.h"
#include "CellFlowModel.generated.h"

class UDAFlowCellGraph;
class UCellFlowLayoutGraph;

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UCellFlowModel : public UDungeonModel {
	GENERATED_BODY()
public:
	
	UPROPERTY()
	TObjectPtr<UCellFlowLayoutGraph> LayoutGraph = nullptr;

	UPROPERTY()
	TObjectPtr<UDAFlowCellGraph> CellGraph = nullptr;
};

