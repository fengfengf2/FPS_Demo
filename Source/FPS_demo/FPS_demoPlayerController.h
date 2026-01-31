// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPS_demoPlayerController.generated.h"

class UMain_UI;
class UEnd_Menu;
class UBegin_UI;
class UInputMappingContext;

/**
 *
 */

UCLASS()
class FPS_DEMO_API AFPS_demoPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	bool IsReady() const;
	
	// 服务器 RPC：通知客户端显示 UI
	UFUNCTION(Client, Reliable)
	void Client_ShowBeginWidget();
    
	// 服务器 RPC：设置准备状态
	UFUNCTION(Server, Reliable,WithValidation)
	void Server_SetReady(bool bReady);
	
	// RPC: 客户端通知服务器更新名字
	UFUNCTION(Server, Reliable)
	void Server_UpdatePlayerName(const FString& NewName);
	
	//PRC：展示endmenu
	UFUNCTION(Client, Reliable)
	void Client_ShowEndMenu(const FString& WinnerTeam, int32 TeamOneScore, int32 TeamTwoScore);
	//PRC：showMainUI
	UFUNCTION(Client, Reliable)
	void Client_CreateAndShowMainUI(int32 TeamIndex);
	
	// 倒计时相关
	UFUNCTION(Client, Reliable)
	void Client_UpdateCountdown(int32 SecondsRemaining);
    
	UFUNCTION(Client, Reliable)
	void Client_CancelCountdown();
    
	UFUNCTION(Client, Reliable)
	void Client_StartMatch();

	
	
	// Main_UI相关
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CreateMainUI();
    
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowMainUI();
    
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideMainUI();
    
	// 更新UI的RPC
	UFUNCTION(Client, Reliable)
	void Client_UpdateMatchTime(float RemainingTime);
    
	UFUNCTION(Client, Reliable)
	void Client_UpdateMyScores();
	
	
	
	UFUNCTION(Client, Reliable)
	void Client_SetPlayerTeam(int32 TeamIndex);

	
protected:

	/** Input Mapping Context to be used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;

	// Begin Actor interface
protected:

	virtual void BeginPlay() override;

	// End Actor interface
private:
	UPROPERTY(EditDefaultsOnly)//用来创建的类
	TSubclassOf<UBegin_UI> Begin_WidgetClass;
	
	UPROPERTY()//创建后的实例
	TObjectPtr<UBegin_UI> BeginWidgetInstance;
	
	
	UPROPERTY()
	UEnd_Menu* EndMenuInstance;
    
	// 结束菜单类
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UEnd_Menu> EndMenuWidgetClass;
	
	
	
	
	
    
	// Main_UI类
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UMain_UI> MainUIClass;
    
	// 定时器更新（MainUi）
	FTimerHandle UIUpdateTimerHandle;
	void StartUIUpdateTimer();
	void StopUIUpdateTimer();
	void UpdateUIFromGameState();
	
public:
	// Main_UI实例
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UMain_UI* MainUIInstance;

	
};
