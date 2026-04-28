// Copyright (c) Alexandr Pereverzev.

#pragma once

class UObject;

struct FDevCorePlaySession final
{
    static DEVCORE_API bool IsGamePaused(const UObject* WorldContextObject);
};
