#include "Viewport/PartEditorViewportClient.h"
#include "Viewport/Utils/AnchorsPanel.h"
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
            .Orientation(Orient_Vertical)

            + SSplitter::Slot()
            .Value(0.75f)
            [
                SNew(SSplitter)
                .Orientation(Orient_Horizontal)

                + SSplitter::Slot()
                .Value(0.2f)
                [
                    SNew(SAnchorsPanel, ViewportWidget)
                ]
                + SSplitter::Slot()
                .Value(0.55f)
                [
                    ViewportWidget
                ]

                + SSplitter::Slot()
                .Value(0.25f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(6.0f)
                    .HAlign(HAlign_Center)
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
            ]

            + SSplitter::Slot()
            .Value(0.25f)
            [
                SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(6.0f, 4.0f)
                [
                    SNew(STextBlock)
                    .Text(INVTEXT("Parts Library Asset Registry"))
                    .Font(FAppStyle::Get().GetFontStyle("NormalFontBold"))
                ]

                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                .Padding(4.0f)
                [
                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(4.0f)
                    [
                        SNew(SBox)
                        .WidthOverride(96.0f)
                        .HeightOverride(96.0f)
                        [
                            SNew(SButton)
                            .ButtonStyle(FAppStyle::Get(), "SimpleButton")
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
                                .Text(INVTEXT("+"))
                                .Font(FAppStyle::Get().GetFontStyle("HeadingExtraLarge"))
                            ]
                        ]
                    ]

                    + SHorizontalBox::Slot()
                    .FillWidth(1.0f)
                    .Padding(8.0f, 0.0f)
                    [
                        SNew(SBorder)
                        .BorderImage(FAppStyle::Get().GetBrush("Brushes.Panel"))
                        .HAlign(HAlign_Center)
                        .VAlign(VAlign_Center)
                        [
                            SNew(STextBlock)
                            .Text(INVTEXT("[Asset Thumbnail Gallery Row Placeholder]"))
                        ]
                    ]
                ]
            ]
        ];
}
