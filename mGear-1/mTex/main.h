#include "engine.h"

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

#define RE_NOTOK 2
#define RE_UNTOK_W 4
#define RE_UNTOK_N 8
#define RE_MISCOM 16
#define RE_WRVAL 32
#define RE_UNCOM 64
#define RE_SEVERR 128
#define RE_MISCOMS 256

struct _mTex
{
	int16 command;
	int16 pannel;
	int16 command2;
	int16 state;

	GLuint *textures;
	Pos *size;
	GLuint *textures_n;
	int selection[512];
	int first_sel, last_sel;
	int anim_selected;
	int anim_slot;
	int selected;
	uint8 mult_selection;
	char prj_path[MAX_PATH];
	int8 dn_mode;

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
	} mgg;
};

typedef struct _mTex mTex;

int16 DirFiles(const char *path, char content[512][512]);

void TextureListSelection();
