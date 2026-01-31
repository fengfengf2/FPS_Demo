// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Begin_UI.generated.h"

class UEditableTextBox;
class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class FPS_DEMO_API UBegin_UI : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	UFUNCTION()
	void OnNameTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	void BindButtonEvents();

	
	// 更新倒计时显示
	UFUNCTION(BlueprintCallable, Category = "Countdown")
	void UpdateCountdownDisplay(int32 SecondsRemaining);
    
	// 取消倒计时
	UFUNCTION(BlueprintCallable, Category = "Countdown")
	void CancelCountdownDisplay();
	void OnMatchStarted();

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> NameTextBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Ready_Button;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ReadyText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CountdownText;
	UFUNCTION()
	void ReadyButtonClicked();
	void UpdateReadyStatus(bool bIsReady);
	
	// 倒计时相关
	FTimerHandle CountdownTimerHandle;
	int32 CurrentCountdown;
};
