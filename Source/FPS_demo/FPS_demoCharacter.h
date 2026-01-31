// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "FPS_demoCharacter.generated.h"

class UTP_WeaponComponent;
class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);
// 死亡/复活事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDied);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerRespawned);

UCLASS(config=Game)
class AFPS_demoCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* TestAction;
public:
	AFPS_demoCharacter();
	
	
	
	// 死亡和复活
	UFUNCTION(BlueprintCallable, Category = "Player")
	void Die(AActor* DamageCauser = nullptr);

	
    
	
    
	// 获取组件
	UFUNCTION(BlueprintPure, Category = "Health")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }
    

	
    
	// 死亡复活委托声明
	UPROPERTY(BlueprintAssignable, Category = "Player Events")
	FOnPlayerDied OnPlayerDied;
    
	UPROPERTY(BlueprintAssignable, Category = "Player Events")
	FOnPlayerRespawned OnPlayerRespawned;
	
	
	// HealthComponent 相关
	UFUNCTION()
	void OnHealthChanged(float CurrentHealth, float MaxHealth);
    
	// 血量更新处理
	void UpdateLocalPlayerHealthUI(float CurrentHealth, float MaxHealth);
	
protected:
	virtual void BeginPlay();

	// 组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;
    
	



public:
		
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void Test() ;
	
	// 武器相关
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	UTP_WeaponComponent* GetCurrentWeapon() const { return CurrentWeapon; }
    
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(TSubclassOf<UTP_WeaponComponent> WeaponClass, UWeaponDataAsset* WeaponDataAsset);
    
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEquipWeapon(TSubclassOf<UTP_WeaponComponent> WeaponClass, UWeaponDataAsset* WeaponDataAsset);
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface
	
	// 当前持有的武器
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Weapon")
	UTP_WeaponComponent* CurrentWeapon;
	
	void SpawnWeapon(TSubclassOf<UTP_WeaponComponent> WeaponClass, UWeaponDataAsset* WeaponDataAsset);

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	
	void AddScore(int32 Score);
	
	// 设置重生点
	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void SetRespawnLocation(FVector Location, FRotator Rotation);

	


	
	
	// 重生点
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respawn")
	FVector RespawnLocation = FVector(0, 0, 100);
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respawn")
	FRotator RespawnRotation = FRotator(0, 0, 0);
	


	// 内部函数
	void TeleportToRespawnLocation();
private:
	// 添加客户端计时器句柄
	FTimerHandle ClientRespawnTimerHandle;
	
	
};

