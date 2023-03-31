//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonModel.h"
#include "GridFlowModel.generated.h"

class UGridFlowAbstractGraph;
class UGridFlowTilemap;

USTRUCT()
struct FGridFlowTilemapBuildSetup {
    GENERATED_BODY()
    
    UPROPERTY()
    FIntPoint TilemapOffset = FIntPoint::ZeroValue;

    UPROPERTY()
    bool bWallsAsEdges = false;
    
    UPROPERTY()
    FIntPoint TilesPerLayoutNode;
    
    UPROPERTY()
    FIntPoint LayoutPadding;
};

UCLASS(Blueprintable)
class DUNGEONARCHITECTRUNTIME_API UGridFlowModel : public UDungeonModel {
    GENERATED_BODY()
public:

    virtual void Reset() override;

public:
    UPROPERTY()
    UGridFlowAbstractGraph* AbstractGraph;

    UPROPERTY()
    UGridFlowTilemap* Tilemap;

    UPROPERTY()
    FGridFlowTilemapBuildSetup TilemapBuildSetup;
};

