#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RangedArrowProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class SOUL_API ARangedArrowProjectile : public AActor
{
	GENERATED_BODY()
	
public:
    ARangedArrowProjectile();

    void InitProjectile(AController* InInstigatorController, AActor* InDamageCauser, float InDamage);

    UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

protected:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USphereComponent> Collision;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    float Damage = 20.f;

    UPROPERTY()
    TObjectPtr<AController> InstigatorController;

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};