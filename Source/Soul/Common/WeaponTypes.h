#pragma once

#include "WeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Empty UMETA(DisplayName = "Empty"),
	Sword UMETA(DisplayName = "Sword"),
	Gun   UMETA(DisplayName = "Gun"),
};