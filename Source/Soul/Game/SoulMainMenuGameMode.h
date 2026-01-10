#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SoulMainMenuGameMode.generated.h"

class UMainMenuWidget;

UCLASS()
class SOUL_API ASoulMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ASoulMainMenuGameMode();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UMainMenuWidget> MainMenuWidget;
};