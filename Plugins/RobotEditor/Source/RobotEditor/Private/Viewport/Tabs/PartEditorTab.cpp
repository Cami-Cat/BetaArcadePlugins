#include "Viewport/Tabs/PartEditorTab.h"
#include "Viewport/PartEditorViewport.h"

TSharedRef<SDockTab> SPartEditorTab::MakeTab(const FSpawnTabArgs& SpawnTabArgs) {
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SPartEditorViewport)
        ];
}
