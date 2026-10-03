#pragma once

#include "CoreMinimal.h"

class SPartEditorTab : public SDockTab {
public:
    static TSharedRef<class SDockTab> MakeTab(const FSpawnTabArgs& SpawnTabArgs);
};
