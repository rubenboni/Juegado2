//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Core/Editors/ThemeEditor/Widgets/SDungeonOutputPin.h"

#include "Core/Editors/ThemeEditor/AppModes/ThemeGraph/Graph/EdGraph_DungeonProp.h"

void SDungeonOutputPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin) {
    this->SetCursor(EMouseCursor::Default);

    bShowLabel = true;
    IsEditable = true;

    GraphPinObj = InPin;
    check(GraphPinObj != NULL);

    const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
    check(Schema);

    // Set up a hover for pins that is tinted the color of the pin.
    SBorder::Construct(SBorder::FArguments()
                       .BorderImage(this, &SDungeonOutputPin::GetPinBorder)
                       .BorderBackgroundColor(this, &SDungeonOutputPin::GetPinColor)
                       .OnMouseButtonDown(this, &SDungeonOutputPin::OnPinMouseDown)
                       .Cursor(this, &SDungeonOutputPin::GetPinCursor)
                       .Padding(FMargin(5.0f))
    );
}

TSharedRef<SWidget> SDungeonOutputPin::GetDefaultValueWidget() {
    return SNew(STextBlock);
}

const FSlateBrush* SDungeonOutputPin::GetPinBorder() const {
    return FAppStyle::GetBrush(TEXT("Graph.StateNode.Body"));
}

FSlateColor SDungeonOutputPin::GetPinColor() const {
    static const FLinearColor MeshPinColor(1.0f, 0.7f, 0.0f);
    static const FLinearColor MarkerPinColor(0.3f, 0.3f, 1.0f);
    static const FLinearColor DarkColor(0.02f, 0.02f, 0.02f);
    if (!IsHovered()) {
        return DarkColor;
    }
    bool IsMarker = (GraphPinObj->PinType.PinCategory == FDungeonDataTypes::PinType_Marker);
    return IsMarker ? MarkerPinColor : MeshPinColor;
}

