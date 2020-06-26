// Fill out your copyright notice in the Description page of Project Settings.


#include "TestCapture.h"
#include "ImageUtils.h"
#include "misc/FileHelper.h"
#include "misc/Paths.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

bool UTestCapture::ImageFileAsArray(FString ImageFile, TArray<uint8>& Image)
{
	return FFileHelper::LoadFileToArray(Image, *(FPaths::ProjectDir() + ImageFile));
}

bool UTestCapture::CaptureAsFile(class USceneCaptureComponent2D* CameraCapture)
{
	if ((CameraCapture == nullptr) || (CameraCapture->TextureTarget == nullptr))
		return false;
	FRenderTarget* RenderTarget = CameraCapture->TextureTarget->GameThread_GetRenderTargetResource();
	FRenderTarget* RenderTarget_test = CameraCapture->TextureTarget->GetRenderTargetResource();
	if (RenderTarget == nullptr)
		return false;

	TArray<FColor> RawPixels;

	if (CameraCapture->TextureTarget->GetFormat() != PF_B8G8R8A8)
		return false;

	CameraCapture->UpdateContent();

	if (!RenderTarget->ReadPixels(RawPixels))
		return false;

	const int32 Width = CameraCapture->TextureTarget->SizeX;
	const int32 Height = CameraCapture->TextureTarget->SizeY;

	for (auto& Pixel : RawPixels)
	{
		// Switch Red/Blue changes.
		const uint8 PR = Pixel.R;
		const uint8 PB = Pixel.B;
		Pixel.R = PB;
		Pixel.B = PR;
		Pixel.A = 255;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> PngImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (PngImageWrapper.IsValid() && PngImageWrapper->SetRaw(&RawPixels[0],
		RawPixels.Num() * sizeof(FColor), Width, Height, ERGBFormat::RGBA, 8))
	{
		TArray<uint8> result = PngImageWrapper->GetCompressed();
		FString FileName = "OutputResult.png";
		FFileHelper::SaveArrayToFile(result, *(FPaths::ProjectDir() + FileName));
		return true;
	}

	return false;
}

bool UTestCapture::CaptureAsArray(class USceneCaptureComponent2D* CameraCapture, TArray<uint8>& Image)
{
	if ((CameraCapture == nullptr) || (CameraCapture->TextureTarget == nullptr))
		return false;
	FRenderTarget* RenderTarget = CameraCapture->TextureTarget->GetRenderTargetResource();
	if (RenderTarget == nullptr)
		return false;

	TArray<FColor> RawPixels;

	if (CameraCapture->TextureTarget->GetFormat() != PF_B8G8R8A8)
		return false;

	CameraCapture->UpdateContent();

	if (!RenderTarget->ReadPixels(RawPixels))
		return false;

	const int32 Width = CameraCapture->TextureTarget->SizeX;
	const int32 Height = CameraCapture->TextureTarget->SizeY;

	for (auto& Pixel : RawPixels)
	{
		// Switch Red/Blue changes.
		const uint8 PR = Pixel.R;
		const uint8 PB = Pixel.B;
		Pixel.R = PB;
		Pixel.B = PR;
		Pixel.A = 255;
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> PngImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (PngImageWrapper.IsValid() && PngImageWrapper->SetRaw(&RawPixels[0],
		RawPixels.Num() * sizeof(FColor), Width, Height, ERGBFormat::RGBA, 8))
	{
		Image = PngImageWrapper->GetCompressed();
		return true;
	}

	return false;
}

bool UTestCapture::CaptureAsFileWithBarcode(class USceneCaptureComponent2D* CameraCapture,
 											int32 Code, int32 PixelsPerBit)//, const int32 NumberOfBits)
{
		if ((CameraCapture == nullptr) || (CameraCapture->TextureTarget == nullptr))
		return false;
	FRenderTarget* RenderTarget = CameraCapture->TextureTarget->GameThread_GetRenderTargetResource();
	if (RenderTarget == nullptr)
		return false;

	TArray<FColor> RawPixels;
	const int32 size = 32;
	std::bitset<size> Barcode = to_bin<size>(Code);

	if (CameraCapture->TextureTarget->GetFormat() != PF_B8G8R8A8)
		return false;

	CameraCapture->UpdateContent();

	if (!RenderTarget->ReadPixels(RawPixels))
		return false;

	const int32 Width = CameraCapture->TextureTarget->SizeX;
	const int32 Height = CameraCapture->TextureTarget->SizeY;

	for (int32 index = 0; index < RawPixels.Num(); ++index)//(auto& Pixel : RawPixels)
	{
		// Switch Red/Blue changes.
		const uint8 PR = RawPixels[index].R;
		const uint8 PB = RawPixels[index].B;
		if(index < RawPixels.Num() - Width)
		{
			RawPixels[index].R = PB;
			RawPixels[index].B = PR;
			RawPixels[index].A = 255;
			continue;
		}
		for(size_t b_index = 0; b_index < Barcode.size(); ++b_index)
		{
			uint8 color = 0;
			if(Barcode[b_index] == 1) color = 255;
			int32 i =  0;
			while(i < PixelsPerBit)
			{
				RawPixels[index].R = color;
				RawPixels[index].G = color;
				RawPixels[index].B = color;
				RawPixels[index].A = 255;
				++index;
				++i;
			}			
		}

	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> PngImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (PngImageWrapper.IsValid() && PngImageWrapper->SetRaw(&RawPixels[0],
		RawPixels.Num() * sizeof(FColor), Width, Height, ERGBFormat::RGBA, 8))
	{
		TArray<uint8> result = PngImageWrapper->GetCompressed();
		FString FileName = "OutputResultBarcode.png";
		FFileHelper::SaveArrayToFile(result, *(FPaths::ProjectDir() + FileName));
		return true;
	}

	return false;
}

bool UTestCapture::CaptureAsArrayWithBarcode(class USceneCaptureComponent2D* CameraCapture, TArray<uint8>& Image, 
												int32 Code, int32 PixelsPerBit)
{
	if ((CameraCapture == nullptr) || (CameraCapture->TextureTarget == nullptr))
		return false;
	FRenderTarget* RenderTarget = CameraCapture->TextureTarget->GameThread_GetRenderTargetResource();
	//UCanvasRenderTarget2D* CanvasRenderTarget = CameraCapture->TextureTarget->GameThread_GetRenderTargetResource();
	
	if (RenderTarget == nullptr)
		return false;

	TArray<FColor> RawPixels;
	const int32 size = 32;
	std::bitset<size> Barcode = to_bin<size>(Code);

	if (CameraCapture->TextureTarget->GetFormat() != PF_B8G8R8A8)
		return false;

	CameraCapture->UpdateContent();

	if (!RenderTarget->ReadPixels(RawPixels))
		return false;

	const int32 Width = CameraCapture->TextureTarget->SizeX;
	const int32 Height = CameraCapture->TextureTarget->SizeY;

	for (int32 index = 0; index < RawPixels.Num(); ++index)
	{
		// Switch Red/Blue changes.
		const uint8 PR = RawPixels[index].R;
		const uint8 PB = RawPixels[index].B;
		if(index < RawPixels.Num() - Width)
		{
			RawPixels[index].R = PB;
			RawPixels[index].B = PR;
			RawPixels[index].A = 255;
			continue;
		}
		for(size_t b_index = 0; b_index < Barcode.size(); ++b_index)
		{
			uint8 color = 0;
			if(Barcode[b_index] == 1) color = 255;
			int32 i =  0;
			while(i < PixelsPerBit)
			{
				RawPixels[index].R = color;
				RawPixels[index].G = color;
				RawPixels[index].B = color;
				RawPixels[index].A = 255;
				++index;
				++i;
			}			
		}

	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> PngImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (PngImageWrapper.IsValid() && PngImageWrapper->SetRaw(&RawPixels[0],
		RawPixels.Num() * sizeof(FColor), Width, Height, ERGBFormat::RGBA, 8))
	{
		Image = PngImageWrapper->GetCompressed();
		return true;
	}

	return false;
}

template<size_t N>
std::bitset<N> UTestCapture::to_bin(int32 n)
{
	std::bitset<N> bits(n);
    size_t bits_size = bits.size();
    
	//revert
    for(std::size_t i=0; i < bits_size/2; i++)
    {
        bool t = bits[i];
        bits[i] = bits[bits_size-i-1];
        bits[bits_size-i-1] = t;
    }
    return bits;
}
