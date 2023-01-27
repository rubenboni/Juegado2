// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimMontage.h"
#include "Core.h"
#include "Net/UnrealNetwork.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Character.h"
#include "ExtraDataHit.h"
#include "Sound/SoundCue.h"
#include "Math/Vector.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "CombatSystemCPP.generated.h"


UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReceiveDamage, bool, CanReceiveDamage, bool, IsBlocking);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable, BlueprintType)
class JUEGARDO_API UCombatSystemCPP : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCombatSystemCPP();

	UPROPERTY(Replicated, BlueprintReadOnly)
		bool IsAttacking;

	UPROPERTY(Replicated, BlueprintReadOnly)
		bool IsBlocking;

	UPROPERTY(Replicated, BlueprintReadWrite)
		bool IsParring;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations | Dash")
		bool CanDashWhenAttacking;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
		FOnReceiveDamage OnReceiveDamageReplicated;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations | Hit")
		TArray<UAnimMontage*>  HitAnimations;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations | Hit")
		TArray<UAnimMontage*>  KnockbackAnimations;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations | Hit")
		UAnimMontage* ReceiveParryAnimation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations | Block")
		UAnimMontage* BlockAnimation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations | Hit")
		UAnimMontage* DamageOverTimeHitAnimation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations | Hit")
		UParticleSystem* EmitterBloodHit;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations | Hit")
		float RecoverTime;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations | Block")
		UParticleSystem* EmitterBlock;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animations | Dash")
		TArray<UAnimMontage*>  Dashes;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
		USoundCue* HitSound;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
		USoundCue* BlockSound;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
		USoundCue* ParrySound;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
		USoundCue* KnockBackSound;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
		USoundAttenuation* HitAttenuation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound")
		USoundCue* DamageOverTimeHitSound;



protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated)
		TArray<UAnimMontage*>  AnimationsAux;

	UPROPERTY(Replicated)
		float AttackSpeedAux;

	UPROPERTY(Replicated)
		bool AttackSave;

	UPROPERTY(Replicated)
		int Combo;

	UPROPERTY(Replicated)
		bool CanReceiveDamage;

	UPROPERTY(Replicated)
		bool CanAttack;

	ACharacter* OwnerCharacter;

	FTimerHandle TimerResetAttack;

	//Attack

	void AttackByAnimations(float _AttackSpeed, const TArray<UAnimMontage*>& _Animations);

	void AttackByAnimations_Simulation(float _AttackSpeed, const TArray<UAnimMontage*>& _Animations);

	UFUNCTION(Server, Reliable)
		void AttackByAnimationsClient(float _AttackSpeed, const TArray<UAnimMontage*>& _Animations);
	void AttackByAnimationsClient_Implementation(float _AttackSpeed, const TArray<UAnimMontage*>& _Animations);

	//Replicate Emitter to all except the owner if run on server 
	UFUNCTION(NetMulticast, Reliable)
		void PlayEmitterSimulated(UParticleSystem* Emitter, FVector Location, FRotator Rotation, ACharacter* Owner);
	void PlayEmitterSimulated_Implementation(UParticleSystem* Emitter, FVector Location, FRotator Rotation, ACharacter* Owner);


	//Replicated on all PC except the owner if run on server and owner is playing the same Montage. If a client owner call this will do nothing.
	UFUNCTION(NetMulticast, Reliable)
		void PlayAnimationMulticastSimulated(ACharacter* Character, UAnimMontage* Montage, float PlayRate, ACharacter* Owner);
	void PlayAnimationMulticastSimulated_Implementation(ACharacter* Character, UAnimMontage* Montage, float PlayRate, ACharacter* Owner);


	UFUNCTION(NetMulticast, Reliable)
		void PlayAnimationMulticast(ACharacter* Character, UAnimMontage* Montage, float PlayRate);
	void PlayAnimationMulticast_Implementation(ACharacter* Character, UAnimMontage* Montage, float PlayRate);

	UFUNCTION(Server, Reliable)
		void PlayAnimationClient(ACharacter* Character, UAnimMontage* Montage, float PlayRate);
	void PlayAnimationClient_Implementation(ACharacter* Character, UAnimMontage* Montage, float PlayRate);

	//Reset
	void ResetAttack();

	//Save attack
	void SaveAttack();

	UFUNCTION(Server, Reliable)
		void SaveAttackClient();
	void SaveAttackClient_Implementation();

	//ReceiveDamage
	void ReceiveDamage(const FExtraData& ExtraData, AController* EventInstigator, AActor* DamageCauser, const FHitResult& HitInfo);


	UFUNCTION(Server, Reliable)
		void ReceiveDamageClient(const FExtraData& ExtraData, AController* EventInstigator, AActor* DamageCauser, const FHitResult& HitInfo);
	void ReceiveDamageClient_Implementation(const FExtraData& ExtraData, AController* EventInstigator, AActor* DamageCauser, const FHitResult& HitInfo);


	//Dash
	void Dash(float RightAxis, float ForwardAxis);

	UFUNCTION(Server, Reliable)
		void DashClient(float RightAxis, float ForwardAxis);
	void DashClient_Implementation(float RightAxis, float ForwardAxis);


	//Inmunity
	UFUNCTION(NetMulticast, Reliable)
		void StartInmunityServer();
	void StartInmunityServer_Implementation();

	UFUNCTION(Server, Reliable)
		void StartInmunityClient();
	void StartInmunityClient_Implementation();

	UFUNCTION(NetMulticast, Reliable)
		void StopInmunityServer();
	void StopInmunityServer_Implementation();

	UFUNCTION(Server, Reliable)
		void StopInmunityClient();
	void StopInmunityClient_Implementation();

	//Block
	UFUNCTION(NetMulticast, Reliable)
		void StartBlockServer();
	void StartBlockServer_Implementation();

	UFUNCTION(Server, Reliable)
		void StartBlockClient();
	void StartBlockClient_Implementation();

	UFUNCTION(NetMulticast, Reliable)
		void StopBlockServer();
	void StopBlockServer_Implementation();

	UFUNCTION(Server, Reliable)
		void StopBlockClient();
	void StopBlockClient_Implementation();

	//Block
	UFUNCTION(NetMulticast, Reliable)
		void StartParryServer();
	void StartParryServer_Implementation();

	UFUNCTION(Server, Reliable)
		void StartParryClient();
	void StartParryClient_Implementation();

	UFUNCTION(NetMulticast, Reliable)
		void StopParryServer();
	void StopParryServer_Implementation();

	UFUNCTION(Server, Reliable)
		void StopParryClient();
	void StopParryClient_Implementation();

	//Play Sound
	UFUNCTION(NetMulticast, Reliable)
		void PlaySoundSimulated(USoundCue* Sound, FVector Location, float Volume, float Pitch, USoundAttenuation* Attenuation, ACharacter* Owner);
	void PlaySoundSimulated_Implementation(USoundCue* Sound, FVector Location, float Volume, float Pitch, USoundAttenuation* Attenuation, ACharacter* Owner);


	UAnimMontage* CalculateHitAnimation(FVector Vector1, FVector Vector2, TArray<UAnimMontage*>  HitAnimations);

	UAnimMontage* CalculateDashAnimation(float RightAxis, float ForwardAxis, TArray<UAnimMontage*>  DashesAnimations);

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
		void AttackByAnimationsMultiplayer(float attackSpeed, UPARAM(ref) const TArray<UAnimMontage*>& animations);

	UFUNCTION(BlueprintCallable)
		void ResetAttackMultiplayer();

	UFUNCTION(BlueprintCallable)
		void SaveAttackMultiplayer();

	UFUNCTION(BlueprintCallable)
		void ReceiveDamageMultiplayer(const FExtraData& ExtraData, AController* EventInstigator, AActor* DamageCauser, const FHitResult& HitInfo);

	UFUNCTION(BlueprintCallable)
		void DashMultiplayer(float RightAxis, float ForwardAxis);

	UFUNCTION(BlueprintCallable)
		void StartInmunity();

	UFUNCTION(BlueprintCallable)
		void StopInmunity();

	UFUNCTION(BlueprintCallable)
		void StartBlock();

	UFUNCTION(BlueprintCallable)
		void StopBlock();

	UFUNCTION(BlueprintCallable)
		void StartParry();

	UFUNCTION(BlueprintCallable)
		void StopParry();

	//Replicated on all PC
	UFUNCTION(BlueprintCallable)
		void PlayAnimationReplicated(ACharacter* Character, UAnimMontage* Montage, float PlayRate);

};
