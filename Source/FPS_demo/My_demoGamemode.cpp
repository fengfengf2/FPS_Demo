// Fill out your copyright notice in the Description page of Project Settings.


#include "My_demoGamemode.h"

#include "FPS_demoPlayerController.h"
#include "My_demoGameState.h"
#include "My_PlayerState.h"
#include "ProfilingDebugging/BootProfiling.h"

AMy_demoGamemode::AMy_demoGamemode()
{
	PlayerStateClass = AMy_PlayerState::StaticClass();
	
	bDelayedStart =	true;
	CountdownTime = 5.0f; // 5秒倒计时
	bIsCountingDown = false;
	PrimaryActorTick.bCanEverTick = true;
	bisGameStart = false;
	
	MatchDuration = 180.0f; // 3分钟
}



void AMy_demoGamemode::StartMatch()
{
	Super::StartMatch();
	bisGameStart  =true;
	//GEngine->AddOnScreenDebugMessage(1,5.f,FColor::Green,"the game started");
}

void AMy_demoGamemode::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMy_demoGamemode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bisGameStart)//游戏开始
	{
		CheckMatchTime();//检查时间
	}
	else
	{
		bool bAllReady = IsAllready();
    
		if (bAllReady && !bIsCountingDown)
		{
			// 全部准备好，开始倒计时
			StartCountdown();
		}
		else if (!bAllReady && bIsCountingDown)
		{
			// 有人取消准备，取消倒计时
			CancelCountdown();
		}
	}
	
}

void AMy_demoGamemode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	if (!NewPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("PostLogin: NewPlayer is null"));
		return;
	}
	
	if (AMy_demoGameState* Game_State = GetGameState<AMy_demoGameState>())
	{
		Game_State->AddTeamMember(NewPlayer);
	}
    
	//通知客户端显示 UI（只在服务端调用）
	if (HasAuthority())
	{
		AFPS_demoPlayerController* PC = Cast<AFPS_demoPlayerController>(NewPlayer);
		if (PC)
		{
			// 使用 RPC 通知客户端显示 UI
			PC->Client_ShowBeginWidget();
		}
	}
    
	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Green, "PostLogin completed");
}

bool AMy_demoGamemode::IsAllready() const
{
	int32 num = 0;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); 
		Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController)
		{
			// 安全转换
			AFPS_demoPlayerController* FPSController = Cast<AFPS_demoPlayerController>(PlayerController);
			if (FPSController && !FPSController->IsReady())
			{
				return false;
			}
		}
		num++;
	}
	
	if (num== 0)
		return false;
	return true;
}


void AMy_demoGamemode::StartCountdown()
{
	if (bIsCountingDown) return;
    
	bIsCountingDown = true;
	CountdownTime = 5.0f; // 重置为5秒
    
	if (HasAuthority())
	{
        
		// 通知所有客户端更新倒计时client RPC
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (PC)
			{
				if (AFPS_demoPlayerController* FPSController = Cast<AFPS_demoPlayerController>(PC))
				{
					
					FPSController->Client_UpdateCountdown(FMath::CeilToInt(CountdownTime));
				}
			}
		}
        
		GetWorldTimerManager().SetTimer(
			MatchStartTimerHandle,
			this,
			&AMy_demoGamemode::UpdateCountdown,
			1.0f,
			true
		);
	}
}

void AMy_demoGamemode::CancelCountdown()
{
	if (!bIsCountingDown) return;
    
	bIsCountingDown = false;
	GetWorldTimerManager().ClearTimer(MatchStartTimerHandle);
    
	UE_LOG(LogTemp, Log, TEXT("Countdown cancelled"));
    
	// 通知所有客户端取消倒计时
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC )
		{
			if (AFPS_demoPlayerController* FPSController = Cast<AFPS_demoPlayerController>(PC))
			{
				FPSController->Client_CancelCountdown();
			}
		}
	}
}


void AMy_demoGamemode::UpdateCountdown()
{
	if (!HasAuthority()) return; // 只在服务器执行
	CountdownTime -= 1.0f;
    
	if (CountdownTime <= 0)
	{
		// 倒计时结束，开始游戏
		GetWorldTimerManager().ClearTimer(MatchStartTimerHandle);
		bIsCountingDown = false;
        
		UE_LOG(LogTemp, Log, TEXT("Countdown finished, starting match"));
        
		// 通知客户端开始游戏
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (PC)
			{
				if (AFPS_demoPlayerController* FPSController = Cast<AFPS_demoPlayerController>(PC))
				{
					FPSController->Client_StartMatch();
				}
			}
		}
        
		// 调用本地的StartMatch
		StartMatch();
	}
	else
	{
		// 更新客户端倒计时显示
		int32 SecondsRemaining = FMath::CeilToInt(CountdownTime);
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (PC)
			{
				if (AFPS_demoPlayerController* FPSController = Cast<AFPS_demoPlayerController>(PC))
				{
					FPSController->Client_UpdateCountdown(SecondsRemaining);
				}
			}
		}
        
		UE_LOG(LogTemp, Log, TEXT("Countdown: %d"), SecondsRemaining);
	}
}

void AMy_demoGamemode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
    
	// 初始化比赛计时
	if (AMy_demoGameState* myGameState = GetGameState<AMy_demoGameState>())
	{
		myGameState->SetRemainingMatchTime(MatchDuration);
	}
    
	// 开始比赛计时
	StartMatchTimer();
    
	// 通知所有玩家显示Main_UI
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (AFPS_demoPlayerController* FPSController = Cast<AFPS_demoPlayerController>(PC))
			{
				// 设置玩家队伍
				if (AMy_demoGameState* MyGameState = GetGameState<AMy_demoGameState>())
				{
					int32 TeamIndex = MyGameState->IsTeamOne(PC) ? 0 : 1;
					FPSController->Client_SetPlayerTeam(TeamIndex);
					// 使用RPC通知客户端创建UI
					FPSController->Client_CreateAndShowMainUI(TeamIndex);
				}
				
				
			}
		}
	}
	
}

void AMy_demoGamemode::HandleMatchHasEnded()
{
	
	Super::HandleMatchHasEnded();
	// 计算获胜队伍
	if (AMy_demoGameState* myGameState = GetGameState<AMy_demoGameState>())
	{
		myGameState->CalculateWinningTeam();
	}
	
	FTimerHandle PauseTimerHandle;
	GetWorldTimerManager().SetTimer(PauseTimerHandle, [this]()
	{
		PauseGameTime();
	}, 1.0f, false);
	
	
    
	
}

void AMy_demoGamemode::StartMatchTimer()
{
	if (!HasAuthority()) return;
    
	// 每秒更新一次比赛时间
	GetWorld()->GetTimerManager().SetTimer(
		MatchTimerHandle,
		this,
		&AMy_demoGamemode::UpdateMatchTimer,
		1.0f,
		true
	);
    
	//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, 
		//TEXT("⏱️ 比赛计时器启动！"));
}



void AMy_demoGamemode::UpdateMatchTimer()
{
	if (!HasAuthority() || !bisGameStart) return;
    
	// 更新GameState中的剩余时间
	if (AMy_demoGameState* myGameState = GetGameState<AMy_demoGameState>())
	{
		float CurrentTime = myGameState->GetRemainingMatchTime();
		CurrentTime -= 1.0f;
        
		if (CurrentTime <= 0)
		{
			CurrentTime = 0;
			myGameState->SetRemainingMatchTime(CurrentTime);
			
			// 通知所有客户端更新时间
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				if (APlayerController* PC = It->Get())
				{
					if (AFPS_demoPlayerController* FPSController = Cast<AFPS_demoPlayerController>(PC))
					{
						FPSController->Client_UpdateMatchTime(CurrentTime);
					}
				}
			}
			
			EndMatch();
		}
		else
		{
			myGameState->SetRemainingMatchTime(CurrentTime);
            
			// 每1秒更新一次队伍分数和自己的分数
            static float ScoreUpdateTimer = 0.0f;
            ScoreUpdateTimer += 1.0f;
            if (ScoreUpdateTimer >= 1.0f)
            {
                ScoreUpdateTimer = 0.0f;
                
                
                // 通知所有客户端更新UI
                for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
                {
                    if (APlayerController* PC = It->Get())
                    {
                        if (AFPS_demoPlayerController* FPSController = Cast<AFPS_demoPlayerController>(PC))
                        {
                            // 传递计算好的分数
                            FPSController->Client_UpdateMyScores();
                        	
                        }
                    }
                }
            }
			
			// 通知所有客户端更新时间
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				if (APlayerController* PC = It->Get())
				{
					if (AFPS_demoPlayerController* FPSController = Cast<AFPS_demoPlayerController>(PC))
					{
						FPSController->Client_UpdateMatchTime(CurrentTime);
					}
				}
			}
		}
	}
}

void AMy_demoGamemode::CheckMatchTime()
{
	// 实时显示剩余时间
	static float DisplayTimer = 0.0f;
	DisplayTimer += GetWorld()->GetDeltaSeconds();
    
	if (DisplayTimer >= 1.0f)
	{
		DisplayTimer = 0.0f;
        
		if (AMy_demoGameState* myGameState = GetGameState<AMy_demoGameState>())
		{
			float RemainingTime = myGameState->GetRemainingMatchTime();
			int32 Minutes = FMath::FloorToInt(RemainingTime / 60.0f);
			int32 Seconds = FMath::FloorToInt(FMath::Fmod(RemainingTime, 60.0f));
            
			FString TimeString = FString::Printf(
				TEXT("⏱️ 剩余时间: %02d:%02d"),
				Minutes, Seconds
			);
            
			//GEngine->AddOnScreenDebugMessage(99, 1.1f, FColor::Yellow, TimeString);
		}
	}
}


void AMy_demoGamemode::EndMatch()
{
	Super::EndMatch();
	if (!bisGameStart) return;
	
	// 停止计时器
	GetWorld()->GetTimerManager().ClearTimer(MatchTimerHandle);
	
}


float AMy_demoGamemode::GetRemainingMatchTime() const
{
	if (AMy_demoGameState* myGameState = GetGameState<AMy_demoGameState>())
	{
		return myGameState->GetRemainingMatchTime();
	}
	return 0.0f;
}

void AMy_demoGamemode::PauseGameTime()
{
	// 暂停游戏时间
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (LocalPC)
	{
		// 暂停游戏
		LocalPC->SetPause(true);
		
	}
	GetWorld()->GetWorldSettings()->SetTimeDilation(0.0f);
}
