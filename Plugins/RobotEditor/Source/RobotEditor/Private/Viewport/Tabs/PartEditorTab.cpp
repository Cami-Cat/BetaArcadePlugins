#include "Viewport/PartEditorViewportClient.h"
#include "Viewport/Tabs/PartEditorTab.h"
#include "Viewport/PartEditorViewport.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Input/SButton.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "RobotData/PartDat.h"
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
                .HAlign(HAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(INVTEXT("Parts Inspector"))
                    .Font(FAppStyle::Get().GetFontStyle("NormalFontBold"))
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(4.0f, 2.0f, 4.0f, 6.0f)
                [
                    SNew(SBox)
                    .HeightOverride(AddPartButtonHeight)
                    [
                        SNew(SButton)
                        .HAlign(HAlign_Center)
                        .VAlign(VAlign_Center)
                        .OnClicked_Lambda([ClientPtr]() -> FReply {
                            if (ClientPtr.IsValid()) {
                                UPDAPart* NewBlankPart = NewObject<UPDAPart>(GetTransientPackage(), UPDAPart::StaticClass());
                                if (NewBlankPart) {
                                    ClientPtr->SetInspectedObject(NewBlankPart);
                                }
                            }
                            return FReply::Handled();
                        })
                        [
                            SNew(STextBlock)
                            .Text(INVTEXT("+ Create New Part"))
                            .Font(FAppStyle::Get().GetFontStyle("NormalFontBold"))
                        ]
                    ]
                ]
                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                [
                    PartsDetailsView
                ]
            ]
        ];
}
