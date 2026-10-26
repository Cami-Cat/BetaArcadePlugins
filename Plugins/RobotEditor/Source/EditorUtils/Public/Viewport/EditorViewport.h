#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "PreviewScene.h"
#include "Viewport/EditorViewportClient.h"

class EDITORUTILS_API SEditorUtilsViewport : public SEditorViewport {
public:
    SLATE_BEGIN_ARGS(SEditorUtilsViewport) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual ~SEditorUtilsViewport() override;

protected:
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;

private:
    TUniquePtr<FPreviewScene> m_PreviewScene;
    TSharedPtr<FEditorViewportClient> m_ViewportClient;
};
