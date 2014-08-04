#include <stdio.h>

typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int int16;
typedef unsigned short int uint16;
typedef signed long int int32;
typedef unsigned long int uint32;
typedef long long unsigned uint64;

#define QLZ_COMPRESSION_LEVEL 3
#define QLZ_STREAMING_BUFFER 0
#define QLZ_MEMORY_SAFE 1

#define MAX_FRAMES 65536
#define MAX_ANIMATIONS 64

enum _MGGTYPE
{
	SPRITEM,
	TEXTUREM
};

struct _MGGANIM
{
	char name[32];
	uint16 num_frames;
	float current_frame;
	uint32 startID;
	uint32 endID;
};

struct _MGGFORMAT
{
	char name[32];
	uint16 num_frames;
	_MGGTYPE type;
	uint32 num_animations;
};