//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Frameworks/Flow/ExecGraph/FlowExecEdGraphSchema.h"

#include "Core/Utils/DungeonGraphUtils.h"
#include "Frameworks/Flow/Domains/FlowDomain.h"
#include "Frameworks/Flow/ExecGraph/FlowExecConnectionDrawingPolicy.h"
#include "Frameworks/Flow/ExecGraph/FlowExecEdGraph.h"
#include "Frameworks/Flow/ExecGraph/Nodes/FlowExecEdGraphNodes.h"
#include "Frameworks/Flow/ExecGraph/Utils/ExecGraphEditorUtils.h"
#include "Frameworks/FlowImpl/GridFlow/Tilemap/Tasks/GridFlowTilemapTaskInitialize.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "UObject/UObjectIterator.h"

#define LOCTEXT_NAMESPACE "GridFlowExecEdGraphSchema"

UFlowExecEdGraphSchema::UFlowExecEdGraphSchema(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
    DomainFilter = MakeShareable(new FFlowExecSchemaDomainFilter);
}

void UFlowExecEdGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const {
    const UFlowExecEdGraph* Graph = Cast<UFlowExecEdGraph>(ContextMenuBuilder.CurrentGraph);

    TArray<TSharedPtr<FEdGraphSchemaAction>> Actions;
    GetActionList(Actions, Graph, ContextMenuBuilder.OwnerOfTemporaries);

    for (TSharedPtr<FEdGraphSchemaAction> Action : Actions) {
        ContextMenuBuilder.AddAction(Action);
    }
}

namespace {
    template <typename TTask>
    void AddTaskContextAction(const FText& Category, const FText& Title, const FText& Tooltip, int32 Priority,
                              TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions, UEdGraph* OwnerOfTemporaries) {
        AddTaskContextAction(TTask::StaticClass(), Category, Title, Tooltip, Priority, OutActions, OwnerOfTemporaries);
    }

    void AddTaskContextAction(UClass* Class, const FText& Category, const FText& Title, const FText& Tooltip, int32 Priority, TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions, UEdGraph* OwnerOfTemporaries) {
        const TSharedPtr<FFlowExecSchemaAction_NewNode> NewActorNodeAction = MakeShared<FFlowExecSchemaAction_NewNode>(Category, Title, Tooltip, 0);
        OutActions.Add(NewActorNodeAction);
        
        UFlowExecEdGraphNode_Task* TaskNodeTemplate = NewObject<UFlowExecEdGraphNode_Task>(OwnerOfTemporaries);
        TaskNodeTemplate->TaskTemplate = NewObject<UFlowExecTask>(TaskNodeTemplate, Class);
        NewActorNodeAction->NodeTemplate = TaskNodeTemplate;
    }
}

void UFlowExecEdGraphSchema::GetActionList(TArray<TSharedPtr<FEdGraphSchemaAction>>& OutActions,
                                               const UEdGraph* Graph, UEdGraph* OwnerOfTemporaries) const {
    struct FTaskClassInfo {
        UClass* Class;
        FText Title;
        FText Tooltip;
        FText Category;
        int32 Priority;
    };
    TArray<FTaskClassInfo> TaskClasses;
    if (DomainFilter.IsValid()) {
        for (IFlowDomainWeakPtr DomainPtr : DomainFilter->GetAllowedDomains()) {
            IFlowDomainPtr Domain = DomainPtr.Pin();
            if (Domain.IsValid()) {
                int32 TaskCounter = 1;
                TArray<UClass*> DomainClasses;
                Domain->GetDomainTasks(DomainClasses);
                const FText DomainCategory = Domain->GetDomainDisplayName();
                for (UClass* Class : DomainClasses) {
                    FTaskClassInfo ClassInfo;
                    ClassInfo.Class = Class;
                    ClassInfo.Category = DomainCategory;
                    FString TitleText = FString::Printf(TEXT("%d.  %s"), TaskCounter++, *Class->GetMetaData("Title"));
                    ClassInfo.Title = FText::FromString(TitleText);
                    ClassInfo.Tooltip = FText::FromString(Class->GetMetaData("Tooltip"));
                    ClassInfo.Priority = Class->GetIntMetaData("MenuPriority");
                    TaskClasses.Add(ClassInfo);
                }
            }
        }
    }
    
    for (const FTaskClassInfo& ClassInfo : TaskClasses) {
        AddTaskContextAction(ClassInfo.Class, ClassInfo.Category, ClassInfo.Title, ClassInfo.Tooltip, ClassInfo.Priority, OutActions, OwnerOfTemporaries);
    }
}

void UFlowExecEdGraphSchema::SetAllowedDomains(const TArray<IFlowDomainWeakPtr>& InAllowedDomains) const {
    if (DomainFilter.IsValid()) {
        DomainFilter->SetAllowedDomains(InAllowedDomains);
    }
}

TArray<IFlowDomainWeakPtr> UFlowExecEdGraphSchema::GetAllowedDomains() const { return DomainFilter->GetAllowedDomains(); }

const FPinConnectionResponse UFlowExecEdGraphSchema::CanCreateConnection(
    const UEdGraphPin* A, const UEdGraphPin* B) const {
    // Make sure the data types match
    if (A->PinType.PinCategory != B->PinType.PinCategory) {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not allowed"));
    }
    // Make sure they are not the same pins
    if (A->GetOwningNode() == B->GetOwningNode()) {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not allowed"));
    }

    // Constrain result node direction
    if (A->GetOwningNode()->IsA<UFlowExecEdGraphNode_Result>()) {
        return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, TEXT("Not allowed"));
    }

    return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

FLinearColor UFlowExecEdGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const {
    return FColor::Yellow;
}

bool UFlowExecEdGraphSchema::ShouldHidePinDefaultValue(UEdGraphPin* Pin) const {
    return false;
}

UEdGraphNode* UFlowExecEdGraphSchema::CreateSubstituteNode(UEdGraphNode* Node, const UEdGraph* Graph,
            FObjectInstancingGraph* InstanceGraph, TSet<FName>& InOutExtraNames) const {
    if (UFlowExecEdGraphNode_Task* TaskNode = Cast<UFlowExecEdGraphNode_Task>(Node)) {
        if (DomainFilter.IsValid()) {
            for (const IFlowDomainWeakPtr& DomainPtr : DomainFilter->GetAllowedDomains()) {
                IFlowDomainPtr Domain = DomainPtr.Pin();
                if (Domain.IsValid()) {
                    UFlowExecTask* CompatibleTemplate =  Domain->TryCreateCompatibleTask(TaskNode->TaskTemplate);
                    if (CompatibleTemplate) {
                        TaskNode->TaskTemplate = CompatibleTemplate;
                        return TaskNode;
                    }
                }
            } 
        }
    }
    
    return nullptr;
}

FConnectionDrawingPolicy* UFlowExecEdGraphSchema::CreateConnectionDrawingPolicy(
    int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect,
    class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const {
    return new FFlowExecConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect,
                                                    InDrawElements, InGraphObj);
}

#if WITH_EDITOR
bool UFlowExecEdGraphSchema::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const {
    UFlowExecEdGraphNodeBase* NodeA = Cast<UFlowExecEdGraphNodeBase>(A->GetOwningNode());
    UFlowExecEdGraphNodeBase* NodeB = Cast<UFlowExecEdGraphNodeBase>(B->GetOwningNode());
    UEdGraphPin* OutputA = NodeA->GetOutputPin();
    UEdGraphPin* InputB = NodeB->GetInputPin();
    if (!OutputA || !InputB) {
        return false;
    }

    bool bSourceIsTilemapCreateNode = false;
    {
        if (UFlowExecEdGraphNode_Task* TaskNodeA = Cast<UFlowExecEdGraphNode_Task>(NodeA)) {
            bSourceIsTilemapCreateNode = TaskNodeA-> TaskTemplate && TaskNodeA->TaskTemplate->IsA<UGridFlowTilemapTaskInitialize>();
        }
    }
    bool bConnectionMade = UEdGraphSchema::TryCreateConnection(OutputA, InputB);
    if (bConnectionMade && OutputA && InputB) {
        if (!bSourceIsTilemapCreateNode) {
            // Allow only one outgoing link
            TArray<UEdGraphPin*> LinkedPins = A->LinkedTo;
            for (UEdGraphPin* LinkedPin : LinkedPins) {
                if (LinkedPin != InputB) {
                    // Break this pin
                    OutputA->BreakLinkTo(LinkedPin);
                }
            }
            // Break a reverse link, if it exists
            {
                UEdGraphPin* InputA = NodeA->GetInputPin();
                UEdGraphPin* OutputB = NodeB->GetOutputPin();
                if (InputA && OutputB) {
                    OutputB->BreakLinkTo(InputA);
                }
            }

            UEdGraph* Graph = A->GetOwningNode()->GetGraph();
            Graph->NotifyGraphChanged();
        }
    }
    return bConnectionMade;
}

void UFlowExecEdGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const {
    UEdGraphSchema::BreakPinLinks(TargetPin, bSendsNodeNotifcation);
    TargetPin.GetOwningNode()->GetGraph()->NotifyGraphChanged();
}

void UFlowExecEdGraphSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const {
    UEdGraphSchema::BreakSinglePinLink(SourcePin, TargetPin);
    SourcePin->GetOwningNode()->GetGraph()->NotifyGraphChanged();
}

void UFlowExecEdGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const {
    UEdGraphSchema::BreakNodeLinks(TargetNode);
    TargetNode.GetGraph()->NotifyGraphChanged();
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE

