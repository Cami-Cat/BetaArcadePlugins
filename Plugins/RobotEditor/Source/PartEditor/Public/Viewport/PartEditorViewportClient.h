#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "IDetailsView.h"
#include "PreviewScene.h"

DECLARE_MULTICAST_DELEGATE(FOnSelectedObjectChanged);

enum class EPartSelectionType : uint8 {
    None,
    Anchor,
    CollisionPoint
};

class FPartEditorViewportClient : public FEditorViewportClient {
public:
    FPartEditorViewportClient(FPreviewScene& InPreviewScene);
    virtual ~FPartEditorViewportClient() override;

    void SaveActiveAsset();
    virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
    virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
    virtual bool InputAxis(const FInputKeyEventArgs& EventArgs) override;
    virtual void Tick(float DeltaSeconds) override;

    void SetDetailsView(TSharedPtr<IDetailsView> InDetailsView);
    TSharedPtr<IDetailsView> GetDetailsView() { return m_AssociatedDetailsView; }
    void SetInspectedObject(UObject* NewPartObject);
    TWeakObjectPtr<UObject> GetInspectedObject() { return m_CurrentInspectedObject; }
    void SetPartSelectionType(EPartSelectionType Type) { m_SelectedType = Type; }

    void SetSelectedIndex(int32 NewIndex) { m_SelectedIndex = NewIndex; }
    int32 GetSelectedIndex() const { return m_SelectedIndex; }

    virtual bool CanSetWidgetMode(UE::Widget::EWidgetMode NewMode) const override { return true; }
    virtual UE::Widget::EWidgetMode GetWidgetMode() const override;
    virtual FVector GetWidgetLocation() const override;
    virtual bool InputWidgetDelta(FViewport* InViewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale) override;

    FOnSelectedObjectChanged OnSelectedObjectChanged;

private:
    FVector m_OrbitTargetPoint;
    float m_CameraOrbitRadius;
    bool m_bIsOrbiting;
    bool m_bDrawAnchors = true;
    bool m_bDrawCollision = true;

    TSharedPtr<IDetailsView> m_AssociatedDetailsView;
    TWeakObjectPtr<UObject> m_CurrentInspectedObject;
    TWeakObjectPtr<AActor> m_SpawnedPreviewActor;

    const FColor c_UnselectedAnchorColor = FColor(255, 255, 255);
    const FColor c_SelectedAnchorColor = FColor(0, 255, 64);
    const FLinearColor c_CollisionPoint = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
    const FLinearColor c_SelectedCollisionPoint = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    const FLinearColor c_BoundingBoxWire = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
    const FLinearColor c_BoundingBoxInnards = FLinearColor(1.0f, 0.0f, 0.0f, 0.2f);
    const int c_BoundingBoxWireThickness = 1;
    const float c_AnchorSize = 12.0f;
    const float c_SelectedAnchorSize = c_AnchorSize * 1.5;
    const float GridSize = 1.0f;

    int32 m_SelectedIndex = -1;
    EPartSelectionType m_SelectedType = EPartSelectionType::None;

    AStaticMeshActor* CreateStaticMeshComponent(UStaticMesh* InObject, FActorSpawnParameters& SpawnParams);
    AActor* CreateActorComponent(UBlueprint* InObject, FActorSpawnParameters& SpawnParams);
    AActor* SpawnActorComponent(UObject* InObject, FActorSpawnParameters& SpawnParams);

    void UpdateCameraPositionFromOrbit();
    void ZoomCamera(float ZoomAmount);

    const float c_MinZoom = 50.0f;
    const float c_MaxZoom = 5000.0f;
    const float c_ZoomAmount = 50.0f;

    void OnInspectorPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);

};
