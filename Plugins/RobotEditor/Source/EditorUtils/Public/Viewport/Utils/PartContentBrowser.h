#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STileView.h"
#include "Widgets/Layout/SWrapBox.h"
#include "AssetRegistry/AssetData.h"

class UPDAPart;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnThumbnailTriggered, UPDAPart*);

typedef TSharedPtr<FAssetData> FPartAssetEntryPtr;

class SThumbnailTile : public SCompoundWidget {
public:
    const int THUMBNAIL_SIZE = 128;

    SLATE_BEGIN_ARGS(SThumbnailTile) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const FAssetData& InAssetData, TSharedPtr<class SEditorViewport> InViewportWidget);

    FOnThumbnailTriggered OnThumbnailTriggered;

private:

    FAssetData m_AssetDataRecord;
    TWeakPtr<class SEditorViewport> m_AssociatedViewport;
    TSharedPtr<FAssetThumbnail> m_AssetThumbnailInstance;

};

class SContentBrowser : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SContentBrowser) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedPtr<class SEditorViewport> InViewportWidget);
    void ScanPluginDirectory();

    static TWeakPtr<SContentBrowser> m_Instance;
private:

    TSharedRef<SWidget> OnGenerateTileWidget(FPartAssetEntryPtr Item);
    TWeakPtr<class SEditorViewport> m_AssociatedViewport;
    TSharedPtr<SWrapBox> m_TileGridContainer;
};

#endif
