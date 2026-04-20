// Copyright (c) Alexandr Pereverzev.

#pragma once

#ifndef UE_LOG_FUNCTION
	#define UE_LOG_FUNCTION(CategoryName, Verbosity, Format, ...) UE_LOG(CategoryName, Verbosity, TEXT("%s: ") Format, ANSI_TO_TCHAR(__FUNCTION__), ##__VA_ARGS__)
#endif

#ifndef UE_CLOG_FUNCTION
	#define UE_CLOG_FUNCTION(Condition, CategoryName, Verbosity, Format, ...) UE_CLOG(Condition, CategoryName, Verbosity, TEXT("%s: ") Format, ANSI_TO_TCHAR(__FUNCTION__), ##__VA_ARGS__)
#endif

namespace DevCore::Logging
{
	template<typename EnumType>
	FString EnumToLog(EnumType InEnumValue) { return StaticEnum<EnumType>()->GetNameStringByValue(static_cast<int64>(InEnumValue)); }
} // namespace DevCore::Logging
