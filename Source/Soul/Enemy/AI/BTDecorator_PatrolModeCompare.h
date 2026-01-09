#pragma once

#include "CoreMinimal.h"
#include "BTDecorator_EnumCompareBase.h"
#include "EnemyAIConfig.h"
#include "BTDecorator_PatrolModeCompare.generated.h"

UCLASS()
class SOUL_API UBTDecorator_PatrolModeCompare : public UBTDecorator_EnumCompareBase
{
	GENERATED_BODY()
	
public:
    UBTDecorator_PatrolModeCompare();

    UPROPERTY(EditAnywhere, Category = "Condition")
    EPatrolMode ExpectedMode = EPatrolMode::RandomArea;

protected:
    virtual UEnum* GetAllowedEnum() const override
    {
        return StaticEnum<EPatrolMode>();
    }

    virtual uint8 GetExpectedValue() const override
    {
        return static_cast<uint8>(ExpectedMode);
    }
};