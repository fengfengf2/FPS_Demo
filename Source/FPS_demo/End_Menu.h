// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "End_Menu.generated.h"

class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class FPS_DEMO_API UEnd_Menu : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;
	
	// 设置获胜者信息
	void SetWinnerInfo(const FString& Winner, int32 TeamOneScore, int32 TeamTwoScore);
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> Quit_Button;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> WinnerText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreText;
	UFUNCTION()
	void QuitButtonClicked();
};
