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

//Apps
#define GAMEAPP 0
#define AUDIOAPP 1
#define ENGINEERAPP 2
#define UIAPP 3
#define TEXAPP 4
#define CODEAPP 5
#define SPRITEAPP 6
#define MGGAPP 7
#define MGVAPP 8

//Font icons
#define DEBUG_ICON 37
#define PLAY_ICON 66
#define STOP_ICON 68
#define PAUSE_ICON 67
#define BACK_ICON 69
#define FOWARD_ICON 70
#define BACKJUMP_ICON 71
#define FOWARDJUMP_ICON 72
#define CONFIG_ICON 35
#define GEARS_ICON 36
#define TERMINAL_ICON 118
#define SAVE_ICON 49
#define VIDEO_ICON 90
#define SOUND_ICON 88
#define IMAGE_ICON 87
#define TEXT_ICON 86
#define FFILE_ICON 84
#define FFOLDER_ICON 50
#define UNDO_ICON 26
#define REDO_ICON 27
#define ROTATE_ICON 31

//Integrated apps
#define INTG_PS 0
#define INTG_IL 1
#define INTG_GP 2
#define INTG_AU 3
#define INTG_VS 4

#define INTG_PS_NAME "Photoshop"
#define INTG_PS_EXE "Photoshop.exe"
#define INTG_IL_NAME "Illustrator"
#define INTG_IL_EXE "Illustrator.exe"
#define INTG_GP_NAME "Gimp"
#define INTG_GP_EXE "gimp.exe"
#define INTG_AU_NAME "Audition"
#define INTG_AU_EXE "Adobe Audition CC.exe"
#define INTG_VS_NAME "Visual Studio IDE"
#define INTG_VS_EXE "devenv.exe"

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
	char hash[512];
};

typedef struct _Files_ _Files;

struct _TODOL
{
	char entry[512];

	uint16 type : 2, //0 = checkbox; 1 = list;
	creator : 3,
	assigned : 1,
	assigned_ids : 8,
	completed : 1,
	reserved : 1;
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

struct LOGPRJ
{
	char user[16];
	int16 rev;
	char *log;
	size_t len;
	_Files *files;
	int16 num_files;

	uint32 min : 6,
	h : 5,
	d : 5,
	m : 4,
	y : 12;
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

	struct LOGPRJ *log;
	int16 *log_list;
	int16 num_logs;
	int16 revisions;
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

	int8 update;

	char open_recent[10][MAX_PATH];

	struct File_sys *prj_files;
	int32 num_prj_files;

	struct APPST
	{
		int32 w : 13,
		h : 13,
		r_index : 6;

		int64 map : 8,
		tex : 10,
		ui : 7,
		mgg : 10,
		map_prj : 10,
		skip_menu : 4;
		
		char argument[1024];
		
		int32 *files;
		int32 selected;
		
		int16 num_files;

		int icon; //Texture ID
		Pos icon_size;

	} app[10];

	int8 num_intg_apps;

	struct INTAPP
	{
		char name[32];
		char *path;
		char *arg;
		int8 valid;
	} intg_app[16];

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
	char hash[512];
};

typedef struct _mSdk mSdk;

