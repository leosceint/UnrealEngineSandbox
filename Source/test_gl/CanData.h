#pragma once

#include "CoreMinimal.h"
#include "CanData.generated.h"

USTRUCT(BlueprintType)
struct FCanData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float lat;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float lon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float height;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float course_angle;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float pitch;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float course;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float roll;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float peleng;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float angleV;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float angleH;

    FCanData()
    {
        // packet 101
        lat = 0;
        lon = 0;
        // 
        // packet 102
        height= 0;
        course_angle= 0;
        pitch= 0;
        //
        // packet 103
        course= 0;
        roll= 0;
        peleng= 0;
        //
        // packet 104
        angleV= 0;
        angleH= 0;
        //
    }   

};

//Data Structs for Can Emulator

struct EmulatorData
{
	uint32 type;
    double part1;
    double part2;
    double part3;     // нет данных
	uint16 time;
};