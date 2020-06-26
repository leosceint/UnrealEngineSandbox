// Fill out your copyright notice in the Description page of Project Settings.


#include "TestServerActor.h"

#include "Engine/Engine.h"

// Sets default values
ATestServerActor::ATestServerActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATestServerActor::BeginPlay()
{
	Super::BeginPlay();
}

void ATestServerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(ConnectionSocketTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ListenerSocketTimerHandle);

	if(ConnectionSocket != nullptr)
	{
		ConnectionSocket->Close();
	}
	if(ListenerSocket != nullptr)
	{
		ListenerSocket->Close();
	}
}

// Called every frame
void ATestServerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ATestServerActor::StartTCPReceiver(const int32 Port)
{
	ListenerSocket = CreateTCPConnectionListener(Port);
	if(!ListenerSocket)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("StartTCPReceiver>> Listen socket could not be created! ~> %s %d"));
		return false;
	}
	GetWorld()->GetTimerManager().SetTimer(ListenerSocketTimerHandle, this, &ATestServerActor::TCPConnectionListener, 1.0f, true);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("StartTCPReceiver>> Listen socket created")));
	return true;
}

FSocket* ATestServerActor::CreateTCPConnectionListener(const int32 Port, int32 ReceiveBufferSize)
{
	FSocket* ListenSocket;
	FIPv4Address Address;
	FIPv4Address::Parse(TEXT("0.0.0.0"), Address);

	FIPv4Endpoint Endpoint(Address, Port);
	ListenSocket = FTcpSocketBuilder(TEXT("<LISTENER>"))
					.AsReusable()
					.BoundToEndpoint(Endpoint)
					.WithReceiveBufferSize(ReceiveBufferSize)
					.Listening(1);
	ListenSocket->SetReceiveBufferSize(ReceiveBufferSize, ReceiveBufferSize);
	ListenSocket->SetSendBufferSize(ReceiveBufferSize, ReceiveBufferSize);
	return ListenSocket;
}

void ATestServerActor::TCPConnectionListener()
{
	if(!ListenerSocket) return;

	TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	bool Pending;
	ListenerSocket->HasPendingConnection(Pending);
	if(Pending)
	{
		if (ConnectionSocket)
		{
			ConnectionSocket->Close();
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
		}
		ConnectionSocket = ListenerSocket->Accept(*RemoteAddress, TEXT("> CONNECTOR <"));

		if(ConnectionSocket != nullptr)
		{
			FIPv4Endpoint RemoteAddressForConnection = FIPv4Endpoint(RemoteAddress);
			GetWorld()->GetTimerManager().SetTimer(ConnectionSocketTimerHandle, this, &ATestServerActor::TCPSocketListener, 0.1f, true);
		}

	}
}

void ATestServerActor::TCPSocketListener()
{

}

// BluePrint

bool ATestServerActor::LaunchTCPServer(int32 Port)
{
	if(!StartTCPReceiver(Port))
	{
		return false;
	}
	return true;
}

// BluePrint

void ATestServerActor::SendJSON(TArray<uint8> JSON_data)
{

}

// BluePrint

void ATestServerActor::ReceivedMessage(const FString& message)
{

}
