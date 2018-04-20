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

struct _mggviewer
{
	int16 command;
	int16 pannel;
	int16 command2;
	int16 state;

	_MGG mgg;
};

typedef struct _mggviewer Mggv;

int16 DirFiles(const char *path, char content[512][512]);

void TextureListSelection();
