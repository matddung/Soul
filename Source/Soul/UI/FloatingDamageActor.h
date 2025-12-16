#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloatingDamageActor.generated.h"

UCLASS()
class SOUL_API AFloatingDamageActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AFloatingDamageActor();

    void SetDamage(float Damage);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY(VisibleAnywhere)
    class UWidgetComponent* WidgetComponent;

    float LifeTime = 1.2f;

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    float RiseSpeed = 50.0f;

private:
    float ElapsedTime = 0.0f;
};