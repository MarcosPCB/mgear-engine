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
#define OBJ_AMBL 10
#define TEX_PAN_OBJ 11
#define RGB_OBJ 12

#define EDIT_OBJ 13
#define EDIT_OBJ_TYPE 14
#define OBJ_EDIT_BOX 15

struct mEng
{
	char mgg_list[64][256];
	uint8 num_mgg;
	uint8 pannel_choice;
	uint8 command;
	int16 scroll;
	int16 scroll2;
	uint16 command2;
	uint16 com_id;
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
	} obj, obj2;

	uint32 palette[256][256];

	Colori tmp_color;

	char *path;
};

extern mEng meng;

int16 DirFiles(const char *path, char content[512][512]);

void Menu();
void ImageList();
