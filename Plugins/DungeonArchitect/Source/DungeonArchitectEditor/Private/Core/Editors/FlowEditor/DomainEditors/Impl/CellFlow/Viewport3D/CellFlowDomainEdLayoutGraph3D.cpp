//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/DomainEditors/Impl/CellFlow/Viewport3D/CellFlowDomainEdLayoutGraph3D.h"

#include "Core/Editors/FlowEditor/DomainEditors/Impl/CellFlow/Viewport3D/SCellFlowDomainEdViewport.h"
#include "Frameworks/Flow/Domains/LayoutGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"
#include "Frameworks/FlowImpl/CellFlow/LayoutGraph/CellFlowLayoutGraph.h"
#include "Frameworks/FlowImpl/CellFlow/LayoutGraph/CellFlowLayoutGraphDomain.h"
#include "Frameworks/FlowImpl/CellFlow/LayoutGraph/CellFlowLayoutVisualization.h"

#define LOCTEXT_NAMESPACE "CellFlowDomainEdLayoutGraph3D"

FCellFlowDomainEdLayoutGraph3D::FCellFlowDomainEdLayoutGraph3D() {
	constexpr float NodeRadius = 100.0f; 
	VisualizerSettings.NodeRadius = NodeRadius;;
	VisualizerSettings.LinkThickness = NodeRadius * 0.2f;
	VisualizerSettings.LinkRefThickness = VisualizerSettings.LinkThickness * 0.5f;
	VisualizerSettings.NodeSeparationDistance = FVector(200, 200, 200);
	VisualizerSettings.DisabledNodeScale = 0.6f;
	VisualizerSettings.DisabledNodeOpacity = 0.75f;
}

IFlowDomainPtr FCellFlowDomainEdLayoutGraph3D::CreateDomain() const {
	return MakeShareable(new FCellFlowLayoutGraphDomain);
}

void FCellFlowDomainEdLayoutGraph3D::BuildCustomVisualization(UWorld* InWorld, const FFlowExecNodeStatePtr& State) {
	const UCellFlowLayoutGraph* LayoutGraph = State->GetState<UCellFlowLayoutGraph>(UFlowAbstractGraphBase::StateTypeID);
	const UDAFlowCellGraph* CellGraph = State->GetState<UDAFlowCellGraph>(UDAFlowCellGraph::StateTypeID);
	if (!LayoutGraph || !CellGraph) return;
	
	const ACellFlowLayoutVisualization* Visualizer = FindActor<ACellFlowLayoutVisualization>(InWorld);
	if (!Visualizer) {
		Visualizer = InWorld->SpawnActor<ACellFlowLayoutVisualization>();
	}

	if (Visualizer) {
		Visualizer->Generate(LayoutGraph, CellGraph, VisualizerSettings.NodeSeparationDistance);
	}
}

TSharedPtr<SFlowDomainEdViewport3D> FCellFlowDomainEdLayoutGraph3D::CreateViewport() const {
	TSharedPtr<SCellFlowDomainEdViewport> NewViewport = SNew(SCellFlowDomainEdViewport);
	NewViewport->GetViewportClient()->SetViewMode(VMI_Lit);
	return NewViewport;
}

float FCellFlowDomainEdLayoutGraph3D::GetLayoutGraphVisualizerZOffset() {
	return VisualizerSettings.NodeRadius * 1.25f;
}


#undef LOCTEXT_NAMESPACE

