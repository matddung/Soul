#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "SoulPlayerController.generated.h"

class UUserWidget;
class UCrosshairWidget;
class UInteractPromptWidget;
class UInputAction;
class UInputMappingContext;
class ASoulCharacter;

UCLASS()
class SOUL_API ASoulPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void ShowCrosshair(bool bShow);

	void OnCrosshairShot();
	void OnCrosshairReset();

	void ShowInteractPrompt(bool bShow, const FText& Text);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCrosshairWidget> CrosshairWidgetClass;

	UPROPERTY()
	TObjectPtr<UCrosshairWidget> CrosshairWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UInteractPromptWidget> InteractPromptClass;

	UPROPERTY()
	TObjectPtr<UInteractPromptWidget> InteractPromptWidget;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> SprintAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> AttackAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> SwapSwordAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> SwapGunAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> SwapEmptyAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> GunAimAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> SwordDodgeAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputAction> InteractAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

private:
    void AddDefaultMappingContext();
    void RemoveDefaultMappingContext();
    void BindInputActions();
    ASoulCharacter* GetSoulCharacter() const;

    void HandleMove(const FInputActionValue& Value);
    void HandleLook(const FInputActionValue& Value);
    void HandleSprintStart(const FInputActionValue& Value);
    void HandleSprintStop(const FInputActionValue& Value);
    void HandleAttack(const FInputActionValue& Value);
    void HandleSwapSword(const FInputActionValue& Value);
    void HandleSwapGun(const FInputActionValue& Value);
    void HandleSwapEmpty(const FInputActionValue& Value);
    void HandleGunAimStart(const FInputActionValue& Value);
    void HandleGunAimStop(const FInputActionValue& Value);
    void HandleDodge(const FInputActionValue& Value);
    void HandleInteract(const FInputActionValue& Value);
    void HandleMoveCompleted(const FInputActionValue& Value);

    bool bMappingContextAdded = false;
};