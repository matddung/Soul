#include "GameSettingSaveData.h"

FString UGameSettingSaveData::GetSlotName()
{
	static const FString SlotName = TEXT("GameSettingSlot");
	return SlotName;
}