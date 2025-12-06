#include "SoulGameModeBase.h"

ASoulGameModeBase::ASoulGameModeBase()
{
	static ConstructorHelpers::FClassFinder<APawn> Player(TEXT("/Game/Blueprints/BP_SoulCharacter"));
	if (Player.Succeeded())
	{
		DefaultPawnClass = Player.Class;
	}
}