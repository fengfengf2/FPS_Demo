// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "My_demoGameState.generated.h"

/**
 * 
 */
UCLASS()
class FPS_DEMO_API AMy_demoGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void AddTeamMember(APlayerController* PlayerController);
	bool IsTeamOne(APlayerController* PlayerController) const;

	
	// 比赛计时相关
	UFUNCTION(BlueprintPure, Category = "Match")
	float GetRemainingMatchTime() const { return RemainingMatchTime; }
    
	UFUNCTION(BlueprintPure, Category = "Match")
	FString GetWinningTeam() const { return WinningTeam; }
    
	// 设置剩余时间（只在服务器调用）
	void SetRemainingMatchTime(float NewTime);
    
	// 计算并设置获胜队伍
	void CalculateWinningTeam();
    
	// 获取队伍分数
	int32 GetTeamOneScore() const;
	int32 GetTeamTwoScore() const;

	
	// 显示结束菜单给所有玩家
	void ShowEndMenuToAllPlayers();
    

	
	
protected:
	// 当比赛时间复制时调用
	UFUNCTION()
	void OnRep_RemainingMatchTime();
    
	// 当获胜队伍复制时调用
	UFUNCTION()
	void OnRep_WinningTeam();
	
	
private:
	
	
	// 比赛剩余时间（秒）
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_RemainingMatchTime)
	float RemainingMatchTime = 0.0f;
    
	// 获胜队伍名称
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_WinningTeam)
	FString WinningTeam = TEXT("None");
    
	// 比赛是否结束
	bool bIsMatchFinished = false;
	
	UPROPERTY(replicated)
	TArray<APlayerController*> TeamOne;
	UPROPERTY(replicated)
	TArray<APlayerController*> TeamTwo;
};
