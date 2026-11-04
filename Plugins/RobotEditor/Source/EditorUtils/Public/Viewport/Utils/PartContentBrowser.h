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

class EDITORUTILS_API SThumbnailTile : public SCompoundWidget {
public:
    const int THUMBNAIL_SIZE = 128;

    SLATE_BEGIN_ARGS(SThumbnailTile) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const FAssetData& InAssetData, TSharedPtr<class SEditorUtilsViewport> InViewportWidget);
    virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

    FOnThumbnailTriggered OnThumbnailTriggered;

protected:

    FAssetData m_AssetDataRecord;
    TWeakPtr<class SEditorUtilsViewport> m_AssociatedViewport;
    TSharedPtr<FAssetThumbnail> m_AssetThumbnailInstance;

};

class EDITORUTILS_API SContentBrowser : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SContentBrowser) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedPtr<class SEditorUtilsViewport> InViewportWidget);
    void ScanPluginDirectory();

    static TWeakPtr<SContentBrowser> m_Instance;

protected:

    TSharedRef<SWidget> OnGenerateTileWidget(FPartAssetEntryPtr Item);
    TWeakPtr<class SEditorUtilsViewport> m_AssociatedViewport;
    TSharedPtr<SWrapBox> m_TileGridContainer;
};

#endif
