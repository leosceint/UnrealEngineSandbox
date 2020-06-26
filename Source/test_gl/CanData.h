#pragma once
#pragma pack(push, 1)

//Data Structs for Can Emulator

struct EmulatorData101
{
	uint32 type;
    double lat;
    double lon;
    double nop;     // нет данных
	uint16 time;
};

struct EmulatorData102
{
	uint32 type;
    double height;
    double course_angle;
    double pitch;    
	uint16 time;
};

struct EmulatorData103
{
	uint32 type;
    double course;
    double roll;
    double peleng;    
	uint16 time;
};

struct EmulatorData104
{
	uint32 type;
    double angleV;
    double angleH;
    double nop;     // нет данных
	uint16 time;
};

#pragma pack(pop)