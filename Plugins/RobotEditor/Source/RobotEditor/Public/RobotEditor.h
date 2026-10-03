// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FRobotEditorModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

    void OnEngineInitialized();

	void PluginButtonClicked();

private:

	void RegisterMenus();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
