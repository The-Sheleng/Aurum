// © 2026 Andrii Shelenhivskyi. Licensed under Creative Commons Attribution-NonCommercial 4.0.

#pragma once

#include "MovementModifier.h"
#include "AurumMovementModifier.generated.h"

class UMoverComponent;

/**
 * Project-level base type for movement modifiers.
 *
 * This wrapper keeps Aurum-specific queue and cancel rules close to the modifier
 * itself while preserving compatibility with the Mover plugin's modifier system.
 */
USTRUCT(BlueprintType)
struct AURUM_API FAurumMovementModifier : public FMovementModifierBase
{
	GENERATED_BODY()

	FAurumMovementModifier() {}
	
	/** Returns true when this modifier is allowed to be queued on the Mover component. */
	virtual bool CanQueue(const UMoverComponent* MoverComponent) const { return true; }

	/** Returns true when this modifier is allowed to be canceled on the Mover component. */
	virtual bool CanCancel(const UMoverComponent* MoverComponent) const { return true; }
	
	virtual FAurumMovementModifier* Clone() const override { return new FAurumMovementModifier(*this); }
	virtual UScriptStruct* GetScriptStruct() const override { return FAurumMovementModifier::StaticStruct(); }
};

template<>
struct TStructOpsTypeTraits<FAurumMovementModifier> : TStructOpsTypeTraitsBase2<FAurumMovementModifier>
{
	enum
	{
		WithCopy = true
	};
};
