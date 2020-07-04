#pragma once

#include "CoreMinimal.h"
#include "CanData.generated.h"

/** Один из пакетов, переданных по CAN (# 101, # 102, # 103, # 104) **/
struct EmulatorData
{
	uint32 type;
    double part1;
    double part2;
    double part3;     // нет данных
	uint16 time;
};

/** Собранные данные из всех необходимых пакетов **/
USTRUCT(BlueprintType)
struct FCanData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float lat;                                      //  гиодезическая широта
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float lon;                                      //  гиодезическая долгота
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float height;                                   //  гиодезическая высота
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float course_angle;                             //  угол истинного курса
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float pitch;                                    //  тангаж
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float course;                                   //  курс
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float roll;                                     //  крен
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float peleng;                                   //  дальность до цели
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float angleV;                                   //  угол цели в вертикальной плоскости
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float angleH;                                   //  угол цели в горизонтальной плоскости

    FCanData()
    {
        // пакет 101
        lat = 0;
        lon = 0;
        // 
        // пакет 102
        height= 0;
        course_angle= 0;
        pitch= 0;
        //
        // пакет 103
        course= 0;
        roll= 0;
        peleng= 0;
        //
        // пакет 104
        angleV= 0;
        angleH= 0;
        //
    }   

};

/** Переведенные для Unreal координаты **/
USTRUCT(BlueprintType)
struct FCoordinates
{
    GENERATED_BODY()
    
    // координаты линейные
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float x;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float y;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float z;

    // координаты угловые
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float rx;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float ry;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float rz;

    // дальность до цели
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float peleng;

    FCoordinates()
    {
        x   = 0;
        y   = 0;
        z   = 0;
        rx  = 0;
        ry  = 0;
        rz  = 0;
    }
};