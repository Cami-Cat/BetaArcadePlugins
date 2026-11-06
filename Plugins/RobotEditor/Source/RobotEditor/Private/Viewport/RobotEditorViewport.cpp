#include "Viewport/RobotEditorViewport.h"
#include "Widgets/SViewport.h"
#include "Components/SkyLightComponent.h"
#include "SViewportToolBar.h"

void SRobotEditorViewport::Construct(const FArguments& InArgs) {
    SEditorUtilsViewport::Construct(SEditorUtilsViewport::FArguments());
}

TSharedRef<FEditorViewportClient> SRobotEditorViewport::MakeEditorViewportClient() {
    m_ViewportClient = MakeShareable(new FRobotEditorViewportClient(*m_PreviewScene.Get()));
    return m_ViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SRobotEditorViewport::BuildViewportToolbar() {
    return SNew(SViewportToolBar);
}
