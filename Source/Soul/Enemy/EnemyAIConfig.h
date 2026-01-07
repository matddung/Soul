#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyAIConfig.generated.h"

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
    Roamer UMETA(DisplayName = "Roamer"),
    Ambush UMETA(DisplayName = "Ambush"),
    Boss UMETA(DisplayName = "Boss")
};

UENUM(BlueprintType)
enum class EPatrolMode : uint8
{
    FixedRoute UMETA(DisplayName = "FixedRoute"),
    RandomArea UMETA(DisplayName = "RandomArea")
};

UCLASS(BlueprintType)
class SOUL_API UEnemyAIConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Type")
    EEnemyType EnemyType = EEnemyType::Roamer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception", meta = (ClampMin = "0.0"))
    float SightRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception", meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float SightFOV = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Perception", meta = (ClampMin = "0.0"))
    float LoseSightRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Combat", meta = (ClampMin = "0.0"))
    float AggroRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Combat", meta = (ClampMin = "0.0"))
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Patrol")
    EPatrolMode PatrolMode = EPatrolMode::RandomArea;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI|Patrol", meta = (ClampMin = "0.0"))
    float PatrolRadius = 1000.0f;
};