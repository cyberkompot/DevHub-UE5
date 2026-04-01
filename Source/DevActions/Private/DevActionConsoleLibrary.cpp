// Copyright (c) Alexandr Pereverzev.

#include "DevActionConsoleLibrary.h"

#include "ConsoleSettings.h"
#include "DevCore.h"
#include "Engine/Console.h"

struct FDevActionDummyConsoleCommand final : IConsoleCommand
{
	FDevActionDummyConsoleCommand(FString&& InCommand, FString&& InDesc)
		: Command(MoveTempIfPossible(InCommand)), Description(MoveTempIfPossible(InDesc)) {}

	//~ Begin IConsoleObject interface.
	virtual const TCHAR* GetHelp() const override { return *Description; }
	virtual void SetHelp(const TCHAR*) override {}
	virtual EConsoleVariableFlags GetFlags() const override { return ECVF_Default; }
	virtual void SetFlags(EConsoleVariableFlags) override {}
	virtual IConsoleVariable* AsVariable() override { return nullptr; }
	virtual IConsoleCommand* AsCommand() override { return this; }
	virtual void Release() override {}
	//~ End IConsoleObject interface.

	//~ Begin IConsoleCommand interface.
	virtual bool Execute(const TArray<FString>& Args, UWorld* InWorld, FOutputDevice& Ar) override { return false; }
	//~ End IConsoleCommand interface.

private:
	FString Command;
	FString Description;
};

FDevActionConsoleLibrary& FDevActionConsoleLibrary::Get()
{
	static FDevActionConsoleLibrary Singleton{};
	return Singleton;
}

IConsoleCommand* FDevActionConsoleLibrary::FindConsoleCommand(const FString& Name)
{
#if UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS
	if (Name.IsEmpty()) { return nullptr; }

	const TCHAR* CommandName;
	TStringBuilder<NAME_SIZE> CommandNameBuffer;

	if (int32 SpaceIndex; Name.FindChar(TEXT(' '), SpaceIndex))
	{
		CommandNameBuffer.Append(*Name, SpaceIndex);
		CommandName = CommandNameBuffer.ToString();
	}
	else
	{
		CommandName = *Name;
	}

	// Always check the UConsole::AutoCompleteList cache first.
	// IConsoleManager may become unhappy if the same object is searched repeatedly.
	const FName CommandNameKey(CommandName);
	if (IConsoleObject** CachedCObjectPtr = ConsoleObjectsCache.Find(CommandNameKey))
	{
		if (IConsoleCommand* CCommand = (*CachedCObjectPtr)->AsCommand())
		{
			return CCommand;
		}
	}

	// Direct lookup in IConsoleManager.
	if (IConsoleObject* CObject = IConsoleManager::Get().FindConsoleObject(CommandName))
	{
		if (IConsoleCommand* CCommand = CObject->AsCommand())
		{
			ConsoleObjectsCache.Emplace(CommandNameKey, CCommand);
			return CCommand;
		}
	}

	// Fallback search in UConsole::AutoCompleteList.
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
				TStringBuilder<NAME_SIZE> AutoNameBuffer;
				const TCHAR* AutoCommandName;

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
					const TUniquePtr<IConsoleObject>& NewCObject = DummyConsoleObjects.Emplace_GetRef(MakeUnique<FDevActionDummyConsoleCommand>(CommandName, *AutoComplete.Desc));
					ConsoleObjectsCache.Emplace(CommandNameKey, NewCObject.Get());
					return NewCObject->AsCommand();
				}
			}
		}
	}
#endif // UE_COMPATIBILITY_DEFINITION_ALLOW_EXEC_COMMANDS
	return nullptr;
}

IConsoleVariable* FDevActionConsoleLibrary::FindConsoleVariable(const FString& Name)
{
#if !NO_CVARS
	if (Name.IsEmpty()) { return nullptr; }

	const TCHAR* VariableName = *Name;

	// Always check the UConsole::AutoCompleteList cache first.
	// IConsoleManager may become unhappy if the same object is searched repeatedly.
	const FName VariableNameKey(VariableName);
	if (IConsoleObject** CachedCObjectPtr = ConsoleObjectsCache.Find(VariableNameKey))
	{
		if (IConsoleVariable* CVar = (*CachedCObjectPtr)->AsVariable())
		{
			return CVar;
		}
	}

	// Direct lookup in IConsoleManager.
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(VariableName))
	{
		ConsoleObjectsCache.Emplace(VariableNameKey, CVar);
		return CVar;
	}
#endif // !NO_CVARS
	return nullptr;
}
