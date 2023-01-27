// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatSystemCPP.h"

// Sets default values for this component's properties
UCombatSystemCPP::UCombatSystemCPP()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	AttackSave = false;
	Combo = 0;
	IsAttacking = false;
	AttackSpeedAux = 1;
	RecoverTime = 1.f;
	CanReceiveDamage = true;
	IsBlocking = false;
	CanAttack = true;
	IsParring = false;
	CanDashWhenAttacking = true;
	OwnerCharacter = Cast<ACharacter>(GetOwner());
}


// Called when the game starts
void UCombatSystemCPP::BeginPlay()
{
	Super::BeginPlay();


	// ...

}

void UCombatSystemCPP::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatSystemCPP, Combo);
	DOREPLIFETIME(UCombatSystemCPP, IsAttacking);
	DOREPLIFETIME(UCombatSystemCPP, AttackSave);
	DOREPLIFETIME(UCombatSystemCPP, AnimationsAux);
	DOREPLIFETIME(UCombatSystemCPP, AttackSpeedAux);
	DOREPLIFETIME(UCombatSystemCPP, IsBlocking);
	DOREPLIFETIME(UCombatSystemCPP, IsParring);
	DOREPLIFETIME(UCombatSystemCPP, CanReceiveDamage);
	DOREPLIFETIME(UCombatSystemCPP, CanAttack);


}

void UCombatSystemCPP::AttackByAnimations(float _AttackSpeed, const TArray<UAnimMontage*>& _Animations)
{
	if (CanAttack) {

		if (_Animations != AnimationsAux) {
			Combo = 0;
			AnimationsAux.Empty(0);

			for (auto i : _Animations) {

				AnimationsAux.Add(i);

			}
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Añadidoooooo"));
			this->AttackSpeedAux = _AttackSpeed;

		}

		if (IsAttacking) {

			AttackSave = true;
			if (GetOwner()->GetLocalRole() != ROLE_Authority) {
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Client:IsAttacking"));
				UE_LOG(LogTemp, Warning, TEXT("Client: IsAttacking"));
			}
			if (GetOwner()->GetLocalRole() == ROLE_Authority) {
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Server: IsAttacking"));
				UE_LOG(LogTemp, Warning, TEXT("Server: IsAttacking"));
			}
		}

		else {

			IsAttacking = true;
			if (GetOwner()->GetLocalRole() == ROLE_Authority) {
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Server: No Attacking"));
				UE_LOG(LogTemp, Warning, TEXT("Server: No Attacking"));
			}
			else {
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Client: No Attacking"));
				UE_LOG(LogTemp, Warning, TEXT("Client: No Attacking"));
			}


			if (OwnerCharacter->IsLocallyControlled()) {
				//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, TEXT("Owner Play montage"));
				OwnerCharacter->PlayAnimMontage(_Animations[Combo], _AttackSpeed);
			}

			PlayAnimationMulticastSimulated(OwnerCharacter, _Animations[Combo], _AttackSpeed, OwnerCharacter);

			if (Combo + 1 >= AnimationsAux.Num()) {

				Combo = 0;
			}
			else {

				Combo++;

			}

		}


	}


}



// Called every frame
void UCombatSystemCPP::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UCombatSystemCPP::AttackByAnimationsClient_Implementation(float _AttackSpeed, const TArray<UAnimMontage*>& _Animations)
{

	AttackByAnimations(_AttackSpeed, _Animations);

}

void UCombatSystemCPP::PlayEmitterSimulated_Implementation(UParticleSystem* Emitter, FVector Location, FRotator Rotation, ACharacter* Owner)
{
	if (!(Owner->IsLocallyControlled())) {

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Emitter, Location, Rotation, FVector(1, 1, 1), true, EPSCPoolMethod::AutoRelease, true);
	}
}

void UCombatSystemCPP::PlayAnimationReplicated(ACharacter* Character, UAnimMontage* Montage, float PlayRate)
{
	if (GetOwner()->GetLocalRole() == ROLE_Authority) {

		PlayAnimationMulticast(Character, Montage, PlayRate);

	}
	else {

		PlayAnimationClient(Character, Montage, PlayRate);

	}

}

void UCombatSystemCPP::PlayAnimationMulticast_Implementation(ACharacter* Character, UAnimMontage* Montage, float PlayRate)
{
	Character->PlayAnimMontage(Montage, PlayRate);
	OwnerCharacter->ForceNetUpdate();
}

void UCombatSystemCPP::PlayAnimationMulticastSimulated_Implementation(ACharacter* Character, UAnimMontage* Montage, float PlayRate, ACharacter* Owner)
{
	//If the character is already playing the same animation, doesnt start it again
	if (!(Owner->IsLocallyControlled()) || !Character->GetMesh()->GetAnimInstance()->Montage_IsPlaying(Montage) && Owner->IsLocallyControlled()) {

		Character->PlayAnimMontage(Montage, PlayRate);
		Owner->ForceNetUpdate();
	}
}

void UCombatSystemCPP::PlayAnimationClient_Implementation(ACharacter* Character, UAnimMontage* Montage, float PlayRate)
{

	PlayAnimationMulticast(Character, Montage, PlayRate);


}

void UCombatSystemCPP::ResetAttack()
{
	CanAttack = true;
	Combo = 0;
	AttackSave = false;
	IsAttacking = false;
	
}


void UCombatSystemCPP::SaveAttack()
{

	if (AttackSave) {

		AttackSave = false;
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::FromInt(Combo));
		PlayAnimationMulticastSimulated(OwnerCharacter, AnimationsAux[Combo], AttackSpeedAux, OwnerCharacter);

		if (Combo + 1 >= AnimationsAux.Num()) {

			Combo = 0;
		}
		else {

			Combo++;

		}
	}

}

void UCombatSystemCPP::SaveAttackMultiplayer()
{
	if (GetOwner()->GetLocalRole() == ROLE_Authority) {

		SaveAttack();

	}
	else if (OwnerCharacter->IsLocallyControlled()) {

		SaveAttack();

	}
}

void UCombatSystemCPP::ReceiveDamage(const FExtraData& ExtraData, AController* EventInstigator, AActor* DamageCauser, const FHitResult& HitInfo)
{

	if (CanReceiveDamage) {

		CanAttack = false;


		if (IsBlocking) {

			//Multicast

			PlayAnimationMulticastSimulated(OwnerCharacter, BlockAnimation, AttackSpeedAux, Cast<ACharacter>(DamageCauser));
			if (EmitterBlock != nullptr)
				PlayEmitterSimulated(EmitterBlock, FVector(HitInfo.ImpactPoint.X, HitInfo.ImpactPoint.Y, HitInfo.ImpactPoint.Z), HitInfo.Normal.Rotation(), Cast<ACharacter>(DamageCauser));

			if (BlockSound != nullptr) {
				PlaySoundSimulated(BlockSound, OwnerCharacter->GetActorLocation(), 1.f, 1.f, HitAttenuation, Cast<ACharacter>(DamageCauser));
			}

			//Simulation
			if (Cast<ACharacter>(DamageCauser)->IsLocallyControlled()) {
				OwnerCharacter->PlayAnimMontage(BlockAnimation, AttackSpeedAux);
				if (EmitterBlock != nullptr) {
					//UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EmitterBloodHit, FVector(HitInfo.ImpactPoint.X, HitInfo.ImpactPoint.Y, HitInfo.ImpactPoint.Z), HitInfo.Normal.Rotation(), FVector(1, 1, 1), true, true, ENCPoolMethod::AutoRelease, true);
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterBlock, FVector(HitInfo.ImpactPoint.X, HitInfo.ImpactPoint.Y, HitInfo.ImpactPoint.Z), HitInfo.Normal.Rotation(), FVector(1, 1, 1), true, EPSCPoolMethod::AutoRelease, true);
				}
				if (BlockSound != nullptr) {
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), BlockSound, OwnerCharacter->GetActorLocation(), 1.f, 1.f, 0.0f, HitAttenuation);
				}

			}



		}
		else if (IsParring) {



			UCombatSystemCPP* Cs = Cast<UCombatSystemCPP>(DamageCauser->GetComponentByClass(UCombatSystemCPP::StaticClass()));

			FExtraData ED = FExtraData();
			ED.TypeOfAttack = AttackType::Parry;
			FHitResult hitResult = FHitResult();

			if (Cs != nullptr)
				Cs->ReceiveDamageMultiplayer(ED, OwnerCharacter->GetController(), OwnerCharacter, hitResult);

			//Simulacion
			if (Cast<ACharacter>(DamageCauser)->IsLocallyControlled()) {
				if (ParrySound != nullptr) {
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), ParrySound, OwnerCharacter->GetActorLocation(), 1.f, 1.f, 0.0f, HitAttenuation);
				}
			}

			//Replication
			if (ParrySound != nullptr) {
				PlaySoundSimulated(ParrySound, OwnerCharacter->GetActorLocation(), 1.f, 1.f, HitAttenuation, Cast<ACharacter>(DamageCauser));
			}

		}

		else {


			UAnimMontage* HitAnimationAux;

			switch (ExtraData.TypeOfAttack.GetValue()) {

			case AttackType::Normal:

				HitAnimationAux = CalculateHitAnimation(OwnerCharacter->GetActorForwardVector(), DamageCauser->GetActorForwardVector(), HitAnimations);


				PlayAnimationMulticastSimulated(OwnerCharacter, HitAnimationAux, AttackSpeedAux, Cast<ACharacter>(DamageCauser));
				if (EmitterBloodHit != nullptr)
					PlayEmitterSimulated(EmitterBloodHit, FVector(HitInfo.ImpactPoint.X, HitInfo.ImpactPoint.Y, HitInfo.ImpactPoint.Z), HitInfo.Normal.Rotation(), Cast<ACharacter>(DamageCauser));



				if (Cast<ACharacter>(DamageCauser)->IsLocallyControlled()) {
					OwnerCharacter->PlayAnimMontage(HitAnimationAux, AttackSpeedAux);
					if (EmitterBloodHit != nullptr) {
						//UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EmitterBloodHit, FVector(HitInfo.ImpactPoint.X, HitInfo.ImpactPoint.Y, HitInfo.ImpactPoint.Z), HitInfo.Normal.Rotation(), FVector(1, 1, 1), true, true, ENCPoolMethod::AutoRelease, true);
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterBloodHit, FVector(HitInfo.ImpactPoint.X, HitInfo.ImpactPoint.Y, HitInfo.ImpactPoint.Z), HitInfo.Normal.Rotation(), FVector(1, 1, 1), true, EPSCPoolMethod::AutoRelease, true);
					}
					if (HitSound != nullptr) {
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, OwnerCharacter->GetActorLocation(), 1.f, 1.f, 0.0f, HitAttenuation);
					}

				}

				if (HitSound != nullptr) {
					PlaySoundSimulated(HitSound, OwnerCharacter->GetActorLocation(), 1.f, 1.f, HitAttenuation, Cast<ACharacter>(DamageCauser));
				}


				break;

			case AttackType::Knockback:


				HitAnimationAux = CalculateHitAnimation(OwnerCharacter->GetActorForwardVector(), DamageCauser->GetActorForwardVector(), KnockbackAnimations);


				PlayAnimationMulticastSimulated(OwnerCharacter, HitAnimationAux, AttackSpeedAux, Cast<ACharacter>(DamageCauser));
				if (EmitterBloodHit != nullptr)
					PlayEmitterSimulated(EmitterBloodHit, FVector(HitInfo.ImpactPoint.X, HitInfo.ImpactPoint.Y, HitInfo.ImpactPoint.Z), HitInfo.Normal.Rotation(), Cast<ACharacter>(DamageCauser));

				if (KnockBackSound != nullptr) {
					PlaySoundSimulated(KnockBackSound, OwnerCharacter->GetActorLocation(), 1.f, 1.f, HitAttenuation, Cast<ACharacter>(DamageCauser));
				}

				//Simulation
				if (Cast<ACharacter>(DamageCauser)->IsLocallyControlled()) {
					OwnerCharacter->PlayAnimMontage(HitAnimationAux, AttackSpeedAux);
					if (EmitterBloodHit != nullptr) {
						//UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EmitterBloodHit, FVector(HitInfo.ImpactPoint.X, HitInfo.ImpactPoint.Y, HitInfo.ImpactPoint.Z), HitInfo.Normal.Rotation(), FVector(1, 1, 1), true, true, ENCPoolMethod::AutoRelease, true);
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EmitterBloodHit, FVector(HitInfo.ImpactPoint.X, HitInfo.ImpactPoint.Y, HitInfo.ImpactPoint.Z), HitInfo.Normal.Rotation(), FVector(1, 1, 1), true, EPSCPoolMethod::AutoRelease, true);
					}
					if (KnockBackSound != nullptr) {
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), KnockBackSound, OwnerCharacter->GetActorLocation(), 1.f, 1.f, 0.0f, HitAttenuation);
					}

				}



				break;

			case AttackType::Parry:


				HitAnimationAux = ReceiveParryAnimation;
				PlayAnimationMulticastSimulated(OwnerCharacter, HitAnimationAux, AttackSpeedAux, Cast<ACharacter>(DamageCauser));

				if (Cast<ACharacter>(DamageCauser)->IsLocallyControlled()) {
					OwnerCharacter->PlayAnimMontage(HitAnimationAux, AttackSpeedAux);
				}

				break;


			case AttackType::DamageOvertime:

				HitAnimationAux = DamageOverTimeHitAnimation;

				PlayAnimationMulticastSimulated(OwnerCharacter, HitAnimationAux, AttackSpeedAux, Cast<ACharacter>(DamageCauser));

				if (Cast<ACharacter>(DamageCauser)->IsLocallyControlled()) {
					OwnerCharacter->PlayAnimMontage(HitAnimationAux, AttackSpeedAux);
					if (HitSound != nullptr) {
						UGameplayStatics::PlaySoundAtLocation(GetWorld(), DamageOverTimeHitSound, OwnerCharacter->GetActorLocation(), 1.f, 1.f, 0.0f, HitAttenuation);
					}

				}

				if (HitSound != nullptr) {
					PlaySoundSimulated(DamageOverTimeHitSound, OwnerCharacter->GetActorLocation(), 1.f, 1.f, HitAttenuation, Cast<ACharacter>(DamageCauser));
				}


				break;
			}

		}


		OwnerCharacter->GetWorldTimerManager().ClearTimer(TimerResetAttack);
		OwnerCharacter->GetWorldTimerManager().SetTimer(
			TimerResetAttack, this, &UCombatSystemCPP::ResetAttackMultiplayer,RecoverTime, false, RecoverTime);

	}


}




void UCombatSystemCPP::ReceiveDamageClient_Implementation(const FExtraData& ExtraData, AController* EventInstigator, AActor* DamageCauser, const FHitResult& HitInfo)
{
	ReceiveDamage(ExtraData, EventInstigator, DamageCauser, HitInfo);
}

void UCombatSystemCPP::Dash(float RightAxis, float ForwardAxis)
{
	if ((IsAttacking && CanDashWhenAttacking) || !IsAttacking) {
		ResetAttackMultiplayer();
		UAnimMontage* auxMontage = CalculateDashAnimation(RightAxis, ForwardAxis, Dashes);

		if (OwnerCharacter->IsLocallyControlled()) {
			OwnerCharacter->PlayAnimMontage(auxMontage, 1);
		}
		PlayAnimationMulticastSimulated(OwnerCharacter, auxMontage, 1, OwnerCharacter);
	}
}



void UCombatSystemCPP::ReceiveDamageMultiplayer(const FExtraData& ExtraData, AController* EventInstigator, AActor* DamageCauser, const FHitResult& HitInfo)
{

	if (DamageCauser->GetLocalRole() == ROLE_Authority) {

		ReceiveDamage(ExtraData, EventInstigator, DamageCauser, HitInfo);

	}
	else {
		if (Cast<ACharacter>(DamageCauser)->IsLocallyControlled()) {

			ReceiveDamage(ExtraData, EventInstigator, DamageCauser, HitInfo);
		}

	}
}

void UCombatSystemCPP::DashMultiplayer(float RightAxis, float ForwardAxis)
{

	if (GetOwner()->GetLocalRole() == ROLE_Authority) {

		Dash(RightAxis, ForwardAxis);

	}
	else {

		//This will simulate the function for reduce lag
		DashClient(RightAxis, ForwardAxis);
		Dash(RightAxis, ForwardAxis);

	}

}

void UCombatSystemCPP::StartInmunity()
{
	if (OwnerCharacter->GetLocalRole() == ROLE_Authority) {

		StartInmunityServer();

	}
	else {

		StartInmunityClient();

	}

	OwnerCharacter->ForceNetUpdate();
}

void UCombatSystemCPP::StopInmunity()
{
	if (OwnerCharacter->GetLocalRole() == ROLE_Authority) {

		StopInmunityServer();

	}
	else {

		StopInmunityClient();

	}

	OwnerCharacter->ForceNetUpdate();


}

void UCombatSystemCPP::DashClient_Implementation(float RightAxis, float ForwardAxis)
{

	Dash(RightAxis, ForwardAxis);

}

void UCombatSystemCPP::StartInmunityServer_Implementation()
{

	CanReceiveDamage = false;
	OwnerCharacter->ForceNetUpdate();
}

void UCombatSystemCPP::StartInmunityClient_Implementation()
{
	StartInmunityServer();

}

void UCombatSystemCPP::StopInmunityServer_Implementation()
{

	CanReceiveDamage = true;
	OwnerCharacter->ForceNetUpdate();

}

void UCombatSystemCPP::StopInmunityClient_Implementation()
{

	StopInmunityServer();

}

//Orden: Frontal, izquierda, derecha, atrás
UAnimMontage* UCombatSystemCPP::CalculateHitAnimation(FVector Vector1, FVector Vector2, TArray<UAnimMontage*> _HitAnimations)
{
	float degreeCos = UKismetMathLibrary::DegAsin(FVector::DotProduct(Vector1, Vector2));
	

	float degreeSin = UKismetMathLibrary::DegAsin(FVector::DotProduct(Vector1.RotateAngleAxis(90,FVector(0,0,1)), Vector2));

	int index;
	
	if (degreeCos <= 0) {

		if (UKismetMathLibrary::InRange_FloatFloat(degreeSin, 55, 90)) {
			
			index = 1;

		}
		else if (UKismetMathLibrary::InRange_FloatFloat(degreeSin, -90, -55)) {

			index = 2;

		}
		else {

			index = 0;

		}

	}
	else {

		index = 3;

	}
	
	return _HitAnimations[index];
}

UAnimMontage* UCombatSystemCPP::CalculateDashAnimation(float RightAxis, float ForwardAxis, TArray<UAnimMontage*> DashesAnimations)
{
	int index = 0;

	if (RightAxis == 0 && ForwardAxis >= 0.01)
		index = 0;
	else if (ForwardAxis >= 0.01 && RightAxis >= 0.1)
		index = 1;
	else if (ForwardAxis == 0 && RightAxis >= 0.1)
		index = 2;
	else if (ForwardAxis <= -0.01 && RightAxis >= 0.1)
		index = 3;
	else if (ForwardAxis <= -0.01 && RightAxis <= -0.1)
		index = 5;
	else if (ForwardAxis == 0 && RightAxis <= -0.1)
		index = 6;
	else if (ForwardAxis >= 0.01 && RightAxis <= -0.1)
		index = 7;
	else
		index = 4;

	if (index < DashesAnimations.Num())
		return DashesAnimations[index];
	else
		return nullptr;
}





void UCombatSystemCPP::SaveAttackClient_Implementation()
{

	SaveAttack();

}


void UCombatSystemCPP::ResetAttackMultiplayer()
{
	if (OwnerCharacter->GetLocalRole() == ROLE_Authority) {
		//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Server: Reset"));

		ResetAttack();
		OwnerCharacter->ForceNetUpdate();
	}


}


void UCombatSystemCPP::AttackByAnimationsMultiplayer(float attackSpeed, const TArray<UAnimMontage*>& animations)
{
	if (GetOwner()->GetLocalRole() == ROLE_Authority) {
		AttackByAnimations(attackSpeed, animations);

	}
	else {
		//This will simulate the function for reduce lag
		AttackByAnimationsClient(attackSpeed, animations);
		AttackByAnimations(attackSpeed, animations);

	}
	OwnerCharacter->ForceNetUpdate();
}

void UCombatSystemCPP::StartBlock()
{
	if (OwnerCharacter->GetLocalRole() == ROLE_Authority) {

		StartBlockServer();

	}
	else {

		StartBlockClient();

	}

	OwnerCharacter->ForceNetUpdate();
}

void UCombatSystemCPP::StopBlock()
{
	if (OwnerCharacter->GetLocalRole() == ROLE_Authority) {

		StopBlockServer();

	}
	else {

		StopBlockClient();

	}

	OwnerCharacter->ForceNetUpdate();


}

void UCombatSystemCPP::StartParry()
{

	if (OwnerCharacter->GetLocalRole() == ROLE_Authority) {

		StartParryServer();

	}
	OwnerCharacter->ForceNetUpdate();

}

void UCombatSystemCPP::StopParry()
{

	if (OwnerCharacter->GetLocalRole() == ROLE_Authority) {

		StopParryServer();

	}

	OwnerCharacter->ForceNetUpdate();

}

void UCombatSystemCPP::StartBlockServer_Implementation()
{

	IsBlocking = true;
	OwnerCharacter->ForceNetUpdate();
}

void UCombatSystemCPP::StartBlockClient_Implementation()
{
	StartBlockServer();

}

void UCombatSystemCPP::StopBlockServer_Implementation()
{

	IsBlocking = false;
	OwnerCharacter->ForceNetUpdate();

}

void UCombatSystemCPP::StopBlockClient_Implementation()
{

	StopBlockServer();

}

void UCombatSystemCPP::StartParryServer_Implementation()
{

	IsParring = true;

}

void UCombatSystemCPP::StartParryClient_Implementation()
{

	StartParryServer();

}

void UCombatSystemCPP::StopParryServer_Implementation()
{

	IsParring = false;

}

void UCombatSystemCPP::StopParryClient_Implementation()
{

	StopParryServer();

}

void UCombatSystemCPP::PlaySoundSimulated_Implementation(USoundCue* Sound, FVector Location, float Volume, float Pitch, USoundAttenuation* Attenuation, ACharacter* Owner)
{
	if (!(Owner->IsLocallyControlled())) {

		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location, Volume, Pitch, 0.f, Attenuation);
	}
}


