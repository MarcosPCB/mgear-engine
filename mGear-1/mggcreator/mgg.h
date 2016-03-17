#include <stdio.h>

typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int int16;
typedef unsigned short int uint16;
typedef signed long int int32;
typedef unsigned long int uint32;
typedef long long unsigned uint64;
/*
#define QLZ_COMPRESSION_LEVEL 3
#define QLZ_STREAMING_BUFFER 0
#define QLZ_MEMORY_SAFE 1
*/
#define MAX_FRAMES 8192
#define MAX_ANIMATIONS 64
#define TEX_RANGE 32768 //16-bit precision

#define LINEAR 0
#define NEAREST 1

enum _MGGTYPE_
{
	SPRITEM,
	TEXTUREM
};

typedef enum _MGGTYPE_ _MGGTYPE;

struct _MGGANIM_
{
	char name[32];
	uint16 num_frames;
	uint16 current_frame;
	uint16 startID;
	uint16 endID;
	int8 speed;
};

typedef struct _MGGANIM_ _MGGANIM;

//File header
struct _MGGFORMAT_
{
	char name[32];
	uint16 num_frames;
	_MGGTYPE type;
	uint8 num_atlas;
	uint16 num_singletex;
	uint16 num_texinatlas;
	uint32 num_animations;
	size_t textures_offset;
	size_t possize_offset;
	size_t framesize_offset;
	size_t framealone_offset;
	int8 mipmap;
};

typedef struct _MGGFORMAT_ _MGGFORMAT;