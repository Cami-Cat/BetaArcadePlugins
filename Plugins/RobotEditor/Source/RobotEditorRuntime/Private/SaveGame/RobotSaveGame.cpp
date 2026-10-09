#include "SaveGame/RobotSaveGame.h"
#include "Kismet/GameplayStatics.h"

void URobotSaveGame::SaveLocalData(const FRobotDataSnapshot& Data, FString SlotName) {
    URobotSaveGame* SaveGameInstance = Cast<URobotSaveGame>(UGameplayStatics::CreateSaveGameObject(URobotSaveGame::StaticClass()));
    if (!SaveGameInstance) return;

    SaveGameInstance->SavedSnapshot = Data;
    UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);
}
