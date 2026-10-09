#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Structs/RobotData.h"
#include "RobotSaveGame.generated.h"

UCLASS()
class ROBOTEDITORRUNTIME_API URobotSaveGame : public USaveGame {
GENERATED_BODY()
public:

    UPROPERTY(BlueprintReadWrite, Category = "Save Data")
    FRobotDataSnapshot SavedSnapshot;

public:

    UFUNCTION(BlueprintCallable, Category = "Save")
    static void SaveLocalData(const FRobotDataSnapshot& Data, FString SlotName);
};
