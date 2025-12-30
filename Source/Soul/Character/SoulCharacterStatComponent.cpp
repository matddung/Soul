#include "SoulCharacterStatComponent.h"

USoulCharacterStatComponent::USoulCharacterStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void USoulCharacterStatComponent::BeginPlay()
{
	Super::BeginPlay();

	BaseInvestCost = FixedInitialInvestCost;

	RecalculateDerivedStats(true);
}

int32 USoulCharacterStatComponent::GetLevel() const
{
	return STR + DEX + VIT + END;
}

int32 USoulCharacterStatComponent::GetCurrentInvestCost() const
{
	const float CostFloat = (float)BaseInvestCost * FMath::Pow(CostMultiplier, (float)InvestCount);
	return FMath::CeilToInt(CostFloat);
}

void USoulCharacterStatComponent::RecalculateDerivedStats(bool bKeepCurrentRatio)
{
	const float OldMaxHP = MaxHP;
	const float OldMaxStamina = MaxStamina;

	MaxHP = HP_Base + (float)(FMath::Max(1, VIT) - 1) * HP_PerVIT;
	MaxStamina = Stamina_Base + (float)(FMath::Max(1, END) - 1) * Stamina_PerEND;

	StaminaRegenRate = StaminaRegen_Base + (float)(FMath::Max(1, END) - 1) * StaminaRegen_PerEND;

	SwordDamage = SwordDamageBase + (float)(FMath::Max(1, STR) - 1) * SwordDamagePerSTR;
	GunDamage = GunDamageBase + (float)(FMath::Max(1, DEX) - 1) * GunDamagePerDEX;

	if (bKeepCurrentRatio)
	{
		const float HPRatio = (OldMaxHP > 0) ? (HP / OldMaxHP) : 1;
		const float StRatio = (OldMaxStamina > 0) ? (Stamina / OldMaxStamina) : 1;

		HP = FMath::Clamp(MaxHP * HPRatio, 0, MaxHP);
		Stamina = FMath::Clamp(MaxStamina * StRatio, 0, MaxStamina);
	}
	else
	{
		HP = FMath::Clamp(HP, 0, MaxHP);
		Stamina = FMath::Clamp(Stamina, 0, MaxStamina);
	}

	OnStatChanged.Broadcast();
}

bool USoulCharacterStatComponent::TryInvestStat(ECharacterStatType StatToIncrease)
{
	const int32 Cost = GetCurrentInvestCost();

	if (Souls < Cost)
	{
		return false;
	}

	Souls -= Cost;

	AddToStat(StatToIncrease, 1);

	InvestCount += 1;

	RecalculateDerivedStats(true);

	return true;
}

void USoulCharacterStatComponent::AddSouls(int32 Amount)
{
	const int32 Clamped = FMath::Max(0, Amount);

	if (Clamped <= 0)
	{
		return;
	}

	Souls += Clamped;

	OnStatChanged.Broadcast();
}

bool USoulCharacterStatComponent::ApplyDamage(float DamageAmount)
{
	if (DamageAmount <= 0)
	{
		return false;
	}

	if (IsDead())
	{
		return false;
	}

	const float OldHP = HP;

	HP = FMath::Clamp(HP - DamageAmount, 0, MaxHP);

	if (HP <= 0 && OldHP > 0)
	{
		OnDead.Broadcast();
	}

	if (!FMath::IsNearlyEqual(OldHP, HP))
	{
		OnStatChanged.Broadcast();
	}

	return !FMath::IsNearlyEqual(OldHP, HP);
}

bool USoulCharacterStatComponent::IsDead() const
{
	return HP <= 0;
}

void USoulCharacterStatComponent::ResetCurrentToMax()
{
	RecalculateDerivedStats(false);
	HP = MaxHP;
	Stamina = MaxStamina;
}

int32 USoulCharacterStatComponent::GetStatRef(ECharacterStatType StatType) const
{
	switch (StatType)
	{
	case ECharacterStatType::STR: return STR;
	case ECharacterStatType::DEX: return DEX;
	case ECharacterStatType::VIT: return VIT;
	case ECharacterStatType::END: return END;
	default: return 0;
	}
}

void USoulCharacterStatComponent::AddToStat(ECharacterStatType StatType, int32 Delta)
{
	switch (StatType)
	{
	case ECharacterStatType::STR: STR = FMath::Max(1, STR + Delta); break;
	case ECharacterStatType::DEX: DEX = FMath::Max(1, DEX + Delta); break;
	case ECharacterStatType::VIT: VIT = FMath::Max(1, VIT + Delta); break;
	case ECharacterStatType::END: END = FMath::Max(1, END + Delta); break;
	default: break;
	}
}

FCharacterDerivedStats USoulCharacterStatComponent::GetCurrentDerivedStats() const
{
	return GetDerivedStatsInternal(STR, DEX, VIT, END);
}

FCharacterDerivedStats USoulCharacterStatComponent::GetPreviewDerivedStats(ECharacterStatType StatToIncrease) const
{
	int32 NextSTR = STR;
	int32 NextDEX = DEX;
	int32 NextVIT = VIT;
	int32 NextEND = END;

	switch (StatToIncrease)
	{
	case ECharacterStatType::STR: ++NextSTR; break;
	case ECharacterStatType::DEX: ++NextDEX; break;
	case ECharacterStatType::VIT: ++NextVIT; break;
	case ECharacterStatType::END: ++NextEND; break;
	default: break;
	}

	return GetDerivedStatsInternal(NextSTR, NextDEX, NextVIT, NextEND);
}

FCharacterDerivedStats USoulCharacterStatComponent::GetDerivedStatsInternal(int32 InSTR, int32 InDEX, int32 InVIT, int32 InEND) const
{
	FCharacterDerivedStats Result;

	Result.MaxHP = HP_Base + (float)(FMath::Max(1, InVIT) - 1) * HP_PerVIT;
	Result.MaxStamina = Stamina_Base + (float)(FMath::Max(1, InEND) - 1) * Stamina_PerEND;
	Result.StaminaRegenRate = StaminaRegen_Base + (float)(FMath::Max(1, InEND) - 1) * StaminaRegen_PerEND;
	Result.SwordDamage = SwordDamageBase + (float)(FMath::Max(1, InSTR) - 1) * SwordDamagePerSTR;
	Result.GunDamage = GunDamageBase + (float)(FMath::Max(1, InDEX) - 1) * GunDamagePerDEX;

	return Result;
}