// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "Containers/Queue.h"
#include "UObject/WeakObjectPtrTemplates.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ImageSender.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FImageSenderDisconnectDelegate, int32, ConnectionId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FImageSenderConnectDelegate, int32, ConnctionId);

UCLASS()
class TEST_GL_API AImageSender : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AImageSender();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BluePrintCallable, Category = "Client to Send Images")
	void ConnectToNeuroServer(const FString& ipAddress, int32 port, 
	const FImageSenderDisconnectDelegate& OnDisconnected, const FImageSenderConnectDelegate& OnConnected, 
	int32& ConnectionId);

	UFUNCTION(BlueprintCallable, Category = "Client to Send Images")
	void Disconnect(int32 ConnectionId);

	UFUNCTION(BlueprintCallable, Category = "Client to Send Images")
	bool SendImage(int32 ConnectionId, TArray<uint8> ImageToSend);

	void ExecuteOnConnected(int32 WorkerId, TWeakObjectPtr<AImageSender> thisObj);

	void ExecuteOnDisconnected(int32 WorkerId, TWeakObjectPtr<AImageSender> thisObj);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Client to Send Images")
	int32 SendBufferSize = 16384;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Client to Send Images")
	float TimeBetweenTicks = 0.02f; // 50 Hz

	static void PrintToConsole(FString Str, bool Error);

private:
	TMap<int32, TSharedRef<class FImageSenderWorker>> ImageSenderWorkers;

	FImageSenderDisconnectDelegate DisconnectedDelegate;
	FImageSenderConnectDelegate ConnectedDelegate;	

};

class FImageSenderWorker: public FRunnable, public TSharedFromThis<FImageSenderWorker>
{
	FRunnableThread* Thread = nullptr;

private:
	class FSocket* ClientSocket;
	FString ipAddress;
	int32 port;
	TWeakObjectPtr<AImageSender> ThreadSpawnerActor;
	int32 id;
	int32 SendBufferSize;
	int32 ActualSendBufferSize;
	float TimeBetweenTicks;
	bool bConnected = false;

	TQueue<TArray<uint8>, EQueueMode::Spsc> Outbox;

public:
	FImageSenderWorker(FString inIp, const int32 inPort, TWeakObjectPtr<AImageSender> InOwner,
		int32 inId, int32 inSendBufferSize, float inTimeBetweenTicks);
	virtual ~FImageSenderWorker();

	void Start();
	void AddToOutBox(TArray<uint8> Image);

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
	/* Blocking send */
	bool BlockingSend(const uint8* Data, int32 BytesToSend);

	/** thread should continue running */
	FThreadSafeBool bRun = false;

	/** Critical section preventing multiple threads from sending simultaneously */
	FCriticalSection SendCriticalSection;

};
