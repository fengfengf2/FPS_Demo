// Fill out your copyright notice in the Description page of Project Settings.


#include "Main_UI.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"


void UMain_UI::NativeConstruct()
{
	Super::NativeConstruct();
	// 初始化血量显示
	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(1.0f);
	}
	if (HealthText)
	{
		HealthText->SetText(FText::FromString(TEXT("100/100")));
	}

    
	// 初始化文本
	if (TimeText)
	{
		TimeText->SetText(FText::FromString(TEXT("00:00")));
	}
    
	if (ScoreText)
	{
		ScoreText->SetText(FText::FromString(TEXT("0")));
	}
    
	
}


void UMain_UI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
    
	
}


void UMain_UI::UpdateRemainingTime(float RemainingSeconds)
{
	if (TimeText)
	{
		FString FormattedTime = FormatTimeString(RemainingSeconds);
		TimeText->SetText(FText::FromString(FormattedTime));
        
		// 根据剩余时间改变颜色
		if (RemainingSeconds <= 30.0f)
		{
			TimeText->SetColorAndOpacity(FLinearColor(1.0f, 0.5f, 0.0f, 1.0f)); // 橙色
		}
		if (RemainingSeconds <= 10.0f)
		{
			TimeText->SetColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f)); // 红色
            
			// 闪烁效果
			static float BlinkTimer = 0.0f;
			BlinkTimer += GetWorld()->GetDeltaSeconds();
			if (FMath::Sin(BlinkTimer * 10.0f) > 0)
			{
				TimeText->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				TimeText->SetVisibility(ESlateVisibility::Hidden);
			}
		}
		else
		{
			TimeText->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)); // 白色
			TimeText->SetVisibility(ESlateVisibility::Visible);
		}
	}
}



void UMain_UI::UpdatePlayerScores(int32 Score)
{
	ScoreText->SetText(FText::FromString(FString::FromInt(Score)));
	
}









void UMain_UI::SetTeam(int32 TeamIndex)
{
	if (TeamText)
	{
		if (TeamIndex==0)
		{
			TeamText->SetText(FText::FromString(TEXT("Team One")));
		}
		else if (TeamIndex==1)
		{
			TeamText->SetText(FText::FromString(TEXT("Team Two")));
		}
	}
}

FString UMain_UI::FormatTimeString(float Seconds) const
{
	int32 Minutes = FMath::FloorToInt(Seconds / 60.0f);
	int32 Secs = FMath::FloorToInt(FMath::Fmod(Seconds, 60.0f));
	return FString::Printf(TEXT("%02d:%02d"), Minutes, Secs);
}


void UMain_UI::UpdateHealthBar(float CurrentHealth, float MaxHealth)
{
	if (MaxHealth <= 0.0f) return;
	float HealthPercent = CurrentHealth / MaxHealth;
    
	// 更新进度条
	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(HealthPercent);
        
		// 根据血量改变颜色
		FLinearColor HealthColor;
		if (HealthPercent > 0.6f)
			HealthColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // 绿色
		else if (HealthPercent > 0.3f)
			HealthColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // 黄色
		else
			HealthColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // 红色
        
		HealthProgressBar->SetFillColorAndOpacity(HealthColor);
	}
    
	// 更新文本
	if (HealthText)
	{
		FString HealthString = FormatHealthString(CurrentHealth, MaxHealth);
		HealthText->SetText(FText::FromString(HealthString));
	}
}

FString UMain_UI::FormatHealthString(float CurrentHealth, float MaxHealth) const
{
	return FString::Printf(TEXT("%.0f/%.0f"), CurrentHealth, MaxHealth);
}
