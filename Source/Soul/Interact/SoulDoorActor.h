#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SoulInteractableInterface.h"
#include "SoulDoorActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class ASoulCharacter;

UCLASS()
class SOUL_API ASoulDoorActor : public AActor, public ISoulInteractableInterface
{
	GENERATED_BODY()

public:
	ASoulDoorActor();

	virtual void Interact_Implementation(ASoulCharacter* Interactor) override;
	virtual bool CanInteract_Implementation(ASoulCharacter* Interactor) const override;
	virtual FText GetInteractText_Implementation() const override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnInteractBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnPortalBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFrmSweep, const FHitResult& SweepResult);

	void StartOpen();

	UFUNCTION()
	void OnInteractorAutoFaceEnd();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> FrameMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> InteractBox;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> PortalTrigger;

	UPROPERTY(EditAnywhere, Category = "Door")
	float OpenDuration = 2;

	UPROPERTY(EditAnywhere, Category = "Door")
	float OpenYawDelta = 90;

	UPROPERTY(VisibleInstanceOnly, Category = "Door")
	bool bOpened = false;

	UPROPERTY(VisibleInstanceOnly, Category = "Door")
	bool bOpening = false;

	float OpenElapsed = 0;
	FRotator DoorStartRot;
	FRotator DoorTargetRot;

	UPROPERTY(EditAnywhere, Category = "Door")
	FName TargetLevelName = FName("TestMap");

	UPROPERTY()
	TWeakObjectPtr<ASoulCharacter> CachedInteractor;
};