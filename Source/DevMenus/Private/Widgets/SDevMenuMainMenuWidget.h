// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "SDevMenuNavigationExtensionWidget.h"

class UDevMenu;

class SDevMenuMainMenuWidget final : public SDevMenuNavigationExtensionWidgetBase
{
	SLATE_DECLARE_WIDGET(SDevMenuMainMenuWidget, SDevMenuNavigationExtensionWidgetBase)

public:
	SLATE_BEGIN_ARGS(SDevMenuMainMenuWidget) {}
		SLATE_ARGUMENT_DEFAULT(TObjectPtr<UDevMenu>, GeneratedMenu);
		SLATE_ARGUMENT_DEFAULT(FName, MenuPath);
		SLATE_DEFAULT_SLOT(FArguments, Content);
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual FName GetMenuPath() const override { return MenuPath; }

private:
	TWeakObjectPtr<UDevMenu> GeneratedMenu;
	FName MenuPath;
};
