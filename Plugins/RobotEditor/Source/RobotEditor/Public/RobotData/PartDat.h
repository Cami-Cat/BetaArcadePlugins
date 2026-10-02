#pragma once

#include "CoreMinimal.h"
#include "DataAsset.h"
#include "PartDat.generated.h"

class UNiagaraSystem;

UCLASS(BlueprintType)
class UPDAPart : public UPublicDataAsset {
GENERATED_BODY()
private:
    float m_Health;

    SoftReference<UNiagraSystem> m_NS_Death;
    SoftReference<UNiagraSystem> m_SQ_Death;
    SoftReference<UObject> m_Object;

};
