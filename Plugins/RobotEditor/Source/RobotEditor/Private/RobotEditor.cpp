// Copyright Epic Games, Inc. All Rights Reserved.

#include "RobotEditor.h"
#include "RobotEditorStyle.h"
#include "RobotEditorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName RobotEditorTabName("RobotEditor");

#define LOCTEXT_NAMESPACE "FRobotEditorModule"

void FRobotEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FRobotEditorStyle::Initialize();
	FRobotEditorStyle::ReloadTextures();

	FRobotEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FRobotEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FRobotEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FRobotEditorModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(RobotEditorTabName, FOnSpawnTab::CreateRaw(this, &FRobotEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FRobotEditorTabTitle", "RobotEditor"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FRobotEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FRobotEditorStyle::Shutdown();

	FRobotEditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(RobotEditorTabName);
}

TSharedRef<SDockTab> FRobotEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FRobotEditorModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("RobotEditor.cpp"))
		);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
}

void FRobotEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(RobotEditorTabName);
}

void FRobotEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FRobotEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FRobotEditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRobotEditorModule, RobotEditor)