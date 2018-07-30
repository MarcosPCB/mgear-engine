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

enum FILE_TYPE
{
	F_MGG,
	F_MGV,
	F_AUDIO,
	F_CODE,
	F_UI,
	F_MISC
};

struct _Files_
{
	char path[MAX_PATH];
	size_t size;
	int16 rev;
	int16 f_rev;
};

typedef struct _Files_ _Files;

struct _TODOL
{
	char entry[512];
	union Propeties
	{
		uint16 word;
		uint16 type : 2, //0 = text; 1 = checkbox; 2 = list;
		creator : 3,
		assigned : 1,
		assigned_ids : 8,
		completed : 1,
		reserved : 1;
	} properties;
};

typedef struct _TODOL ToDo;

enum USER_TYPE
{
	ADMIN = 4,
	ART = 3,
	SOUND = 2,
	CODE = 1,
	MAP = 0
};

struct SDKEXP
{
	char name[32];
	int16 rev;
	int16 curr_rev;

	uint8 num_users;
	uint8 encrypted;
};

struct _SDKPRJ
{
	char name[32];
	char prj_path[MAX_PATH];

	char prj_raw_path[MAX_PATH];
	char exp_path[MAX_PATH];

	uint8 code;
	uint8 audio;
	uint8 tex;
	uint8 map;
	uint8 ui;
	uint8 sprites;

	uint8 code_type;
	char code_path[MAX_PATH];

	size_t blog_len;
	char *base_log;
	char *log;
	size_t log_len;
	int16 revisions;
	ToDo *TDList;
	int16 TDList_entries;
	int16 curr_rev;

	int32 user_id;
	char user_name[16];
	enum USER_TYPE user_type;
	char users[8][16];
	int8 num_users;

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

	char filepath[MAX_PATH];
	char program_path[MAX_PATH];

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

struct File_sys
{
	char path[MAX_PATH];
	char parent[MAX_PATH];
	char file[32];
	uint8 type;
	uint8 commit;
	int16 filenum; //Folder type only
	size_t size;
};

typedef struct _mSdk mSdk;

