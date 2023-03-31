//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/FlowEditor/DomainEditors/Common/Viewport3D/FlowDomainEdLayoutGraph3D.h"

#include "Core/Editors/FlowEditor/DomainEditors/Common/Viewport3D/FlowDomainEdViewport3D.h"
#include "Frameworks/Flow/Domains/LayoutGraph/Core/FlowAbstractGraph.h"
#include "Frameworks/Flow/Domains/LayoutGraph/Utils/FlowLayoutGraphVisualization.h"
#include "Frameworks/Flow/ExecGraph/FlowExecTask.h"
#include "Frameworks/Meshing/Geometry/DungeonProceduralMesh.h"

#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"

#define LOCTEXT_NAMESPACE "FlowDomainEdAbstractGraph3D"
DEFINE_LOG_CATEGORY_STATIC(LogAbstractGraphEd3D, Log, All);

//////////////////////////////// FFlowDomainEdAbstractGraph3D ////////////////////////////////
void FFlowDomainEdLayoutGraph3D::InitializeImpl(const FDomainEdInitSettings& InSettings) {
    Viewport = CreateViewport();
    Viewport->GetActorSelectionChanged().BindSP(SharedThis(this), &FFlowDomainEdLayoutGraph3D::OnActorSelectionChanged);
    Viewport->GetActorDoubleClicked().BindSP(SharedThis(this), &FFlowDomainEdLayoutGraph3D::OnActorDoubleClicked);

    if (UWorld* World = Viewport->GetWorld()) {
        UStaticMesh* SkyboxMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr,
            TEXT("/DungeonArchitect/Samples/DA_SnapMap_GameDemo/Art/Meshes/Skybox")));
        Skybox = World->SpawnActor<AStaticMeshActor>();
        Skybox->GetStaticMeshComponent()->SetStaticMesh(SkyboxMesh);
        Skybox->SetActorScale3D(FVector(100.0f, 100.0f, 100.0f));
        
        UMaterialInterface* SkyboxMaterial = Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr,
            TEXT("/DungeonArchitect/Core/Editors/FlowGraph/AbstractGraph3D/Materials/M_AbstractGraph3DSkybox_Inst")));
        Skybox->GetStaticMeshComponent()->SetMaterial(0, SkyboxMaterial);
    }
    
    ContentWidget = SNew(SOverlay)
    // The graph panel
    +SOverlay::Slot()
    [
        Viewport.ToSharedRef()
    ]
    +SOverlay::Slot()
    .Padding(10)
    .VAlign(VAlign_Bottom)
    .HAlign(HAlign_Right)
    [
        SNew(STextBlock)
        .Visibility( EVisibility::HitTestInvisible )
        .TextStyle( FAppStyle::Get(), "Graph.CornerText" )
        .Text( LOCTEXT("CornerText", "Layout Graph") )
    ];
}

FFlowDomainEditorTabInfo FFlowDomainEdLayoutGraph3D::GetTabInfo() const {
    static const FFlowDomainEditorTabInfo TabInfo = {
        TEXT("LayoutGraph3DTab"),
        LOCTEXT("LayoutGraph3DTabLabel", "Layout Graph 3D"),
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details")
    };

    return TabInfo;
}

TSharedRef<SWidget> FFlowDomainEdLayoutGraph3D::GetContentWidget() {
    return ContentWidget.ToSharedRef();
}

void FFlowDomainEdLayoutGraph3D::Build(const FFlowExecNodeStatePtr& State) {
    if (!State.IsValid()) return;
    
    UWorld* World = Viewport->GetWorld();
    
    // Find or create the visualization actor
    AFlowLayoutGraphVisualizer* LayoutGraphVisualizer = FindActor<AFlowLayoutGraphVisualizer>(World);
    if (!LayoutGraphVisualizer) {
        LayoutGraphVisualizer = World->SpawnActor<AFlowLayoutGraphVisualizer>();
        LayoutGraphVisualizer->SetActorLocation(FVector(0, 0, GetLayoutGraphVisualizerZOffset()));
    }
    
    if (LayoutGraphVisualizer) {
        BuildLayoutGraphVisualization(LayoutGraphVisualizer, State);
    }
    
    BuildCustomVisualization(World, State);

}

void FFlowDomainEdLayoutGraph3D::BuildLayoutGraphVisualization(AFlowLayoutGraphVisualizer* LayoutGraphVisualizer, const FFlowExecNodeStatePtr& State) {
    if (UFlowAbstractGraphBase* Graph = State->GetState<UFlowAbstractGraphBase>(UFlowAbstractGraphBase::StateTypeID)) {
        LayoutGraphVisualizer->Generate(Graph, VisualizerSettings);    
    }
}

void FFlowDomainEdLayoutGraph3D::BuildCustomVisualization(UWorld* InWorld, const FFlowExecNodeStatePtr& State) {
    // Override this method in child classes to add your own visual overlays on the scene
}

TSharedPtr<SFlowDomainEdViewport3D> FFlowDomainEdLayoutGraph3D::CreateViewport() const {
    TSharedPtr<SFlowDomainEdViewport3D> NewViewport = SNew(SFlowDomainEdViewport3D);
    NewViewport->GetViewportClient()->SetViewMode(VMI_Lit);
    return NewViewport;
}

float FFlowDomainEdLayoutGraph3D::GetLayoutGraphVisualizerZOffset() { return 0; }

void FFlowDomainEdLayoutGraph3D::RecenterView(const FFlowExecNodeStatePtr& State) {
    if (!State.IsValid()) {
        return;
    }
    UFlowAbstractGraphBase* ScriptGraph = State->GetState<UFlowAbstractGraphBase>(UFlowAbstractGraphBase::StateTypeID);
    if (ScriptGraph) {
        FBox TotalGridBounds(EForceInit::ForceInit);
        FBox ActiveGridBounds(EForceInit::ForceInit);

        int32 NumActiveNodes = 0;
        bool bHasActiveNodes = false;
        for (const UFlowAbstractNode* Node : ScriptGraph->GraphNodes) {
            if (!Node) continue;
            TotalGridBounds += Node->PreviewLocation;
            if (Node->bActive) {
                NumActiveNodes++;
                bHasActiveNodes = true;
                ActiveGridBounds += Node->PreviewLocation;
                for (UFlowAbstractNode* const SubNode : Node->MergedCompositeNodes) {
                    if (!SubNode) continue;
                    ActiveGridBounds += SubNode->PreviewLocation;
                }
            }
        }

        FBox Focus = bHasActiveNodes ? ActiveGridBounds : TotalGridBounds;
        const float ExpandAmount = (NumActiveNodes == 1) ? 100 : 50;
        Focus = Focus.ExpandBy(FVector(ExpandAmount));
        Viewport->GetViewportClient()->FocusViewportOnBox(Focus);
    }
}

void FFlowDomainEdLayoutGraph3D::Tick(float DeltaTime) {
    const UWorld* World = Viewport.IsValid() ? Viewport->GetWorld() : nullptr;
    if (World) {
        if (const AFlowLayoutGraphVisualizer* LayoutGraphVisualizer = FindActor<AFlowLayoutGraphVisualizer>(World)) {
            const FVector ViewLocation = Viewport->GetViewportClient()->GetViewLocation();
            LayoutGraphVisualizer->AlignToCamera(ViewLocation);
        }
    }
}

void FFlowDomainEdLayoutGraph3D::AddReferencedObjects(FReferenceCollector& Collector) {
    if (Skybox) {
        Collector.AddReferencedObject(Skybox);
    }
}

FString FFlowDomainEdLayoutGraph3D::GetReferencerName() const {
    static const FString NameString = TEXT("FFlowDomainEdAbstractGraph3D");
    return NameString;
}

void FFlowDomainEdLayoutGraph3D::OnActorSelectionChanged(AActor* InActor) {
    // Deselect the old actor
    if (SelectedNode.IsValid()) {
        SelectedNode->SetSelected(false);
    }
    
    UFDAbstractNodePreview* NewSelection = Cast<UFDAbstractNodePreview>(InActor);
    if (NewSelection) {
        SelectedNode = NewSelection;
        SelectedNode->SetSelected(true);
    }
    else {
        SelectedNode = nullptr;
    }
}

void FFlowDomainEdLayoutGraph3D::OnActorDoubleClicked(AActor* InActor) {
    if (InActor) {
        UE_LOG(LogAbstractGraphEd3D, Log, TEXT("Actor DoubleClick: %s"), *InActor->GetName());
    }
}


#undef LOCTEXT_NAMESPACE

