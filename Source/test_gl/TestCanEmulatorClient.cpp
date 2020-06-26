// Fill out your copyright notice in the Description page of Project Settings.


#include "TestCanEmulatorClient.h"

#include "Engine/Engine.h"

// Sets default values
ATestCanEmulatorClient::ATestCanEmulatorClient()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	EmulatorServerAddress = TEXT("127.0.0.1");
	EmulatorServerPort = 8888;
}

// Called when the game starts or when spawned
void ATestCanEmulatorClient::BeginPlay()
{
	Super::BeginPlay();
	
	SocketSubsystem = ISocketSubsystem::Get();
	TSharedRef<FInternetAddr> addr = SocketSubsystem->CreateInternetAddr();

	// Задаем адрес и порт подключения
	TCHAR* tServerAdress = EmulatorServerAddress.GetCharArray().GetData();
	bool bIp_IsValid;

	addr->SetIp(tServerAdress, bIp_IsValid);
	if(bIp_IsValid)
	{
		addr->SetPort(EmulatorServerPort);
		Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("TCP_TEST"), false);
		
		bConnected = Socket->Connect(*addr);
		
		if(bConnected)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString(TEXT("!!! SUCCESSFUL Connection to Emulator !!!")));
		}
			else
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(TEXT("!!! Not Connected TO Emulator!!!")));
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(TEXT("!!! NEED RESTART PROGRAM !!!")));
		}	
	}
}

void ATestCanEmulatorClient::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if(bConnected)
	{
		Socket->Close();
		SocketSubsystem->DestroySocket(Socket);
		Socket = nullptr;
		SocketSubsystem = nullptr;	
	}
}

// Called every frame
void ATestCanEmulatorClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATestCanEmulatorClient::ReceiveDataFromEmulator()
{
	if(bConnected)
	{
		int32 BytesRead = 0;
		EmulatorData ReceivedData;
		int32 BufferSize = sizeof(EmulatorData);

		bool bRecvResult = Socket->Recv((uint8*)&ReceivedData, BufferSize, BytesRead);
		if(bRecvResult && BytesRead == BufferSize)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Blue,
			FString::Printf(TEXT("!!! We get <type: %i> -- <chunk 1: %5.2f> -- <chunk 2: %5.2f> -- <chunk 3: %5.2f> -- <time: %i>!!!"),
			ReceivedData.type, ReceivedData.chunk1, ReceivedData.chunk2, ReceivedData.chunk3, ReceivedData.time));

			if(ReceivedData.type == 0x0101)
			{
				return;
			}
			if(ReceivedData.type == 0x0102)
			{
				return;
			}
			if(ReceivedData.type == 0x0103)
			{
				return;
			}
			if(ReceivedData.type == 0x0104)
			{
				return;
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(TEXT("!!! Data Lost!!!")));
		}
	}
	else
	{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(TEXT("!!! Not Connected TO Emulator!!!")));
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(TEXT("!!! NEED RESTART PROGRAM !!!")));
	}
}
