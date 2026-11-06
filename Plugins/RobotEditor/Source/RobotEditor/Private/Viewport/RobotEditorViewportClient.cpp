#include "Viewport/RobotEditorViewportClient.h"
#include "Viewport/Utils/PartContentBrowser.h"
#include "Components/StaticMeshComponent.h"
#include "Structs/RobotData.h"
#include "Data/PartDat.h"

FRobotEditorViewportClient::FRobotEditorViewportClient(FPreviewScene& InPreviewScene)
    : FEditorUtilsViewportClient(InPreviewScene) {
        return;
}

void FRobotEditorViewportClient::Tick(float DeltaSeconds) {
    FEditorUtilsViewportClient::Tick(DeltaSeconds);

    /*
     * TODO : Turn this into a function and take it out of tick. Preferrably this could be something in the parent class: FEditorUtilsViewportClient.
     */

    FHitResult bHit = GetCastHitResult();
    if (bHit.bBlockingHit) {
        AActor* HitActor = bHit.GetActor();
        FVector HitLocation = bHit.ImpactPoint;

        if (HitActor) {
            UpdatePreviewActor(true);
            MovePreviewActorToLocation(HitLocation);
            return;
        }
    } else {
        UpdatePreviewActor(false);
        MovePreviewActorToLocation(bHit.TraceEnd);
    }
}

FHitResult FRobotEditorViewportClient::GetCastHitResult() {
    UWorld* World = PreviewScene->GetWorld();

    TTuple<FVector, FVector> RayData = FEditorUtilsViewportClient::CastRayFromCameraToMouse();
    const auto& [RayStart, RayEnd] = RayData;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(m_PreviewActor);

    bool bHit = World->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_Visibility, Params);
    return HitResult;
}

bool FRobotEditorViewportClient::InputKey(const FInputKeyEventArgs& EventArgs) {
    if (EventArgs.Key == EKeys::LeftMouseButton && EventArgs.Event == IE_Pressed) {
        /*
         *   If there isn't a part displayed on the screen, place immediately at 0, 0, 0. If there is a part on the screen, perform a cast and ensure that the cast is hitting something.
         *   If it isn't, pass this down thorugh FEditorUtilsViewportClient::InputKey(EventArgs);
         */
        if (!m_SelectedPart) { return FEditorUtilsViewportClient::InputKey(EventArgs); }
        if (m_Actors.Num() == 0) {
            CreatePartAtLocation(FVector::ZeroVector);
        }
        else {
            FHitResult HitResult = GetCastHitResult();
            if (HitResult.bBlockingHit) {
                CreatePartAtLocation(HitResult.ImpactPoint);
            }
        }

         return true;
    }

    return FEditorUtilsViewportClient::InputKey(EventArgs);
}

void FRobotEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) {
    FEditorUtilsViewportClient::Draw(View, PDI);
}

void FRobotEditorViewportClient::SetSelectedPart(UPDAPart* PartData) {
    UWorld* PreviewWorld = PreviewScene->GetWorld();
    if (!PreviewWorld) return;

    m_SelectedPart = PartData;

    if (m_PreviewActor) {
        m_PreviewActor->Destroy();
        m_PreviewActor->Reset();
    }

    AActor* NewActor = nullptr;
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    if (PartData) {
        if (PartData->Object) {
            NewActor = FEditorUtilsViewportClient::CreateActorComponent(PartData->Object, SpawnParams);
            if (NewActor) {
                m_PreviewActor = NewActor;
            }
        }
    }
    FEditorUtilsViewportClient::Invalidate();
}

void FRobotEditorViewportClient::UpdatePreviewActor(bool ShouldShow) {
    const bool bPropagate = true;
    if (m_PreviewActor) {
        if (USceneComponent* Root = m_PreviewActor->GetRootComponent()) {
            Root->SetVisibility(ShouldShow, bPropagate);
        }
    }
    return;
}

void FRobotEditorViewportClient::MovePreviewActorToLocation(FVector Location) {
    if (m_PreviewActor) {
        m_PreviewActor->SetActorLocation(Location);
    }
    FEditorUtilsViewportClient::Invalidate();
    return;
}

void FRobotEditorViewportClient::CreatePartAtLocation(FVector Location) {
    if (!m_SelectedPart) { return; }

    if (m_SelectedPart->Object) {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        FTransform SpawnTransform;
        SpawnTransform.SetLocation(Location);
        SpawnTransform.SetRotation(FRotator(0.0f, 90.0f, 0.0f).Quaternion());

        AActor* NewActor = FEditorUtilsViewportClient::CreateActorComponent(m_SelectedPart->Object, SpawnParams);
        NewActor->SetActorTransform(SpawnTransform);

        if (NewActor) {
            m_Actors.Emplace(NewActor, FPartData(m_SelectedPart, SpawnTransform, {}));
        }
    }

    FEditorUtilsViewportClient::Invalidate();
    return;
}

void FRobotEditorViewportClient::SetSelectedActorsData(const FPartData& PartData) {
    return;
}

TArray<FTransform> FRobotEditorViewportClient::GetAnchorsRelativeToPart(UPDAPart* Part) {
    return {};
}

FPartData FRobotEditorViewportClient::GetSelectedPartData() {
    return m_Actors[m_SelectedActor];
}


AActor* FRobotEditorViewportClient::GetActor() {
    return m_SelectedActor;
}

FVector FRobotEditorViewportClient::GetActorLocation(AActor* Actor) {
    return Actor->GetActorLocation();
}

void FRobotEditorViewportClient::OnThumbnailTriggered(UPDAPart* Part) {
    SetSelectedPart(Part);
    return;
}

void FRobotEditorViewportClient::ConnectTileSingleClick(SThumbnailTile* Tile) {
    Tile->OnThumbnailClicked.AddSP(this, &FRobotEditorViewportClient::OnThumbnailTriggered);
}

void FRobotEditorViewportClient::ConnectTileDoubleClick(SThumbnailTile* Tile) {
    return;
}

void FRobotEditorViewportClient::OnAddButtonPressed() {
    return;
}
