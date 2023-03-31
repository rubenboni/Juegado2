//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Frameworks/Flow/Domains/LayoutGraph/Tasks/BaseFlowLayoutTaskFinalize.h"
#include "CellFlowLayoutTaskFinalize.generated.h"

UCLASS(Meta = (AbstractTask, Title = "Finalize Graph", Tooltip = "Call this to finalize the layout graph", MenuPriority = 1500))
class UCellFlowLayoutTaskFinalize : public UBaseFlowLayoutTaskFinalize {
	GENERATED_BODY()
public:

};

