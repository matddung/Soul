#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SoulCharacterWeapon.generated.h"

UCLASS()
class SOUL_API ASoulCharacterWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ASoulCharacterWeapon();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};