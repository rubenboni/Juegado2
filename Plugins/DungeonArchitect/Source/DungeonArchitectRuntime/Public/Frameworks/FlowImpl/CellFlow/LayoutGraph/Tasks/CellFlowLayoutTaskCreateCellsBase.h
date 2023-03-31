//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/LayoutGraph/Tasks/BaseFlowLayoutTask.h"
#include "Frameworks/FlowImpl/CellFlow/Common/CellFlowLib.h"
#include "CellFlowLayoutTaskCreateCellsBase.generated.h"

class UDAFlowCellGenerator;
class UDAFlowCellGraph;
class UCellFlowLayoutGraph;

UCLASS(Abstract)
class UCellFlowLayoutTaskCreateCellsBase : public UBaseFlowLayoutTask  {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "CellFlow")
	FIntPoint WorldSize = { 30, 30 };

	UPROPERTY(EditAnywhere, Category = "CellFlow")
	int MinGroupArea = 20;
	
public:
	virtual void Execute(const FFlowExecutionInput& Input, const FFlowTaskExecutionSettings& InExecSettings, FFlowExecutionOutput& Output) override;
	virtual bool GetParameter(const FString& InParameterName, FDAAttribute& OutValue) override;
	virtual bool SetParameter(const FString& InParameterName, const FDAAttribute& InValue) override;
	virtual bool SetParameterSerialized(const FString& InParameterName, const FString& InSerializedText) override;
    
private:
	virtual void GenerateCellsImpl(UDAFlowCellGraph* InCellGraph, const FRandomStream& InRandom) {}
	static void CreateGraphs(UCellFlowLayoutGraph* InLayoutGraph, UDAFlowCellGraph* InCellGraph);
	
};

