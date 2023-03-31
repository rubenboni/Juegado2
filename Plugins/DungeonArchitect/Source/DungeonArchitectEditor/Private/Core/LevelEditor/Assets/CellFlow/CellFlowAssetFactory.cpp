//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/LevelEditor/Assets/CellFlow/CellFlowAssetFactory.h"

#include "Builders/CellFlow/CellFlowAsset.h"
#include "Core/Editors/FlowEditor/FlowEditorUtils.h"

UCellFlowAssetFactory::UCellFlowAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	SupportedClass = UCellFlowAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UCellFlowAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) {
	UCellFlowAsset* NewAsset = NewObject<UCellFlowAsset>(InParent, Class, Name, Flags | RF_Transactional);
	FFlowEditorUtils::InitializeFlowAsset(NewAsset);
	NewAsset->Version = static_cast<int>(ECellFlowAssetVersion::LatestVersion);
	return NewAsset;
}

bool UCellFlowAssetFactory::CanCreateNew() const {
	return true;
}
