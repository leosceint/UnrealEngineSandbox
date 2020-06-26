// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Components/ActorComponent.h"
#include "Networking.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestServerActor.generated.h"

UCLASS()
class TEST_GL_API ATestServerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestServerActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//Timer functions
	void TCPConnectionListener();
	void TCPSocketListener();

	bool StartTCPReceiver(const int32 Port);
	FSocket* CreateTCPConnectionListener(const int32 Port, int32 ReceiveBufferSize = 2*1024*1024);

	UFUNCTION(BlueprintCallable, Category = "TCP_Server")
		bool LaunchTCPServer(int32 Port);
	
	UFUNCTION(BlueprintCallable, Category = "TCP_Server")
		void SendJSON(TArray<uint8> JSON_data);
	
	UFUNCTION(BlueprintCallable, Category = "TCP_Server")
		void ReceivedMessage(const FString& message);

private:
	FSocket* ConnectionSocket; 	//Само подключение
	FSocket* ListenerSocket; 	// Слушает подключения
	FTimerHandle ConnectionSocketTimerHandle;
	FTimerHandle ListenerSocketTimerHandle;
	bool bConnected;

};
