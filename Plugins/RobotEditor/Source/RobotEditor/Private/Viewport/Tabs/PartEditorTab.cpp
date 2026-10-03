#include "Viewport/Tabs/PartEditorTab.h"
#include "Viewport/PartEditorViewport.h"
#include "Viewport/PartEditorViewportClient.h"
#include "Widgets/Layout/SSplitter.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"

TSharedRef<SDockTab> SPartEditorTab::MakeTab(const FSpawnTabArgs& SpawnTabArgs) {

    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bAllowSearch = false;
    DetailsViewArgs.bUpdatesFromSelection = false;
    DetailsViewArgs.bShowOptions = false;
    DetailsViewArgs.bAllowFavoriteSystem = false;

    TSharedRef<IDetailsView> PartsDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    TSharedRef<SPartEditorViewport> ViewportWidget = SNew(SPartEditorViewport);
    TSharedPtr<FPartEditorViewportClient> ClientPtr = ViewportWidget->GetPartClient();
    if (ClientPtr.IsValid()) {
        ClientPtr->SetDetailsView(PartsDetailsView);
    }

    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SSplitter)
            .Orientation(Orient_Horizontal)

            +SSplitter::Slot()
            .Value(0.7f)
            [
                ViewportWidget
            ]

            +SSplitter::Slot()
            .Value(0.3f)
            [
                SNew(SVerticalBox)
                +SVerticalBox::Slot()
                .AutoHeight()
                .Padding(4.0f)
                [
                    SNew(STextBlock)
                    .Text(INVTEXT("Parts Inspector"))
                    .Font(FAppStyle::Get().GetFontStyle("NormalFontBold"))
                ]
                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                [
                    PartsDetailsView
                ]
            ]
        ];
}
