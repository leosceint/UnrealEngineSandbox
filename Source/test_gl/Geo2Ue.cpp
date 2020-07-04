// Fill out your copyright notice in the Description page of Project Settings.


#include "Geo2Ue.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AGeo2Ue::AGeo2Ue():proj(55.056248, 38.774146, 188.0)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	m_radAlpha = UKismetMathLibrary::GetPI();
	m_sa = sin(m_radAlpha);
	m_ca = cos(m_radAlpha);

}

AGeo2Ue::AGeo2Ue(float lat_0, float lon_0, float h_0, float alpha):proj(lat_0, lon_0, h_0)
{
	m_radAlpha = alpha / 180.0 * UKismetMathLibrary::GetPI();
    m_sa = sin(m_radAlpha);
    m_ca = cos(m_radAlpha);

}

// Called when the game starts or when spawned
void AGeo2Ue::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGeo2Ue::TranslateCoordinates(FCanData inData, FCoordinates &outData)
{
	double x, y, z;
	proj.Forward(inData.lat, inData.lon, inData.height, x, y, z);

	outData.x = m_ca * x - m_sa * y;
	outData.y = m_sa * x + m_ca * y;
	outData.x = -outData.x;

	outData.rx = inData.roll;
	outData.ry = inData.pitch;
	outData.rz = 270 + inData.course;
	outData.peleng = inData.peleng;
}

// Called every frame
void AGeo2Ue::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

