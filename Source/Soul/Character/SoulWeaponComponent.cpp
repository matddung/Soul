#include "SoulWeaponComponent.h"
#include "SoulWeaponData.h"

#include "GameFramework/Character.h"

USoulWeaponComponent::USoulWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void USoulWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	
    AActor* Owner = GetOwner();
    if (!Owner) return;

    EquippedStaticMeshComp = NewObject<UStaticMeshComponent>(Owner, TEXT("EquippedWeapon_StaticMesh"));
    if (EquippedStaticMeshComp)
    {
        EquippedStaticMeshComp->RegisterComponent();
        EquippedStaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        EquippedStaticMeshComp->SetGenerateOverlapEvents(false);
        EquippedStaticMeshComp->SetHiddenInGame(true);
        EquippedStaticMeshComp->SetVisibility(false, true);
    }
}

void USoulWeaponComponent::GiveWeapon(TObjectPtr<USoulWeaponData> WeaponData)
{
    if (!WeaponData) return;

    OwnedWeapons.FindOrAdd(WeaponData->WeaponType) = WeaponData;
}

bool USoulWeaponComponent::HasWeapon(EWeaponType Type) const
{
    return OwnedWeapons.Contains(Type);
}

bool USoulWeaponComponent::EquipWeapon(EWeaponType Type)
{
    if (Type == EWeaponType::Empty)
    {
        EquippedType = EWeaponType::Empty;
        ClearVisual();
        return true;
    }

    TObjectPtr<USoulWeaponData> const* Found = OwnedWeapons.Find(Type);
    if (!Found || !(*Found))
    {
        return false;
    }

    EquippedType = Type;
    ApplyVisual(*Found);
    return true;
}

void USoulWeaponComponent::ApplyVisual(USoulWeaponData* Data)
{
    if (!Data || !EquippedStaticMeshComp) return;

    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar || !OwnerChar->GetMesh()) return;

    EquippedStaticMeshComp->SetStaticMesh(Data->StaticMesh);

    EquippedStaticMeshComp->AttachToComponent(
        OwnerChar->GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        Data->AttachSocketName
    );

    EquippedStaticMeshComp->SetRelativeTransform(Data->AttachOffset);

    EquippedStaticMeshComp->SetHiddenInGame(false);
    EquippedStaticMeshComp->SetVisibility(true, true);
}

void USoulWeaponComponent::ClearVisual()
{
    if (!EquippedStaticMeshComp) return;

    EquippedStaticMeshComp->SetHiddenInGame(true);
    EquippedStaticMeshComp->SetVisibility(false, true);
    EquippedStaticMeshComp->SetStaticMesh(nullptr);
}