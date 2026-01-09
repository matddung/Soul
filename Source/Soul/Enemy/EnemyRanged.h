#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "EnemyRanged.generated.h"

class ARangedArrowProjectile;

UCLASS()
class SOUL_API AEnemyRanged : public AEnemyBase
{
	GENERATED_BODY()
	
public:
	AEnemyRanged();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void SpawnProjectileAtTarget(AActor* TargetActor);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	TSubclassOf<ARangedArrowProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	FName MuzzleSocketName = TEXT("arrow_anchor");
};