#include "engine.h"

#pragma once

#define ARIAL 0
#define ARIAL_BOULD 1
#define GEOMET 2

#define NONE_MODE -1
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
#define EDIT_SPRITE_TYPE_S 26
#define RGB_SPRITE 18

#define EDIT_SPRITE 19
#define EDIT_SPRITE_TYPE 20
#define SPRITE_EDIT_BOX 21
#define SPRITE_SELECTION 25

#define ADD_LIGHT 27
#define CREATE_LIGHTMAP 28
#define ADD_LIGHT_TO_LIGHTMAP 29
#define RGB_LIGHTMAP 30
#define RGB_LIGHT 31
#define LIGHT_TAG 32
#define LIGHTMAP_RES 33
#define EDIT_LIGHTMAP 34
#define EDIT_LIGHT_LIGHTMAP 35
#define EDIT_LIGHTMAP_BOX 36
#define EDIT_LIGHT_LIGHTMAP_BOX 37
#define EDIT_LIGHTMAP2 39
#define CREATE_LIGHTMAP_STEP2 38
#define REMOVE_LIGHTMAP 40
#define MOVE_LIGHTMAP 45
#define LOAD_LIGHTMAP 46
#define LOAD_LIGHTMAP2 47
#define RESIZE_LIGHTMAP 48

#define DRAW_SECTOR2 41

#define MAP_PROPERTIES 42
#define VIEWMODE_BOX 43

#define EDIT_SECTOR 44

#define CAM_AREA 49
#define CAM_AREA_EDIT 50
#define CAM_LIM_X 51
#define CAM_LIM_Y 52

#define TRANSFORM_BOX 60
#define CAMERA_BOX 61
#define MAPPR_BOX 62
#define LAYERS_BOX 63

#define FOREGROUND_MODE 0
#define MIDGROUND_MODE 1
#define BACKGROUND1_MODE 2
#define BACKGROUND2_MODE 3
#define BACKGROUND3_MODE 4
#define INGAMEVIEW_MODE 5
#define LIGHTVIEW_MODE 6
#define ALLVIEW_MODE 7

#define NLIGHTING 100
#define NADD_LIGHT 101
#define NADD_LIGHT_PANNEL 102
#define NLOAD_LIGHT 103
#define NEDIT_LIGHT 104
#define NCREATE_LIGHTMAP 105
#define NEDIT_LIGHTMAP 106
#define NLIGHTMAP_PANNEL 107

#define SCALER_SELECT 108
#define PICK_TAG 109

//Font icons
#define DEBUG_ICON 37
#define PLAY_ICON 66
#define STOP_ICON 68
#define PAUSE_ICON 67
#define BACK_ICON 69
#define FOWARD_ICON 70
#define BACKJUMP_ICON 71
#define FOWARDJUMP_ICON 72
#define CONFIG_ICON 35
#define GEARS_ICON 36
#define TERMINAL_ICON 118
#define SAVE_ICON 49
#define VIDEO_ICON 90
#define SOUND_ICON 88
#define IMAGE_ICON 87
#define TEXT_ICON 86
#define FFILE_ICON 84
#define FFOLDER_ICON 50
#define UNDO_ICON 26
#define REDO_ICON 27
#define ROTATE_ICON 31
#define UP_ICON 100
#define DOWN_ICON 102
#define RIGHT_ICON 101
#define LEFT_ICON 103
#define LOADING_ICON 30
#define LINK_ICON 126
#define UNLINK_ICON 125
#define PICKER_ICON 119
#define BIN_ICON 38

struct _mEng
{
	int8 viewmode;

	int8 hide_ui;

	char mgg_list[32][256];
	uint16 num_mgg;
	int8 pannel_choice;
	int8 command;
	int16 scroll;
	int16 scroll2;
	int16 command2;
	int16 sub_com;
	int16 com_id;
	TEX_DATA tex_selection;
	TEX_DATA tex2_sel;
	uint32 tex2_ID;
	int16 tex2_MGGID;
	uint32 tex_ID;
	int16 tex_MGGID;
	Pos pre_size;
	uint16 mgg_sel;
	uint16 menu_sel;
	int16 curlayer;
	int8 LayerBar;

	int8 playing_sound;

	int16 sprite_selection;
	int16 sprite_frame_selection;

	int16 sprite_edit_selection;

	int16 obj_edit_selection;

	int16 sector_edit_selection;

	int16 light_edit_selection;

	int16 editview;

	int8 scaling;

	int8 picking_tag;

	Pos p;
	int32 got_it;

	struct ObjEN
	{
		_OBJTYPE type;
		Color color;
		float amblight;
		Pos texsize;
		Pos texpan;
		uint16 flag;
	} obj, obj2;

	struct SpriteEN
	{
		_OBJTYPE type;
		Color color;
		int16 tag;
		int16 health;
		uint16 anim; //if the game actor is animated than leave it NULL
		uint32 gid;
		Body body;
		Pos size;
		int16 flags;
	} spr, spr2;

	uPos16 lightmap_res;

	Color lightmap_color;

	int16 obj_lightmap_sel;
	Pos lightmappos;
	Pos lightmapsize;

	unsigned char *tmplightdata;
	unsigned char *tmplightdata2;

	struct Light
	{
		float intensity;
		Color color;
		float falloff;
		float c, l, q;
		uint8 light_id;
		LIGHT_TYPE type;
	} light;

	uint32 palette[256][256];

	Color tmp_color;

	char *path;

	int8 current_command;

	int16 z_slot[57];
	int16 z_buffer[57][2048];
	int16 z_used;

	int16 layers[57][2048];
	int16 active_layer;

	int8 loop_complete;

	int8 editor;

	int16 temp;

	char soundlist[128][128];
	char musiclist[128][128];

	char prj_path[MAX_PATH];

	enum NKUITheme theme;
};

typedef struct _mEng mEng;

extern mEng meng;

int16 DirFiles(const char *path, char content[512][512]);

void Menu();
void ImageList();
void NewMap();

void TextureListSelection();
