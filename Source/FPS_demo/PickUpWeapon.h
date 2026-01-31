// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUpWeapon.generated.h"

class UWeaponDataAsset;
class UTP_WeaponComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponPickedUp, class AFPS_demoCharacter*, PickingCharacter, class UWeaponDataAsset*, WeaponData);
UCLASS()
class FPS_DEMO_API APickUpWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APickUpWeapon();
    
	// 武器类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<UTP_WeaponComponent> WeaponClass;
    
	// 武器数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UWeaponDataAsset* WeaponData;
    
	// 碰撞组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* CollisionComponent;
    
	// 网格组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComponent;
	
	// 拾取事件委托
	UPROPERTY(BlueprintAssignable, Category = "Pickup")
	FOnWeaponPickedUp OnWeaponPickedUp;
    
protected:
	virtual void BeginPlay() override;
    
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, 
						AActor* OtherActor, 
						UPrimitiveComponent* OtherComp, 
						int32 OtherBodyIndex, 
						bool bFromSweep, 
						const FHitResult& SweepResult);
    
	// 服务器RPC处理拾取
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerHandlePickup(AFPS_demoCharacter* PickingCharacter);
};
