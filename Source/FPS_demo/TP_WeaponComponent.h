// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WeaponDataAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "TP_WeaponComponent.generated.h"





class AFPS_demoCharacter;

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPS_DEMO_API UTP_WeaponComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:

	
	/** Sets default values for this component's properties */
	UTP_WeaponComponent();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	/////////////////////////////////////////////////
	// 开火控制
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void BeginFire();
    
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFire();
    
	// 装弹
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Reload();
    
	// 技能
	UFUNCTION(BlueprintCallable, Category = "Weapon|Skills")
	void ActivateSkill1();
    
	UFUNCTION(BlueprintCallable, Category = "Weapon|Skills")
	void ActivateSkill2();
    
	// 属性获取
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }
    
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool CanFire() const;
    
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsReloading() const { return bIsReloading; }
	//详细技能函数
	void ApplySuperJump();
	void ApplyDodge();
	void UpdateDodgeMovement(float DeltaTime);
	void ApplyFireRateBuff();
	void StartAimingMode();
	void EndAimingMode();
    
	// 技能结束回调
	void OnDodgeEnd();
	
	
	//详细技能属性公开
	// 大跳相关
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ability)
	float SuperJumpForce;

	
	void InitializeWeapon();
	
	
protected:
	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
							  FActorComponentTickFunction* ThisTickFunction) override;
private:
	/** The Character holding this weapon*/
	AFPS_demoCharacter* Character;
	
	
	
	
	// 弹药管理
	int32 CurrentAmmo;
	bool bIsReloading;
    
	// 开火控制
	float LastFireTime;
	FTimerHandle FireTimerHandle;
	FTimerHandle ReloadTimerHandle;
    
	// 技能状态
	bool bSkill1Active;
	bool bSkill2Active;
	float Skill1CooldownRemaining;
	float Skill2CooldownRemaining;

	UFUNCTION(Server, Reliable,WithValidation)
	void ServerFireWeapon(const FVector& SpawnLocation, const FRotator& SpawnRotation, float CalculateDamage,AActor* DamageCauser);
	void PlayFireEffects();
	// 计时器回调
	void FireShot();
	void FinishReloading();
	void AutoFire();
	// 技能冷却更新
	void UpdateSkillCooldowns(float DeltaTime);
	
	float GetTimeBetweenShots()const;
	
	float CalculateDamage() const;
	
	
	//详细技能属性
	// 大跳参数
	// 瞄准相关
	float DefaultFieldOfView;
	float AimingFieldOfView;
	float DefaultMaxWalkSpeed;
	float AimingMaxWalkSpeed;
	bool bIsAiming;  // 瞄准状态
	// 射速爆发相关
	float OriginalFireRate;  // 原始射速
	float BuffedFireRate;    // 射速爆发时的射速
	FTimerHandle FireRateBuffTimerHandle;
    
	// 翻滚相关
	FTimerHandle DodgeTimerHandle;
	bool bIsDodging;
	FVector DodgeDirection;
	float DodgeDistance;
	float DodgeDuration;
	float DodgeStartTime;
    
public:
	UPROPERTY(ReplicatedUsing =OnRep_WeaponData, EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UWeaponDataAsset* WeaponData;

	// 简化的初始化函数（不绑定输入）
	void InitializeWeaponOnly();
    
	// 绑定输入的函数
	void SetupInputBindings();
    
	// 设置角色的函数
	void SetOwningCharacter(AFPS_demoCharacter* NewCharacter);
	
	// 添加复制通知
	UFUNCTION()
	void OnRep_WeaponData();
    
private:
	// 输入绑定是否已设置
	bool bInputBound;	
	
	
public:
	// 添加技能RPC函数
	UFUNCTION(Server, Reliable)
	void Server_ApplySuperJump(float JumpForce);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ApplySuperJump();

	UFUNCTION(Server, Reliable)
	void Server_ApplyDodge(FVector Direction);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ApplyDodge(FVector Direction);

	
};
