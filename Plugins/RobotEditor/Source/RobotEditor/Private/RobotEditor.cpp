#include "RobotEditor.h"
#include "RobotEditorStyle.h"
#include "RobotEditorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Viewport/RobotEditorViewport.h"
#include "ToolMenus.h"

static const FName RobotEditorTabName("RobotEditor");

#define LOCTEXT_NAMESPACE "FRobotEditorModule"

void FRobotEditorModule::StartupModule() {
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

void FRobotEditorModule::ShutdownModule() {
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FRobotEditorStyle::Shutdown();
	FRobotEditorCommands::Unregister();
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(RobotEditorTabName);
}

TSharedRef<SDockTab> FRobotEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs) {
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
            SNew(SRobotEditorViewport)
		];
}

void FRobotEditorModule::PluginButtonClicked() {
	FGlobalTabmanager::Get()->TryInvokeTab(RobotEditorTabName);
}

void FRobotEditorModule::RegisterMenus() {
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
