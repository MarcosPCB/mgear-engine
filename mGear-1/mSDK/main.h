#include "engine.h"
#include <curl.h>

#pragma once

#define ARIAL 0
#define ARIAL_BOULD 1
#define GEOMET 2

#define NONE 0

#define MCODE_C 0
#define MCODE_MGL 1

#define MCODE_MSVC 0

struct _SDKPRJ
{
	char name[32];
	char prj_path[MAX_PATH];

	char prj_raw_path[MAX_PATH];

	uint8 code;
	uint8 code_type;
	char code_path[MAX_PATH];

	char *log;
	char todolist[16384];

	uint8 loaded;
};

typedef struct _SDKPRJ SDKPRJ;

struct _mSdk
{
	int16 command;
	int16 pannel;
	int16 command2;
	int16 state;

	int theme;

	SDKPRJ prj;

	CURL *curl;
	CURLM *curlm;
	CURLcode res;
	CURLMcode resm;
	CURLMsg *curlmsg;

	uint8 ffmpeg_downloaded;
	uint8 ffmpeg_installed;

	double download_total;
	double download_now;

	uint8 downloading;
};

typedef struct _mSdk mSdk;

