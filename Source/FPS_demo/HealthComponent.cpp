// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

#include "FPS_demoCharacter.h"
#include "TP_WeaponComponent.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	MaxHealth = 100.0f;
	bIsDead = false;
}

void UHealthComponent::ApplyDamage(const FDamageInfo& DamageInfo)
{
	//只有服务器能应用伤害
	if (!GetOwner()->HasAuthority()||bIsDead || DamageInfo.DamageAmount <= 0.0f) return;
	
	// 应用伤害
	float DamageApplied = FMath::Min(CurrentHealth, DamageInfo.DamageAmount);
	CurrentHealth -= DamageApplied;
	
	//OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    
	// 检查死亡
	if (CurrentHealth <= 0.0f && !bIsDead)
	{
		Die(DamageInfo.DamageCauser);
	}
}

void UHealthComponent::Heal(float HealAmount)
{
	if (!GetOwner()->HasAuthority()||bIsDead || HealAmount <= 0.0f) return;
    
	float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Min(CurrentHealth + HealAmount, MaxHealth);
    
	// 如果生命值有变化，触发事件(有rep了)
	//if (CurrentHealth > OldHealth)
	//{
		//OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	//}
}

float UHealthComponent::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f) return 0.0f;
	return CurrentHealth / MaxHealth;
}


void UHealthComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass,CurrentHealth);
	DOREPLIFETIME(ThisClass,bIsDead);
	
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
    
	// 初始化生命值
	CurrentHealth = MaxHealth;
    
	// 触发初始生命值更新
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	
}



void UHealthComponent::OnRep_CurrentHealth()
{
	//FString message = FString::Printf(TEXT("Health changed to %.0f %%"),CurrentHealth );
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, message);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);//客户端广播生命调用
}

void UHealthComponent::Die(AActor* DamageCauser)
{
	if (bIsDead) return;
    
	bIsDead = true;
    
	// 服务器端死亡处理
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		
        
		// 通知Character执行死亡逻辑
		if (AFPS_demoCharacter* Character = Cast<AFPS_demoCharacter>(GetOwner()))
		{
			Character->Die(DamageCauser);
		}
		else if (APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			// 如果是其他Pawn，执行基本死亡逻辑
			Pawn->DisableInput(nullptr);
            
			if (APlayerController* PC = Pawn->GetController<APlayerController>())
			{
				PC->SetIgnoreMoveInput(true);
				PC->SetIgnoreLookInput(true);
			}
		}
	}
	// 触发死亡事件（所有端都会触发）
	OnDeath.Broadcast(DamageCauser);
}


