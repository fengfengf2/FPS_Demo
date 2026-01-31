// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"
//伤害信息
USTRUCT(Blueprintable, BlueprintType)
struct FDamageInfo
{
	GENERATED_BODY()
    
	UPROPERTY(BlueprintReadWrite)
	float DamageAmount = 0.0f;
    
	UPROPERTY(BlueprintReadWrite)
	AActor* DamageCauser = nullptr;
};

// 委托声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DamageCauser);
UCLASS(Blueprintable,ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPS_DEMO_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyDamage(const FDamageInfo& DamageInfo);
    
	// 恢复生命值
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float HealAmount);
    
	// 属性获取
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }
    
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }
    
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const;
    
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return bIsDead; }
    
	// 事件委托
	UPROPERTY(BlueprintAssignable, Category = "Health Events")
	FOnHealthChanged OnHealthChanged;
    
	UPROPERTY(BlueprintAssignable, Category = "Health Events")
	FOnDeath OnDeath;
    
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	virtual void BeginPlay() override;
    
private:
	// 生命值属性
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float MaxHealth = 100.0f;
    
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth , BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	float CurrentHealth;
	UPROPERTY(Replicated,VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	bool bIsDead;
    
	// 处理死亡
	void Die(AActor* DamageCauser);
	
	UFUNCTION()
	void OnRep_CurrentHealth();
	
};
