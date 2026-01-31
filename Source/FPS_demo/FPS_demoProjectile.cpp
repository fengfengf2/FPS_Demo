// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPS_demoProjectile.h"

#include "HealthComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

AFPS_demoProjectile::AFPS_demoProjectile() 
{
	bNetLoadOnClient = true;//使actor也会在客户端加载
	bReplicates = true;//actor变为可复制对象（只要变量标记为复制，数据就会自动同步）
	//SetReplicates(true);在构造函数用上面的，其他函数可以用这个函数
	SetReplicatingMovement(true);//实现移动同步

    
	// 初始化变量
	Damage = 0.0f;
	DamageCauser = nullptr;
	bHasHit = false;
	
	
	
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AFPS_demoProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 5.0f;
	
	
	
}

void AFPS_demoProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPS_demoProjectile, Damage);
	DOREPLIFETIME(AFPS_demoProjectile, DamageCauser);
	DOREPLIFETIME_CONDITION(AFPS_demoProjectile, bHasHit, COND_SimulatedOnly);
}

void AFPS_demoProjectile::InitializeProjectile(float InDamage, AActor* InDamageCauser)
{
	if (HasAuthority())//只有服务器设置伤害
	{
		Damage = InDamage;
		DamageCauser = InDamageCauser;
	}
}

void AFPS_demoProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 避免重复触发
	if (bHasHit) return;
	if (HasAuthority())
	{
		// 标记已命中
		bHasHit = true;
		// Only add impulse and destroy projectile if we hit a physics
		if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) )
		{
			if (OtherComp->IsSimulatingPhysics())
			{
				OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
			}
			ApplyDamage(OtherActor, Hit);
			
			Destroy();
		}
	}
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
    
	// 延迟销毁，给效果播放时间
	SetLifeSpan(0.5f);
}

void AFPS_demoProjectile::OnRep_bHasHit()
{
	if (bHasHit)
	{
		// 客户端：隐藏投射物
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}

void AFPS_demoProjectile::ApplyDamage(AActor* HitActor, const FHitResult& Hit) const
{
	if (!HitActor) return;
	// 检查是否有健康组件
	UHealthComponent* HealthComp = HitActor->FindComponentByClass<UHealthComponent>();
	if (HealthComp)
	{
		// 创建伤害信息
		FDamageInfo DamageInfo;
		DamageInfo.DamageAmount = Damage;
		DamageInfo.DamageCauser = DamageCauser;
        
		// 应用伤害
		HealthComp->ApplyDamage(DamageInfo);

	}
}


