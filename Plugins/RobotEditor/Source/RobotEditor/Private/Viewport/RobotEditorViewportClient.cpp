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
        if (bHit.GetActor()) {
            UpdatePreviewActor(true);

            if (m_PreviewActor) {
                m_PreviewActor->SetActorLocationAndRotation(bHit.ImpactPoint, FRotator::ZeroRotator);
            }

            SnapToNearestAnchor(bHit, m_PreviewActor);
        }
    } else {
        UpdatePreviewActor(false);
        MovePreviewActorToLocation(bHit.TraceEnd);
    }
    FEditorUtilsViewportClient::Invalidate();
}

void FRobotEditorViewportClient::SnapToNearestAnchor(const FHitResult& HitResult, AActor* SnapActor) {
    if (!HitResult.bBlockingHit || !SnapActor) {
        return;
    }

    AActor* HitActor = HitResult.GetActor();
    const FPartData& HitData = GetPartDataFromActor(HitActor);

    if (!HitData.Part || !m_SelectedPart || HitData.Part->Anchors.Num() == 0 || m_SelectedPart->Anchors.Num() == 0) {
        return;
    }

    /*
     * Our first iteration. In this loop, we find the closest anchor to the impact position of the ray.
     * We then store a copy of the anchor to perform some transforms on.
     */

    FTransform HitTransform = HitActor->GetActorTransform();

    FAnchor TargetHitAnchor;
    FVector ClosestAnchorWorldPos = FVector::ZeroVector;
    float ClosestDistanceSq = MAX_FLT;
    int TargetAnchorIndex = 0;
    int CurrentIndex = 0;

    for (const FAnchor& Anchor : HitData.Part->Anchors) {
        FVector AnchorWorldPos = HitTransform.TransformPosition(Anchor.Position);
        float DistanceSq = FVector::DistSquared(AnchorWorldPos, HitResult.ImpactPoint);

        if (DistanceSq < ClosestDistanceSq) {
            ClosestAnchorWorldPos = AnchorWorldPos;
            TargetAnchorIndex = CurrentIndex;
            ClosestDistanceSq = DistanceSq;
            TargetHitAnchor = Anchor;
        }
        CurrentIndex++;
    }

    /*
     * If there isn't an anchor within a relatively normal set distance, (in this case we'll use the imaginary numer Infinity as our test point), then there's nothing to snap to.
     */

    if (ClosestDistanceSq > MAX_FLT) {
        return;
    }

    /*
     * We need to get the normal of the anchor based on the transform of the actor that we're reading from. We just simply call GetSafeNormal(); A fun little function that already exists inside Unreal.
     * But that's not all, we also need to find the target snap. This acts as the "opposite" of the hit anchor normal, so it ensures that the anchor that you get always has a normal facing INTO it,
     * rather than away from.
     */

    FVector HitAnchorNormalWorld = HitTransform.TransformVector(TargetHitAnchor.Position.GetSafeNormal()).GetSafeNormal();
    FVector TargetSnapNormalWorld = -HitAnchorNormalWorld;

    /*
     * Now we score differently here, because the object can be drastically different depending on where you're colliding. We need to factor in a few things here.
     * 1) Orientation of the object:
     *
     *  An object might be a cylinder, as opposed to a cube. This cylinder may be harder to calculate distances from if they're equidistant from the center.
     *
     * 2) The dot product of the normals
     *
     *  A normal might have a greater or lesser dot-product depending on where it is on the object.
     *
     * 3) The position of the anchors relative to the object.
     *
     * Combining these three rules gets us a total score that allows us to closer choose an anchor.
     */

    FTransform SnapTransform = SnapActor->GetActorTransform();
    FAnchor SnapAnchor;
    int TargetSnapAnchorIndex = 0;
    CurrentIndex = 0;

    float BestAnchorScore = -MAX_FLT;

    for (const FAnchor& Anchor : m_SelectedPart->Anchors) {
        FVector LocalNormal = Anchor.Position.IsNearlyZero() ? FVector::UpVector : Anchor.Position.GetSafeNormal();

        FVector AnchorNormalWorld = SnapTransform.TransformVector(LocalNormal).GetSafeNormal();
        float DirectionScore = FVector::DotProduct(AnchorNormalWorld, TargetSnapNormalWorld);

        FVector AnchorWorldPos = SnapTransform.TransformPosition(Anchor.Position);
        float DistanceSq = FVector::DistSquared(AnchorWorldPos, ClosestAnchorWorldPos);
        float ProximityScore = -(DistanceSq / 10000.0f);

        float TotalScore = DirectionScore + ProximityScore;

        if (TotalScore > BestAnchorScore) {
            TargetSnapAnchorIndex = CurrentIndex;
            BestAnchorScore = TotalScore;
            SnapAnchor = Anchor;
        }
        CurrentIndex++;
    }

    /* Long, Boring and indescribable maths up ahead. This required Quarternions, Dot Products and Vector Transforms. I never want to touch maths ever again.
     * Nevertheless, time to explain how this works.
     *
     * Below, we declare LocalNormal and WorldNormals. LocalNormal determines that if an Anchor's position is nearly 0, 0, 0, we prevent a divide by 0 issue and declare that the anchor is pointing
     * upwards through an arbitrary rule. This should only affect anchors that are not correctly put in place.
     *
     * We then calculate the actual normal based on the transform in the world, and turn it into a value between -1.0 and 1.0 for our necessary transformations.
     */
    FVector LocalNormal = SnapAnchor.Position.IsNearlyZero() ? FVector::UpVector : SnapAnchor.Position.GetSafeNormal();
    FVector SnapAnchorNormalWorld = SnapTransform.TransformVector(LocalNormal).GetSafeNormal();

    /* We want our objects to handle rotations. For example, should you be hovering over the side of a six-sided cube and have an object with a single anchor point, this will align the points together.
     * Thankfully, Unreal has a bunch of useful Quarternion functions inside their actual Quarternion data, so we just need to find the shortest safe angular rotation to get us to our desired point.
     * That's what this lovely FindBetweenNormals function does, it finds the shortest angular path to a point.
     */

    FQuat AlignmentRotation = FQuat::FindBetweenNormals(SnapAnchorNormalWorld, TargetSnapNormalWorld);
    FQuat FinalRotation = AlignmentRotation * SnapTransform.GetRotation();

    /* Here we handle user-input-based rotation. When the user presses either Q or E, the m_CurrentRotationInputSteps will increment or decrement with a % of 4, ensuring that it will always wrap around
     * between 0 -> 3.
     *
     * We then multiply our current rotation input steps by the amount we actually want an object to rotate, in this case we just want snapping every 90 degrees or so, so we can do so here.
     * (I also did this during Advanced Development Techniques, through `(TAU * 4)) * rotation_num`. So this is a fun useful little tidbit for rotation snapping!)
     *
     * We then apply this rotation after the previous one. (Quarternion multiplication is different to actual multiplication. Consider it adding angles sequentially rather than multiplying the
     * values directly.)
     *
     * TODO : Actually increment m_CurrentRotationNum in InputKey()
     */
    float AngleOffset = m_CurrentRotationNum * 90.0f;
    FQuat AxisStepRotation = FQuat(TargetSnapNormalWorld, FMath::DegreesToRadians(AngleOffset));
    FinalRotation = AxisStepRotation * FinalRotation;

    /*
     * We then rotate our anchor vector by this final quarternion rotation, and can set our location relative to all of the maths that we just performed to find the optimal place to put the damn thing.
     */
    FVector RotatedSnapLocalAnchor = FinalRotation.RotateVector(SnapAnchor.Position);
    FVector FinalWorldLocation = ClosestAnchorWorldPos - RotatedSnapLocalAnchor;

    /*
     * Then all we need to do is set the location and the rotation, and the snapping is complete.
     */

    SnapActor->SetActorLocationAndRotation(FinalWorldLocation, FinalRotation);

    FEditorUtilsViewportClient::Invalidate();
    return;
}

void FRobotEditorViewportClient::CreatePartFromPreview() {
    if (!m_PreviewActor || !m_SelectedPart) {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    FTransform SpawnTransform = m_PreviewActor->GetActorTransform();

    AActor* NewActor = FEditorUtilsViewportClient::CreateActorComponent(m_SelectedPart->Object, SpawnParams);

    if (NewActor) {
        NewActor->SetActorTransform(SpawnTransform);
        m_Actors.Emplace(NewActor, FPartData(m_SelectedPart, SpawnTransform, {}));
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
        if (!m_SelectedPart) { return FEditorUtilsViewportClient::InputKey(EventArgs); }
        /*
         *   If there isn't a part displayed on the screen, place immediately at 0, 0, 0. If there is a part on the screen, perform a cast and ensure that the cast is hitting something.
         *   If it isn't, pass this down thorugh FEditorUtilsViewportClient::InputKey(EventArgs);
         */
        if (m_Actors.Num() == 0) {
            CreatePartAtLocation(FVector::ZeroVector);
        }
        else {
            FHitResult HitResult = GetCastHitResult();
            if (HitResult.bBlockingHit) {
                if (m_PreviewActor) {
                    CreatePartFromPreview();
                } else {
                    CreatePartAtLocation(HitResult.ImpactPoint);
                }
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

FPartData FRobotEditorViewportClient::GetPartDataFromActor(AActor* Actor) {
    if (m_Actors.Find(Actor)) {
        FPartData& FindData = *m_Actors.Find(Actor);
        return FindData;
    }

    return FPartData();
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
