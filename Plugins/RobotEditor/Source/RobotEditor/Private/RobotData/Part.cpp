#include "RobotData/Part.h"


FTooltip& APart::GetTooltip() {
    return PartData->GetTooltipData();
}

void APart::GetBoundingBox() {
    return;
}

TArray<FAnchor> APart::GetAnchors() {
    return m_Anchors;
}

const FAnchor& APart::GetAnchorAtIndex(uint8 Index) {
    if (Index >= m_Anchors.Num()) {
        return m_Anchors.Last();
    }
    return m_Anchors[Index];
}

bool APart::CanAnchorConnect(const FAnchor& Anchor) {
    return Anchor.bIsConnected == false;
}

bool APart::ConnectAnchor(FAnchor& Anchor, FAnchor& OtherAnchor) {
    if (Anchor.bIsConnected == true && OtherAnchor.bIsConnected == true) return false;
    Anchor.bIsConnected = true;
    OtherAnchor.bIsConnected = true;
    return (Anchor.bIsConnected == true && OtherAnchor.bIsConnected == true);
}

bool APart::DisconnectAnchor(FAnchor& Anchor) {
    Anchor.bIsConnected = false;
    return Anchor.bIsConnected == false;
}

const FTransform& APart::GetAnchorTransform(const FAnchor& Anchor) {
    return Anchor.AnchorTransform;
}

const FAnchor& APart::GetAnchorAtTransform(const FTransform& Transform) {
    static FAnchor DefaultFallback = FAnchor();
    for (const FAnchor& Anchor : m_Anchors) {
        if (Anchor.AnchorTransform.GetLocation() == Transform.GetLocation()) {
            return Anchor;
        }
    }
    return DefaultFallback;
}

const FAnchor& APart::GetClosestAnchorToTransform(const FTransform& Transform) {
    float CurrentDistance = 0.0;
    uint8 FoundIndex = 0;
    uint8 CurrentIndex = 0;
    for (const FAnchor& Anchor : m_Anchors) {
        if (Anchor.AnchorTransform.GetLocation() == Transform.GetLocation()) {
            return Anchor;
        }
        else {
            float Distance = (Anchor.AnchorTransform.GetLocation() - Transform.GetLocation()).Length();
            if (Distance < CurrentDistance) {
                CurrentDistance = Distance;
                FoundIndex = CurrentIndex;
                CurrentIndex++;
            }
        }
    }
    return m_Anchors[FoundIndex];
}
