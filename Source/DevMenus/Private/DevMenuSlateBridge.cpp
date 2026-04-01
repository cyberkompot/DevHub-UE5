// Copyright (c) Alexandr Pereverzev.

#include "DevMenuSlateBridge.h"

#include "DevCore.h"
#include UE_COMPATIBILITY_INCLUDE_VIEWPORT_CLIENT_PATH

#include "Framework/Application/SlateApplication.h"
#include "Rendering/RenderingCommon.h"
#include "Slate/SceneViewport.h"
#include "Widgets/SWindow.h"

UWorld* FDevMenuSlateBridge::GetWorld(const SWidget* ViewportContextWidget)
{
	if (!ViewportContextWidget) { return nullptr; }
	if (!FSlateApplication::IsInitialized()) { return nullptr; }

	if (const TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(ViewportContextWidget->AsShared()))
	{
		if (const TSharedPtr<ISlateViewport> SlateViewport = Window->GetViewport())
		{
			if (const FSceneViewport* SceneViewport = static_cast<FSceneViewport*>(SlateViewport.Get()))
			{
				if (const FViewportClient* ViewportClient = SceneViewport->GetClient())
				{
					return ViewportClient->GetWorld();
				}
			}
		}
	}
	return nullptr;
}
