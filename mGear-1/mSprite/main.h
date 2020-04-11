#include "engine.h"
//#include <curl.h>

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
#define UP_ICON 100
#define DOWN_ICON 102
#define RIGHT_ICON 101
#define LEFT_ICON 103
#define LOADING_ICON 30
#define LINK_ICON 126
#define UNLINK_ICON 125
#define PICKER_ICON 119
#define BIN_ICON 38
#define SWITCH_ICON 83

struct _mSprite
{
	int16 command;
	int16 pannel;
	int16 command2;
	int16 state;

	int theme;

	char filepath[MAX_PATH];
	char program_path[MAX_PATH];

	char open_recent[10][MAX_PATH];

	struct File_sys *prj_files;
	int32 num_prj_files;

	int selected_spr;

	uint8 play;
	int16 state_selected;

	int16 anim_frame;
	int16 curframe;

	_SPRITES sprbck[MAX_SPRITES];

	uint8 changes_detected;
};

typedef struct _mSprite mSpr;

