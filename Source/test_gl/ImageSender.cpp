// Fill out your copyright notice in the Description page of Project Settings.


#include "ImageSender.h"

#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "IPAddress.h"
#include "Sockets.h"
#include "HAL/RunnableThread.h"
#include "Async/Async.h"
#include <string>
#include "Logging/MessageLog.h"

// Sets default values
AImageSender::AImageSender()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AImageSender::BeginPlay()
{
	Super::BeginPlay();
	
}

void AImageSender::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TArray<int32> keys;
	ImageSenderWorkers.GetKeys(keys);

	for(auto &key : keys)
	{
		Disconnect(key);
	}

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AImageSender::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AImageSender::ConnectToNeuroServer(const FString& ipAddress, int32 port, const FImageSenderDisconnectDelegate& OnDisconnected,
	const FImageSenderConnectDelegate& OnConnected, int32& ConnectionId)
{
	DisconnectedDelegate = OnDisconnected;
	ConnectedDelegate = OnConnected;

	ConnectionId = ImageSenderWorkers.Num();

	TWeakObjectPtr<AImageSender> thisWeakObjPtr = TWeakObjectPtr<AImageSender>(this);
	TSharedRef<FImageSenderWorker> worker(new FImageSenderWorker(ipAddress, port, thisWeakObjPtr, ConnectionId, SendBufferSize, TimeBetweenTicks));
	ImageSenderWorkers.Add(ConnectionId, worker);
	worker->Start();
}

void AImageSender::Disconnect(int32 ConnectionId)
{
	auto worker = ImageSenderWorkers.Find(ConnectionId);
	if (worker)
	{
		UE_LOG(LogTemp, Log, TEXT("Tcp Socket: Disconnected from server."));
		worker->Get().Stop();
		ImageSenderWorkers.Remove(ConnectionId);
	}
}

bool AImageSender::SendImage(int32 ConnectionId, TArray<uint8> ImageToSend)
{
	if(ImageSenderWorkers.Contains(ConnectionId))
	{
		if(ImageSenderWorkers[ConnectionId]->isConnected())
		{
			ImageSenderWorkers[ConnectionId]->AddToOutBox(ImageToSend);
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Log: Client %d isn't connected"), ConnectionId);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Log: ClientId %d doesn't exist"), ConnectionId);
	}
	return false;
}

void AImageSender::ExecuteOnConnected(int32 WorkerId, TWeakObjectPtr<AImageSender> thisObj)
{
	if (!thisObj.IsValid())
		return;

	ConnectedDelegate.ExecuteIfBound(WorkerId);
}

void AImageSender::ExecuteOnDisconnected(int32 WorkerId, TWeakObjectPtr<AImageSender> thisObj)
{
	if (!thisObj.IsValid())
		return;

	if (ImageSenderWorkers.Contains(WorkerId))
	{		
		ImageSenderWorkers.Remove(WorkerId);		
	}
	DisconnectedDelegate.ExecuteIfBound(WorkerId);
}

void AImageSender::PrintToConsole(FString Str, bool Error)
{
	if (Error)
	{
		auto messageLog = FMessageLog("<-- Image Sender -->");
		messageLog.Open(EMessageSeverity::Error, true);
		messageLog.Message(EMessageSeverity::Error, FText::AsCultureInvariant(Str));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Log: %s"), *Str);
	}
}

// Worker

FImageSenderWorker::FImageSenderWorker(FString inIp, const int32 inPort, TWeakObjectPtr<AImageSender> InOwner, int32 inId, int32 inSendBufferSize, float inTimeBetweenTicks)
	: ipAddress(inIp)
	, port(inPort)
	, ThreadSpawnerActor(InOwner)
	, id(inId)
	, SendBufferSize(inSendBufferSize)
	, TimeBetweenTicks(inTimeBetweenTicks)
{

}

FImageSenderWorker::~FImageSenderWorker()
{
	AsyncTask(ENamedThreads::GameThread, []() {	
		UE_LOG(LogTemp, Log, TEXT("Client thread was destroyed.")); 
		});
	Stop();

	if (Thread)
	{
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}
}

bool FImageSenderWorker::isConnected()
{
	FScopeLock ScopeLock(&SendCriticalSection);
	return bConnected;
}

void FImageSenderWorker::Start()
{
	check(!Thread && "Thread wasn't null at the start!");
	check(FPlatformProcess::SupportsMultithreading() && "This platform doesn't support multithreading!");	
	if (Thread)
	{
		UE_LOG(LogTemp, Log, TEXT("Log: Thread isn't null. It's: %s"), *Thread->GetThreadName());
	}
	Thread = FRunnableThread::Create(this, *FString::Printf(TEXT("ClientWorker %s:%d"), *ipAddress, port), 128 * 1024, TPri_Normal);
	UE_LOG(LogTemp, Log, TEXT("Log: Created thread"));
}

void FImageSenderWorker::AddToOutBox(TArray<uint8> Image)
{
	Outbox.Enqueue(Image);
}

bool FImageSenderWorker::Init()
{
	bRun = true;
	bConnected = false;
	return true;
}

uint32 FImageSenderWorker::Run()
{
	AsyncTask(ENamedThreads::GameThread, []() {	
	UE_LOG(LogTemp, Log, TEXT("START Client Socket thread")); 
	});

	while (bRun)
	{
		FDateTime timeBeginningOfTick = FDateTime::UtcNow();

		// Connect
		if (!bConnected)
		{
			ClientSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);
			if (!ClientSocket)
			{
				return 0;
			}

			ClientSocket->SetSendBufferSize(SendBufferSize, ActualSendBufferSize);

			FIPv4Address ip;
			FIPv4Address::Parse(ipAddress, ip);

			TSharedRef<FInternetAddr> internetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
			internetAddr->SetIp(ip.Value);
			internetAddr->SetPort(port);

			bConnected = ClientSocket->Connect(*internetAddr);
			if (bConnected) 
			{
				AsyncTask(ENamedThreads::GameThread, [this]() {
					ThreadSpawnerActor.Get()->ExecuteOnConnected(id, ThreadSpawnerActor);
				});
			}
			else 
			{
				AsyncTask(ENamedThreads::GameThread, []() { 
					AImageSender::PrintToConsole(FString::Printf(TEXT("Couldn't connect to server. ImageSender.cpp: line %d"), __LINE__), true); 
				});
				bRun = false;				
			}
			continue;
		}

		if (!ClientSocket)
		{
			AsyncTask(ENamedThreads::GameThread, []() { 
				AImageSender::PrintToConsole(FString::Printf(TEXT("Socket is null. ImageSender.cpp: line %d"), __LINE__), true); 
			});
			bRun = false;
			continue;
		}

		// check if we weren't disconnected from the socket
		ClientSocket->SetNonBlocking(true); // set to NonBlocking, because Blocking can't check for a disconnect for some reason
		int32 t_BytesRead;
		uint8 t_Dummy;
		if (!ClientSocket->Recv(&t_Dummy, 1, t_BytesRead, ESocketReceiveFlags::Peek))
		{
			bRun = false;
			continue;
		}
		ClientSocket->SetNonBlocking(false);	// set back to Blocking

		// if Outbox has Image to send, send it
		while (!Outbox.IsEmpty())
		{
			ClientSocket->SetNonBlocking(true);
			TArray<uint8> ImageToSend; 
			Outbox.Dequeue(ImageToSend);
			
			// Send Command
			const uint8* Command = (uint8*)TCHAR_TO_UTF8(TEXT("IMAGE___"));

			if (!BlockingSend(Command, 8))
			{
				// if sending failed, stop running the thread
				bRun = false;
				continue;
			}

			// Send Image size
			uint32 ImageSize = (uint32)ImageToSend.Num();

			if (!BlockingSend((uint8*)&ImageSize, sizeof(uint32)))
			{
				// if sending failed, stop running the thread
				bRun = false;
				continue;
			}

			// Send Image
			if (!BlockingSend(ImageToSend.GetData(), ImageToSend.Num()))
			{
				// if sending failed, stop running the thread
				bRun = false;
				continue;
			}
			ClientSocket->SetNonBlocking(false);
		}

		/* In order to sleep, we will account for how much this tick took due to sending and receiving */
		FDateTime timeEndOfTick = FDateTime::UtcNow();
		FTimespan tickDuration = timeEndOfTick - timeBeginningOfTick;
		float secondsThisTickTook = tickDuration.GetTotalSeconds();
		float timeToSleep = TimeBetweenTicks - secondsThisTickTook;
		if (timeToSleep > 0.f)
		{
			FPlatformProcess::Sleep(timeToSleep);
		}
	}

	// close session
	
	if(bConnected)
	{
		const uint8* Command = (uint8*)TCHAR_TO_UTF8(TEXT("END_____"));
		BlockingSend(Command, 8);
	}

	bConnected = false;

	AsyncTask(ENamedThreads::GameThread, [this]() {
		ThreadSpawnerActor.Get()->ExecuteOnDisconnected(id, ThreadSpawnerActor);
	});

	SocketShutdown();
	
	return 0;
}

void FImageSenderWorker::Stop()
{
	bRun = false;
}

void FImageSenderWorker::Exit() 
{
	
}

bool FImageSenderWorker::BlockingSend(const uint8* Data, int32 BytesToSend)
{
	if (BytesToSend > 0)
	{
		int32 BytesSent = 0;
		if (!ClientSocket->Send(Data, BytesToSend, BytesSent))
		{
			return false;
		}
	}
	return true;
}

void FImageSenderWorker::SocketShutdown()
{
	// if there is still a socket, close it so our peer will get a quick disconnect notification
	if (ClientSocket)
	{
		ClientSocket->Close();
	}
}
