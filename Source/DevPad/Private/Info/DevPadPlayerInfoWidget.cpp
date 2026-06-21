// Copyright (c) Alexandr Pereverzev.

#include "Info/DevPadPlayerInfoWidget.h"

#include "Framework/DevCorePlaySession.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/WorldSettings.h"

FText UDevPadPlayerInfoWidget::GetInfoText(const FString& Format) const
{
	return FText::FromString(FString::Format(*Format, FormatArguments));
}

void UDevPadPlayerInfoWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	const UWorld* World = GetWorld();
	const APawn* Pawn = GetOwningPlayerPawn();
	const APlayerCameraManager* CameraManager = GetOwningPlayerCameraManager();
	const AWorldSettings* WorldSettings = (Pawn) ? Pawn->GetWorldSettings() : nullptr;

	const FVector Location = (Pawn) ? Pawn->GetActorLocation() : (CameraManager) ? CameraManager->GetCameraLocation() : FVector::ZeroVector;
	const FRotator Rotation = (CameraManager) ? CameraManager->GetCameraRotation() : FRotator::ZeroRotator;
	const float EffectiveTimeDilation = (WorldSettings) ? WorldSettings->GetEffectiveTimeDilation() : 1.f;
	const bool bIsPaused = FDevCorePlaySession::IsGamePaused(World);

	FString ModeArg = (Pawn) ? TEXT("Pawn") : (CameraManager) ? TEXT("Camera") : TEXT("No Pawn/Camera");
	FString NameArg = (Pawn) ? Pawn->GetName() : (CameraManager) ? CameraManager->GetName() : TEXT("None");
	FString ClassArg = (Pawn) ? Pawn->GetClass()->GetName() : (CameraManager) ? CameraManager->GetClass()->GetName() : TEXT("None");
	FString LocationArg = FString::Printf(TEXT("X=%3.3f Y=%3.3f Z=%3.3f"), Location.X, Location.Y, Location.Z);
	FString CameraArg = FString::Printf(TEXT("P=%3.3f Y=%3.3f R=%3.3f"), Rotation.Pitch, Rotation.Yaw, Rotation.Roll);
	FString SlomoArg = (bIsPaused) ? TEXT("Paused") : FString::Printf(TEXT("x%1.2f"), EffectiveTimeDilation);

	FString LocationXArg = FString::Printf(TEXT("%3.3f"), Location.X);
	FString LocationYArg = FString::Printf(TEXT("%3.3f"), Location.Y);
	FString LocationZArg = FString::Printf(TEXT("%3.3f"), Location.Z);

	FString CameraPArg = FString::Printf(TEXT("%3.3f"), Rotation.Pitch);
	FString CameraYArg = FString::Printf(TEXT("%3.3f"), Rotation.Yaw);
	FString CameraRArg = FString::Printf(TEXT("%3.3f"), Rotation.Roll);

	LocationArg.ReplaceInline(TEXT("-0.000"), TEXT("0.000"));
	CameraArg.ReplaceInline(TEXT("-0.000"), TEXT("0.000"));

	LocationXArg.ReplaceInline(TEXT("-0.000"), TEXT("0.000"));
	LocationYArg.ReplaceInline(TEXT("-0.000"), TEXT("0.000"));
	LocationZArg.ReplaceInline(TEXT("-0.000"), TEXT("0.000"));

	CameraPArg.ReplaceInline(TEXT("-0.000"), TEXT("0.000"));
	CameraYArg.ReplaceInline(TEXT("-0.000"), TEXT("0.000"));
	CameraRArg.ReplaceInline(TEXT("-0.000"), TEXT("0.000"));

	FormatArguments.Emplace(TEXT("Mode"), MoveTemp(ModeArg));
	FormatArguments.Emplace(TEXT("Name"), MoveTemp(NameArg));
	FormatArguments.Emplace(TEXT("Class"), MoveTemp(ClassArg));
	FormatArguments.Emplace(TEXT("Location"), MoveTemp(LocationArg));
	FormatArguments.Emplace(TEXT("Camera"), MoveTemp(CameraArg));
	FormatArguments.Emplace(TEXT("Slomo"), MoveTemp(SlomoArg));

	FormatArguments.Emplace(TEXT("LocationX"), MoveTemp(LocationXArg));
	FormatArguments.Emplace(TEXT("LocationY"), MoveTemp(LocationYArg));
	FormatArguments.Emplace(TEXT("LocationZ"), MoveTemp(LocationZArg));

	FormatArguments.Emplace(TEXT("CameraP"), MoveTemp(CameraPArg));
	FormatArguments.Emplace(TEXT("CameraY"), MoveTemp(CameraYArg));
	FormatArguments.Emplace(TEXT("CameraR"), MoveTemp(CameraRArg));

	Super::NativeTick(MyGeometry, InDeltaTime);
}
