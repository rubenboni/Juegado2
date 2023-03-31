//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "FlowExecGraphScript.generated.h"

class UFlowExecTask;

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UFlowExecScriptGraphNode : public UObject {
    GENERATED_BODY()
public:
    UPROPERTY()
    FGuid NodeId;

    UPROPERTY()
    TArray<UFlowExecScriptGraphNode*> OutgoingNodes;

    UPROPERTY()
    TArray<UFlowExecScriptGraphNode*> IncomingNodes;
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UFlowExecScriptTaskNode : public UFlowExecScriptGraphNode {
    GENERATED_BODY()
public:
    UPROPERTY()
    UFlowExecTask* Task;
};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UFlowExecScriptResultNode : public UFlowExecScriptGraphNode {
    GENERATED_BODY()
public:

};

UCLASS()
class DUNGEONARCHITECTRUNTIME_API UFlowExecScriptGraph : public UObject {
    GENERATED_BODY()
public:
    UPROPERTY()
    TArray<UFlowExecScriptGraphNode*> Nodes;
};


UCLASS()
class DUNGEONARCHITECTRUNTIME_API UFlowExecScript : public UObject {
    GENERATED_BODY()
public:
    UPROPERTY()
    UFlowExecScriptGraph* ScriptGraph;

    UPROPERTY()
    UFlowExecScriptResultNode* ResultNode;
};

