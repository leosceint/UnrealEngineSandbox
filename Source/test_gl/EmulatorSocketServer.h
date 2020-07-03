// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CanData.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "Containers/Queue.h"
#include "UObject/WeakObjectPtrTemplates.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EmulatorSocketServer.generated.h"

DECLARE_DYNAMIC_DELEGATE(FEmulatorServerDisconnectDelegate);
DECLARE_DYNAMIC_DELEGATE(FEmulatorServerConnectDelegate);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEmulatorServerDataReceivedDelegate, UPARAM(ref) FCanData, DataFromCan);


UCLASS()
class TEST_GL_API AEmulatorSocketServer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEmulatorSocketServer();

	/* Returns the ID of the new connection. */
	UFUNCTION(BlueprintCallable, Category = "Emulator Server")
	void StartServer(const FString& ipAddress, int32 port, 
		const FEmulatorServerDisconnectDelegate& OnDisconnected, const FEmulatorServerConnectDelegate& OnConnected,
		const FEmulatorServerDataReceivedDelegate& OnEmulatorDataReceived);

	/* Disconnect from connection ID. */
	UFUNCTION(BlueprintCallable, Category = "Emulator Server")
	void StopServer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ExecuteOnConnected(TWeakObjectPtr<AEmulatorSocketServer> thisObj);

	void ExecuteOnDisconnected(TWeakObjectPtr<AEmulatorSocketServer> thisObj);

	void ExecuteOnMessageReceived(TWeakObjectPtr<AEmulatorSocketServer> thisObj);


	/* Used by the separate threads to print to console on the main thread. */
	static void PrintToConsole(FString Str, bool Error);

	/* Buffer size in bytes. It's set only when creating a socket, never afterwards. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emulator Server")
	int32 ReceiveBufferSize = sizeof(EmulatorData);

	/* Time between ticks. Please account for the fact that it takes 1ms to wake up on a modern PC, so 0.01f would effectively be 0.011f */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emulator Server")
	float TimeBetweenTicks = 0.008f;

private:
	TUniquePtr<class FServerWorker> ServerWorker;
	FEmulatorServerDisconnectDelegate DisconnectedDelegate;
	FEmulatorServerConnectDelegate ConnectedDelegate;
	FEmulatorServerDataReceivedDelegate MessageReceivedDelegate;

};

/** Thread Socket Class, based on FSocketWorker, developed by Spartan **/
class FServerWorker : public FRunnable, public TSharedFromThis<FServerWorker>
{

	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread = nullptr;

private:
	class FSocket* ListenSocket;
	class FSocket* ConnectionSocket;
	FString ipAddress;
	int port;
	TWeakObjectPtr<AEmulatorSocketServer> ThreadSpawnerActor;
	int32 RecvBufferSize;
	int32 ActualRecvBufferSize;
	float TimeBetweenTicks;
	bool bConnected 	= false;	
	bool bShouldListen 	= true;
	// SPSC = single producer, single consumer.
	TQueue<EmulatorData, EQueueMode::Spsc> Inbox; // Messages we read from socket and send to main thread. Runner thread is producer, main thread is consumer.

public:

	//Constructor / Destructor
	FServerWorker(FString inIp, const int32 inPort, TWeakObjectPtr<AEmulatorSocketServer> InOwner, int32 inRecvBufferSize, float inTimeBetweenTicks);
	virtual ~FServerWorker();

	/*  Starts processing of the connection. Needs to be called immediately after construction	 */
	void Start();

	/* Reads a message from the inbox queue */
	EmulatorData ReadFromInbox();

	// Begin FRunnable interface.
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	// End FRunnable interface	

	/** Shuts down the thread */
	void SocketShutdown();

	/* Getter for bConnected */
	bool isConnected();

private:

	/** thread should continue running */
	FThreadSafeBool bRun = false;

	/** Critical section preventing multiple threads from sending simultaneously */
	FCriticalSection SendCriticalSection;
};
