// Fill out your copyright notice in the Description page of Project Settings.


#include "TestConnectionActor.h"

#include "Engine/Engine.h"


// Sets default values
ATestConnectionActor::ATestConnectionActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	ServerAddress = TEXT("127.0.0.1");
	ServerPort = 9999;
}

// Called when the game starts or when spawned
void ATestConnectionActor::BeginPlay()
{
	Super::BeginPlay();
	SocketSubsystem = ISocketSubsystem::Get();
	TSharedRef<FInternetAddr> addr = SocketSubsystem->CreateInternetAddr();

	// Задаем адрес и порт подключения
	TCHAR* tServerAdress = ServerAddress.GetCharArray().GetData();
	bool bIp_IsValid;

	addr->SetIp(tServerAdress, bIp_IsValid);
	if(bIp_IsValid)
	{
		addr->SetPort(ServerPort);
		Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("TCP_TEST"), false);
		
		bConnected = Socket->Connect(*addr);
		
		if(bConnected)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, FString(TEXT("!!! SUCCESSFUL Connection !!!")));
		}
			else
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(TEXT("!!! Not Connected TO Server!!!")));
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(TEXT("!!! NEED RESTART PROGRAM !!!")));
		}	
	}

}

void ATestConnectionActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (bConnected)
	{
		int Sent = 0;
		const uint8* Command = (uint8*)TCHAR_TO_UTF8(TEXT("END_____"));
		Socket->Send(Command, 8, Sent);
		Socket->Close();
		SocketSubsystem->DestroySocket(Socket);
		Socket = nullptr;
		SocketSubsystem = nullptr;		
	}
}

// Called every frame
void ATestConnectionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATestConnectionActor::SendImageWithEvent(TArray<uint8> Image, const FReceivedImageDelegate& OnImageReceived)
{
	if(bConnected)
	{
		int Sent = 0;
		// Отправляем команду
		const uint8* Command = (uint8*)TCHAR_TO_UTF8(TEXT("IMAGE___"));
		bool bCmd = Socket->Send(Command, 8, Sent);
		if(bCmd)
		{
		
			uint32 ImageSize = (uint32)Image.Num();
			Socket->Send((uint8*)&ImageSize, sizeof(uint32), Sent);			
			// Отправляем картинку			
			bool bImgSent = Socket->Send(Image.GetData(), Image.Num(), Sent);
			if(bImgSent)
			{
				ImageReceivedDelegate = OnImageReceived;
				ImageReceivedDelegate.ExecuteIfBound();
			}
		}	
	}	
	else
	{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(TEXT("!!! Not Connected TO Server!!!")));
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(TEXT("!!! NEED RESTART PROGRAM !!!")));
	}

}
