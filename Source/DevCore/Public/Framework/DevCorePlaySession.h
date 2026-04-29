// Copyright (c) Alexandr Pereverzev.

#pragma once

class UObject;

struct FDevCorePlaySession final
{
    static DEVCORE_API bool IsGamePaused(const UObject* WorldContextObject);
    static DEVCORE_API void PausedGame(const UObject* WorldContextObject);
    static DEVCORE_API void ResumeGame(const UObject* WorldContextObject);
    static DEVCORE_API void StopGame(const UObject* WorldContextObject);
};
