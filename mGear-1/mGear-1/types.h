#pragma once

#include <stdio.h>

typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int int16;
typedef unsigned short int uint16;
typedef signed long int int32;
typedef unsigned long int uint32;
typedef long long unsigned uint64;

typedef struct
{
	double x;
	double y;
} Pos;

typedef struct
{
	float r;
	float g;
	float b;
	float a;
} Color;

typedef struct
{
	uint8 r;
	uint8 g;
	uint8 b;
	float a;
} Colori;