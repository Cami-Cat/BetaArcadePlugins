#include "Data/PartDat.h"

UPDAPart::UPDAPart()
    : Super() {

    int Sides = 6;

    TArray<FVector> InitialPoints = {
        FVector(100, 0, 0),
        FVector(-100, 0, 0),
        FVector(0, 100, 0),
        FVector(0, -100, 0),
        FVector(0, 0, 100),
        FVector(0, 0, -100)
    };

    if (CollisionPoints.IsEmpty()) {
        CollisionPoints = InitialPoints;
    }
}

float UPDAPart::GetHealth() {
    return Health;
}

float UPDAPart::SetHealth(float NewHealth) {
    Health = NewHealth;
    return Health;
}

UNiagaraSystem* UPDAPart::GetDeathParticle() {
    return NS_Death;
}

USoundBase* UPDAPart::GetDeathSound() {
    return SQ_Death;
}

bool UPDAPart::IsObjectMesh(UObject* InObject) {
    UStaticMesh* Mesh = Cast<UStaticMesh>(InObject);
    return IsValid(Mesh);
}

FTooltip& UPDAPart::GetTooltipData() {
    return Tooltip;
}

FWeapon& UPDAPart::GetWeaponData() {
    return WeaponTooltip;
}
