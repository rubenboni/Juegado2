//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Core/Editors/FlowEditor/DomainEditors/Common/Viewport3D/FlowDomainEdViewport3D.h"

class DUNGEONARCHITECTEDITOR_API FFlowDomainEdViewportClient3D
	: public FEditorViewportClient
	, public TSharedFromThis<FFlowDomainEdViewportClient3D>
{
public:
	FFlowDomainEdViewportClient3D(FPreviewScene& InPreviewScene, const TWeakPtr<SEditorViewport>& InEditorViewport);
	FFDViewportActorMouseEvent& GetActorSelectionChanged() { return ActorSelectionChanged; }
	FFDViewportActorMouseEvent& GetActorDoubleClicked() { return ActorDoubleClicked; }
    
	// FEditorViewportClient interface
	virtual void Tick(float DeltaSeconds) override;
	virtual void ProcessClick(FSceneView& View, HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;
	virtual void SetupViewForRendering(FSceneViewFamily& ViewFamily, FSceneView& View) override;
	// End of FEditorViewportClient interface

	private:
	FFDViewportActorMouseEvent ActorSelectionChanged;
	FFDViewportActorMouseEvent ActorDoubleClicked;
};


