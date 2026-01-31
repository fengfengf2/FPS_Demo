// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUpWeapon.h"

#include "FPS_demoCharacter.h"
#include "TP_WeaponComponent.h"
#include "Components/SphereComponent.h"


// Sets default values
APickUpWeapon::APickUpWeapon()
{
	// 创建根组件
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->SetSphereRadius(50.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
	// 创建网格组件
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
    
	// 设置网络复制
	bReplicates = true;
}

// Called when the game starts or when spawned
void APickUpWeapon::BeginPlay()
{
	Super::BeginPlay();
	// 绑定重叠事件
	if (CollisionComponent)
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APickUpWeapon::OnOverlapBegin);
	}
}

void APickUpWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, 
								   AActor* OtherActor, 
								   UPrimitiveComponent* OtherComp, 
								   int32 OtherBodyIndex, 
								   bool bFromSweep, 
								   const FHitResult& SweepResult)
{
	AFPS_demoCharacter* Character = Cast<AFPS_demoCharacter>(OtherActor);
	if (!Character) return;
	
	if (Character->GetCurrentWeapon())return;
	
	OnWeaponPickedUp.Broadcast(Character,WeaponData);
	// 调用服务器RPC处理拾取
	ServerHandlePickup(Character);
}

void APickUpWeapon::ServerHandlePickup_Implementation(AFPS_demoCharacter* PickingCharacter)
{
	if (!PickingCharacter || !WeaponClass || !WeaponData) return;
   
	// 让角色装备武器
	PickingCharacter->EquipWeapon(WeaponClass, WeaponData);
    
	// 销毁拾取物
	Destroy();
}

bool APickUpWeapon::ServerHandlePickup_Validate(AFPS_demoCharacter* PickingCharacter)
{
	return true;
}

