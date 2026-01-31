// Fill out your copyright notice in the Description page of Project Settings.


#include "End_Menu.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

void UEnd_Menu::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	//umg加载时只能作为ui输入
	FInputModeUIOnly InputMode;
	GetOwningPlayer()->SetInputMode(InputMode);
	GetOwningPlayer()->SetShowMouseCursor(true);
	
	Quit_Button->OnClicked.AddDynamic(this, &UEnd_Menu::QuitButtonClicked);
}

void UEnd_Menu::QuitButtonClicked()
{
	UWorld* World = GetWorld();
	if (!World) return;
    
	// 获取玩家控制器
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController) return;
    
	// 退出游戏
	UKismetSystemLibrary::QuitGame(
		World,
		PlayerController,
		EQuitPreference::Quit,  
		false  
	);
}
void UEnd_Menu::SetWinnerInfo(const FString& Winner, int32 TeamOneScore, int32 TeamTwoScore)
{
	if (WinnerText)
	{
		if (Winner == TEXT("Draw"))
		{
			WinnerText->SetText(FText::FromString(TEXT("🏆 平局！")));
		}
		else
		{
			FString WinnerStr = FString::Printf(TEXT("🏆 获胜队伍: %s"), *Winner);
			WinnerText->SetText(FText::FromString(WinnerStr));
		}
	}
    
	if (ScoreText)
	{
		FString ScoreStr = FString::Printf(
			TEXT("队伍一: %d 分\n队伍二: %d 分"),
			TeamOneScore, TeamTwoScore
		);
		ScoreText->SetText(FText::FromString(ScoreStr));
	}
}
