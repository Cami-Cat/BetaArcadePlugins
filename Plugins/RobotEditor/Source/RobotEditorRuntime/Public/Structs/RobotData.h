#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Structs/ConnectedAnchor.h"
#include "Data/PartDat.h"
#include "RobotData.generated.h"

USTRUCT(BlueprintType)
struct FRobotDataSnapshot {
GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "Save Data")
    TSoftObjectPtr<UPDAPart> Part;
    UPROPERTY(BlueprintReadWrite, Category = "Save Data")
    TMap<FName, FConnectedAnchor> ConnectedTo;
    UPROPERTY(BlueprintReadWrite, Category = "Save Data")
    FTransform RelativeTransform;
    UPROPERTY(BlueprintReadWrite, Category = "Save Data")
    FGuid RefID;
    UPROPERTY(BlueprintReadWrite, Category = "Save Data")
    float CurrentHealth;
};

UCLASS(BlueprintType, Blueprintable)
class ROBOTEDITORRUNTIME_API URobotData : public UObject {
    GENERATED_BODY()

public:
    FRobotDataSnapshot Data;
};
