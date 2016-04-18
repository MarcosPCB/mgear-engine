#include "engine.h"
#include "main.h"
#include "UI.h"

#ifndef MGGEDITOR_H_
#define MGGEDITOR_H_

struct _SPBOX
{
	int32 x, y, sx, sy;
};

typedef struct _SPBOX SPBOX;

struct _MGGEd
{
	int8 mgg_loaded;
	
	_MGG mgg;

	int16 atlas_editor;

	unsigned char *data;

	TEX_DATA tex;

	uint8 bckrgb[3];
	int16 num_sprites;

	SPBOX *spbox;

	unsigned char *setting_file;

	int w, h, channel;
};

typedef struct _MGGEd MGGEd;

extern MGGEd mged;

void MGGEditorMain();

#endif