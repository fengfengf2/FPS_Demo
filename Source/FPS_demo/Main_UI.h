// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Main_UI.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class FPS_DEMO_API UMain_UI : public UUserWidget
{
	GENERATED_BODY()
	
public:
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    
	// 更新剩余时间
	UFUNCTION(BlueprintCallable, Category = "Match")
	void UpdateRemainingTime(float RemainingSeconds);
	
	
	UFUNCTION(BlueprintCallable, Category = "Match")
	void UpdatePlayerScores(int32 Score);
    

    
	// 更新复活倒计时
	void UpdateRespawnCountdown(float RemainingTime);
	
	// 设置队伍
	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetTeam(int32 TeamIndex);

	// 血量更新函数
	UFUNCTION(BlueprintCallable, Category = "Health")
	void UpdateHealthBar(float CurrentHealth, float MaxHealth);
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimeText;
    
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;
	
	
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TeamText;//玩家所属于的队伍
	
	// 血量显示控件
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthProgressBar;
    
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;
    
	
	
private:
	bool bIsShowingDeathMessage = false;
	FLinearColor TeamOneColor = FLinearColor(0.2f, 0.2f, 0.8f, 1.0f); // 蓝色
	FLinearColor TeamTwoColor = FLinearColor(0.8f, 0.2f, 0.2f, 1.0f); // 红色
    
	// 格式化时间字符串
	FString FormatTimeString(float Seconds) const;
    
	// 格式化血量显示
	FString FormatHealthString(float CurrentHealth, float MaxHealth) const;
};
