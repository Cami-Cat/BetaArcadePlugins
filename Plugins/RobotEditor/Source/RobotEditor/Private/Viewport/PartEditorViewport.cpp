#include "Viewport/PartEditorViewport.h"
#include "Widgets/SViewport.h"
#include "Components/SkyLightComponent.h"
#include "SViewportToolBar.h"

void SPartEditorViewport::Construct(const FArguments& InArgs) {
    SEditorViewport::Construct(SEditorViewport::FArguments());
}

TSharedRef<FEditorViewportClient> SPartEditorViewport::MakeEditorViewportClient() {
    m_ViewportClient = MakeShareable(new FPartEditorViewportClient(m_PreviewScene));
    return m_ViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SPartEditorViewport::BuildViewportToolbar() {
    return SNew(SViewportToolBar);
}
