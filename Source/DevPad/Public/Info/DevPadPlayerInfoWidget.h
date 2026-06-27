#pragma once

#include "DevPadTypes.h"
#include "DevPadPlayerInfoWidget.generated.h"

UCLASS(Abstract, Category = "DevHub|Pad")
class UDevPadPlayerInfoWidget : public UDevPadInfoWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "DevHub|Pad")
	FText GetInfoText(const FString& Format = TEXT("{Mode}: {Name} ({Class}), {Location}, {Camera}, Slomo: {Slomo}")) const;

protected:
	//~ Begin UUserWidget Interface.
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~ End UUserWidget Interface.

	FStringFormatNamedArguments FormatArguments;
};
