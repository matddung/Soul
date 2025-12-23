#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulCharacterStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDead);

UENUM(BlueprintType)
enum class ECharacterStatType : uint8
{
	STR UMETA(DisplayName = "STR"),
	DEX UMETA(DisplayName = "DEX"),
	VIT UMETA(DisplayName = "VIT"),
	END UMETA(DisplayName = "END"),
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOUL_API USoulCharacterStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USoulCharacterStatComponent();

	UFUNCTION()
	int32 GetLevel() const;

	UFUNCTION()
	int32 GetCurrentInvestCost() const;

	UFUNCTION()
	void RecalculateDerivedStats(bool bKeepCurrentRatio = true);

	UFUNCTION()
	bool TryInvestStat(ECharacterStatType StatToIncrease);

	UFUNCTION()
	void AddSouls(int32 Amount);

	UFUNCTION()
	bool ApplyDamage(float DamageAmount);

	UFUNCTION()
	bool IsDead() const;

	UFUNCTION()
	void ResetCurrentToMax();

protected:
	virtual void BeginPlay() override;

	int32 GetStatRef(ECharacterStatType StatType) const;
	void AddToStat(ECharacterStatType StatType, int32 Delta);

public:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 STR = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 DEX = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 VIT = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 END = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MaxHP = 300;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MaxStamina = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float HP = 300;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float Stamina = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float HP_Base = 300;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float HP_PerVIT = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float Stamina_Base = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float Stamina_PerEND = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 Souls = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 BaseInvestCost = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float CostMultiplier = 1.1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 InvestCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float StaminaRegenRate = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float StaminaRegenDelay = 0.8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	bool bCanRegenStamina = true;

	UPROPERTY(BlueprintAssignable)
	FOnDead OnDead;
};