// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "My_PlayerState.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyChanged,bool,bReady);
UCLASS()
class FPS_DEMO_API AMy_PlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void Add_Score(int32 ScoreToAdd);// 只能在服务器用
	AMy_PlayerState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	int32 GetGameScore() const;
	void SetGameScore(int32 NewScore);
	// 客户端请求增加分数的RPC
	UFUNCTION(Server, Reliable)
	void Server_SetScore(int32 ScoreToSet);
	void SetReady(bool bReady);
	UFUNCTION(BlueprintCallable)
	FString GetMyName() const;
	void SetMyName(FString myname);
	bool Is_Ready() const;
protected:
	UFUNCTION()
	void OnRep_Ready();
private:
	UPROPERTY(replicated, EditAnywhere)
	int32 GameScore = 0;
	UPROPERTY(replicated, EditAnywhere)
	FString MyName;
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_Ready)
	bool bIs_Ready =false;
	
public:
	
	UPROPERTY(BlueprintAssignable)
	FOnReadyChanged OnReadyChanged;
};
