// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDataAsset.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	NormalRifle ,
	SniperRifle
};
//技能数据
USTRUCT(BlueprintType)
struct FWeaponSkillData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SkillName;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CooldownTime;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SkillDuration;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInstance*  IconMaterial;
};
UCLASS(BlueprintType,Blueprintable)
class FPS_DEMO_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	// 基础属性
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float BaseDamage = 10.0f; // 每秒射击次数
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float FireRate = 10.0f; // 每秒射击次数
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	int32 MaxAmmo = 10;//最大弹药数量
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	float ReloadTime = 2.0f;
    
	// 武器类型
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Type")
	EWeaponType WeaponType;
    
	// 技能数据
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	FWeaponSkillData Skill1Data;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	FWeaponSkillData Skill2Data;
	
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AFPS_demoProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* ReloadSound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* Skill1Sound;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* Skill2Sound;
	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* ReloadAnimation;
	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector MuzzleOffset= FVector(100.0f, 0.0f, 10.0f);

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* FireMappingContext;

	/** Fire Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* FireAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* SkillAction1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* SkillAction2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* ReloadAction;
	
};
