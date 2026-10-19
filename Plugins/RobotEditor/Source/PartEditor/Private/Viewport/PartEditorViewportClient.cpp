#include "Viewport/PartEditorViewportClient.h"
#include "Engine/StaticMesh.h"
#include "Engine/Blueprint.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "SceneManagement.h"
#include "Viewport/Utils/ContentPanel.h"
#include "DrawDebugHelpers.h"
#include "Data/PartDat.h"
#include "SnappingUtils.h"
#include "ScopedTransaction.h"
#include "MaterialDomain.h"
#include "Materials/MaterialRenderProxy.h"
#include "MouseDeltaTracker.h"
#include "Framework/Application/SlateApplication.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "FileHelpers.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "UObject/Package.h"
#include "Interfaces/IMainFrameModule.h"
#include "UnrealWidget.h"

#define LOCTEXT_NAMESPACE "PartEditor"

FPartEditorViewportClient::FPartEditorViewportClient(FPreviewScene& InPreviewScene)
    : FEditorViewportClient(nullptr, &InPreviewScene) {
        bDrawAxes = true;
        SetViewMode(VMI_Unlit);
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

void FPartEditorViewportClient::SetDetailsView(TSharedPtr<IDetailsView> InDetailsView) {
    m_AssociatedDetailsView = InDetailsView;
    if (m_AssociatedDetailsView.IsValid()) {
        m_AssociatedDetailsView->OnFinishedChangingProperties().AddRaw(this, &FPartEditorViewportClient::OnInspectorPropertyChanged);
    }
}

FPartEditorViewportClient::~FPartEditorViewportClient() {
    if (m_AssociatedDetailsView.IsValid()) {
        m_AssociatedDetailsView->OnFinishedChangingProperties().RemoveAll(this);
    }
}

void FPartEditorViewportClient::OnInspectorPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent) {
    if (!m_CurrentInspectedObject.IsValid()) return;
    UObject* ActiveObject = m_CurrentInspectedObject.Get();
    SetInspectedObject(ActiveObject);
}

void FPartEditorViewportClient::Tick(float DeltaSeconds) {
    FEditorViewportClient::Tick(DeltaSeconds);
    PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
}

bool FPartEditorViewportClient::InputKey(const FInputKeyEventArgs& EventArgs) {
    /*
    Toggle drawing collision. Also ensure that we deselect our current selected collision point and set our selection type to none. Just below we'll also toggle drawing Anchors in the exact same way. You could consider it some copied code. Deselecting everything could easily be turned into it's own function.

    TODO : Turn Deselection into its own function.
    */
    if (EventArgs.Key == EKeys::C && EventArgs.Event == IE_Pressed && EventArgs.Viewport->KeyState(EKeys::LeftAlt)) {
        m_bDrawCollision = !m_bDrawCollision;
        SetSelectedIndex(INDEX_NONE);
        SetPartSelectionType(EPartSelectionType::None);
        Invalidate();
        return true;
    }
    if (EventArgs.Key == EKeys::A && EventArgs.Event == IE_Pressed && EventArgs.Viewport->KeyState(EKeys::LeftAlt)) {
        m_bDrawAnchors = !m_bDrawAnchors;
        SetSelectedIndex(INDEX_NONE);
        SetPartSelectionType(EPartSelectionType::None);
        Invalidate();
        return true;
    }

    /*
    Handle zooming inputs, whether we should zoom - or + based on scroll inputs.
    */
    if (EventArgs.Key == EKeys::MouseScrollUp && EventArgs.Event == IE_Pressed) {
        ZoomCamera(-c_ZoomAmount);
        return true;
    }
    if (EventArgs.Key == EKeys::MouseScrollDown && EventArgs.Event == IE_Pressed) {
        ZoomCamera(c_ZoomAmount);
        return true;
    }

    /*
    Handle orbit logic by locking our mouse to the viewport and toggling m_bIsOrbiting so our InputAxis function can correctly update the camera position.
    Unlock the mouse on releasing our orbit button.
    */
    if (EventArgs.Key == EKeys::RightMouseButton) {
        if (EventArgs.Event == IE_Pressed) {
            m_bIsOrbiting = true;
            Viewport->LockMouseToViewport(true);
        }
        else if (EventArgs.Event == IE_Released) {
            m_bIsOrbiting = false;
            Viewport->LockMouseToViewport(false);
        }
        return true;
    }

    if (EventArgs.Key == EKeys::LeftMouseButton) {
        if (EventArgs.Event == IE_Pressed) {
            if (Viewport && m_CurrentInspectedObject.IsValid()) {
                int32 MouseX = Viewport->GetMouseX();
                int32 MouseY = Viewport->GetMouseY();

                /*
                Firstly, we create a hit result. This hit result just checks for whether we've intersected with a UI Gizmo. In the case that we have, we ensure that this gizmo is selected.
                */

                HHitProxy* HitResult = Viewport->GetHitProxy(MouseX, MouseY);
                if (HitResult && HitResult->IsA(HWidgetAxis::StaticGetType())) {
                    HWidgetAxis* WidgetAxis = (HWidgetAxis*)HitResult;
                    SetCurrentWidgetAxis(WidgetAxis->Axis);
                    return FEditorViewportClient::InputKey(EventArgs);
                }

                /*
                If there was no selection, we can simply tell the viewport that there is no gizmo to edit.
                */

                SetCurrentWidgetAxis(EAxisList::None);

                /*
                We simulate our clicks by using a raycast with a large depth multiplier. This allows us to select objects far away.
                We don't specifically need any of our objects to be on this ray, in our later calculations we try to actually get the closest point *to* the ray
                rather than an exact click. This gives the user some lee-way and makes our selection logic slightly easier too.
                */

                FViewportCursorLocation CursorLocation = GetCursorWorldLocationFromMousePos();
                FVector RayOrigin = CursorLocation.GetOrigin();
                FVector RayDirection = CursorLocation.GetDirection();
                FVector RayEnd = RayOrigin + (RayDirection * 10000.0f);

                if (UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get())) {
                    float BestHitDistance = FLT_MAX;
                    int32 SelectedIndex = INDEX_NONE;
                    EPartSelectionType SelectedType = EPartSelectionType::None;

                    /*
                    We only check for our anchors if the user is drawing anchors, this is the same for the check for whether we should draw collision below.
                    This check works quite simply, we just check whichever point is closest to a line from the camera. If it is closer than the current BestHitDistance and is within an acceptable radius
                    (10.0f by default,) then that is the currently chosen index.

                    We need to update our m_SelectedType to our relevant EPartSelectionType
                    And our selection index, which tells our draw loop and gizmo tool which specific index from our array we want to draw and edit
                    */

                    if (m_bDrawAnchors) {
                        for (int32 i = 0; i < ActivePart->Anchors.Num(); i++) {
                            FVector PointPos = ActivePart->Anchors[i].Position;
                            // Handy dandy little FMath function for finding the closest point on a line.
                            FVector ClosestPointOnRay = FMath::ClosestPointOnSegment(PointPos, RayOrigin, RayEnd);
                            float DistanceToRay = FVector::Dist(PointPos, ClosestPointOnRay);

                            if (DistanceToRay < 10.0f &&  DistanceToRay < BestHitDistance) {
                                BestHitDistance = DistanceToRay;
                                SelectedIndex = i;
                                SelectedType = EPartSelectionType::Anchor;
                            }
                        }
                    }

                    if (m_bDrawCollision) {
                        for (int32 i = 0; i < ActivePart->CollisionPoints.Num(); i++) {
                            FVector PointPos = ActivePart->CollisionPoints[i];
                            FVector ClosestPointOnRay = FMath::ClosestPointOnSegment(PointPos, RayOrigin, RayEnd);
                            float DistanceToRay = FVector::Dist(PointPos, ClosestPointOnRay);

                            if (DistanceToRay < 10.0f && DistanceToRay < BestHitDistance) {
                                BestHitDistance = DistanceToRay;
                                SelectedIndex = i;
                                SelectedType = EPartSelectionType::CollisionPoint;
                            }
                        }
                    }

                    m_SelectedIndex = SelectedIndex;
                    m_SelectedType = SelectedType;
                    Invalidate();
                }
            }
            return true;
        }
        /*
        On release, if we have a widget - we need to deselect our widget.
        Otherwise, we can just simply return false and return true to force handle any inputs.
        */
        else if (EventArgs.Event == IE_Released) {
            if (GetCurrentWidgetAxis() != EAxisList::None) {
                SetCurrentWidgetAxis(EAxisList::None);
                Invalidate();
                return FEditorViewportClient::InputKey(EventArgs);
            }

            SetCurrentWidgetAxis(EAxisList::None);
            Invalidate();
            return true;
        }
    }

    return FEditorViewportClient::InputKey(EventArgs);
}

bool FPartEditorViewportClient::InputAxis(const FInputKeyEventArgs& EventArgs) {

    EAxisList::Type ActiveAxis = GetCurrentWidgetAxis();
    float Delta = EventArgs.AmountDepressed;

    if (ActiveAxis != EAxisList::None) {
        return FEditorViewportClient::InputAxis(EventArgs);
    }

    if (m_bIsOrbiting) {
        FRotator CurrentRotation = GetViewRotation();

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

    OnSelectedObjectChanged.Broadcast();
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
    m_CameraOrbitRadius = FMath::Clamp(m_CameraOrbitRadius + ZoomAmount, c_MinZoom, c_MaxZoom);
    UpdateCameraPositionFromOrbit();
    Invalidate();
}

/*
This is a custom drawing override function so that we can draw our own little things ontop. This allows us to draw Anchors, Collision Points and even custom boxes around that for preview.
We can extend this to draw more in the future, or we can re-use this later on in a different plugin or feature to draw some other things with similar logic.
For now, this function exists solely to give the user a way to see them edit things within our viewport client.
*/
void FPartEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) {
    FEditorViewportClient::Draw(View, PDI);

    if (!PDI) return;

    /*
    If we have an active part, we want to be able to draw our specific data. Anchors and Collision Points are not themselves objects, and therefore are not drawn by the Engine.
    We also want the end-user to be able to toggle whether they should draw Anchors and/or Collision Points. Those toggles are handled within InputKey.

    While FPrimitiveDrawInterface has many useful functions, it is also a function parameter itself. In the case of drawing Boxes:
    (DrawBox)
    (DrawWireBox)
    PDI is an input rather than the caller, despite the fact that PDI itself has DrawPoint.

    This is an odd quirk of UnrealEngine.
    */

    if (UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get())) {

        if (m_bDrawAnchors) {
            for (int32 i = 0; i < ActivePart->Anchors.Num(); i++) {
                FVector AnchorLocation = ActivePart->Anchors[i].Position;

                /*
                TODO : Change FColor to FLinearColor on these properties.
                */
                FColor RenderColor = c_UnselectedAnchorColor;
                float CorePointSize = c_AnchorSize;

                if (m_SelectedType == EPartSelectionType::Anchor) {
                    if (i == m_SelectedIndex) {
                        RenderColor = c_SelectedAnchorColor;
                        CorePointSize = c_SelectedAnchorSize;
                    }
                }

                PDI->DrawPoint(AnchorLocation, FLinearColor(RenderColor), CorePointSize, SDPG_World);
            }
        }

        if (m_bDrawCollision) {

            for (int32 i = 0; i < ActivePart->CollisionPoints.Num(); i++) {
                FVector CollisionLocation = ActivePart->CollisionPoints[i];

                FLinearColor RenderColor = c_CollisionPoint;
                float CorePointSize = c_AnchorSize;

                if (m_SelectedType == EPartSelectionType::CollisionPoint) {
                    if (i == m_SelectedIndex) {
                        RenderColor = c_SelectedCollisionPoint;
                        CorePointSize = c_AnchorSize;
                    }
                }

                PDI->DrawPoint(CollisionLocation, RenderColor, CorePointSize, SDPG_World);
            }

            /*
            We want to draw our box, which means we need to find the minimum and maximum extent of our points. We have 6 points in total since we're working with a square.
            We just need to find the minimum of all of the X, Y and Z axis through a loop. This has O(n) complexity.
            */

            FVector MinBounds = ActivePart->CollisionPoints[0];
            FVector MaxBounds = ActivePart->CollisionPoints[0];

            for (int32 i = 0; i < ActivePart->CollisionPoints.Num(); i++) {
                MinBounds.X = FMath::Min(MinBounds.X, ActivePart->CollisionPoints[i].X);
                MinBounds.Y = FMath::Min(MinBounds.Y, ActivePart->CollisionPoints[i].Y);
                MinBounds.Z = FMath::Min(MinBounds.Z, ActivePart->CollisionPoints[i].Z);

                MaxBounds.X = FMath::Max(MaxBounds.X, ActivePart->CollisionPoints[i].X);
                MaxBounds.Y = FMath::Max(MaxBounds.Y, ActivePart->CollisionPoints[i].Y);
                MaxBounds.Z = FMath::Max(MaxBounds.Z, ActivePart->CollisionPoints[i].Z);
            }

            /*
            We can construct an FBox. FBox is also usable in Blueprints, though it is instead called "Box" in blueprints, as they remove the prefix.
            This can be later saved in the UPDAPart* when editing, or created as a function call like GetBoundingBox()

            TODO : Make above function ref or save FBox in the part itself.
            */

            FBox BoundingBox(MinBounds, MaxBounds);

            /*
            We want a transparent texture for our box, while it would be great if we could simply use our c_BoundingBoxInnards colour, we have to use a material proxy, in this case a:
            FColoredMaterialRenderProxy. But, all we have to do is feed in our colour and that sets the material's albedo to our requested transparent c_BoundingBoxInnards.
            */

            FColoredMaterialRenderProxy* SafeProxy = new FColoredMaterialRenderProxy(
                GEngine->GeomMaterial->GetRenderProxy(),
                c_BoundingBoxInnards
            );

            DrawBox(PDI, FTransform(BoundingBox.GetCenter()).ToMatrixWithScale(), BoundingBox.GetExtent(), SafeProxy, SDPG_World);
            DrawWireBox(PDI, BoundingBox, FLinearColor::Red, SDPG_World, c_BoundingBoxWireThickness, 0.0f, false);
        }
    }
}

UE::Widget::EWidgetMode FPartEditorViewportClient::GetWidgetMode() const {
    if (m_SelectedType == EPartSelectionType::Anchor) {
        if (m_SelectedIndex != INDEX_NONE && m_CurrentInspectedObject.IsValid()) {
            return UE::Widget::WM_Translate;
        }
    } else if (m_SelectedType == EPartSelectionType::CollisionPoint) {
        if (m_SelectedIndex != INDEX_NONE && m_CurrentInspectedObject.IsValid()) {
            return UE::Widget::WM_Translate;
        }
    }

    return UE::Widget::WM_None;
}

/*
This updates the position of the gizmo based on where our selected part is, if there is no selected part, the gizmo will just draw at ZERO in the world, although this would mean something is broken.
In that case, revise this code.
*/
FVector FPartEditorViewportClient::GetWidgetLocation() const {
    switch(m_SelectedType) {
        case EPartSelectionType::Anchor: {
            if (UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get())) {
                if (ActivePart->Anchors.IsValidIndex(m_SelectedIndex)) {
                    return ActivePart->Anchors[m_SelectedIndex].Position;
                }
            }
            break;
        }
        case EPartSelectionType::CollisionPoint: {
            if (UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get())) {
                if (ActivePart->CollisionPoints.IsValidIndex(m_SelectedIndex)) {
                    return ActivePart->CollisionPoints[m_SelectedIndex];
                }
            }
            break;
        }
        default:
            break;
    }

    return FVector::ZeroVector;
}

/*
Here, we move our gizmos based on the delta-input of our widget. This is handled separate to InputAxis and InputKey, although the latter is what allows this function to work.
We don't need to snap positions for our anchors, these can be freely placed and fine-tuned through the details panel on the right. But for our collision points,
we wish to snap to a specific value.
*/
bool FPartEditorViewportClient::InputWidgetDelta(FViewport* InViewport, EAxisList::Type InAxis, FVector& Drag, FRotator& Rot, FVector& Scale) {
    if (InAxis == EAxisList::None || Drag.IsZero()) {
        return false;
    }

    UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get());
    if (!ActivePart) return false;

    if (!ActivePart->HasAnyFlags(RF_Transactional)) {
        ActivePart->SetFlags(RF_Transactional);
    }

    FScopedTransaction Transaction(LOCTEXT("Move Anchor or Collision Point", "Edit internal part properties."));

    bool bHandled = false;

    switch(m_SelectedType) {
        case EPartSelectionType::Anchor: {
            if (ActivePart->Anchors.IsValidIndex(m_SelectedIndex)) {
                ActivePart->Modify();
                ActivePart->Anchors[m_SelectedIndex].Position += Drag;
                bHandled = true;
            }
            break;
        }
        case EPartSelectionType::CollisionPoint: {
            if (ActivePart->CollisionPoints.IsValidIndex(m_SelectedIndex)) {
                ActivePart->Modify();
                FVector CurrentPos = ActivePart->CollisionPoints[m_SelectedIndex];
                FVector NewPos = CurrentPos + Drag;

                FSnappingUtils::SnapPointToGrid(NewPos, FVector(c_GridSize));
                Drag = NewPos - CurrentPos;

                ActivePart->CollisionPoints[m_SelectedIndex] = NewPos;
                bHandled = true;
            }
            break;
        }
        default:
            break;
    }
    if (bHandled) {
        ActivePart->MarkPackageDirty();

        if (m_AssociatedDetailsView.IsValid()) {
            m_AssociatedDetailsView->SetObject(ActivePart, true);
        }
        Invalidate();
        return bHandled;
    }

    return false;
}

void FPartEditorViewportClient::SaveActiveAsset() {
    if (!m_CurrentInspectedObject.IsValid()) return;

    UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get());
    if (!ActivePart) return;

    UPackage* CurrentPackage = ActivePart->GetPackage();
    FString CurrentName = ActivePart->PartName;
    CurrentName = CurrentName.Replace(TEXT(" "), TEXT("_"));

    if (CurrentName.IsEmpty() || CurrentName.Len() == 0) {
        CurrentName = TEXT("UNNAMED");
    }

    ActivePart->PartName = CurrentName;

    if (CurrentPackage != GetTransientPackage()) {
        TArray<UPackage*> PackagesToSave;
        PackagesToSave.Add(CurrentPackage);

        CurrentPackage->MarkPackageDirty();
        ActivePart->MarkPackageDirty();

        UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);
    } else if (CurrentName.Equals(TEXT("UNNAMED"), ESearchCase::IgnoreCase)) {
        FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

        FSaveAssetDialogConfig SaveDialogConfig;
        SaveDialogConfig.DialogTitleOverride = INVTEXT("Save new Asset");
        SaveDialogConfig.DefaultPath = TEXT("/RobotEditor/Data");
        SaveDialogConfig.DefaultAssetName = TEXT("Unnamed");
        SaveDialogConfig.AssetClassNames.Add(UPDAPart::StaticClass()->GetClassPathName());

        FOnObjectPathChosenForSave OnSavePressed = FOnObjectPathChosenForSave::CreateLambda([this, ActivePart](const FString& ChosenPackagePath) {
            FString TargetFolderPackagePath = FPaths::GetPath(ChosenPackagePath);
            FString CleanAssetName = FPaths::GetBaseFilename(ChosenPackagePath);

            ActivePart->Rename(*CleanAssetName, ActivePart->GetOuter());

            FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
            UPDAPart* NewSavedAsset = Cast<UPDAPart>(AssetToolsModule.Get().CreateAsset(
                CleanAssetName,
                TargetFolderPackagePath,
                UPDAPart::StaticClass(),
                nullptr
            ));

            if (NewSavedAsset) {
                NewSavedAsset->Object = ActivePart->Object;
                NewSavedAsset->Anchors = ActivePart->Anchors;
                NewSavedAsset->PartName = CleanAssetName;

                NewSavedAsset->MarkPackageDirty();
                NewSavedAsset->GetPackage()->MarkPackageDirty();

                TArray<UPackage*> PackagesToSave;
                PackagesToSave.Add(NewSavedAsset->GetPackage());

                UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);

                SetInspectedObject(NewSavedAsset);
            }
        });

        FOnAssetDialogCancelled OnCancelPressed = FOnAssetDialogCancelled::CreateLambda([]() {
            UE_LOG(LogTemp, Warning, TEXT("Save action cancelled."));
        });

        ContentBrowserModule.Get().CreateSaveAssetDialog(SaveDialogConfig, OnSavePressed, OnCancelPressed);
    } else if (CurrentPackage == GetTransientPackage()) {
        FString TargetFolderPackagePath = TEXT("/RobotEditor/Data");

        FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
        FString UniquePackagePath;
        FString UniqueAssetName;

        AssetToolsModule.Get().CreateUniqueAssetName(TargetFolderPackagePath + TEXT("/") + CurrentName, TEXT(""), UniquePackagePath, UniqueAssetName);

        UPDAPart* NewSavedAsset = Cast<UPDAPart>(AssetToolsModule.Get().CreateAsset(
            UniqueAssetName,
            TargetFolderPackagePath,
            UPDAPart::StaticClass(),
            nullptr
        ));

        if (NewSavedAsset) {
            NewSavedAsset->Object = ActivePart->Object;
            NewSavedAsset->Anchors = ActivePart->Anchors;
            NewSavedAsset->PartName = ActivePart->PartName;

            NewSavedAsset->MarkPackageDirty();
            NewSavedAsset->GetPackage()->MarkPackageDirty();

            TArray<UPackage*> PackagesToSave;
            PackagesToSave.Add(NewSavedAsset->GetPackage());

            UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);

            SetInspectedObject(NewSavedAsset);
        }
    }

    if (SPartContentBrowser::m_Instance.IsValid()) {
        SPartContentBrowser::m_Instance.Pin()->ScanPluginDirectory();
    }
}

#undef LOCTEXT_NAMESPACE
