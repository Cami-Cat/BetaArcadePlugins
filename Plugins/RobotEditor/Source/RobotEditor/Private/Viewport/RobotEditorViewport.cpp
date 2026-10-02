#include "Viewport/RobotEditorViewport.h"
#include "Widgets/SViewport.h"
#include "SViewportToolBar.h"

void SRobotEditorViewport::Construct(const FArguments& InArgs) {
    SEditorViewport::Construct(SEditorViewport::FArguments());

    // Here we can test spawn something into our preview scene on constructing the viewport. Here I'm going to spawn a skylight.

    USkyLightComponent* Skylight = NewObject<USkyLightComponent>(m_PreviewScene.GetWorld());
    m_PreviewScene.AddComponent(SkyLight, FTransform::Identiy);
}

TSharedPtr<FEditorViewportClient> SRobotEditorViewport::CreateViewportClient() {
    m_ViewportClient = MakeShareable(new FRobotEditorViewportClient(m_PreviewScene))
    return m_ViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SRobotEditorViewport::BuildViewportToolbar() {
    return SNew(SViewportToolBar);
}
