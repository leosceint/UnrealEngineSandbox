// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <bitset>
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TestCapture.generated.h"

/**
 * 
 */
UCLASS()
class TEST_GL_API UTestCapture : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BluePrintCallable, Category = "ImagePreparing")
		static bool ImageFileAsArray(FString ImageFile, TArray<uint8>& Image);

	UFUNCTION(BluePrintCallable, Category = "Image Preparing")
		static bool CaptureAsFile(class USceneCaptureComponent2D* CameraCapture);

	UFUNCTION(BluePrintCallable, Category = "Image Preparing")
		static bool CaptureAsArray(class USceneCaptureComponent2D* CameraCapture, TArray<uint8>& Image);
	
	UFUNCTION(BluePrintCallable, Category = "Image Preparing")
		static bool CaptureAsFileWithBarcode(class USceneCaptureComponent2D* CameraCapture, 
											int32 Code, int32 PixelsPerBit=20);//, const int32 NumberOfBits=32);

	UFUNCTION(BluePrintCallable, Category = "Image Preparing")
		static bool CaptureAsArrayWithBarcode(class USceneCaptureComponent2D* CameraCapture, TArray<uint8>& Image, 
											int32 Code, int32 PixelsPerBit=20);

private:
	template<size_t N>
	static std::bitset<N> to_bin(int32 n);

};
