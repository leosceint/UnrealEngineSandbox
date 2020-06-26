// Fill out your copyright notice in the Description page of Project Settings.


#include "ImageCapture.h"

#include "HAL/RunnableThread.h"
#include "Async/Async.h"
#include "Logging/MessageLog.h"

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
	CaptureWorker = TUniquePtr<FCaptureWorker>(new FCaptureWorker(TimeBetweenTicks, CameraCapture));
	CaptureWorker->Start();
}

void AImageCapture::ExecuteOnImageCaptured(TWeakObjectPtr<AImageCapture> thisObj)
{
	if(!thisObj.IsValid())
 		return;

	//TArray<uint8> Image = CaptureWorker->ReadFromInbox();
	//ImageCapturedDelegate.ExecuteIfBound(Image);
}

FCaptureWorker::FCaptureWorker(float inTimeBetweenTicks, class USceneCaptureComponent2D* Camera)
:TimeBetweenTicks(inTimeBetweenTicks)
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
	while(bRun)
	{
		FDateTime timeBeginningOfTick = FDateTime::UtcNow();

		TArray<uint8> CapturedImage;
		bool bCaptured = false;
		

		if (bCaptured)
		{
			Inbox.Enqueue(CapturedImage);
			AsyncTask(ENamedThreads::GameThread, [this](){
				ThreadSpawnerActor.Get()->ExecuteOnImageCaptured(ThreadSpawnerActor);
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

	return 0;
}

void FCaptureWorker::Stop()
{
	bRun = false;
}

void FCaptureWorker::Exit()
{

}
