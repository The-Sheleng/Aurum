// © 2026 Andrii Shelenhivskyi. Licensed under Creative Commons Attribution-NonCommercial 4.0.


#include "Mover/Common/Structs/MovementModifiers/MovementModifier_Sprint.h"

#include "MoverComponent.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "Mover/Components/AurumMoverComponent.h"
#include "Mover/Objects/SharedSettings/MovementSettings_Gait.h"
#include "Pawns/AurumCharacter.h"

void FMovementModifier_Sprint::OnStart(UMoverComponent* MoverComp, const FMoverTimeStep& TimeStep,
	const FMoverSyncState& SyncState, const FMoverAuxStateContext& AuxState)
{
	const float CurrentMovementCos = GetMovementCos(MoverComp);

	CachedMovementCos = CurrentMovementCos;
	UpdateMovementSettings(MoverComp);
}

void FMovementModifier_Sprint::OnPreMovement(UMoverComponent* MoverComp, const FMoverTimeStep& TimeStep)
{
	const float CurrentMovementCos = GetMovementCos(MoverComp);

	if (FMath::Abs(CurrentMovementCos - CachedMovementCos) > KINDA_SMALL_NUMBER)
	{
		CachedMovementCos = CurrentMovementCos;
		UpdateMovementSettings(MoverComp);
	}
}

void FMovementModifier_Sprint::OnEnd(
	UMoverComponent* MoverComp,
	const FMoverTimeStep& TimeStep,
	const FMoverSyncState& SyncState, 
	const FMoverAuxStateContext& AuxState)
{
	RevertMovementSettings(MoverComp);
}

float FMovementModifier_Sprint::GetMovementCos(const UMoverComponent* MoverComp)
{
	const FVector CharacterForward = MoverComp->GetOwner()->GetActorForwardVector();
	const FVector MovementVelocityDir = MoverComp->GetVelocity().GetSafeNormal();

	return FVector::DotProduct(CharacterForward, MovementVelocityDir);
}

void FMovementModifier_Sprint::UpdateMovementSettings(UMoverComponent* MoverComp)
{
	/** Validity check and settings. */
    const UMovementSettings_Gait* SprintSettings = MoverComp->FindSharedSettings_Mutable<UMovementSettings_Gait>();
    if (!ensure(IsValid(SprintSettings))) return;

	/**
	 * Note: The smaller the angle, the LARGER the cosine.
	 * If the cosine of the current direction is less 
	 * than the permissible value (the angle is too large), cancel the sprint.
	 */
	const float MinimalSideslipCos = FMath::Cos(FMath::DegreesToRadians(SprintSettings->MinimalSideslipAngle));
	if (CachedMovementCos < MinimalSideslipCos)
	{
		if (bIsSprintSuppressed) return;
		
		bIsSprintSuppressed = true;
		RevertMovementSettings(MoverComp);

		return;
	}
	
	bIsSprintSuppressed = false;
	
    UCommonLegacyMovementSettings* ActiveMovementSettings = 
    	MoverComp->FindSharedSettings_Mutable<UCommonLegacyMovementSettings>();
    if (!ensure(IsValid(ActiveMovementSettings))) return;
	
	const UCommonLegacyMovementSettings* DefaultMovementSettings = GetDefaultMovementSettings(MoverComp);
    if (!ensure(IsValid(DefaultMovementSettings))) return;

	/**
	 * Speed interpolation depending on the drift angle.
	 * It doesn't hurt to use a clamp to avoid division by zero or values outside the bounds.
	 */
    const float InterpDenominator = 1.0f - MinimalSideslipCos;
    const float MaxSpeedCoefficient = (InterpDenominator > KINDA_SMALL_NUMBER) 
        ? FMath::Clamp((CachedMovementCos - MinimalSideslipCos) / InterpDenominator, 0.0f, 1.0f) 
        : 1.0f;

    const float MaxSpeedDelta = SprintSettings->SprintMaxSpeedOverride - DefaultMovementSettings->MaxSpeed;

    /** Apply modifiers. */
    ActiveMovementSettings->MaxSpeed = DefaultMovementSettings->MaxSpeed + (MaxSpeedCoefficient * MaxSpeedDelta);
    ActiveMovementSettings->Acceleration = SprintSettings->SprintAccelerationOverride;
    ActiveMovementSettings->Deceleration = SprintSettings->SprintDecelerationOverride;
    ActiveMovementSettings->TurningRate = SprintSettings->SprintTurningRateOverride;
}

void FMovementModifier_Sprint::RevertMovementSettings(UMoverComponent* MoverComp)
{
	/** Validity check and settings. */
	UCommonLegacyMovementSettings* MovementSettings =
		MoverComp->FindSharedSettings_Mutable<UCommonLegacyMovementSettings>();
	if (!ensure(IsValid(MovementSettings))) return;
	
	const UCommonLegacyMovementSettings* DefaultMovementSettings = GetDefaultMovementSettings(MoverComp);
	if (!ensure(IsValid(DefaultMovementSettings))) return;
	
	/** Apply modifiers. */
	MovementSettings->MaxSpeed = DefaultMovementSettings->MaxSpeed;
	MovementSettings->Acceleration = DefaultMovementSettings->Acceleration;
	MovementSettings->Deceleration = DefaultMovementSettings->Deceleration;
	MovementSettings->TurningRate = DefaultMovementSettings->TurningRate;
}

const UCommonLegacyMovementSettings* FMovementModifier_Sprint::GetDefaultMovementSettings(const UMoverComponent* MoverComp)
{
	const AAurumCharacter* CharacterCDO = MoverComp->GetOwner()->GetClass()->GetDefaultObject<AAurumCharacter>();
	const UAurumMoverComponent* MoverComponentCDO = CharacterCDO->GetMoverComponent();
	if (!IsValid(MoverComponentCDO)) return nullptr;
	
	return MoverComponentCDO->FindSharedSettings<UCommonLegacyMovementSettings>();
}
