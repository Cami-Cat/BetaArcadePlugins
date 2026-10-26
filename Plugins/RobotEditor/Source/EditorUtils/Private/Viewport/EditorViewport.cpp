#include "Viewport/EditorViewport.h"
#include "Widgets/SViewport.h"
#include "SViewportToolBar.h"
#include "Framework/Commands/UICommandInfo.h"

void SEditorUtilsViewport::Construct(const FArguments& InArgs) {
    m_PreviewScene = MakeUnique<FPreviewScene>();
    SEditorViewport::Construct(SEditorViewport::FArguments());
}

SEditorUtilsViewport::~SEditorUtilsViewport()
{
    if (m_ViewportClient.IsValid()) {
        m_ViewportClient->Viewport = nullptr;
    }
    if (m_PreviewScene.IsValid()) {
        m_PreviewScene.Reset();
    }
}

TSharedRef<FEditorViewportClient> SEditorUtilsViewport::MakeEditorViewportClient() {
    if (m_PreviewScene.IsValid()) {
        m_ViewportClient = MakeShareable(new FEditorUtilsViewportClient(*m_PreviewScene.Get()));
    } else {
        m_PreviewScene = MakeUnique<FPreviewScene>();
        m_ViewportClient = MakeShareable(new FEditorUtilsViewportClient(*m_PreviewScene.Get()));
    }

    m_ViewportClient->ViewportType = LVT_Perspective;
    m_ViewportClient->bDrawAxes = true;

    return m_ViewportClient.ToSharedRef();
}
