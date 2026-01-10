#include "SoulGameModeBase.h"

ASoulGameModeBase::ASoulGameModeBase()
{
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerFinder(
		TEXT("/Game/Blueprints/BP_SoulPlayerController"));
	if (PlayerControllerFinder.Succeeded())
	{
		PlayerControllerClass = PlayerControllerFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<APawn> Player(TEXT("/Game/Blueprints/Character/BP_SoulCharacter"));
	if (Player.Succeeded())
	{
		DefaultPawnClass = Player.Class;
	}
}