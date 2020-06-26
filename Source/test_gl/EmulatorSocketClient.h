// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CanData.h"
#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "Containers/Queue.h"
#include "UObject/WeakObjectPtrTemplates.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EmulatorSocketClient.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FEmulatorClientDisconnectDelegate, int32, ConnectionId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FEmulatorClientConnectDelegate, int32, ConnectionId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FEmulatorClientDataReceivedDelegate, int32, ConnectionId, UPARAM(ref) TArray<uint8>&, Message);


UCLASS()
class TEST_GL_API AEmulatorSocketClient : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEmulatorSocketClient();

		/* Returns the ID of the new connection. */
	UFUNCTION(BlueprintCallable, Category = "Emulator Client")
	void ConnectToEmulator(const FString& ipAddress, int32 port, 
		const FEmulatorClientDisconnectDelegate& OnDisconnected, const FEmulatorClientConnectDelegate& OnConnected,
		const FEmulatorClientDataReceivedDelegate& OnEmulatorDataReceived, int32& ConnectionId);

	/* Disconnect from connection ID. */
	UFUNCTION(BlueprintCallable, Category = "Emulator Client")
	void DisconnectFromEmulator(int32 ConnectionId);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ExecuteOnConnected(int32 WorkerId, TWeakObjectPtr<AEmulatorSocketClient> thisObj);

	void ExecuteOnDisconnected(int32 WorkerId, TWeakObjectPtr<AEmulatorSocketClient> thisObj);

	void ExecuteOnMessageReceived(int32 ConnectionId, TWeakObjectPtr<AEmulatorSocketClient> thisObj);


	/* Used by the separate threads to print to console on the main thread. */
	static void PrintToConsole(FString Str, bool Error);

	/* Buffer size in bytes. It's set only when creating a socket, never afterwards. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emulator Client")
	int32 ReceiveBufferSize = sizeof(EmulatorData101);

	/* Time between ticks. Please account for the fact that it takes 1ms to wake up on a modern PC, so 0.01f would effectively be 0.011f */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emulator Client")
	float TimeBetweenTicks = 0.008f;

private:
	TMap<int32, TSharedRef<class FTcpSocketWorker>> TcpWorkers;

	FEmulatorClientDisconnectDelegate DisconnectedDelegate;
	FEmulatorClientConnectDelegate ConnectedDelegate;
	FEmulatorClientDataReceivedDelegate MessageReceivedDelegate;

};

/** Thread Socket Class, developed by Spartan **/
class FTcpSocketWorker : public FRunnable, public TSharedFromThis<FTcpSocketWorker>
{

	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread = nullptr;

private:
	class FSocket* Socket;
	FString ipAddress;
	int port;
	TWeakObjectPtr<AEmulatorSocketClient> ThreadSpawnerActor;
	int32 id;
	int32 RecvBufferSize;
	int32 ActualRecvBufferSize;
	float TimeBetweenTicks;
	bool bConnected = false;	

	// SPSC = single producer, single consumer.
	TQueue<TArray<uint8>, EQueueMode::Spsc> Inbox; // Messages we read from socket and send to main thread. Runner thread is producer, main thread is consumer.

public:

	//Constructor / Destructor
	FTcpSocketWorker(FString inIp, const int32 inPort, TWeakObjectPtr<AEmulatorSocketClient> InOwner, int32 inId, int32 inRecvBufferSize, float inTimeBetweenTicks);
	virtual ~FTcpSocketWorker();

	/*  Starts processing of the connection. Needs to be called immediately after construction	 */
	void Start();

	/* Reads a message from the inbox queue */
	TArray<uint8> ReadFromInbox();

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

	void ConvertData(EmulatorData101 Data);

private:

	/** thread should continue running */
	FThreadSafeBool bRun = false;

	/** Critical section preventing multiple threads from sending simultaneously */
	FCriticalSection SendCriticalSection;
};
