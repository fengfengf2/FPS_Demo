// Copyright Epic Games, Inc. All Rights Reserved.


#include "FPS_demoPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "My_PlayerState.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "Begin_UI.h"
#include "End_Menu.h"
#include "Main_UI.h"
#include "My_demoGameState.h"

bool AFPS_demoPlayerController::IsReady() const
{
	if (AMy_PlayerState* PS = GetPlayerState<AMy_PlayerState>())
	{
		return PS->Is_Ready();
	}
	return false;
}

void AFPS_demoPlayerController::Client_ShowBeginWidget_Implementation()
{
	// 只在客户端执行
	if (!IsLocalController())
		return;
	
	if (Begin_WidgetClass)
	{
		
		BeginWidgetInstance = CreateWidget<UBegin_UI>(this, Begin_WidgetClass);
		if (BeginWidgetInstance)
		{
			BeginWidgetInstance->SetOwningPlayer(this);
			BeginWidgetInstance->AddToViewport();
		}
	}
}
void AFPS_demoPlayerController::Server_SetReady_Implementation(bool bReady)
{
	if (AMy_PlayerState* PS = GetPlayerState<AMy_PlayerState>())
	{
		PS->SetReady(bReady);
	}
}

bool AFPS_demoPlayerController::Server_SetReady_Validate(bool bReady)
{
	return true; // 简单的验证
}

void AFPS_demoPlayerController::Server_UpdatePlayerName_Implementation(const FString& NewName)
{
	if (AMy_PlayerState* PS = GetPlayerState<AMy_PlayerState>())
	{
		PS->SetMyName(NewName);
	}
}



void AFPS_demoPlayerController::BeginPlay()
{
	Super::BeginPlay();
	// 检查是否是本地玩家控制器
	if (!IsLocalController())
	{
		return;
	}
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("无法获取本地玩家"));
		return;
	}
	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}
}


void AFPS_demoPlayerController::Client_UpdateCountdown_Implementation(int32 SecondsRemaining)
{

	if (IsLocalController())
	{
		if (BeginWidgetInstance)
		{
			BeginWidgetInstance->UpdateCountdownDisplay(SecondsRemaining);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Countdown: %d"), SecondsRemaining);
}

void AFPS_demoPlayerController::Client_CancelCountdown_Implementation()
{
	if (IsLocalController())
	{
		UE_LOG(LogTemp, Log, TEXT("Countdown cancelled"));
		if (BeginWidgetInstance)
		{
			BeginWidgetInstance->CancelCountdownDisplay();
		}
	}
}

void AFPS_demoPlayerController::Client_StartMatch_Implementation()
{
	if (IsLocalController())
	{
		if (BeginWidgetInstance)
		{
			BeginWidgetInstance->OnMatchStarted();
		}
	}
}


void AFPS_demoPlayerController::Client_ShowEndMenu_Implementation(const FString& WinnerTeam, int32 TeamOneScore, int32 TeamTwoScore)
{
	// 只在客户端执行
	if (!IsLocalController())
		return;
    
	// 创建结束菜单
	if (EndMenuWidgetClass && !EndMenuInstance)
	{
		EndMenuInstance = CreateWidget<UEnd_Menu>(this, EndMenuWidgetClass);
		if (EndMenuInstance)
		{
			EndMenuInstance->SetOwningPlayer(this);
			EndMenuInstance->AddToViewport(100); // 设置较高的z-order
			EndMenuInstance->SetWinnerInfo(WinnerTeam, TeamOneScore,TeamTwoScore);
			
		}
	}
}



void AFPS_demoPlayerController::CreateMainUI()
{
	if (!IsLocalController()) return;
    
	if (MainUIClass && !MainUIInstance)
	{
		MainUIInstance = CreateWidget<UMain_UI>(this, MainUIClass);
		if (MainUIInstance)
		{
			MainUIInstance->SetOwningPlayer(this);
			MainUIInstance->AddToViewport(5);
            
			// 初始隐藏，等待比赛开始
			MainUIInstance->SetVisibility(ESlateVisibility::Hidden);
			
		}
	}
}

void AFPS_demoPlayerController::ShowMainUI()
{
	if (!IsLocalController() || !MainUIInstance) return;
    
	MainUIInstance->SetVisibility(ESlateVisibility::Visible);
	// 开始UI更新定时器
	StartUIUpdateTimer();
}

void AFPS_demoPlayerController::HideMainUI()
{
	if (!IsLocalController() || !MainUIInstance) return;
    
	MainUIInstance->SetVisibility(ESlateVisibility::Hidden);
    
	// 停止UI更新定时器
	StopUIUpdateTimer();
}

void AFPS_demoPlayerController::StartUIUpdateTimer()
{
    // 每0.2秒更新一次UI，平衡性能和响应性
    GetWorldTimerManager().SetTimer(
        UIUpdateTimerHandle,
        this,
        &AFPS_demoPlayerController::UpdateUIFromGameState,
        0.2f, // 0.2秒更新一次
        true  // 循环
    );
}

void AFPS_demoPlayerController::StopUIUpdateTimer()
{
    GetWorldTimerManager().ClearTimer(UIUpdateTimerHandle);
}

void AFPS_demoPlayerController::UpdateUIFromGameState()
{
    if (!IsLocalController() || !MainUIInstance) return;
    
    // 获取GameState
    AMy_demoGameState* MyGameState = GetWorld()->GetGameState<AMy_demoGameState>();
    if (!MyGameState) return;
    
    // 更新剩余时间
    float RemainingTime = MyGameState->GetRemainingMatchTime();
    MainUIInstance->UpdateRemainingTime(RemainingTime);
	
}

// RPC实现（均交给服务器实现）
void AFPS_demoPlayerController::Client_UpdateMatchTime_Implementation(float RemainingTime)
{
    if (!IsLocalController() || !MainUIInstance) return;
    
    MainUIInstance->UpdateRemainingTime(RemainingTime);
}

void AFPS_demoPlayerController::Client_UpdateMyScores_Implementation()
{
    if (!IsLocalController() || !MainUIInstance) return;
	
	//顺便吧自己的分数更新一下
	MainUIInstance->UpdatePlayerScores(GetPlayerState<AMy_PlayerState>()->GetGameScore());
	
}




void AFPS_demoPlayerController::Client_SetPlayerTeam_Implementation(int32 TeamIndex)
{
    if (!IsLocalController() || !MainUIInstance) return;
    
    MainUIInstance->SetTeam(TeamIndex);
}


void AFPS_demoPlayerController::Client_CreateAndShowMainUI_Implementation(int32 TeamIndex)
{
	// 只在客户端执行
	if (!IsLocalController()) return;
    
	CreateMainUI();
	ShowMainUI();
	Client_SetPlayerTeam(TeamIndex);
}




