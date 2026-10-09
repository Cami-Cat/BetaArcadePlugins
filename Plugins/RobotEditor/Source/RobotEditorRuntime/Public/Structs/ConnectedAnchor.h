#pragma once

#include "CoreMinimal.h"
#include "Structs/Anchor.h"

USTRUCT(BlueprintType)
struct FConnectedAnchor {
GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FName ConnectedAnchor;
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FGuid ConnectedID;
};
