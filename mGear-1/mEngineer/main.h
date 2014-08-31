#pragma once

#include "engine.h"

#define ARIAL 0
#define ARIAL_BOULD 1
#define GEOMET 2

#define DRAW_SECTOR 0
#define ADD_OBJ 3
#define ADD_SPRITE 4
#define SELECT_EDIT 2
#define TEX_SEL 5
#define MGG_SEL 6
#define MGG_LOAD 7
#define ADD_OBJ_TYPE 8
#define TEX_SIZE_OBJ 9

struct mEng
{
	char mgg_list[64][256];
	uint8 num_mgg;
	uint8 pannel_choice;
	uint8 command;
	int16 scroll;
	int16 scroll2;
	uint16 command2;
	GLuint tex_selection;
	uint32 tex_ID;
	int8 tex_MGGID;
	uint16 mgg_sel;
	uint16 menu_sel;

	struct ObjEN
	{
		_OBJTYPE type;
		Colori color;
		float amblight;
		Pos texsize;
		Pos texpan;
	} obj;

	char *path;
};

extern mEng meng;

int16 DirFiles(const char *path, char content[512][512]);

void Menu();
void ImageList();
