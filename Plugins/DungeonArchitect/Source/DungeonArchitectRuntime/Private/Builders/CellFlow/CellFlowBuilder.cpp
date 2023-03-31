//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Builders/CellFlow/CellFlowBuilder.h"

#include "Builders/CellFlow/CellFlowAsset.h"
#include "Builders/CellFlow/CellFlowConfig.h"
#include "Builders/CellFlow/CellFlowModel.h"
#include "Builders/CellFlow/CellFlowQuery.h"
#include "Builders/CellFlow/CellFlowToolData.h"
#include "Core/Dungeon.h"
#include "Core/DungeonMarkerNames.h"
#include "Frameworks/Flow/Domains/LayoutGraph/Utils/FlowLayoutGraphVisualization.h"
#include "Frameworks/Flow/ExecGraph/FlowExecGraphScript.h"
#include "Frameworks/Flow/FlowProcessor.h"
#include "Frameworks/FlowImpl/CellFlow/LayoutGraph/CellFlowLayoutGraph.h"
#include "Frameworks/FlowImpl/CellFlow/LayoutGraph/CellFlowLayoutGraphDomain.h"
#include "Frameworks/FlowImpl/CellFlow/LayoutGraph/CellFlowLayoutVisualization.h"

#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(CellFlowBuilderLog, Log, All);

void UCellFlowBuilder::BuildDungeonImpl(UWorld* World) {
	CellModel = Cast<UCellFlowModel>(DungeonModel);
	CellConfig = Cast<UCellFlowConfig>(DungeonConfig);
	WorldMarkers.Reset();

	if (!CellModel.IsValid()) {
		UE_LOG(CellFlowBuilderLog, Error, TEXT("Invalid dungeon model provided to the cell flow builder"));
		return;
	}

	if (!CellConfig.IsValid()) {
		UE_LOG(CellFlowBuilderLog, Error, TEXT("Invalid dungeon config provided to the cell flow builder"));
		return;
	}

	if (!ExecuteGraph()) {
		UE_LOG(CellFlowBuilderLog, Error, TEXT("Failed to execute cell flow graph"));
	}

	if (Dungeon->bDrawDebugData) {
		CreateDebugVisualizations(Dungeon->Uid, Dungeon->GetActorTransform());
	}
}

void UCellFlowBuilder::DestroyDungeonImpl(UWorld* InWorld) {
	CellModel = Cast<UCellFlowModel>(DungeonModel);
	CellConfig = Cast<UCellFlowConfig>(DungeonConfig);
	
	CellModel->Reset();

	if (Dungeon) {
		DestroyDebugVisualizations(Dungeon->Uid);
	}
}

void UCellFlowBuilder::EmitDungeonMarkers_Implementation() {
	Super::EmitDungeonMarkers_Implementation();

	CellModel = Cast<UCellFlowModel>(DungeonModel);
	CellConfig = Cast<UCellFlowConfig>(DungeonConfig);

	// TODO: Implement me
	
}

void UCellFlowBuilder::CreateDebugVisualizations(const FGuid& InDungeonId, const FTransform& InTransform) const {
	DestroyDebugVisualizations(InDungeonId);

	if (!CellModel.IsValid() || !CellConfig.IsValid()) {
		return;
	}
	if (CellModel->LayoutGraph && CellModel->CellGraph) {
		UWorld* World = GetWorld();
		
		const float NodeRadius = FMath::Max(CellConfig->GridSize.X, CellConfig->GridSize.Y) * 0.4f;
		const FTransform BaseTransform = Dungeon ? Dungeon->GetActorTransform() : FTransform::Identity;
		const FTransform LayoutGraphOffsetZ = FTransform(FVector(0, 0, NodeRadius * 1.5f));
		
		AFlowLayoutGraphVisualizer* LayoutVisualizer = World->SpawnActor<AFlowLayoutGraphVisualizer>();
		LayoutVisualizer->DungeonID = InDungeonId;
		LayoutVisualizer->SetAutoAlignToLevelViewport(true);
		LayoutVisualizer->SetActorTransform(LayoutGraphOffsetZ * BaseTransform);

		FDAAbstractGraphVisualizerSettings VisualizerSettings;
		VisualizerSettings.NodeRadius = NodeRadius; // ModuleWidth * 0.05;
		VisualizerSettings.LinkThickness = VisualizerSettings.NodeRadius * 0.2f;
		VisualizerSettings.LinkRefThickness = VisualizerSettings.LinkThickness * 0.5f;
		VisualizerSettings.NodeSeparationDistance = CellConfig->GridSize;
		VisualizerSettings.DisabledNodeScale = 0.6f;
		VisualizerSettings.DisabledNodeOpacity = 0.75f;
		LayoutVisualizer->Generate(CellModel->LayoutGraph, VisualizerSettings);

		ACellFlowLayoutVisualization* CellVisualizer = World->SpawnActor<ACellFlowLayoutVisualization>();
		CellVisualizer->SetActorTransform(BaseTransform);
		CellVisualizer->DungeonID = InDungeonId;
		CellVisualizer->Generate(CellModel->LayoutGraph, CellModel->CellGraph, VisualizerSettings.NodeSeparationDistance);
	}
}

void UCellFlowBuilder::DestroyDebugVisualizations(const FGuid& InDungeonId) const {
	const UWorld* World = GetWorld();
	for (TActorIterator<AFlowLayoutGraphVisualizer> It(World); It; ++It) {
		AFlowLayoutGraphVisualizer* Visualizer = *It;
		if (Visualizer && Visualizer->DungeonID == InDungeonId) {
			Visualizer->Destroy();
		}
	}

	
	for (TActorIterator<ACellFlowLayoutVisualization> It(World); It; ++It) {
		ACellFlowLayoutVisualization* Visualizer = *It;
		if (Visualizer && Visualizer->DungeonID == InDungeonId) {
			Visualizer->Destroy();
		}
	}
}

TSubclassOf<UDungeonModel> UCellFlowBuilder::GetModelClass() {
	return UCellFlowModel::StaticClass();
}

TSubclassOf<UDungeonConfig> UCellFlowBuilder::GetConfigClass() {
	return UCellFlowConfig::StaticClass();
}

TSubclassOf<UDungeonToolData> UCellFlowBuilder::GetToolDataClass() {
	return UCellFlowToolData::StaticClass();
}

TSubclassOf<UDungeonQuery> UCellFlowBuilder::GetQueryClass() {
	return UCellFlowQuery::StaticClass();
}

void UCellFlowBuilder::GetDefaultMarkerNames(TArray<FString>& OutMarkerNames) {
	OutMarkerNames.Reset();
	OutMarkerNames.Add(FCellFlowBuilderMarkers::MARKER_GROUND);
	OutMarkerNames.Add(FCellFlowBuilderMarkers::MARKER_WALL);
	OutMarkerNames.Add(FCellFlowBuilderMarkers::MARKER_WALL_SEPARATOR);
	OutMarkerNames.Add(FCellFlowBuilderMarkers::MARKER_DOOR);
	OutMarkerNames.Add(FCellFlowBuilderMarkers::MARKER_DOOR_ONEWAY);
	OutMarkerNames.Add(FCellFlowBuilderMarkers::MARKER_FENCE);
	OutMarkerNames.Add(FCellFlowBuilderMarkers::MARKER_FENCE_SEPARATOR);
}

bool UCellFlowBuilder::ExecuteGraph() {
	CellModel->LayoutGraph = nullptr;

	const UCellFlowAsset* CellFlowAsset = CellConfig->CellFlow.LoadSynchronous();

    if (!CellFlowAsset) {
        UE_LOG(CellFlowBuilderLog, Error, TEXT("Missing Cell Flow graph"));
        return false;
    }

    if (!CellFlowAsset->ExecScript) {
        UE_LOG(CellFlowBuilderLog, Error, TEXT("Invalid Cell Flow graph state. Please resave in editor"));
        return false;
    }

    FFlowProcessor FlowProcessor;
    
    // Register the domains
    {
        FCellFlowProcessDomainExtender Extender;
        Extender.ExtendDomains(FlowProcessor);
    }
    
    const int32 MAX_RETRIES = FMath::Max(1, CellConfig->MaxRetries);
    int32 NumTries = 0;
    FFlowProcessorResult Result;
    while (NumTries < MAX_RETRIES) {
        FFlowProcessorSettings FlowProcessorSettings;
        FlowProcessorSettings.AttributeList = AttributeList;
        FlowProcessorSettings.SerializedAttributeList = CellConfig->ParameterOverrides;
        Result = FlowProcessor.Process(CellFlowAsset->ExecScript, Random, FlowProcessorSettings);
        NumTries++;
        if (Result.ExecResult == EFlowTaskExecutionResult::Success) {
            break;
        }
        if (Result.ExecResult == EFlowTaskExecutionResult::FailHalt) {
            break;
        }
    }

    if (Result.ExecResult != EFlowTaskExecutionResult::Success) {
        UE_LOG(CellFlowBuilderLog, Error, TEXT("Failed to generate grid flow graph"));
        return false;
    }

    if (!CellFlowAsset->ExecScript->ResultNode) {
        UE_LOG(CellFlowBuilderLog, Error, TEXT("Cannot find result node in the grid flow exec graph. Please resave the grid flow asset in the editor"));
        return false;
    }

    const FGuid ResultNodeId = CellFlowAsset->ExecScript->ResultNode->NodeId;
    if (FlowProcessor.GetNodeExecStage(ResultNodeId) != EFlowTaskExecutionStage::Executed) {
        UE_LOG(CellFlowBuilderLog, Error, TEXT("Grid Flow Graph execution failed"));
        return false;
    }

    FFlowExecutionOutput ResultNodeState;
    FlowProcessor.GetNodeState(ResultNodeId, ResultNodeState);
    if (ResultNodeState.ExecutionResult != EFlowTaskExecutionResult::Success) {
        UE_LOG(CellFlowBuilderLog, Error, TEXT("Grid Flow Result node execution did not succeed"));
        return false;
    }

    // Save a copy in the model
    if (CellModel.IsValid()) {
        UCellFlowModel* CellModelPtr = CellModel.Get();

    	// Clone the layout graph and save it in the model
	    {
		    const UCellFlowLayoutGraph* ResultLayoutGraphState = ResultNodeState.State->GetState<UCellFlowLayoutGraph>(UFlowAbstractGraphBase::StateTypeID);
        	CellModel->LayoutGraph = NewObject<UCellFlowLayoutGraph>(CellModelPtr, "LayoutGraph");
        	if (ResultLayoutGraphState) {
        		CellModel->LayoutGraph->CloneFromStateObject(ResultLayoutGraphState);
        	}
	    }

    	// Clone the cell graph and save it in the model
        {
        	const UDAFlowCellGraph* ResultCellGraphState = ResultNodeState.State->GetState<UDAFlowCellGraph>(UDAFlowCellGraph::StateTypeID);
        	CellModel->CellGraph = NewObject<UDAFlowCellGraph>(CellModelPtr, "CellGraph");
        	if (ResultCellGraphState) {
        		CellModel->CellGraph->CloneFromStateObject(ResultCellGraphState);
        	}
        }
    }
    return true;
}

void FCellFlowProcessDomainExtender::ExtendDomains(FFlowProcessor& InProcessor) {
	const TSharedPtr<FCellFlowLayoutGraphDomain> AbstractGraphDomain = MakeShareable(new FCellFlowLayoutGraphDomain);
	InProcessor.RegisterDomain(AbstractGraphDomain);
}

