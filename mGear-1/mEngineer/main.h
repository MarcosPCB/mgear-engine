#pragma once

#include "engine.h"

#define ARIAL 0
#define ARIAL_BOULD 1
#define GEOMET 2

struct mEng
{
	char mgg_list[64][256];
	uint8 num_mgg;
	uint8 pannel_choice;
};

extern mEng meng;

void Menu();
