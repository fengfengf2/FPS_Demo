// Fill out your copyright notice in the Description page of Project Settings.


#include "My_demoGameState.h"

#include "FPS_demoPlayerController.h"
#include "My_PlayerState.h"
#include "Net/UnrealNetwork.h"

void AMy_demoGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass,TeamOne);
	DOREPLIFETIME(ThisClass,TeamTwo);
	DOREPLIFETIME(AMy_demoGameState, RemainingMatchTime);
	DOREPLIFETIME(AMy_demoGameState, WinningTeam);
}

void AMy_demoGameState::AddTeamMember(APlayerController* PlayerController)
{
	//均匀分配
	if (TeamOne.Num()>TeamTwo.Num())
	{
		TeamTwo.Add(PlayerController);
	}
	else
	{
		TeamOne.Add(PlayerController);
	}
}

bool AMy_demoGameState::IsTeamOne(APlayerController* PlayerController) const
{
	return TeamOne.Contains(PlayerController);
}

void AMy_demoGameState::SetRemainingMatchTime(float NewTime)
{
	if (HasAuthority())
	{
		RemainingMatchTime = NewTime;
		OnRep_RemainingMatchTime(); // 手动触发复制事件
	}
}

void AMy_demoGameState::CalculateWinningTeam()
{
	if (bIsMatchFinished) return;
    
	int32 TeamOneScore = GetTeamOneScore();
	int32 TeamTwoScore = GetTeamTwoScore();
    
	if (TeamOneScore > TeamTwoScore)
	{
		WinningTeam = TEXT("Team One");
	}
	else if (TeamTwoScore > TeamOneScore)
	{
		WinningTeam = TEXT("Team Two");
	}
	else
	{
		WinningTeam = TEXT("Draw");
	}
    
	bIsMatchFinished = true;
	
    
	// 显示结束菜单
	ShowEndMenuToAllPlayers();
	
	OnRep_WinningTeam();
}


int32 AMy_demoGameState::GetTeamOneScore() const
{
	
    int32 TotalScore = 0;
    for (APlayerController* PC : TeamOne)
    {
        if (PC && PC->PlayerState)
        {
            if (AMy_PlayerState* PS = Cast<AMy_PlayerState>(PC->PlayerState))
            {
                TotalScore += PS->GetGameScore();
            }
        }
    }
    return TotalScore;
}

int32 AMy_demoGameState::GetTeamTwoScore() const
{
    int32 TotalScore = 0;
    for (APlayerController* PC : TeamTwo)
    {
        if (PC && PC->PlayerState)
        {
            if (AMy_PlayerState* PS = Cast<AMy_PlayerState>(PC->PlayerState))
            {
                TotalScore += PS->GetGameScore();
            }
        }
    }
    return TotalScore;
}

void AMy_demoGameState::OnRep_RemainingMatchTime()
{
    // 当剩余时间更新时，可以在这里触发UI更新等
    // 例如：通知所有玩家更新倒计时显示
}

void AMy_demoGameState::OnRep_WinningTeam()
{
	if (!HasAuthority())return ;//只让服务端输出debug
	/**
    // 当获胜队伍更新时，显示胜利消息
    FString WinMessage = FString::Printf(
        TEXT("🎉 比赛结束！获胜队伍: %s"),
        *WinningTeam
    );
    
    GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Yellow, 
        TEXT("========================================"));
    GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Yellow, WinMessage);
    
    // 显示队伍分数
    if (WinningTeam != TEXT("Draw"))
    {
        int32 WinningScore = (WinningTeam == TEXT("Team One")) ? 
            GetTeamOneScore() : GetTeamTwoScore();
        int32 LosingScore = (WinningTeam == TEXT("Team One")) ? 
            GetTeamTwoScore() : GetTeamOneScore();
            
        FString ScoreMessage = FString::Printf(
            TEXT("%s: %d 分 vs %s: %d 分"),
            *WinningTeam,
            WinningScore,
            (WinningTeam == TEXT("Team One") ? TEXT("Team Two") : TEXT("Team One")),
            LosingScore
        );
        GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Yellow, ScoreMessage);
    }
    else
    {
        FString ScoreMessage = FString::Printf(
            TEXT("平局！两队都是 %d 分"),
            GetTeamOneScore()
        );
        GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Yellow, ScoreMessage);
    }
    
    GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Yellow, 
        TEXT("========================================"));
        **/
}


void AMy_demoGameState::ShowEndMenuToAllPlayers()
{
	if (!HasAuthority()) return;
    
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			// 使用RPC通知客户端显示结束菜单
			if (AFPS_demoPlayerController* FPSController = Cast<AFPS_demoPlayerController>(PC))
			{
				FPSController->Client_ShowEndMenu(WinningTeam, GetTeamOneScore(), GetTeamTwoScore());
			}
		}
	}
}