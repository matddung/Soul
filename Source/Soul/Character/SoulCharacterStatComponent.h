#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Game/CharacterStatSaveData.h"
#include "../Common/WeaponTypes.h"
#include "SoulCharacterStatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDead);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatChanged);

USTRUCT(BlueprintType)
struct FCharacterDerivedStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float MaxHP = 0;

	UPROPERTY(BlueprintReadOnly)
	float MaxStamina = 0;

	UPROPERTY(BlueprintReadOnly)
	float StaminaRegenRate = 0;

	UPROPERTY(BlueprintReadOnly)
	float SwordDamage = 0;

	UPROPERTY(BlueprintReadOnly)
	float GunDamage = 0;
};

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
	bool TryRefundStat(ECharacterStatType StatToDecrease);

	UFUNCTION()
	void AddSouls(int32 Amount);

	UFUNCTION()
	bool ApplyDamage(float DamageAmount);

	UFUNCTION()
	bool IsDead() const;

	UFUNCTION()
	void ResetCurrentToMax();

	bool EnhanceWeapon(EWeaponType Type);

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetSwordEnhancementLevel() const { return SwordEnhancementLevel; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE int32 GetGunEnhancementLevel() const { return GunEnhancementLevel; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetSwordDamage() const { return SwordDamage; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetGunDamage() const { return GunDamage; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetHP() const { return HP; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetMaxHP() const { return MaxHP; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetStamina() const { return Stamina; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure)
	FORCEINLINE float GetStaminaRegen() const { return StaminaRegenRate; }

	UFUNCTION(BlueprintPure)
	FCharacterDerivedStats GetCurrentDerivedStats() const;

	UFUNCTION(BlueprintPure)
	FCharacterDerivedStats GetPreviewDerivedStats(ECharacterStatType StatToIncrease) const;

	UFUNCTION(BlueprintCallable)
	bool ConsumeStamina(float Amount);

	UFUNCTION(BlueprintCallable)
	void RegenerateStamina(float DeltaSeconds);

	UFUNCTION(BlueprintPure)
	bool HasStamina(float Amount) const;

	FORCEINLINE static FString GetSaveSlotName() { return UCharacterStatSaveData::GetSlotName(); }

	UFUNCTION()
	bool RestoreHP(float Amount);

	void SaveStatData() const;

protected:
	virtual void BeginPlay() override;

	int32 GetStatRef(ECharacterStatType StatType) const;
	void AddToStat(ECharacterStatType StatType, int32 Delta);
	FCharacterDerivedStats GetDerivedStatsInternal(int32 InSTR, int32 InDEX, int32 InVIT, int32 InEND) const;

	void LoadStatData();

	void StartStaminaRegenDelay();

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
	float StaminaRegen_Base = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float StaminaRegen_PerEND = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 Souls = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 BaseInvestCost = FixedInitialInvestCost;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float SwordDamage = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float GunDamage = 15;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float SwordDamageBase = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float GunDamageBase = 15;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float SwordDamagePerSTR = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float GunDamagePerDEX = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float WeaponEnhancementBonus = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float WeaponEnhancementRate = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 SwordEnhancementLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 GunEnhancementLevel = 0;

	UPROPERTY(BlueprintAssignable)
	FOnDead OnDead;

	UPROPERTY(BlueprintAssignable)
	FOnStatChanged OnStatChanged;

	static constexpr int32 MinStatValue = 1;
	static constexpr int32 MaxStatValue = 99;
	static constexpr int32 MaxWeaponEnhancementLevel = 10;

	FTimerHandle StaminaRegenTimer;

protected:
	static constexpr int32 FixedInitialInvestCost = 100;
	
};