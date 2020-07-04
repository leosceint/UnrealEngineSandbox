// Fill out your copyright notice in the Description page of Project Settings.


#include "EmulatorSocketServer.h"

#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Common/TcpSocketBuilder.h"
#include "IPAddress.h"
#include "Sockets.h"
#include "HAL/RunnableThread.h"
#include "Async/Async.h"
#include <string>
#include "Logging/MessageLog.h"

// Sets default values
AEmulatorSocketServer::AEmulatorSocketServer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AEmulatorSocketServer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEmulatorSocketServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEmulatorSocketServer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopServer();
	Super::EndPlay(EndPlayReason);
}

void AEmulatorSocketServer::StartServer(const FString& ipAddress, int32 port, const FEmulatorServerDisconnectDelegate& OnDisconnected,
	const FEmulatorServerConnectDelegate& OnConnected, const FEmulatorServerDataReceivedDelegate& OnEmulatorDataReceived)
{
	DisconnectedDelegate = OnDisconnected;
	ConnectedDelegate = OnConnected;
	MessageReceivedDelegate = OnEmulatorDataReceived;

	TWeakObjectPtr<AEmulatorSocketServer> thisWeakObjPtr = TWeakObjectPtr<AEmulatorSocketServer>(this);
	ServerWorker = TUniquePtr<FServerWorker>(new FServerWorker(ipAddress, port, thisWeakObjPtr, ReceiveBufferSize, TimeBetweenTicks));
	ServerWorker->Start();
}

void AEmulatorSocketServer::StopServer()
{	

	if(ServerWorker)
	{
		UE_LOG(LogTemp, Log, TEXT("Server Stopped"));
		ServerWorker->Stop();
	}
}

void AEmulatorSocketServer::PrintToConsole(FString Str, bool Error)
{
	if (Error)
	{
		auto messageLog = FMessageLog("Socket Server for Emulator CAN");
		messageLog.Open(EMessageSeverity::Error, true);
		messageLog.Message(EMessageSeverity::Error, FText::AsCultureInvariant(Str));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Log: %s"), *Str);
	}
}

void AEmulatorSocketServer::ExecuteOnConnected(TWeakObjectPtr<AEmulatorSocketServer> thisObj)
{
	if (!thisObj.IsValid())
		return;

	ConnectedDelegate.ExecuteIfBound();
}

void AEmulatorSocketServer::ExecuteOnDisconnected(TWeakObjectPtr<AEmulatorSocketServer> thisObj)
{
	if (!thisObj.IsValid())
		return;

	DisconnectedDelegate.ExecuteIfBound();
}

void AEmulatorSocketServer::ExecuteOnMessageReceived(TWeakObjectPtr<AEmulatorSocketServer> thisObj)
{

	// the second check is for when we quit PIE, we may get a message about a disconnect, but it's too late to act on it, because the thread has already been killed
	if (!thisObj.IsValid())
		return;	
		
	// how to crash:
	// 1 connect with both clients
	// 2 stop PIE
	// 3 close editor
	if (!ServerWorker->isConnected()) {
		return;
	}

	EmulatorData DataPacket = ServerWorker->ReadFromInbox();
	
	if(DataPacket.type == expectedId)
	{
		switch(DataPacket.type)
		{
			case 0x0101: 
				CanData.lat = DataPacket.part1; 
				CanData.lon = DataPacket.part2;
				break;
			case 0x0102:
				CanData.height = DataPacket.part1;
				CanData.course_angle = DataPacket.part2;
				CanData.pitch = DataPacket.part3; 
				break;
			case 0x0103:
				CanData.course = DataPacket.part1;
				CanData.roll = DataPacket.part2;
				CanData.peleng = DataPacket.part3;
				break;
			case 0x0104: 
				CanData.angleV = DataPacket.part1;
				CanData.angleH = DataPacket.part2;
				break;
		}
		++expectedId;	
	}
	else
	{
		expectedId = 0x0101;
	}
	if(expectedId > 0x0104)
	{
		MessageReceivedDelegate.ExecuteIfBound(CanData);
		expectedId = 0x0101;
	}
}

bool FServerWorker::isConnected()
{
	FScopeLock ScopeLock(&SendCriticalSection);
	return bConnected;
}

FServerWorker::FServerWorker(FString inIp, const int32 inPort, TWeakObjectPtr<AEmulatorSocketServer> InOwner, int32 inRecvBufferSize, float inTimeBetweenTicks)
	: ipAddress(inIp)
	, port(inPort)
	, ThreadSpawnerActor(InOwner)
	, RecvBufferSize(inRecvBufferSize)
	, TimeBetweenTicks(inTimeBetweenTicks)
{
	
}

FServerWorker::~FServerWorker()
{
	AsyncTask(ENamedThreads::GameThread, []() {	AEmulatorSocketServer::PrintToConsole("Tcp socket thread was destroyed.", false); });
	Stop();
	if (Thread)
	{
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}
}

void FServerWorker::Start()
{
	check(!Thread && "Thread wasn't null at the start!");
	check(FPlatformProcess::SupportsMultithreading() && "This platform doesn't support multithreading!");	
	if (Thread)
	{
		UE_LOG(LogTemp, Log, TEXT("Log: Thread isn't null. It's: %s"), *Thread->GetThreadName());
	}
	Thread = FRunnableThread::Create(this, *FString::Printf(TEXT("FServerWorker %s:%d"), *ipAddress, port), 128 * 1024, TPri_Normal);
	UE_LOG(LogTemp, Log, TEXT("Log: Created thread"));
}

EmulatorData FServerWorker::ReadFromInbox()
{
	EmulatorData Data;
	Inbox.Dequeue(Data);
	return Data;
}

bool FServerWorker::Init()
{
	bRun = true;
	bConnected = false;
	return true;
}

uint32 FServerWorker::Run()
{
	FIPv4Address ip;
	FIPv4Address::Parse(ipAddress, ip);
	FIPv4Endpoint Endpoint(ip, port);

	ListenSocket = FTcpSocketBuilder(TEXT("Listener"))
	.AsReusable()
	.BoundToEndpoint(Endpoint)
	.WithReceiveBufferSize(RecvBufferSize);
	if(!ListenSocket)
	{
		return 0;
	}

	AsyncTask(ENamedThreads::GameThread, []() {	AEmulatorSocketServer::PrintToConsole("Starting Tcp socket server thread.", false); });

	ListenSocket->SetReceiveBufferSize(RecvBufferSize, ActualRecvBufferSize);
	ListenSocket->Listen(8);

	while (bRun)
	{
		FDateTime timeBeginningOfTick = FDateTime::UtcNow();

		if(bShouldListen)
		{
			bool bHasPendingConnection;
			ListenSocket->HasPendingConnection(bHasPendingConnection);
			if(bHasPendingConnection)
			{
				TSharedPtr<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
				ConnectionSocket = ListenSocket->Accept(*Addr, TEXT("Connection"));
				
				if(!ConnectionSocket)
				{
					AsyncTask(ENamedThreads::GameThread, []() { AEmulatorSocketServer::PrintToConsole(FString::Printf(TEXT("Connection Socket is null. EmulatorSocketServer.cpp: line %d"), __LINE__), true); });
                    bRun = false;
                    continue;
				}
				bShouldListen = false;
				bConnected = true;
			}
		}
		if(bConnected)
		{
			// check if we weren't disconnected from the socket
			ConnectionSocket->SetNonBlocking(true); // set to NonBlocking, because Blocking can't check for a disconnect for some reason
			int32 t_BytesRead;
			uint8 t_Dummy;
			if (!ConnectionSocket->Recv(&t_Dummy, 1, t_BytesRead, ESocketReceiveFlags::Peek))
			{
				bRun = false;
				continue;
			}
			ConnectionSocket->SetNonBlocking(false);	// set back to Blocking

			// read bytes as emulator data struct (see CanData.h)
			EmulatorData receivedData;
			int32 BytesRead = 0;
			if(!ConnectionSocket->Recv((uint8*)&receivedData, sizeof(EmulatorData), BytesRead))
			{
				AsyncTask(ENamedThreads::GameThread, []() {
				AEmulatorSocketServer::PrintToConsole(FString::Printf(TEXT("In progress read failed. EmulatorSocketServer.cpp: line %d"), __LINE__), true);
				});
				continue;
			}
			

			Inbox.Enqueue(receivedData);
			AsyncTask(ENamedThreads::GameThread, [this]() {
				UE_LOG(LogTemp, Log, TEXT("RECEIVED PACKET"));
				ThreadSpawnerActor.Get()->ExecuteOnMessageReceived(ThreadSpawnerActor);
			});	
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

	bConnected = false;
	bShouldListen = true;
	AsyncTask(ENamedThreads::GameThread, [this]() {
		ThreadSpawnerActor.Get()->ExecuteOnDisconnected(ThreadSpawnerActor);
	});

	SocketShutdown();
	
	return 0;
}

void FServerWorker::Stop()
{
	bRun = false;
}

void FServerWorker::Exit() 
{
	
}

void FServerWorker::SocketShutdown()
{
	// if there is still a socket, close it so our peer will get a quick disconnect notification
	if(bConnected)
	{
		ConnectionSocket->Close();
	}
	
	if (ListenSocket)
	{
		ListenSocket->Close();
	}
}
