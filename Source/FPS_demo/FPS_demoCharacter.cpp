// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPS_demoCharacter.h"
#include "FPS_demoProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FPS_demoPlayerController.h"
#include "HealthComponent.h"
#include "InputActionValue.h"
#include "Main_UI.h"
#include "My_demoGamemode.h"
#include "My_demoGameState.h"
#include "My_PlayerState.h"
#include "TP_WeaponComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AFPS_demoCharacter

AFPS_demoCharacter::AFPS_demoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	
	// 创建并设置HealthComponent
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	
	// 设置网络复制
	bReplicates = true;
	ACharacter::SetReplicateMovement(true);
	bAlwaysRelevant = true;  
	if (GetCharacterMovement())
	{
		// 提高移动更新的频率
		GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
		GetCharacterMovement()->MaxSimulationTimeStep = 0.05f;  // 20Hz
		GetCharacterMovement()->MaxSimulationIterations = 8;
		
		GetCharacterMovement()->MaxDepenetrationWithGeometry = 100.0f;
		
		GetCharacterMovement()->bRequestedMoveUseAcceleration = true;
	}
	
	CurrentWeapon = nullptr;
}



void AFPS_demoCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	
	// 绑定HealthComponent事件
	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &AFPS_demoCharacter::OnHealthChanged);
	}
	
	if (HasAuthority())
	{
		RespawnLocation = GetActorLocation();
		RespawnRotation = GetActorRotation();
	}
}

void AFPS_demoCharacter::OnHealthChanged(float CurrentHealth, float MaxHealth)
{
	UWorld* World = GetWorld();
	if (!World) return;
    
	// 添加调试输出
	/**
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, 
		FString::Printf(TEXT("Health Changed: %.1f/%.1f, Authority: %d, Dead:,charaonhealthchange"), 
			CurrentHealth, MaxHealth, HasAuthority()));
    
	**/
    
	// 更新本地UI（包括监听服务器上的本地玩家）
	if (IsLocallyControlled())
	{
		UpdateLocalPlayerHealthUI(CurrentHealth, MaxHealth);
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,"not locally");
		
	}
	
}



void AFPS_demoCharacter::UpdateLocalPlayerHealthUI(float CurrentHealth, float MaxHealth)
{
	
	
    AFPS_demoPlayerController* PC = Cast<AFPS_demoPlayerController>(GetController());
    if (PC && PC->MainUIInstance)
    {
    	
        PC->MainUIInstance->UpdateHealthBar(CurrentHealth, MaxHealth);
    }
}


void AFPS_demoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
	//DOREPLIFETIME(AFPS_demoCharacter, bIsDead);
	DOREPLIFETIME(AFPS_demoCharacter, CurrentWeapon);  
}

//////////////////////////////////////////////////////////////////////////// Input
//SetupPlayerInputComponent 是APawn类的一个重要虚函数，在被控制器控制时使用
void AFPS_demoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFPS_demoCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFPS_demoCharacter::Look);
		
		//Testing
		EnhancedInputComponent->BindAction(TestAction, ETriggerEvent::Started, this, &AFPS_demoCharacter::Test);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AFPS_demoCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AFPS_demoCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AFPS_demoCharacter::Test() 
{
	/**
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, HasAuthority()?"server Test":"client Test");
	
	
	// 展示玩家信息
	AMy_PlayerState* player_state = Cast<AMy_PlayerState>(GetPlayerState());
	if (player_state)
	{
		// 显示当前信息
		FString PlayerStateName = FString::Printf(TEXT("My Name is %s"), *player_state->GetMyName());
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Green, PlayerStateName);
        
		// 显示当前分数
		FString CurrentScore = FString::Printf(TEXT("Current Score is %d"), player_state->GetGameScore());
		GEngine->AddOnScreenDebugMessage(2, 5.f, FColor::Green, CurrentScore);
        
		// 增加分数
		if (HasAuthority())
		{
			// 服务器直接增加
			player_state->SetGameScore(player_state->GetGameScore() + 10);
			GEngine->AddOnScreenDebugMessage(3, 5.f, FColor::Yellow, 
				FString::Printf(TEXT("服务器增加10分，新分数: %d"), player_state->GetGameScore()));
		}
		else
		{
			// 客户端通过RPC请求增加分数
			player_state->Server_SetScore(player_state->GetGameScore() + 10);
			GEngine->AddOnScreenDebugMessage(3, 5.f, FColor::Yellow, 
				TEXT("客户端请求增加10分"));
		}
        
		// 再次显示分数（可能有延迟）
		FString PlayerStateScore = FString::Printf(TEXT("Updated Score: %d"), player_state->GetGameScore());
		GEngine->AddOnScreenDebugMessage(4, 5.f, FColor::Green, PlayerStateScore);
		
		
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, "PlayerState not found!");
	}
	
	
	if (HealthComponent)
	{
		HealthComponent->Heal(HealthComponent->GetMaxHealth());
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "heal");
	// 传送到重生点
	TeleportToRespawnLocation();
	FString message = FString::Printf(TEXT("Health  now: %.0f %%"),HealthComponent->GetCurrentHealth() );
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, message);
	
	**/
}

void AFPS_demoCharacter::Die(AActor* DamageCauser)
{
	//服务端死亡执行
	if (!HasAuthority()) return;
	//bIsDead = true;
    
	// 停止所有运动
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}
    
	// 广播死亡事件
	OnPlayerDied.Broadcast();
	
    if (AFPS_demoCharacter* killer =Cast<AFPS_demoCharacter>(DamageCauser))//如果是玩家沙的
    {
	    killer->AddScore(100);
    	
    }
	
	if (!HasAuthority()) 
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, 
			"只有服务器可以结束比赛");
		return;
	}
    
	UWorld* World = GetWorld();
	if (!World) return;
    
	// 获取GameMode直接结束游戏
	AMy_demoGamemode* GameMode = Cast<AMy_demoGamemode>(World->GetAuthGameMode());
	if (GameMode)
	{
		GameMode->EndMatch();
	}
}



void AFPS_demoCharacter::AddScore(int32 Score)
{
	// 展示玩家信息
	AMy_PlayerState* player_state = Cast<AMy_PlayerState>(GetPlayerState());
	if (player_state)
	{
		// 增加分数
		if (HasAuthority())
		{
			// 服务器直接增加
			player_state->SetGameScore(player_state->GetGameScore() + Score);
			
		}
		else
		{
			// 客户端通过RPC请求增加分数
			player_state->Server_SetScore(player_state->GetGameScore() + Score);
			
		}
        
		
		//FString PlayerStateScore = FString::Printf(TEXT("Updated Score: %d"), player_state->GetGameScore());
		//GEngine->AddOnScreenDebugMessage(4, 5.f, FColor::Green, PlayerStateScore);
		
		
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, "PlayerState not found!");
	}
}

//武器
void AFPS_demoCharacter::EquipWeapon(TSubclassOf<UTP_WeaponComponent> WeaponClass, UWeaponDataAsset* WeaponDataAsset)
{
	if (!WeaponClass || !WeaponDataAsset) return;
    
	// 如果是服务器，直接生成武器
	if (HasAuthority())
	{
		SpawnWeapon(WeaponClass, WeaponDataAsset);
	}
	else
	{
		// 如果是客户端，发送RPC到服务器
		ServerEquipWeapon(WeaponClass, WeaponDataAsset);
	}
}

void AFPS_demoCharacter::ServerEquipWeapon_Implementation(TSubclassOf<UTP_WeaponComponent> WeaponClass, UWeaponDataAsset* WeaponDataAsset)
{
	SpawnWeapon(WeaponClass, WeaponDataAsset);
}

bool AFPS_demoCharacter::ServerEquipWeapon_Validate(TSubclassOf<UTP_WeaponComponent> WeaponClass, UWeaponDataAsset* WeaponDataAsset)
{
	return true;
}

void AFPS_demoCharacter::SpawnWeapon(TSubclassOf<UTP_WeaponComponent> WeaponClass, UWeaponDataAsset* WeaponDataAsset)
{
	if (!WeaponClass || !WeaponDataAsset) return;
    
	// 如果已经有武器，先销毁
	if (CurrentWeapon)
	{
		CurrentWeapon->DestroyComponent();
		CurrentWeapon = nullptr;
	}
    
	// 创建新的武器组件
	CurrentWeapon = NewObject<UTP_WeaponComponent>(this, WeaponClass);
	if (CurrentWeapon)
	{
		// 设置武器数据
		CurrentWeapon->WeaponData = WeaponDataAsset;
        
		// 注册组件
		CurrentWeapon->RegisterComponent();
        
		// 附加到角色
		CurrentWeapon->AttachToComponent(GetMesh1P(), 
			FAttachmentTransformRules::SnapToTargetIncludingScale, 
			FName(TEXT("GripPoint")));
        
		// 添加到实例组件列表
		AddInstanceComponent(CurrentWeapon);
		
		// 初始化武器
		CurrentWeapon->InitializeWeaponOnly();
        
		// 如果是本地控制的角色（包括客户端和监听服务器的本地角色），设置输入绑定
		if (IsLocallyControlled())
		{
			CurrentWeapon->SetupInputBindings();
		}
	}
}








void AFPS_demoCharacter::SetRespawnLocation(FVector Location, FRotator Rotation)
{
	RespawnLocation = Location;
	RespawnRotation = Rotation;
}

void AFPS_demoCharacter::TeleportToRespawnLocation()
{
	// 停止所有运动
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
	}
	
	// 传送到重生点
	SetActorLocation(RespawnLocation);
	SetActorRotation(RespawnRotation);
	
	// 重置控制器旋转
	if (Controller)
	{
		Controller->SetControlRotation(RespawnRotation);
	}
}
