#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Common/WeaponTypes.h"
#include "SoulWeaponComponent.generated.h"

class USoulWeaponData;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOUL_API USoulWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USoulWeaponComponent();

	void GiveWeapon(TObjectPtr<USoulWeaponData> WeaponData);
	bool HasWeapon(EWeaponType Type) const;
	bool EquipWeapon(EWeaponType Type);

	FORCEINLINE EWeaponType GetEquippedType() const { return EquippedType; }

protected:
    virtual void BeginPlay() override;

    void ApplyVisual(USoulWeaponData* Data);
    void ClearVisual();

protected:
    UPROPERTY()
    TMap<EWeaponType, TObjectPtr<USoulWeaponData>> OwnedWeapons;

    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> EquippedStaticMeshComp;

    UPROPERTY()
    EWeaponType EquippedType = EWeaponType::Empty;

};