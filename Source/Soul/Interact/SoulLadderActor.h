#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SoulInteractableInterface.h"
#include "SoulLadderActor.generated.h"

class UBoxComponent;
class UArrowComponent;

UENUM(BlueprintType)
enum class ELadderUseSide : uint8
{
	None	UMETA(DisplayName = "None"),
	Bottom	UMETA(DisplayName = "Bottom"),
	Top		UMETA(DisplayName = "Top")
};

UCLASS()
class SOUL_API ASoulLadderActor : public AActor, public ISoulInteractableInterface
{
	GENERATED_BODY()
	
public:	
	ASoulLadderActor();

	FORCEINLINE ELadderUseSide GetLastUseSide() const { return LastUseSide; }

	virtual void Interact_Implementation(ASoulCharacter* Interactor) override;
	virtual bool CanInteract_Implementation(ASoulCharacter* Interactor) const override;
	virtual FText GetInteractText_Implementation() const override;

	void GetClimbZRange(float& OutMinZ, float& OutMaxZ) const;
	void GetSnapTransform(const ASoulCharacter* Character, FVector& OutLoc, FRotator& OutRot) const;

	FVector GetTopExitLocation() const;
	FVector GetBottomExitLocation() const;

	FVector GetTopMountStartLocation() const;
	FRotator GetTopMountStartRotation() const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBottomBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTopBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBottomEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnTopEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	FVector GetForward() const;
	

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> LadderMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> BottomInteractBox;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> TopInteractBox;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UArrowComponent> LadderForward;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> BottomPoint;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> TopPoint;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> TopMountStartPoint;

	UPROPERTY(EditAnywhere, Category = "Ladder")
	float SnapDistanceFromLadder = -40;

	UPROPERTY(VisibleInstanceOnly, Category = "Ladder")
	ELadderUseSide LastUseSide = ELadderUseSide::None;

	UPROPERTY(EditAnywhere, Category = "Ladder")
	float ExitForwardDistance = 60;

	UPROPERTY(EditAnywhere, Category = "Ladder")
	float BottomExitForwardDistance = 40;
};