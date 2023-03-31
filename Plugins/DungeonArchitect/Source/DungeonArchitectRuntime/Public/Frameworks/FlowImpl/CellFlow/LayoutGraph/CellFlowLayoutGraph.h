//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/LayoutGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"
#include "Frameworks/FlowImpl/CellFlow/Common/CellFlowLib.h"
#include "CellFlowLayoutGraph.generated.h"

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UCellFlowLayoutGraph : public UFlowAbstractGraphBase, public IFlowExecCloneableState {
	GENERATED_BODY()
public:
	virtual void CloneFromStateObject(const UObject* SourceObject) override;

};

