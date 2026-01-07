#pragma once

#include "CoreMinimal.h"
#include "BTDecorator_EnumTypeEquals.h"
#include "EnemyAIConfig.h"
#include "BTDecorator_EnemyTypeEquals.generated.h"

UCLASS()
class SOUL_API UBTDecorator_EnemyTypeEquals : public UBTDecorator_EnumTypeEquals
{
	GENERATED_BODY()
	
public:
    UBTDecorator_EnemyTypeEquals();

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