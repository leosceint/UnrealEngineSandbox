// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "CoreMinimal.h"
#include "Misc/FileHelper.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TestConnection.generated.h"

UCLASS()
class TEST_GL_API UTestConnection : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	// ƒл¤ тестировани¤ соединени¤
	UFUNCTION(BluePrintCallable, Category = "LibraryTcpConnection")
		static bool TestConnection(FString ServerAddress, int ServerPort);

	// ƒл¤ посылки сообщений
	UFUNCTION(BluePrintCallable, Category = "LibraryTcpConnection")
		static bool SendMessage(FString ServerAddress, int ServerPort, FString Message);
	// ƒл¤ посылки картинок
	UFUNCTION(BluePrintCallable, Category = "LibraryTcpConnection")
		static bool SendImage(FString ServerAddress, int ServerPort, TArray<uint8> Image, FString& Status);

};
