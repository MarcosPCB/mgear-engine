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
#define WALKB 2
#define CROUCHING 3
#define CROUCH 4
#define JUMPING 5
#define FALLING 6
#define PUNCHS1 7
#define PUNCHS2 8
#define PUNCHS3 9
#define PUNCHM1 10
#define PUNCHM2 11
#define KICKS 12
#define KICKM 13
#define CPUNCHS 14
#define CPUNCHM 15
#define CKICKS 16
#define CKICKM 17
#define JPUNCHS 18
#define JPUNCHM 19
#define JUMP 20
#define JKICKS 21
#define JKICKM 22
#define DEFENSE1 23
#define DEFENSE2 24
#define CDEFENSE1 25
#define CDEFENSE2 26
#define PAIN1 27
#define PAIN2 28
#define PAIN3 29
#define PAIN4 30
#define FALL 31
#define FLY 32
#define STANDUP 33
#define FALLTIRED 34
#define FELLTIRED 35
#define STANDUPTIRED 36
#define DIE 37
#define SPECIAL1 38
#define SPECIAL2 39
#define SPECIAL3 40
#define SPECIAL4 41
#define SPECIAL5 42
#define SPECIAL6 43
#define SPECIAL7 44

uint16 WriteCFG();
uint16 LoadCFG();
void SpawnPlayer(Pos pos, Pos size, int16 ang);
void PreGameEvent();

void Menu();

#endif