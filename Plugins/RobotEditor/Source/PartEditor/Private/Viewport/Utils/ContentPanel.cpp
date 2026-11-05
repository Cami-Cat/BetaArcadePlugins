#include "ThumbnailRendering/ThumbnailManager.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetToolsModule.h"
#include "AssetThumbnail.h"
#include "Viewport/Utils/ContentPanel.h"
#include "Viewport/PartEditorViewport.h"
#include "Viewport/PartEditorViewportClient.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Viewport/EditorViewportClient.h"
#include "Data/PartDat.h"

void SPartThumbnailTile::Construct(const FArguments& InArgs, const FAssetData& InAssetData, TSharedPtr<class SEditorUtilsViewport> InViewportWidget) {
    m_AssetDataRecord = InAssetData;
    m_AssociatedViewport = InViewportWidget;

    TSharedPtr<FEditorUtilsViewportClient> ViewportClient = m_AssociatedViewport.Pin()->GetViewportClient();
    if (ViewportClient.IsValid()) {
        ViewportClient.ConnectTileSingleClick(this);
        ViewportClient.ConnectTileDoubleClick(this);
    }

    FString AssetNameStr = m_AssetDataRecord.AssetName.ToString();

    if (!m_AssetDataRecord.IsValid()) {
        ChildSlot [ SNew(STextBlock).Text(FText::FromString(AssetNameStr)) ];
        return;
    }

    TSharedPtr<FAssetThumbnailPool> GlobalPool = UThumbnailManager::Get().GetSharedThumbnailPool();
    TSharedRef<SWidget> ThumbnailWidget = SNew(STextBlock).Text(INVTEXT("Data"));

    UObject* Outer = m_AssetDataRecord.GetAsset();
    UPDAPart* OuterCast = Cast<UPDAPart>(Outer);

    if (!OuterCast) return;

    UObject* Inner = OuterCast->Object;

    if (Inner) {
        m_AssetThumbnailInstance = MakeShareable(new FAssetThumbnail(Inner, THUMBNAIL_SIZE, THUMBNAIL_SIZE, GlobalPool));
    } else {
        m_AssetThumbnailInstance = MakeShareable(new FAssetThumbnail(OuterCast, THUMBNAIL_SIZE, THUMBNAIL_SIZE, GlobalPool));
    }

    if (m_AssetThumbnailInstance.IsValid()) {
        TSharedPtr<SWidget> NativeWidget = m_AssetThumbnailInstance->MakeThumbnailWidget();
        if (NativeWidget.IsValid()) {
            ThumbnailWidget = NativeWidget.ToSharedRef();
        }
    }

    ChildSlot
    [
        SNew(SBox)
        .WidthOverride(96.0f)
        .HeightOverride(96.0f)
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::Get().GetBrush("Brushes.Panel"))
            .Padding(FMargin(4.0f))
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                [
                    SNew(SBox)
                    .WidthOverride(64.0f)
                    .HeightOverride(64.0f)
                    [
                        ThumbnailWidget
                    ]
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(FMargin(2.0f, 0.0f, 2.0f, 4.0f))
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(AssetNameStr))
                    .Font(FAppStyle::Get().GetFontStyle("NormalFontMicro"))
                ]
            ]
        ]
    ];
}

FReply SPartThumbnailTile::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) {
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
        UObject* FullyLoadedAsset = m_AssetDataRecord.GetAsset();

        UPDAPart* CastPart = Cast<UPDAPart>(FullyLoadedAsset);
        if (CastPart) {
            OnThumbnailTriggered.Broadcast(CastPart);
        }
    }
    return FReply::Unhandled();
}

void SPartContentBrowser::Construct(const FArguments& InArgs, TSharedPtr<SEditorUtilsViewport> InViewportWidget) {
    SContentBrowser::FArguments Args;
    SContentBrowser::Construct(Args, InViewportWidget);
}


void SPartContentBrowser::ScanPluginDirectory() {
    SContentBrowser::ScanPluginDirectory();

    TWeakPtr<SEditorUtilsViewport> WeakViewport = m_AssociatedViewport;
    m_TileGridContainer->AddSlot()
        .Padding(FMargin(4.0f))
        [
            SNew(SBox)
            .WidthOverride(96.0f)
            .HeightOverride(96.0f)
            [
                SNew(SButton)
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                .OnClicked_Lambda([WeakViewport]() -> FReply {
                    TSharedPtr<SPartEditorViewport> CastViewport = StaticCastSharedPtr<SPartEditorViewport>(WeakViewport.Pin());
                    if (!CastViewport.IsValid()) return FReply::Handled();

                    UPDAPart* NewBlankPart = NewObject<UPDAPart>(GetTransientPackage(), UPDAPart::StaticClass());
                    if (NewBlankPart) {
                        CastViewport->SetEditingObject(NewBlankPart);
                    }

                    return FReply::Handled();
                })
                [
                    SNew(STextBlock)
                    .Text(INVTEXT("+"))
                    .Font(FAppStyle::Get().GetFontStyle("HeadingExtraLarge"))
                ]
            ]
        ];
}
