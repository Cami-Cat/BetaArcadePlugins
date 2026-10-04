#include "Viewport/RobotEditorViewportClient.h"

FRobotEditorViewportClient::FRobotEditorViewportClient(FPreviewScene& InPreviewScene)
    : FEditorViewportClient(nullptr, &InPreviewScene) {
        bDrawAxes = true;
        SetViewMode(VMI_Unlit);
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
        ZoomCamera(-50.0f);
        return true;
    }
    if (EventArgs.Key == EKeys::MouseScrollDown && EventArgs.Event == IE_Pressed) {
        ZoomCamera(50.0f);
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

        FViewportCursorLocation CursorLocation(this, this, MouseX, MouseY);
        FVector RayOrigin = CursorLocation.GetOrigin();
        FVector RayDirection = CursorLocation.GetDirection();
        FVector RayEnd = RayOrigin + (RayDirection * 10000.0f);

        if (UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get())) {
            float BestHitDistance = FLT_MAX;
            int32 SelectedAnchorIndex = INDEX_NONE;

            for (int32 i = 0, i < ActivePart->Anchors.Num(); i++) {
                FVector PointPos = ActivePart->Anchors[i].Position;

                FVector ClosestPointOnRay = FMath::ClosestPointOnSegment(PointPos, RayOrigin, RayEnd);
                float DistanceToRay = FVector::Dist(PointPos, ClosestPointOnRay);

                if (DistanceToRay < 25.0f &&  DistanceToRay < BestHitDistance) {
                    BestHitDistance = DistanceToRay;
                    SelectedAnchorIndex = i;
                }
            }

            if (SelectedAnchorIndex != INDEX_NONE) {
                if (PreviewScene && PreviewScene->GetWorld() && PreviewScene->GetWorld()->LineBatcher) {
                    PreviewScene->GetWorld()->LineBatcher->DrawPoint(ActivePart->Anchors[SelectedAnchorIndex].Position, FLinearColor::Green, 20.0f, SDPG_Foreground, 2.0f);
                }
            }
        }
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

void FRobotEditorViewportClient::ZoomCamera(float ZoomAmount) {
    m_CameraOrbitRadius = FMath::Clamp(m_CameraOrbitRadius + ZoomAmount, MinZoom, MaxZoom);
    UpdateCameraPositionFromOrbit();
    Invalidate();
}
