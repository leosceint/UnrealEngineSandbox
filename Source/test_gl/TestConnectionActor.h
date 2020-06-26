// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/WeakObjectPtrTemplates.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestConnectionActor.generated.h"

UCLASS()
class TEST_GL_API ATestConnectionActor : public AActor
{
	GENERATED_BODY()
	
public:	
	DECLARE_DYNAMIC_DELEGATE(FReceivedImageDelegate);
	// Sets default values for this actor's properties
	ATestConnectionActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category="Server Parameters")
		FString ServerAddress;
	UPROPERTY(EditAnywhere, Category="Server Parameters")
		int ServerPort;

	UFUNCTION(BluePrintCallable, Category = "LibraryTcpConnection")
		void SendImageWithEvent(TArray<uint8> Image, const FReceivedImageDelegate& OnImageReceived);

private:
	FReceivedImageDelegate ImageReceivedDelegate;
	FSocket* Socket;
	ISocketSubsystem* SocketSubsystem;
	bool bConnected;
};
