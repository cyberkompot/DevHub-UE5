// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "DevCore.h"
#include "DevCoreTypeTraits.h"
#include UE_COMPATIBILITY_INCLUDE_INSTANCED_STRUCT_PATH
#include UE_COMPATIBILITY_INCLUDE_VIEWPORT_CLIENT_PATH

#include "DevActionTypes.h"
#include "DevInputTypes.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/ToolMenuBase.h"
#include "Misc/Attribute.h"
#include "Templates/ChooseClass.h"
#include "Templates/UnrealTemplate.h"
#include "Templates/UnrealTypeTraits.h"
#include "DevMenuTypes.generated.h"

struct FDevAction;
struct FDevMenuEntry;
struct FDevMenuEntryId;
struct IDevMenuEntriesIterator;
struct IDevMenuEntriesQuery;
struct IDevMenuBuilderContext;
class UDevMenu;

using FDevMenuMenus = TArray<TObjectPtr<UDevMenu>>;
using FDevMenuInstancedEntry = FInstancedStruct;
using FDevMenuInstancedEntries = TArray<FDevMenuInstancedEntry>;

UENUM(BlueprintType, Category = "DevHub|Menu", Meta = (Bitflags))
enum struct EDevMenuEntryFlags : uint8
{
	NoFlags = 0 UMETA(Hidden),

	SubMenu = 1,
	Section = 2,
	Embedded = 4,

	StaticEntries = 8,
	DynamicEntries = 16,

	DynamicEntry = 32,
	ProxyEntry = 64,
	TempEntry = 128,
};

ENUM_CLASS_FLAGS(EDevMenuEntryFlags);


UENUM(BlueprintType, Category = "DevHub|Menu")
enum struct EDevMenuEntryLayout : uint8
{
	NoLayout = 0 UMETA(Hidden),

	SubMenu = 1,
	Section = 2,
	Embedded = 3,
};


struct DEVMENUS_API IDevMenuEntry
{
	//~ Begin IDevMenuEntry interface.
	virtual FDevMenuEntryId GetEntryId() const;
	virtual FName GetEntryName() const;
	virtual EDevMenuEntryFlags GetEntryFlags() const;
	virtual UStruct* GetEntryType() const;

	virtual TAttribute<FText> GetLabel() const;
	virtual TAttribute<FText> GetToolTip() const;
	virtual TAttribute<FDevInputShortcut> GetInputShortcut() const;
	virtual ECheckBoxState GetCheckState(const UObject* WorldContextObject) const;
	virtual bool IsEnabled(const UObject* WorldContextObject) const;
	virtual bool IsVisible(const UObject* WorldContextObject) const;

	virtual FDevMenuInstancedEntries* GetSubEntries();
	virtual void QuerySubEntries(const IDevMenuEntriesQuery& InQuery);

	virtual void ExecuteEntry(const UObject* WorldContextObject);

	virtual void SetEntryId(const FDevMenuEntryId InId);
	virtual void SetEntryName(const FName InName);

	virtual FName GetEntryPath() const;
	virtual void ResolveEntryPath();

	virtual void PopulateMenuBuilder(IDevMenuBuilderContext& InContext);

	virtual FDevMenuInstancedEntry DuplicateEntry() const;
	//~ End IDevMenuEntry interface.

	template<
		typename TEntryType,
		typename TEntryPtr,
		typename = typename TEnableIf<
			TIsDerivedFrom<typename TRemoveConst<TEntryType>::Type, IDevMenuEntry>::IsDerived &&
			TIsDerivedFrom<typename TRemoveConst<TEntryPtr>::Type, IDevMenuEntry>::IsDerived>::Type>
	FORCEINLINE static typename TChooseClass<TIsConst<TEntryPtr>::Value, const TEntryType*,	TEntryType*>::Result CastTo(TEntryPtr* InEntry)
	{
		using ReturnType = typename TChooseClass<TIsConst<TEntryPtr>::Value, const TEntryType*, TEntryType*>::Result;
		return (InEntry && InEntry->template IsChildOf<TEntryType>()) ? static_cast<ReturnType>(InEntry) : nullptr;
	}

	template<
		typename TEntryType,
		typename = typename TEnableIf<TIsDerivedFrom<typename TRemoveConst<TEntryType>::Type, IDevMenuEntry>::IsDerived>::Type>
	FORCEINLINE static TEntryType* CastTo(FDevMenuInstancedEntry& InInstancedEntry) { return CastTo<TEntryType>(InInstancedEntry.GetMutablePtr<FDevMenuEntry>()); }

	template<typename TEntryType>
	FORCEINLINE TEntryType* CastTo() { return (IsChildOf<TEntryType>()) ? static_cast<TEntryType*>(this) : nullptr; }

	FORCEINLINE bool HasAnyEntryFlags(const EDevMenuEntryFlags InFlags) const { return !!(GetEntryFlags() & InFlags); }
	FORCEINLINE bool HasAllEntryFlags(const EDevMenuEntryFlags InFlags) const { return ((GetEntryFlags() & InFlags) == InFlags); }

	template<typename TEntryType>
	FORCEINLINE bool IsChildOf() const
	{
		if (const UStruct* EntryType = GetEntryType())
		{
			if constexpr (TIsDerivedFrom<TEntryType, UObject>::Value)
			{
				return EntryType->IsChildOf(TEntryType::StaticClass());
			}
			else
			{
				return EntryType->IsChildOf(TEntryType::StaticStruct());
			}
		}
		return false;
	}

	FORCEINLINE operator FDevMenuInstancedEntry() const { return DuplicateEntry(); }

	virtual ~IDevMenuEntry() = default;
};


/** Base struct for building menu entries. Inheriting structs should provide the logic for virtual methods. */
USTRUCT(BlueprintType, NotBlueprintable, Category = "DevHub|Menu", Meta = (Hidden))
struct DEVMENUS_API FDevMenuEntry
#if CPP
	: public IDevMenuEntry
#endif
{
	GENERATED_BODY()

	virtual ~FDevMenuEntry() override {};
};


USTRUCT(BlueprintType, Category = "DevHub|Menu", Meta = (ShowOnlyInnerProperties))
struct DEVMENUS_API FDevMenuEntryId
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "DevHub|Menu", DisplayName = "Entry ID")
	int32 Id = 0;

	static const FDevMenuEntryId Empty;

	constexpr FDevMenuEntryId() = default;
	constexpr FDevMenuEntryId(const FDevMenuEntryId&) = default;
	constexpr FDevMenuEntryId(FDevMenuEntryId&&) = default;
	constexpr FDevMenuEntryId(const int32 Value) : Id(Value) {}

	static FDevMenuEntryId NewEntryId();

	FORCEINLINE bool IsValid() const { return (Id != 0); }

	FORCEINLINE FDevMenuEntryId& Resolve() { if (Id == 0) { Id = NewEntryId().Id; } return *this; }
	FORCEINLINE void Invalidate() { Id = 0; }

	FORCEINLINE FName ToName() const { return (Id != 0) ? FName("ID", NAME_EXTERNAL_TO_INTERNAL(Id)) : NAME_None; }
	FORCEINLINE FString ToString() const { return FString::FromInt(Id); }

	FORCEINLINE bool ExportTextItem(FString& ValueStr, const FDevMenuEntryId& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const { ValueStr.AppendInt(Id); return true; }
	FORCEINLINE bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText) {	Id = FCString::Atoi(Buffer); return true; }

	FDevMenuEntryId& operator =(const FDevMenuEntryId&) = default;
	FDevMenuEntryId& operator =(FDevMenuEntryId&&) = default;
	FORCEINLINE bool operator ==(const FDevMenuEntryId& Rhs) const { return (Id == Rhs.Id); }
	FORCEINLINE bool operator !=(const FDevMenuEntryId& Rhs) const { return (Id != Rhs.Id); }
	FORCEINLINE bool operator <(const FDevMenuEntryId& Rhs) const { return (Id < Rhs.Id); }

	FORCEINLINE operator int32() const { return Id; }
	FORCEINLINE FDevMenuEntryId& operator =(const int32 Value) { Id = Value; return *this; }
	FORCEINLINE bool operator ==(const int32 Value) const { return (Id == Value); }
	FORCEINLINE bool operator !=(const int32 Value) const { return (Id != Value); }
	FORCEINLINE bool operator <(const int32 Value) const { return (Id < Value); }

	FORCEINLINE friend uint32 GetTypeHash(const FDevMenuEntryId& Value) { return static_cast<uint32>(Value.Id); }
};

template <> struct TStructOpsTypeTraits<FDevMenuEntryId> : TStructOpsTypeTraitsBase2<FDevMenuEntryId>
{
	enum
	{
		WithExportTextItem = true,
		WithImportTextItem = true,
	};
};


struct DEVMENUS_API IDevMenuEntriesIterator
{
	//~ Begin IDevMenuEntriesIterator interface.
	virtual void operator ++() = 0;
	virtual operator bool() const = 0;
	virtual IDevMenuEntry* operator *() = 0;
	//~ End IDevMenuEntriesIterator interface.

	FORCEINLINE IDevMenuEntry* operator ->() { return **this; }

	virtual ~IDevMenuEntriesIterator() = default;
};


struct DEVMENUS_API FDevMenuQueryResult final
{
	const static FDevMenuQueryResult Empty;

	FDevMenuQueryResult() = default;
	FDevMenuQueryResult(UDevMenu* InMenu, IDevMenuEntry* InOuter, IDevMenuEntry* InEntry, const FString& Path)
		: Menu(InMenu), Outer(InOuter), Entry(InEntry), Path(Path) {}
	FDevMenuQueryResult(UDevMenu* InMenu, IDevMenuEntry* InOuter, IDevMenuEntry* InEntry, FString&& Path)
		: Menu(InMenu), Outer(InOuter), Entry(InEntry), Path(Forward<FString>(Path)) {}

	UDevMenu* Menu = nullptr;
	IDevMenuEntry* Outer = nullptr;
	IDevMenuEntry* Entry = nullptr;
	FString Path;
};

UENUM(BlueprintType, Category = "DevHub|Menu")
enum struct EDevMenuQueryExecution : uint8
{
	Break = 0,
	Continue = 1,
};

/** Delegate used by .... */
DEVMENUS_API DECLARE_DELEGATE_RetVal_OneParam(EDevMenuQueryExecution, FDevMenuQueryDelegate, const FDevMenuQueryResult&);

struct DEVMENUS_API IDevMenuEntriesQuery
{
	virtual void ExecuteQuery(IDevMenuEntriesIterator& InEntriesIterator) const = 0;

	virtual ~IDevMenuEntriesQuery() = default;
};


struct DEVMENUS_API IDevMenuBuilderContext
{
	IDevMenuBuilderContext(FMenuBuilder& InMenuBuilder, UDevMenu& InGeneratedMenu, UObject* InWorldContextObject)
		: GeneratedMenu(InGeneratedMenu), MenuBuilder(InMenuBuilder), WorldContextObject(InWorldContextObject) {}
	virtual ~IDevMenuBuilderContext() = default;

	virtual UDevMenu& GetGeneratedMenu() const { return GeneratedMenu; }
	virtual FMenuBuilder& GetMenuBuilder() const { return MenuBuilder; }
	virtual UObject* GetWorldContextObject() const { return WorldContextObject; }

	virtual void PopulateMenuBuilder(IDevMenuEntriesIterator& InEntriesIterator);

	virtual FCanExecuteAction CreateGetEntryEnabledStateDelegate(const IDevMenuEntry& InEntry);
	virtual FGetActionCheckState CreateGetEntryCheckStateDelegate(const IDevMenuEntry& InEntry) const;
	virtual FIsActionButtonVisible CreateGetEntryVisibilityDelegate(const IDevMenuEntry& InEntry) const;
	virtual FExecuteAction CreateExecuteEntryDelegate(IDevMenuEntry& InEntry) const;

	virtual FNewMenuDelegate CreatePopulateNewMenuDelegate(IDevMenuEntry& InEntry) const = 0;

private:
	UDevMenu& GeneratedMenu;
	FMenuBuilder& MenuBuilder;
	UObject* WorldContextObject;
};


UENUM(BlueprintType, Category = "DevHub|Menu")
enum struct EDevMenuDynamicEntriesCachePolicy : uint8
{
	CacheNotAllowed = 0,
	CacheAllowed = 1,
};

USTRUCT(NotBlueprintable, NotBlueprintType, Category = "DevHub|Menu", Meta = (Hidden))
struct DEVMENUS_API FDevMenuDynamicEntriesCache
{
	GENERATED_BODY()

	~FDevMenuDynamicEntriesCache();

	FORCEINLINE bool IsValid() const { return bIsSet; }
	FORCEINLINE bool IsValid(const uint32 InValidationHash) const { return (bIsSet && (InValidationHash == ValidationHash)); }

	FDevMenuInstancedEntries& Get();
	void Set(const FDevMenuInstancedEntries& InDynamicEntries, const uint32 InValidationHash = 0);
	void Set(FDevMenuInstancedEntries&& InDynamicEntries, const uint32 InValidationHash = 0);
	void Reset();

private:
	UPROPERTY(Transient, SkipSerialization)
	bool bIsSet = false;

	UPROPERTY(Transient, SkipSerialization, Meta = (BaseStruct = "/Script/DevMenus.DevMenuEntry", ExcludeBaseStruct))
	TArray<FInstancedStruct> DynamicEntries;

	UPROPERTY(Transient, SkipSerialization)
	uint32 ValidationHash = 0;
};


USTRUCT(BlueprintType, NotBlueprintable, Category = "DevHub|Menu", Meta = (Hidden))
struct DEVMENUS_API FDevMenuItemBase : public FDevMenuEntry
{
	GENERATED_BODY()

	/** Menu entry ID. */
	UPROPERTY(VisibleAnywhere, Category = "Dev Menu", DisplayName = "Entry ID")
	FDevMenuEntryId EntryId;

	/** Menu entry name. */
	UPROPERTY(EditAnywhere, Category = "Dev Menu")
	FName EntryName;

	/** Menu entry label. */
	UPROPERTY(EditAnywhere, Category = "Dev Menu")
	FText Label;

	/** Menu entry tooltip. */
	UPROPERTY(EditAnywhere, Category = "Dev Menu")
	FText ToolTip;

	//~ Begin IDevMenuEntry interface.
	virtual FDevMenuEntryId GetEntryId() const override { return EntryId; }
	virtual FName GetEntryName() const override { return EntryName; }
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };

	virtual TAttribute<FText> GetLabel() const override;
	virtual TAttribute<FText> GetToolTip() const override { return ToolTip; }

	virtual void SetEntryId(const FDevMenuEntryId InId) override { EntryId = InId; }
	virtual void SetEntryName(const FName InName) override { EntryName = InName; }

	virtual void ResolveEntryPath() override { EntryId.Resolve(); }
	//~ End IDevMenuEntry interface.

private:
	using Super = FDevMenuEntry;
};


USTRUCT(NotBlueprintType, NotBlueprintable, Category = "DevHub|Menu", Meta = (Hidden))
struct DEVMENUS_API FDevMenuExecutionBase : public FDevMenuItemBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "DevHub|Menu", Meta = (DisplayAfter = "ToolTip"))
	FDevInputShortcut InputShortcut;

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };
	virtual TAttribute<FDevInputShortcut> GetInputShortcut() const override { return InputShortcut; }
	virtual void PopulateMenuBuilder(IDevMenuBuilderContext& InContext) override;
	//~ End IDevMenuEntry interface.

	virtual EUserInterfaceActionType GetUserInterfaceType(const UObject* WorldContextObject) const { return EUserInterfaceActionType::Button; }

private:
	using Super = FDevMenuItemBase;
};


USTRUCT(NotBlueprintable, NotBlueprintType, Category = "DevHub|Menu", Meta = (Hidden))
struct DEVMENUS_API FDevMenuOuterBase : public FDevMenuItemBase
{
	GENERATED_BODY()

	static EDevMenuEntryFlags GetEntryFlagByLayout(const EDevMenuEntryLayout InLayout)
	{
		switch (InLayout)
		{
			case EDevMenuEntryLayout::SubMenu: return EDevMenuEntryFlags::SubMenu;
			case EDevMenuEntryLayout::Section: return EDevMenuEntryFlags::Section;
			case EDevMenuEntryLayout::Embedded: return EDevMenuEntryFlags::Embedded;
			case EDevMenuEntryLayout::NoLayout: return EDevMenuEntryFlags::NoFlags;
			default: return EDevMenuEntryFlags::NoFlags;
		}
	}

	virtual EDevMenuEntryLayout GetOuterLayout() const { return EDevMenuEntryLayout::SubMenu; };

	//~ Begin FDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };
	virtual EDevMenuEntryFlags GetEntryFlags() const override { return Super::GetEntryFlags() | GetEntryFlagByLayout(GetOuterLayout()); };
	//~ End FDevMenuEntry interface.

private:
	using Super = FDevMenuItemBase;
};


USTRUCT(NotBlueprintable, NotBlueprintType, Category = "DevHub|Menu", Meta = (Hidden))
struct DEVMENUS_API FDevMenuStaticOuterBase : public FDevMenuOuterBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "DevHub|Menu", Meta = (BaseStruct = "/Script/DevMenus.DevMenuEntry", ExcludeBaseStruct, DisplayAfter = "ToolTip"))
	TArray<FInstancedStruct> Entries;

	//~ Begin FDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };
	virtual EDevMenuEntryFlags GetEntryFlags() const override { return Super::GetEntryFlags() | EDevMenuEntryFlags::StaticEntries; };

	virtual FDevMenuInstancedEntries* GetSubEntries() override { return &Entries; };
	virtual void QuerySubEntries(const IDevMenuEntriesQuery& InQuery) override;

	virtual void ResolveEntryPath() override;
	//~ End FDevMenuEntry interface.

private:
	using Super = FDevMenuOuterBase;
};


USTRUCT(NotBlueprintable, NotBlueprintType, Category = "DevHub|Menu", Meta = (Hidden))
struct DEVMENUS_API FDevMenuStaticOuterWithLayoutBase : public FDevMenuStaticOuterBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Dev Menu", Meta = (DisplayAfter = "ToolTip"))
	EDevMenuEntryLayout Layout = EDevMenuEntryLayout::SubMenu;

	//~ Begin FDevMenuOuterBase interface.
	virtual EDevMenuEntryLayout GetOuterLayout() const override { return Layout; };
	//~ End FDevMenuOuterBase interface.

	//~ Begin FDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };
	//~ End FDevMenuEntry interface.

private:
	using Super = FDevMenuStaticOuterBase;
};


USTRUCT(NotBlueprintable, NotBlueprintType, Category = "DevHub|Menu", Meta = (Hidden))
struct DEVMENUS_API FDevMenuDynamicOuterBase : public FDevMenuOuterBase
{
	GENERATED_BODY()

	FDevMenuInstancedEntries& GetDynamicEntries();

	//~ Begin FDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };
	virtual EDevMenuEntryFlags GetEntryFlags() const override { return Super::GetEntryFlags() | EDevMenuEntryFlags::DynamicEntries; };

	virtual FDevMenuInstancedEntries* GetSubEntries() override { return &GetDynamicEntries(); };
	virtual void QuerySubEntries(const IDevMenuEntriesQuery& InQuery) override;
	//~ End FDevMenuEntry interface.

protected:
	UPROPERTY(Transient, SkipSerialization)
	FDevMenuDynamicEntriesCache DynamicEntriesCache;

	virtual FDevMenuInstancedEntries CreateDynamicEntries() const { return FDevMenuInstancedEntries(); };
	virtual EDevMenuDynamicEntriesCachePolicy GetDynamicEntriesCachingPolicy() const { return EDevMenuDynamicEntriesCachePolicy::CacheAllowed; };
	virtual int32 GetDynamicEntriesValidationHash() const { return 0; }

private:
	using Super = FDevMenuOuterBase;
};


USTRUCT(NotBlueprintable, NotBlueprintType, Category = "DevHub|Menu", Meta = (Hidden))
struct DEVMENUS_API FDevMenuDynamicOuterWithLayoutBase : public FDevMenuDynamicOuterBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Dev Menu", Meta = (DisplayAfter = "ToolTip"))
	EDevMenuEntryLayout Layout = EDevMenuEntryLayout::SubMenu;

	//~ Begin FDevMenuOuterBase interface.
	virtual EDevMenuEntryLayout GetOuterLayout() const override { return Layout; };
	//~ End FDevMenuOuterBase interface.

	//~ Begin FDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };
	//~ End FDevMenuEntry interface.

private:
	using Super = FDevMenuDynamicOuterBase;
};


USTRUCT(NotBlueprintType, NotBlueprintable, Category = "DevHub|Menu", Meta = (Hidden))
struct DEVMENUS_API FDevMenuProxy : public FDevMenuEntry
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Dev Menu", DisplayName = "Entry ID")
	FDevMenuEntryId EntryId;

	UPROPERTY(EditAnywhere, Category = "Dev Menu")
	FName EntryName;

	UPROPERTY(EditAnywhere, Category = "Dev Menu")
	TObjectPtr<UDevMenu> Menu;

	IDevMenuEntry* GetProxyEntry() const;

	FORCEINLINE bool IsValid() const { return (!!Menu); }

	//~ Begin IDevMenuEntry interface.
	virtual FDevMenuEntryId GetEntryId() const override { return EntryId; }
	virtual FName GetEntryName() const override { return EntryName; }
	virtual EDevMenuEntryFlags GetEntryFlags() const override { return ((IsValid()) ? GetProxyEntry()->GetEntryFlags() : EDevMenuEntryFlags::NoFlags) | EDevMenuEntryFlags::ProxyEntry; }
	virtual UStruct* GetEntryType() const override { return StaticStruct(); }

	virtual TAttribute<FText> GetLabel() const override { return (IsValid()) ? GetProxyEntry()->GetLabel() : FText::GetEmpty(); };
	virtual TAttribute<FText> GetToolTip() const override { return (IsValid()) ? GetProxyEntry()->GetToolTip() : FText::GetEmpty(); };
	virtual TAttribute<FDevInputShortcut> GetInputShortcut() const override { return (IsValid()) ? GetProxyEntry()->GetInputShortcut() : FDevInputShortcut::GetEmpty(); };
	virtual ECheckBoxState GetCheckState(const UObject* WorldContextObject) const override { return (IsValid()) ? GetProxyEntry()->GetCheckState(WorldContextObject) : ECheckBoxState::Unchecked; };
	virtual bool IsVisible(const UObject* WorldContextObject) const override { return (IsValid()) ? GetProxyEntry()->IsVisible(WorldContextObject) : false; };

	virtual FDevMenuInstancedEntries* GetSubEntries() override { return (IsValid()) ? GetProxyEntry()->GetSubEntries() : nullptr; };
	virtual void QuerySubEntries(const IDevMenuEntriesQuery& InQuery) override { if (IsValid()) { GetProxyEntry()->QuerySubEntries(InQuery); } };

	virtual void ExecuteEntry(const UObject* WorldContextObject) override { if (IsValid()) { GetProxyEntry()->ExecuteEntry(WorldContextObject); } };

	virtual void SetEntryId(const FDevMenuEntryId InId) override { EntryId = InId; }
	virtual void SetEntryName(const FName InName) override { EntryName = InName; }

	virtual void ResolveEntryPath() override { EntryId.Resolve(); if (IsValid()) { GetProxyEntry()->ResolveEntryPath(); } }

	virtual void PopulateMenuBuilder(IDevMenuBuilderContext& InContext) override { if (IsValid()) { GetProxyEntry()->PopulateMenuBuilder(InContext); } };
	//~ End IDevMenuEntry interface.

private:
	using Super = FDevMenuEntry;
};


USTRUCT(NotBlueprintable, BlueprintType, Category = "DevHub|Menu", Meta = (DisplayName = "Separator"))
struct DEVMENUS_API FDevMenuSeparator : public FDevMenuEntry
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Dev Menu", DisplayName = "Entry ID")
	FDevMenuEntryId EntryId;

	//~ Begin IDevMenuEntry interface.
	virtual FDevMenuEntryId GetEntryId() const override { return EntryId; }
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };

	virtual void SetEntryId(const FDevMenuEntryId InId) override { EntryId = InId; }

	virtual void ResolveEntryPath() override { EntryId.Resolve(); }

	virtual void PopulateMenuBuilder(IDevMenuBuilderContext& InContext) override { InContext.GetMenuBuilder().AddSeparator(GetEntryPath()); };
	//~ End IDevMenuEntry interface.

private:
	using Super = FDevMenuEntry;
};


USTRUCT(NotBlueprintable, BlueprintType, Category = "DevHub|Menu", Meta = (DisplayName = "Section"))
struct DEVMENUS_API FDevMenuSection : public FDevMenuStaticOuterBase
{
	GENERATED_BODY()

	//~ Begin FDevMenuOuterBase interface.
	virtual EDevMenuEntryLayout GetOuterLayout() const override { return EDevMenuEntryLayout::Section; };
	//~ End FDevMenuOuterBase interface.

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };
	//~ End IDevMenuEntry interface.

private:
	using Super = FDevMenuStaticOuterBase;
};


USTRUCT(NotBlueprintable, BlueprintType, Category = "DevHub|Menu", Meta = (DisplayName = "Sub-menu"))
struct DEVMENUS_API FDevMenuSubMenu : public FDevMenuStaticOuterBase
{
	GENERATED_BODY()

	//~ Begin FDevMenuOuterBase interface.
	virtual EDevMenuEntryLayout GetOuterLayout() const override { return EDevMenuEntryLayout::SubMenu; };
	//~ End FDevMenuOuterBase interface.

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };
	//~ End IDevMenuEntry interface.

private:
	using Super = FDevMenuStaticOuterBase;
};


USTRUCT(NotBlueprintable, BlueprintType, Category = "DevHub|Menu", Meta = (DisplayName = "Action Button"))
struct DEVMENUS_API FDevMenuActionButton : public FDevMenuExecutionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Dev Menu", Meta = (NoClear, BaseStruct = "/Script/DevActions.DevAction", ExcludeBaseStruct, DisplayAfter = "InputShortcut"))
	FInstancedStruct Action;

	//~ Begin IDevMenuEntry interface.
	virtual UStruct* GetEntryType() const override { return StaticStruct(); };

	virtual ECheckBoxState GetCheckState(const UObject* WorldContextObject) const override;
	virtual bool IsVisible(const UObject* WorldContextObject) const override;
	virtual void ExecuteEntry(const UObject* WorldContextObject) override;
	//~ End IDevMenuEntry interface.

	//~ Begin FDevMenuExecutionBase interface.
	virtual EUserInterfaceActionType GetUserInterfaceType(const UObject* WorldContextObject) const override;
	//~ End FDevMenuExecutionBase interface.

private:
	using Super = FDevMenuExecutionBase;
};


UENUM(BlueprintType, Category = "DevHub|Menu")
enum class EDevMenuType : uint8
{
	/** Horizontal menu bar (main menu). */
	MenuBar UMETA(DisplayName = "Main Menu"),

	/** Vertical menu (pull-down menu, or context menu) */
	Menu UMETA(DisplayName = "Context Menu"),
};

UCLASS(BlueprintType, Blueprintable, Category = "DevHub|Menu", EditInlineNew, CollapseCategories, Meta = (LoadBehavior = "LazyOnDemand"))
class DEVMENUS_API UDevMenu : public UToolMenuBase
#if CPP
	, public IDevMenuEntry
#endif
{
	GENERATED_BODY()

public:
	static const FPrimaryAssetType PrimaryAssetType;

	/** Menu ID. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Menu", DisplayName = "Menu ID")
	mutable FDevMenuEntryId MenuId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Menu")
	FName MenuPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dev Menu")
	EDevMenuType MenuType = EDevMenuType::Menu;

#pragma warning(push)
#pragma warning(disable: 5101)
	UPROPERTY(
		EditAnywhere,
#if UE_COMPATIBILITY_INSTANCED_STRUCT_BLUEPRINT_READ_WRITE
		BlueprintReadWrite,
#endif
		Category = "Dev Menu",
		Meta = (BaseStruct = "/Script/DevMenus.DevMenuEntry", ExcludeBaseStruct))
	TArray<FInstancedStruct> Entries;
#pragma warning(pop)

	static UDevMenu* CreateMenu(const FName InMenuPath, UObject* InOuter, const FName InBaseName = NAME_None, void (*OnConstruct)(UDevMenu*& Menu) = nullptr);

	FORCEINLINE FGetActionCheckState CreateGetEntryCheckStateDelegate(const UObject* WorldContextObject, const IDevMenuEntry& InEntry) const { return FGetActionCheckState::CreateUObject(this, &ThisClass::OnGetEntryCheckState, TWeakObjectPtr<const UObject>(WorldContextObject), &InEntry); }
	FORCEINLINE FIsActionButtonVisible CreateGetEntryVisibilityDelegate(const UObject* WorldContextObject, const IDevMenuEntry& InEntry) const { return FIsActionButtonVisible::CreateUObject(this, &ThisClass::OnGetEntryVisibility, TWeakObjectPtr<const UObject>(WorldContextObject), &InEntry); }
	FORCEINLINE FExecuteAction CreateExecuteEntryDelegate(const UObject* WorldContextObject, IDevMenuEntry& InEntry) const { return FExecuteAction::CreateUObject(this, &ThisClass::OnExecuteEntry, TWeakObjectPtr<const UObject>(WorldContextObject), &InEntry); }

	template<typename FunctorType, typename... VarTypes>
	FORCEINLINE FExecuteAction CreateExecuteEntryWeakLambda(const UObject* WorldContextObject, FunctorType&& InFunctor, VarTypes... Vars)
	{
		return FExecuteAction::CreateWeakLambda(this, [WorldContextObjectPtr = TWeakObjectPtr<const UObject>(WorldContextObject), Functor = Forward<FunctorType>(InFunctor), Vars...]{ Functor(WorldContextObjectPtr.Get(), Vars...); });
	}

	//~ Begin IDevMenuEntry interface.
	virtual FDevMenuEntryId GetEntryId() const override { return MenuId; }
	virtual FName GetEntryName() const override { return MenuPath; }
	virtual EDevMenuEntryFlags GetEntryFlags() const override { return IDevMenuEntry::GetEntryFlags() | EDevMenuEntryFlags::SubMenu | EDevMenuEntryFlags::StaticEntries; };
	virtual UStruct* GetEntryType() const override { return GetClass(); };

	virtual TAttribute<FText> GetLabel() const override;

	virtual FDevMenuInstancedEntries* GetSubEntries() override { return &Entries; };
	virtual void QuerySubEntries(const IDevMenuEntriesQuery& InQuery) override;

	virtual void SetEntryId(const FDevMenuEntryId InId) override { MenuId = InId; }
	virtual void SetEntryName(const FName InName) override { MenuPath = InName; }

	virtual void ResolveEntryPath() override;

	virtual void PopulateMenuBuilder(IDevMenuBuilderContext& InContext) override;
	//~ End IDevMenuEntry interface.

	//~ Begin UObject interface.
	virtual void BeginDestroy() override;
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	//~ End UObject interface.

private:
	ECheckBoxState OnGetEntryCheckState(const TWeakObjectPtr<const UObject> WorldContextObjectPtr, const IDevMenuEntry* InEntry) const { return InEntry->GetCheckState(WorldContextObjectPtr.Get()); }
	bool OnGetEntryVisibility(const TWeakObjectPtr<const UObject> WorldContextObjectPtr, const IDevMenuEntry* InEntry) const { return InEntry->IsVisible(WorldContextObjectPtr.Get()); }
	void OnExecuteEntry(const TWeakObjectPtr<const UObject> WorldContextObjectPtr, IDevMenuEntry* InEntry) const { InEntry->ExecuteEntry(WorldContextObjectPtr.Get()); }
};

DECLARE_MULTICAST_DELEGATE(FDevMenuEvent);
