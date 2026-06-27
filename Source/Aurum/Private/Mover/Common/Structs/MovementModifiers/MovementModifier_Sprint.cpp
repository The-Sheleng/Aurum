// © 2026 Andrii Shelenhivskyi. Licensed under Creative Commons Attribution-NonCommercial 4.0.


#include "Mover/Common/Structs/MovementModifiers/MovementModifier_Sprint.h"

#include "MoverComponent.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "Mover/Components/AurumMoverComponent.h"
#include "Mover/Objects/SharedSettings/MovementSettings_Gait.h"
#include "Pawns/AurumCharacter.h"

void FMovementModifier_Sprint::OnStart(UMoverComponent* MoverComponent, 
	const FMoverTimeStep& TimeStep,
	const FMoverSyncState& SyncState, 
	const FMoverAuxStateContext& AuxState)
{
	const float CurrentMovementCos = GetMovementCos(MoverComponent);

	CachedMovementCos = CurrentMovementCos;
	UpdateMovementSettings(MoverComponent);
}

void FMovementModifier_Sprint::OnPreMovement(
	UMoverComponent* MoverComponent,
	const FMoverTimeStep& TimeStep)
{
	const float CurrentMovementCos = GetMovementCos(MoverComponent);

	if (FMath::Abs(CurrentMovementCos - CachedMovementCos) > KINDA_SMALL_NUMBER)
	{
		CachedMovementCos = CurrentMovementCos;
		UpdateMovementSettings(MoverComponent);
	}
}

void FMovementModifier_Sprint::OnEnd(
	UMoverComponent* MoverComponent,
	const FMoverTimeStep& TimeStep,
	const FMoverSyncState& SyncState, 
	const FMoverAuxStateContext& AuxState)
{
	RevertMovementSettings(MoverComponent);
}

float FMovementModifier_Sprint::GetMovementCos(const UMoverComponent* MoverComponent)
{
	const FVector CharacterForward = MoverComponent->GetOwner()->GetActorForwardVector();
	const FVector MovementVelocityDir = MoverComponent->GetVelocity().GetSafeNormal();

	return FVector::DotProduct(CharacterForward, MovementVelocityDir);
}

void FMovementModifier_Sprint::UpdateMovementSettings(UMoverComponent* MoverComponent)
{
	/** Validity check and settings. */
    const UMovementSettings_Gait* SprintSettings = MoverComponent->FindSharedSettings_Mutable<UMovementSettings_Gait>();
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
		RevertMovementSettings(MoverComponent);

		return;
	}
	
	bIsSprintSuppressed = false;
	
    UCommonLegacyMovementSettings* ActiveMovementSettings 
		= MoverComponent->FindSharedSettings_Mutable<UCommonLegacyMovementSettings>();
    if (!ensure(IsValid(ActiveMovementSettings))) return;
	
	const UCommonLegacyMovementSettings* DefaultMovementSettings = GetDefaultMovementSettings(MoverComponent);
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

void FMovementModifier_Sprint::RevertMovementSettings(UMoverComponent* MoverComponent)
{
	/** Validity check and settings. */
	UCommonLegacyMovementSettings* MovementSettings =
		MoverComponent->FindSharedSettings_Mutable<UCommonLegacyMovementSettings>();
	if (!ensure(IsValid(MovementSettings))) return;
	
	const UCommonLegacyMovementSettings* DefaultMovementSettings = GetDefaultMovementSettings(MoverComponent);
	if (!ensure(IsValid(DefaultMovementSettings))) return;
	
	/** Apply modifiers. */
	MovementSettings->MaxSpeed = DefaultMovementSettings->MaxSpeed;
	MovementSettings->Acceleration = DefaultMovementSettings->Acceleration;
	MovementSettings->Deceleration = DefaultMovementSettings->Deceleration;
	MovementSettings->TurningRate = DefaultMovementSettings->TurningRate;
}

const UCommonLegacyMovementSettings* FMovementModifier_Sprint::GetDefaultMovementSettings(
	const UMoverComponent* MoverComponent)
{
	const AAurumCharacter* CharacterCDO = MoverComponent->GetOwner()->GetClass()->GetDefaultObject<AAurumCharacter>();
	const UAurumMoverComponent* MoverComponentCDO = CharacterCDO->GetMoverComponent();
	if (!IsValid(MoverComponentCDO)) return nullptr;
	
	return MoverComponentCDO->FindSharedSettings<UCommonLegacyMovementSettings>();
}
