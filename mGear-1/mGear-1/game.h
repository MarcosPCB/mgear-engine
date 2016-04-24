//#include <stdio.h>
//#include <stdlib.h>
#include "engine.h"

#ifndef _GAME_H
#define _GAME_H

#define ARIAL 0
#define FIGHTFONT 1

struct _PLAYER_BSE
{
	int16 current_anim;
	int16 current_frame;

	int16 state;
};

typedef struct _PLAYER_BSE PLAYERC;

//Base Sequences
#define STAND 0
#define WALK 1
#define PUNCH1 2
#define PUNCH2 3

uint16 WriteCFG();
uint16 LoadCFG();
void PreGameEvent();

void Menu();

#endif