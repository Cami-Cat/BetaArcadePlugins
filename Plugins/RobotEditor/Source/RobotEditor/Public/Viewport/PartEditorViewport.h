#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "PreviewScene.h"
#include "Viewport/PartEditorViewportClient.h"

class SPartEditorViewport : public SEditorViewport {
public:
    SLATE_BEGIN_ARGS(SPartEditorViewport) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual ~SPartEditorViewport() override {}

protected:
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
    virtual TSharedPtr<SWidget> BuildViewportToolbar() override;

private:
    FPreviewScene m_PreviewScene;
    TSharedPtr<FPartEditorViewportClient> m_ViewportClient;
};
