#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SoulCharacter.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS()
class SOUL_API ASoulCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASoulCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

protected:
	UPROPERTY(EditAnywhere,Category="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* DefaultMappingContext;
};
