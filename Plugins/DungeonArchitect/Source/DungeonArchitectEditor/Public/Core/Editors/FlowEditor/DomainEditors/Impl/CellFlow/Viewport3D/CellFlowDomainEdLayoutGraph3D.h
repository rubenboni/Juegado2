//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/FlowEditor/DomainEditors/Common/Viewport3D/FlowDomainEdLayoutGraph3D.h"

class FCellFlowDomainEdLayoutGraph3D : public FFlowDomainEdLayoutGraph3D {
public:
	FCellFlowDomainEdLayoutGraph3D();

private:
	virtual IFlowDomainPtr CreateDomain() const override;
	virtual void BuildCustomVisualization(UWorld* InWorld, const FFlowExecNodeStatePtr& State) override;
	virtual TSharedPtr<SFlowDomainEdViewport3D> CreateViewport() const override;
	virtual float GetLayoutGraphVisualizerZOffset() override;
};

