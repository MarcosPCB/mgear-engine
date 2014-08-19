#pragma once

#include "engine.h"

#define ARIAL 0
#define ARIAL_BOULD 1
#define GEOMET 2

#define DRAW_SECTOR 1
#define ADD_OBJ 2
#define ADD_SPRITE 3
#define SELECT_EDIT 4

struct mEng
{
	char mgg_list[64][256];
	uint8 num_mgg;
	uint8 pannel_choice;
	uint8 command;
};

extern mEng meng;

void Menu();
