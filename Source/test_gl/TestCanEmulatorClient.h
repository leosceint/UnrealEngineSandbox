// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/WeakObjectPtrTemplates.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestCanEmulatorClient.generated.h"

//Data Structs for Can Emulator

struct EmulatorData
{
	uint32 type;
	double chunk1;
	double chunk2;
	double chunk3;
	uint16 time;
};

UCLASS()
class TEST_GL_API ATestCanEmulatorClient : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATestCanEmulatorClient();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category="Emulator Parameters")
		FString EmulatorServerAddress;
	UPROPERTY(EditAnywhere, Category="Emulator Parameters")
		int32 EmulatorServerPort;
	
	UFUNCTION(BluePrintCallable, Category="LibraryTcpConnection")
		void ReceiveDataFromEmulator();

	FSocket* Socket;
	ISocketSubsystem* SocketSubsystem;
	bool bConnected;
};
