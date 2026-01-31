// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "My_demoGamemode.generated.h"

/**
 * 
 */

UCLASS()
class FPS_DEMO_API AMy_demoGamemode : public AGameMode
{
	GENERATED_BODY()
public:
	AMy_demoGamemode();
	
	// 重写GameMode的MatchState相关函数
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
    
	// 获取比赛剩余时间
	UFUNCTION(BlueprintPure, Category = "Match")
	float GetRemainingMatchTime() const;
	void PauseGameTime();

	UPROPERTY(BlueprintReadOnly, Category = "Match Settings", meta = (ClampMin = "30", ClampMax = "3600"))
	float MatchDuration = 300.0f; // 5分钟
	
	virtual void EndMatch() override;
	
private:
	bool bisGameStart;
	// 倒计时相关
	FTimerHandle MatchStartTimerHandle;
	float CountdownTime;
	bool bIsCountingDown;
	
	// 准备检查
	bool IsAllready() const;
    
	// 开始倒计时
	void StartCountdown();
    
	// 取消倒计时
	void CancelCountdown();
    
	// 倒计时更新
	void UpdateCountdown();
    
	

	// 比赛计时器句柄
	FTimerHandle MatchTimerHandle;
	
	void StartMatchTimer();
	void UpdateMatchTimer();
	
	void CheckMatchTime();
    

protected:
	
	virtual void StartMatch() override;
	
	virtual void BeginPlay() override;
	
	virtual void Tick( float DeltaTime ) override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	

};
