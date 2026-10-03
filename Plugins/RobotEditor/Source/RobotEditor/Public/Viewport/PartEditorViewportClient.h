#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "IDetailsView.h"
#include "PreviewScene.h"

class FPartEditorViewportClient : public FEditorViewportClient {
public:
    FPartEditorViewportClient(FPreviewScene& InPreviewScene);
    virtual ~FPartEditorViewportClient() override {}

    virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
    virtual bool InputAxis(const FInputKeyEventArgs& EventArgs) override;
    virtual void Tick(float DeltaSeconds) override;

    void SetDetailsView(TSharedPtr<IDetailsView> InDetailsView) { m_AssociatedDetailsView = InDetailsView; }
    void SetInspectedObject(UObject* NewPartObject);

private:
    FVector m_OrbitTargetPoint;
    float m_CameraOrbitRadius;
    bool m_bIsOrbiting;

    TSharedPtr<IDetailsView> m_AssociatedDetailsView;
    TWeakObjectPtr<UObject> m_CurrentInspectedObject;
    TWeakObjectPtr<AActor> m_SpawnedPreviewActor;

    AStaticMeshActor* CreateStaticMeshComponent(UStaticMesh* InObject, FActorSpawnParameters& SpawnParams);
    AActor* CreateActorComponent(UBlueprint* InObject, FActorSpawnParameters& SpawnParams);
    AActor* SpawnActorComponent(UObject* InObject, FActorSpawnParameters& SpawnParams);

    void UpdateCameraPositionFromOrbit();

    float MinZoom = 50.0f;
    float MaxZoom = 5000.0f;

    void ZoomCamera(float ZoomAmount);
};
