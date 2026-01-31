// Fill out your copyright notice in the Description page of Project Settings.


#include "LANMenu.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Kismet/GameplayStatics.h"

void ULANMenu::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	//umg加载时只能作为ui输入
	FInputModeUIOnly InputMode;
	GetOwningPlayer()->SetInputMode(InputMode);
	GetOwningPlayer()->SetShowMouseCursor(true);
	
	//绑定响应函数
	Button_Host->OnClicked.AddDynamic(this,&ThisClass::HostButtonClicked);
	Button_Join->OnClicked.AddDynamic(this,&ThisClass::JoinButtonClicked);
	
	
}

void ULANMenu::HostButtonClicked() 
{
	//输入改为游戏输入
	FInputModeGameOnly InputMode;
	GetOwningPlayer()->SetInputMode(InputMode);
	GetOwningPlayer()->SetShowMouseCursor(false);
	
	UGameplayStatics::OpenLevelBySoftObjectPtr(this,HostingLevel,true,TEXT("listen"));//"listen" 的特殊含义：这个选项告诉引擎，以 监听服务器模式 打开关卡。
}

void ULANMenu::JoinButtonClicked() 
{
	//输入改为游戏输入
	FInputModeGameOnly InputMode;
	GetOwningPlayer()->SetInputMode(InputMode);
	GetOwningPlayer()->SetShowMouseCursor(false);
	
	const FString Address = TextBox_IpAddress->GetText().ToString();
	
	UGameplayStatics::OpenLevel(this, *Address);
}
