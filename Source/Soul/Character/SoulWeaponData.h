#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "../Common/WeaponTypes.h"
#include "SoulWeaponData.generated.h"

UCLASS()
class SOUL_API USoulWeaponData : public UDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    EWeaponType WeaponType = EWeaponType::Empty;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Visual")
    TObjectPtr<UStaticMesh> StaticMesh = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attach")
    FName AttachSocketName = TEXT("HandGrip_R");

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Attach")
    FTransform AttachOffset = FTransform::Identity;
};