#include "engine.h"
#include "main.h"

void NewMap()
{
	int i;

	meng.scroll = 0;
	meng.tex_selection.data = -1;
	meng.command2 = 0;
	meng.scroll2 = 0;
	meng.mgg_sel = 0;
	meng.curlayer = 1;
	meng.LayerBar = 0;

	if (st.Current_Map.obj)
		free(st.Current_Map.obj);

	if (st.Current_Map.sprites)
		free(st.Current_Map.sprites);

	if (st.Current_Map.sector)
		free(st.Current_Map.sector);

	if (st.num_lights>0)
	{
		for (i = 1; i <= st.num_lights; i++)
		{
			free(st.game_lightmaps[i].data);
			st.game_lightmaps[i].obj_id = -1;
			st.game_lightmaps[i].stat = 0;
			glDeleteTextures(1, &st.game_lightmaps[i].tex);
		}
	}

	st.Current_Map.obj = (_MGMOBJ*)malloc(MAX_OBJS*sizeof(_MGMOBJ));
	st.Current_Map.sector = (_SECTOR*)malloc(MAX_SECTORS*sizeof(_SECTOR));
	st.Current_Map.sprites = (_MGMSPRITE*)malloc(MAX_SPRITES*sizeof(_MGMSPRITE));

	st.Current_Map.num_sector = 0;
	st.Current_Map.num_obj = 0;
	st.Current_Map.num_sprites = 0;
	st.num_lights = 0;

	//st.num_lights = 0;

	for (i = 0; i<MAX_SECTORS; i++)
	{
		st.Current_Map.sector[i].id = -1;
		///st.Current_Map.sector[i].layers=1;
		st.Current_Map.sector[i].material = CONCRETE;
		st.Current_Map.sector[i].tag = 0;
		st.Current_Map.sector[i].floor_y_continued = 0;
		st.Current_Map.sector[i].floor_y_up = 0;
		st.Current_Map.sector[i].floor_y_down = 0;
	}

	for (i = 0; i<MAX_OBJS; i++)
	{
		st.Current_Map.obj[i].type = BLANK;
		//st.Current_Map.obj[i].lightmapid = -1;
	}

	for (i = 0; i<MAX_SPRITES; i++)
		st.Current_Map.sprites[i].stat = 0;

	if (st.Current_Map.num_mgg>0)
	{
		for (i = 0; i<st.Current_Map.num_mgg; i++)
			FreeMGG(&mgg_map[i]);
	}

	memset(st.Current_Map.MGG_FILES, 0, 32 * 256);
	meng.num_mgg = st.Current_Map.num_mgg = 0;
	//st.Current_Map.num_mgg = 0;

	memset(meng.mgg_list, 0, 64 * 256);

	st.gt = INGAME;

	st.Camera.position.x = 0;
	st.Camera.position.y = 0;

	meng.pannel_choice = 2;
	meng.command = 2;

	memset(&meng.spr, 0, sizeof(meng.spr));

	meng.obj.amblight = 1;
	meng.obj.color.r = meng.spr.color.r = 255;
	meng.obj.color.g = meng.spr.color.g = 255;
	meng.obj.color.b = meng.spr.color.b = 255;
	meng.obj.color.a = meng.spr.color.a = 255;
	meng.obj.texsize.x = 32768;
	meng.obj.texsize.y = 32768;
	meng.obj.texpan.x = 0;
	meng.obj.texpan.y = 0;
	meng.obj.type = meng.spr.type = MIDGROUND;
	meng.obj_lightmap_sel = -1;

	meng.lightmapsize.x = 0;
	meng.lightmapsize.y = 0;

	meng.spr.gid = -1;
	meng.spr2.gid = -1;
	meng.sprite_selection = 0;
	meng.sprite_frame_selection = 0;
	meng.spr.size.x = 2048;
	meng.spr.size.y = 2048;

	meng.playing_sound = 0;

	meng.lightmap_res.x = meng.lightmap_res.y = 256;

	st.Current_Map.bck1_v = BCK1_DEFAULT_VEL;
	st.Current_Map.bck2_v = BCK2_DEFAULT_VEL;
	st.Current_Map.fr_v = FR_DEFAULT_VEL;
	st.Current_Map.bcktex_id = -1;
	st.Current_Map.bcktex_mgg = 0;
	memset(&st.Current_Map.amb_color, 255, sizeof(Color));

	meng.viewmode = 7;

	memset(meng.z_buffer, 0, 2048 * 57 * sizeof(int16));
	memset(meng.z_slot, 0, 57 * sizeof(int16));
	meng.z_used = 0;

	memset(meng.layers, 0, 2048 * 57 * sizeof(int16));

	meng.picking_tag = 0;

	//memset(meng.used_names, 0, 2048);
	memset(st.Current_Map.activator_table, 0, 2048);

	st.Current_Map.cam_area.area_pos.x = st.Current_Map.cam_area.area_pos.y = 0;
	st.Current_Map.cam_area.area_size.x = 1.0;
	st.Current_Map.cam_area.area_size.y = 1.0;
	st.Current_Map.cam_area.horiz_lim = 0;
	st.Current_Map.cam_area.vert_lim = 0;
	st.Current_Map.cam_area.max_dim.x = 6.0;
	st.Current_Map.cam_area.max_dim.y = 6.0;
	st.Current_Map.cam_area.limit[0].x = 0;
	st.Current_Map.cam_area.limit[1].x = 16384;
	st.Current_Map.cam_area.limit[0].y = 0;
	st.Current_Map.cam_area.limit[1].y = 8192;

	st.Current_Map.bck3_size.x = st.Current_Map.bck3_size.y = TEX_PAN_RANGE;

	meng.curlayer = 1;
	meng.light.type = POINT_LIGHT_NORMAL;
	meng.light.color.r = meng.light.color.g = meng.light.color.b = 255;
	meng.light.falloff = 256;
	meng.light.c = 0.01;
	meng.light.intensity = 1;

	meng.gridsize = 64;

	meng.tex_selection.vb_id = -1;
}