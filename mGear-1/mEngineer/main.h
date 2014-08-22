#pragma once

#include "engine.h"

#define ARIAL 0
#define ARIAL_BOULD 1
#define GEOMET 2

#define DRAW_SECTOR 0
#define ADD_OBJ 3
#define ADD_SPRITE 4
#define SELECT_EDIT 2

struct mEng
{
	char mgg_list[64][256];
	uint8 num_mgg;
	uint8 pannel_choice;
	uint8 command;
};

extern mEng meng;

void Menu();
