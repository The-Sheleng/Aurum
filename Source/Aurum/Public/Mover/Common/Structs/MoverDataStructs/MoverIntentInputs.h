// © 2026 Andrii Shelenhivskyi. Licensed under Creative Commons Attribution-NonCommercial 4.0.

#pragma once

#include "MoverTypes.h"
#include "GameplayTagContainer.h"
#include "MoverIntentInputs.generated.h"

/**
 * Input payload containing Gameplay Tags that represent player intents.
 *
 * This allows gameplay commands (Dash, Slide, Interact, etc.)
 * to be extended without modifying the data structure or network serialization.
 */
USTRUCT(BlueprintType)
struct FMoverIntentInputs : public FMoverDataStructBase
{
	GENERATED_BODY()

	/** Active gameplay intent tags included in this Mover input payload. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mover)
	FGameplayTagContainer IntentTags;

	virtual FMoverDataStructBase* Clone() const override { return new FMoverIntentInputs(*this); }
    
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override 
	{
		IntentTags.NetSerialize(Ar, Map, bOutSuccess);
		bOutSuccess = true; 
		return true;
	}

	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }
	
	virtual bool ShouldReconcile(const FMoverDataStructBase& AuthorityState) const override
	{
		const FMoverIntentInputs& TypedAuthority = static_cast<const FMoverIntentInputs&>(AuthorityState);
		return IntentTags != TypedAuthority.IntentTags;
	}
	
	virtual void Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct) override
	{
		const FMoverIntentInputs& TypedFrom = static_cast<const FMoverIntentInputs&>(From);
		const FMoverIntentInputs& TypedTo = static_cast<const FMoverIntentInputs&>(To);
		
		*this = (Pct < 0.5f) ? TypedFrom : TypedTo;
	}
	
	virtual void Merge(const FMoverDataStructBase& From) override
	{
		const FMoverIntentInputs& TypedFrom = static_cast<const FMoverIntentInputs&>(From);
		IntentTags.AppendTags(TypedFrom.IntentTags);
	}
	
	virtual void ToString(FAnsiStringBuilderBase& Out) const override
	{
		Out.Appendf("IntentTags: %s", *IntentTags.ToString());
	}
};

template<>
struct TStructOpsTypeTraits<FMoverIntentInputs> : TStructOpsTypeTraitsBase2<FMoverIntentInputs>
{
	enum
	{
		WithNetSerializer = true, 
		WithCopy = true
	};
};
