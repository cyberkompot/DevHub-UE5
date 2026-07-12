// Copyright (c) Alexandr Pereverzev.

#include "DevMenuWidgetBuilder.h"

#include "DevMenuGenerator.h"
#include "DevMenuLogging.h"
#include "DevMenuUtils.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/SDevMenuMainMenuWidget.h"

TSharedRef<SWidget> UDevMenuWidgetBuilder::MakeMainMenuWidget(const FName InMenuPath)
{
	UDevMenu* GeneratedMenu = MenuGenerator->GenerateMenu(InMenuPath);
	return MakeMainMenuWidget(*GeneratedMenu);
}

TSharedRef<SWidget> UDevMenuWidgetBuilder::MakeMainMenuWidget(UDevMenu& InGeneratedMenu)
{
	UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Make main menu widget: Path = %s"), *InGeneratedMenu.GetEntryName().ToString());

	FMenuBarBuilder MenuBuilder(TSharedPtr<FUICommandList>(), TSharedPtr<FExtender>(), &FCoreStyle::Get(), InGeneratedMenu.GetEntryName());
	PopulateMainMenu(MenuBuilder, &InGeneratedMenu);
	const TSharedRef<SWidget> MultiBoxWidget = MenuBuilder.MakeWidget();

	const TSharedRef<SDevMenuMainMenuWidget> MainMenuWidget = SNew(SDevMenuMainMenuWidget)
		.GeneratedMenu(&InGeneratedMenu)
		.MenuPath(InGeneratedMenu.GetEntryName())
		[
			MultiBoxWidget
		];
	return MainMenuWidget;
}

TSharedRef<SWidget> UDevMenuWidgetBuilder::MakeSubMenuWidget(const FName InMenuPath)
{
	UDevMenu* GeneratedMenu = MenuGenerator->GenerateMenu(InMenuPath);
	return MakeSubMenuWidget(*GeneratedMenu);
}

TSharedRef<SWidget> UDevMenuWidgetBuilder::MakeSubMenuWidget(UDevMenu& InGeneratedMenu)
{
	UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Make sub menu widget: Path = %s"), *InGeneratedMenu.GetEntryName().ToString());

	FMenuBuilder MenuBuilder(true, TSharedPtr<FUICommandList>(), TSharedPtr<FExtender>(), false, &FCoreStyle::Get(), false, InGeneratedMenu.GetEntryName());
	PopulateSubMenu(MenuBuilder, &InGeneratedMenu);
	const TSharedRef<SWidget> MultiBoxWidget = MenuBuilder.MakeWidget();

	// TODO: Add wrapper widget for Keyboard and Gamepad navigation.
	return MultiBoxWidget;
}

void UDevMenuWidgetBuilder::PopulateMainMenu(FMenuBarBuilder& InMenuBuilder, const FName InMenuPath)
{
	UDevMenu* GeneratedMenu = MenuGenerator->GenerateMenu(InMenuPath);
	PopulateMainMenu(InMenuBuilder, GeneratedMenu);
}

void UDevMenuWidgetBuilder::PopulateMainMenu(FMenuBarBuilder& InMenuBuilder, UDevMenu* InGeneratedMenu)
{
	UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Populating main menu builder: Path = %s"), *InGeneratedMenu->GetEntryName().ToString());

	const TSharedRef<FMultiBox> MultiBox = InMenuBuilder.GetMultiBox();
	MultiBox->WeakToolMenu = InGeneratedMenu;
	MultiBox->ModifyBlockWidgetAfterMake.BindUObject(this, &ThisClass::OnModifyBlockWidgetAfterMake, TWeakObjectPtr<UDevMenu>(InGeneratedMenu));

	// TODO: Introduce a facade for FMenuBuilder and FMenuBarBuilder in FDevMenuWidgetBuilderContext to menu population unification.
	FDevMenuInstancedEntries* MenuInstancedEntries = InGeneratedMenu->GetSubEntries();
	for (FDevMenuInstancedEntry& MenuInstancedEntry : *MenuInstancedEntries)
	{
		FDevMenuEntry& MenuEntry = MenuInstancedEntry.GetMutable<FDevMenuEntry>();
		const FName MenuEntryPath = FDevMenuPaths::Combine(InGeneratedMenu->GetEntryName(), MenuEntry.GetEntryName());

		FMenuEntryParams EntryParams;
		EntryParams.ExtensionHook = MenuEntry.GetEntryName();
		EntryParams.Type = EMultiBlockType::MenuEntry;
		EntryParams.LabelOverride = MenuEntry.GetLabel(InGeneratedMenu);
		EntryParams.ToolTipOverride = MenuEntry.GetToolTip(InGeneratedMenu);
		EntryParams.EntryBuilder = FNewMenuDelegate::CreateUObject(this, &ThisClass::PopulateSubMenu, MenuEntryPath);
		EntryParams.UserInterfaceActionType = EUserInterfaceActionType::Button;
		EntryParams.bIsSubMenu = false; // Menu bar items cannot be sub-menus.
		EntryParams.bShouldCloseWindowAfterMenuSelection = false;
		EntryParams.DirectActions.ExecuteAction.BindLambda([]{}); // FMultiBlock expects valid Action or bounded ExecuteAction.
		InMenuBuilder.AddMenuEntry(EntryParams);
	}
	AddEmptyEntryIfNeeded(InMenuBuilder, InGeneratedMenu);

	WidgetObjectReferences.AddUnique(MultiBox);
}

void UDevMenuWidgetBuilder::PopulateSubMenu(FMenuBuilder& InMenuBuilder, const FName InMenuPath)
{
	UDevMenu* GeneratedMenu = MenuGenerator->GenerateMenu(InMenuPath);
	PopulateSubMenu(InMenuBuilder, GeneratedMenu);
}

void UDevMenuWidgetBuilder::PopulateSubMenu(FMenuBuilder& InMenuBuilder, UDevMenu* InGeneratedMenu)
{
	UE_LOG_FUNCTION(LogDevMenus, Verbose, TEXT("Populating sub menu builder: Path = %s"), *InGeneratedMenu->GetEntryName().ToString());

	const TSharedRef<FMultiBox> MultiBox = InMenuBuilder.GetMultiBox();
	MultiBox->WeakToolMenu = InGeneratedMenu;
	MultiBox->ModifyBlockWidgetAfterMake.BindUObject(this, &ThisClass::OnModifyBlockWidgetAfterMake, TWeakObjectPtr<UDevMenu>(InGeneratedMenu));

	FDevMenuWidgetBuilderContext Context(*this, InMenuBuilder, *InGeneratedMenu);
	InGeneratedMenu->PopulateMenuBuilder(Context);
	AddEmptyEntryIfNeeded(InMenuBuilder, InGeneratedMenu);

	WidgetObjectReferences.AddUnique(MultiBox);
}

void UDevMenuWidgetBuilder::AddEmptyEntryIfNeeded(FMenuBarBuilder& InMenuBuilder, UDevMenu* InGeneratedMenu) const
{
	if (!InMenuBuilder.GetMultiBox()->GetBlocks().IsEmpty()) { return; }

	FMenuEntryParams EntryParams;
	EntryParams.ExtensionHook = TEXT("EmptyMenu");
	EntryParams.Type = EMultiBlockType::MenuEntry;
	EntryParams.LabelOverride = INVTEXT("Empty");
	EntryParams.UserInterfaceActionType = EUserInterfaceActionType::None;
	EntryParams.DirectActions.ExecuteAction.BindLambda([]{}); // FMultiBlock expects valid Action or bounded ExecuteAction.
	EntryParams.DirectActions.CanExecuteAction.BindLambda([]{ return false; });
	InMenuBuilder.AddMenuEntry(EntryParams);
}

void UDevMenuWidgetBuilder::AddEmptyEntryIfNeeded(FMenuBuilder& InMenuBuilder, UDevMenu* InGeneratedMenu) const
{
	if (!InMenuBuilder.GetMultiBox()->GetBlocks().IsEmpty()) { return; }

	InMenuBuilder.AddWidget(
		SNew(STextBlock)
		.Margin(FMargin(32.0f, 0.0f))
		.Text(INVTEXT("Empty"))
		.TextStyle(FAppStyle::Get(), TEXT("HintText")),
		FText::GetEmpty(), /*bNoIndent*/ true, /*bSearchable*/ false);
}

TSharedRef<SWidget> UDevMenuWidgetBuilder::OnModifyBlockWidgetAfterMake(const TSharedRef<SMultiBoxWidget>& InMultiBoxWidget, const FMultiBlock& InMultiBlock, const TSharedRef<SWidget>& InWidget, TWeakObjectPtr<UDevMenu> InGeneratedMenu)
{
	// Placeholder for adding metadata to widget.
	return InWidget;
}

void UDevMenuWidgetBuilder::Dispose()
{
	WidgetObjectReferences.Empty();
}

void UDevMenuWidgetBuilder::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(InThis, Collector);

	UDevMenuWidgetBuilder* This = Cast<UDevMenuWidgetBuilder>(InThis);
	if (!This) { return; }

	TArray<TWeakPtr<FMultiBox>>& WidgetObjectReferences = This->WidgetObjectReferences;
	for (int i = WidgetObjectReferences.Num() - 1; i >= 0; --i)
	{
		if (const TSharedPtr<FMultiBox> MultiBox = WidgetObjectReferences[i].Pin())
		{
#if UE_COMPATIBILITY_REFERENCE_COLLECTOR_ADD_REFERENCED_OBJECT_WEAK_OBJECT_PTR
			Collector.AddReferencedObject(MultiBox->WeakToolMenu, InThis);
			if (MultiBox->WeakToolMenu.IsValid()) { continue; } // Continue to skip removing.
#else
			UToolMenuBase* Ptr = MultiBox->WeakToolMenu.GetEvenIfUnreachable();
			Collector.AddReferencedObject(Ptr, InThis);
			MultiBox->WeakToolMenu = Ptr;
			if (Ptr) { continue; } // Continue to skip removing.
#endif // UE_COMPATIBILITY_REFERENCE_COLLECTOR_ADD_REFERENCED_OBJECT_WEAK_OBJECT_PTR
		}

		// Remove from object references if the widget or menu object has expired.
		WidgetObjectReferences.RemoveAtSwap(i);
	}
}
