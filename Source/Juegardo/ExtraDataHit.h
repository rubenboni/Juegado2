// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UObject/Class.h"
#include "ExtraDataHit.generated.h"

/**
 * 
 */


UENUM(BlueprintType)
enum AttackType
{
	Normal,
	Parry,
	Knockback,
	Stun,
	DamageOvertime,
};

USTRUCT(BlueprintType)
struct FExtraData
{
	GENERATED_BODY()

		//~ The following member variable will be accessible by Blueprint Graphs:
		// This is the tooltip for our test variable.
		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<AttackType> TypeOfAttack;
		
		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool IsCritic;

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Damage;

};