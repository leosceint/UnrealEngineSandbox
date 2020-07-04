// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CanData.h"
#include "./GeographicLib/LocalCartesian.hpp"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Geo2Ue.generated.h"

UCLASS()
class TEST_GL_API AGeo2Ue : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGeo2Ue();
	AGeo2Ue(float lat_0, float lon_0, float h_0, float alpha);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Coordinate Translation")
	void TranslateCoordinates(FCanData inData, FCoordinates &outData);

private:
	GeographicLib::LocalCartesian proj;
	float m_radAlpha;
	float m_sa;
	float m_ca;

};
