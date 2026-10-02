#include "Viewport/RobotEditorViewportClient.h"

FRobotEditorViewportClient::FRobotEditorViewportClient(FPreviewScene& InPreviewScene)
    : FEditorViewportClient(nullptr, &InPreviewScene) {
        bDrawAxes = true;
        SetViewMode(VMI_Lit);
        SetViewportType(LVT_Perspective);

        SetViewLocation(FVector(-500.0f, 300.0f, 300.0f));
        SetViewRotation(FRotator(-20.0f, -30.0f, 0.0f));

        DrawHelper.bDrawGrid = true;
        DrawHelper.GridColor = FColor(100, 100, 100);
        DrawHelper.GridCellSize = 16.0f;
}

void FRobotEditorViewportClient::Tick(float DeltaSeconds) {
    FEditorViewportClient::Tick(DeltaSeconds);

    PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
}
