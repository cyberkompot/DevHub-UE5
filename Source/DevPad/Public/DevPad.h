// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCoreGameInstanceSubsystem.h"
#include "DevPad.generated.h"

class UDevPadPage;
class UDevPadManager;
class UDevPadRegistry;

UCLASS(NotBlueprintable, Category = "DevHub|Pad", DisplayName = "DevPad")
class DEVPAD_API UDevPad final : public UDevCoreGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UDevPad* Get(const UObject* WorldContextObject) { return UDevCoreGameInstanceSubsystem::Get<UDevPad>(WorldContextObject); }

public:
	UFUNCTION(BlueprintPure, Category = "DevHub|Pad")
	bool IsPadVisible() const;

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void ShowPad(const FName InPageName = NAME_None) const;

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void TogglePad() const;

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void HidePad() const;

public:
	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void NavigateToPreviousPage() const;

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void NavigateToNextPage() const;

public:
	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void OpenSubPage(UDevPadPage* InPage) const;

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void CloseSubPage(UDevPadPage* InPage) const;

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void CloseCurrentSubPage() const;

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void CloseAllSubPages() const;

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

private:
	UDevPad();

	UPROPERTY(Transient)
	TObjectPtr<UDevPadManager> PadManager = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UDevPadRegistry> PadRegistry = nullptr;

	void BindShortcut();
	void UnbindShortcut() const;

	void OnShortcutTriggered();
};
