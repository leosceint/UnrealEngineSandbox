// Fill out your copyright notice in the Description page of Project Settings.

#include "TestConnection.h"

// 
bool UTestConnection::TestConnection(FString ServerAddress, int ServerPort)
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get();
	TSharedRef<FInternetAddr> addr = SocketSubsystem->CreateInternetAddr();

	// 
	TCHAR* tServerAdress = ServerAddress.GetCharArray().GetData();
	bool bIp_IsValid;

	addr->SetIp(tServerAdress, bIp_IsValid);

	if (!bIp_IsValid)
	{
		return false;
	}
	addr->SetPort(ServerPort);

	// 
	FSocket* Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("TCP_TEST"), false);

	// 
	bool bConnected = Socket->Connect(*addr);
	Socket->Close();
	SocketSubsystem->DestroySocket(Socket);
	Socket = NULL;

	return bConnected;
}


// 
bool UTestConnection::SendMessage(FString ServerAddress, int ServerPort, FString Message)
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get();
	TSharedRef<FInternetAddr> addr = SocketSubsystem->CreateInternetAddr();

	// 
	int32 BufferSize = 4096;
	uint8* RecvBuffer = new uint8[BufferSize];

	// 
	bool bIp_IsValid;
	TCHAR* tServerAdress = ServerAddress.GetCharArray().GetData();

	addr->SetIp(tServerAdress, bIp_IsValid);

	if (!bIp_IsValid)
	{
		return false;
	}
	addr->SetPort(ServerPort);

	// 
	FSocket* Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("TCP_TEST"), false);

	// 
	bool bConnected = Socket->Connect(*addr);
	if (bConnected)
	{
		TCHAR* tMessage = Message.GetCharArray().GetData();
		int MessageSize = FCString::Strlen(tMessage);
		int Sent = 0;
		// 
		Socket->Send((uint8*)TCHAR_TO_UTF8(tMessage), MessageSize, Sent);
	}

	Socket->Close();
	SocketSubsystem->DestroySocket(Socket);
	Socket = NULL;

	return bConnected;
}

// 
bool UTestConnection::SendImage(FString ServerAddress, int ServerPort, TArray<uint8> Image,
	FString& Status)
{
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get();
	TSharedRef<FInternetAddr> addr = SocketSubsystem->CreateInternetAddr();

	// Задаем адрес и порт подключения
	TCHAR* tServerAdress = ServerAddress.GetCharArray().GetData();
	bool bIp_IsValid;

	addr->SetIp(tServerAdress, bIp_IsValid);

	if (!bIp_IsValid)
	{
		return false;
	}
	addr->SetPort(ServerPort);

	// Создаем сокет
	FSocket* Socket = SocketSubsystem->CreateSocket(NAME_Stream, TEXT("TCP_TEST"), false);

	bool bImgSent = false;
	// Проверяем подключение
	bool bConnected = Socket->Connect(*addr);
	if (bConnected)
	{
		int Sent = 0;
		// Отправляем команду
		const uint8* Command = (uint8*)TCHAR_TO_UTF8(TEXT("IMAGE___"));
		bool bCmd = Socket->Send(Command, 8, Sent);
		if(bCmd)
		{
			uint8* cImgSize = new uint8;
			UInt32ToUInt8(Image.Num(), cImgSize);
			Socket->Send(cImgSize, 4, Sent);
			delete cImgSize;

			// Отправляем картинку			
			bImgSent = Socket->Send(Image.GetData(), Image.Num(), Sent);

			// Оканчиваем передачу
			Command = (uint8*)TCHAR_TO_UTF8(TEXT("END_____"));
			Socket->Send(Command, 8, Sent);
		}
	}

	Socket->Close();
	SocketSubsystem->DestroySocket(Socket);
	Socket = NULL;

	return bImgSent;
}
