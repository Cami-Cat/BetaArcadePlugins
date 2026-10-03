#pragma once

#include "CoreMinimal.h"
#include "Anchor.generated.h"

USTRUCT(BlueprintType)
struct FAnchor {
GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Data")
    FTransform AnchorTransform;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Data")
    bool bIsConnected;

    FAnchor()
        : AnchorTransform(),
          bIsConnected(false)
    {}
};
