#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "PreviewScene.h"

class FRobotEditorViewportClient : public FEditorViewportClient {
public:
    FRobotEditorViewportClient(FPreviewScene& InPreviewScene);
    virtual ~FRobotEditorViewportClient() override {}

    virtual void Tick(float DeltaSeconds) override;
};
