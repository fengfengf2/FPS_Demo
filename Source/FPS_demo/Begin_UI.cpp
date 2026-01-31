// Fill out your copyright notice in the Description page of Project Settings.


#include "Begin_UI.h"

#include "FPS_demoPlayerController.h"
#include "My_PlayerState.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void UBegin_UI::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	//umg加载时只能作为ui输入
	FInputModeUIOnly InputMode;
	GetOwningPlayer()->SetInputMode(InputMode);
	GetOwningPlayer()->SetShowMouseCursor(true);
	
	//延迟绑定按钮
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		BindButtonEvents();
	}, 0.1f, false);
}

void UBegin_UI::NativeDestruct()
{
	// 清理定时器
	GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
    
	Super::NativeDestruct();
}




void UBegin_UI::BindButtonEvents()
{
	if (Ready_Button)
	{
		// 清除旧绑定
		Ready_Button->OnClicked.Clear();
		// 绑定新事件
		Ready_Button->OnClicked.AddDynamic(this, &UBegin_UI::ReadyButtonClicked);
	}
    
	if (NameTextBox)
	{
		// 添加文本提交事件（回车或失去焦点时提交到服务器）
		NameTextBox->OnTextCommitted.Clear();
		NameTextBox->OnTextCommitted.AddDynamic(this, &UBegin_UI::OnNameTextCommitted);
	}
}
void UBegin_UI::ReadyButtonClicked()
{
	// 获取 PlayerController
	AFPS_demoPlayerController* PC = Cast<AFPS_demoPlayerController>(GetOwningPlayer());
	if (PC)
	{
		// 切换准备状态
		AMy_PlayerState* PS = PC->GetPlayerState<AMy_PlayerState>();
		if (PS)
		{
			bool bCurrentReady = PS->Is_Ready();
			PC->Server_SetReady(!bCurrentReady);
			UpdateReadyStatus(!bCurrentReady);
		}
	}
}

// 更新UI显示
void UBegin_UI::UpdateReadyStatus(bool bIsReady)
{
	if (Ready_Button && ReadyText)
	{
		if (bIsReady)
		{
			ReadyText->SetText(FText::FromString("Ready"));
			Ready_Button->SetBackgroundColor(FLinearColor::Green);
		}
		else
		{
			ReadyText->SetText(FText::FromString("Not Ready"));
			Ready_Button->SetBackgroundColor(FLinearColor::Gray);
		}
	}
}



void UBegin_UI::UpdateCountdownDisplay(int32 SecondsRemaining)
{
	if (CountdownText)
	{
		if (SecondsRemaining > 0)
		{
			//显示倒计时
			CountdownText->SetText(FText::FromString(FString::Printf(TEXT("Starting in %d..."), SecondsRemaining)));
			CountdownText->SetVisibility(ESlateVisibility::Visible);
            
			//动画效果
			CountdownText->SetRenderScale(FVector2D(1.2f, 1.2f));
            
			FTimerHandle ScaleTimer;
			GetWorld()->GetTimerManager().SetTimer(ScaleTimer, [this]()
			{
				CountdownText->SetRenderScale(FVector2D(1.0f, 1.0f));
			}, 0.1f, false);
            
			// 更新当前倒计时
			CurrentCountdown = SecondsRemaining;
		}
		else
		{
			// 倒计时结束，显示"GO!"
			CountdownText->SetText(FText::FromString("GO!"));
            
			// 2秒后隐藏
			FTimerHandle HideTimer;
			GetWorld()->GetTimerManager().SetTimer(HideTimer, [this]()
			{
				CountdownText->SetVisibility(ESlateVisibility::Collapsed);
			}, 2.0f, false);
		}
	}
}
void UBegin_UI::CancelCountdownDisplay()
{
	if (CountdownText){
			CountdownText->SetVisibility(ESlateVisibility::Collapsed);
			CountdownText->SetText(FText::FromString(""));
	}
	
	// 清除倒计时
	CurrentCountdown = 0;
	GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
}
void UBegin_UI::OnMatchStarted()
{
	UE_LOG(LogTemp, Log, TEXT("Match started, hiding Begin_UI"));
    
	// 游戏开始，隐藏准备UI
	SetVisibility(ESlateVisibility::Collapsed);
    
	// 恢复游戏输入模式
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
	}
	
}





void UBegin_UI::OnNameTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// 只有当用户按下回车或失去焦点时才提交到服务器
	if (CommitMethod == ETextCommit::OnEnter || CommitMethod == ETextCommit::OnUserMovedFocus)
	{
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Blue, "name committed to server");
		
		// 提交到服务器
		AFPS_demoPlayerController* PC = Cast<AFPS_demoPlayerController>(GetOwningPlayer());
		if (PC)
		{
			FString NewName = Text.ToString().TrimStartAndEnd();
			
			// 如果名称为空，恢复默认
			if (NewName.IsEmpty())
			{
				NewName = FString::Printf(TEXT("Player_%d"), FMath::RandRange(1000, 9999));
				if (NameTextBox)
				{
					NameTextBox->SetText(FText::FromString(NewName));
				}
			}
			
			// 调用服务器RPC函数
			PC->Server_UpdatePlayerName(NewName);
		}
	}
}
