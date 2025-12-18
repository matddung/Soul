#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SoulInteractableInterface.generated.h"

class ASoulCharacter;

UINTERFACE(MinimalAPI)
class USoulInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SOUL_API ISoulInteractableInterface
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
	void Interact(ASoulCharacter* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interact")
	bool CanInteract(ASoulCharacter* Interactor) const;
};