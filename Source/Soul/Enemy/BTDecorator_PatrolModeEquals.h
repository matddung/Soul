#pragma once

#include "CoreMinimal.h"
#include "BTDecorator_EnumTypeEquals.h"
#include "EnemyAIConfig.h"
#include "BTDecorator_PatrolModeEquals.generated.h"

UCLASS()
class SOUL_API UBTDecorator_PatrolModeEquals : public UBTDecorator_EnumTypeEquals
{
    GENERATED_BODY()

public:
    UBTDecorator_PatrolModeEquals();

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