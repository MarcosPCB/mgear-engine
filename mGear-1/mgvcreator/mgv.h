#include <stdio.h>

typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int int16;
typedef unsigned short int uint16;
typedef signed long int int32;
typedef unsigned long int uint32;
typedef long long unsigned uint64;

//File header for the video
struct _MGVFORMAT
{
	uint32 num_frames;
	uint8 fps;
	uint32 sound_buffer_lenght;
};