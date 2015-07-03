#ifndef _MGTYPES_H
#define _MGTYPES_H

typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int int16;
typedef unsigned short int uint16;
typedef signed long int int32;
typedef unsigned long int uint32;
typedef long long unsigned uint64;

typedef struct
{
	int32 x;
	int32 y;
	int32 z;
} Pos;

typedef struct
{
	uint16 x;
	uint16 y;
	uint16 z;
} uPos16;

typedef struct
{
	float x;
	float y;
	float z;
} PosF;

typedef struct
{
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;
} Color;

typedef struct
{
	float r;
	float g;
	float b;
	float a;
} ColorF;

#endif