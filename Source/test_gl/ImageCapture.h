// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HAL/Runnable.h"
#include "HAL/ThreadSafeBool.h"
#include "Containers/Queue.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ImageCapture.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FImageCapturedDelegate, UPARAM(ref) TArray<uint8>&, Image);

UCLASS()
class TEST_GL_API AImageCapture : public AActor
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BluePrintCallable, Category = "Image Capture")
	void CaptureImage(class USceneCaptureComponent2D* CameraCapture, 
		const FImageCapturedDelegate& OnImageCaptured);

	// Sets default values for this actor's properties
	AImageCapture();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	void ExecuteOnImageCaptured(TWeakObjectPtr<AImageCapture> thisObj);

	/* Time between ticks. Please account for the fact that it takes 1ms to wake up on a modern PC, so 0.01f would effectively be 0.011f */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Image Capture")
	float TimeBetweenTicks = 0.008f;

private:
	TUniquePtr<class FCaptureWorker> CaptureWorker;
	FImageCapturedDelegate ImageCapturedDelegate;

};

// Thread capture class
class FCaptureWorker : public FRunnable, public TSharedFromThis<FCaptureWorker>
{
	FRunnableThread* CaptureThread = nullptr;

private:
	TWeakObjectPtr<AImageCapture> ThreadSpawnerActor;
	float TimeBetweenTicks;
	class USceneCaptureComponent2D* CameraCapture;
	class FRenderTarget* RenderTarget;
	TQueue<TArray<uint8>, EQueueMode::Spsc> Inbox;

public:
	FCaptureWorker(TWeakObjectPtr<AImageCapture> InOwner, float inTimeBetweenTicks, class USceneCaptureComponent2D* Camera);
	virtual ~FCaptureWorker();

	void Start();

	TArray<uint8> ReadFromInbox();

	// Begin FRunnable interface.
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

private:

	bool ThreadSafe_ReadPixels(FRenderTarget* RT, TArray<FColor>& OutImageData, 
	FReadSurfaceDataFlags InFlags = FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX), FIntRect InRect = FIntRect(0, 0, 0, 0));

private:
	FThreadSafeBool bRun = false;

	FCriticalSection SendCriticalSection;
};
