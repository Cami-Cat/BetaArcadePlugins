#include "Viewport/RobotEditorViewportClient.h"

FRobotEditorViewportClient::FRobotEditorViewportClient(FPreviewScene& InPreviewScene)
    : FEditorViewportClient(nullptr, &InPreviewScene) {
        bDrawAxes = true;
        SetViewMode(VMI_Lit);
        SetViewportType(LVT_Perspective);
        DrawHelper.bDrawGrid = true;

        m_OrbitTargetPoint = FVector::ZeroVector;
        m_CameraOrbitRadius = 600.0f;
        m_bIsOrbiting = false;

        SetViewRotation(FRotator(-30.0f, -45.0f, 0.0f));
        UpdateCameraPositionFromOrbit();
        EngineShowFlags.SetEyeAdaptation(false);
        Invalidate();
}

void FRobotEditorViewportClient::Tick(float DeltaSeconds) {
    FEditorViewportClient::Tick(DeltaSeconds);

    PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
}

bool FRobotEditorViewportClient::InputKey(const FInputKeyEventArgs& EventArgs) {
    if (EventArgs.Key == EKeys::MouseScrollUp && EventArgs.Event == IE_Pressed) {
        m_CameraOrbitRadius = FMath::Max(50.0f, m_CameraOrbitRadius - 50.0f);
        UpdateCameraPositionFromOrbit();
        Invalidate();
        return true;
    }
    if (EventArgs.Key == EKeys::MouseScrollDown && EventArgs.Event == IE_Pressed) {
        m_CameraOrbitRadius = FMath::Min(5000.0f, m_CameraOrbitRadius + 50.0f);
        UpdateCameraPositionFromOrbit();
        Invalidate();
        return true;
    }

    if (EventArgs.Key == EKeys::RightMouseButton) {
        if (EventArgs.Event == IE_Pressed) {
            m_bIsOrbiting = true;
            Viewport->LockMouseToViewport(true);
        }
        else {
            m_bIsOrbiting = false;
            Viewport->LockMouseToViewport(false);
        }
        return true;
    }

    if (EventArgs.Key == EKeys::LeftMouseButton && EventArgs.Event == IE_Pressed) {
        int32 MouseX = Viewport->GetMouseX();
        int32 MouseY = Viewport->GetMouseY();

        UE_LOG(LogTemp, Warning, TEXT("Click triggered at coordinates: [%d,%d]"), MouseX, MouseY);
        return true;
    }

    return FEditorViewportClient::InputKey(EventArgs);
}

bool FRobotEditorViewportClient::InputAxis(const FInputKeyEventArgs& EventArgs) {
    if (!m_bIsOrbiting) return false;


    FRotator CurrentRotation = GetViewRotation();
    float Delta = EventArgs.AmountDepressed;

    if (EventArgs.Key == EKeys::MouseX) {
        CurrentRotation.Yaw += Delta * 0.5f;
        SetViewRotation(CurrentRotation);
        UpdateCameraPositionFromOrbit();
        Invalidate();
        return true;
    }
    else if (EventArgs.Key == EKeys::MouseY) {
        CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch + (Delta * 0.5f), -85.0f, 85.0f);
        SetViewRotation(CurrentRotation);
        UpdateCameraPositionFromOrbit();
        Invalidate();
        return true;
    }

    return FEditorViewportClient::InputAxis(EventArgs);
}

void FRobotEditorViewportClient::UpdateCameraPositionFromOrbit() {
    FVector LookDirection = GetViewRotation().Vector();
    FVector NewLocation = m_OrbitTargetPoint - (LookDirection * m_CameraOrbitRadius);
    SetViewLocation(NewLocation);
}
