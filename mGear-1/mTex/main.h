#include "engine.h"
#include "skins.h"

#pragma once

#define ARIAL 0
#define ARIAL_BOULD 1
#define GEOMET 2

#define SELECT_EDIT 1
#define NONE 0
#define IMPORT_TEX 2
#define OPEN_RCP 3
#define OPEN_MGG 4
#define ANIM_TOOL 5
#define MGG_BOX 6
#define COMPILE_MGG 7
#define CREATE_ATLAS 8
#define MULT_SELECT 9
#define MOV_ANIM 10
#define MOV_TEX 11

#define RE_NOTOK 2
#define RE_UNTOK_W 4
#define RE_UNTOK_N 8
#define RE_MISCOM 16
#define RE_WRVAL 32
#define RE_UNCOM 64
#define RE_SEVERR 128
#define RE_MISCOMS 256

// Integrated apps
#define INTG_PS 0
#define INTG_IL 1
#define INTG_GP 2

#define INTG_PS_NAME "Photoshop"
#define INTG_PS_EXE "Photoshop.exe"
#define INTG_IL_NAME "Illustrator"
#define INTG_IL_EXE "Illustrator.exe"
#define INTG_GP_NAME "Gimp"
#define INTG_GP_EXE "gimp.exe"

struct IntApps
{
	char name[32];
	char *path, *args;
	int8 valid;
};

struct _mTex
{
	int16 command;
	int16 pannel;
	int16 command2;
	int16 state;
	int canvas;
	int16 switch_place;

	GLuint *textures;
	Pos *size;
	GLuint *textures_n;
	int selection[512];
	int first_sel, last_sel, first_sel_slot, last_sel_slot;
	int anim_selected;
	int anim_slot;
	int anim_frame;
	int selected;
	int sel_slot;
	int sel_slots[512];
	uint8 mult_selection;
	char prj_path[MAX_PATH], filename[32];
	int8 dn_mode;
	enum NKUITheme theme;

	uint8 play, stop, pause, next, back;

	struct MGGformat
	{
		char name[32], path[MAX_PATH], files[512][MAX_PATH], files_n[512][MAX_PATH], texnames[512][64], texnames_n[512][64];
		int16 num_frames, num_f_n, num_c_atlas, num_c_n_atlas, num_anims, num_atlas, *num_f_a, *fn, *fnn, *num_f0, *num_ff, *an;
		_MGGANIM *mga;
		int8 *frames_atlas;
		uint8 RLE, mipmap;
		int16 frameoffset_x[512];
		int16 frameoffset_y[512];

		FILETIME ftime[512];
		FILETIME ftime_n[512];
	} mgg, mgg2;

	struct IntApps intg_app[16];

	int8 changes_detected;
};

typedef struct _mTex mTex;

int16 DirFiles(const char *path, char content[512][512]);

void TextureListSelection();
