// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "FPS_demoCharacter.h"
#include "FPS_demoProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	bIsReloading = false;
	CurrentAmmo = 0;  
	bSkill1Active = false;
	bSkill2Active = false;
	Skill1CooldownRemaining = 0.0f;
	Skill2CooldownRemaining = 0.0f;
	LastFireTime = 0.0f;
	
	
	
	
	//详细技能初始化
	// 瞄准参数
	AimingFieldOfView = 45.0f;
	AimingMaxWalkSpeed = 200.0f;
	bIsAiming = false;
	// 射速爆发参数
	OriginalFireRate = 0.0f;
	BuffedFireRate = 0.0f;
    
	// 翻滚参数
	DodgeDistance = 800.0f;
	DodgeDuration = 0.3f;
	DodgeStartTime = 0.0f;
	bIsDodging = false;
	// 大跳参数
	SuperJumpForce = 1200.0f;
	
	
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(TEXT("/Game/FPWeapon/Mesh/SK_FPGun"));
	if (MeshFinder.Succeeded())
	{
		USkeletalMeshComponent::SetSkeletalMesh(MeshFinder.Object);
	}
}



void UTP_WeaponComponent::BeginFire()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("begin fire"));
	if (!CanFire() || !WeaponData) return;  // 检查是否可以开火
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("can fire"));
	switch (WeaponData->WeaponType)
	{
	case EWeaponType::NormalRifle:
		// 停止可能存在的定时器
		StopFire();
		// 发射第一发子弹
		FireShot();
		GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this,
			&UTP_WeaponComponent::AutoFire, GetTimeBetweenShots(), true);
		// 参数说明：timer句柄，对象，回调函数，间隔时间，循环执行，初始延迟
		break;
	case EWeaponType::SniperRifle:
		FireShot();  // 单发模式：直接开火一次
		break;
	default: 
		break;
	}
	
}
//自动射击函数
void UTP_WeaponComponent::AutoFire()
{
	if (CanFire())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("autofire"));
		FireShot();
		// 动态设置下一次射击的时间
		
	}
	if (CurrentAmmo > 0 && !bIsReloading)//跳过canfire检查
	{
		// 清除旧的定时器
		GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);
            
		// 根据当前射速设置下一次射击
		float NextShotDelay = GetTimeBetweenShots();
		GetWorld()->GetTimerManager().SetTimer(FireTimerHandle, this,
			&UTP_WeaponComponent::AutoFire, NextShotDelay, false);
	}
	else if (CurrentAmmo <= 0)
	{
		// 弹药耗尽，停止射击
		StopFire();
	}
}
void UTP_WeaponComponent::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(FireTimerHandle);  // 清除自动模式的定时器
}

void UTP_WeaponComponent::Reload()
{
	// 检查是否可以装弹
	if (bIsReloading || CurrentAmmo == WeaponData->MaxAmmo ) return;
    
	bIsReloading = true;  // 设置装弹状态
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("start reload"));
	// 设置装弹定时器
	GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this,
		&UTP_WeaponComponent::FinishReloading, WeaponData->ReloadTime, false);
}

void UTP_WeaponComponent::ActivateSkill1()
{
	if (Skill1CooldownRemaining > 0.0f || !WeaponData) return;
    
	bSkill1Active = true;
	Skill1CooldownRemaining = WeaponData->Skill1Data.CooldownTime;
    
	// 设置技能持续时间计时器
	FTimerHandle Skill1TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(Skill1TimerHandle,
		[this]() { bSkill1Active = false; },
		WeaponData->Skill1Data.SkillDuration, false);
	
	switch (WeaponData->WeaponType)
	{
		case EWeaponType::NormalRifle: //翻滚
			{
				if (GEngine)
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "active skill1");
				ApplyDodge();
				break;
			}
			
			
		case EWeaponType::SniperRifle://大跳
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "active skill1");
				ApplySuperJump();
			break;
		default: 
			break;
	}
	
	
}

void UTP_WeaponComponent::ActivateSkill2()
{
	if (Skill2CooldownRemaining > 0.0f || !WeaponData) return;
    
	bSkill2Active = true;
	Skill2CooldownRemaining = WeaponData->Skill2Data.CooldownTime;
	FTimerHandle Skill2TimerHandle;
	switch (WeaponData->WeaponType)
	{
		
	case EWeaponType::NormalRifle: //加速
		{
			// 设置技能持续时间计时器
			
			GetWorld()->GetTimerManager().SetTimer(Skill2TimerHandle,
				[this]() { bSkill2Active = false; },
				WeaponData->Skill2Data.SkillDuration, false);
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "active skill2");
			ApplyFireRateBuff();
			break;
		}
	case EWeaponType::SniperRifle://开镜
		// 设置技能持续时间计时器
		GetWorld()->GetTimerManager().SetTimer(Skill2TimerHandle,
			[this]() { bSkill2Active = false; },
			WeaponData->Skill2Data.SkillDuration, false);
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, "active skill2");
		StartAimingMode();
		break;
	default: 
		break;
	}
}



bool UTP_WeaponComponent::CanFire() const
{
	if (!WeaponData) return false;          // 没有武器数据
	if (bIsReloading) return false;         // 正在装弹
	if (CurrentAmmo <= 0) return false; // 弹夹为空
    
	// 计算距离上次开火的时间间隔
	float TimeBetweenShots = GetTimeBetweenShots();
	float TimeSinceLastShot = GetWorld()->GetTimeSeconds() - LastFireTime;
    
	return TimeSinceLastShot >= TimeBetweenShots - 0.01f;  // 检查是否达到射速限制
}




void UTP_WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Character && Character->IsLocallyControlled())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				if (WeaponData)
				{
					Subsystem->RemoveMappingContext(WeaponData->FireMappingContext);
				}
			}
		}
	}
    
	Super::EndPlay(EndPlayReason);
}

void UTP_WeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 如果没有设置角色，尝试获取
	if (!Character)
	{
		Character = Cast<AFPS_demoCharacter>(GetOwner());
	}
    
	// 如果角色有效且在本地控制，设置输入绑定
	if (Character && Character->IsLocallyControlled() && !bInputBound)
	{
		SetupInputBindings();
	}
}

void UTP_WeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateSkillCooldowns(DeltaTime);
}


void UTP_WeaponComponent::ServerFireWeapon_Implementation(const FVector& SpawnLocation, const FRotator& SpawnRotation,
	float CalculateDamage,AActor* DamageCauser)
{	
	
	// 服务器验证调用者权限
	if (!Character || !Character->HasAuthority()) return;
	
	
	// 生成投射物
	UWorld* const World = GetWorld();
	if (World == nullptr) return;
    
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = 
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	ActorSpawnParams.Instigator = Cast<APawn>(GetOwner());
	ActorSpawnParams.Owner = GetOwner();
    
	AFPS_demoProjectile* Projectile = World->SpawnActor<AFPS_demoProjectile>(
		WeaponData->ProjectileClass, 
		SpawnLocation, 
		SpawnRotation, 
		ActorSpawnParams
	);
    
	if (Projectile)
	{
		Projectile->InitializeProjectile(CalculateDamage, DamageCauser);
	}
}
bool UTP_WeaponComponent::ServerFireWeapon_Validate(const FVector& SpawnLocation, const FRotator& SpawnRotation,
	float CalculateDamage,AActor* DamageCauser)
{
	return true;
}
void UTP_WeaponComponent::PlayFireEffects()
{
	// Try and play the sound if specified
	if (WeaponData->FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponData->FireSound, Character->GetActorLocation());
	}
	
	// Try and play a firing animation if specified
	if (WeaponData->FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(WeaponData->FireAnimation, 1.f);
		}
	}
}

void UTP_WeaponComponent::FireShot()
{
	if (!Character || !Character->GetController() || !WeaponData) return;
    
	// 检查是否可以开火
	if (!CanFire()) return;
    
	// 消耗弹药
	CurrentAmmo--;
	LastFireTime = GetWorld()->GetTimeSeconds();
    
	// 获取射击参数
	APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	if (!PlayerController) return;
    
	const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
	const FVector SpawnLocation = GetOwner()->GetActorLocation() + 
								 SpawnRotation.RotateVector(WeaponData->MuzzleOffset);
    
	AActor* Causer = Cast<AActor>(Character);
	float FinalDamage = CalculateDamage();
    
	// 播放效果
	PlayFireEffects();
    
	// 如果是服务器，直接生成子弹
	if (Character->HasAuthority())
	{
		UWorld* const World = GetWorld();
		if (World && WeaponData->ProjectileClass)
		{
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = 
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			ActorSpawnParams.Instigator = Cast<APawn>(GetOwner());
			ActorSpawnParams.Owner = GetOwner();
            
			AFPS_demoProjectile* Projectile = World->SpawnActor<AFPS_demoProjectile>(
				WeaponData->ProjectileClass, 
				SpawnLocation, 
				SpawnRotation, 
				ActorSpawnParams
			);
            
			if (Projectile)
			{
				Projectile->InitializeProjectile(FinalDamage, Causer);
			}
		}
	}
	else
	{
	
        
		ServerFireWeapon(SpawnLocation, SpawnRotation, FinalDamage, Causer);
	}

}

void UTP_WeaponComponent::FinishReloading()
{
	if (!WeaponData) return;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("reload finish"));
	CurrentAmmo = WeaponData->MaxAmmo;
	if (WeaponData->ReloadSound != nullptr)
    	{
    		UGameplayStatics::PlaySoundAtLocation(this, WeaponData->ReloadSound, Character->GetActorLocation());
    	}
	bIsReloading = false;
}

void UTP_WeaponComponent::UpdateSkillCooldowns(float DeltaTime)
{
	if (Skill1CooldownRemaining > 0.0f)
		Skill1CooldownRemaining -= DeltaTime;
    
	if (Skill2CooldownRemaining > 0.0f)
		Skill2CooldownRemaining -= DeltaTime;
}

void UTP_WeaponComponent::InitializeWeapon()
{
	if (WeaponData)
	{
		CurrentAmmo = WeaponData->MaxAmmo;
	}
	else
	{
		CurrentAmmo = 0;
		UE_LOG(LogTemp, Warning, TEXT("WeaponData is not set!"));
	}
	
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))//本地角色设置输入
	{
		if (OwnerPawn->IsLocallyControlled())
		{
			SetupInputBindings();
		}
	}
}





void UTP_WeaponComponent::ApplySuperJump() 
{
	if (!Character) return;
    
	// 如果是服务器，直接执行并广播
	if (Character->HasAuthority())
	{
		if (!Character) return;
    
		// 保存原始跳跃设置
		float OriginalJumpZVelocity = Character->GetCharacterMovement()->JumpZVelocity;
    
		// 应用大跳
		Character->GetCharacterMovement()->JumpZVelocity = SuperJumpForce;
		Character->Jump();
    
	
		// 播放音效
		if (WeaponData->Skill1Sound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, WeaponData->Skill1Sound, 
				Character->GetActorLocation());
		}
    
		// 恢复原始跳跃设置
		FTimerHandle ResetJumpTimer;
		GetWorld()->GetTimerManager().SetTimer(ResetJumpTimer,
			[this, OriginalJumpZVelocity]() {
				if (Character && Character->GetCharacterMovement())
				{
					Character->GetCharacterMovement()->JumpZVelocity = OriginalJumpZVelocity;
				}
			},
			0.1f, false);

		Multicast_ApplySuperJump();
	}
	else
	{
		//（本地也直接先执行）
		// 保存原始跳跃设置
		float OriginalJumpZVelocity = Character->GetCharacterMovement()->JumpZVelocity;
        
		// 应用大跳
		Character->GetCharacterMovement()->JumpZVelocity = SuperJumpForce;
		Character->Jump();
        
		// 恢复原始跳跃设置
		FTimerHandle ResetJumpTimer;
		GetWorld()->GetTimerManager().SetTimer(ResetJumpTimer,
			[this, OriginalJumpZVelocity]() {
				if (Character && Character->GetCharacterMovement())
				{
					Character->GetCharacterMovement()->JumpZVelocity = OriginalJumpZVelocity;
				}
			},
			0.1f, false);
        
		// 客户端本地播放音效
		if (WeaponData && WeaponData->Skill1Sound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, WeaponData->Skill1Sound, 
				Character->GetActorLocation());
		}
    

		// 客户端发送RPC到服务器
		Server_ApplySuperJump(SuperJumpForce);
		
	}
	
	
	
	
	
}

void UTP_WeaponComponent::ApplyDodge()
{
	if (!Character || bIsDodging) return;
    
	bIsDodging = true;
	DodgeStartTime = GetWorld()->GetTimeSeconds();
    
	// 确定翻滚方向
	FVector ForwardVector = Character->GetActorForwardVector();
	FVector RightVector = Character->GetActorRightVector();
    
	// 获取玩家输入方向
	FVector InputDirection = FVector::ZeroVector;
	if (Character->GetCharacterMovement())
	{
		InputDirection = Character->GetCharacterMovement()->GetLastInputVector();
	}
    
	// 如果有输入方向，按输入方向翻滚，否则按面朝方向
	if (!InputDirection.IsNearlyZero())
	{
		DodgeDirection = InputDirection.GetSafeNormal();
	}
	else
	{
		DodgeDirection = ForwardVector;
	}
    
	// 如果是服务器，直接执行
	if (Character->HasAuthority())
	{
		// 禁用玩家控制
		Character->GetCharacterMovement()->DisableMovement();
	
	
		// 播放翻滚音效
		if (WeaponData->Skill1Sound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, WeaponData->Skill1Sound, 
				Character->GetActorLocation());
		}
    
		// 开始翻滚更新
		FTimerDelegate DodgeDelegate;
		DodgeDelegate.BindUObject(this, &UTP_WeaponComponent::UpdateDodgeMovement, 0.016f);
    
		GetWorld()->GetTimerManager().SetTimer(DodgeTimerHandle, DodgeDelegate, 0.016f, true);
    
		// 设置翻滚结束计时器
		FTimerHandle DodgeEndTimer;
		GetWorld()->GetTimerManager().SetTimer(DodgeEndTimer,
			[this]() {
				OnDodgeEnd();
			},
			DodgeDuration, false);
	}
	else
	{
		// 客户端发送RPC到服务器
		Server_ApplyDodge(DodgeDirection);
	}
}

void UTP_WeaponComponent::UpdateDodgeMovement(float DeltaTime)
{
	if (!Character || !bIsDodging) return;
    
	// 计算当前翻滚进度（0到1）
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float Progress = (CurrentTime - DodgeStartTime) / DodgeDuration;
    
	if (Progress >= 1.0f)
	{
		OnDodgeEnd();
		return;
	}
    
	// 使用缓动函数使翻滚更平滑
	float EaseProgress = FMath::Sin(Progress * PI * 0.5f); // 缓入缓出
    
	// 计算当前帧的移动距离
	float FrameDistance = (DodgeDistance / DodgeDuration) * DeltaTime * EaseProgress;
    
	// 应用移动
	FVector DodgeVelocity = DodgeDirection * FrameDistance / DeltaTime;
	Character->GetCharacterMovement()->Velocity = DodgeVelocity;
    
	// 强制更新位置
	FVector NewLocation = Character->GetActorLocation() + DodgeVelocity * DeltaTime;
	Character->SetActorLocation(NewLocation, true);
}

void UTP_WeaponComponent::ApplyFireRateBuff()
{
	if (!WeaponData) return;
    
	// 保存原始射速
	OriginalFireRate = WeaponData->FireRate;
    
	// 计算爆发后的射速
	BuffedFireRate = OriginalFireRate * 3.0f;
	
	// 播放音效
	if (WeaponData->Skill2Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponData->Skill2Sound, 
			Character->GetActorLocation());
	}
    
}

void UTP_WeaponComponent::StartAimingMode()
{
	if (!Character || bIsAiming) return;
    
	bIsAiming = true;
    
	// 保存原始设置
	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			DefaultFieldOfView = PC->PlayerCameraManager->GetFOVAngle();
			PC->PlayerCameraManager->SetFOV(AimingFieldOfView);
		}
	}
    
	// 降低移动速度
	DefaultMaxWalkSpeed = Character->GetCharacterMovement()->MaxWalkSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeed = AimingMaxWalkSpeed;
	
    
	// 播放瞄准音效
	if (WeaponData->Skill2Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponData->Skill2Sound, 
			Character->GetActorLocation(), 0.5f);
	}
    
	UE_LOG(LogTemp, Log, TEXT("Aiming started"));
}

void UTP_WeaponComponent::EndAimingMode()
{
	if (!bIsAiming) return;
    
	bIsAiming = false;
    
	// 恢复视野
	if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->SetFOV(DefaultFieldOfView);
		}
	}
    
	// 恢复移动速度
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeed;
	}
    
	
}

void UTP_WeaponComponent::OnDodgeEnd()
{
	if (!bIsDodging) return;
    
	bIsDodging = false;
    
	// 停止翻滚更新计时器
	GetWorld()->GetTimerManager().ClearTimer(DodgeTimerHandle);
    
	// 恢复玩家控制
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		Character->GetCharacterMovement()->Velocity = FVector::ZeroVector;
	}
}


float UTP_WeaponComponent::GetTimeBetweenShots() const
{
	if (!WeaponData) return 0.1f;
    
	float EffectiveFireRate = WeaponData->FireRate;
    
	// 如果射速爆发技能激活，使用爆发后的射速
	if (bSkill2Active && WeaponData->WeaponType == EWeaponType::NormalRifle)
	{
		EffectiveFireRate = BuffedFireRate;
	}
    
	return 1.0f / EffectiveFireRate;
}

float UTP_WeaponComponent::CalculateDamage() const
{
	if (!WeaponData) return 0.0f;
    
	float Damage = WeaponData->BaseDamage;
    
	// 技能加成（射速爆发时）
	if (bSkill2Active && WeaponData->WeaponType == EWeaponType::NormalRifle)
	{
		// 射速爆发时伤害降低
		Damage *= 0.8f;
	}
    
	// 狙击枪
	if (WeaponData->WeaponType == EWeaponType::SniperRifle)
	{
		Damage *= 2.0f;
	}
    
	return Damage;
}

void UTP_WeaponComponent::SetOwningCharacter(AFPS_demoCharacter* NewCharacter)
{
    Character = NewCharacter;
}

void UTP_WeaponComponent::InitializeWeaponOnly()
{
    if (WeaponData)
    {
        CurrentAmmo = WeaponData->MaxAmmo;
    }
    else
    {
        CurrentAmmo = 0;
        UE_LOG(LogTemp, Warning, TEXT("WeaponData is not set!"));
    }
    
    // 只在本地控制的角色上绑定输入
    if (Character && Character->IsLocallyControlled())
    {
        SetupInputBindings();
    }
}

void UTP_WeaponComponent::SetupInputBindings()
{
    if (!Character || !Character->IsLocallyControlled() || !WeaponData || bInputBound) 
    {
        return;
    }
    
    APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
    if (!PlayerController) return;

    // 设置输入映射上下文
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(WeaponData->FireMappingContext, 1);
    }

    // 绑定输入动作
    if (UEnhancedInputComponent* EnhancedInputComponent = 
        Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
    {
        // Fire
        switch (WeaponData->WeaponType)
        {
            case EWeaponType::NormalRifle:
                EnhancedInputComponent->BindAction(WeaponData->FireAction, 
                    ETriggerEvent::Started, this, &UTP_WeaponComponent::BeginFire);
                EnhancedInputComponent->BindAction(WeaponData->FireAction, 
                    ETriggerEvent::Completed, this, &UTP_WeaponComponent::StopFire);
        		// Skill
        		EnhancedInputComponent->BindAction(WeaponData->SkillAction1, 
					ETriggerEvent::Started, this, &UTP_WeaponComponent::ActivateSkill1);
    		
    		
        		EnhancedInputComponent->BindAction(WeaponData->SkillAction2, 
					ETriggerEvent::Started, this, &UTP_WeaponComponent::ActivateSkill2);
                break;
                
            case EWeaponType::SniperRifle:
                EnhancedInputComponent->BindAction(WeaponData->FireAction, 
                    ETriggerEvent::Started, this, &UTP_WeaponComponent::BeginFire);
        	
        		// Skill
        		EnhancedInputComponent->BindAction(WeaponData->SkillAction1, 
					ETriggerEvent::Started, this, &UTP_WeaponComponent::ActivateSkill1);
    		
    		
        		EnhancedInputComponent->BindAction(WeaponData->SkillAction2, 
					ETriggerEvent::Started, this, &UTP_WeaponComponent::ActivateSkill2);
        		EnhancedInputComponent->BindAction(WeaponData->SkillAction2, 
					ETriggerEvent::Completed, this, &UTP_WeaponComponent::EndAimingMode);
                break;
                
            default:
                EnhancedInputComponent->BindAction(WeaponData->FireAction, 
                    ETriggerEvent::Started, this, &UTP_WeaponComponent::BeginFire);
                EnhancedInputComponent->BindAction(WeaponData->FireAction, 
                    ETriggerEvent::Completed, this, &UTP_WeaponComponent::StopFire);
                break;
        }
        
        
        
        // Reload
        EnhancedInputComponent->BindAction(WeaponData->ReloadAction, 
            ETriggerEvent::Started, this, &UTP_WeaponComponent::Reload);
    }
    
    bInputBound = true;
}


void UTP_WeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
	DOREPLIFETIME(UTP_WeaponComponent, WeaponData);
	// 确保其他需要复制的变量也在这里添加
}


void UTP_WeaponComponent::OnRep_WeaponData()
{
	// 当WeaponData被复制到客户端时，重新初始化
	if (WeaponData && GetOwner() && GetOwner()->HasAuthority() == false)
	{
		InitializeWeapon();
	}
}



void UTP_WeaponComponent::Server_ApplySuperJump_Implementation(float JumpForce)
{
	if (!Character) return;
    
	// 保存原始跳跃设置
	float OriginalJumpZVelocity = Character->GetCharacterMovement()->JumpZVelocity;
    
	// 应用大跳
	Character->GetCharacterMovement()->JumpZVelocity = SuperJumpForce;
	Character->Jump();
    
	
	// 播放音效
	if (WeaponData->Skill1Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponData->Skill1Sound, 
			Character->GetActorLocation());
	}
    
	// 恢复原始跳跃设置
	FTimerHandle ResetJumpTimer;
	GetWorld()->GetTimerManager().SetTimer(ResetJumpTimer,
		[this, OriginalJumpZVelocity]() {
			if (Character && Character->GetCharacterMovement())
			{
				Character->GetCharacterMovement()->JumpZVelocity = OriginalJumpZVelocity;
			}
		},
		0.1f, false);

	Multicast_ApplySuperJump();
}


void UTP_WeaponComponent::Multicast_ApplySuperJump_Implementation()
{
	// 如果不是服务器，并且在非本地控制的角色上播放效果
	if (!Character || !Character->HasAuthority())
	{
		// 播放音效
		if (WeaponData->Skill1Sound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, WeaponData->Skill1Sound, 
				Character->GetActorLocation());
		}
	}
}


void UTP_WeaponComponent::Server_ApplyDodge_Implementation(FVector Direction)
{
	if (!Character || bIsDodging) return;
	// 服务器验证并执行
	DodgeDirection = Direction;
    
	// 开始翻滚
	bIsDodging = true;
	DodgeStartTime = GetWorld()->GetTimeSeconds();
    
	// 禁用玩家控制
	Character->GetCharacterMovement()->DisableMovement();
    
	// 播放翻滚音效
	if (WeaponData->Skill1Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponData->Skill1Sound, 
			Character->GetActorLocation());
	}
    
	// 开始翻滚更新
	FTimerDelegate DodgeDelegate;
	DodgeDelegate.BindUObject(this, &UTP_WeaponComponent::UpdateDodgeMovement, 0.016f);
    
	GetWorld()->GetTimerManager().SetTimer(DodgeTimerHandle, DodgeDelegate, 0.016f, true);
    
	// 设置翻滚结束计时器
	FTimerHandle DodgeEndTimer;
	GetWorld()->GetTimerManager().SetTimer(DodgeEndTimer,
		[this]() {
			OnDodgeEnd();
		},
		DodgeDuration, false);
    
	// 广播到所有客户端
	Multicast_ApplyDodge(Direction);
}


void UTP_WeaponComponent::Multicast_ApplyDodge_Implementation(FVector Direction)
{
	// 如果不是服务器，并且在非本地控制的角色上播放效果
	if (!Character || Character->HasAuthority()) return;
    
	// 播放翻滚音效
	if (WeaponData->Skill1Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponData->Skill1Sound, 
			Character->GetActorLocation());
	}
	bIsDodging = false;
}