//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/DungeonBuilder.h"
#include "Core/Utils/Attributes.h"
#include "Frameworks/Flow/FlowProcessor.h"
#include "CellFlowBuilder.generated.h"

class UCellFlowConfig;
class UCellFlowModel;

UCLASS(Experimental, HideDropdown)
class DUNGEONARCHITECTRUNTIME_API UCellFlowBuilder : public UDungeonBuilder {
	GENERATED_BODY()
public:
	virtual void BuildDungeonImpl(UWorld* World) override;
    virtual void DestroyDungeonImpl(UWorld* InWorld) override;
	virtual void EmitDungeonMarkers_Implementation() override;
	virtual bool SupportsBackgroundTask() const override { return false; }
	virtual TSubclassOf<UDungeonModel> GetModelClass() override;
	virtual TSubclassOf<UDungeonConfig> GetConfigClass() override;
	virtual TSubclassOf<UDungeonToolData> GetToolDataClass() override;
	virtual TSubclassOf<UDungeonQuery> GetQueryClass() override;
	
	virtual void GetDefaultMarkerNames(TArray<FString>& OutMarkerNames) override;
	
private:
	bool ExecuteGraph();
	void CreateDebugVisualizations(const FGuid& InDungeonId, const FTransform& InTransform) const;
	void DestroyDebugVisualizations(const FGuid& InDungeonId) const;

private:
	TWeakObjectPtr<UCellFlowModel> CellModel;
	TWeakObjectPtr<UCellFlowConfig> CellConfig;
    FDAAttributeList AttributeList;
};


class DUNGEONARCHITECTRUNTIME_API FCellFlowProcessDomainExtender : public IFlowProcessDomainExtender {
public:
	virtual void ExtendDomains(FFlowProcessor& InProcessor) override;

};

