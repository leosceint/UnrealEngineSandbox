// Fill out your copyright notice in the Description page of Project Settings.


#include "ImageCapture.h"

#include "HAL/RunnableThread.h"
#include "Async/Async.h"
#include "Logging/MessageLog.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

// Sets default values
AImageCapture::AImageCapture()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AImageCapture::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AImageCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AImageCapture::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if(CaptureWorker)
	{
		UE_LOG(LogTemp, Log, TEXT("Capture Stopped"));
		CaptureWorker->Stop();
	}

	Super::EndPlay(EndPlayReason);
}

void AImageCapture::CaptureImage(class USceneCaptureComponent2D* CameraCapture, const FImageCapturedDelegate& OnImageCaptured)
{
	ImageCapturedDelegate = OnImageCaptured;
	
	TWeakObjectPtr<AImageCapture> thisWeakObjPtr = TWeakObjectPtr<AImageCapture>(this);
	CaptureWorker = TUniquePtr<FCaptureWorker>(new FCaptureWorker(thisWeakObjPtr, TimeBetweenTicks, CameraCapture));
	CaptureWorker->Start();
}

void AImageCapture::ExecuteOnImageCaptured(TWeakObjectPtr<AImageCapture> thisObj)
{
	if(!thisObj.IsValid())
 		return;

	TArray<uint8> Image = CaptureWorker->ReadFromInbox();
	ImageCapturedDelegate.ExecuteIfBound(Image);
}

FCaptureWorker::FCaptureWorker(TWeakObjectPtr<AImageCapture> InOwner, float inTimeBetweenTicks, class USceneCaptureComponent2D* Camera)
:ThreadSpawnerActor(InOwner)
,TimeBetweenTicks(inTimeBetweenTicks)
,CameraCapture(Camera)
{
	RenderTarget = CameraCapture->TextureTarget->GameThread_GetRenderTargetResource();
}

FCaptureWorker::~FCaptureWorker()
{
	Stop();
	if(CaptureThread)
	{
		CaptureThread->WaitForCompletion();
		delete CaptureThread;
		CaptureThread = nullptr;
	}
}

void FCaptureWorker::Start()
{
	check(!CaptureThread && "Thread wasn't null at the start!");
	check(FPlatformProcess::SupportsMultithreading() && "This platform doesn't support multithreading!");	
	if (CaptureThread)
	{
		UE_LOG(LogTemp, Log, TEXT("Log: Thread isn't null. It's: %s"), *CaptureThread->GetThreadName());
	}
	CaptureThread = FRunnableThread::Create(this, TEXT("FCaptureWorker"), 128 * 1024, TPri_Normal);
	UE_LOG(LogTemp, Log, TEXT("Log: Created thread"));
}

TArray<uint8> FCaptureWorker::ReadFromInbox()
{
	TArray<uint8> Image;
	Inbox.Dequeue(Image);
	return Image;
}

bool FCaptureWorker::Init()
{
	bRun = true;
	return true;
}

uint32 FCaptureWorker::Run()
{
	AsyncTask(ENamedThreads::GameThread, [this](){
			UE_LOG(LogTemp, Log, TEXT("Start worker"));
		});

	while(bRun)
	{
		FDateTime timeBeginningOfTick = FDateTime::UtcNow();

		bool bCaptured = false;
		
		TArray<FColor> RawPixels;

		CameraCapture->UpdateContent();

		bCaptured = ThreadSafe_ReadPixels(RenderTarget, RawPixels);

		if (bCaptured)
		{
			TArray<uint8> CapturedImage;
			for (auto& Pixel : RawPixels)
			{
				const uint8 PR = Pixel.R;
				const uint8 PB = Pixel.B;
				Pixel.R = PB;
				Pixel.B = PR;
				Pixel.A = 255;
			}

			IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
			TSharedPtr<IImageWrapper> PngImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

			const int32 Width = CameraCapture->TextureTarget->SizeX;
			const int32 Height = CameraCapture->TextureTarget->SizeY;

			if (PngImageWrapper.IsValid() && PngImageWrapper->SetRaw(&RawPixels[0],
			RawPixels.Num() * sizeof(FColor), Width, Height, ERGBFormat::RGBA, 8))
			{
				CapturedImage = PngImageWrapper->GetCompressed();

				FFileHelper::SaveArrayToFile(CapturedImage, *(FPaths::ProjectDir() + TEXT("Out.png")));

				AsyncTask(ENamedThreads::GameThread, [this](){
					UE_LOG(LogTemp, Log, TEXT("WE SAVE Image as FILE!"));
				});

				Inbox.Enqueue(CapturedImage);

				AsyncTask(ENamedThreads::GameThread, [this](){
				 	ThreadSpawnerActor.Get()->ExecuteOnImageCaptured(ThreadSpawnerActor);
				 });
			}
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

	return 0;
}

void FCaptureWorker::Stop()
{
	bRun = false;
}

void FCaptureWorker::Exit()
{

}

bool FCaptureWorker::ThreadSafe_ReadPixels(FRenderTarget* RT, TArray<FColor>& OutImageData, 
	FReadSurfaceDataFlags InFlags, FIntRect InRect)
{
	if (InRect == FIntRect(0, 0, 0, 0))
    {
        InRect = FIntRect(0, 0, RT->GetSizeXY().X, RT->GetSizeXY().Y);
    }

	// Read the render target surface data back.    
    struct FReadSurfaceContext
    {
        FRenderTarget* SrcRenderTarget;
        TArray<FColor>* OutData;
        FIntRect Rect;
        FReadSurfaceDataFlags Flags;
    };
    OutImageData.Reset();
    FReadSurfaceContext ReadSurfaceContext =
    {
        RT,
        &OutImageData,
        InRect,
        InFlags
    };

	    ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
        ReadSurfaceCommand,
        FReadSurfaceContext, Context, ReadSurfaceContext,
        {
            RHICmdList.ReadSurfaceData(
                Context.SrcRenderTarget->GetRenderTargetTexture(),
                Context.Rect,
                *Context.OutData,
                Context.Flags
            );
        });

	while(OutImageData.Num() == 0)
	{
		//FWindowsPlatformProcess::Sleep(1.0f);
		FPlatformProcess::Sleep(TimeBetweenTicks);		
	}

	return OutImageData.Num() > 0;
}