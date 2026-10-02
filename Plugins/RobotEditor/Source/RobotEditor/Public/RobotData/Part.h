#pragma once

#include "CoreMinimal.h"
#include "RobotData/PartDat.h"
#include "Meta/Structs/Anchor.h"
#include "Part.Generated.h"

UCLASS(BlueprintType)
class APart : public AActor {
GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
    TObjectPtr<UPDAPart> PartData;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Data")
    bool bIsCore;

private:
    UPROPERTY()
    TArray<FAnchor> m_Anchors;

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI")
    FTooltip& GetTooltip();
    void GetBoundingBox();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Anchors")
    TArray<FAnchor> GetAnchors();
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Anchors")
    const FAnchor& GetAnchorAtIndex(uint8 Index);
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Anchors")
    bool CanAnchorConnect(const FAnchor& Anchor);
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Anchors")
    bool ConnectAnchor(UPARAM(ref) FAnchor& Anchor, UPARAM(ref) FAnchor& OtherAnchor);
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Anchors")
    bool DisconnectAnchor(UPARAM(ref) FAnchor& Anchor);
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Anchors")
    const FTransform& GetAnchorTransform(const FAnchor& Anchor);
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Anchors")
    const FAnchor& GetAnchorAtTransform(const FTransform& Transform);
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Anchors")
    const FAnchor& GetClosestAnchorToTransform(const FTransform& Transform);
};
