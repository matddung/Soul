#pragma once

#include "CoreMinimal.h"
#include "BTDecorator_EnumCompareBase.h"
#include "EnemyAIConfig.h"
#include "BTDecorator_EnemyTypeCompare.generated.h"

UCLASS()
class SOUL_API UBTDecorator_EnemyTypeCompare : public UBTDecorator_EnumCompareBase
{
	GENERATED_BODY()
	
public:
    UBTDecorator_EnemyTypeCompare();

    UPROPERTY(EditAnywhere, Category = "Condition")
    EEnemyType ExpectedType = EEnemyType::Roamer;

protected:
    virtual UEnum* GetAllowedEnum() const override
    {
        return StaticEnum<EEnemyType>();
    }

    virtual uint8 GetExpectedValue() const override
    {
        return static_cast<uint8>(ExpectedType);
    }
};