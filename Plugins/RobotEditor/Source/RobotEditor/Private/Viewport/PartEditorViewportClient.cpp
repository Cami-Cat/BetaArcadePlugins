#include "Viewport/PartEditorViewportClient.h"
#include "Engine/StaticMesh.h"
#include "Engine/Blueprint.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "RobotData/PartDat.h"

FPartEditorViewportClient::FPartEditorViewportClient(FPreviewScene& InPreviewScene)
    : FEditorViewportClient(nullptr, &InPreviewScene) {
        bDrawAxes = true;
        SetViewMode(VMI_Lit);
        SetViewportType(LVT_Perspective);
        DrawHelper.bDrawGrid = true;

        m_OrbitTargetPoint = FVector::ZeroVector;
        m_CameraOrbitRadius = 50.0f;
        m_bIsOrbiting = false;

        SetViewRotation(FRotator(-30.0f, -45.0f, 0.0f));
        UpdateCameraPositionFromOrbit();
        EngineShowFlags.SetEyeAdaptation(false);
        Invalidate();
}

void FPartEditorViewportClient::Tick(float DeltaSeconds) {
    FEditorViewportClient::Tick(DeltaSeconds);

    PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
}

bool FPartEditorViewportClient::InputKey(const FInputKeyEventArgs& EventArgs) {
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

        UE_LOG(LogTemp, Warning, TEXT("Click triggered at coordinates: [%d,%d]"), MouseX, MouseY);
        return true;
    }

    return FEditorViewportClient::InputKey(EventArgs);
}

bool FPartEditorViewportClient::InputAxis(const FInputKeyEventArgs& EventArgs) {
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

void FPartEditorViewportClient::UpdateCameraPositionFromOrbit() {
    FVector LookDirection = GetViewRotation().Vector();
    FVector NewLocation = m_OrbitTargetPoint - (LookDirection * m_CameraOrbitRadius);
    SetViewLocation(NewLocation);
}

void FPartEditorViewportClient::SetInspectedObject(UObject* NewPartObject) {
    if (!NewPartObject) return;

    m_CurrentInspectedObject = NewPartObject;

    if (m_AssociatedDetailsView.IsValid()) {
        m_AssociatedDetailsView->SetObject(NewPartObject);
    }

    UWorld* PreviewWorld = PreviewScene->GetWorld();
    if (!PreviewWorld) return;

    if (m_SpawnedPreviewActor.IsValid()) {
        m_SpawnedPreviewActor->Destroy();
        m_SpawnedPreviewActor.Reset();
    }

    AActor* NewActor = nullptr;
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    UPDAPart* DataAsset = Cast<UPDAPart>(NewPartObject);

    if (!DataAsset) {
        NewActor = SpawnActorComponent(NewPartObject, SpawnParams);
    } else {
        if (DataAsset->Object) {
            NewActor = SpawnActorComponent(DataAsset->Object, SpawnParams);
         }
    }

    if (NewActor) {
        m_SpawnedPreviewActor = NewActor;

        TInlineComponentArray<UActorComponent*> ActorComponents;
        NewActor->GetComponents(ActorComponents);
        for (UActorComponent* Component : ActorComponents) {
            if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component)) {
                PreviewScene->AddComponent(PrimitiveComponent, PrimitiveComponent->GetRelativeTransform());
            }
        }
    }

    Invalidate();
}

AStaticMeshActor* FPartEditorViewportClient::CreateStaticMeshComponent(UStaticMesh* InObject, FActorSpawnParameters& SpawnParams) {
    UWorld* PreviewWorld = PreviewScene->GetWorld();
    AStaticMeshActor* MeshActor = PreviewWorld->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (MeshActor && MeshActor->GetStaticMeshComponent()) {
        MeshActor->GetStaticMeshComponent()->SetStaticMesh(InObject);
        return MeshActor;
    }
    return nullptr;
}

AActor* FPartEditorViewportClient::CreateActorComponent(UBlueprint* InObject, FActorSpawnParameters& SpawnParams) {
    UWorld* PreviewWorld = PreviewScene->GetWorld();
    if (UClass* ActorClass = InObject->GeneratedClass) {
        return PreviewWorld->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    }
    return nullptr;
}

AActor* FPartEditorViewportClient::SpawnActorComponent(UObject* InObject, FActorSpawnParameters& SpawnParams) {
    if (UStaticMesh* InnerMesh = Cast<UStaticMesh>(InObject)) {
        return CreateStaticMeshComponent(InnerMesh, SpawnParams);
    }
    else if (UBlueprint* InnerBP = Cast<UBlueprint>(InObject)) {
        return CreateActorComponent(InnerBP, SpawnParams);
    }
    return nullptr;
}

void FPartEditorViewportClient::ZoomCamera(float ZoomAmount) {
    m_CameraOrbitRadius = FMath::Clamp(m_CameraOrbitRadius + ZoomAmount, MinZoom, MaxZoom);
    UpdateCameraPositionFromOrbit();
    Invalidate();
}
