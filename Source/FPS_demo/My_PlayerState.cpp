// Fill out your copyright notice in the Description page of Project Settings.


#include "My_PlayerState.h"

#include "FPS_demoPlayerController.h"
#include "Net/UnrealNetwork.h"


void AMy_PlayerState::Add_Score(int32 ScoreToAdd)
{
	SetGameScore(GetGameScore()+ScoreToAdd);
}

AMy_PlayerState::AMy_PlayerState()
{
	bReplicates = true;
	MyName = TEXT("Player"); 
}

void AMy_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, GameScore);
	DOREPLIFETIME(ThisClass,MyName);
	DOREPLIFETIME(ThisClass, bIs_Ready);
}



int32 AMy_PlayerState::GetGameScore() const
{
	return GameScore;
}

void AMy_PlayerState::SetGameScore(int32 NewScore)
{
	if (!HasAuthority())return;//不是服务器不能修改
	GameScore = NewScore;
}



void AMy_PlayerState::Server_SetScore_Implementation(int32 ScoreToSet)
{
	// 只有服务器执行这个函数
	if (HasAuthority())
	{
		GameScore = ScoreToSet;
		
	}
}




void AMy_PlayerState::SetReady(bool bReady)
{
	if (HasAuthority())
	{
		bIs_Ready = bReady;
		OnRep_Ready(); // 手动调用以通知客户端
	}
}

FString AMy_PlayerState::GetMyName() const
{
	return MyName;
}

void AMy_PlayerState::SetMyName(FString myname)
{
	MyName = myname;
}


void AMy_PlayerState::OnRep_Ready()
{
	OnReadyChanged.Broadcast(bIs_Ready);
	
}
bool AMy_PlayerState::Is_Ready() const
{
	return bIs_Ready;
}
