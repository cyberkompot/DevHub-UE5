// Copyright (c) Alexandr Pereverzev.

#pragma once

#include "Misc/EngineVersionComparison.h"

/**
 * Unreal Engine version compatibility utilities.
 */

/** Version comparison macro that is defined to true if the UE version is at least MajorVer.MinorVer.PatchVer and false otherwise. */
#ifndef UE_VERSION_AT_LEAST
	#define UE_VERSION_AT_LEAST(MajorVersion, MinorVersion, PatchVersion) UE_VERSION_NEWER_THAN(MajorVersion, MinorVersion, PatchVersion - 1)
#endif


/**
 * Unreal Engine version compatibility conditional definitions.
 */

/** UE_ALLOW_EXEC_COMMANDS */
#ifndef UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS
	#ifdef UE_ALLOW_EXEC_COMMANDS
		#define UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS UE_ALLOW_EXEC_COMMANDS
	#else
		#define UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS 1
	#endif
#endif

/** ViewportClient.h */
#ifndef UE_COMPATIBILITY_ALLOWSHRINKING_BOOL_DEPRECATED
	#if UE_VERSION_AT_LEAST(5, 6, 0)
		#define ALLOW_SHRINKING EAllowShrinking::Yes
		#define DONT_ALLOW_SHRINKING EAllowShrinking::No
	#else
		#define ALLOW_SHRINKING true
		#define DONT_ALLOW_SHRINKING false
	#endif
#endif

/** StructUtils/InstancedStruct.h, StructUtils/StructView.h */
#ifndef UE_COMPATIBILITY_INCLUDE_INSTANCED_STRUCT_PATH
	#if UE_VERSION_AT_LEAST(5, 5, 0) && !(defined(UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_5) && UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_5)
		#define UE_COMPATIBILITY_INCLUDE_INSTANCED_STRUCT_PATH "StructUtils/InstancedStruct.h"
		#define UE_COMPATIBILITY_INCLUDE_PROPERTY_BAG_PATH "StructUtils/PropertyBag.h"
		#define UE_COMPATIBILITY_INCLUDE_STRUCT_VIEW_PATH "StructUtils/StructView.h"
	#else
		#define UE_COMPATIBILITY_INCLUDE_INSTANCED_STRUCT_PATH "InstancedStruct.h"
		#define UE_COMPATIBILITY_INCLUDE_PROPERTY_BAG_PATH "PropertyBag.h"
		#define UE_COMPATIBILITY_INCLUDE_STRUCT_VIEW_PATH "StructView.h"
	#endif
#endif

/** ViewportClient.h */
#ifndef UE_COMPATIBILITY_INCLUDE_VIEWPORT_CLIENT_PATH
	#if UE_VERSION_AT_LEAST(5, 2, 0)
		#define UE_COMPATIBILITY_INCLUDE_VIEWPORT_CLIENT_PATH "ViewportClient.h"
	#else
		#define UE_COMPATIBILITY_INCLUDE_VIEWPORT_CLIENT_PATH "UnrealClient.h"
	#endif
#endif

/** FInputKeyEventArgs */
#define UE_COMPATIBILITY_INPUT_KEY_EVENT_ARGS UE_VERSION_AT_LEAST(5, 6, 0)

/** FInstancedStruct */
#define UE_COMPATIBILITY_INSTANCED_STRUCT_BLUEPRINT_READ_WRITE UE_VERSION_AT_LEAST(5, 1, 0)

/** ObjectPtrDecay() */
#define UE_COMPATIBILITY_OBJECT_PTR_DECAY UE_VERSION_AT_LEAST(5, 3, 0)

/** APlayerController::GetCurrentInputModeDebugString(), FInputModeDataBase::GetDebugDisplayName() */
#define UE_COMPATIBILITY_SUPPORTED_INPUT_MODE_DATA_BASE_GET_DEBUG_DISPLAY_NAME (UE_ENABLE_DEBUG_DRAWING && UE_VERSION_AT_LEAST(5, 3, 0))

/** APlayerController::ShouldFlushKeysWhenViewportFocusChanges() */
#define UE_COMPATIBILITY_SUPPORTED_PLAYER_CONTROLLER_SHOULD_FLUSH_KEYS_WHEN_VIEWPORT_FOCUS_CHANGES UE_VERSION_AT_LEAST(5, 1, 0)

/** FReferenceCollector::AddReferencedObject() */
#define UE_COMPATIBILITY_REFERENCE_COLLECTOR_ADD_REFERENCED_OBJECT_WEAK_OBJECT_PTR UE_VERSION_AT_LEAST(5, 3, 0)

/** IConsoleVariable::IsEnabled() */
#define UE_COMPATIBILITY_SUPPORTED_CONSOLE_VARIABLE_IS_ENABLED UE_VERSION_AT_LEAST(5, 5, 0)

/** FKeyEvent::GetInputDeviceId() */
#define UE_COMPATIBILITY_SUPPORTED_KEY_EVENT_GET_INPUT_DEVICE_ID UE_VERSION_AT_LEAST(5, 1, 0)

/** FHardwareDeviceIdentifier, EHardwareDevicePrimaryType, IPlatformInputDeviceMapper::GetHardwareDeviceDetails() - removed in 5.7 */
#define UE_COMPATIBILITY_SUPPORTED_HARDWARE_DEVICE_DETAILS (UE_VERSION_AT_LEAST(5, 2, 0) && !UE_VERSION_AT_LEAST(5, 7, 0))

/** UGameViewportClient::AddGameLayerWidget() */
#define UE_COMPATIBILITY_GAME_VIEWPORT_CLIENT_ADD_GAME_LAYER_WIDGET UE_VERSION_AT_LEAST(5, 3, 0)

/** UGameViewportClient::GetMouseLockMode() */
#define UE_COMPATIBILITY_GAME_VIEWPORT_CLIENT_GET_MOUSE_LOCK_MODE UE_VERSION_AT_LEAST(5, 3, 0)
