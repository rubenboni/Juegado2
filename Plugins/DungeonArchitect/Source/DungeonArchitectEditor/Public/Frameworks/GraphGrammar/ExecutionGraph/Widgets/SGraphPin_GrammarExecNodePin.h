//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "SGraphPin.h"

class DUNGEONARCHITECTEDITOR_API SGraphPin_GrammarExecNodePin : public SGraphPin {
public:
    SLATE_BEGIN_ARGS(SGraphPin_GrammarExecNodePin) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UEdGraphPin* InPin);
protected:
    // Begin SGraphPin interface
    virtual TSharedRef<SWidget> GetDefaultValueWidget() override;
    // End SGraphPin interface

    /** @return The color that we should use to draw this pin */
    virtual FSlateColor GetPinColor() const override;

    // End SGraphPin interface

    const FSlateBrush* GetPinBorder() const;
};

