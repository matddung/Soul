#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SoulInteractableInterface.h"
#include "SoulBoxActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UParticleSystem;
class UParticleSystemComponent;

UCLASS()
class SOUL_API ASoulBoxActor : public AActor, public ISoulInteractableInterface
{
	GENERATED_BODY()
	
public:	
	ASoulBoxActor();

	virtual void Interact_Implementation(ASoulCharacter* Interactor) override;
	virtual bool CanInteract_Implementation(ASoulCharacter* Interactor) const override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnInteractBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void FinishOpen();

	void FinishDisappear();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ChestMesh;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* InteractBox;

	UPROPERTY(VisibleInstanceOnly, Category = "Box")
	bool bOpened = false;

	UPROPERTY(EditAnywhere, Category = "Box")
	float OpenDelaySeconds = 0.6f;

private:
	FTimerHandle OpenTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Box")
	TObjectPtr<UParticleSystem> OpenParticle;

	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> SpawnedParticle;

	UPROPERTY(EditAnywhere, Category = "Box")
	float DisappearDelay = 2.0f;

	FTimerHandle DisappearTimerHandle;
};