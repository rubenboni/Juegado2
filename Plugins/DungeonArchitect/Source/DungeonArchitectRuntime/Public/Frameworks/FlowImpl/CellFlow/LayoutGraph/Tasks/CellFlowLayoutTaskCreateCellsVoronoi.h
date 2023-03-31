//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/FlowImpl/CellFlow/LayoutGraph/Tasks/CellFlowLayoutTaskCreateCellsBase.h"
#include "CellFlowLayoutTaskCreateCellsVoronoi.generated.h"

UCLASS(Meta = (AbstractTask, Title="Create Cells (Voronoi)", Tooltip="Create the initial voronoi cell graph to work on", MenuPriority = 1001))
class UCellFlowLayoutTaskCreateCellsVoronoi : public UCellFlowLayoutTaskCreateCellsBase {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "CellFlow")
	int NumPoints = 100;

	UPROPERTY(EditAnywhere, Category = "CellFlow")
	int NumRelaxIteration = 5; 
	
private:
	virtual void GenerateCellsImpl(UDAFlowCellGraph* InCellGraph, const FRandomStream& InRandom) override;
};


