// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LANMenu.generated.h"

class UButton;
class UEditableTextBox;
/**
 * 
 */
UCLASS()
class FPS_DEMO_API ULANMenu : public UUserWidget
{
	GENERATED_BODY()
public:
		virtual void NativeOnInitialized() override;//重载的控件初始函数
	
private:
	UPROPERTY(meta=(BindWidget)) 
	TObjectPtr<UEditableTextBox> TextBox_IpAddress;
	
	UPROPERTY(meta=(BindWidget)) 
	TObjectPtr<UButton>Button_Host;
	
	UPROPERTY(meta=(BindWidget)) 
	TObjectPtr<UButton>Button_Join;
	
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> HostingLevel;//联机大厅（或者真正的游戏）
	//点击响应函数
	UFUNCTION()
	void HostButtonClicked();
	
	UFUNCTION()
	void JoinButtonClicked();
};
