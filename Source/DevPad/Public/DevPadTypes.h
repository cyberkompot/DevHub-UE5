// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "Blueprint/UserWidget.h"
#include "DevPadTypes.generated.h"

struct FDevAction;
class UDevPadPage;
class UDevPad;

UCLASS(Abstract, Category = "DevHub|Pad")
class DEVPAD_API UDevPadWidget : public UUserWidget
{
	GENERATED_BODY()
};

UCLASS(Abstract, Category = "DevHub|Pad")
class DEVPAD_API UDevPadContentData : public UObject
{
	GENERATED_BODY()
};

UCLASS(Abstract, Category = "DevHub|Pad")
class DEVPAD_API UDevPadContentWidget : public UDevPadWidget
{
	GENERATED_BODY()

public:
	template<typename TContentDataType,
			 typename = typename TEnableIf<TIsDerivedFrom<typename TDecay<TContentDataType>::Type, UDevPadContentData>::Value>::Type>
	TContentDataType* GetData() const
	{
		return Cast<TContentDataType>(WidgetData);
	}

	template<typename TContentDataType,
			 typename = typename TEnableIf<TIsDerivedFrom<typename TDecay<TContentDataType>::Type, UDevPadContentData>::Value>::Type>
	TContentDataType* GetDataOrCreate()
	{
		if (!WidgetData) { WidgetData = GetExpectedDataClass() ? NewObject<UDevPadContentData>(this, GetExpectedDataClass(), "DevPadData", RF_Transient) : nullptr; }
		return Cast<TContentDataType>(WidgetData);
	}

	template<typename TContentDataType,
			 typename = typename TEnableIf<TIsDerivedFrom<typename TDecay<TContentDataType>::Type, UDevPadContentData>::Value>::Type>
	const TContentDataType* GetDataOrExpectedDefault() const
	{
		return (WidgetData)
			? Cast<TContentDataType>(WidgetData)
			: Cast<TContentDataType>(GetExpectedDataClass() ? GetExpectedDataClass()->GetDefaultObject() : nullptr);
	}

	template<typename TContentDataType,
			 typename = typename TEnableIf<TIsDerivedFrom<typename TDecay<TContentDataType>::Type, UDevPadContentData>::Value>::Type>
	const TContentDataType& GetDataOrRequestedDefault() const
	{
		const TContentDataType* Data = GetDataOrExpectedDefault<TContentDataType>();
		return (Data) ? *Data : *TContentDataType::StaticClass()->template GetDefaultObject<TContentDataType>();
	}

	UFUNCTION(BlueprintPure, Category = "DevHub|Pad")
	FORCEINLINE UDevPadContentData* GetData() const { return WidgetData; }

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	void SetData(UDevPadContentData* InData)
	{
		WidgetData = InData;
		UpdateWidget();
	}

public:
	virtual TSubclassOf<UDevPadContentData> GetExpectedDataClass() const { return nullptr; }
	virtual void UpdateWidget() {}

private:
	UPROPERTY(Transient)
	TObjectPtr<UDevPadContentData> WidgetData = nullptr;
};

UCLASS(Abstract, Category = "DevHub|Pad")
class DEVPAD_API UDevPadPageData : public UDevPadContentData
{
	GENERATED_BODY()
};

UCLASS(Abstract, Category = "DevHub|Pad")
class DEVPAD_API UDevPadPageWidget : public UDevPadContentWidget
{
	GENERATED_BODY()
};

UCLASS(Abstract, Category = "DevHub|Pad")
class DEVPAD_API UDevPadStack : public UObject
{
	GENERATED_BODY()

public:
	// Returns common page and all pages in the stack in the following order: Common page (if exists), Main page, then Sub-pages.
	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	virtual const TArray<UDevPadPage*>& GetAllPages() const { return Empty; }

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	virtual UDevPadPage* GetCommonPage() const { return nullptr; }

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	virtual UDevPadPage* GetMainPage() const { return nullptr; }

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	virtual UDevPadPage* GetTopPage() const { return nullptr; }

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	virtual bool IsStackEmpty() const { return true; }

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	virtual bool IsCommonPage(const UDevPadPage* InPage) const { return false; }

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	virtual bool IsMainPage(const UDevPadPage* InPage) const { return false; }

	UFUNCTION(BlueprintCallable, Category = "DevHub|Pad")
	virtual bool IsSubPage(const UDevPadPage* InPage) const { return false; }

private:
	static const TArray<UDevPadPage*> Empty;
};

UENUM(BlueprintType, Category = "DevHub|Pad")
enum struct EDevPadAlignment : uint8
{
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
};

UENUM(BlueprintType, Category = "DevHub|Pad")
enum struct EDevPadInput : uint8
{
	Invalid = 0 UMETA(Hidden),

	DPadUp,
	DPadLeft,
	DPadRight,
	DPadDown,

	FaceUp,
	FaceLeft,
	FaceRight,
	FaceDown,

	LeftShoulder,
	RightShoulder,

	LeftTrigger,
	RightTrigger,

	LeftThumbstick,
	RightThumbstick,

	LeftPlusRightShoulders,
	LeftPlusRightTriggers,
	LeftPlusRightThumbsticks,
};

UENUM(BlueprintType, Category = "DevHub|Pad")
enum struct EDevPadInputExecution : uint8
{
	Break = 0,
	Continue = 1,
};

USTRUCT(BlueprintType, Category = "DevHub|Pad")
struct DEVPAD_API FDevPadExecutionContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevPad")
	TObjectPtr<UDevPad> DevPad = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevPad")
	TObjectPtr<UDevPadStack> PadStack = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevPad")
	EDevPadInput PadInput = EDevPadInput::Invalid;

	FORCEINLINE UDevPad* GetPad() const { return DevPad; }
	FORCEINLINE UDevPadStack* GetPadStack() const { return PadStack; }
	FORCEINLINE EDevPadInput GetPadInput() const { return PadInput; }
};

USTRUCT(BlueprintType, Category = "DevHub|Pad")
struct DEVPAD_API FDevPadWidgetContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevPad")
	TObjectPtr<UDevPad> DevPad = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevPad")
	TObjectPtr<UDevPadStack> PadStack = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevPad")
	TObjectPtr<UDevPadPageData> PageData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DevPad")
	TObjectPtr<UDevPadPageWidget> PageWidget = nullptr;

	FORCEINLINE UDevPad* GetPad() const { return DevPad; }
	FORCEINLINE UDevPadStack* GetPadStack() const { return PadStack; }
	FORCEINLINE UDevPadPageData* GetPageData() const { return PageData; }
	FORCEINLINE UDevPadPageWidget* GetPageWidget() const { return PageWidget; }

	template<typename TPageDataType,
			 typename = typename TEnableIf<TIsDerivedFrom<typename TDecay<TPageDataType>::Type, UDevPadPageData>::Value>::Type>
	FORCEINLINE TPageDataType* GetPageData() const
	{
		return Cast<TPageDataType>(PageData);
	}

	template<typename TPageWidgetType,
			 typename = typename TEnableIf<TIsDerivedFrom<typename TDecay<TPageWidgetType>::Type, UDevPadPageWidget>::Value>::Type>
	FORCEINLINE TPageWidgetType* GetPageWidget() const
	{
		return Cast<TPageWidgetType>(PageWidget);
	}
};

UCLASS(Abstract, BlueprintType, Blueprintable, Category = "DevHub|Pad", EditInlineNew, CollapseCategories, Meta = (Hidden, LoadBehavior = "LazyOnDemand"))
class DEVPAD_API UDevPadPage : public UObject
{
	GENERATED_BODY()

public:
	virtual FName GetPageName() const { return GetFName(); }
	virtual TSoftClassPtr<UDevPadPageWidget> GetPageWidgetClass(const UObject* WorldContextObject) const;

	virtual TAttribute<FText> GetPageTitle() const;

	virtual EDevPadInputExecution ExecutePageInput(const UObject* WorldContextObject, const FDevPadExecutionContext& PageExecutionContext) const { return EDevPadInputExecution::Continue; };
	virtual void PopulatePageWidget(const UObject* WorldContextObject, const FDevPadWidgetContext& PageWidgetContext) const {}
};

UCLASS(Abstract, BlueprintType, Blueprintable, Category = "DevHub|Pad", Meta = (Hidden))
class DEVPAD_API UDevPadPageBase : public UDevPadPage
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "DevPad")
	FText Title;

	//~ Begin IDevPadPage interface.
	virtual TAttribute<FText> GetPageTitle() const override { return (!Title.IsEmpty()) ? Title : Super::GetPageTitle(); };
	//~ End IDevPadPage interface.
};
