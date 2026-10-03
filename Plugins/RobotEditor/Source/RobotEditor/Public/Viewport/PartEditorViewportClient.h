#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "PreviewScene.h"

class FPartEditorViewportClient : public FEditorViewportClient {
public:
    FPartEditorViewportClient(FPreviewScene& InPreviewScene);
    virtual ~FPartEditorViewportClient() override {}

    virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
    virtual bool InputAxis(const FInputKeyEventArgs& EventArgs) override;
    virtual void Tick(float DeltaSeconds) override;

private:
    FVector m_OrbitTargetPoint;
    float m_CameraOrbitRadius;
    bool m_bIsOrbiting;

    void UpdateCameraPositionFromOrbit();
};
