// Copyright (c) Alexandr Pereverzev.

#include "DevConsoleLibrary.h"

#include "ConsoleSettings.h"
#include "DevConsole.h"
#include "DevCore.h"
#include "Engine/Console.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"

struct FDevConsoleProxyCommand final : IConsoleCommand
{
	FDevConsoleProxyCommand(FString&& InCommand, FString&& InDesc)
		: Command(Forward<FString>(InCommand)), Description(Forward<FString>(InDesc)) {}

	//~ Begin IConsoleObject interface.
	virtual const TCHAR* GetHelp() const override { return *Description; }
	virtual void SetHelp(const TCHAR*) override {}
	virtual EConsoleVariableFlags GetFlags() const override { return ECVF_Default; }
	virtual void SetFlags(EConsoleVariableFlags) override {}
	virtual IConsoleVariable* AsVariable() override { return nullptr; }
	virtual IConsoleCommand* AsCommand() override { return this; }
	//~ End IConsoleObject interface.

	//~ Begin IConsoleCommand interface.
	virtual bool Execute(const TArray<FString>& Args, UWorld* InWorld, FOutputDevice& Ar) override
	{
		return FDevConsole::ConsoleCommand(InWorld, FString::Join(Args, TEXT(" ")));
	}
	//~ End IConsoleCommand interface.

private:
	virtual void Release() override {}

	FString Command;
	FString Description;
};

FDevConsoleLibrary& FDevConsoleLibrary::Get()
{
	static FDevConsoleLibrary Singleton;
	return Singleton;
}

IConsoleCommand* FDevConsoleLibrary::FindConsoleCommand(const FString& Name)
{
#if UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS
	if (Name.IsEmpty()) { return nullptr; }

	const TCHAR* CommandName;
	FNameBuilder CommandNameBuffer; // Must remain valid for at least the lifetime of CommandName.

	if (int32 SpaceIndex; Name.FindChar(TEXT(' '), SpaceIndex))
	{
		CommandNameBuffer.Append(*Name, SpaceIndex);
		CommandName = CommandNameBuffer.ToString();
	}
	else
	{
		CommandName = *Name;
	}

	const FName CommandNameKey(CommandName);

	if (IConsoleObject* CObject = IConsoleManager::Get().FindConsoleObject(CommandName, false))
	{
		if (IConsoleCommand* CCommand = CObject->AsCommand())
		{
			ProxyCommandCache.Remove(CommandNameKey);
			return CCommand;
		}
		else
		{
			return nullptr; // Name is registered as a CVar, not a command.
		}
	}

	// IConsoleManager has no entry — check proxy cache (AutoComplete-derived commands).
	if (const TUniquePtr<IConsoleObject>* CachedProxy = ProxyCommandCache.Find(CommandNameKey))
	{
		return (*CachedProxy)->AsCommand();
	}

	// Fallback search in UConsole::AutoCompleteList for exec-style commands not in IConsoleManager.
	if (GEngine && GEngine->GameViewport)
	{
		if (UConsole* Console = GEngine->GameViewport->ViewportConsole)
		{
			if (!Console->bIsRuntimeAutoCompleteUpToDate || Console->AutoCompleteList.IsEmpty())
			{
				Console->BuildRuntimeAutoCompleteList(true);
			}

			for (const FAutoCompleteCommand& AutoComplete : Console->AutoCompleteList)
			{
				if (AutoComplete.Command.IsEmpty()) { continue; }

				// Extract the command name part from this AutoComplete entry.
				const TCHAR* AutoCommandName;
				FNameBuilder AutoNameBuffer; // Must remain valid for at least the lifetime of AutoCommandName.

				if (int32 AutoSpaceIndex; AutoComplete.Command.FindChar(TEXT(' '), AutoSpaceIndex))
				{
					AutoNameBuffer.Append(*AutoComplete.Command, AutoSpaceIndex);
					AutoCommandName = AutoNameBuffer.ToString();
				}
				else
				{
					AutoCommandName = *AutoComplete.Command;
				}

				// Case-insensitive match against extracted Command name.
				if (FCString::Stricmp(CommandName, AutoCommandName) == 0)
				{
					const TUniquePtr<IConsoleObject>& NewProxy = ProxyCommandCache.Emplace(CommandNameKey, MakeUnique<FDevConsoleProxyCommand>(CommandName, *AutoComplete.Desc));
					return NewProxy->AsCommand();
				}
			}
		}
	}
#endif // UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS
	return nullptr;
}

IConsoleVariable* FDevConsoleLibrary::FindConsoleVariable(const FString& Name)
{
#if !NO_CVARS
	if (Name.IsEmpty()) { return nullptr; }

	const TCHAR* VariableName;
	FNameBuilder VariableNameBuffer; // Must remain valid for at least the lifetime of VariableName.

	if (int32 SpaceIndex; Name.FindChar(TEXT(' '), SpaceIndex))
	{
		VariableNameBuffer.Append(*Name, SpaceIndex);
		VariableName = VariableNameBuffer.ToString();
	}
	else
	{
		VariableName = *Name;
	}

	return IConsoleManager::Get().FindConsoleVariable(VariableName, false);
#else
	return nullptr;
#endif // !NO_CVARS
}

void FDevConsoleLibrary::Reset()
{
	ProxyCommandCache.Reset();
}
