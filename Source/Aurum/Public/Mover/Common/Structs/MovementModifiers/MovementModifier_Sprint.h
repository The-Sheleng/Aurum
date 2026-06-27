// © 2026 Andrii Shelenhivskyi. Licensed under Creative Commons Attribution-NonCommercial 4.0.

#pragma once

#include "AurumMovementModifier.h"
#include "MovementModifier_Sprint.generated.h"

class UCommonLegacyMovementSettings;

/**
 * Movement modifier that applies sprint-specific movement settings.
 * @See UMovementSettings_Gait
 */
USTRUCT(BlueprintType)
struct AURUM_API FMovementModifier_Sprint : public FAurumMovementModifier
{
	GENERATED_BODY()

	virtual void OnStart(
		UMoverComponent* MoverComponent, 
		const FMoverTimeStep& TimeStep, 
		const FMoverSyncState& SyncState, 
		const FMoverAuxStateContext& AuxState) override;
	
	virtual void OnPreMovement(
		UMoverComponent* MoverComponent, 
		const FMoverTimeStep& TimeStep) override;
	
	virtual void OnEnd(
		UMoverComponent* MoverComponent, 
		const FMoverTimeStep& TimeStep, 
		const FMoverSyncState& SyncState, 
		const FMoverAuxStateContext& AuxState) override;

	virtual FMovementModifier_Sprint* Clone() const override { return new FMovementModifier_Sprint(*this); }
	virtual UScriptStruct* GetScriptStruct() const override { return FMovementModifier_Sprint::StaticStruct(); }
	
protected:
	/** Returns the cosine of the angle between facing and movement directions. */
	static float GetMovementCos(const UMoverComponent* MoverComponent);
	
	/** Applies sprint movement settings according to the current movement state. */
	virtual void UpdateMovementSettings(UMoverComponent* MoverComponent);
	
	/** Restores movement settings to their default values. */
	virtual void RevertMovementSettings(UMoverComponent* MoverComponent);
	
private:
	/** Cached movement direction cosine used to avoid unnecessary updates. */
	float CachedMovementCos = 0;
	
	/** True when sprint effects are temporarily disabled due to excessive sideslip angle. */
	bool bIsSprintSuppressed = false;
	
	/**
	 * Returns the default movement settings from the character CDO.
	 *
	 * These values are treated as the baseline state when applying
	 * and reverting sprint modifications.
	 *
	 * TODO:
	 * Replace this with a runtime aggregation system once movement
	 * settings can be affected by multiple independent sources.
	 */
	static const UCommonLegacyMovementSettings* GetDefaultMovementSettings(const UMoverComponent* MoverComponent);
};

template<>
struct TStructOpsTypeTraits<FMovementModifier_Sprint> : TStructOpsTypeTraitsBase2<FMovementModifier_Sprint>
{
	enum
	{
		WithCopy = true
	};
};
