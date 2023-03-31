//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonConfig.h"
#include "CellFlowConfig.generated.h"

class UCellFlowAsset;

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UCellFlowConfig : public UDungeonConfig {
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	TSoftObjectPtr<UCellFlowAsset> CellFlow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	FVector GridSize = FVector(400, 400, 200);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	int32 MaxRetries = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Dungeon)
	TMap<FString, FString> ParameterOverrides;
};

