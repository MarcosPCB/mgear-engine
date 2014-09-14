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

#define ADD_SPRITE_TYPE 16
#define ACTOR_SPRITE 17
#define SPRITE_TAG 22
#define SPRITE_HEALTH 23
#define SPRITE_PHY 24
#define RGB_SPRITE 18

#define EDIT_SPRITE 19
#define EDIT_SPRITE_TYPE 20
#define SPRITE_EDIT_BOX 21

struct mEng
{
	char mgg_list[32][256];
	uint16 num_mgg;
	uint8 pannel_choice;
	uint8 command;
	int16 scroll;
	int16 scroll2;
	uint16 command2;
	uint16 sub_com;
	uint16 com_id;
	GLuint tex_selection;
	GLuint tex2_sel;
	uint32 tex2_ID;
	int16 tex2_MGGID;
	uint32 tex_ID;
	int16 tex_MGGID;
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

	struct SpriteEN
	{
		_OBJTYPE type;
		Colori color;
		int16 tag;
		int16 health;
		uint16 anim; //if the game actor is animated than leave it NULL
		uint32 gid;
		Body body;
	} spr, spr2;

	uint32 palette[256][256];

	Colori tmp_color;

	char *path;
};

extern mEng meng;

int16 DirFiles(const char *path, char content[512][512]);

void Menu();
void ImageList();
