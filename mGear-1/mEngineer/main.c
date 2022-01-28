#include <Windows.h>
//#include <ShlObj.h>
#include <Shlwapi.h>
#include <commdlg.h>
#undef PlaySound
#include "input.h"
#include "main.h"
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include "dirent.h"
#include "UI.h"
#include "physics.h"
#include "mggeditor.h"
#include "funcs.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_gl3.h"

#define _NKUI_SKINS
#include "skins.h"

#ifdef _DEBUG
	#include <crtdbg.h>
#endif

#define NK_TEXT_LB NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_BOTTOM

#define MAX_NK_VERTEX_BUFFER 512 * 1024
#define MAX_NK_ELEMENT_BUFFER 128 * 1024
#define MAX_NK_COMMAND_BUFFER 5000000
#define MAX_NK_BUFFER 16000000

//extern _MGG mgg_sys[3];

mEng meng, nmeng;

struct nk_font *fonts[4];

int nkrendered = 0;

struct nk_context *ctx;

int prev_tic, curr_tic, delta;

char *Layers[] = { "Background 1", "Background 2", "Background 3", "Midground", "Foreground" };

void SetThemeBack()
{
	switch (meng.theme)
	{
		case THEME_BLACK:
			SetSkin(ctx, THEME_BLACK);
			break;

		case THEME_RED:
			SetSkin(ctx, THEME_RED);
			break;

		case THEME_WHITE:
			SetSkin(ctx, THEME_WHITE);
			break;

		case THEME_BLUE:
			SetSkin(ctx, THEME_BLUE);
			break;

		case THEME_DARK:
			SetSkin(ctx, THEME_DARK);
	}
}

int nk_button_icon_set(uint8 icon)
{
	char buf[2];

	buf[0] = icon;
	buf[1] = 0;

	nk_style_set_font(ctx, &fonts[1]->handle);

	int ret = nk_button_label(ctx, buf);

	nk_style_set_font(ctx, &fonts[0]->handle);

	return ret;
}

uint16 WriteCFG()
{
	FILE *file;

	if((file=fopen("meng_settings.cfg","w"))==NULL)
		return 0;

	st.screenx=1024;
	st.screeny=576;
	st.fullscreen=0;
	st.bpp=32;
	st.audiof=44100;
	st.audioc=2;
	st.vsync=0;

	fprintf(file,"ScreenX = %d\n",st.screenx);
	fprintf(file,"ScreenY = %d\n",st.screeny);
	fprintf(file,"FullScreen = %d\n",st.fullscreen);
	fprintf(file,"ScreenBPP = %d\n",st.bpp);
	fprintf(file,"AudioFrequency = %d\n",st.audiof);
	fprintf(file,"AudioChannels = %d\n",st.audioc);
	fprintf(file,"VSync = %d\n",st.vsync);

	fclose(file);

	return 1;
}

uint16 LoadCFG()
{
	FILE *file;
	char buf[2048], str[128], str2[2048], *buf2, buf3[2048];
	int value = 0, recent = 0;

	//for (int i = 0; i < 10; i++)
		//msdk.open_recent[i][0] = '\0';

	if ((file = fopen("meng_settings.cfg", "r")) == NULL)
	{
		if (WriteCFG() == 0)
			return 0;

		if ((file = fopen("meng_settings.cfg", "r")) == NULL)
			return 0;
	}

	while(!feof(file))
	{
		fgets(buf,sizeof(buf),file);
		sscanf(buf,"%s = %d", str, &value);
		if(strcmp(str,"ScreenX")==NULL) st.screenx=value;
		if(strcmp(str,"ScreenY")==NULL) st.screeny=value;
		if(strcmp(str,"ScreenBPP")==NULL) st.bpp=value;
		if(strcmp(str,"FullScreen")==NULL) st.fullscreen=value;
		if(strcmp(str,"AudioFrequency")==NULL) st.audiof=value;
		if(strcmp(str,"AudioChannels")==NULL) st.audioc=value;
		if(strcmp(str,"VSync")==NULL) st.vsync=value;
		if (strcmp(str, "CurrentPath") == NULL)
		{
			memcpy(buf3, buf, 2048);
			buf2 = strtok(buf3, "\"");
			buf2 = strtok(NULL, "\"");
			strcpy(st.CurrPath, buf2);
			continue;
		}
	}

	if(!st.screenx || !st.screeny || !st.bpp || !st.audioc || !st.audioc || st.vsync>1)
	{
		fclose(file);
		if(WriteCFG()==0)
			return 0;
	}

	fclose(file);

	return 1;

}

int16 DirFiles(const char *path, char content[512][512])
{
	DIR *dir;
	dirent *ent;
	uint16 i=0;
	int16 filenum=0;

	if((dir=opendir(path))!=NULL)
	{
		while((ent=readdir(dir))!=NULL)
		{
			strcpy(content[i],ent->d_name);
			i++;
			filenum++;
		}

		closedir(dir);
	}
	else
	{
		LogApp("Coulnd not open directory");
		return -1;
	}

	return filenum;
}

void SnapToGrid(int32 *x, int32 *y)
{
	if (meng.gridsize > 0 && meng.snap == 1)
	{
		int32 grid_ceil = ceil((double)meng.gridsize / GAME_ASPECT);

		int32 g_sx = GAME_WIDTH / meng.gridsize;
		int32 g_sy = GAME_HEIGHT / grid_ceil;

		*x *= st.Camera.dimension.x;
		*y *= st.Camera.dimension.y;

		int64 st_x = (float)(((*x - GAME_UNIT_MIN) % GAME_WIDTH) % (GAME_WIDTH / meng.gridsize));
		int64 st_y = (float)(((*y - GAME_UNIT_MIN) % GAME_HEIGHT) % (GAME_HEIGHT / grid_ceil));

		*x = st_x > g_sx / 2 ? (float)((*x + g_sx - st_x) / st.Camera.dimension.x): (float)((*x - st_x) / st.Camera.dimension.x);
		*y = st_y > g_sy / 2 ? (float)((*y + g_sy - st_y) / st.Camera.dimension.y) : (float)((*y - st_y) / st.Camera.dimension.y);
	}
}

void PositionToEdge(int32 *x, int32 *y, int32 sizex, int32 sizey)
{
	sizex /= 2;
	sizey /= 2;
	switch (meng.select_edge)
	{
		case 0:
			*x += sizex;
			*y += sizey;
			break;

		case 1:
			//*x -= sizex;
			*y += sizey;
			break;

		case 2:
			*x -= sizex;
			*y += sizey;
			break;

		case 3:
			*x += sizex;
			//*y += sizey;
			break;

		case 5:
			*x -= sizex;
			//*y += sizey;
			break;

		case 6:
			*x += sizex;
			*y -= sizey;
			break;

		case 7:
			//*x -= sizex;
			*y -= sizey;
			break;

		case 8:
			*x -= sizex;
			*y -= sizey;
			break;
	}
}

void FixZLayers()
{
	uint16 i, j;

	int16 z;

	for (i = 16; i < 57; i++)
	{
		if (meng.z_slot[i] > 0)
		{
			for (j = 0; j < meng.z_slot[i]; j++)
			{
				if (meng.z_buffer[i][j] < 2000 && st.Current_Map.obj[meng.z_buffer[i][j]].position.z != i && meng.z_buffer[i][j] != -1)
				{
					z = st.Current_Map.obj[meng.z_buffer[i][j]].position.z;
					meng.z_buffer[z][meng.z_slot[z]] = meng.z_buffer[i][j];
					meng.z_slot[z]++;
					meng.z_buffer[i][j] = -1;
				}

				if (meng.z_buffer[i][j] >= 2000 && meng.z_buffer[i][j] < 10000 && st.Current_Map.sprites[meng.z_buffer[i][j] - 2000].position.z != i && meng.z_buffer[i][j] != -1)
				{
					z = st.Current_Map.sprites[meng.z_buffer[i][j] - 2000].position.z;
					meng.z_buffer[z][meng.z_slot[z]] = meng.z_buffer[i][j];
					meng.z_slot[z]++;
					meng.z_buffer[i][j] = -1;
				}

				if (meng.z_buffer[i][j] >= 12000 && st.game_lightmaps[meng.z_buffer[i][j] - 12000].falloff[4] + 24 != i && meng.z_buffer[i][j] != -1)
				{
					z = st.game_lightmaps[meng.z_buffer[i][j] - 12000].falloff[4] + 24;
					meng.z_buffer[z][meng.z_slot[z]] = meng.z_buffer[i][j];
					meng.z_slot[z]++;
					meng.z_buffer[i][j] = -1;
				}
			}
		}
	}

	for (i = 16; i < 57; i++)
	{
		if (meng.z_slot[i] > 0)
		{
			z = 0;
			for (j = 0; j < meng.z_slot[i]; j++)
			{
				if (meng.z_buffer[i][j] == -1)
				{
					meng.z_buffer[i][j] = meng.z_buffer[i][j + 1];
					if (meng.z_buffer[i][j] != -1)
						z++;

					meng.z_buffer[i][j + 1] = -1;
				}
				else
					z++;
			}

			meng.z_slot[i] = z;
			meng.z_used = i;
		}
	}
}

void FixLayerBar()
{
	register int16 i;
	register int32 z;

	FixZLayers();

	if (meng.command2 == EDIT_OBJ)
	{
		z = st.Current_Map.obj[meng.obj_edit_selection].position.z;

		for (i = 0; i < meng.z_slot[z]; i++)
		{
			if (meng.z_buffer[z][i] == meng.obj_edit_selection)
			{
				memset(meng.layers, 0, sizeof(int16)* 57 * 2048);
				meng.layers[z][i] = 1;
				break;
			}
		}
	}
	else
	if (meng.command2 == EDIT_SPRITE)
	{
		z = st.Current_Map.sprites[meng.sprite_edit_selection].position.z;

		for (i = 0; i < meng.z_slot[z]; i++)
		{
			if (meng.z_buffer[z][i] == meng.sprite_edit_selection + 2000)
			{
				memset(meng.layers, 0, sizeof(int16)* 57 * 2048);
				meng.layers[z][i] = 1;
				break;
			}
		}
	}
	else
	if (meng.command2 == NEDIT_LIGHT)
	{
		z = st.game_lightmaps[meng.light_edit_selection].falloff[4] + 24;

		for (i = 0; i < meng.z_slot[z]; i++)
		{
			if (meng.z_buffer[z][i] == meng.light_edit_selection + 12000)
			{
				memset(meng.layers, 0, sizeof(int16)* 57 * 2048);
				meng.layers[z][i] = 1;
				break;
			}
		}
	}
}

void RedoZBuffers()
{
	int16 m = 0;

	memset(meng.z_buffer, -1, 2048 * 57 * 2);
	memset(meng.z_slot, 0, 57 * 2);
	meng.z_used = 0;

	for (m = 0; m < st.Current_Map.num_obj; m++)
	{
		meng.z_buffer[st.Current_Map.obj[m].position.z][meng.z_slot[st.Current_Map.obj[m].position.z]] = m;
		meng.z_slot[st.Current_Map.obj[m].position.z]++;
		if (st.Current_Map.obj[m].position.z>meng.z_used)
			meng.z_used = st.Current_Map.obj[m].position.z;
	}

	for (m = 0; m < st.Current_Map.num_sprites; m++)
	{
		meng.z_buffer[st.Current_Map.sprites[m].position.z][meng.z_slot[st.Current_Map.sprites[m].position.z]] = m + 2000;
		meng.z_slot[st.Current_Map.sprites[m].position.z]++;
		if (st.Current_Map.sprites[m].position.z>meng.z_used)
			meng.z_used = st.Current_Map.sprites[m].position.z;
	}

	for (m = 0; m < st.Current_Map.num_sector; m++)
	{
		meng.z_buffer[24][meng.z_slot[24]] = m + 10000;
		meng.z_slot[24]++;
		if (24>meng.z_used)
			meng.z_used = 24;
	}

	for (m = 1; m <= st.num_lights; m++)
	{
		int8 lz = st.game_lightmaps[m].falloff[4] + 24;

		meng.z_buffer[lz][meng.z_slot[lz]] = m + 12000;
		meng.z_slot[lz]++;

		if (lz>meng.z_used)
			meng.z_used = lz;
	}
}

void DeleteScenario(int16 id)
{
	if (st.Current_Map.num_obj == id + 1) //Just delete it
	{
		st.Current_Map.obj[id].flag = 1024;
		st.Current_Map.obj[id].type = BLANK;
		st.Current_Map.num_obj--;
	}
	else
	{
		st.Current_Map.obj[id].type = BLANK;

		for (uint16 i = id; i < st.Current_Map.num_obj; i++)
			st.Current_Map.obj[i] = st.Current_Map.obj[i + 1];

		st.Current_Map.num_obj--;
	}
}

void DeleteSprite(int16 id)
{
	if (st.Current_Map.num_sprites == id + 1) //Just delete it
	{
		st.Current_Map.sprites[id].flags = 1024;
		st.Current_Map.sprites[id].stat = 0;
		st.Current_Map.num_sprites--;
	}
	else
	{
		st.Current_Map.sprites[id].stat = 0;

		for (uint16 i = id; i < st.Current_Map.num_sprites; i++)
			st.Current_Map.sprites[i] = st.Current_Map.sprites[i + 1];

		st.Current_Map.num_sprites--;
	}
}

void DeleteSector(int16 id)
{
	if (st.Current_Map.num_sector == id + 1) //Just delete it
	{
		st.Current_Map.sector[id].id = -1;
		st.Current_Map.num_sector--;
	}
	else
	{
		st.Current_Map.sector[id].id = -1;

		for (uint16 i = id; i < st.Current_Map.num_sector; i++)
			st.Current_Map.sector[i] = st.Current_Map.sector[i + 1];

		st.Current_Map.num_sector--;
	}
}

void DeleteLight(int16 id)
{
	if (st.num_lights == id) //Just delete it
	{
		st.game_lightmaps[id].stat = 0;
		st.num_lights--;
	}
	else
	{
		st.game_lightmaps[id].stat = 0;

		for (uint16 i = id; i <= st.num_lights; i++)
			st.game_lightmaps[i] = st.game_lightmaps[i + 1];

		st.num_lights--;
	}
}

static void MGGList()
{
	uint16 j=0, i;
	UIData(8192,4608,2275,st.gamey,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,5);
	if(st.Current_Map.num_mgg>0)
	{
		for(i=227;i<454*st.Current_Map.num_mgg;i+=454)
		{
			if(j==st.Current_Map.num_mgg)
				break;
			else
			{
				if(!CheckCollisionMouse(8192,i+meng.scroll2,1365,455,0))
				{
					StringUIData(meng.mgg_list[j],8192,i+meng.scroll2,0,0,0,255,128,32,255,ARIAL,2048,2048,0);
				}
				else
				{
					StringUIData(meng.mgg_list[j],8192,i+meng.scroll2,0,0,0,255,32,0,255,ARIAL,2048,2048,0);
					if(st.mouse1)
					{
						meng.mgg_sel=j;
						meng.command=meng.pannel_choice;
						st.mouse1=0;
						break;
					}
				}
				j++;
			}
		}

		if(st.mouse_wheel>0)
		{
			if(meng.scroll2<0) meng.scroll2+=454;
			st.mouse_wheel=0;
		}

		if(st.mouse_wheel<0)
		{
			meng.scroll2-=454;
			st.mouse_wheel=0;
		}
	}
	else
		meng.command=meng.pannel_choice;
}

void ImageList()
{
	uint32 m=0, i=728, j=728, id=0, l=0;

	if(!st.Current_Map.num_mgg)
		StringUIData("No MGGs loaded",8192,4608,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
	else
	{
		if(meng.scroll<0)
				m=(meng.scroll/1546)*(-10);
		
			for(id=0;id<st.Current_Map.num_mgg;id++)
			{
				for(m=0;m<mgg_map[id].num_frames;m++)
					{
						if((CheckCollisionMouse(j,i+meng.scroll,1638,1456,0) && st.mouse1) || (meng.tex_selection.data==mgg_map[id].frames[m].data && meng.tex_selection.posx==mgg_map[id].frames[m].posx && meng.tex_selection.posy==mgg_map[id].frames[m].posy))
						{
							UIData(j,i+meng.scroll,1638,1456,0,255,128,32,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_map[id].frames[m],255,0);
							meng.tex_selection=mgg_map[id].frames[m];
							meng.tex_ID=m;
							meng.tex_MGGID=id;

							if(mgg_map[id].frames[m].vb_id!=-1)
							{
								meng.pre_size.x=mgg_map[id].frames[m].sizex;
								meng.pre_size.y=mgg_map[id].frames[m].sizey;
							}
							else
							{
								meng.pre_size.x=(mgg_map[id].frames[m].w*16384)/st.screenx;
								meng.pre_size.y=(mgg_map[id].frames[m].h*st.gamey)/st.screeny;
							}
						}
						else
							UIData(j,i+meng.scroll,1638,1456,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_map[id].frames[m],255,0);

						if(l<9)
						{
							j+=1638;
							l++;
						}
						else
						{
							j=728;
							i+=1638;
							l=0;
						}
					}
			}
		

		if(st.mouse_wheel>0)
		{
			if(meng.scroll<0) meng.scroll+=1546;
			st.mouse_wheel=0;
		}

		if(st.mouse_wheel<0)
		{
			meng.scroll-=1546;
			st.mouse_wheel=0;
		}
	}
}

void SpriteList()
{
	uint32 m = 0, i = 728, j = 728, k = 0, l = 0, n = 0;

	
		if(meng.scroll<0)
				n=(meng.scroll/1546)*(-10);

		for(n=0;n<st.num_sprites;n++)
		{
			m = st.sprite_id_list[n];
			if(st.Game_Sprites[m].num_start_frames>0)
			{
				for(k=0;k<st.Game_Sprites[m].num_start_frames;k++)
				{
					if((CheckCollisionMouse(j,i+meng.scroll,1638,1638,0) && st.mouse1) || (meng.sprite_frame_selection==st.Game_Sprites[m].frame[k] && meng.sprite_selection==m))
					{
						UIData(j,i+meng.scroll,1638,1456,0,255,128,32,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_game[st.Game_Sprites[m].MGG_ID].frames[st.Game_Sprites[m].frame[k]],255,2);
						StringUIData(st.Game_Sprites[m].name,j,i+meng.scroll+819,2048,2048,0,255,128,32,255,ARIAL,1024,1024,0);
						meng.sprite_selection=m;
						meng.sprite_frame_selection=st.Game_Sprites[m].frame[k];

						meng.spr.health=st.Game_Sprites[m].health;
						meng.spr.body=st.Game_Sprites[m].body;
						meng.spr.flags=st.Game_Sprites[m].flags;

						meng.spr.body.size=st.Game_Sprites[m].body.size;
							//meng.tex_selection=mgg_map[id].frames[m];
							//meng.tex_ID=m;
							//meng.tex_MGGID=id;
					}
					else
					{
						UIData(j,i+meng.scroll,1638,1456,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_game[st.Game_Sprites[m].MGG_ID].frames[st.Game_Sprites[m].frame[k]],255,2);
						StringUIData(st.Game_Sprites[m].name,j,i+meng.scroll+819,2048,2048,0,255,128,32,255,ARIAL,1024,1024,0);
					}

					if(l<9)
					{
						j+=1638;
						l++;
					}
					else
					{
						j=728;
						i+=1638;
						l=0;
					}

				}
			}
			else continue;
		}

		if(st.mouse_wheel>0)
		{
			if(meng.scroll<0) meng.scroll+=1546;
			st.mouse_wheel=0;
		}

		if(st.mouse_wheel<0)
		{
			meng.scroll-=1546;
			st.mouse_wheel=0;
		}
}

static int16 MGGLoad()
{
	uint16 j=0, i, u, check;
	int16 num_files;
	FILE *f;
	DIR *dir;

	int16 id, loaded=0;
	uint16 id2=st.Current_Map.num_mgg, id3=0;

	char files[512][512];
	char *path2;

	size_t size;

	for(i=0;i<MAX_MAP_MGG;i++)
	{
		if(i==MAX_MAP_MGG-1 && mgg_map[i].type!=NONE)
		{
			LogApp("Cannot load MGG, reached max number of map MGGs loaded");
			return 0;
		}

		if(mgg_map[i].type==NONE)
		{
			id=i;
			break;
		}
	}

	num_files=DirFiles(meng.path,files);

	for(i=227;i<num_files*455;i+=455)
	{
		if(j==num_files) break;

		if(CheckCollisionMouse(8192,i+meng.scroll,2730,455,0))
		{
			StringUIData(files[j],8192,i+meng.scroll,0,0,0,255,128,32,255,ARIAL,FONT_SIZE*2,FONT_SIZE*2,0);

			if(st.mouse1)
			{
				size=strlen(files[j]);
				size+=4;
				size+=strlen(meng.path);
				path2=(char*) malloc(size);
				strcpy(path2,meng.path);
				strcat(path2,"//");
				strcat(path2,files[j]);

				//Check if it's a file
				if((f=fopen(path2,"rb"))==NULL)
				{
					//Check if it's a directory
					if((dir=opendir(path2))==NULL)
					{
						LogApp("Error invalid file or directory: %s",files[j]);
						st.mouse1=0;
						j++;
						free(path2);
						continue;
					}
					else
					{
						meng.path=(char*) realloc(meng.path,size);
						strcpy(meng.path,path2);
						closedir(dir);
						meng.scroll=0;
						st.mouse1=0;
						free(path2);
						break;
					}
				}
				else
				{
					fclose(f);
					if(CheckMGGFile(path2))
					{
						check=CheckMGGInSystem(path2);
						if(check>99999)
						{
							st.mouse1=0;
							j++;
							free(path2);
							continue;
						}

						DrawUI(8192,4608,16384,4608,0,0,0,0,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,0);
						DrawString2UI("Loading...",8192,4608,1,1,0,255,255,255,255,ARIAL,FONT_SIZE*3,FONT_SIZE*3,0);
						Renderer(1);
						LoadMGG(&mgg_map[id],path2);

						for(u=0;u<st.Current_Map.num_mgg;u++)
						{
							if(strcmp(meng.mgg_list[u],mgg_map[id].name)==NULL)
							{
								loaded=1;
								break;
							}

						}

						if(loaded==1)
						{
							FreeMGG(&mgg_map[id]);
							st.mouse1=0;
							j++;
							free(path2);
							continue;
						}
						else if(loaded==0);
						{
							strcpy(st.Current_Map.MGG_FILES[st.Current_Map.num_mgg],path2);
							strcpy(meng.mgg_list[st.Current_Map.num_mgg],mgg_map[id].name);
							meng.num_mgg++;
							st.Current_Map.num_mgg++;
							st.num_mgg++;
							LogApp("MGG %s loaded",path2);
							meng.scroll=0;
							//meng.pannel_choice=2;
							meng.command=meng.pannel_choice;
							st.mouse1=0;
							free(path2);
							free(meng.path);
							meng.path=(char*) malloc(2);
							strcpy(meng.path,".");
							break;
						}
					}
					else
					{
						st.mouse1=0;
						j++;
						free(path2);
						continue;
					}
				}
			}
		}
		else
		{
			StringUIData(files[j],8192,i+meng.scroll,0,0,0,255,255,255,255,ARIAL,FONT_SIZE*2,FONT_SIZE*2,0);
		}

		j++;

		if(st.mouse_wheel>0)
		{
			if(meng.scroll<0) meng.scroll+=455;
			st.mouse_wheel=0;
		}

		if(st.mouse_wheel<0)
		{
			meng.scroll-=455;
			st.mouse_wheel=0;
		}
	}
	
	
	if(st.keys[ESC_KEY].state)
	{
		if(meng.path)
		{
			free(meng.path);
			meng.path=(char*) malloc(2);
			strcpy(meng.path,".");
		}

		//if(path2) free(path2);

		meng.command=meng.pannel_choice;
		st.keys[ESC_KEY].state=0;
	}

	return 1;
}

void LightingMisc()
{
	_GAME_LIGHTMAPS *gl = st.game_lightmaps;

	if (st.mouse1)
	{
		for (int i = 1; i < MAX_LIGHTMAPS; i++)
		{
			if (!gl[i].stat)
			{
				gl[i].stat = 2;
					
				gl[i].obj_id = -1;

				gl[i].w_pos = st.mouse;
				STW(&gl[i].w_pos.x, &gl[i].w_pos.y);
				SnapToGrid(&gl[i].w_pos.x, &gl[i].w_pos.y);

				gl[i].w_pos.z = meng.light.l + 24;

				//gl[i].T_w = gl[i].T_h = 

				gl[i].W_w = gl[i].W_h = (float)(meng.light.falloff * (mSqrt(1.0f / meng.light.c) - 1.0f)) * 4;
				STW(&gl[i].W_w, &gl[i].W_h);

				gl[i].falloff[0] = meng.light.falloff;
				gl[i].falloff[2] = meng.light.c;
				//gl[i].color[0] = meng.light.color;
				gl[i].ambient_color = meng.light.color;
				gl[i].falloff[1] = meng.light.intensity;
				gl[i].falloff[4] = meng.light.l;

				gl[i].type = meng.light.type;

				gl[i].spotcos = meng.light.spotang;
				gl[i].spotinnercos = meng.light.spotinnerang;
				meng.light.spotdir.x = gl[i].w_pos.x;
				meng.light.spotdir.y = gl[i].w_pos.y + 1024;
				gl[i].s_dir = meng.light.spotdir;

				int8 lz = meng.light.l + 24;

				meng.z_buffer[lz][meng.z_slot[lz]] = i + 12000;
				meng.z_slot[lz]++;

				st.num_lights++;
				st.mouse1 = 0;
				break;
			}
		}
	}
}

static void ViewPortCommands()
{
	Pos vertextmp[4];
	uint8 got_it=0;
	char str[64];
	const char options[5][16]={"Metal", "Wood", "Plastic", "Concrete", "Organic"};
	int16 i, j, k, l, m;
	static Pos p, p2, s, pm;
	uPos16 p3;
	float tmp;
	static int16 temp;
	static int8 winid;
	static float asp;

	if(meng.command!=DRAW_SECTOR2 && meng.current_command==1)
	{
		st.Current_Map.num_sector--;
		meng.sub_com=0;
		st.Current_Map.sector[meng.com_id].id=-1;
		st.Current_Map.sector[meng.com_id].num_vertexadded=0;
		meng.com_id=0;
		meng.current_command=0;
	}	

	if (!CheckCollisionMouse(2470 / 2, 4885, 2470, 8939, 0) && meng.command != MGG_SEL && !CheckCollisionMouse(8192, 128, 16384, 256, 0)
		&& !CheckCollisionMouse(16384 - 1024, st.gamey / 2, 2048, st.gamey, 0))
	{
		if(meng.command==CAM_LIM_X)
		{
			if(CheckCollisionMouseWorld(st.Current_Map.cam_area.limit[0].x,4608,256,16384,0,24) && st.mouse1)
			{
				p=st.mouse;
				STW(&p.x,&p.y);

				st.Current_Map.cam_area.limit[0].x=p.x;
				//st.mouse1=0;
			}
			else
			if(CheckCollisionMouseWorld(st.Current_Map.cam_area.limit[1].x,4608,256,16384,0,24) && st.mouse1)
			{
				p=st.mouse;
				STW(&p.x,&p.y);

				st.Current_Map.cam_area.limit[1].x=p.x;
				//st.mouse1=0;
			}

			if(st.keys[RETURN_KEY].state)
			{
				meng.command=meng.pannel_choice;
				st.keys[RETURN_KEY].state=0;
			}
		}
		else
		if(meng.command==CAM_LIM_Y)
		{
			if(CheckCollisionMouseWorld(8192,st.Current_Map.cam_area.limit[0].y,32768,256,0,24) && st.mouse1)
			{
				p=st.mouse;
				STW(&p.x,&p.y);

				st.Current_Map.cam_area.limit[0].y=p.y;
				//st.mouse1=0;
			}
			else
			if(CheckCollisionMouseWorld(8192,st.Current_Map.cam_area.limit[1].y,32768,256,0,24) && st.mouse1)
			{
				p=st.mouse;
				STW(&p.x,&p.y);

				st.Current_Map.cam_area.limit[1].y=p.y;
				//st.mouse1=0;
			}

			if(st.keys[RETURN_KEY].state)
			{
				meng.command=meng.pannel_choice;
				st.keys[RETURN_KEY].state=0;
			}
		}
		else
		if(meng.command==CAM_AREA_EDIT)
		{
			st.Current_Map.cam_area.area_pos=st.Camera.position;
			st.Current_Map.cam_area.area_size=st.Camera.dimension;

			if(st.keys[RETURN_KEY].state)
			{
				meng.command=meng.pannel_choice;
				st.keys[RETURN_KEY].state=0;
			}
		}
		else
		if(meng.command==LOAD_LIGHTMAP2)
		{
			i=st.num_lights;
			
			p=st.mouse;
			STW(&p.x,&p.y);

			st.game_lightmaps[i].w_pos=p;

			if(st.mouse1)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
		if (meng.command == NADD_LIGHT)
			LightingMisc();
		else
		if (meng.command == PICK_TAG)
		{
			if (st.Current_Map.num_sprites > 0)
			{
				for (l = 16; l<57; l++)
				{
					if (meng.viewmode == 0 && l>23)
						break;
					else
					if (meng.viewmode == 1 && l<24)
						continue;
					else
					if (meng.viewmode == 1 && l>31)
						break;
					else
					if (meng.viewmode == 2 && l<32)
						continue;
					else
					if (meng.viewmode == 2 && l>39)
						break;
					else
					if (meng.viewmode == 3 && l<40)
						continue;
					else
					if (meng.viewmode == 3 && l>47)
						break;
					else
					if (meng.viewmode == 4 && l<48)
						continue;

					for (k = meng.z_slot[l] - 1; k>-1; k--)
					{
						if (meng.curlayer == 0 && l > 23)
							break;

						if (meng.curlayer == 1 && l<24 || meng.curlayer == 1 && l>31)
							break;

						if (meng.curlayer == 2 && l<32 || meng.curlayer == 2 && l>39)
							break;

						if (meng.curlayer == 3 && l<40 || meng.curlayer == 3 && l>47)
							break;

						if (meng.curlayer == 4 && l < 48)
							break;

						i = meng.z_buffer[l][k];

						if (i<2000 || i>9999) continue;

						i -= 2000;

						if (st.keys[ESC_KEY].state)
						{
							meng.command = SPRITE_TAG;
							meng.picking_tag = 0;
							st.cursor_type = CURSOR_DEFAULT;
						}

						if (CheckCollisionMouseWorld(st.Current_Map.sprites[i].position.x, st.Current_Map.sprites[i].position.y, st.Current_Map.sprites[i].body.size.x, st.Current_Map.sprites[i].body.size.y,
							st.Current_Map.sprites[i].angle, st.Current_Map.sprites[i].position.z) && meng.scaling == 0)
						{
							if (st.mouse1)
							{
								for (j = 0; j < st.Game_Sprites[st.Current_Map.sprites[i].GameID].num_tags; j++)
								{
									if (!strcmp(st.Game_Sprites[st.Current_Map.sprites[i].GameID].tag_names[j], "INPUT") && meng.picking_tag == 1)
									{
										st.Current_Map.sprites[meng.sprite_edit_selection].tags[meng.sub_com] = st.Current_Map.sprites[i].tags[j];
										meng.command = SPRITE_TAG;
										meng.picking_tag = 0;
										st.cursor_type = CURSOR_DEFAULT;
										break;
									}

									if (!strcmp(st.Game_Sprites[st.Current_Map.sprites[i].GameID].tag_names[j], "OUTPUT") && meng.picking_tag == 2)
									{
										st.Current_Map.sprites[meng.sprite_edit_selection].tags[meng.sub_com] = st.Current_Map.sprites[i].tags[j];
										meng.command = SPRITE_TAG;
										st.cursor_type = CURSOR_DEFAULT;
										break;
									}

									if (!strcmp(st.Game_Sprites[st.Current_Map.sprites[i].GameID].tag_names[j],
										st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[meng.sub_com]) && meng.picking_tag == 3)
									{
										st.Current_Map.sprites[meng.sprite_edit_selection].tags[meng.sub_com] = st.Current_Map.sprites[i].tags[j];
										meng.command = SPRITE_TAG;
										st.cursor_type = CURSOR_DEFAULT;
										break;
									}
								}

								break;
							}
							
							if (st.mouse2)
							{
								for (j = 0; j < st.Game_Sprites[st.Current_Map.sprites[i].GameID].num_tags; j++)
								{
									if (!strcmp(st.Game_Sprites[st.Current_Map.sprites[i].GameID].tag_names[j], "INPUT") && meng.picking_tag == 2)
									{
										st.Current_Map.sprites[meng.sprite_edit_selection].tags[meng.sub_com] = st.Current_Map.sprites[i].tags[j];
										meng.command = SPRITE_TAG;
										meng.picking_tag = 0;
										st.cursor_type = CURSOR_DEFAULT;
										break;
									}

									if (!strcmp(st.Game_Sprites[st.Current_Map.sprites[i].GameID].tag_names[j], "OUTPUT") && meng.picking_tag == 1)
									{
										st.Current_Map.sprites[meng.sprite_edit_selection].tags[meng.sub_com] = st.Current_Map.sprites[i].tags[j];
										meng.command = SPRITE_TAG;
										meng.picking_tag = 0;
										st.cursor_type = CURSOR_DEFAULT;
										break;
									}
								}

								break;
							}
						}
					}
				}
			}
		}
		else
		if(meng.command==DRAW_SECTOR)
		{
			if(st.mouse1)
			{
				if(st.Current_Map.num_sector<MAX_SECTORS)
				{
					i=st.Current_Map.num_sector;
					//for(i=0;i<MAX_SECTORS;i++)
					//{
						if(st.Current_Map.sector[i].id==-1)
						{
							st.Current_Map.sector[i].id=1;

							st.Current_Map.sector[i].vertex[0]=st.mouse;
							STW(&st.Current_Map.sector[i].vertex[0].x,&st.Current_Map.sector[i].vertex[0].y);

							SnapToGrid(&st.Current_Map.sector[i].vertex[0].x, &st.Current_Map.sector[i].vertex[0].y);

							st.Current_Map.sector[i].num_vertexadded=1;

							st.Current_Map.num_sector++;

							meng.command=DRAW_SECTOR2;

							meng.current_command=1;

							meng.sub_com=1;
							meng.com_id=i;
							//break;
						}
					//}
				}
				LogApp("First Sector vertex added");
				st.mouse1=0;
			}
		}
		else
		if(meng.command==DRAW_SECTOR2)
		{
			i=meng.com_id;

			if(st.mouse1)
			{
				if(meng.sub_com==1)
				{
					if(st.keys[LSHIFT_KEY].state)
					{
						st.Current_Map.sector[i].vertex[meng.sub_com]=st.mouse;
						STW(&st.Current_Map.sector[i].vertex[meng.sub_com].x,&st.Current_Map.sector[i].vertex[meng.sub_com].y);
						SnapToGrid(&st.Current_Map.sector[i].vertex[meng.sub_com].x, &st.Current_Map.sector[i].vertex[meng.sub_com].y);
						st.Current_Map.sector[i].sloped=1;
					}
					else
					{
						st.Current_Map.sector[i].vertex[meng.sub_com].x=st.mouse.x;
						STW(&st.Current_Map.sector[i].vertex[meng.sub_com].x,&st.Current_Map.sector[i].vertex[meng.sub_com].y);
						SnapToGrid(&st.Current_Map.sector[i].vertex[meng.sub_com].x, &st.Current_Map.sector[i].vertex[meng.sub_com].y);
						st.Current_Map.sector[i].vertex[meng.sub_com].y=st.Current_Map.sector[i].vertex[0].y;
						st.Current_Map.sector[i].base_y=st.Current_Map.sector[i].vertex[0].y;
						st.Current_Map.sector[i].sloped=0;
					}

					st.Current_Map.sector[i].num_vertexadded++;

					meng.command=DRAW_SECTOR;
					meng.sub_com=0;
					meng.com_id=0;

					st.mouse1=0;

					meng.z_buffer[24][meng.z_slot[24]]=i+10000;
					meng.z_slot[24]++;

					meng.current_command=0;

					LogApp("Sector Added");
				}
			}
		}
		else
		if(meng.command==SELECT_EDIT)
		{
			/*
			if(meng.command2==EDIT_SECTOR)
			{
				i=meng.com_id;

				if(!st.Current_Map.sector[i].destructive)
				{
					if(UIWin2_MarkBox(winid,0,0,"Destructable",UI_COL_NORMAL,UI_COL_SELECTED)==2)
						st.Current_Map.sector[i].destructive=1;
				}
				else
				{
					if(UIWin2_MarkBox(winid,0,1,"Destructable",UI_COL_NORMAL,UI_COL_SELECTED)==1)
						st.Current_Map.sector[i].destructive=0;
				}

				UIWin2_NumberBoxi16(winid,1,&st.Current_Map.sector[i].tag,"Tag:",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);

				switch(st.Current_Map.sector[i].material)
				{
					case METAL:
						sprintf(str,"Material: Metal");
						break;

					case CONCRETE:
						sprintf(str,"Material: Concrete");
						break;

					case WOOD:
						sprintf(str,"Material: Wood");
						break;

					case PLASTIC:
						sprintf(str,"Material: Plastic");
						break;

					case ORGANIC:
						sprintf(str,"Material: Organic");
						break;
				}

				if(UIWin2_StringButton(winid,2,str,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
					meng.sub_com=1;

				if(meng.sub_com==1)
				{
					switch(UIOptionBox(8192+2048,4608,CUSTOM,options,5,ARIAL,1536,UI_COL_NORMAL,UI_COL_SELECTED))
					{
						case UI_SEL:
							st.Current_Map.sector[i].material=METAL;
							meng.sub_com=0;
							break;

						case UI_SEL+1:
							st.Current_Map.sector[i].material=WOOD;
							meng.sub_com=0;
							break;

						case UI_SEL+2:
							st.Current_Map.sector[i].material=PLASTIC;
							meng.sub_com=0;
							break;

						case UI_SEL+3:
							st.Current_Map.sector[i].material=CONCRETE;
							meng.sub_com=0;
							break;

						case UI_SEL+4:
							st.Current_Map.sector[i].material=ORGANIC;
							meng.sub_com=0;
							break;
					}
				}

				if(UIWin2_StringButton(winid,3,"Done",UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
				{
					meng.command2=meng.command;
					UIDestroyWindow(winid);
				}
			}
			*/
			if (st.Current_Map.num_sector > 0)
			{
				l=24;
				//for(l=16;l<58;l++)
				//{
				for (k = meng.z_slot[l]; k > -1; k--)
					{
						if (meng.viewmode != 1 && meng.viewmode < 5)
							break;

						if (meng.curlayer == 1 && l<24 || meng.curlayer == 1 && l>31)
							break;

						i = meng.z_buffer[l][k];

						if(i < 10000 || i > 12000)
							continue;

						i -= 10000;

						if (st.mouse1 && meng.got_it == i + 10000)
						{
							p = st.mouse;

							STW(&p.x, &p.y);

							if (meng.sector_edit_selection < 1000)
							{
								if (st.Current_Map.sector[i].sloped)
									st.Current_Map.sector[i].vertex[0] = p;
								else
								{
									if (st.keys[LSHIFT_KEY].state)
									{
										st.Current_Map.sector[i].vertex[0] = p;
										st.Current_Map.sector[i].vertex[1].y = st.Current_Map.sector[i].base_y = p.y;
									}

									if (st.keys[LCTRL_KEY].state)
									{
										st.Current_Map.sector[i].vertex[1].y = st.Current_Map.sector[i].base_y = p.y;
										st.Current_Map.sector[i].vertex[1].x = (st.Current_Map.sector[i].vertex[1].x - st.Current_Map.sector[i].vertex[0].x) + p.x;
										st.Current_Map.sector[i].vertex[0] = p;
									}

									if (!st.keys[LCTRL_KEY].state && !st.keys[LSHIFT_KEY].state)
										st.Current_Map.sector[i].vertex[0].x = p.x;
								}
							}
							
							if (meng.sector_edit_selection >= 1000)
							{
								if (st.Current_Map.sector[i].sloped)
									st.Current_Map.sector[i].vertex[1] = p;
								else
								{
									if (st.keys[LSHIFT_KEY].state)
									{
										st.Current_Map.sector[i].vertex[1] = p;
										st.Current_Map.sector[i].vertex[0].y = st.Current_Map.sector[i].base_y = p.y;
									}

									if (st.keys[LCTRL_KEY].state)
									{
										st.Current_Map.sector[i].vertex[0].y = st.Current_Map.sector[i].base_y = p.y;
										st.Current_Map.sector[i].vertex[0].x = p.x - (st.Current_Map.sector[i].vertex[1].x - st.Current_Map.sector[i].vertex[0].x);
										st.Current_Map.sector[i].vertex[1] = p;
									}

									if (!st.keys[LCTRL_KEY].state && !st.keys[LSHIFT_KEY].state)
										st.Current_Map.sector[i].vertex[1].x = p.x;
								}
							}
						}

						if((CheckCollisionMouseWorld(st.Current_Map.sector[i].vertex[0].x,st.Current_Map.sector[i].vertex[0].y,484,484,0,0) || 
							CheckCollisionMouseWorld(st.Current_Map.sector[i].vertex[1].x,st.Current_Map.sector[i].vertex[1].y,484,484,0,0)))
						{
							if(st.mouse1)
							{
								if (meng.got_it == -1)
									meng.got_it = i + 10000;
								
								if (meng.got_it != -1 && meng.got_it != i + 10000)
									continue;

								if (meng.command2 != EDIT_SECTOR && meng.sub_com == SCALER_SELECT)
									continue;

								p=st.mouse; 

								STW(&p.x,&p.y);
								SnapToGrid(&p.x, &p.y);

								memset(meng.layers, 0, 57 * 2048 * 2);
								meng.layers[l][k] = 1;

								if(CheckCollisionMouseWorld(st.Current_Map.sector[i].vertex[0].x,st.Current_Map.sector[i].vertex[0].y,484,484,0,0))
								{
									got_it = 1;

									meng.command2 = EDIT_SECTOR;
									meng.com_id = i;
									meng.sector_edit_selection = i;

									if(st.Current_Map.sector[i].sloped)
										st.Current_Map.sector[i].vertex[0]=p;
									else
									{
										if(st.keys[LSHIFT_KEY].state)
										{
											st.Current_Map.sector[i].vertex[0]=p;
											st.Current_Map.sector[i].vertex[1].y=st.Current_Map.sector[i].base_y=p.y;
										}
										
										if(st.keys[LCTRL_KEY].state)
										{
											st.Current_Map.sector[i].vertex[1].y=st.Current_Map.sector[i].base_y=p.y;
											st.Current_Map.sector[i].vertex[1].x=(st.Current_Map.sector[i].vertex[1].x-st.Current_Map.sector[i].vertex[0].x)+p.x;
											st.Current_Map.sector[i].vertex[0]=p;
										}

										if(!st.keys[LCTRL_KEY].state && !st.keys[LSHIFT_KEY].state)
											st.Current_Map.sector[i].vertex[0].x=p.x;
									}

								}
								else
								if(CheckCollisionMouseWorld(st.Current_Map.sector[i].vertex[1].x,st.Current_Map.sector[i].vertex[1].y,484,484,0,0))
								{
									got_it = 1;

									meng.command2 = EDIT_SECTOR;
									meng.com_id = i;
									meng.sector_edit_selection = i + 1000;

									if(st.Current_Map.sector[i].sloped)
										st.Current_Map.sector[i].vertex[1]=p;
									else
									{
										if(st.keys[LSHIFT_KEY].state)
										{
											st.Current_Map.sector[i].vertex[1]=p;
											st.Current_Map.sector[i].vertex[0].y=st.Current_Map.sector[i].base_y=p.y;
										}

										if(st.keys[LCTRL_KEY].state)
										{
											st.Current_Map.sector[i].vertex[0].y=st.Current_Map.sector[i].base_y=p.y;
											st.Current_Map.sector[i].vertex[0].x=p.x-(st.Current_Map.sector[i].vertex[1].x-st.Current_Map.sector[i].vertex[0].x);
											st.Current_Map.sector[i].vertex[1]=p;
										}

										if(!st.keys[LCTRL_KEY].state && !st.keys[LSHIFT_KEY].state)
											st.Current_Map.sector[i].vertex[1].x=p.x;
									}
								}

								if(st.keys[DELETE_KEY].state)
								{
									st.Current_Map.sector[i].id=-1;

									if(i<st.Current_Map.num_sector-1)
									{
										for(j=i;j<st.Current_Map.num_sector-1;j++)
										{
											memcpy(&st.Current_Map.sector[j],&st.Current_Map.sector[j+1],sizeof(_SECTOR));
											st.Current_Map.sector[j+1].id=-1;
										}
									}

									for(k=0;k<meng.z_slot[24];k++)
										if(meng.z_buffer[24][k]==st.Current_Map.num_sector-1+10000)
											break;

									if(k<meng.z_slot[24]-1)
									{
										for(;k<meng.z_slot[24];k++)
										{
											meng.z_buffer[24][k]=meng.z_buffer[24][k+1];
											meng.z_buffer[24][k+1]=0;
										}
									}
									else
										meng.z_buffer[24][k]=0;

									meng.z_slot[24]--;

									st.Current_Map.num_sector--;
									st.keys[DELETE_KEY].state=0;

									break;
								}
							}
							//else
								//meng.got_it=-1;

							//if(st.mouse2)
							//{
								//meng.command2=EDIT_SECTOR;
								//meng.com_id=i;
								//winid=UICreateWindow2(0,0,CENTER,6,4,2048,32,ARIAL);
								//st.mouse2=0;
								//break;
							//}
						}

						if(got_it) break;
					}
				//}
			}
			
			if (st.num_lights > 0)
			{
				for (l = 24; l < 32; l++)
				{
					for (k = meng.z_slot[l] - 1; k>-1; k--)
					{
						if (meng.curlayer != MIDGROUND_MODE || l < 24 || l > 31)
							break;

						i = meng.z_buffer[l][k];

						if (i < 12000)
							break;

						i -= 12000;

						_GAME_LIGHTMAPS *gl = st.game_lightmaps;

						if (got_it) break;

						if (st.mouse1 && meng.got_it == i + 12000)
						{
							p = st.mouse;

							STW(&p.x, &p.y);

							meng.com_id = i;

							Pos dif;

							dif.x = gl[i].s_dir.x - gl[i].w_pos.x;
							dif.y = gl[i].s_dir.y - gl[i].w_pos.y;

							gl[i].w_pos.x = p.x;
							gl[i].w_pos.y = p.y;

							gl[i].w_pos.x -= meng.p.x;
							gl[i].w_pos.y -= meng.p.y;

							gl[i].s_dir.x = gl[i].w_pos.x + dif.x;
							gl[i].s_dir.y = gl[i].w_pos.y + dif.y;

							p.x = gl[i].w_pos.x + 150;
							p.y = gl[i].w_pos.y - 150;
						}
						else if (st.mouse1 && meng.got_it == i + 12000 + 256)
						{
							p = st.mouse;

							STW(&p.x, &p.y);

							meng.com_id = i;

							gl[i].s_dir.x = p.x;
							gl[i].s_dir.y = p.y;

							gl[i].s_dir.x -= meng.p.x;
							gl[i].s_dir.y -= meng.p.y;

							p.x = gl[i].s_dir.x + 150;
							p.y = gl[i].s_dir.y - 150;
						}

						if (CheckCollisionMouseWorld(gl[i].w_pos.x, gl[i].w_pos.y, 300, 300, 0, l))
						{
							if (st.mouse1)
							{
								if (meng.command2 != NEDIT_LIGHT && meng.sub_com == SCALER_SELECT)
									continue;

								memset(meng.layers, 0, 57 * 2048 * 2);
								meng.layers[l][k] = 1;

								if (meng.got_it == -1)
								{
									meng.command2 = NEDIT_LIGHT;
									meng.p = st.mouse;
									meng.got_it = i + 12000;

									meng.light_edit_selection = i;

									STW(&meng.p.x, &meng.p.y);

									meng.p.x -= gl[i].w_pos.x;
									meng.p.y -= gl[i].w_pos.y;
								}

								if (meng.got_it != -1 && meng.got_it != i + 12000)
									continue;

								p = st.mouse;

								STW(&p.x, &p.y);

								meng.com_id = i;

								Pos dif;

								dif.x = gl[i].s_dir.x - gl[i].w_pos.x;
								dif.y = gl[i].s_dir.y - gl[i].w_pos.y;

								gl[i].w_pos.x = p.x;
								gl[i].w_pos.y = p.y;

								gl[i].w_pos.x -= meng.p.x;
								gl[i].w_pos.y -= meng.p.y;

								p.x = gl[i].w_pos.x + 150;
								p.y = gl[i].w_pos.y - 150;

								gl[i].s_dir.x = gl[i].w_pos.x + dif.x;
								gl[i].s_dir.y = gl[i].w_pos.y + dif.y;

								got_it = 1;
								break;
							}
						}

						if (gl[i].type == SPOTLIGHT && CheckCollisionMouseWorld(gl[i].s_dir.x, gl[i].s_dir.y, 128, 128, 0, l))
						{
							if (st.mouse1)
							{
								if (meng.command2 != NEDIT_LIGHT && meng.sub_com == SCALER_SELECT)
									continue;

								memset(meng.layers, 0, 57 * 2048 * 2);
								meng.layers[l][k] = 1;

								if (meng.got_it == -1)
								{
									meng.command2 = NEDIT_LIGHT;
									meng.p = st.mouse;
									meng.got_it = i + 12000 + 256;

									meng.light_edit_selection = i;

									STW(&meng.p.x, &meng.p.y);

									meng.p.x -= gl[i].s_dir.x;
									meng.p.y -= gl[i].s_dir.y;
								}

								if (meng.got_it != -1 && meng.got_it != i + 12000 + 256)
									continue;

								p = st.mouse;

								STW(&p.x, &p.y);

								meng.com_id = i;

								gl[i].s_dir.x = p.x;
								gl[i].s_dir.y = p.y;

								gl[i].s_dir.x -= meng.p.x;
								gl[i].s_dir.y -= meng.p.y;

								p.x = gl[i].s_dir.x + 150;
								p.y = gl[i].s_dir.y - 150;

								got_it = 1;
								break;
							}
						}
					}
				}
			}

			if(st.Current_Map.num_sprites>0)
			{
				for(l=16;l<57;l++)
				{
					if(meng.viewmode==0 && l>23)
						break;
					else
					if(meng.viewmode==1 && l<24)
						continue;
					else
					if(meng.viewmode==1 && l>31)
						break;
					else
					if(meng.viewmode==2 && l<32)
						continue;
					else
					if(meng.viewmode==2 && l>39)
						break;
					else
					if(meng.viewmode==3 && l<40)
						continue;
					else
					if(meng.viewmode==3 && l>47)
						break;
					else
					if(meng.viewmode==4 && l<48)
						continue;

					for(k=meng.z_slot[l]-1;k>-1;k--)
					{
						if (meng.curlayer == 0 && l>23)
							break;

						if (meng.curlayer == 1 && l<24 || meng.curlayer == 1 && l>31)
							break;

						if (meng.curlayer == 2 && l<32 || meng.curlayer == 2 && l>39)
							break;

						if (meng.curlayer == 3 && l<40 || meng.curlayer == 3 && l>47)
							break;

						if (meng.curlayer == 4 && l<48)
							break;

						i=meng.z_buffer[l][k];

						if(i<2000 || i>9999) continue;

						i-=2000;
						/*
						if(st.Current_Map.sprites[i].flags & 1)
						{
							if(Sys_ResizeController(st.Current_Map.sprites[i].position.x,st.Current_Map.sprites[i].position.y,&st.Current_Map.sprites[i].body.size.x,&st.Current_Map.sprites[i].body.size.y,
								0,0,st.Current_Map.sprites[i].position.z))
								break;
						}
						*/

						if(got_it) break;

						if (st.mouse1 && meng.got_it == i + 2000 && meng.scaling == 0 && meng.sub_com != OBJEXTRUDE)
						{
							p = st.mouse;

							STW(&p.x, &p.y);

							meng.com_id = i;

							st.Current_Map.sprites[i].position.x = p.x;
							st.Current_Map.sprites[i].position.y = p.y;

							st.Current_Map.sprites[i].position.x -= meng.p.x;
							st.Current_Map.sprites[i].position.y -= meng.p.y;

							SnapToGrid(&st.Current_Map.sprites[i].position.x, &st.Current_Map.sprites[i].position.y);
							PositionToEdge(&st.Current_Map.sprites[i].position.x, &st.Current_Map.sprites[i].position.y, st.Current_Map.sprites[i].body.size.x, st.Current_Map.sprites[i].body.size.y);

							p.x = st.Current_Map.sprites[i].position.x + (st.Current_Map.sprites[i].body.size.x / 2);
							p.y = st.Current_Map.sprites[i].position.y - (st.Current_Map.sprites[i].body.size.y / 2);
						}

						if(CheckCollisionMouseWorld(st.Current_Map.sprites[i].position.x,st.Current_Map.sprites[i].position.y,st.Current_Map.sprites[i].body.size.x,st.Current_Map.sprites[i].body.size.y,
							st.Current_Map.sprites[i].angle,st.Current_Map.sprites[i].position.z) && meng.scaling == 0)
						{
							if(st.mouse1)
							{
								if (meng.command2 != EDIT_SPRITE && meng.sub_com == SCALER_SELECT)
									continue;

								memset(meng.layers, 0, 57 * 2048 * 2);
								meng.layers[l][k] = 1;

								if(meng.got_it==-1)
								{
									meng.command2=EDIT_SPRITE;
									meng.p=st.mouse;
									meng.got_it = i + 2000;

									meng.sprite_edit_selection=i;

									STW(&meng.p.x, &meng.p.y);

									SnapToGrid(&meng.p.x, &meng.p.y);
									PositionToEdge(&meng.p.x, &meng.p.y, st.Current_Map.sprites[i].body.size.x, st.Current_Map.sprites[i].body.size.y);

									meng.p.x-=st.Current_Map.sprites[i].position.x;
									meng.p.y-=st.Current_Map.sprites[i].position.y;
								}
								
								if(meng.got_it != -1 && meng.got_it != i + 2000)
									continue;

								p=st.mouse;

								STW(&p.x, &p.y);

								meng.com_id=i;

								st.Current_Map.sprites[i].position.x=p.x;
								st.Current_Map.sprites[i].position.y=p.y;

								st.Current_Map.sprites[i].position.x-=meng.p.x;
								st.Current_Map.sprites[i].position.y-=meng.p.y;

								SnapToGrid(&st.Current_Map.sprites[i].position.x, &st.Current_Map.sprites[i].position.y);
								PositionToEdge(&st.Current_Map.sprites[i].position.x, &st.Current_Map.sprites[i].position.y, st.Current_Map.sprites[i].body.size.x, st.Current_Map.sprites[i].body.size.y);

								p.x=st.Current_Map.sprites[i].position.x+(st.Current_Map.sprites[i].body.size.x/2);
								p.y=st.Current_Map.sprites[i].position.y-(st.Current_Map.sprites[i].body.size.y/2);

								got_it = 1;
								break;
							}
							//else
								//meng.got_it=-1;
						}

						if (meng.sub_com == SCALER_SELECT && meng.command2 == EDIT_SPRITE && meng.sprite_edit_selection == i)
						{
							p = st.Current_Map.sprites[i].position;
							p2 = st.Current_Map.sprites[i].body.size;

							//p.x -= (float)st.Camera.position.x*st.Current_Map.fr_v;
							//p.y -= (float)st.Camera.position.y*st.Current_Map.fr_v;

							if (st.keys[RETURN_KEY].state)
							{
								meng.sub_com = 0;
								meng.scaling = 0;
							}

							if (p.z > 31 && p.z < 40)
							{
								p.x -= (float)st.Camera.position.x*st.Current_Map.bck1_v;
								p.y -= (float)st.Camera.position.y*st.Current_Map.bck1_v;
							}

							if (p.z > 39 && p.z < 48)
							{
								p.x -= (float)st.Camera.position.x * st.Current_Map.bck2_v;
								p.y -= (float)st.Camera.position.y * st.Current_Map.bck2_v;

								p.x += st.Camera.position.x;
								p.y += st.Camera.position.y;
							}

							if (p.z > 47)
							{
								p.x /= (float)st.Camera.dimension.x;
								p.y /= (float)st.Camera.dimension.y;

								p.x += st.Camera.position.x;
								p.y += st.Camera.position.y;

								p2.x /= (float)st.Camera.dimension.x;
								p2.y /= (float)st.Camera.dimension.y;
							}

							//UIData(p.x, p.x, p.y + (p2.y / 2), 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 255, 0, 0, 0, 0, 32768, 32768, st.BasicTex, 255, 0);

							if (meng.scaling == 1)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);
									//p.x -= pm.x;
									p.y -= pm.y;

									p.y *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.y *= -1;
											p.y += pm.y;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.sprites[i].body.size.y = abs(p.y - st.Current_Map.sprites[i].position.y) * 2;
										}
										else
											st.Current_Map.sprites[i].body.size.y += p.y;
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.y *= -1;
											p.y += pm.y;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.sprites[i].body.size.y = abs(p.y - st.Current_Map.sprites[i].position.y) * 2;
											st.Current_Map.sprites[i].body.size.x = (float)st.Current_Map.sprites[i].body.size.y * asp;
										}
										else
										{
											st.Current_Map.sprites[i].body.size.x += p.y;
											st.Current_Map.sprites[i].body.size.y = (float)st.Current_Map.sprites[i].body.size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.scaling == 2)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);
									//p.x -= pm.x;
									p.y -= pm.y;

									//p.y *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.y *= -1;
											p.y += pm.y;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.sprites[i].body.size.y = abs(p.y - st.Current_Map.sprites[i].position.y) * 2;
										}
										else
											st.Current_Map.sprites[i].body.size.y += p.y;
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.y *= -1;
											p.y += pm.y;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.sprites[i].body.size.y = abs(p.y - st.Current_Map.sprites[i].position.y) * 2;
											st.Current_Map.sprites[i].body.size.x = (float)st.Current_Map.sprites[i].body.size.y * asp;
										}
										else
										{
											st.Current_Map.sprites[i].body.size.x += p.y;
											st.Current_Map.sprites[i].body.size.y = (float)st.Current_Map.sprites[i].body.size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.scaling == 3)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);
									p.x -= pm.x;
									//p.y -= pm.y;

									p.x *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.sprites[i].body.size.x = abs(p.x - st.Current_Map.sprites[i].position.x) * 2;
										}
										else
											st.Current_Map.sprites[i].body.size.x += p.x;
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.sprites[i].body.size.x = abs(p.x - st.Current_Map.sprites[i].position.x) * 2;
											st.Current_Map.sprites[i].body.size.y = (float)st.Current_Map.sprites[i].body.size.x / asp;
										}
										else
										{
											st.Current_Map.sprites[i].body.size.x += p.x;
											st.Current_Map.sprites[i].body.size.y = (float)st.Current_Map.sprites[i].body.size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.scaling == 4)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);
									p.x -= pm.x;
									//p.y -= pm.y;

									//p.x *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.sprites[i].body.size.x = abs(p.x - st.Current_Map.sprites[i].position.x) * 2;
										}
										else
											st.Current_Map.sprites[i].body.size.x += p.x;
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.sprites[i].body.size.x = abs(p.x - st.Current_Map.sprites[i].position.x) * 2;
											st.Current_Map.sprites[i].body.size.y = (float)st.Current_Map.sprites[i].body.size.x / asp;
										}
										else
										{
											st.Current_Map.sprites[i].body.size.x += p.x;
											st.Current_Map.sprites[i].body.size.y = (float)st.Current_Map.sprites[i].body.size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.scaling == 5)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);
									p.x -= pm.x;
									p.y -= pm.y;

									p.x *= -1;
									p.y *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.sprites[i].body.size.x = abs(p.x - st.Current_Map.sprites[i].position.x) * 2;
										}
										else
										{
											st.Current_Map.sprites[i].body.size.x += p.x;
											st.Current_Map.sprites[i].body.size.y += p.y;
										}
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.sprites[i].body.size.x = abs(p.x - st.Current_Map.sprites[i].position.x) * 2;
											st.Current_Map.sprites[i].body.size.y = (float)st.Current_Map.sprites[i].body.size.x / asp;
										}
										else
										{
											st.Current_Map.sprites[i].body.size.x += p.y;
											st.Current_Map.sprites[i].body.size.y = (float)st.Current_Map.sprites[i].body.size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.scaling == 6)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);
									p.x -= pm.x;
									p.y -= pm.y;

									//p.x *= -1;
									p.y *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.sprites[i].body.size.x = abs(p.x - st.Current_Map.sprites[i].position.x) * 2;
										}
										else
										{
											st.Current_Map.sprites[i].body.size.x += p.x;
											st.Current_Map.sprites[i].body.size.y += p.y;
										}
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.sprites[i].body.size.x = abs(p.x - st.Current_Map.sprites[i].position.x) * 2;
											st.Current_Map.sprites[i].body.size.y = (float)st.Current_Map.sprites[i].body.size.x / asp;
										}
										else
										{
											st.Current_Map.sprites[i].body.size.x += p.y;
											st.Current_Map.sprites[i].body.size.y = (float)st.Current_Map.sprites[i].body.size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.scaling == 7)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);
									p.x -= pm.x;
									p.y -= pm.y;

									//p.x *= -1;
									//p.y *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.sprites[i].body.size.x = abs(p.x - st.Current_Map.sprites[i].position.x) * 2;
										}
										else
										{
											st.Current_Map.sprites[i].body.size.x += p.x;
											st.Current_Map.sprites[i].body.size.y += p.y;
										}
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.sprites[i].body.size.x = abs(p.x - st.Current_Map.sprites[i].position.x) * 2;
											st.Current_Map.sprites[i].body.size.y = (float)st.Current_Map.sprites[i].body.size.x / asp;
										}
										else
										{
											st.Current_Map.sprites[i].body.size.x += p.y;
											st.Current_Map.sprites[i].body.size.y = (float)st.Current_Map.sprites[i].body.size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.scaling == 8)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);
									p.x -= pm.x;
									p.y -= pm.y;

									p.x *= -1;
									//p.y *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.sprites[i].body.size.x = abs(p.x - st.Current_Map.sprites[i].position.x) * 2;
										}
										else
										{
											st.Current_Map.sprites[i].body.size.x += p.x;
											st.Current_Map.sprites[i].body.size.y += p.y;
										}
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.sprites[i].body.size.x = abs(p.x - st.Current_Map.sprites[i].position.x) * 2;
											st.Current_Map.sprites[i].body.size.y = (float)st.Current_Map.sprites[i].body.size.x / asp;
										}
										else
										{
											st.Current_Map.sprites[i].body.size.x += p.y;
											st.Current_Map.sprites[i].body.size.y = (float)st.Current_Map.sprites[i].body.size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (CheckCollisionMouseWorld(p.x, p.y - (p2.y / 2) - 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 1;

										asp = (float) st.Current_Map.sprites[i].body.size.x / st.Current_Map.sprites[i].body.size.y;

										st.cursor_type = CURSOR_S_UD;
									}
								}
							}

							if (CheckCollisionMouseWorld(p.x, p.y + (p2.y / 2) + 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 2;

										asp = (float)st.Current_Map.sprites[i].body.size.x / st.Current_Map.sprites[i].body.size.y;

										st.cursor_type = CURSOR_S_UD;
									}
								}
							}

							if (CheckCollisionMouseWorld(p.x - (p2.x / 2) - 96, p.y, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 3;

										asp = (float)st.Current_Map.sprites[i].body.size.x / st.Current_Map.sprites[i].body.size.y;

										st.cursor_type = CURSOR_S_LR;
									}
								}
							}

							if (CheckCollisionMouseWorld(p.x + (p2.x / 2) + 96, p.y, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 4;

										asp = (float)st.Current_Map.sprites[i].body.size.x / st.Current_Map.sprites[i].body.size.y;

										st.cursor_type = CURSOR_S_LR;
									}
								}
							}

							if (CheckCollisionMouseWorld(p.x - (p2.x / 2) - 96, p.y - (p2.y / 2) - 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 5;

										asp = (float)st.Current_Map.sprites[i].body.size.x / st.Current_Map.sprites[i].body.size.y;

										st.cursor_type = CURSOR_S_C_LR;
									}
								}
							}

							if (CheckCollisionMouseWorld(p.x + (p2.x / 2) + 96, p.y - (p2.y / 2) - 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 6;

										asp = (float)st.Current_Map.sprites[i].body.size.x / st.Current_Map.sprites[i].body.size.y;

										st.cursor_type = CURSOR_S_C_RL;
									}
								}
							}

							if (CheckCollisionMouseWorld(p.x + (p2.x / 2) + 96, p.y + (p2.y / 2) + 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 7;

										asp = (float)st.Current_Map.sprites[i].body.size.x / st.Current_Map.sprites[i].body.size.y;

										st.cursor_type = CURSOR_S_C_LR;
									}
								}
							}

							if (CheckCollisionMouseWorld(p.x - (p2.x / 2) - 96, p.y + (p2.y / 2) + 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 8;

										asp = (float)st.Current_Map.sprites[i].body.size.x / st.Current_Map.sprites[i].body.size.y;

										st.cursor_type = CURSOR_S_C_RL;
									}
								}
							}
						}
					}
				}
			}

			if(st.Current_Map.num_obj > 0)
			{
				for (l = 16; l<57; l++)
				{
					if(meng.viewmode==0 && l>23)
						break;
					else
					if(meng.viewmode==1 && l<24)
						continue;
					else
					if(meng.viewmode==1 && l>31)
						break;
					else
					if(meng.viewmode==2 && l<32)
						continue;
					else
					if(meng.viewmode==2 && l>39)
						break;
					else
					if(meng.viewmode==3 && l<40)
						continue;
					else
					if(meng.viewmode==3 && l>47)
						break;
					else
					if(meng.viewmode==4 && l<48)
						continue;

					for(k=meng.z_slot[l]-1;k>-1;k--)
					{
						if (meng.curlayer == 0 && l>23)
							break;

						if (meng.curlayer == 1 && l<24 || meng.curlayer == 1 && l>31)
							break;

						if (meng.curlayer == 2 && l<32 || meng.curlayer == 2 && l>39)
							break;

						if (meng.curlayer == 3 && l<40 || meng.curlayer == 3 && l>47)
							break;

						if (meng.curlayer == 4 && l<48)
							break;

						i=meng.z_buffer[l][k];

						if(i>1099) continue;

						if(got_it) break;

						if (st.mouse1 && meng.got_it == i && meng.sub_com != OBJEXTRUDE && meng.extruding == 0)
						{
							p = st.mouse;

							STW(&p.x, &p.y);

							meng.com_id = i;

							st.Current_Map.obj[i].position.x = p.x;
							st.Current_Map.obj[i].position.y = p.y;

							st.Current_Map.obj[i].position.x -= meng.p.x;
							st.Current_Map.obj[i].position.y -= meng.p.y;

							SnapToGrid(&st.Current_Map.obj[i].position.x, &st.Current_Map.obj[i].position.y);
							PositionToEdge(&st.Current_Map.obj[i].position.x, &st.Current_Map.obj[i].position.y, st.Current_Map.obj[i].size.x, st.Current_Map.obj[i].size.y);
						
							p.x = st.Current_Map.obj[i].position.x + (st.Current_Map.obj[i].size.x / 2);
							p.y = st.Current_Map.obj[i].position.y - (st.Current_Map.obj[i].size.y / 2);
						}

						if (meng.sub_com == OBJEXTRUDE && meng.command2 == EDIT_OBJ && meng.obj_edit_selection == i)
						{
							p = st.Current_Map.obj[i].position;
							Pos s = st.Current_Map.obj[i].size;

							//AddCamCalc(&p, &s);

							if (CheckCollisionMouseWorld(p.x + 512 + s.x / 2, p.y, 512, 512, 0, 2) && meng.sub_com == OBJEXTRUDE)
							{
								meng.extruding = 1;
								st.cursor_type = CURSOR_S_LR;

								pm = st.mouse;
								STW(&pm.x, &pm.y);
								SnapToGrid(&pm.x, &pm.y);
							}

							if (CheckCollisionMouseWorld(p.x - 512 - s.x / 2, p.y, 512, 512, 0, 2) && meng.sub_com == OBJEXTRUDE)
							{
								meng.extruding = 2;
								st.cursor_type = CURSOR_S_LR;

								pm = st.mouse;
								STW(&pm.x, &pm.y);
								SnapToGrid(&pm.x, &pm.y);
							}

							if (CheckCollisionMouseWorld(p.x - 600, p.y - 512 - s.y / 2, 512, 512, 0, 2) && meng.sub_com == OBJEXTRUDE)
							{
								meng.extruding = 3;
								st.cursor_type = CURSOR_S_UD;

								pm = st.mouse;
								STW(&pm.x, &pm.y);
								SnapToGrid(&pm.x, &pm.y);
							}

							if (CheckCollisionMouseWorld(p.x - 600, p.y + 512 + s.y / 2, 512, 512, 0, 2) && meng.sub_com == OBJEXTRUDE)
							{
								meng.extruding = 4;
								st.cursor_type = CURSOR_S_UD;

								pm = st.mouse;
								STW(&pm.x, &pm.y);
								SnapToGrid(&pm.x, &pm.y);
							}

							if (CheckCollisionMouseWorld(p.x + 512 + s.x / 2, p.y - 512, 512, 512, 0, 2) && st.mouse1 && meng.sub_com == OBJEXTRUDE)
							{
								//meng.extruding = 2;

								int32 sx3, sy3;
								TEX_DATA d = mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID];

								if (d.vb_id != -1)
								{
									sx3 = d.sizex;
									sy3 = d.sizey;
								}
								else
								{
									sx3 = (float)((d.w * 16384) / st.screenx);
									sy3 = (float)((d.h * st.gamey) / st.screeny);
								}

								float sx = ((float)32768.0f * st.Current_Map.obj[i].size.x) /
									(float)(((float)mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].w /
									mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].h) * sy3);

								sx = ceil(sx / 32768.0f);

								int32 tx = st.Current_Map.obj[i].texsize.x;

								st.Current_Map.obj[i].texsize.x = sx * 32768;

								if (tx == st.Current_Map.obj[i].texsize.x)
								{
									st.Current_Map.obj[i].texsize.x = (sx + 1) * 32768;
									sx += 1;
								}

								int32 sx2 = st.Current_Map.obj[i].size.x;

								st.Current_Map.obj[i].size.x = sx * (float)(((float)mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].w /
									mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].h) * sy3);

								st.Current_Map.obj[i].position.x -= (sx2 - st.Current_Map.obj[i].size.x) / 2;
							}

							if (CheckCollisionMouseWorld(p.x - 512 - s.x / 2, p.y - 512, 512, 512, 0, 2) && st.mouse1 && meng.sub_com == OBJEXTRUDE)
							{
								//meng.extruding = 2;

								int32 sx3, sy3;
								TEX_DATA d = mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID];

								if (d.vb_id != -1)
								{
									sx3 = d.sizex;
									sy3 = d.sizey;
								}
								else
								{
									sx3 = (float)((d.w * 16384) / st.screenx);
									sy3 = (float)((d.h * st.gamey) / st.screeny);
								}

								float sx = ((float)32768.0f * st.Current_Map.obj[i].size.x) /
									(float)(((float)mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].w /
									mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].h) * sy3);

								sx = ceil(sx / 32768.0f);

								int32 tx = st.Current_Map.obj[i].texsize.x;

								st.Current_Map.obj[i].texsize.x = sx * 32768;

								if (tx == st.Current_Map.obj[i].texsize.x)
								{
									st.Current_Map.obj[i].texsize.x = (sx + 1) * 32768;
									sx += 1;
								}

								int32 sx2 = st.Current_Map.obj[i].size.x;

								st.Current_Map.obj[i].size.x = sx * (float)(((float)mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].w /
									mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].h) * sy3);

								st.Current_Map.obj[i].position.x += (sx2 - st.Current_Map.obj[i].size.x) / 2;
							}

							if (CheckCollisionMouseWorld(p.x + 600, p.y - 512 - s.y / 2, 512, 512, 0, 2) && st.mouse1 && meng.sub_com == OBJEXTRUDE)
							{
								//meng.extruding = 2;

								int32 sx3, sy3;
								TEX_DATA d = mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID];

								if (d.vb_id != -1)
								{
									sx3 = d.sizex;
									sy3 = d.sizey;
								}
								else
								{
									sx3 = (float)((d.w * 16384) / st.screenx);
									sy3 = (float)((d.h * st.gamey) / st.screeny);
								}

								float sy = ((float)32768.0f * st.Current_Map.obj[i].size.y) /
									(float)(sx3 / ((float)mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].w /
									mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].h));

								sy = ceil(sy / 32768.0f);

								int32 ty = st.Current_Map.obj[i].texsize.y;

								st.Current_Map.obj[i].texsize.y = sy * 32768;

								if (ty == st.Current_Map.obj[i].texsize.y)
								{
									st.Current_Map.obj[i].texsize.y = (sy + 1) * 32768;
									sy += 1;
								}

								int32 sy2 = st.Current_Map.obj[i].size.y;

								st.Current_Map.obj[i].size.y = (float)sy * (float)((float)sx3 / ((float)mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].w /
									mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].h));

								st.Current_Map.obj[i].position.y += (sy2 - st.Current_Map.obj[i].size.y) / 2;
							}

							if (CheckCollisionMouseWorld(p.x + 600, p.y + 512 + s.y / 2, 512, 512, 0, 2) && st.mouse1 && meng.sub_com == OBJEXTRUDE)
							{
								//meng.extruding = 2;

								int32 sx3, sy3;
								TEX_DATA d = mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID];

								if (d.vb_id != -1)
								{
									sx3 = d.sizex;
									sy3 = d.sizey;
								}
								else
								{
									sx3 = (float)((d.w * 16384) / st.screenx);
									sy3 = (float)((d.h * st.gamey) / st.screeny);
								}

								float sy = ((float)32768.0f * st.Current_Map.obj[i].size.y) /
									(float)(sx3 / ((float)mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].w /
									mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].h));

								sy = ceil(sy / 32768.0f);

								int32 ty = st.Current_Map.obj[i].texsize.y;

								st.Current_Map.obj[i].texsize.y = sy * 32768;

								if (ty == st.Current_Map.obj[i].texsize.y)
								{
									st.Current_Map.obj[i].texsize.y = (sy + 1) * 32768;
									sy += 1;
								}

								int32 sy2 = st.Current_Map.obj[i].size.y;

								st.Current_Map.obj[i].size.y = (float)sy * (float)((float)sx3 / ((float)mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].w /
									mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].h));

								st.Current_Map.obj[i].position.y -= (sy2 - st.Current_Map.obj[i].size.y) / 2;
							}

							if (meng.extruding == 1)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);

									SnapToGrid(&p.x, &p.y);

									//p.x -= pm.x;
									p.x -= pm.x;

									p.x *= -1;

									int32 sx, sy;
									TEX_DATA d = mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID];

									if (d.vb_id != -1)
									{
										sx = d.sizex;
										sy = d.sizey;
									}
									else
									{
										sx = (float)((d.w * 16384) / st.screenx);
										sy = (float)((d.h * st.gamey) / st.screeny);
									}

									st.Current_Map.obj[i].position.x -= p.x % 2 != 0 ? (p.x + 1) / 2 : p.x / 2;
									st.Current_Map.obj[i].size.x -= p.x % 2 != 0 ? p.x + 1 : p.x;
									st.Current_Map.obj[i].texsize.x = ((float)32768.0f * st.Current_Map.obj[i].size.x) /
										(float)(sy * ((float)mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].w /
										mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].h));

									pm = st.mouse;
									STW(&pm.x, &pm.y);

									SnapToGrid(&pm.x, &pm.y);
								}
								else
								{
									meng.extruding = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.extruding == 2)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);

									SnapToGrid(&p.x, &p.y);

									//p.x -= pm.x;
									p.x -= pm.x;

									//p.x *= -1;

									int32 sx, sy;
									TEX_DATA d = mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID];

									if (d.vb_id != -1)
									{
										sx = d.sizex;
										sy = d.sizey;
									}
									else
									{
										sx = (float)((d.w * 16384) / st.screenx);
										sy = (float)((d.h * st.gamey) / st.screeny);
									}

									st.Current_Map.obj[i].position.x += p.x % 2 != 0 ? (p.x + 1) / 2 : p.x / 2;
									st.Current_Map.obj[i].size.x -= p.x % 2 != 0 ? p.x + 1 : p.x;
									st.Current_Map.obj[i].texsize.x = ((float)32768.0f * st.Current_Map.obj[i].size.x) /
										(float)(sy * ((float)mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].w /
										mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].h));

									pm = st.mouse;
									STW(&pm.x, &pm.y);
									SnapToGrid(&pm.x, &pm.y);
								}
								else
								{
									meng.extruding = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.extruding == 3)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);

									SnapToGrid(&p.x, &p.y);

									//p.x -= pm.x;
									p.y -= pm.y;

									//p.y *= -1;

									int32 sx, sy;
									TEX_DATA d = mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID];

									if (d.vb_id != -1)
									{
										sx = d.sizex;
										sy = d.sizey;
									}
									else
									{
										sx = (float)((d.w * 16384) / st.screenx);
										sy = (float)((d.h * st.gamey) / st.screeny);
									}

									st.Current_Map.obj[i].position.y += p.y % 2 != 0 ? (p.y + 1) / 2 : p.y / 2;
									st.Current_Map.obj[i].size.y -= p.y % 2 != 0 ? p.y + 1 : p.y;
									st.Current_Map.obj[i].texsize.y = ((float)32768.0f * st.Current_Map.obj[i].size.y) /
										(float)(sx / ((float)mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].w /
										mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].h));

									pm = st.mouse;
									STW(&pm.x, &pm.y);

									SnapToGrid(&pm.x, &pm.y);
								}
								else
								{
									meng.extruding = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.extruding == 4)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);

									SnapToGrid(&p.x, &p.y);

									//p.x -= pm.x;
									p.y -= pm.y;

									p.y *= -1;

									int32 sx, sy;
									TEX_DATA d = mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID];

									if (d.vb_id != -1)
									{
										sx = d.sizex;
										sy = d.sizey;
									}
									else
									{
										sx = (float)((d.w * 16384) / st.screenx);
										sy = (float)((d.h * st.gamey) / st.screeny);
									}

									st.Current_Map.obj[i].position.y -= p.y % 2 != 0 ? (p.y + 1) / 2 : p.y / 2;
									st.Current_Map.obj[i].size.y -= p.y % 2 != 0 ? p.y + 1 : p.y;
									st.Current_Map.obj[i].texsize.y = ((float)32768.0f * st.Current_Map.obj[i].size.y) /
										(float)(sx / ((float)mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].w /
										mgg_map[st.Current_Map.obj[i].tex.MGG_ID].frames[st.Current_Map.obj[i].tex.ID].h));

									pm = st.mouse;
									STW(&pm.x, &pm.y);

									SnapToGrid(&pm.x, &pm.y);
								}
								else
								{
									meng.extruding = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}
						}

						if(CheckCollisionMouseWorld(st.Current_Map.obj[i].position.x,st.Current_Map.obj[i].position.y,st.Current_Map.obj[i].size.x,st.Current_Map.obj[i].size.y,st.Current_Map.obj[i].angle,
							st.Current_Map.obj[i].position.z) && meng.scaling == 0 && meng.extruding == 0)
						{
							if(st.mouse1)
							{
								if (meng.command2 != EDIT_OBJ && meng.sub_com == SCALER_SELECT)
									continue;

								memset(meng.layers, 0, 57 * 2048 * 2);
								meng.layers[l][k] = 1;

								if(meng.got_it==-1)
								{
									meng.command2 = EDIT_OBJ;
									meng.p = st.mouse;
									meng.got_it = i;

									meng.obj_edit_selection = i;

									STW(&meng.p.x, &meng.p.y);

									SnapToGrid(&meng.p.x, &meng.p.y);
									PositionToEdge(&meng.p.x, &meng.p.y, st.Current_Map.obj[i].size.x, st.Current_Map.obj[i].size.y);

									meng.p.x -= st.Current_Map.obj[i].position.x;
									meng.p.y -= st.Current_Map.obj[i].position.y;
								}
								
								if(meng.got_it!=-1 && meng.got_it != i)
									continue;
								
								
									p = st.mouse;

									STW(&p.x, &p.y);
	
									meng.com_id = i;

									//SnapToGrid(&p.x, &p.y);
									//PositionToEdge(&p.x, &p.y, st.Current_Map.obj[i].size.x, st.Current_Map.obj[i].size.y);

									st.Current_Map.obj[i].position.x = p.x;
									st.Current_Map.obj[i].position.y = p.y;

									st.Current_Map.obj[i].position.x -= meng.p.x;
									st.Current_Map.obj[i].position.y -= meng.p.y;

									SnapToGrid(&st.Current_Map.obj[i].position.x, &st.Current_Map.obj[i].position.y);
									PositionToEdge(&st.Current_Map.obj[i].position.x, &st.Current_Map.obj[i].position.y, st.Current_Map.obj[i].size.x, st.Current_Map.obj[i].size.y);
									
									p.x = st.Current_Map.obj[i].position.x;
									p.y = st.Current_Map.obj[i].position.y;

									//SnapToGrid(&p.x, &p.y);
									//PositionToEdge(&p.x, &p.y, st.Current_Map.obj[i].size.x, st.Current_Map.obj[i].size.y);

									got_it = 1;
									break;
								
							}
							//else
								//meng.got_it=-1;
						}

						if (meng.sub_com == SCALER_SELECT && meng.command2 == EDIT_OBJ && meng.obj_edit_selection == i)
						{
							p = st.Current_Map.obj[i].position;
							p2 = st.Current_Map.obj[i].size;

							//p.x -= (float)st.Camera.position.x*st.Current_Map.fr_v;
							//p.y -= (float)st.Camera.position.y*st.Current_Map.fr_v;

							if (st.keys[RETURN_KEY].state)
							{
								meng.sub_com = 0;
								meng.scaling = 0;
							}

							if (p.z > 31 && p.z < 40)
							{
								p.x -= (float)st.Camera.position.x*st.Current_Map.bck1_v;
								p.y -= (float)st.Camera.position.y*st.Current_Map.bck1_v;
							}

							if (p.z > 39 && p.z < 48)
							{
								p.x -= (float)st.Camera.position.x * st.Current_Map.bck2_v;
								p.y -= (float)st.Camera.position.y * st.Current_Map.bck2_v;

								p.x += st.Camera.position.x;
								p.y += st.Camera.position.y;
							}

							if (p.z > 47)
							{
								p.x /= (float)st.Camera.dimension.x;
								p.y /= (float)st.Camera.dimension.y;

								p.x += st.Camera.position.x;
								p.y += st.Camera.position.y;

								p2.x /= (float)st.Camera.dimension.x;
								p2.y /= (float)st.Camera.dimension.y;
							}

							//UIData(p.x, p.x, p.y + (p2.y / 2), 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 255, 0, 0, 0, 0, 32768, 32768, st.BasicTex, 255, 0);

							if (meng.scaling == 1)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);

									//p.x -= pm.x;
									p.y -= pm.y;

									p.y *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.y *= -1;
											p.y += pm.y;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.obj[i].size.y = abs(p.y - st.Current_Map.obj[i].position.y) * 2;
										}
										else
											st.Current_Map.obj[i].size.y += p.y;
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.y *= -1;
											p.y += pm.y;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.obj[i].size.y = abs(p.y - st.Current_Map.obj[i].position.y) * 2;
											st.Current_Map.obj[i].size.x = (float)st.Current_Map.obj[i].size.y * asp;
										}
										else
										{
											st.Current_Map.obj[i].size.x += p.y;
											st.Current_Map.obj[i].size.y = (float)st.Current_Map.obj[i].size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.scaling == 2)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);
									//p.x -= pm.x;
									p.y -= pm.y;

									//p.y *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.y *= -1;
											p.y += pm.y;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.obj[i].size.y = abs(p.y - st.Current_Map.obj[i].position.y) * 2;
										}
										else
											st.Current_Map.obj[i].size.y += p.y;
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.y *= -1;
											p.y += pm.y;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.obj[i].size.y = abs(p.y - st.Current_Map.obj[i].position.y) * 2;
											st.Current_Map.obj[i].size.x = (float)st.Current_Map.obj[i].size.y * asp;
										}
										else
										{
											st.Current_Map.obj[i].size.x += p.y;
											st.Current_Map.obj[i].size.y = (float)st.Current_Map.obj[i].size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.scaling == 3)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);
									p.x -= pm.x;
									//p.y -= pm.y;

									p.x *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.obj[i].size.x = abs(p.x - st.Current_Map.obj[i].position.x) * 2;
										}
										else
											st.Current_Map.obj[i].size.x += p.x;
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.obj[i].size.x = abs(p.x - st.Current_Map.obj[i].position.x) * 2;
											st.Current_Map.obj[i].size.y = (float)st.Current_Map.obj[i].size.x / asp;
										}
										else
										{
											st.Current_Map.obj[i].size.x += p.x;
											st.Current_Map.obj[i].size.y = (float)st.Current_Map.obj[i].size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.scaling == 4)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);
									p.x -= pm.x;
									//p.y -= pm.y;

									//p.x *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.obj[i].size.x = abs(p.x - st.Current_Map.obj[i].position.x) * 2;
										}
										else
											st.Current_Map.obj[i].size.x += p.x;
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.obj[i].size.x = abs(p.x - st.Current_Map.obj[i].position.x) * 2;
											st.Current_Map.obj[i].size.y = (float)st.Current_Map.obj[i].size.x / asp;
										}
										else
										{
											st.Current_Map.obj[i].size.x += p.x;
											st.Current_Map.obj[i].size.y = (float)st.Current_Map.obj[i].size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.scaling == 5)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);
									p.x -= pm.x;
									p.y -= pm.y;

									p.x *= -1;
									p.y *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.obj[i].size.x = abs(p.x - st.Current_Map.obj[i].position.x) * 2;
										}
										else
										{
											st.Current_Map.obj[i].size.x += p.x;
											st.Current_Map.obj[i].size.y += p.y;
										}
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;
											//p.y *= -1;
											//p.y += pm.y;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.obj[i].size.x = abs(p.x - st.Current_Map.obj[i].position.x) * 2;
											st.Current_Map.obj[i].size.y = (float)st.Current_Map.obj[i].size.x / asp;
										}
										else
										{
											st.Current_Map.obj[i].size.x += p.x;
											st.Current_Map.obj[i].size.y = (float)st.Current_Map.obj[i].size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.scaling == 6)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);
									p.x -= pm.x;
									p.y -= pm.y;

									//p.x *= -1;
									p.y *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.obj[i].size.x = abs(p.x - st.Current_Map.obj[i].position.x) * 2;
										}
										else
										{
											st.Current_Map.obj[i].size.x += p.x;
											st.Current_Map.obj[i].size.y += p.y;
										}
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.obj[i].size.x = abs(p.x - st.Current_Map.obj[i].position.x) * 2;
											st.Current_Map.obj[i].size.y = (float)st.Current_Map.obj[i].size.x / asp;
										}
										else
										{
											st.Current_Map.obj[i].size.x += p.x;
											st.Current_Map.obj[i].size.y = (float)st.Current_Map.obj[i].size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.scaling == 7)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);
									p.x -= pm.x;
									p.y -= pm.y;

									//p.x *= -1;
									//p.y *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.obj[i].size.x = abs(p.x - st.Current_Map.obj[i].position.x) * 2;
										}
										else
										{
											st.Current_Map.obj[i].size.x += p.x;
											st.Current_Map.obj[i].size.y += p.y;
										}
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.obj[i].size.x = abs(p.x - st.Current_Map.obj[i].position.x) * 2;
											st.Current_Map.obj[i].size.y = (float)st.Current_Map.obj[i].size.x / asp;
										}
										else
										{
											st.Current_Map.obj[i].size.x += p.x;
											st.Current_Map.obj[i].size.y = (float)st.Current_Map.obj[i].size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (meng.scaling == 8)
							{
								if (st.mouse1)
								{
									p = st.mouse;
									STW(&p.x, &p.y);
									p.x -= pm.x;
									p.y -= pm.y;

									p.x *= -1;
									//p.y *= -1;

									if (st.keys[LSHIFT_KEY].state)
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);
											st.Current_Map.obj[i].size.x = abs(p.x - st.Current_Map.obj[i].position.x) * 2;
										}
										else
										{
											st.Current_Map.obj[i].size.x += p.x;
											st.Current_Map.obj[i].size.y += p.y;
										}
									}
									else
									{
										if (meng.snap_scale == 1)
										{
											p.x *= -1;
											p.x += pm.x;

											SnapToGrid(&p.x, &p.y);

											st.Current_Map.obj[i].size.x = abs(p.x - st.Current_Map.obj[i].position.x) * 2;
											st.Current_Map.obj[i].size.y = (float)st.Current_Map.obj[i].size.x / asp;
										}
										else
										{
											st.Current_Map.obj[i].size.x += p.x;
											st.Current_Map.obj[i].size.y = (float)st.Current_Map.obj[i].size.x / asp;
										}
									}

									pm = st.mouse;
									STW(&pm.x, &pm.y);
								}
								else
								{
									meng.scaling = 0;
									st.cursor_type = CURSOR_DEFAULT;
								}
							}

							if (CheckCollisionMouseWorld(p.x, p.y - (p2.y / 2) - 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 1;

										asp = (float)st.Current_Map.obj[i].size.x / st.Current_Map.obj[i].size.y;

										st.cursor_type = CURSOR_S_UD;
									}
								}
							}

							if (CheckCollisionMouseWorld(p.x, p.y + (p2.y / 2) + 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 2;

										asp = (float)st.Current_Map.obj[i].size.x / st.Current_Map.obj[i].size.y;

										st.cursor_type = CURSOR_S_UD;
									}
								}
							}

							if (CheckCollisionMouseWorld(p.x - (p2.x / 2) - 96, p.y, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 3;

										asp = (float)st.Current_Map.obj[i].size.x / st.Current_Map.obj[i].size.y;

										st.cursor_type = CURSOR_S_LR;
									}
								}
							}

							if (CheckCollisionMouseWorld(p.x + (p2.x / 2) + 96, p.y, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 4;

										asp = (float)st.Current_Map.obj[i].size.x / st.Current_Map.obj[i].size.y;

										st.cursor_type = CURSOR_S_LR;
									}
								}
							}

							if (CheckCollisionMouseWorld(p.x - (p2.x / 2) - 96, p.y - (p2.y / 2) - 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 5;

										asp = (float)st.Current_Map.obj[i].size.x / st.Current_Map.obj[i].size.y;

										st.cursor_type = CURSOR_S_C_LR;
									}
								}
							}

							if (CheckCollisionMouseWorld(p.x + (p2.x / 2) + 96, p.y - (p2.y / 2) - 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 6;

										asp = (float)st.Current_Map.obj[i].size.x / st.Current_Map.obj[i].size.y;

										st.cursor_type = CURSOR_S_C_RL;
									}
								}
							}

							if (CheckCollisionMouseWorld(p.x + (p2.x / 2) + 96, p.y + (p2.y / 2) + 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 7;

										asp = (float)st.Current_Map.obj[i].size.x / st.Current_Map.obj[i].size.y;

										st.cursor_type = CURSOR_S_C_LR;
									}
								}
							}

							if (CheckCollisionMouseWorld(p.x - (p2.x / 2) - 96, p.y + (p2.y / 2) + 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0))
							{
								if (st.mouse1)
								{
									if (meng.scaling == 0)
									{
										pm = st.mouse;
										STW(&pm.x, &pm.y);

										meng.scaling = 8;

										asp = (float)st.Current_Map.obj[i].size.x / st.Current_Map.obj[i].size.y;

										st.cursor_type = CURSOR_S_C_RL;
									}
								}
							}
						}
					}
				}
			}

			if (!st.mouse1 && meng.got_it != -1)
				meng.got_it = -1;
		}
		else
		if(meng.command==ADD_OBJ && st.Current_Map.num_mgg > 0)
		{
			if(st.mouse1)
			{
				if (st.Current_Map.num_obj < MAX_OBJS)
				{
					i=st.Current_Map.num_obj;
					//for(i=0;i<MAX_OBJS;i++)
					//{
						if(st.Current_Map.obj[i].type==BLANK)
						{
							st.Current_Map.obj[i].type = meng.obj.type;
								
							st.Current_Map.obj[i].amblight = meng.obj.amblight;
							st.Current_Map.obj[i].color = meng.obj.color;
							st.Current_Map.obj[i].tex.ID = meng.tex_ID;
							st.Current_Map.obj[i].tex.MGG_ID = meng.tex_MGGID;
							st.Current_Map.obj[i].texsize = meng.obj.texsize;
							st.Current_Map.obj[i].texpan = meng.obj.texpan;

							st.Current_Map.obj[i].position = st.mouse;
							STWci(&st.Current_Map.obj[i].position.x, &st.Current_Map.obj[i].position.y);

							if(meng.obj.type==BACKGROUND2)
							{
								st.Current_Map.obj[i].position.x/=(float) st.Camera.dimension.x;
								st.Current_Map.obj[i].position.y/=(float) st.Camera.dimension.y;

								st.Current_Map.obj[i].position.x+=(float) st.Camera.position.x*st.Current_Map.bck2_v;
								st.Current_Map.obj[i].position.y+=(float) st.Camera.position.y*st.Current_Map.bck2_v;
							}
							else
							if(meng.obj.type==BACKGROUND1)
							{
								st.Current_Map.obj[i].position.x/=(float) st.Camera.dimension.x;
								st.Current_Map.obj[i].position.y/=(float) st.Camera.dimension.y;

								st.Current_Map.obj[i].position.x+=(float) st.Camera.position.x*st.Current_Map.bck1_v;
								st.Current_Map.obj[i].position.y+=(float) st.Camera.position.y*st.Current_Map.bck1_v;
							}
							else
							if(meng.obj.type==MIDGROUND)
							{
								st.Current_Map.obj[i].position.x/=(float) st.Camera.dimension.x;
								st.Current_Map.obj[i].position.y/=(float) st.Camera.dimension.y;

								st.Current_Map.obj[i].position.x+=st.Camera.position.x;
								st.Current_Map.obj[i].position.y+=st.Camera.position.y;
							}
							if(meng.obj.type==FOREGROUND)
							{
								st.Current_Map.obj[i].position.x/=(float) st.Camera.dimension.x;
								st.Current_Map.obj[i].position.y/=(float) st.Camera.dimension.y;

								st.Current_Map.obj[i].position.x += (float)st.Camera.position.x*st.Current_Map.fr_v;
								st.Current_Map.obj[i].position.y += (float)st.Camera.position.y*st.Current_Map.fr_v;
							}

							st.Current_Map.obj[i].size.x = meng.pre_size.x;
							st.Current_Map.obj[i].size.y = meng.pre_size.y;
							st.Current_Map.obj[i].angle = 0;
							st.Current_Map.obj[i].flag = meng.obj.flag;

							int32 mx2, my2, mx3, my3, mx4, my4;

							mx2 = st.Current_Map.obj[i].position.x;
							my2 = st.Current_Map.obj[i].position.y;

							SnapToGrid(&mx2, &my2);
							PositionToEdge(&mx2, &my2, st.Current_Map.obj[i].size.x, st.Current_Map.obj[i].size.y);

							if ((meng.select_edge == 0 || meng.select_edge == 3 || meng.select_edge == 6) && meng.snap_fit == 1)
							{
								mx3 = mx2;
								my3 = my2;

								mx3 -= st.Current_Map.obj[i].size.x / 2;

								mx4 = mx3 + st.Current_Map.obj[i].size.x;

								SnapToGrid(&mx4, &my3);

								mx4 -= mx3;

								float aspect = (float)st.Current_Map.obj[i].size.x / st.Current_Map.obj[i].size.y;

								mx3 = (mx4 - st.Current_Map.obj[i].size.x) / 2;

								my4 = (float)mx4 / aspect;
								my3 = (my4 - st.Current_Map.obj[i].size.y) / 2;

								st.Current_Map.obj[i].size.x = mx4;
								st.Current_Map.obj[i].size.y = my4;

								st.Current_Map.obj[i].position.x = mx2 + mx3;
								st.Current_Map.obj[i].position.y = my2 + my3;
							}
							else
							if ((meng.select_edge == 2 || meng.select_edge == 5 || meng.select_edge == 8) && meng.snap_fit == 1)
							{
								mx3 = mx2;
								my3 = my2;

								mx3 += st.Current_Map.obj[i].size.x / 2;

								mx4 = mx3 + st.Current_Map.obj[i].size.x;

								SnapToGrid(&mx4, &my3);

								mx4 -= mx3;

								float aspect = (float)st.Current_Map.obj[i].size.x / st.Current_Map.obj[i].size.y;

								mx3 = (mx4 - st.Current_Map.obj[i].size.x) / 2;

								my4 = (float)mx4 / aspect;
								my3 = (my4 - st.Current_Map.obj[i].size.y) / 2;

								st.Current_Map.obj[i].size.x = mx4;
								st.Current_Map.obj[i].size.y = my4;

								st.Current_Map.obj[i].position.x = mx2 - mx3;
								st.Current_Map.obj[i].position.y = my2 + my3;
							}
							else
							if ((meng.select_edge == 1) && meng.snap_fit == 1)
							{
								mx3 = mx2;
								my3 = my2;

								my3 -= st.Current_Map.obj[i].size.y / 2;

								my4 = my3 + st.Current_Map.obj[i].size.y;

								SnapToGrid(&mx3, &my4);

								my4 -= my3;

								float aspect = (float)st.Current_Map.obj[i].size.x / st.Current_Map.obj[i].size.y;

								my3 = (my4 - st.Current_Map.obj[i].size.y) / 2;

								mx4 = (float)my4 * aspect;
								mx3 = (mx4 - st.Current_Map.obj[i].size.x) / 2;

								st.Current_Map.obj[i].size.x = mx4;
								st.Current_Map.obj[i].size.y = my4;

								st.Current_Map.obj[i].position.x = mx2 + mx3;
								st.Current_Map.obj[i].position.y = my2 + my3;
							}
							else
							if ((meng.select_edge == 7) && meng.snap_fit == 1)
							{
								mx3 = mx2;
								my3 = my2;

								my3 += st.Current_Map.obj[i].size.y / 2;

								my4 = my3 + st.Current_Map.obj[i].size.y;

								SnapToGrid(&mx3, &my4);

								my4 -= my3;

								float aspect = (float)st.Current_Map.obj[i].size.x / st.Current_Map.obj[i].size.y;

								my3 = (my4 - st.Current_Map.obj[i].size.y) / 2;

								mx4 = (float)my4 * aspect;
								mx3 = (mx4 - st.Current_Map.obj[i].size.x) / 2;

								st.Current_Map.obj[i].size.x = mx4;
								st.Current_Map.obj[i].size.y = my4;

								st.Current_Map.obj[i].position.x = mx2 + mx3;
								st.Current_Map.obj[i].position.y = my2 - my3;
							}
							else
							{
								st.Current_Map.obj[i].position.x = mx2;
								st.Current_Map.obj[i].position.y = my2;
							}

							if(meng.obj.type==BACKGROUND3)
								st.Current_Map.obj[i].position.z=55;
							else
							if(meng.obj.type==BACKGROUND2)
								st.Current_Map.obj[i].position.z=47;
							else
							if(meng.obj.type==BACKGROUND1)
								st.Current_Map.obj[i].position.z=39;
							else
							if(meng.obj.type==MIDGROUND)
								st.Current_Map.obj[i].position.z=31;
							else
							if(meng.obj.type==FOREGROUND)
								st.Current_Map.obj[i].position.z=23;

							meng.z_buffer[st.Current_Map.obj[i].position.z][meng.z_slot[st.Current_Map.obj[i].position.z]] = i;
							meng.z_slot[st.Current_Map.obj[i].position.z]++;

							if (st.Current_Map.obj[i].position.z > meng.z_used)
								meng.z_used = st.Current_Map.obj[i].position.z;

							st.Current_Map.num_obj++;
							//break;
						}
					//}
				}

				LogApp("Object added");
				st.mouse1=0;
			}
		}
		else
		if (meng.command == ADD_SPRITE)
		{
			if(st.mouse1)
			{
				if(st.Current_Map.num_sprites<MAX_SPRITES)
				{
					i=st.Current_Map.num_sprites;
					//for(i=0;i<MAX_SPRITES;i++)
					//{
						if(st.Current_Map.sprites[i].stat==0)
						{
							st.Current_Map.sprites[i].stat=1;
							st.Current_Map.sprites[i].color=meng.spr.color;
							st.Current_Map.sprites[i].health=meng.spr.health;
							st.Current_Map.sprites[i].body=meng.spr.body;
							st.Current_Map.sprites[i].GameID=meng.sprite_selection;
							st.Current_Map.sprites[i].frame_ID=meng.sprite_frame_selection;
							st.Current_Map.sprites[i].type_s=meng.spr.type;
							st.Current_Map.sprites[i].flags=meng.spr.flags;
							st.Current_Map.sprites[i].MGG_ID=st.Game_Sprites[meng.sprite_selection].MGG_ID;
							st.Current_Map.sprites[i].size_a.x = st.Game_Sprites[meng.sprite_selection].size_a.x;
							st.Current_Map.sprites[i].size_a.y = st.Game_Sprites[meng.sprite_selection].size_a.y;
							st.Current_Map.sprites[i].size_m.x = st.Game_Sprites[meng.sprite_selection].size_m.x + 1;
							st.Current_Map.sprites[i].size_m.y = st.Game_Sprites[meng.sprite_selection].size_m.y + 1;

							st.Current_Map.sprites[i].position=st.mouse;
							STWci(&st.Current_Map.sprites[i].position.x,&st.Current_Map.sprites[i].position.y);
							
							if(meng.spr.type==BACKGROUND2)
							{
								st.Current_Map.sprites[i].position.x/=(float) st.Camera.dimension.x;
								st.Current_Map.sprites[i].position.y/=(float) st.Camera.dimension.y;

								st.Current_Map.sprites[i].position.x+=(float) st.Camera.position.x*st.Current_Map.bck2_v;
								st.Current_Map.sprites[i].position.y+=(float) st.Camera.position.y*st.Current_Map.bck2_v;

								//st.Current_Map.sprites[i].position.x*=st.Camera.dimension.x;
								//st.Current_Map.sprites[i].position.y*=st.Camera.dimension.y;
							}
							else
							if(meng.spr.type==BACKGROUND1)
							{
								st.Current_Map.sprites[i].position.x/=(float) st.Camera.dimension.x;
								st.Current_Map.sprites[i].position.y/=(float) st.Camera.dimension.y;
				
								st.Current_Map.sprites[i].position.x+=(float) st.Camera.position.x*st.Current_Map.bck1_v;
								st.Current_Map.sprites[i].position.y+=(float) st.Camera.position.y*st.Current_Map.bck1_v;

								//st.Current_Map.sprites[i].position.x*=st.Camera.dimension.x;
								//st.Current_Map.sprites[i].position.y*=st.Camera.dimension.y;
							}
							else
							if(meng.spr.type==MIDGROUND)
							{

								st.Current_Map.sprites[i].position.x/=(float) st.Camera.dimension.x;
								st.Current_Map.sprites[i].position.y/=(float) st.Camera.dimension.y;

								st.Current_Map.sprites[i].position.x+=st.Camera.position.x;
								st.Current_Map.sprites[i].position.y+=st.Camera.position.y;
							}
							if(meng.spr.type==FOREGROUND)
							{
								st.Current_Map.sprites[i].position.x/=(float) st.Camera.dimension.x;
								st.Current_Map.sprites[i].position.y/=(float) st.Camera.dimension.y;

								st.Current_Map.sprites[i].position.x+=(float) st.Camera.position.x*st.Current_Map.fr_v;
								st.Current_Map.sprites[i].position.y+=(float) st.Camera.position.y*st.Current_Map.fr_v;

							}
							
							//st.Current_Map.sprites[i].body.size=meng.spr.size;
							memcpy(st.Current_Map.sprites[i].tags,st.Game_Sprites[meng.sprite_selection].tags,8*sizeof(int16));
							st.Current_Map.sprites[i].num_tags=st.Game_Sprites[meng.sprite_selection].num_tags;

							for(j=1;j<st.Game_Sprites[meng.sprite_selection].num_tags;j++)
								strcpy(st.Current_Map.sprites[i].tags_str[j],st.Game_Sprites[meng.sprite_selection].tags_str[j]);

							st.Current_Map.sprites[i].angle=0;

							SnapToGrid(&st.Current_Map.sprites[i].position.x, &st.Current_Map.sprites[i].position.y);
							PositionToEdge(&st.Current_Map.sprites[i].position.x, &st.Current_Map.sprites[i].position.y, st.Current_Map.sprites[i].body.size.x, st.Current_Map.sprites[i].body.size.y);

							if(meng.spr.type==BACKGROUND3)
								st.Current_Map.sprites[i].position.z=55;
							else
							if(meng.spr.type==BACKGROUND2)
								st.Current_Map.sprites[i].position.z=47;
							else
							if(meng.spr.type==BACKGROUND1)
								st.Current_Map.sprites[i].position.z=39;
							else
							if(meng.spr.type==MIDGROUND)
								st.Current_Map.sprites[i].position.z=31;
							else
							if(meng.spr.type==FOREGROUND)
								st.Current_Map.sprites[i].position.z=23;

							meng.z_buffer[st.Current_Map.sprites[i].position.z][meng.z_slot[st.Current_Map.sprites[i].position.z]]=i+2000;
							meng.z_slot[st.Current_Map.sprites[i].position.z]++;
							if(st.Current_Map.sprites[i].position.z>meng.z_used)
								meng.z_used=st.Current_Map.sprites[i].position.z;

							st.Current_Map.num_sprites++;

							LogApp("Sprite Added");
							st.mouse1=0;

							//break;
						}
					//}
				}
			}
		}
	}

	if(meng.command!=TEX_SIZE_OBJ && meng.command!=TEX_PAN_OBJ && meng.command!=OBJ_AMBL && meng.command!=RGB_OBJ && meng.command!=OBJ_EDIT_BOX && !st.Text_Input)
	{
		if(st.keys[UP_KEY].state)
			st.Camera.position.y-=32*delta;

		if(st.keys[DOWN_KEY].state)
			st.Camera.position.y+=32*delta;

		if(st.keys[RIGHT_KEY].state)
			st.Camera.position.x+=32*delta;

		if(st.keys[LEFT_KEY].state)
			st.Camera.position.x-=32*delta;

		if (st.mouse3)
		{
			if (meng.mouse_move == 0)
			{
				meng.mouse_move_pos = st.mouse;
				meng.mouse_move_pos2 = st.mouse;
				meng.mouse_move = 1;
			}
			else
			{
				meng.mouse_move_pos2.x -= meng.mouse_move_pos.x;
				meng.mouse_move_pos2.y -= meng.mouse_move_pos.y;

				STWci(&meng.mouse_move_pos2.x, &meng.mouse_move_pos2.y);

				st.Camera.position.x += meng.mouse_move_pos2.x / 100;
				st.Camera.position.y += meng.mouse_move_pos2.y / 100;

				meng.mouse_move_pos2 = st.mouse;
			}
		}
		else
			meng.mouse_move = 0;

		if (meng.command != ADD_LIGHT_TO_LIGHTMAP && meng.command != EDIT_LIGHTMAP2  && meng.command != MOVE_LIGHTMAP && meng.command != MGG_LOAD && meng.sub_com < 100
			&& meng.command != SPRITE_SELECTION && meng.command != TEX_SEL)
		{
			if(st.mouse_wheel > 0 && st.keys[LALT_KEY].state)
			{
				if (st.Camera.dimension.x < 6) st.Camera.dimension.x += 0.1;
				if (st.Camera.dimension.y < 6) st.Camera.dimension.y += 0.1;
				st.mouse_wheel = 0;
			}

			if (st.mouse_wheel < 0 && st.keys[LALT_KEY].state)
			{
				if (st.Camera.dimension.x > 0.2) st.Camera.dimension.x -= 0.1;
				if (st.Camera.dimension.y > 0.2) st.Camera.dimension.y -= 0.1;
				st.mouse_wheel = 0;
			}
		}
	}
}

static void MGGListLoad()
{
	FILE *file;
	char str[512], str2[512];
	uint16 j=0, i=0;

	if((file=fopen("mgg.list","r"))==NULL)
	{
		LogApp("Could not open mgg list file");
		Quit();
	}

	while(!feof(file))
	{
		DrawString2UI("Loading...",400,300,200,50,0,255,255,255,255,ARIAL,0,0,0);
		memset(str,0,sizeof(str));
		fgets(str,512,file);
		sscanf(str,"%s",str2);
		
		if(LoadMGG(&mgg_map[j],str2)!=NULL)
		{
			strcpy(meng.mgg_list[i],mgg_map[j].name);
			meng.num_mgg++;
			j++;
			i++;
			LogApp("Loaded: %s",str2);
		}

		Renderer(1);
	}

	fclose(file);

	LogApp("MGGs loaded");

}

static void ENGDrawLight()
{
	int32 i = 0, j = 0;

	TEX_DATA texture;

	int32 dist2;

	float dist;

	Pos p, s;
	uPos16 p2;

	int32 mx = st.mouse.x, my = st.mouse.y;

	STW(&mx, &my);

	if(meng.viewmode!=INGAMEVIEW_MODE)
	{
		if(st.Current_Map.cam_area.horiz_lim)
		{
			DrawLine((float)(st.Current_Map.cam_area.limit[0].x-st.Camera.position.x)*st.Camera.dimension.x,0,(float)(st.Current_Map.cam_area.limit[0].x-st.Camera.position.x)*st.Camera.dimension.x,st.gamey,230,255,0,255,64,15);
			DrawLine((float)(st.Current_Map.cam_area.limit[1].x-st.Camera.position.x)*st.Camera.dimension.x,0,(float)(st.Current_Map.cam_area.limit[1].x-st.Camera.position.x)*st.Camera.dimension.x,st.gamey,230,255,0,255,64,15);
		}

		if(st.Current_Map.cam_area.vert_lim)
		{
			DrawLine(0,(float)(st.Current_Map.cam_area.limit[0].y-st.Camera.position.y)*st.Camera.dimension.y,16384,(float)(st.Current_Map.cam_area.limit[0].y-st.Camera.position.y)*st.Camera.dimension.y,230,255,0,255,64,15);
			DrawLine(0,(float)(st.Current_Map.cam_area.limit[1].y-st.Camera.position.y)*st.Camera.dimension.y,16384,(float)(st.Current_Map.cam_area.limit[1].y-st.Camera.position.y)*st.Camera.dimension.y,230,255,0,255,64,15);
		}

		if(meng.command==CAM_AREA_EDIT)
		{
			DrawGraphic(st.Current_Map.cam_area.area_pos.x+(st.Current_Map.cam_area.area_size.x/2),st.Current_Map.cam_area.area_pos.y+(st.Current_Map.cam_area.area_size.y/2),
				st.Current_Map.cam_area.area_size.x,st.Current_Map.cam_area.area_size.y,0,255,255,255,mgg_sys[0].frames[4],64,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,24,0);

			DrawLine(st.Current_Map.cam_area.area_pos.x-128,st.Current_Map.cam_area.area_pos.y,st.Current_Map.cam_area.area_pos.x+st.Current_Map.cam_area.area_size.x+128,st.Current_Map.cam_area.area_pos.y,
				230,255,0,255,256,16);

			DrawLine(st.Current_Map.cam_area.area_pos.x-128,st.Current_Map.cam_area.area_pos.y+st.Current_Map.cam_area.area_size.y,st.Current_Map.cam_area.area_pos.x+st.Current_Map.cam_area.area_size.x+128,
				st.Current_Map.cam_area.area_pos.y+st.Current_Map.cam_area.area_size.y,230,255,0,255,256,16);

			DrawLine(st.Current_Map.cam_area.area_pos.x,st.Current_Map.cam_area.area_pos.y,st.Current_Map.cam_area.area_pos.x,st.Current_Map.cam_area.area_pos.y+st.Current_Map.cam_area.area_size.y,230,255,0,255,256,24);

			DrawLine(st.Current_Map.cam_area.area_pos.x+st.Current_Map.cam_area.area_size.x,st.Current_Map.cam_area.area_pos.y,st.Current_Map.cam_area.area_pos.x+st.Current_Map.cam_area.area_size.x,
				st.Current_Map.cam_area.area_pos.y+st.Current_Map.cam_area.area_size.y,230,255,0,255,256,16);
		}

		if (meng.command == ADD_OBJ && st.Current_Map.num_mgg > 0)
		{
			int32 mx2 = mx, my2 = my, mx3, my3, mx4 = 0, my4 = 0;
			float aspect = 1;

			SnapToGrid(&mx2, &my2);
			PositionToEdge(&mx2, &my2, meng.pre_size.x, meng.pre_size.y);

			if ((meng.select_edge == 0 || meng.select_edge == 3 || meng.select_edge == 6) && meng.snap_fit == 1 && meng.snap == 1)
			{
				mx3 = mx2;
				my3 = my2;

				mx3 -= meng.pre_size.x / 2;

				mx4 = mx3 + meng.pre_size.x;

				SnapToGrid(&mx4, &my3);

				mx4 -= mx3;

				aspect = (float) meng.pre_size.x / meng.pre_size.y;

				mx3 = (mx4 - meng.pre_size.x) / 2;

				my4 = (float)mx4 / aspect;
				my3 = (my4 - meng.pre_size.y) / 2;

				DrawGraphic(mx2 + mx3, my2 + my3, mx4, my4, 0, meng.obj.color.r, meng.obj.color.g, meng.obj.color.b, mgg_map[meng.tex_MGGID].frames[meng.tex_ID], 128,
					meng.obj.texpan.x, meng.obj.texpan.y, meng.obj.texsize.x, meng.obj.texsize.y, 16, 2);
			}
			else
			if ((meng.select_edge == 2 || meng.select_edge == 5 || meng.select_edge == 8) && meng.snap_fit == 1 && meng.snap == 1)
			{
				mx3 = mx2;
				my3 = my2;

				mx3 += meng.pre_size.x / 2;

				mx4 = mx3 + meng.pre_size.x;

				SnapToGrid(&mx4, &my3);

				mx4 -= mx3;

				aspect = (float)meng.pre_size.x / meng.pre_size.y;

				mx3 = (mx4 - meng.pre_size.x) / 2;

				my4 = (float)mx4 / aspect;
				my3 = (my4 - meng.pre_size.y) / 2;

				DrawGraphic(mx2 - mx3, my2 + my3, mx4, my4, 0, meng.obj.color.r, meng.obj.color.g, meng.obj.color.b, mgg_map[meng.tex_MGGID].frames[meng.tex_ID], 128,
					meng.obj.texpan.x, meng.obj.texpan.y, meng.obj.texsize.x, meng.obj.texsize.y, 16, 2);
			}
			else
			if ((meng.select_edge == 1) && meng.snap_fit == 1 && meng.snap == 1)
			{
				mx3 = mx2;
				my3 = my2;

				my3 -= meng.pre_size.y / 2;

				my4 = my3 + meng.pre_size.y;

				SnapToGrid(&mx3, &my4);

				my4 -= my3;

				aspect = (float)meng.pre_size.x / meng.pre_size.y;

				my3 = (my4 - meng.pre_size.y) / 2;

				mx4 = (float)my4 * aspect;
				mx3 = (mx4 - meng.pre_size.x) / 2;

				DrawGraphic(mx2 + mx3, my2 + my3, mx4, my4, 0, meng.obj.color.r, meng.obj.color.g, meng.obj.color.b, mgg_map[meng.tex_MGGID].frames[meng.tex_ID], 128,
					meng.obj.texpan.x, meng.obj.texpan.y, meng.obj.texsize.x, meng.obj.texsize.y, 16, 2);
			}
			else
			if ((meng.select_edge == 7) && meng.snap_fit == 1 && meng.snap == 1)
			{
				mx3 = mx2;
				my3 = my2;

				my3 += meng.pre_size.y / 2;

				my4 = my3 + meng.pre_size.y;

				SnapToGrid(&mx3, &my4);

				my4 -= my3;

				aspect = (float)meng.pre_size.x / meng.pre_size.y;

				my3 = (my4 - meng.pre_size.y) / 2;

				mx4 = (float)my4 * aspect;
				mx3 = (mx4 - meng.pre_size.x) / 2;

				DrawGraphic(mx2 + mx3, my2 - my3, mx4, my4, 0, meng.obj.color.r, meng.obj.color.g, meng.obj.color.b, mgg_map[meng.tex_MGGID].frames[meng.tex_ID], 128,
					meng.obj.texpan.x, meng.obj.texpan.y, meng.obj.texsize.x, meng.obj.texsize.y, 16, 2);
			}
			else
				DrawGraphic(mx2, my2, meng.pre_size.x, meng.pre_size.y, 0, meng.obj.color.r, meng.obj.color.g, meng.obj.color.b, mgg_map[meng.tex_MGGID].frames[meng.tex_ID], 128,
				meng.obj.texpan.x, meng.obj.texpan.y, meng.obj.texsize.x, meng.obj.texsize.y, 16, 2);

		}

		if (meng.command == ADD_SPRITE)
		{
			int32 mx2 = mx, my2 = my;

			SnapToGrid(&mx2, &my2);

			PositionToEdge(&mx2, &my2, meng.spr.body.size.x, meng.spr.body.size.y);

			DrawGraphic(mx2, my2, meng.spr.body.size.x, meng.spr.body.size.y, 0, meng.obj.color.r, meng.obj.color.g, meng.obj.color.b,
				mgg_game[st.Game_Sprites[meng.sprite_selection].MGG_ID].frames[st.Game_Sprites[meng.sprite_selection].frame[meng.sprite_frame_selection]], 128,
				meng.obj.texpan.x, meng.obj.texpan.y, meng.obj.texsize.x, meng.obj.texsize.y, 16, 2);
		}

		if (meng.command == NADD_LIGHT)
		{
			int32 mx2 = mx, my2 = my;

			SnapToGrid(&mx2, &my2);

			Pos tmp = st.game_lightmaps[i].w_pos;
			char textI[2] = { 124, 0 };

			DrawGraphic(mx2, my2, 300, 300, 0, 255, 255, 255, st.BasicTex, 128, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
			DrawStringUI(textI, mx2, my2, 1024, 1024, 0, 0, 0, 0, 255, 1, 2048, 2048, 0);
		}
		
		if (meng.command == DRAW_SECTOR)
		{
			int32 mx2 = mx, my2 = my;

			SnapToGrid(&mx2, &my2);

			DrawCircle(mx2, my2, 128, 255, 118, 117, 128, 16);
		}

		if (meng.command == DRAW_SECTOR2)
		{
			i = meng.com_id;

			if (st.Current_Map.sector[i].id > -1 && st.Current_Map.sector[i].num_vertexadded == 1)
			{
				Pos tmp;
				if (st.keys[LSHIFT_KEY].state)
				{
					tmp = st.mouse;
					STW(&tmp.x, &tmp.y);
				}
				else
				{
					tmp.x = st.mouse.x;
					STW(&tmp.x, &tmp.y);

					tmp.y = st.Current_Map.sector[i].vertex[0].y;
				}

				SnapToGrid(&tmp.x, &tmp.y);

				DrawLine(st.Current_Map.sector[i].vertex[0].x, st.Current_Map.sector[i].vertex[0].y, tmp.x, tmp.y, 214, 48, 49, 255, 32, 16);
				//DrawGraphic(st.Current_Map.sector[i].vertex[0].x, st.Current_Map.sector[i].vertex[0].y, 255, 118, 117, 255, 0, 0, mgg_sys[0].frames[4], 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				//DrawGraphic(tmp.x, tmp.y, 256, 256, 0, 255, 118, 117, mgg_sys[0].frames[4], 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawCircle(st.Current_Map.sector[i].vertex[0].x, st.Current_Map.sector[i].vertex[0].y, 128, 255, 118, 117, 255, 16);
				DrawCircle(tmp.x, tmp.y, 128, 255, 118, 117, 255, 16);
			}
		}

		if (meng.gridsize > 0)
		{
			float camx = st.Camera.position.x / st.Camera.dimension.x;
			float camx_w = st.Camera.position.x + (GAME_WIDTH / st.Camera.dimension.x);
			float gx_w = GAME_WIDTH / st.Camera.dimension.x;
			float camx_m = st.Camera.position.x * st.Camera.dimension.x;
				
			float camy = st.Camera.position.y / st.Camera.dimension.y;
			float camy_h = st.Camera.position.y + (GAME_HEIGHT / st.Camera.dimension.y);
			float gy_h = GAME_HEIGHT / st.Camera.dimension.y;
			float camy_m = st.Camera.position.y * st.Camera.dimension.y;

			int64 st_i = camx_m + ((GAME_WIDTH / meng.gridsize) - ((((int32)camx_m - GAME_UNIT_MIN) % (int32)gx_w) % (GAME_WIDTH / meng.gridsize)));

			Pos sp;

			sp.x = sp.y = 0;

			STW(&sp.x, &sp.y);

			for (i = (float)st_i / st.Camera.dimension.x; i < camx_w; i += ((float)GAME_WIDTH / st.Camera.dimension.x) / meng.gridsize)
				DrawLine(i, sp.y, i, camy_h, 64, 64, 64, 255, 16 / st.Camera.dimension.x, 54);

			int32 grid_ceil = ceil((double)meng.gridsize / GAME_ASPECT);

			st_i = camy_m + ((GAME_HEIGHT / grid_ceil) - ((((int32)camy_m - GAME_UNIT_MIN) % (int32)gy_h) % (int32)(GAME_HEIGHT / grid_ceil)));

			for (i = (float)st_i / st.Camera.dimension.y; i < camy_h; i += ((float)GAME_HEIGHT / st.Camera.dimension.y) / grid_ceil)
				DrawLine(sp.x, i, camx_w, i, 64, 64, 64, 255, 16 / st.Camera.dimension.y, 54);
			
			int32 g_sx = GAME_WIDTH / meng.gridsize;
			int32 g_sy = GAME_HEIGHT / grid_ceil;

			p = st.mouse;
			STW(&p.x, &p.y);

			p.x *= st.Camera.dimension.x;
			p.y *= st.Camera.dimension.y;

			st_i = (float)((((p.x - GAME_UNIT_MIN) % GAME_WIDTH) % (GAME_WIDTH / meng.gridsize)));
			int64 st_y = (float)((((p.y - GAME_UNIT_MIN) % GAME_HEIGHT) % (GAME_HEIGHT / grid_ceil)));

			DrawCircle(st_i > g_sx / 2 ? (float)((p.x + g_sx - st_i) / st.Camera.dimension.x) : (float)((p.x - st_i) / st.Camera.dimension.x),
				st_y > g_sy / 2 ? (float)((p.y + g_sy - st_y) / st.Camera.dimension.y) : (float)((p.y - st_y) / st.Camera.dimension.y), 32, 255, 0, 0, 255, 54);
		}

		if (meng.sub_com == OBJEXTRUDE && meng.command2 == EDIT_OBJ && meng.obj_edit_selection != -1)
		{
			i = meng.obj_edit_selection;

			p = st.Current_Map.obj[i].position;
			s = st.Current_Map.obj[i].size;

			AddCamCalc(&p, &s);
			char icon[2];

			icon[0] = FOWARD2_ICON;
			icon[1] = '\0';

			DrawStringUI(icon, p.x + 512 + s.x / 2, p.y, 1, 1, 0, 255, 255, 255, 255, 1, 2048, 2048, 2);

			icon[0] = FOWARDJUMP2_ICON;

			DrawStringUI(icon, p.x + 512 + s.x / 2, p.y - 512, 1, 1, 0, 255, 255, 255, 255, 1, 2048, 2048, 2);

			icon[0] = BACK2_ICON;

			DrawStringUI(icon, p.x - 512 - s.x / 2, p.y, 1, 1, 0, 255, 255, 255, 255, 1, 2048, 2048, 2);

			icon[0] = BACKJUMP2_ICON;

			DrawStringUI(icon, p.x - 512 - s.x / 2, p.y - 512, 1, 1, 0, 255, 255, 255, 255, 1, 2048, 2048, 2);

			icon[0] = FOWARD2_ICON;
			icon[1] = '\0';

			DrawStringUI(icon, p.x - 600, p.y + 512 + s.y / 2, 1, 1, 900, 255, 255, 255, 255, 1, 2048, 2048, 2);

			icon[0] = FOWARDJUMP2_ICON;

			DrawStringUI(icon, p.x + 600, p.y + 512 + s.y / 2, 1, 1, 900, 255, 255, 255, 255, 1, 2048, 2048, 2);

			icon[0] = BACK2_ICON;

			DrawStringUI(icon, p.x - 600, p.y - 512 - s.y / 2, 1, 1, 900, 255, 255, 255, 255, 1, 2048, 2048, 2);

			icon[0] = BACKJUMP2_ICON;

			DrawStringUI(icon, p.x + 600, p.y - 512 -  s.y / 2, 1, 1, 900, 255, 255, 255, 255, 1, 2048, 2048, 2);
		}

		if (st.num_lights > 0)
		{
			for (i = 1; i <= st.num_lights; i++)
			{
				Pos tmp = st.game_lightmaps[i].w_pos;
				char textI[2] = { 124, 0 };

				//WTSci(&tmp.x, &tmp.y);

				tmp.x -= st.Camera.position.x;
				tmp.y -= st.Camera.position.y;

				tmp.x *= st.Camera.dimension.x;
				tmp.y *= st.Camera.dimension.y;

				if (st.game_lightmaps[i].type == SPOTLIGHT)
				{
					DrawLine(st.game_lightmaps[i].w_pos.x + 12, st.game_lightmaps[i].w_pos.y + 12, st.game_lightmaps[i].s_dir.x + 12, st.game_lightmaps[i].s_dir.y + 12, 0, 0, 0, 255, 32, 16);
					DrawLine(st.game_lightmaps[i].w_pos.x, st.game_lightmaps[i].w_pos.y, st.game_lightmaps[i].s_dir.x, st.game_lightmaps[i].s_dir.y, 235, 235, 235, 255, 32, 16);
					DrawCircle(st.game_lightmaps[i].s_dir.x + 12, st.game_lightmaps[i].s_dir.y + 12, 32, 0, 0, 0, 255, 16);
					DrawCircle(st.game_lightmaps[i].s_dir.x, st.game_lightmaps[i].s_dir.y, 32, 255, 255, 255, 255, 16);
				}

				//DrawUI(tmp.x, tmp.y, 512, 512, 0, 255, 255, 255, 0, 0, 32768, 32768, st.BasicTex, 255, 0);
				DrawGraphic(st.game_lightmaps[i].w_pos.x, st.game_lightmaps[i].w_pos.y, 300, 300, 0, 255, 255, 255, st.BasicTex,
					255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawStringUI(textI, tmp.x, tmp.y, 1024, 1024, 0, 0, 0, 0, 255, 1, 2048, 2048, 0);
			}
		}

		if (st.Current_Map.num_sprites > 0)
		{
			for (i = 0; i < st.Current_Map.num_sprites; i++)
			{
				p = st.Current_Map.sprites[i].position;
				Pos s = st.Current_Map.sprites[i].body.size;


				if (st.Game_Sprites[st.Current_Map.sprites[i].GameID].flags & 4)
					s.y = (s.y * st.Current_Map.sprites[i].size_m.y) + st.Current_Map.sprites[i].size_a.y;

				p.y -= (s.y / 2) + 256;

				AddCamCalc(&p, &s);

				DrawStringUI(StringFormat("%s - %d", st.Game_Sprites[st.Current_Map.sprites[i].GameID].name, i), p.x + 16, p.y + 16, 1024, 1024, 0, 0, 0, 0, 255, 0, 1536, 1536, 0);
				DrawStringUI(StringFormat("%s - %d", st.Game_Sprites[st.Current_Map.sprites[i].GameID].name, i), p.x, p.y, 1024, 1024, 0, 255, 255, 255, 255, 0, 1536, 1536, 0);

				if (st.Current_Map.sprites[i].num_tags > 0)
				{
					j = st.Current_Map.sprites[i].GameID;

					for (int16 k = 0; k < st.Game_Sprites[j].num_tags; k++)
					{
						if (!strcmp(st.Game_Sprites[j].tag_names[k], "INPUT"))
						{
							int16 l = st.Current_Map.sprites[i].tags[k];

							if (l != 0)
							{
								for (int16 m = 0; m < st.Current_Map.num_sprites; m++)
								{
									if (m == i)
										continue;

									if (st.Current_Map.sprites[m].num_tags > 0)
									{
										int16 n = st.Current_Map.sprites[m].GameID;

										for (int16 o = 0; o < st.Current_Map.sprites[m].num_tags; o++)
										{
											if (!strcmp(st.Game_Sprites[n].tag_names[o], "OUTPUT") && st.Current_Map.sprites[m].tags[o] == l)
											{
												p = st.Current_Map.sprites[i].position;
												s = st.Current_Map.sprites[m].position;

												if (p.z < 24)
												{
													p.x -= (float)st.Camera.position.x*st.Current_Map.fr_v;
													p.y -= (float)st.Camera.position.y*st.Current_Map.fr_v;

													p.x += st.Camera.position.x;
													p.y += st.Camera.position.y;
												}

												if (p.z > 31 && p.z < 40)
												{
													p.x -= (float)st.Camera.position.x*st.Current_Map.bck1_v;
													p.y -= (float)st.Camera.position.y*st.Current_Map.bck1_v;

													p.x += st.Camera.position.x;
													p.y += st.Camera.position.y;
												}

												if (p.z > 39 && p.z < 48)
												{
													p.x -= (float)st.Camera.position.x * st.Current_Map.bck2_v;
													p.y -= (float)st.Camera.position.y * st.Current_Map.bck2_v;

													p.x += st.Camera.position.x;
													p.y += st.Camera.position.y;
												}

												if (p.z > 47)
												{
													p.x /= (float)st.Camera.dimension.x;
													p.y /= (float)st.Camera.dimension.y;

													p.x += st.Camera.position.x;
													p.y += st.Camera.position.y;
												}

												if (s.z < 24)
												{
													s.x -= (float)st.Camera.position.x*st.Current_Map.fr_v;
													s.y -= (float)st.Camera.position.y*st.Current_Map.fr_v;

													s.x += st.Camera.position.x;
													s.y += st.Camera.position.y;
												}

												if (s.z > 31 && s.z < 40)
												{
													s.x -= (float)st.Camera.position.x*st.Current_Map.bck1_v;
													s.y -= (float)st.Camera.position.y*st.Current_Map.bck1_v;

													s.x += st.Camera.position.x;
													s.y += st.Camera.position.y;
												}

												if (s.z > 39 && s.z < 48)
												{
													s.x -= (float)st.Camera.position.x * st.Current_Map.bck2_v;
													s.y -= (float)st.Camera.position.y * st.Current_Map.bck2_v;

													s.x += st.Camera.position.x;
													s.y += st.Camera.position.y;
												}

												if (s.z > 47)
												{
													s.x /= (float)st.Camera.dimension.x;
													s.y /= (float)st.Camera.dimension.y;

													s.x += st.Camera.position.x;
													s.y += st.Camera.position.y;
												}

												DrawLine(p.x, p.y, s.x, s.y, 253, 203, 110, 255, 16.0f / st.Camera.dimension.x, 16);

												break;
											}
										}
									}
								}
							}
						}

						if (!strcmp(st.Game_Sprites[j].tag_names[k], "OUTPUT"))
						{
							int16 l = st.Current_Map.sprites[i].tags[k];

							if (l != 0)
							{
								for (int16 m = 0; m < st.Current_Map.num_sprites; m++)
								{
									if (m == i)
										continue;

									if (st.Current_Map.sprites[m].num_tags > 0)
									{
										int16 n = st.Current_Map.sprites[m].GameID;

										for (int16 o = 0; o < st.Current_Map.sprites[m].num_tags; o++)
										{
											if (!strcmp(st.Game_Sprites[n].tag_names[o], "INPUT") && st.Current_Map.sprites[m].tags[o] == l)
											{
												p = st.Current_Map.sprites[i].position;
												s = st.Current_Map.sprites[m].position;

												if (p.z < 24)
												{
													p.x -= (float)st.Camera.position.x*st.Current_Map.fr_v;
													p.y -= (float)st.Camera.position.y*st.Current_Map.fr_v;

													p.x += st.Camera.position.x;
													p.y += st.Camera.position.y;
												}

												if (p.z > 31 && p.z < 40)
												{
													p.x -= (float)st.Camera.position.x*st.Current_Map.bck1_v;
													p.y -= (float)st.Camera.position.y*st.Current_Map.bck1_v;

													p.x += st.Camera.position.x;
													p.y += st.Camera.position.y;
												}

												if (p.z > 39 && p.z < 48)
												{
													p.x -= (float)st.Camera.position.x * st.Current_Map.bck2_v;
													p.y -= (float)st.Camera.position.y * st.Current_Map.bck2_v;

													p.x += st.Camera.position.x;
													p.y += st.Camera.position.y;
												}

												if (p.z > 47)
												{
													p.x /= (float)st.Camera.dimension.x;
													p.y /= (float)st.Camera.dimension.y;

													p.x += st.Camera.position.x;
													p.y += st.Camera.position.y;
												}

												if (s.z < 24)
												{
													s.x -= (float)st.Camera.position.x*st.Current_Map.fr_v;
													s.y -= (float)st.Camera.position.y*st.Current_Map.fr_v;

													s.x += st.Camera.position.x;
													s.y += st.Camera.position.y;
												}

												if (s.z > 31 && s.z < 40)
												{
													s.x -= (float)st.Camera.position.x*st.Current_Map.bck1_v;
													s.y -= (float)st.Camera.position.y*st.Current_Map.bck1_v;

													s.x += st.Camera.position.x;
													s.y += st.Camera.position.y;
												}

												if (s.z > 39 && s.z < 48)
												{
													s.x -= (float)st.Camera.position.x * st.Current_Map.bck2_v;
													s.y -= (float)st.Camera.position.y * st.Current_Map.bck2_v;

													s.x += st.Camera.position.x;
													s.y += st.Camera.position.y;
												}

												if (s.z > 47)
												{
													s.x /= (float)st.Camera.dimension.x;
													s.y /= (float)st.Camera.dimension.y;

													s.x += st.Camera.position.x;
													s.y += st.Camera.position.y;
												}

												DrawLine(p.x, p.y, s.x, s.y, 253, 203, 110, 255, 16.0f / st.Camera.dimension.x, 16);

												break;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		switch (meng.command2)
		{
			case EDIT_OBJ:
			case EDIT_SPRITE:

				if (meng.command2 == EDIT_OBJ)
				{
					p = st.Current_Map.obj[meng.obj_edit_selection].position;
					s = st.Current_Map.obj[meng.obj_edit_selection].size;
				}
				else
				{
					p = st.Current_Map.sprites[meng.sprite_edit_selection].position;
					s = st.Current_Map.sprites[meng.sprite_edit_selection].body.size;
				}

				if (p.z < 24)
				{
					p.x -= (float)st.Camera.position.x*st.Current_Map.fr_v;
					p.y -= (float)st.Camera.position.y*st.Current_Map.fr_v;

					p.x += st.Camera.position.x;
					p.y += st.Camera.position.y;
				}

				if (p.z > 31 && p.z < 40)
				{
					p.x -= (float)st.Camera.position.x*st.Current_Map.bck1_v;
					p.y -= (float)st.Camera.position.y*st.Current_Map.bck1_v;

					p.x += st.Camera.position.x;
					p.y += st.Camera.position.y;

					//p.x *= st.Camera.dimension.x;
					//p.y *= st.Camera.dimension.y;

					//s.x *= st.Camera.dimension.x;
					//s.y *= st.Camera.dimension.y;
				}

				if (p.z > 39 && p.z < 48)
				{
					p.x -= (float)st.Camera.position.x * st.Current_Map.bck2_v;
					p.y -= (float)st.Camera.position.y * st.Current_Map.bck2_v;

					p.x += st.Camera.position.x;
					p.y += st.Camera.position.y;

					//p.x *= (float) st.Camera.dimension.x;
					//p.y *= (float) st.Camera.dimension.y;

					//s.x *= (float) st.Camera.dimension.x;
					//s.y *= (float) st.Camera.dimension.y;
				}

				if (p.z > 47)
				{
					p.x /= (float)st.Camera.dimension.x;
					p.y /= (float)st.Camera.dimension.y;

					p.x += st.Camera.position.x;
					p.y += st.Camera.position.y;

					s.x /= (float)st.Camera.dimension.x;
					s.y /= (float)st.Camera.dimension.y;
				}

				DrawLine(p.x + 12 - (s.x / 2), p.y + 12 - (s.y / 2), p.x + 12 + (s.x / 2), p.y + 12 - (s.y / 2), 0, 0, 0, 255, 16.0f / st.Camera.dimension.x, 16);
				DrawLine(p.x - (s.x / 2), p.y - (s.y / 2), p.x + (s.x / 2), p.y - (s.y / 2), 9, 132, 237, 255, 16.0f / st.Camera.dimension.x, 16);

				DrawLine(p.x + 12 + (s.x / 2), p.y + 12 - (s.y / 2), p.x + 12 + (s.x / 2), p.y + 12 + (s.y / 2), 0, 0, 0, 255, 16.0f / st.Camera.dimension.x, 16);
				DrawLine(p.x + (s.x / 2), p.y - (s.y / 2), p.x + (s.x / 2), p.y + (s.y / 2), 9, 132, 237, 255, 16.0f / st.Camera.dimension.x, 16);

				DrawLine(p.x + 12 + (s.x / 2), p.y + 12 + (s.y / 2), p.x + 12 - (s.x / 2), p.y + 12 + (s.y / 2), 0, 0, 0, 255, 16.0f / st.Camera.dimension.x, 16);
				DrawLine(p.x + (s.x / 2), p.y + (s.y / 2), p.x - (s.x / 2), p.y + (s.y / 2), 9, 132, 237, 255, 16.0f / st.Camera.dimension.x, 16);

				DrawLine(p.x + 12 - (s.x / 2), p.y + 12 + (s.y / 2), p.x + 12 - (s.x / 2), p.y + 12 - (s.y / 2), 0, 0, 0, 255, 16.0f / st.Camera.dimension.x, 16);
				DrawLine(p.x - (s.x / 2), p.y + (s.y / 2), p.x - (s.x / 2), p.y - (s.y / 2), 9, 132, 237, 255, 16.0f / st.Camera.dimension.x, 16);

				//Vertices

				DrawGraphic(p.x + 12 - (s.x / 2), p.y + 12 - (s.y / 2), 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawGraphic(p.x - (s.x / 2), p.y - (s.y / 2), 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 9, 132, 237, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

				DrawGraphic(p.x + 12 + (s.x / 2), p.y + 12 - (s.y / 2), 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawGraphic(p.x + (s.x / 2), p.y - (s.y / 2), 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 9, 132, 237, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

				DrawGraphic(p.x + 12 + (s.x / 2), p.y + 12 + (s.y / 2), 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawGraphic(p.x + (s.x / 2), p.y + (s.y / 2), 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 9, 132, 237, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

				DrawGraphic(p.x + 12 - (s.x / 2), p.y + 12 + (s.y / 2), 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawGraphic(p.x - (s.x / 2), p.y + (s.y / 2), 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 9, 132, 237, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

				if (meng.sub_com == SCALER_SELECT)
				{
					DrawGraphic(p.x + 12 - (s.x / 2) - 96, p.y + 12, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
					DrawGraphic(p.x - (s.x / 2) - 96, p.y, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 214, 48, 49, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

					DrawGraphic(p.x + 12 + (s.x / 2) + 96, p.y + 12, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
					DrawGraphic(p.x + (s.x / 2) + 96, p.y, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 214, 48, 49, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

					DrawGraphic(p.x + 12, p.y + 12 - (s.y / 2) - 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
					DrawGraphic(p.x, p.y - (s.y / 2) - 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 214, 48, 49, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

					DrawGraphic(p.x + 12, p.y + 12 + (s.y / 2) + 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
					DrawGraphic(p.x, p.y + (s.y / 2) + 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 214, 48, 49, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

					DrawGraphic(p.x + 12 - (s.x / 2) - 96, p.y + 12 - (s.y / 2) - 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
					DrawGraphic(p.x - (s.x / 2) - 96, p.y - (s.y / 2) - 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 214, 48, 49, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

					DrawGraphic(p.x + 12 + (s.x / 2) + 96, p.y + 12 - (s.y / 2) - 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
					DrawGraphic(p.x + (s.x / 2) + 96, p.y - (s.y / 2) - 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 214, 48, 49, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

					DrawGraphic(p.x + 12 + (s.x / 2) + 96, p.y + 12 + (s.y / 2) + 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
					DrawGraphic(p.x + (s.x / 2) + 96, p.y + (s.y / 2) + 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 214, 48, 49, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

					DrawGraphic(p.x + 12 - (s.x / 2) - 96, p.y + 12 + (s.y / 2) + 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
					DrawGraphic(p.x - (s.x / 2) - 96, p.y + (s.y / 2) + 96, 128.0f / st.Camera.dimension.x, 128.0f / st.Camera.dimension.x, 0, 214, 48, 49, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				}

				//Anchor point

				DrawGraphic(p.x + 96 + 12, p.y + 12, 96.0f / st.Camera.dimension.x, 96.0f / st.Camera.dimension.x, 450, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawGraphic(p.x + 96, p.y, 96.0f / st.Camera.dimension.x, 96.0f / st.Camera.dimension.x, 450, 9, 132, 237, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

				DrawGraphic(p.x - 96 + 12, p.y + 12, 96.0f / st.Camera.dimension.x, 96.0f / st.Camera.dimension.x, 450, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawGraphic(p.x - 96, p.y, 96.0f / st.Camera.dimension.x, 96.0f / st.Camera.dimension.x, 450, 9, 132, 237, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

				DrawGraphic(p.x + 12, p.y + 96 + 12, 96.0f / st.Camera.dimension.x, 96.0f / st.Camera.dimension.x, 450, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawGraphic(p.x, p.y + 96, 96.0f / st.Camera.dimension.x, 96.0f / st.Camera.dimension.x, 450, 9, 132, 237, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

				DrawGraphic(p.x + 12, p.y - 96 + 12, 96.0f / st.Camera.dimension.x, 96.0f / st.Camera.dimension.x, 450, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawGraphic(p.x, p.y - 96, 96.0f / st.Camera.dimension.x, 96.0f / st.Camera.dimension.x, 450, 9, 132, 237, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

				break;

			case NEDIT_LIGHT:
				p = st.game_lightmaps[meng.light_edit_selection].w_pos;

				DrawGraphic(p.x + 12, p.y + 12, 96.0f / st.Camera.dimension.x, 96.0f / st.Camera.dimension.x, 450, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawGraphic(p.x, p.y, 96.0f / st.Camera.dimension.x, 96.0f / st.Camera.dimension.x, 450, 9, 132, 237, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

				break;
				
			case EDIT_SECTOR:

				if (meng.sector_edit_selection >= 1000)
					i = meng.sector_edit_selection - 1000;
				else
					i = meng.sector_edit_selection;

				p = st.Current_Map.sector[i].vertex[0];
				s = st.Current_Map.sector[i].vertex[1];

				DrawLine(p.x + 12, p.y + 12, s.x, s.y, 0, 0, 0, 255, 32, 16);
				DrawLine(p.x, p.y, s.x, s.y, 30, 162, 267, 255, 32, 16);

				DrawCircle(p.x + 12, p.y + 12, 128, 0, 0, 0, 255, 16);
				DrawCircle(p.x, p.y, 128, 9, 132, 237, 255, 16);

				DrawCircle(s.x + 12, s.y + 12, 128, 0, 0, 0, 255, 16);
				DrawCircle(s.x, s.y, 128, 9, 132, 237, 255, 16);

				break;

				/*
				DrawLine(p.x + 12 - 128, p.y + 12 - 128, s.x + 12 + 128, s.y + 12 - 128, 0, 0, 0, 255, 16.0f / st.Camera.dimension.x, 16);
				DrawLine(p.x - 128, p.y - 128, s.x + 128, s.y - 128, 9, 132, 237, 255, 16.0f / st.Camera.dimension.x, 16);

				DrawLine(s.x + 12 + 128, s.y + 12 - 128, s.x + 12 + 128, s.y + 12 + 128, 0, 0, 0, 255, 16.0f / st.Camera.dimension.x, 16);
				DrawLine(s.x + 128, s.y - 128, s.x + 128, s.y + 128, 9, 132, 237, 255, 16.0f / st.Camera.dimension.x, 16);

				DrawLine(s.x + 12 + 128, s.y + 12 + 128, s.x + 12 - 128, s.y + 12 + 128, 0, 0, 0, 255, 16.0f / st.Camera.dimension.x, 16);
				DrawLine(s.x + 128, s.y + 128, p.x - 128, p.y + 128, 9, 132, 237, 255, 16.0f / st.Camera.dimension.x, 16);

				DrawLine(p.x + 12 - 128, p.y + 12 + 128, p.x + 12 - 128, p.y + 12 - 128, 0, 0, 0, 255, 16.0f / st.Camera.dimension.x, 16);
				DrawLine(p.x - 128, p.y + 128, p.x - 128, p.y - 128, 9, 132, 237, 255, 16.0f / st.Camera.dimension.x, 16);

				//Vertices

				DrawGraphic(p.x + 12 - 128, p.y + 12 - 128, 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawGraphic(p.x - 128, p.y - 128, 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 9, 132, 237, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

				DrawGraphic(s.x + 12 + 128, s.y + 12 - 128, 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawGraphic(s.x + 128, s.y - 128, 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 9, 132, 237, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

				DrawGraphic(s.x + 12 + 128, s.y + 12 + 128, 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawGraphic(s.x + 128, s.y + 128, 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 9, 132, 237, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

				DrawGraphic(p.x + 12 - 128, p.y + 12 + 128, 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawGraphic(p.x - 128, p.y + 128, 64.0f / st.Camera.dimension.x, 64.0f / st.Camera.dimension.x, 0, 9, 132, 237, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

				//Anchor point

				DrawGraphic(p.x + 96 + 12, p.y + 12, 96.0f / st.Camera.dimension.x, 96.0f / st.Camera.dimension.x, 450, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawGraphic(p.x + 96, p.y, 96.0f / st.Camera.dimension.x, 96.0f / st.Camera.dimension.x, 450, 9, 132, 237, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

				DrawGraphic(s.x - 96 + 12, s.y + 12, 96.0f / st.Camera.dimension.x, 96.0f / st.Camera.dimension.x, 450, 0, 0, 0, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);
				DrawGraphic(s.x - 96, s.y, 96.0f / st.Camera.dimension.x, 96.0f / st.Camera.dimension.x, 450, 9, 132, 237, st.BasicTex, 255, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, 16, 2);

				break;
				*/
		}
	}

	if (meng.mouse_move == 1)
	{
		p = meng.mouse_move_pos;

		STWci(&p.x, &p.y);

		s = st.mouse;
		STWci(&s.x, &s.y);

		DrawLine(p.x, p.y, s.x, s.y, 128, 128, 128, 255, 16, 2);
	}
}

struct nk_color ColorPicker(struct nk_color color)
{
	if (nk_combo_begin_color(ctx, color, nk_vec2(200, 250)))
	{
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf c = nk_color_picker(ctx, nk_color_cf(color), NK_RGBA);
		color = nk_rgba_cf(c);
		nk_layout_row_dynamic(ctx, 25, 1);
		color.r = (nk_byte)nk_propertyi(ctx, "R:", 0, color.r, 255, 1, 1);
		color.g = (nk_byte)nk_propertyi(ctx, "G:", 0, color.g, 255, 1, 1);
		color.b = (nk_byte)nk_propertyi(ctx, "B:", 0, color.b, 255, 1, 1);
		color.a = (nk_byte)nk_propertyi(ctx, "A:", 0, color.a, 255, 1, 1);

		nk_combo_end(ctx);
	}

	return color;
}

int MapProperties()
{
	static int bk3_tex = 0, state = 0;
	static struct nk_color amb_color;

	amb_color.r = st.Current_Map.amb_color.r;
	amb_color.g = st.Current_Map.amb_color.g;
	amb_color.b = st.Current_Map.amb_color.b;
	amb_color.a = 255;

	if (nk_begin(ctx, "Map properties", nk_rect((st.screenx / 2) - 200, (st.screeny / 2) - (370/2), 400, 370), NK_WINDOW_BORDER | NK_WINDOW_CLOSABLE | NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(ctx, 30, 1);
		nk_checkbox_label(ctx, "Textured Background 3", &bk3_tex);

		nk_layout_row_dynamic(ctx, 30, 3);
		nk_spacing(ctx, 1);
		if (bk3_tex)
		{
			st.Current_Map.bcktex_id = meng.tex_ID;
			st.Current_Map.bcktex_mgg = meng.tex_MGGID;

			if (nk_button_label(ctx, "BK3 texture"))
				state = 1;
		}
		else
		{
			st.Current_Map.bcktex_id = -1;
			st.Current_Map.bcktex_mgg = 0;
			nk_spacing(ctx, 1);
		}

		nk_label(ctx, " ", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, " ", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 30, 2);
		st.Current_Map.bck2_v=nk_propertyf(ctx, "Background 2 vel:", 0.0, st.Current_Map.bck2_v, 512, 0.1, 0.1);
		st.Current_Map.bck1_v = nk_propertyf(ctx, "Background 1 vel:", 0.0, st.Current_Map.bck1_v, 512, 0.1, 0.1);
		st.Current_Map.fr_v = nk_propertyf(ctx, "Foreground vel:", 0.0, st.Current_Map.fr_v, 512, 0.1, 0.1);

		amb_color = ColorPicker(amb_color);

		st.Current_Map.amb_color.r = amb_color.r;
		st.Current_Map.amb_color.g = amb_color.g;
		st.Current_Map.amb_color.b = amb_color.b;

		nk_layout_row_dynamic(ctx, 30, 1);
		nk_label(ctx, " ", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 30, 2);
		st.Current_Map.bck3_pan.x = nk_propertyi(ctx, "BCK3 pan x:", 0, st.Current_Map.bck3_pan.x, TEX_PAN_RANGE, 512, 100);
		st.Current_Map.bck3_pan.y = nk_propertyi(ctx, "BCK3 pan Y:", 0, st.Current_Map.bck3_pan.y, TEX_PAN_RANGE, 512, 100);
		st.Current_Map.bck3_size.x = nk_propertyi(ctx, "BCK3 size x:", 0, st.Current_Map.bck3_size.x, 65536, 512, 100);
		st.Current_Map.bck3_size.y = nk_propertyi(ctx, "BCK3 size y:", 0, st.Current_Map.bck3_size.y, 65536, 512, 100);

		nk_layout_row_dynamic(ctx, 30, 1);
		nk_label(ctx, " ", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 30, 2);
		nk_spacing(ctx, 1);
		if (nk_button_label(ctx, "OK"))
		{
			nk_end(ctx);
			return 1;
		}
	}

	nk_end(ctx);

	if (state)
		TextureListSelection();

	if (meng.command == ADD_OBJ)
	{
		meng.command = meng.pannel_choice = SELECT_EDIT;
		state = 0;
	}

	st.mouse1 = 0;

	return 0;
}

int CameraProperties()
{
	char text[16], len;
	static int lxm, lym;
	if (nk_begin(ctx, "Camera properties", nk_rect((st.screenx / 2) - (650 / 2), (st.screeny / 2) - (512 / 2), 650, 330), NK_WINDOW_BORDER | NK_WINDOW_CLOSABLE | NK_WINDOW_MOVABLE))
	{
		nk_layout_row_dynamic(ctx, 25, 3);

		st.Current_Map.cam_area.area_pos.x = nk_propertyi(ctx, "Area POS X:", GAME_UNIT_MIN, st.Current_Map.cam_area.area_pos.x, GAME_UNIT_MAX, 1024, 128);
		st.Current_Map.cam_area.area_pos.y = nk_propertyi(ctx, "Area POS Y:", GAME_UNIT_MIN, st.Current_Map.cam_area.area_pos.y, GAME_UNIT_MAX, 1024, 128);
		st.Current_Map.cam_area.area_size.x = nk_propertyf(ctx, "Area scale:",0, st.Current_Map.cam_area.area_size.x, 64, 0.1, 0.01);
		st.Current_Map.cam_area.area_size.y = st.Current_Map.cam_area.area_size.x;

		nk_layout_space_begin(ctx, NK_DYNAMIC, 200, INT_MAX);

		nk_layout_space_push(ctx, nk_rect(0.01, 0.05, 0.32, 0.12));
		st.Current_Map.cam_area.max_dim.x = st.Current_Map.cam_area.max_dim.y = nk_propertyf(ctx, "Area MAX scale:", 0.1, st.Current_Map.cam_area.max_dim.x, 64.0, 0.1, 0.01);

		nk_layout_space_push(ctx, nk_rect(0.4, 0.05, 0.59, 0.45));
		if (nk_group_begin(ctx, "X mov. limit", NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
		{
			nk_layout_row_dynamic(ctx, 25, 2);
			nk_checkbox_label(ctx, "Limit X movement", &st.Current_Map.cam_area.horiz_lim);
			nk_button_label(ctx, "Edit X limit");
			st.Current_Map.cam_area.limit[0].x = nk_propertyi(ctx, "Area MIN X:", GAME_UNIT_MIN, st.Current_Map.cam_area.limit[0].x, GAME_UNIT_MAX, 1024, 128);
			st.Current_Map.cam_area.limit[1].x = nk_propertyi(ctx, "Area MAX X:", GAME_UNIT_MIN, st.Current_Map.cam_area.limit[1].x, GAME_UNIT_MAX, 1024, 128);
			nk_group_end(ctx);
		}

		nk_layout_space_push(ctx, nk_rect(0.01, 0.50, 0.35, 0.12));
		nk_button_label(ctx, "Edit area scale");

		nk_layout_space_push(ctx, nk_rect(0.4, 0.55, 0.59, 0.45));
		if (nk_group_begin(ctx, "Y mov. limit", NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
		{
			nk_layout_row_dynamic(ctx, 25, 2);
			nk_checkbox_label(ctx, "Limit Y movement", &st.Current_Map.cam_area.vert_lim);
			nk_button_label(ctx, "Edit Y limit");
			st.Current_Map.cam_area.limit[0].y = nk_propertyi(ctx, "Area MIN Y:", GAME_UNIT_MIN, st.Current_Map.cam_area.limit[0].y, GAME_UNIT_MAX, 1024, 128);
			st.Current_Map.cam_area.limit[1].y = nk_propertyi(ctx, "Area MAX Y:", GAME_UNIT_MIN, st.Current_Map.cam_area.limit[1].y, GAME_UNIT_MAX, 1024, 128);
			nk_group_end(ctx);
		}

		nk_layout_space_end(ctx);

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, " ", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 25, 6);
		nk_spacing(ctx, 5);
		if (nk_button_label(ctx, "ok"))
		{
			nk_end(ctx);
			return 1;
		}

	}

	nk_end(ctx);
	return 0;

}

void TransformBox(Pos *pos, Pos *size, int16 *ang, Pos *tpan, Pos *tsize)
{
	static int chained = 1;
	float aspect;

	if (nk_begin(ctx, "Transform Box", nk_rect((st.screenx / 2) - (300 / 2), (st.screeny / 2) - (450 / 2), 300, 450), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
	{
		nk_layout_row_dynamic(ctx, 100, 1);
		if (nk_group_begin(ctx, "Position", NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
		{
			nk_layout_row_dynamic(ctx, 25, 2);

			pos->x = nk_propertyi(ctx, "X:", GAME_UNIT_MIN, pos->x, GAME_UNIT_MAX, 1024, 128);
			pos->y = nk_propertyi(ctx, "Y:", GAME_UNIT_MIN, pos->y, GAME_UNIT_MAX, 1024, 128);
			pos->z = nk_propertyi(ctx, "Z:", 0, pos->z, 7 * 8, 1, 1);
			*ang = nk_propertyi(ctx, "Ang:", -3600, *ang, 3600*2, 150, 30);

			nk_group_end(ctx);
		}

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, " ", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 75, 1);
		if (nk_group_begin(ctx, "Size", NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
		{
			nk_layout_row_begin(ctx,NK_DYNAMIC,25,3);

			aspect = (float) size->x / size->y;

			nk_layout_row_push(ctx, 0.45f);
			size->x = nk_propertyi(ctx, "W:", GAME_UNIT_MIN, size->x, GAME_UNIT_MAX, 1024, 128);

			if (chained && aspect != 0)
				size->y = (float) size->x / aspect;

			nk_layout_row_push(ctx, 0.1f);
			if (chained)
			{
				ctx->style.button.normal = ctx->style.button.active;
				ctx->style.button.hover = ctx->style.button.active;
				if (nk_button_icon_set(LINK_ICON))
					chained = 0;

				SetThemeBack(ctx,meng.theme);
			}
			else
			if (nk_button_icon_set(UNLINK_ICON))
					chained = 1;


			nk_layout_row_push(ctx, 0.45f);
			size->y = nk_propertyi(ctx, "H:", GAME_UNIT_MIN, size->y, GAME_UNIT_MAX, 1024, 128);

			if (chained)
				size->x = (float) size->y * aspect;

			nk_group_end(ctx);
		}

		if (tpan != NULL && tsize != NULL)
		{
			nk_layout_row_dynamic(ctx, 100, 1);
			if (nk_group_begin(ctx, "Texture", NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
			{
				nk_layout_row_begin(ctx, NK_DYNAMIC, 25, 3);

				if (tsize->y != 0)
					aspect = (float)tsize->x / tsize->y;
				else
					aspect = 0;

				nk_layout_row_push(ctx, 0.45f);
				tsize->x = nk_propertyi(ctx, "SW:", GAME_UNIT_MIN, tsize->x, GAME_UNIT_MAX, 1024, 128);

				if (chained)
					tsize->y = (float)tsize->x / aspect;

				nk_layout_row_push(ctx, 0.1f);
				if (chained)
				{
					ctx->style.button.normal = ctx->style.button.active;
					ctx->style.button.hover = ctx->style.button.active;
					if (nk_button_icon_set(LINK_ICON))
						chained = 0;

					SetThemeBack(ctx, meng.theme);
				}
				else
				if (nk_button_icon_set(UNLINK_ICON))
					chained = 1;


				nk_layout_row_push(ctx, 0.45f);
				tsize->y = nk_propertyi(ctx, "SH:", GAME_UNIT_MIN, tsize->y, GAME_UNIT_MAX, 1024, 128);

				if (chained && aspect != 0)
					tsize->x = (float)tsize->y * aspect;

				nk_layout_row_end(ctx);


				nk_layout_row_begin(ctx, NK_DYNAMIC, 25, 3);

				if (tpan->y != 0)
					aspect = (float)tpan->x / tpan->y;
				else
					aspect = 0;

				nk_layout_row_push(ctx, 0.45f);
				tpan->x = nk_propertyi(ctx, "PW:", GAME_UNIT_MIN, tpan->x, GAME_UNIT_MAX, 1024, 128);

				if (chained && aspect != 0)
					tpan->y = (float)tpan->x / aspect;

				nk_layout_row_push(ctx, 0.1f);
				if (chained)
				{
					ctx->style.button.normal = ctx->style.button.active;
					ctx->style.button.hover = ctx->style.button.active;
					if (nk_button_icon_set(LINK_ICON))
						chained = 0;

					SetThemeBack(ctx, meng.theme);
				}
				else
				if (nk_button_icon_set(UNLINK_ICON))
					chained = 1;


				nk_layout_row_push(ctx, 0.45f);
				tpan->y = nk_propertyi(ctx, "PH:", GAME_UNIT_MIN, tpan->y, GAME_UNIT_MAX, 1024, 128);

				if (chained && aspect != 0)
					tpan->x = (float)tpan->y * aspect;

				nk_layout_row_end(ctx);

				nk_group_end(ctx);
			}
		}

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, " ", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 25, 1);
		if (nk_button_label(ctx, "OK"))
			meng.command = meng.pannel_choice;
	}

	nk_end(ctx);

	FixLayerBar();
}

int FileBrowser(char *filename)
{
	char path[1024];

	register int i, j;

	static int select = -1, time = 0, len, doubleclick = -1, doubleclick2 = -1, backpath_available = 0, fowardpath_available = 0, lenpath;

	static char backpath[2048], fowardpath[2048], temp[2048];

	size_t lenght;

	FILE *f;
	DIR *d;

	if (nk_begin(ctx, "File Browser", nk_rect(st.screenx/2 - 400, st.screeny/2 - 225, 800, 450), NK_WINDOW_BORDER | NK_WINDOW_CLOSABLE | NK_WINDOW_MOVABLE))
	{
		nk_layout_space_begin(ctx, NK_DYNAMIC, 390, INT_MAX);

		nk_layout_space_push(ctx, nk_rect(0.01, 0.01, 0.08, 0.08));
		if (nk_button_label(ctx, "<"))
		{
			if (backpath_available)
			{
				strcpy(temp, UI_Sys.current_path);

				if ((d = opendir(backpath)) != NULL)
				{
					strcpy(fowardpath, UI_Sys.current_path);
					strcpy(UI_Sys.current_path, backpath);
					backpath_available = 0;
					fowardpath_available = 1;
					strcpy(backpath, temp);
					closedir(d);
					SetDirContent(UI_Sys.extension);
					doubleclick = -1;
					doubleclick2 = -1;
					select = -1;
					time = 0;
				}
			}
		}

		nk_layout_space_push(ctx, nk_rect(0.09, 0.01, 0.08, 0.08));
		if(nk_button_label(ctx, ">"))
		{
			if (fowardpath_available)
			{
				strcpy(temp, UI_Sys.current_path);

				if ((d = opendir(fowardpath)) != NULL)
				{
					strcpy(backpath, UI_Sys.current_path);
					strcpy(UI_Sys.current_path, fowardpath);
					backpath_available = 1;
					fowardpath_available = 0;
					strcpy(fowardpath, temp);
					closedir(d);
					SetDirContent(UI_Sys.extension);
					doubleclick = -1;
					doubleclick2 = -1;
					select = -1;
					time = 0;
				}
			}
		}

		nk_layout_space_push(ctx, nk_rect(0.18, 0.01, 0.08, 0.08));
		if (nk_button_label(ctx, "^"))
		{
			strcpy(temp, UI_Sys.current_path);

			lenght = strlen(temp);

			for (i = lenght; i > 0; i--)
			{
				if (temp[i] != 92 && temp[i] != 47)
					temp[i] = 0;
				
				if (temp[i] == 92 || temp[i] == 47)
				{
					temp[i] = 0;
					break;
				}
			}

			if ((d = opendir(temp)) != NULL)
			{
				strcpy(backpath, UI_Sys.current_path);
				strcpy(UI_Sys.current_path, temp);
				backpath_available = 1;
				fowardpath_available = 0;
				closedir(d);
				SetDirContent(UI_Sys.extension);
				doubleclick = -1;
				doubleclick2 = -1;
				select = -1;
				time = 0;
			}
		}

		nk_layout_space_push(ctx, nk_rect(0.27, 0.01, 0.20, 0.08));
//		nk_button_image_label(ctx, nk_image_id(mgg_sys[UI_Sys.mgg_id].frames[UI_Sys.folder_icon].data), "New Folder", NK_TEXT_CENTERED);

		nk_layout_space_push(ctx, nk_rect(0.01, 0.10, 0.99, 0.08));

		lenpath = strlen(UI_Sys.current_path);

		nk_edit_string(ctx, NK_EDIT_FIELD, UI_Sys.current_path, &lenpath, 2048, nk_filter_default);

		nk_layout_space_push(ctx, nk_rect(0.01, 0.20, 0.99, 0.70));

		if (nk_group_begin(ctx, "content", NK_WINDOW_BORDER))
		{
			nk_layout_row_dynamic(ctx, 25, 1);

			if (UI_Sys.num_files > 2)
			{
				for (i = 2, j = 0; i < UI_Sys.num_files; i++)
				{
					if (select == i)
					{
						//if (UI_Sys.filesp[i] == UI_Sys.foldersp[i])
							//nk_select_image_label(ctx, nk_image_id(mgg_sys[UI_Sys.mgg_id].frames[UI_Sys.folder_icon].data), UI_Sys.files[UI_Sys.foldersp[i]], NK_TEXT_RIGHT, 1);
						/*
						{
						if (doubleclick2 == i && (GetTimerM() - time) > 50)
						{
						strcpy(temp, UI_Sys.current_path);
						strcat(temp, "\\");
						strcat(temp, UI_Sys.files[UI_Sys.foldersp[i]]);

						if ((d = opendir(temp)) != NULL)
						{
						strcpy(backpath, UI_Sys.current_path);
						strcpy(UI_Sys.current_path, temp);
						backpath_available = 1;
						closedir(d);
						SetDirContent(UI_Sys.extension);
						doubleclick = -1;
						doubleclick2 = -1;
						select = -1;
						time = 0;
						break;
						}
						}
						else
						{
						doubleclick2 = i;
						time = GetTimerM();
						}
						}
						*/
						//else
							//nk_select_label(ctx, UI_Sys.files[UI_Sys.filesp[i]], NK_TEXT_RIGHT, 1);
						/*
						{
							if (doubleclick2 == i && (GetTimerM() - time) > 50)
							{
								strcpy(temp, UI_Sys.current_path);
								strcat(temp, "\\");
								strcat(temp, UI_Sys.files[UI_Sys.filesp[i]]);

								if ((f = fopen(temp, "rb")) == NULL)
								{
									fclose(f);
									return temp;
								}
							}
							else
								doubleclick2 = i;
						}
						*/
					}
					else
					{
						if (UI_Sys.filesp[i] == UI_Sys.foldersp[i])
						{
							if (nk_select_image_label(ctx, nk_image_id(NULL), UI_Sys.files[UI_Sys.foldersp[i]], NK_TEXT_RIGHT, 0))
							{
								select = i;
								doubleclick = i;
								time = GetTimerM();
							}
						}
						else
						{
							if (nk_select_label(ctx, UI_Sys.files[UI_Sys.filesp[i]], NK_TEXT_RIGHT, 0))
								select = i;
						}
					}
				}

				nk_group_end(ctx);
			}
			else
				nk_spacing(ctx, 1);
		}

		nk_layout_space_push(ctx, nk_rect(0.80, 0.92, 0.10, 0.08));
		if (nk_button_label(ctx, "Open"))
		{
			if (UI_Sys.filesp[select] == UI_Sys.foldersp[select])
			{
				strcpy(temp, UI_Sys.current_path);
				strcat(temp, "\\");
				strcat(temp, UI_Sys.files[UI_Sys.foldersp[select]]);

				if ((d = opendir(temp)) != NULL)
				{
					strcpy(backpath, UI_Sys.current_path);
					strcpy(UI_Sys.current_path, temp);
					backpath_available = 1;
					closedir(d);
					SetDirContent(UI_Sys.extension);
					doubleclick = -1;
					doubleclick2 = -1;
					select = -1;
					time = 0;
				}
			}
			else
			{
				strcpy(temp, UI_Sys.current_path);
				strcat(temp, "\\");
				strcat(temp, UI_Sys.files[UI_Sys.filesp[select]]);

				if ((f = fopen(temp, "rb")) != NULL)
				{
					fclose(f);
					nk_end(ctx);
					strcpy(filename, temp);
					return 1;
				}
			}
		}

		nk_layout_space_push(ctx, nk_rect(0.90, 0.92, 0.10, 0.08));
		if (nk_button_label(ctx, "Cancel"))
		{
			nk_end(ctx);
			return -1;
		}

	}

	nk_end(ctx);

	return 0;
}

void MenuBar()
{
	register int i, a, m;
	char mapname[2048], str[128], filename[2048];
	int id = 0, id2 = 0, check;
	static int state = 0, mggid;

	if (nkrendered==0)
	{
		if (nk_begin(ctx, "Menu", nk_rect(0, 0, st.screenx, 30), NK_WINDOW_NO_SCROLLBAR))
		{
			nk_menubar_begin(ctx);
			nk_layout_row_begin(ctx, NK_STATIC, 25, 5);

			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(120, 210)))
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				if (nk_menu_item_label(ctx, "New map", NK_TEXT_LEFT))
				{
					meng.scroll = 0;
					meng.tex_selection.data = -1;
					meng.command2 = 0;
					meng.scroll2 = 0;
					meng.mgg_sel = 0;

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

					st.num_lights = 0;

					for (i = 0; i<MAX_SECTORS; i++)
					{
						st.Current_Map.sector[i].id = -1;
						///st.Current_Map.sector[i].layers=1;
						st.Current_Map.sector[i].material = CONCRETE;
						st.Current_Map.sector[i].tag = 0;
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
					meng.num_mgg -= st.Current_Map.num_mgg;
					st.Current_Map.num_mgg = 0;

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

					memset(meng.z_buffer, -1, 2048 * 57 * sizeof(int16));
					memset(meng.z_slot, 0, 57 * sizeof(int16));
					meng.z_used = 0;

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
				}

				if (nk_menu_item_label(ctx, "Open map", NK_TEXT_LEFT))
				{
					//SetDirContent("mgm");

					

					state = 1;
				}

				nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT);
				if (nk_menu_item_label(ctx, "Save as...", NK_TEXT_LEFT))
				{
					SaveMap(StringFormat("%s/test.mgm", meng.prj_path));
				}

				nk_menu_item_label(ctx, "Compile map", NK_TEXT_LEFT);
				if (nk_menu_item_label(ctx, "Exit", NK_TEXT_LEFT)) st.quit = 1;
				nk_menu_end(ctx);
			}

			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "Edit", NK_TEXT_LEFT, nk_vec2(120, 200)))
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				nk_menu_item_label(ctx, "Cut", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "Copy", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "Paste", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "Preferences", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "Test map", NK_TEXT_LEFT);
				nk_menu_end(ctx);
			}

			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "View", NK_TEXT_LEFT, nk_vec2(120, 300)))
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				if (nk_menu_item_label(ctx, "Background 3", NK_TEXT_LEFT))
				{
					meng.viewmode = BACKGROUND3_MODE;
					st.viewmode = 16;
				}

				if (nk_menu_item_label(ctx, "Background 2", NK_TEXT_LEFT))
				{
					meng.viewmode = BACKGROUND2_MODE;
					st.viewmode = 8;
				}

				if (nk_menu_item_label(ctx, "Background 1", NK_TEXT_LEFT))
				{
					meng.viewmode = BACKGROUND1_MODE;
					st.viewmode = 4;
				}

				if (nk_menu_item_label(ctx, "Midground", NK_TEXT_LEFT))
				{
					meng.viewmode = MIDGROUND_MODE;
					st.viewmode = 2;
				}

				if (nk_menu_item_label(ctx, "Foreground", NK_TEXT_LEFT))
				{
					meng.viewmode = FOREGROUND_MODE;
					st.viewmode = 1;
				}

				if (nk_menu_item_label(ctx, "InGame View", NK_TEXT_LEFT))
				{
					meng.viewmode = INGAMEVIEW_MODE;
					st.viewmode = 31 + 32;
				}
				
				if (nk_menu_item_label(ctx, "No light view", NK_TEXT_LEFT))
				{
					meng.viewmode = LIGHTVIEW_MODE;
					st.viewmode = 31;
				}

				if (nk_menu_item_label(ctx, "All", NK_TEXT_LEFT))
				{
					meng.viewmode = ALLVIEW_MODE;
					st.viewmode = 31 + 64;
				}

				nk_menu_end(ctx);
			}

			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "Map", NK_TEXT_LEFT, nk_vec2(150, 250)))
			{
				meng.command = meng.pannel_choice = NONE_MODE;
				nk_layout_row_dynamic(ctx, 30, 1);
				if (nk_menu_item_label(ctx, "Load MGG", NK_TEXT_LEFT))
				{
					for (i = 0; i<MAX_MAP_MGG; i++)
					{
						if (i == MAX_MAP_MGG - 1 && mgg_map[i].type != NONE)
						{
							LogApp("Cannot load MGG, reached max number of map MGGs loaded");
							break;
						}

						if (mgg_map[i].type == NONE)
						{
							mggid = i;
							meng.command == MGG_SEL;
							OPENFILENAME ofn;
							char path[MAX_PATH];
							ZeroMemory(&path, sizeof(path));
							ZeroMemory(&ofn, sizeof(ofn));
							ofn.lStructSize = sizeof(ofn);
							ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
							ofn.lpstrFilter = "MGG files\0*.mgg\0";
							ofn.nMaxFile = MAX_PATH;
							ofn.lpstrFile = path;
							ofn.lpstrTitle = "Select the MGG file";
							//ofn.hInstance = OFN_EXPLORER;
							ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_EXPLORER;

							if (GetOpenFileName(&ofn))
							{
								strcpy(filename, path);
								state = 8;
							}
							break;
						}
					}
				}

				if (nk_menu_item_label(ctx, "Camera properties", NK_TEXT_LEFT))
					state = 9;

				if (nk_menu_item_label(ctx, "Map properties", NK_TEXT_LEFT))
					state = 10;

				nk_menu_end(ctx);
			}

			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "Help", NK_TEXT_LEFT, nk_vec2(120, 200)))
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				nk_menu_item_label(ctx, "Help", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "About", NK_TEXT_LEFT);
				nk_menu_end(ctx);
			}

			nk_layout_row_end(ctx);
			nk_menubar_end(ctx);
		}

		nk_end(ctx);
	}

	if (state == 8)
	{
		meng.command = meng.pannel_choice =  MGG_SEL;

		//check = FileBrowser(filename);

		//if (check == -1)
			//state = 0;

		//if (check == 1)
		//{
			if (LoadMGG(&mgg_map[mggid], filename))
			{
				PathRelativePathTo(st.Current_Map.MGG_FILES[st.Current_Map.num_mgg], meng.prj_path, FILE_ATTRIBUTE_DIRECTORY, filename, FILE_ATTRIBUTE_DIRECTORY);
				//strcpy(st.Current_Map.MGG_FILES[st.Current_Map.num_mgg], filename);
				strcpy(meng.mgg_list[st.Current_Map.num_mgg], mgg_map[mggid].name);
				meng.num_mgg++;
				st.Current_Map.num_mgg++;
				st.num_mgg++;

				meng.tex_MGGID = mggid;
				meng.tex_ID = 0;
				meng.tex_selection = mgg_map[mggid].frames[0];

				state = 0;
				meng.command = meng.pannel_choice =  NONE_MODE;
			}
		//}
	}

	if (state == 9)
	{
		meng.command = meng.pannel_choice = NONE_MODE;
		if (CameraProperties())
			state = 0;
	}
	
	if (state == 10)
	{
		meng.command = meng.pannel_choice = NONE_MODE;
		if (MapProperties())
			state = 0;
	}

	if (state == 1)
	{
		OPENFILENAME ofn;
		char path[MAX_PATH];
		ZeroMemory(&path, sizeof(path));
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
		ofn.lpstrFilter = "MGM files\0*.mgm\0";
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrFile = path;
		ofn.lpstrTitle = "Select the MGM file";
		//ofn.hInstance = OFN_EXPLORER;
		ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_EXPLORER;

		if (GetOpenFileName(&ofn))
		{
			if (LoadMap(path))
			{
				memset(meng.mgg_list, 0, 32 * 256);
				meng.num_mgg = 0;
				LogApp("Map %s loaded", st.Current_Map.name);

				for (a = 0; a<st.Current_Map.num_mgg; a++)
				{
					DrawUI(8192, 4608, 16384, 8192, 0, 0, 0, 0, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, mgg_sys[0].frames[4], 255, 0);
					sprintf(str, "Loading %d%", (a / st.Current_Map.num_mgg) * 100);
					DrawString2UI(str, 8192, 4608, 1, 1, 0, 255, 255, 255, 255, ARIAL, FONT_SIZE * 2, FONT_SIZE * 2, 0);
					char path2[MAX_PATH];
					//PathRelativePathTo(path2, meng.prj_path, FILE_ATTRIBUTE_DIRECTORY, st.Current_Map.MGG_FILES[a], FILE_ATTRIBUTE_DIRECTORY);
					strcpy(path2, meng.prj_path);
					strcat(path2, "\\");
					strcat(path2, st.Current_Map.MGG_FILES[a]);

					if (CheckMGGFile(path2))
					{
						LoadMGG(&mgg_map[id], path2);
						strcpy(meng.mgg_list[a], mgg_map[id].name);
						meng.num_mgg++;
						id++;
					}
					else
					{
						FreeMap();

						LogApp("Error while loading map's MGG: %s", st.Current_Map.MGG_FILES[a]);
						state = 0;
						break;
					}

				}

				st.Camera.position.x = 0;
				st.Camera.position.y = 0;
				meng.scroll = 0;
				meng.tex_selection.data = -1;
				meng.command2 = 0;
				meng.scroll2 = 0;
				meng.mgg_sel = 0;
				meng.pannel_choice = 2;
				meng.command = 2;
				meng.menu_sel = 0;
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
				st.gt = INGAME;
				st.mouse1 = 0;
				//free(path2);
				free(meng.path);
				meng.path = (char*)malloc(2);
				strcpy(meng.path, ".");

				meng.viewmode = 7;

				memset(meng.z_buffer, -1, 2048 * 57 * sizeof(int16));
				memset(meng.z_slot, 0, 57 * sizeof(int16));
				meng.z_used = 0;

				for (m = 0; m<st.Current_Map.num_obj; m++)
				{
					meng.z_buffer[st.Current_Map.obj[m].position.z][meng.z_slot[st.Current_Map.obj[m].position.z]] = m;
					meng.z_slot[st.Current_Map.obj[m].position.z]++;
					if (st.Current_Map.obj[m].position.z>meng.z_used)
						meng.z_used = st.Current_Map.obj[m].position.z;
				}

				for (m = 0; m<st.Current_Map.num_sprites; m++)
				{
					meng.z_buffer[st.Current_Map.sprites[m].position.z][meng.z_slot[st.Current_Map.sprites[m].position.z]] = m + 2000;
					meng.z_slot[st.Current_Map.sprites[m].position.z]++;
					if (st.Current_Map.sprites[m].position.z>meng.z_used)
						meng.z_used = st.Current_Map.sprites[m].position.z;
				}

				for (m = 0; m<st.Current_Map.num_sector; m++)
				{
					meng.z_buffer[24][meng.z_slot[24]] = m + 10000;
					meng.z_slot[24]++;
					if (24>meng.z_used)
						meng.z_used = 24;
				}

				for (m = 1; m <= st.num_lights; m++)
				{
					int8 lz = st.game_lightmaps[m].falloff[4] + 24;

					meng.z_buffer[lz][meng.z_slot[lz]] = m + 12000;
					meng.z_slot[lz]++;

					if (lz>meng.z_used)
						meng.z_used = lz;
				}

				SetCurrentDirectory(meng.prj_path);

				//break;
			}
		}

		state = 0;
	}
}

void SpriteListSelection()
{
	register int i, j, k, l = 0, m;
	TEX_DATA data;
	int temp;
	float px, py, sx, sy;
	struct nk_image texid;
	char labels[6][32], label_active[6] = { 0, 0, 0, 0, 0, 0 };

	if (nk_begin(ctx, "Sprite Selection", nk_rect(st.screenx / 2 - 300, st.screeny / 2 - 300, 600, 600),
		NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_SCALABLE))
	{
		nk_layout_row_dynamic(ctx, 515, 1);

		if (nk_group_begin(ctx, "SPRSEL", NK_WINDOW_BORDER))
		{
			/*
			ctx->style.selectable.text_normal = nk_rgb(255, 128, 32);
			ctx->style.selectable.text_hover = nk_rgb(255, 128, 8);
			ctx->style.selectable.text_pressed = nk_rgb(255, 128, 8);
			ctx->style.selectable.text_normal_active = nk_rgb(255, 32, 32);
			ctx->style.selectable.text_hover_active = nk_rgb(255, 32, 32);
			ctx->style.selectable.text_pressed_active = nk_rgb(255, 32, 32);
			*/

			ctx->style.selectable.hover = nk_style_item_color(nk_rgb(206, 206, 206));
			ctx->style.selectable.normal_active = nk_style_item_color(nk_rgb(255, 128, 32));
			ctx->style.selectable.hover_active = nk_style_item_color(nk_rgb(255, 128, 32));

			nk_layout_row_dynamic(ctx, 100, 6);
			//l = 0;
			for (i = 0; i < st.num_sprites; i++)
			{
				j = st.sprite_id_list[i];
				if (st.Game_Sprites[j].num_start_frames > 0)
				{
					for (k = 0; k < st.Game_Sprites[j].num_start_frames; k++)
					{
						data = mgg_game[st.Game_Sprites[j].MGG_ID].frames[st.Game_Sprites[j].frame[k]];

						if (meng.sprite_selection == j && meng.sprite_frame_selection == st.Game_Sprites[j].frame[k])
							temp = 1;
						else
							temp = 0;

						if (data.vb_id != -1)
						{
							px = ((float)data.posx / 32768) * data.w;
							ceil(px);
							//px += data.x_offset;
							py = ((float)data.posy / 32768) * data.h;
							ceil(py);
							//py += data.y_offset;
							sx = ((float)data.sizex / 32768) * data.w;
							ceil(sx);
							sy = ((float)data.sizey / 32768) * data.h;
							ceil(sy);
							texid = nk_subimage_id(data.data, data.w, data.h, nk_recta(nk_vec2(px, py), nk_vec2(sx, sy)));
						}
						else
							texid = nk_image_id(data.data);

						if (nk_selectable_image_label(ctx, texid," ", NK_TEXT_ALIGN_CENTERED, &temp))
						{
							meng.sprite_selection = j;
							meng.sprite_frame_selection = st.Game_Sprites[j].frame[k];

							meng.spr.health = st.Game_Sprites[j].health;
							meng.spr.body = st.Game_Sprites[j].body;
							meng.spr.flags = st.Game_Sprites[j].flags;

							meng.spr.body.size = st.Game_Sprites[j].body.size;
						}

						strcpy(labels[l], st.Game_Sprites[j].name);

						//nk_label(ctx, labels[l], NK_TEXT_ALIGN_CENTERED);

						label_active[l] = temp;

						l++;

						if (l == 6)
						{
							nk_layout_row_dynamic(ctx, 15, 6);
							for (m = 0; m < l; m++)
							{
								if (label_active[m])
									ctx->style.text.color = nk_rgb(255, 255, 255);
								else
									ctx->style.text.color = nk_rgb(128, 128, 128);

								nk_label(ctx, labels[m], NK_TEXT_ALIGN_CENTERED);
							}

							memset(label_active, 6, 0);

							nk_layout_row_dynamic(ctx, 100, 6);
							l = 0;
						}
					}
				}
			}

			if (l != 0)
			{
				nk_layout_row_dynamic(ctx, 15, 6);
				for (m = 0; m < l; m++)
					nk_label(ctx, labels[m], NK_TEXT_ALIGN_CENTERED);

				l = 0;
			}

			nk_group_end(ctx);
		}

		SetThemeBack(ctx,meng.theme);

		nk_layout_row_dynamic(ctx, 30, 3);
		nk_spacing(ctx, 2);

		if (nk_button_label(ctx, "Select"))
			meng.command = ADD_SPRITE;
	}

	st.mouse1 = 0;

	nk_end(ctx);
}

void TextureListSelection()
{
	register int i, j, k, l = 0, m;
	TEX_DATA data;
	int temp;
	float px, py, sx, sy;
	struct nk_image texid;

	if (st.Current_Map.num_mgg == 0)
	{
		if (nk_begin(ctx, "Error", nk_rect(st.screenx / 2 - 128, st.screeny / 2 - 43, 256, 86), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR))
		{
			nk_layout_row_dynamic(ctx, 30, 1);
			nk_label(ctx, "Error: No MGGs loaded", NK_TEXT_ALIGN_CENTERED);
			nk_layout_row_dynamic(ctx, 30, 3);
			nk_spacing(ctx, 1);
			if (nk_button_label(ctx, "Ok"))
				meng.command = ADD_OBJ;
		}

		st.mouse1 = 0;
		nk_end(ctx);
	}
	else
	{
		if (nk_begin(ctx, "Texture Selection", nk_rect(st.screenx / 2 - 300, st.screeny / 2 - 300, 600, 600),
			NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_SCALABLE))
		{
			nk_layout_row_dynamic(ctx, 515, 1);

			if (nk_group_begin(ctx, "TEXSEL", NK_WINDOW_BORDER))
			{
				ctx->style.selectable.hover = nk_style_item_color(nk_rgb(206, 206, 206));
				ctx->style.selectable.normal_active = nk_style_item_color(nk_rgb(255, 128, 32));
				ctx->style.selectable.hover_active = nk_style_item_color(nk_rgb(255, 128, 32));

				nk_layout_row_dynamic(ctx, 100, 6);
				//l = 0;
				for (i = 0; i < st.Current_Map.num_mgg; i++)
				{
					for (j = 0; j < mgg_map[i].num_frames; j++)
					{
						data = mgg_map[i].frames[j];

						if (meng.tex_ID == j && meng.tex_MGGID == i)
							temp = 1;
						else
							temp = 0;

						if (data.vb_id != -1)
						{
							px = ((float)data.posx / 32768.0f) * data.w;
							ceil(px);
							//px += data.x_offset;
							py = ((float)data.posy / 32768.0f) * data.h;
							ceil(py);
							//py += data.y_offset;
							sx = ((float)data.sizex / 32768.0f) * data.w;
							ceil(sx);
							sy = ((float)data.sizey / 32768.0f) * data.h;
							ceil(sy);
							texid = nk_subimage_id(data.data, data.w, data.h, nk_rect(px, py, sx, sy));
						}
						else
							texid = nk_image_id(data.data);

						if (nk_selectable_image_label(ctx, texid, " ", NK_TEXT_ALIGN_CENTERED, &temp))
						{
							meng.tex_selection = mgg_map[i].frames[j];
							meng.tex_ID = j;
							meng.tex_MGGID = i;

							if (mgg_map[i].frames[j].vb_id != -1)
							{
								meng.pre_size.x = mgg_map[i].frames[j].sizex;
								meng.pre_size.y = mgg_map[i].frames[j].sizey;
							}
							else
							{
								meng.pre_size.x = (float)(mgg_map[i].frames[j].w * 16384) / st.screenx;
								meng.pre_size.y = (float)(mgg_map[i].frames[j].h * st.gamey) / st.screeny;
							}
						}
					}
				}

				nk_group_end(ctx);
			}

			SetThemeBack(ctx,meng.theme);

			nk_layout_row_dynamic(ctx, 30, 3);
			nk_spacing(ctx, 2);

			if (nk_button_label(ctx, "Select"))
				meng.command = ADD_OBJ;
		}

		st.mouse1 = 0;

		nk_end(ctx);
	}
}

void TagBox(int16 game_sprite, int32 map_sprite)
{
	char str[1024];
	register int i, j;
	static int state = 0, len, lenstr;
	int16 l = 0, m = -1, n = -1;

	if (nk_begin(ctx, "Tags", nk_rect(st.screenx / 2 - 256, st.screeny / 2 - 128, 512, 300), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
	{
		nk_layout_row_dynamic(ctx, 200,1);

		if (nk_group_begin(ctx, "tagg", NK_WINDOW_BORDER))
		{
			for (i = 0; i < st.Game_Sprites[game_sprite].num_tags; i++)
			{
				if (strcmp(st.Game_Sprites[game_sprite].tag_names[i], "MUSFX") == NULL)
				{
					//nk_layout_row_dynamic(ctx, 30, 1);
					nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 3);
					nk_layout_row_push(ctx, 0.85f);

					if (nk_combo_begin_label(ctx, meng.musiclist[st.Current_Map.sprites[map_sprite].tags[i]], nk_vec2(nk_widget_width(ctx), 20 + 20 * st.num_musics)))
					{
						nk_layout_row_dynamic(ctx, 20, 1);
						for (j = 0; j < st.num_musics; j++)
						if (nk_combo_item_label(ctx, meng.musiclist[j], NK_TEXT_ALIGN_LEFT))
							st.Current_Map.sprites[map_sprite].tags[i] = j;

						nk_combo_end(ctx);
					}

					nk_layout_row_push(ctx, 0.075f);
					if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT))
					{
						StopMusic();
						StopAllSounds();
						PlayMusic(st.Current_Map.sprites[map_sprite].tags[i], 0);
					}

					nk_layout_row_push(ctx, 0.075f);
					if (nk_button_symbol(ctx, NK_SYMBOL_RECT_SOLID))
					{
						StopMusic();
						StopAllSounds();
					}

					nk_layout_row_end(ctx);

					continue;
				}

				if (strcmp(st.Game_Sprites[game_sprite].tag_names[i], "SNDFX") == NULL)
				{
					//nk_layout_row_dynamic(ctx, 30, 1);
					nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 3);
					nk_layout_row_push(ctx, 0.85f);

					if (nk_combo_begin_label(ctx, meng.soundlist[st.Current_Map.sprites[map_sprite].tags[i]], nk_vec2(nk_widget_width(ctx), 20 + 20 * st.num_sounds)))
					{
						nk_layout_row_dynamic(ctx, 20, 1);
						for (j = 0; j < st.num_sounds; j++)
							if (nk_combo_item_label(ctx, meng.soundlist[j], NK_TEXT_ALIGN_LEFT))
								st.Current_Map.sprites[map_sprite].tags[i] = j;

						nk_combo_end(ctx);
					}

					nk_layout_row_push(ctx, 0.075f);
					if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT))
					{
						StopMusic();
						StopAllSounds();
						PlaySound(st.Current_Map.sprites[map_sprite].tags[i],0);
					}

					nk_layout_row_push(ctx, 0.075f);
					if (nk_button_symbol(ctx, NK_SYMBOL_RECT_SOLID))
					{
						StopMusic();
						StopAllSounds();
					}

					nk_layout_row_end(ctx);

					continue;
				}

				len = strlen(st.Game_Sprites[game_sprite].tag_names[i]);

				if (st.Game_Sprites[game_sprite].tag_names[i][len - 1] == 'S' && st.Game_Sprites[game_sprite].tag_names[i][len - 2] == '_')
				{
					nk_layout_row_dynamic(ctx, 30, 2);
					nk_label(ctx, st.Game_Sprites[game_sprite].tag_names[i], NK_TEXT_ALIGN_LEFT);
					nk_edit_string(ctx, NK_EDIT_SIMPLE, st.Current_Map.sprites[map_sprite].tags_str[i], &lenstr, 256, nk_filter_default);
					continue;
				}
				
				if (!strcmp(st.Game_Sprites[game_sprite].tag_names[i], "INPUT") || !strcmp(st.Game_Sprites[game_sprite].tag_names[i], "OUTPUT"))
				{
					nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 2);

					nk_layout_row_push(ctx, 0.9);
					st.Current_Map.sprites[map_sprite].tags[i] = nk_propertyi(ctx, st.Game_Sprites[game_sprite].tag_names[i], -32768,
						st.Current_Map.sprites[map_sprite].tags[i], 32768, 1, 5);
									

					nk_layout_row_push(ctx, 0.1);

					if (meng.picking_tag == 1)
						ctx->style.button.normal = ctx->style.button.hover;

					if (nk_button_icon_set(PICKER_ICON))
					{
						if (meng.picking_tag == 0)
						{
							meng.command = PICK_TAG;
							meng.sub_com = i;

							st.cursor_type = CURSOR_TARGET;

							if(!strcmp(st.Game_Sprites[game_sprite].tag_names[i], "INPUT"))
								meng.picking_tag = 2;
							else
								meng.picking_tag = 1;

						}
						else
						{
							meng.picking_tag = 0;
							st.cursor_type = CURSOR_DEFAULT;
						}
					}

					SetThemeBack(ctx);

					nk_layout_row_end(ctx);

					continue;
				}
				
				nk_layout_row_dynamic(ctx, 30, 2);
				st.Current_Map.sprites[map_sprite].tags[i] = nk_propertyi(ctx, st.Game_Sprites[game_sprite].tag_names[i], -32768,
					st.Current_Map.sprites[map_sprite].tags[i], 32768, 1, 5);

			}

			nk_group_end(ctx);
		}
	}

	nk_layout_row_dynamic(ctx, 30, 1);

	if (nk_button_label(ctx, "Ok"))
		meng.command = meng.pannel_choice;

	nk_end(ctx);
}

void CoordBar()
{
	int32 x = st.mouse.x, y = st.mouse.y;

	STW(&x, &y);

	if (nk_begin(ctx, "Coord bar", nk_rect(st.screenx - 200, st.screeny - 160, 200, 160),NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER))
	{
		nk_layout_row_dynamic(ctx, 20, 1);
		nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "M X: %d M Y: %d", st.mouse.x, st.mouse.y);
		nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "MG X: %d MG Y: %d", x, y);
		nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "X: %d Y: %d", st.Camera.position.x, st.Camera.position.y);
		//nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "Zoom: %f", st.Camera.dimension.x);
		st.Camera.dimension.y = st.Camera.dimension.x = nk_propertyf(ctx, "Zoom:", 0.2f, st.Camera.dimension.x, 6.0f, 0.1f, 0.1f);
		meng.gridsize = nk_propertyi(ctx, "Grid size: ", 0, meng.gridsize == 1 ? 0 : POFCeil(meng.gridsize), 256, meng.gridsize == 0 ? 2 : (meng.gridsize / 2), 0);
		nk_layout_row_dynamic(ctx, 15, 1);
	}

	nk_end(ctx);
}

void LayerBar()
{
	if (nk_begin(ctx, "Layer bar", nk_rect(st.screenx - 200, 30, 200, st.screeny - 30 - 130), NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		//nk_layout_row_dynamic(ctx, 320, 1);

		int16 i, j;

		nk_layout_row_dynamic(ctx, 20, 2);
		if (nk_button_icon_set(DOWN_ICON))
		{
			switch (meng.command2)
			{
			case EDIT_OBJ:
				st.Current_Map.obj[meng.obj_edit_selection].position.z++;
				if (st.Current_Map.obj[meng.obj_edit_selection].position.z > 56)
					st.Current_Map.obj[meng.obj_edit_selection].position.z = 56;
				break;

			case EDIT_SPRITE:
				st.Current_Map.sprites[meng.sprite_edit_selection].position.z++;
				if (st.Current_Map.sprites[meng.sprite_edit_selection].position.z > 56)
					st.Current_Map.sprites[meng.sprite_edit_selection].position.z = 56;
				break;

			case NEDIT_LIGHT:
				st.game_lightmaps[meng.light_edit_selection].falloff[4] += 1.0f;
				if (st.game_lightmaps[meng.light_edit_selection].falloff[4] + 24 > 31.0f)
					st.game_lightmaps[meng.light_edit_selection].falloff[4] = 31.0f - 24;
				break;
			}
		}

		if (nk_button_icon_set(UP_ICON))
		{
			switch (meng.command2)
			{
			case EDIT_OBJ:
				st.Current_Map.obj[meng.obj_edit_selection].position.z--;
				if (st.Current_Map.obj[meng.obj_edit_selection].position.z < 16)
					st.Current_Map.obj[meng.obj_edit_selection].position.z = 16;
				break;

			case EDIT_SPRITE:
				st.Current_Map.sprites[meng.sprite_edit_selection].position.z--;
				if (st.Current_Map.sprites[meng.sprite_edit_selection].position.z < 16)
					st.Current_Map.sprites[meng.sprite_edit_selection].position.z = 16;
				break;

			case NEDIT_LIGHT:
				st.game_lightmaps[meng.light_edit_selection].falloff[4] -= 1.0f;
				if (st.game_lightmaps[meng.light_edit_selection].falloff[4] + 24 < 24.0f)
					st.game_lightmaps[meng.light_edit_selection].falloff[4] = 0;
				break;
			}
		}

		FixLayerBar();

		nk_layout_row_dynamic(ctx, 20, 1);

		if (meng.command2 != EDIT_OBJ && meng.command2 != EDIT_SPRITE && meng.command2 != EDIT_SECTOR && meng.command2 != NEDIT_LIGHT)
			ctx->style.button.active = ctx->style.button.hover;

		if (nk_button_icon_set(BIN_ICON) && (meng.command2 == EDIT_OBJ || meng.command2 == EDIT_SPRITE || meng.command2 == EDIT_SECTOR || meng.command2 == NEDIT_LIGHT))
		{
			switch (meng.command2)
			{
				case EDIT_OBJ:
					i = st.Current_Map.obj[meng.obj_edit_selection].position.z;

					for (j = 0; j < meng.z_slot[i]; j++)
					{
						if (meng.z_buffer[i][j] == meng.obj_edit_selection)
						{
							meng.z_buffer[i][j] = -1;
							DeleteScenario(meng.obj_edit_selection);
							meng.obj_edit_selection = -1;
							meng.command2 = 0;
							//st.Current_Map.num_obj--;
							break;
						}
					}

					break;

				case EDIT_SPRITE:
					i = st.Current_Map.sprites[meng.sprite_edit_selection].position.z;

					for (j = 0; j < meng.z_slot[i]; j++)
					{
						if (meng.z_buffer[i][j] == meng.sprite_edit_selection + 2000)
						{
							meng.z_buffer[i][j] = -1;
							DeleteSprite(meng.sprite_edit_selection);
							meng.sprite_edit_selection = -1;
							meng.command2 = 0;
							break;
						}
					}

					break;

				case EDIT_SECTOR:
					i = 24;

					for (j = 0; j < meng.z_slot[i]; j++)
					{
						if (meng.z_buffer[i][j] == meng.sector_edit_selection + 10000)
						{
							meng.z_buffer[i][j] = -1;
							DeleteSector(meng.sector_edit_selection);
							meng.sector_edit_selection = -1;
							meng.command2 = 0;
							break;
						}
					}

					break;

				case NEDIT_LIGHT:
					i = st.game_lightmaps[meng.light_edit_selection].falloff[4] + 24;

					for (j = 0; j < meng.z_slot[i]; j++)
					{
						if (meng.z_buffer[i][j] == meng.light_edit_selection + 12000)
						{
							meng.z_buffer[i][j] = -1;
							DeleteLight(meng.light_edit_selection);
							meng.light_edit_selection = -1;
							meng.command2 = 0;
							break;
						}
					}

					break;
			}

			RedoZBuffers();
		}

		SetThemeBack(ctx);

		if (nk_tree_push(ctx, NK_TREE_TAB, "Foreground", NK_MINIMIZED))
		{
			//nk_layout_row_dynamic(ctx, 15, 1);

			for (i = 16; i < 24; i++)
			{
				if (meng.z_slot[i] > 0)
				{
					if (nk_tree_push_id(ctx, NK_TREE_NODE, StringFormat("Z %d", i), NK_MINIMIZED, i))
					{
						//nk_layout_row_dynamic(ctx, 15, 1);

						for (j = 0; j < meng.z_slot[i]; j++)
						{
							if (meng.z_buffer[i][j] < 2000)
							{
								if (nk_select_label(ctx, StringFormat("Scene %d", meng.z_buffer[i][j]), NK_TEXT_ALIGN_LEFT, meng.layers[i][j] == 1))
								{
									if (st.keys[LCTRL_KEY].state || st.keys[RCTRL_KEY].state)
										meng.layers[i][j] = 1;
									else
									{
										memset(meng.layers, 0, 57 * 2048 * 2);
										meng.layers[i][j] = 1;
										meng.obj_edit_selection = meng.z_buffer[i][j];
										meng.command2 = EDIT_OBJ;
									}
								}

							}

							if (meng.z_buffer[i][j] >= 2000 && meng.z_buffer[i][j] < 10000)
							{
								if (nk_select_label(ctx, StringFormat("%s %d", st.Game_Sprites[st.Current_Map.sprites[meng.z_buffer[i][j] - 2000].GameID].name,
									meng.z_buffer[i][j] - 2000), NK_TEXT_ALIGN_LEFT, meng.layers[i][j] == 1))
								{
									if (st.keys[LCTRL_KEY].state || st.keys[RCTRL_KEY].state)
										meng.layers[i][j] = 1;
									else
									{
										memset(meng.layers, 0, 57 * 2048 * 2);
										meng.layers[i][j] = 1;
										meng.sprite_edit_selection = meng.z_buffer[i][j] - 2000;
										meng.command2 = EDIT_SPRITE;
									}
								}

							}
						}

						nk_tree_pop(ctx);
					}
				}
			}

			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Midground", NK_MINIMIZED))
		{
			//nk_layout_row_dynamic(ctx, 15, 1);

			for (i = 24; i < 32; i++)
			{
				if (meng.z_slot[i] > 0)
				{
					if (nk_tree_push_id(ctx, NK_TREE_NODE, StringFormat("Z %d", i), NK_MINIMIZED, i))
					{
						//nk_layout_row_dynamic(ctx, 15, 1);

						for (j = 0; j < meng.z_slot[i]; j++)
						{
							if (meng.z_buffer[i][j] < 2000)
							{
								if (nk_select_label(ctx, StringFormat("Scene %d", meng.z_buffer[i][j]), NK_TEXT_ALIGN_LEFT, meng.layers[i][j] == 1))
								{
									if (st.keys[LCTRL_KEY].state || st.keys[RCTRL_KEY].state)
										meng.layers[i][j] = 1;
									else
									{
										memset(meng.layers, 0, 57 * 2048 * 2);
										meng.layers[i][j] = 1;
										meng.obj_edit_selection = meng.z_buffer[i][j];
										meng.command2 = EDIT_OBJ;
									}
								}

							}

							if (meng.z_buffer[i][j] >= 2000 && meng.z_buffer[i][j] < 10000)
							{
								if (nk_select_label(ctx, StringFormat("%s %d", st.Game_Sprites[st.Current_Map.sprites[meng.z_buffer[i][j] - 2000].GameID].name,
									meng.z_buffer[i][j] - 2000), NK_TEXT_ALIGN_LEFT, meng.layers[i][j] == 1))
								{
									if (st.keys[LCTRL_KEY].state || st.keys[RCTRL_KEY].state)
										meng.layers[i][j] = 1;
									else
									{
										memset(meng.layers, 0, 57 * 2048 * 2);
										meng.layers[i][j] = 1;
										meng.sprite_edit_selection = meng.z_buffer[i][j] - 2000;
										meng.command2 = EDIT_SPRITE;
									}
								}

							}

							if (meng.z_buffer[i][j] >= 10000 && meng.z_buffer[i][j] < 12000)
							{
								if (nk_select_label(ctx, StringFormat("Sector %d", meng.z_buffer[i][j] - 10000), NK_TEXT_ALIGN_LEFT, meng.layers[i][j] == 1))
								{
									if (st.keys[LCTRL_KEY].state || st.keys[RCTRL_KEY].state)
										meng.layers[i][j] = 1;
									else
									{
										memset(meng.layers, 0, 57 * 2048 * 2);
										meng.layers[i][j] = 1;
										meng.sector_edit_selection = meng.z_buffer[i][j] - 10000;
										meng.command2 = EDIT_SECTOR;
									}
								}

							}

							if (meng.z_buffer[i][j] >= 12000)
							{
								if (nk_select_label(ctx, StringFormat("Light %d", meng.z_buffer[i][j] - 12000), NK_TEXT_ALIGN_LEFT, meng.layers[i][j] == 1))
								{
									if (st.keys[LCTRL_KEY].state || st.keys[RCTRL_KEY].state)
										meng.layers[i][j] = 1;
									else
									{
										memset(meng.layers, 0, 57 * 2048 * 2);
										meng.layers[i][j] = 1;
										meng.light_edit_selection = meng.z_buffer[i][j] - 12000;
										meng.command2 = NEDIT_LIGHT;
									}
								}
								
							}

						}

						nk_tree_pop(ctx);
					}
				}
			}

			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Background1", NK_MINIMIZED))
		{
			//nk_layout_row_dynamic(ctx, 15, 1);

			for (i = 32; i < 40; i++)
			{
				if (meng.z_slot[i] > 0)
				{
					if (nk_tree_push_id(ctx, NK_TREE_NODE, StringFormat("Z %d", i), NK_MINIMIZED, i))
					{
						//nk_layout_row_dynamic(ctx, 15, 1);

						for (j = 0; j < meng.z_slot[i]; j++)
						{
							if (meng.z_buffer[i][j] < 2000)
							{
								if (nk_select_label(ctx, StringFormat("Scene %d", meng.z_buffer[i][j]), NK_TEXT_ALIGN_LEFT, meng.layers[i][j] == 1))
								{
									if (st.keys[LCTRL_KEY].state || st.keys[RCTRL_KEY].state)
										meng.layers[i][j] = 1;
									else
									{
										memset(meng.layers, 0, 57 * 2048 * 2);
										meng.layers[i][j] = 1;
										meng.obj_edit_selection = meng.z_buffer[i][j];
										meng.command2 = EDIT_OBJ;
									}
								}

							}

							if (meng.z_buffer[i][j] >= 2000 && meng.z_buffer[i][j] < 10000)
							{
								if (nk_select_label(ctx, StringFormat("%s %d", st.Game_Sprites[st.Current_Map.sprites[meng.z_buffer[i][j] - 2000].GameID].name,
									meng.z_buffer[i][j] - 2000), NK_TEXT_ALIGN_LEFT, meng.layers[i][j] == 1))
								{
									if (st.keys[LCTRL_KEY].state || st.keys[RCTRL_KEY].state)
										meng.layers[i][j] = 1;
									else
									{
										memset(meng.layers, 0, 57 * 2048 * 2);
										meng.layers[i][j] = 1;
										meng.sprite_edit_selection = meng.z_buffer[i][j] - 2000;
										meng.command2 = EDIT_SPRITE;
									}
								}

							}
						}

						nk_tree_pop(ctx);
					}
				}
			}

			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Background2", NK_MINIMIZED))
		{
			//nk_layout_row_dynamic(ctx, 15, 1);

			for (i = 40; i < 48; i++)
			{
				if (meng.z_slot[i] > 0)
				{
					if (nk_tree_push_id(ctx, NK_TREE_NODE, StringFormat("Z %d", i), NK_MINIMIZED, i))
					{
						//nk_layout_row_dynamic(ctx, 15, 1);

						for (j = 0; j < meng.z_slot[i]; j++)
						{
							if (meng.z_buffer[i][j] < 2000)
							{
								if (nk_select_label(ctx, StringFormat("Scene %d", meng.z_buffer[i][j]), NK_TEXT_ALIGN_LEFT, meng.layers[i][j] == 1))
								{
									if (st.keys[LCTRL_KEY].state || st.keys[RCTRL_KEY].state)
										meng.layers[i][j] = 1;
									else
									{
										memset(meng.layers, 0, 57 * 2048 * 2);
										meng.layers[i][j] = 1;
										meng.obj_edit_selection = meng.z_buffer[i][j];
										meng.command2 = EDIT_OBJ;
									}
								}

							}

							if (meng.z_buffer[i][j] >= 2000 && meng.z_buffer[i][j] < 10000)
							{
								if (nk_select_label(ctx, StringFormat("%s %d", st.Game_Sprites[st.Current_Map.sprites[meng.z_buffer[i][j] - 2000].GameID].name,
									meng.z_buffer[i][j] - 2000), NK_TEXT_ALIGN_LEFT, meng.layers[i][j] == 1))
								{
									if (st.keys[LCTRL_KEY].state || st.keys[RCTRL_KEY].state)
										meng.layers[i][j] = 1;
									else
									{
										memset(meng.layers, 0, 57 * 2048 * 2);
										meng.layers[i][j] = 1;
										meng.sprite_edit_selection = meng.z_buffer[i][j] - 2000;
										meng.command2 = EDIT_SPRITE;
									}
								}

							}
						}

						nk_tree_pop(ctx);
					}
				}
			}

			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Background3", NK_MINIMIZED))
		{
			//nk_layout_row_dynamic(ctx, 15, 1);

			for (i = 48; i < 56; i++)
			{
				if (meng.z_slot[i] > 0)
				{
					if (nk_tree_push_id(ctx, NK_TREE_NODE, StringFormat("Z %d", i), NK_MINIMIZED, i))
					{
						//nk_layout_row_dynamic(ctx, 15, 1);

						for (j = 0; j < meng.z_slot[i]; j++)
						{
							if (meng.z_buffer[i][j] < 2000)
							{
								if (nk_select_label(ctx, StringFormat("Scene %d", meng.z_buffer[i][j]), NK_TEXT_ALIGN_LEFT, meng.layers[i][j] == 1))
								{
									if (st.keys[LCTRL_KEY].state || st.keys[RCTRL_KEY].state)
										meng.layers[i][j] = 1;
									else
									{
										memset(meng.layers, 0, 57 * 2048 * 2);
										meng.layers[i][j] = 1;
										meng.obj_edit_selection = meng.z_buffer[i][j];
										meng.command2 = EDIT_OBJ;
									}
								}

							}

							if (meng.z_buffer[i][j] >= 2000 && meng.z_buffer[i][j] < 10000)
							{
								if (nk_select_label(ctx, StringFormat("%s %d", st.Game_Sprites[st.Current_Map.sprites[meng.z_buffer[i][j] - 2000].GameID].name,
									meng.z_buffer[i][j] - 2000), NK_TEXT_ALIGN_LEFT, meng.layers[i][j] == 1))
								{
									if (st.keys[LCTRL_KEY].state || st.keys[RCTRL_KEY].state)
										meng.layers[i][j] = 1;
									else
									{
										memset(meng.layers, 0, 57 * 2048 * 2);
										meng.layers[i][j] = 1;
										meng.sprite_edit_selection = meng.z_buffer[i][j] - 2000;
										meng.command2 = EDIT_SPRITE;
									}
								}

							}
						}

						nk_tree_pop(ctx);
					}
				}
			}

			nk_tree_pop(ctx);
		}
	}

	nk_end(ctx);
}

void PhysicsBox(int16 sprite)
{
	if (nk_begin(ctx, "Physics", nk_rect(st.screenx / 2 - 200, st.screeny / 2 - 256, 200, 220),NK_WINDOW_TITLE | NK_WINDOW_MOVABLE | NK_WINDOW_BORDER))
	{
		nk_layout_row_dynamic(ctx, 30, 1);

		st.Current_Map.sprites[sprite].body.physics_on = nk_check_label(ctx, "Physics", st.Current_Map.sprites[sprite].body.physics_on);

		if (st.Current_Map.sprites[sprite].body.physics_on)
		{
			st.Current_Map.sprites[sprite].body.flamable = nk_check_label(ctx, "Flammable", st.Current_Map.sprites[sprite].body.flamable);
			st.Current_Map.sprites[sprite].body.explosive = nk_check_label(ctx, "Explosive", st.Current_Map.sprites[sprite].body.explosive);
			st.Current_Map.sprites[sprite].body.mass = nk_propertyi(ctx, "Mass:", 0, st.Current_Map.sprites[sprite].body.mass, 65536, 32, 8);
		}
		else
		{
			nk_spacing(ctx, 1);
			nk_spacing(ctx, 1);
			nk_spacing(ctx, 1);
		}

		if (nk_button_label(ctx, "Ok"))
			meng.command = meng.pannel_choice;
	}

	nk_end(ctx);
}

void NewLeftPannel()
{
	register int i, j, k;
	static int sl, pannel_state = 0;
	static struct nk_color editcolor = { 255, 255, 255, 255 };
	static char strbuf[32];
	static struct nk_rect bounds;
	TEX_DATA data;
	int temp, px, py, sx, sy;
	struct nk_image texid;

	if (nkrendered==0)
	{
		if (nk_begin(ctx, "Left Pannel", nk_rect(0, 30, st.screenx * 0.15f, st.screeny), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER))
		{
			nk_layout_row_dynamic(ctx, 30, 2);

			if (meng.pannel_choice == SELECT_EDIT)
				ctx->style.button.normal = ctx->style.button.hover;

			if (nk_button_label(ctx, "Select"))
			{
				meng.command = SELECT_EDIT;
				meng.pannel_choice = SELECT_EDIT;
			}

			SetThemeBack(ctx,meng.theme);

			if (meng.pannel_choice == DRAW_SECTOR)
				ctx->style.button.normal = ctx->style.button.hover;

			if (nk_button_label(ctx, "Add sector"))
			{
				meng.command = DRAW_SECTOR;
				meng.pannel_choice = DRAW_SECTOR;
			}

			SetThemeBack(ctx,meng.theme);

			if (meng.pannel_choice == ADD_OBJ)
				ctx->style.button.normal = ctx->style.button.hover;

			if (nk_button_label(ctx, "Add scene"))
			{
				meng.command = ADD_OBJ;
				meng.pannel_choice = ADD_OBJ;
			}

			SetThemeBack(ctx,meng.theme);

			if (meng.pannel_choice == ADD_SPRITE)
				ctx->style.button.normal = ctx->style.button.hover;

			if (nk_button_label(ctx, "Add Sprite"))
				meng.command = meng.pannel_choice = ADD_SPRITE;

			SetThemeBack(ctx,meng.theme);

			if (meng.pannel_choice == NLIGHTING)
				ctx->style.button.normal = ctx->style.button.hover;

			if (nk_button_label(ctx, "Lighting"))
			{
				meng.command = NADD_LIGHT;
				meng.pannel_choice = NLIGHTING;
			}

			SetThemeBack(ctx,meng.theme);

			if(meng.pannel_choice == NLBLOCK_PANNEL)
				ctx->style.button.normal = ctx->style.button.hover;

			if (nk_button_label(ctx, "Light Blocker"))
				meng.command = meng.pannel_choice = NLBLOCK_PANNEL;

			SetThemeBack(ctx, meng.theme);

			//nk_spacing(ctx, 1);

			if (meng.pannel_choice == NLBLOCK_PANNEL)
			{
				nk_spacing(ctx, 1);
				nk_layout_row_dynamic(ctx, 30, 2);

				if (meng.command == NLBLOCK_ADD_CIRCLE)
					ctx->style.button.normal = ctx->style.button.hover;

				if (nk_button_label(ctx, "Circle"))
					meng.command = NLBLOCK_ADD_CIRCLE;

				SetThemeBack(ctx, meng.theme);

				if (meng.command == NLBLOCK_ADD_QUAD)
					ctx->style.button.normal = ctx->style.button.hover;

				if (nk_button_label(ctx, "Quad"))
					meng.command = NLBLOCK_ADD_QUAD;

				SetThemeBack(ctx, meng.theme);

				if (meng.command == NLBLOCK_ADD_CUSTOM)
					ctx->style.button.normal = ctx->style.button.hover;

				if (nk_button_label(ctx, "Custom"))
					meng.command = NLBLOCK_ADD_CUSTOM;

				SetThemeBack(ctx, meng.theme);
			}

			if (meng.pannel_choice == NLIGHTING)
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				//if (nk_button_label(ctx, "Add light"))
					//meng.command = NADD_LIGHT;

				struct nk_color lcolor;
				lcolor.r = meng.light.color.r;
				lcolor.b = meng.light.color.b;
				lcolor.g = meng.light.color.g;

				lcolor = ColorPicker(lcolor);

				//meng.lightmap_color.r = lcolor.r;
				//meng.lightmap_color.g = lcolor.g;
				//meng.lightmap_color.b = lcolor.b;

				//nk_label(ctx, "Lightmap resolution", NK_TEXT_LB);
				nk_layout_row_dynamic(ctx, 30, 1);
				//nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, StringFormat("%d", meng.lightmap_res.x), 2048, nk_filter_decimal);
				
				//meng.lightmap_res.y = meng.lightmap_res.x;
				meng.light.color.r = lcolor.r;
				meng.light.color.g = lcolor.g;
				meng.light.color.b = lcolor.b;

				//char *lighttype[] = { "Point Light medium", "Point Light strong", "Point Light normal", "SpotLight medium", "SpotLight strong", "SpotLight normal" };
				//meng.light.type = nk_combo(ctx, lighttype, 6, meng.light.type - 1, 30, nk_vec2(110, 360)) + 1;

				meng.light.falloff = nk_propertyi(ctx, "Radius", 0, meng.light.falloff, 32768, 64, 8);
				meng.light.intensity = nk_propertyi(ctx, "Intensity", 0, meng.light.intensity, 256, 8, 1);
				meng.light.c = nk_propertyf(ctx, "Cutoff", 0, meng.light.c, 32, 0.1, 0.01);
				meng.light.l = nk_propertyi(ctx, "Midground Z", 0, meng.light.l, 7, 1, 1);

				//nk_button_label(ctx, "Load lightmap");
			}

			if (meng.pannel_choice == SELECT_EDIT)
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				char *layers2[] = { "Foreground", "Midground", "Background 1", "Background 2", "Background 3" };

				nk_label(ctx, "Selection layer", NK_TEXT_LB);
				meng.curlayer = nk_combo(ctx, layers2, 5, meng.curlayer, 25, nk_vec2(110, 200));
				meng.snap = nk_check_label(ctx, "Snap", meng.snap == 1 ? 1 : 0);

				if (meng.command2 == EDIT_SECTOR)
				{
					nk_layout_row_dynamic(ctx, 30, 1);

					st.Current_Map.sector[meng.com_id].floor_y_continued = nk_check_label(ctx, "Perspective floor", st.Current_Map.sector[meng.com_id].floor_y_continued);

					st.Current_Map.sector[meng.com_id].floor_y_up = nk_propertyi(ctx, "Floor up", 0, st.Current_Map.sector[meng.com_id].floor_y_up, 8192, 64, 8);
					st.Current_Map.sector[meng.com_id].floor_y_down = nk_propertyi(ctx, "Floor down", 0, st.Current_Map.sector[meng.com_id].floor_y_down, 8192, 64, 8);
				}

				if (meng.command2 == EDIT_SPRITE)
				{
					editcolor.r = st.Current_Map.sprites[meng.sprite_edit_selection].color.r;
					editcolor.g = st.Current_Map.sprites[meng.sprite_edit_selection].color.g;
					editcolor.b = st.Current_Map.sprites[meng.sprite_edit_selection].color.b;

					nk_layout_row_dynamic(ctx, 30, 1);

					editcolor = ColorPicker(editcolor);

					st.Current_Map.sprites[meng.sprite_edit_selection].color.r = editcolor.r;
					st.Current_Map.sprites[meng.sprite_edit_selection].color.g = editcolor.g;
					st.Current_Map.sprites[meng.sprite_edit_selection].color.b = editcolor.b;

					meng.spr.health = nk_propertyi(ctx, "Health", -32768, meng.spr.health, 32768, 20, 2);

					if (nk_button_label(ctx, "Tags"))
						meng.command = SPRITE_TAG;

					if (nk_button_label(ctx, "Physics"))
						meng.command = SPRITE_PHY;

					if (nk_button_label(ctx, "Transform"))
						meng.command = TRANSFORM_BOX;

					if (st.Current_Map.sprites[meng.sprite_edit_selection].flags & 32 || st.Current_Map.sprites[meng.sprite_edit_selection].flags & 64)
					{
						nk_spacing(ctx, 1);
						nk_label(ctx, "Default shadow", NK_TEXT_ALIGN_CENTERED);

						nk_layout_row_dynamic(ctx, 30, 2);

						uint8 flg = (st.Current_Map.sprites[meng.sprite_edit_selection].flags & 8) == 8;

						if (nk_option_label(ctx, "Block Shadow", flg) == 1 && flg == 0 && st.Current_Map.sprites[meng.sprite_edit_selection].flags & 32)
						{
							st.Current_Map.sprites[meng.sprite_edit_selection].flags |= 8;

							if (st.Current_Map.sprites[meng.sprite_edit_selection].flags & 64)
								st.Current_Map.sprites[meng.sprite_edit_selection].flags -= 16;
						}

						if (nk_option_label(ctx, "Persp. Shadow", !flg) && flg == 1 && st.Current_Map.sprites[meng.sprite_edit_selection].flags & 64)
						{
							if (st.Current_Map.sprites[meng.sprite_edit_selection].flags & 32)
								st.Current_Map.sprites[meng.sprite_edit_selection].flags -= 8;

							st.Current_Map.sprites[meng.sprite_edit_selection].flags |= 16;
						}
					}

					for (i = 0; i < st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].num_tags; i++)
					{
						if (strcmp(st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i], "MUSFX") == NULL)
						{
							//nk_layout_row_dynamic(ctx, 30, 2);

							if (nk_button_label(ctx, "Play Music"))
							{
								StopMusic();
								StopAllSounds();
								PlayMusic(st.Current_Map.sprites[meng.sprite_edit_selection].tags[i], 0);
							}

							if (nk_button_label(ctx, "Stop All Audio"))
							{
								StopMusic();
								StopAllSounds();
							}

							nk_layout_row_dynamic(ctx, 30, 1);
						}

						if (strcmp(st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i], "SNDFX") == NULL)
						{
							//nk_layout_row_dynamic(ctx, 30, 2);

							if (nk_button_label(ctx, "Play Sound"))
							{
								StopMusic();
								StopAllSounds();
								PlaySound(st.Current_Map.sprites[meng.sprite_edit_selection].tags[i],0);
							}

							if (nk_button_label(ctx, "Stop All Audio"))
							{
								StopMusic();
								StopAllSounds();
							}

							nk_layout_row_dynamic(ctx, 30, 1);
						}
					}

					nk_layout_row_dynamic(ctx, 30, 1);

					nk_label(ctx, "Current sprite layer", NK_TEXT_LB);
					meng.spr2.type = nk_combo(ctx, Layers, 5, st.Current_Map.sprites[meng.sprite_edit_selection].type_s, 25, nk_vec2(110, 200));

					st.Current_Map.sprites[meng.sprite_edit_selection].position.z = GetZLayer(st.Current_Map.sprites[meng.sprite_edit_selection].position.z,
						st.Current_Map.sprites[meng.sprite_edit_selection].type_s, meng.spr2.type);

					st.Current_Map.sprites[meng.sprite_edit_selection].type_s = meng.spr2.type;

					if (meng.sub_com == SCALER_SELECT)
						ctx->style.button.normal = ctx->style.button.hover;

					if (nk_button_label(ctx, "Scaler"))
					{
						if (meng.sub_com != SCALER_SELECT)
							meng.sub_com = SCALER_SELECT;
						else
							meng.sub_com = 0;
					}

					SetThemeBack(ctx);

					nk_layout_row_dynamic(ctx, 20, 1);
					nk_spacing(ctx, 1);
					nk_label(ctx, "Edge snap", NK_TEXT_ALIGN_CENTERED);
					nk_layout_row_dynamic(ctx, 20, 7);

					nk_spacing(ctx, 1);

					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 0 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 0 : meng.select_edge;
					nk_spacing(ctx, 1);
					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 1 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 1 : meng.select_edge;
					nk_spacing(ctx, 1);
					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 2 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 2 : meng.select_edge;

					nk_spacing(ctx, 1);
					nk_spacing(ctx, 7);
					nk_spacing(ctx, 1);

					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 3 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 3 : meng.select_edge;
					nk_spacing(ctx, 1);
					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 4 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 4 : meng.select_edge;
					nk_spacing(ctx, 1);
					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 5 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 5 : meng.select_edge;

					nk_spacing(ctx, 1);
					nk_spacing(ctx, 7);
					nk_spacing(ctx, 1);

					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 6 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 6 : meng.select_edge;
					nk_spacing(ctx, 1);
					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 7 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 7 : meng.select_edge;
					nk_spacing(ctx, 1);
					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 8 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 8 : meng.select_edge;

					nk_spacing(ctx, 1);

					nk_layout_row_dynamic(ctx, 25, 2);

					nk_spacing(ctx, 2);
					meng.snap_scale = nk_check_label(ctx, "Snap scale", meng.snap_scale == 1 ? 1 : 0);
					meng.snap_rot = nk_check_label(ctx, "Snap rotation", meng.snap_rot == 1 ? 1 : 0);
				}

				if (meng.command2 == EDIT_OBJ)
				{
					editcolor.r = st.Current_Map.obj[meng.obj_edit_selection].color.r;
					editcolor.g = st.Current_Map.obj[meng.obj_edit_selection].color.g;
					editcolor.b = st.Current_Map.obj[meng.obj_edit_selection].color.b;
					editcolor.a = st.Current_Map.obj[meng.obj_edit_selection].color.a;

					nk_layout_row_dynamic(ctx, 30, 1);

					editcolor = ColorPicker(editcolor);

					st.Current_Map.obj[meng.obj_edit_selection].color.r = editcolor.r;
					st.Current_Map.obj[meng.obj_edit_selection].color.g = editcolor.g;
					st.Current_Map.obj[meng.obj_edit_selection].color.b = editcolor.b;
					st.Current_Map.obj[meng.obj_edit_selection].color.a = editcolor.a;

					st.Current_Map.obj[meng.obj_edit_selection].amblight = nk_propertyf(ctx, "Amb. Light", 0, st.Current_Map.obj[meng.obj_edit_selection].amblight, 1.0, 0.1, 0.01);

					if (st.Current_Map.obj[meng.obj_edit_selection].type == BACKGROUND3 || st.Current_Map.obj[meng.obj_edit_selection].type == BACKGROUND2
						|| st.Current_Map.obj[meng.obj_edit_selection].type == FOREGROUND)
					{
						if (st.Current_Map.obj[meng.obj_edit_selection].flag & 1)
							temp = 1;
						else
							temp = 0;

						if (nk_checkbox_label(ctx, "Texture Mov.", &temp))
						{
							if (temp)
								st.Current_Map.obj[meng.obj_edit_selection].flag |= 1;
							else
								st.Current_Map.obj[meng.obj_edit_selection].flag -= 1;
						}
					}

					if (nk_button_label(ctx, "Transform"))
						meng.command = TRANSFORM_BOX;

					nk_label(ctx, "Current scenario layer", NK_TEXT_LB);
					meng.obj2.type = nk_combo(ctx, Layers, 5, st.Current_Map.obj[meng.obj_edit_selection].type, 25, nk_vec2(110, 200));

					st.Current_Map.obj[meng.obj_edit_selection].position.z = GetZLayer(st.Current_Map.obj[meng.obj_edit_selection].position.z,
						st.Current_Map.obj[meng.obj_edit_selection].type, meng.obj2.type);

					st.Current_Map.obj[meng.obj_edit_selection].type = meng.obj2.type;

					if (meng.sub_com == SCALER_SELECT)
						ctx->style.button.normal = ctx->style.button.hover;

					if (nk_button_label(ctx, "Scaler"))
					{
						if (meng.sub_com != SCALER_SELECT)
							meng.sub_com = SCALER_SELECT;
						else
							meng.sub_com = 0;
					}


					SetThemeBack(ctx);

					nk_layout_row_dynamic(ctx, 20, 1);
					nk_spacing(ctx, 1);
					meng.snap = nk_check_label(ctx, "Snap", meng.snap == 1 ? 1 : 0);
					nk_label(ctx, "Edge snap", NK_TEXT_ALIGN_CENTERED);
					nk_layout_row_dynamic(ctx, 20, 7);

					nk_spacing(ctx, 1);

					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 0 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 0 : meng.select_edge;
					nk_spacing(ctx, 1);
					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 1 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 1 : meng.select_edge;
					nk_spacing(ctx, 1);
					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 2 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 2 : meng.select_edge;

					nk_spacing(ctx, 1);
					nk_spacing(ctx, 7);
					nk_spacing(ctx, 1);

					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 3 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 3 : meng.select_edge;
					nk_spacing(ctx, 1);
					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 4 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 4 : meng.select_edge;
					nk_spacing(ctx, 1);
					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 5 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 5 : meng.select_edge;

					nk_spacing(ctx, 1);
					nk_spacing(ctx, 7);
					nk_spacing(ctx, 1);

					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 6 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 6 : meng.select_edge;
					nk_spacing(ctx, 1);
					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 7 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 7 : meng.select_edge;
					nk_spacing(ctx, 1);
					meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 8 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 8 : meng.select_edge;

					nk_spacing(ctx, 1);

					nk_layout_row_dynamic(ctx, 25, 2);

					nk_spacing(ctx, 2);
					meng.snap_scale = nk_check_label(ctx, "Snap scale", meng.snap_scale == 1 ? 1 : 0);
					meng.snap_rot = nk_check_label(ctx, "Snap rotation", meng.snap_rot == 1 ? 1 : 0);

					nk_layout_row_dynamic(ctx, 25, 1);

					nk_spacing(ctx, 1);

					if (meng.sub_com == OBJEXTRUDE)
						ctx->style.button.normal = ctx->style.button.hover;

					if (nk_button_label(ctx, "Extrude"))
					{
						if (meng.sub_com != OBJEXTRUDE)
							meng.sub_com = OBJEXTRUDE;
						else
							meng.sub_com = 0;
					}

					SetThemeBack(ctx);

					/*
					if (st.Current_Map.obj[meng.obj_edit_selection].type == MIDGROUND)
					{
						if (st.Current_Map.obj[meng.obj_edit_selection].flag & OBJF_ANIMATED_TEXTURE_MOV_CAM)
						{
							if (nk_check_label(ctx, "Anim. Tex. Mov. with Camera", 1))
								st.Current_Map.obj[meng.obj_edit_selection].flag -= 2;
						}
						else
						{
							if (nk_check_label(ctx, "Anim. Tex. Mov. with Camera", 0))
								st.Current_Map.obj[meng.obj_edit_selection].flag |= 2;
						}
					}
					*/
				}

				if (meng.command2 == NEDIT_LIGHT)
				{
					//nk_layout_row_dynamic(ctx, 30, 1);
					//if (nk_button_label(ctx, "Add light"))
						//meng.command = NADD_LIGHT;

					_GAME_LIGHTMAPS *ls;
					ls = &st.game_lightmaps[meng.light_edit_selection];

					struct nk_color lcolor;
					lcolor.r = ls->ambient_color.r;
					lcolor.b = ls->ambient_color.b;
					lcolor.g = ls->ambient_color.g;

					lcolor = ColorPicker(lcolor);

					//meng.lightmap_color.r = lcolor.r;
					//meng.lightmap_color.g = lcolor.g;
					//meng.lightmap_color.b = lcolor.b;

					//nk_label(ctx, "Lightmap resolution", NK_TEXT_LB);
					nk_layout_row_dynamic(ctx, 30, 1);
					//nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, StringFormat("%d", meng.lightmap_res.x), 2048, nk_filter_decimal);

					//meng.lightmap_res.y = meng.lightmap_res.x;
					ls->ambient_color.r = lcolor.r;
					ls->ambient_color.g = lcolor.g;
					ls->ambient_color.b = lcolor.b;

					//char *lighttype[] = { "Point Light medium", "Point Light strong", "Point Light normal", "SpotLight medium", "SpotLight strong", "SpotLight normal" };
					//meng.light.type = nk_combo(ctx, lighttype, 6, meng.light.type - 1, 30, nk_vec2(110, 360)) + 1;

					int ltype = ls->type;
					nk_combobox_string(ctx, "Point Light\0Spot Light\0", &ltype, 2, 25, nk_vec2(100, 100));
					ls->type = ltype;

					ls->falloff[0] = nk_propertyi(ctx, "Radius", 0, ls->falloff[0], 32768, 32, 4);
					ls->falloff[1] = nk_propertyi(ctx, "Intensity", 0, ls->falloff[1], 256, 8, 1);
					ls->falloff[2] = nk_propertyf(ctx, "Cutoff", 0, ls->falloff[2], 32, 0.1, 0.01);
					ls->falloff[4] = nk_propertyi(ctx, "Midground Z", 0, ls->falloff[4], 7, 1, 1);

					if (ls->type == SPOTLIGHT)
					{
						ls->spotcos = nk_propertyi(ctx, "Spot angle", 10, ls->spotcos, 900, 50, 10);
						ls->spotinnercos = nk_propertyi(ctx, "Spot inner angle", 5, ls->spotinnercos, ls->spotcos - 5, 100, 10);
					}

					ls->W_w = ls->W_h = (float)(ls->falloff[0] * (mSqrt(1.0f / ls->falloff[2]) - 1.0f)) * 4.0f;

					STW(&ls->W_w, &ls->W_h);

					FixLayerBar();

					nk_button_label(ctx, "Load lightmap");
				}
			}

			if (meng.pannel_choice == ADD_OBJ)
			{
				editcolor.r = meng.obj.color.r;
				editcolor.g = meng.obj.color.g;
				editcolor.b = meng.obj.color.b;
				editcolor.a = meng.obj.color.a;

				nk_layout_row_dynamic(ctx, 30, 1);

				meng.obj.type = nk_combo(ctx, Layers, 5, meng.obj.type, 25, nk_vec2(110, 200));

				editcolor = ColorPicker(editcolor);

				meng.obj.color.r = editcolor.r;
				meng.obj.color.g = editcolor.g;
				meng.obj.color.b = editcolor.b;
				meng.obj.color.a = editcolor.a;

				meng.obj.amblight = nk_propertyf(ctx, "Amb. Light", 0, meng.obj.amblight, 1.0, 0.1, 0.01);

				nk_layout_row_dynamic(ctx, (st.screenx * 0.15f) / 2, 2);

				data = meng.tex_selection;

				if (st.Current_Map.num_mgg > 0)
				{
					if (data.vb_id != -1)
					{
						px = ((float)data.posx / 32768) * data.w;
						ceil(px);
						//px += data.x_offset;
						py = ((float)data.posy / 32768) * data.h;
						ceil(py);
						//py += data.y_offset;
						sx = ((float)data.sizex / 32768) * data.w;
						ceil(sx);
						sy = ((float)data.sizey / 32768) * data.h;
						ceil(sy);
						texid = nk_subimage_id(data.data, data.w, data.h, nk_rect(px, py, sx, sy));
					}
					else
						texid = nk_image_id(data.data);

					nk_image(ctx, texid);
				}
				else
					nk_label(ctx, "No MGGs loaded", NK_TEXT_ALIGN_CENTERED);

				//nk_layout_row_dynamic(ctx, 30, 1);

				if (nk_button_label(ctx, "Select Texture"))
					meng.command = TEX_SEL;

				nk_layout_row_dynamic(ctx, 30, 1);

				if (meng.obj.type == BACKGROUND3 || meng.obj.type == BACKGROUND2
					|| meng.obj.type == FOREGROUND)
				{
					if (meng.obj.flag & 1)
						temp = 1;
					else
						temp = 0;

					if (nk_checkbox_label(ctx, "Texture Mov.", &temp))
					{
						if (temp)
							meng.obj.flag |= 1;
						else
							meng.obj.flag -= 1;
					}
				}

				nk_layout_row_dynamic(ctx, 20, 1);
				nk_label(ctx, "Edge snap", NK_TEXT_ALIGN_CENTERED);
				nk_layout_row_dynamic(ctx, 20, 7);

				nk_spacing(ctx, 1);

				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 0 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 0 : meng.select_edge;
				nk_spacing(ctx, 1);
				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 1 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 1 : meng.select_edge;
				nk_spacing(ctx, 1);
				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 2 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 2 : meng.select_edge;

				nk_spacing(ctx, 1);
				nk_spacing(ctx, 7);
				nk_spacing(ctx, 1);

				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 3 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 3 : meng.select_edge;
				nk_spacing(ctx, 1);
				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 4 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 4 : meng.select_edge;
				nk_spacing(ctx, 1);
				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 5 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 5 : meng.select_edge;

				nk_spacing(ctx, 1);
				nk_spacing(ctx, 7);
				nk_spacing(ctx, 1);

				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 6 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 6 : meng.select_edge;
				nk_spacing(ctx, 1);
				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 7 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 7 : meng.select_edge;
				nk_spacing(ctx, 1);
				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 8 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 8 : meng.select_edge;

				nk_spacing(ctx, 1);

				nk_layout_row_dynamic(ctx, 20, 2);
				meng.snap = nk_check_label(ctx, "Snap", meng.snap == 1 ? 1 : 0);
				meng.snap_fit = nk_check_label(ctx, "Snap fit", meng.snap_fit == 1 ? 1 : 0);
			}

			if (meng.pannel_choice == ADD_SPRITE)
			{
				editcolor.r = meng.spr.color.r;
				editcolor.g = meng.spr.color.g;
				editcolor.b = meng.spr.color.b;

				nk_layout_row_dynamic(ctx, 30, 1);

				meng.spr.type = nk_combo(ctx, Layers, 5, meng.spr.type, 25, nk_vec2(110, 200));

				editcolor = ColorPicker(editcolor);

				meng.spr.color.r = editcolor.r;
				meng.spr.color.g = editcolor.g;
				meng.spr.color.b = editcolor.b;

				meng.spr.health = nk_propertyi(ctx, "Health", -32768, meng.spr.health, 32768, 20, 2);

				nk_layout_row_dynamic(ctx, (st.screenx * 0.15f)/2, 2);

				data = mgg_game[st.Game_Sprites[meng.sprite_selection].MGG_ID].frames[meng.sprite_frame_selection];

				if (data.vb_id != -1)
				{
					px = ((float)data.posx / 32768) * data.w;
					ceil(px);
					//px += data.x_offset;
					py = ((float)data.posy / 32768) * data.h;
					ceil(py);
					//py += data.y_offset;
					sx = ((float)data.sizex / 32768) * data.w;
					ceil(sx);
					sy = ((float)data.sizey / 32768) * data.h;
					ceil(sy);
					texid = nk_subimage_id(data.data, data.w, data.h, nk_recta(nk_vec2(px, py), nk_vec2(sx, sy)));
				}
				else
					texid = nk_image_id(data.data);

				nk_image(ctx, texid);

				//nk_layout_row_dynamic(ctx, 30, 2);

				if (nk_button_label(ctx, "Select Sprite"))
					meng.command = SPRITE_SELECTION;

				nk_layout_row_dynamic(ctx, 30, 2);

				nk_label(ctx, st.Game_Sprites[meng.sprite_selection].name, NK_TEXT_ALIGN_CENTERED);

				nk_layout_row_dynamic(ctx, 20, 1);
				meng.snap = nk_check_label(ctx, "Snap", meng.snap == 1 ? 1 : 0);
				nk_label(ctx, "Edge snap", NK_TEXT_ALIGN_CENTERED);
				nk_layout_row_dynamic(ctx, 20, 7);

				nk_spacing(ctx, 1);

				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 0 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 0 : meng.select_edge;
				nk_spacing(ctx, 1);
				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 1 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 1 : meng.select_edge;
				nk_spacing(ctx, 1);
				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 2 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 2 : meng.select_edge;

				nk_spacing(ctx, 1);
				nk_spacing(ctx, 7);
				nk_spacing(ctx, 1);

				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 3 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 3 : meng.select_edge;
				nk_spacing(ctx, 1);
				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 4 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 4 : meng.select_edge;
				nk_spacing(ctx, 1);
				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 5 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 5 : meng.select_edge;

				nk_spacing(ctx, 1);
				nk_spacing(ctx, 7);
				nk_spacing(ctx, 1);

				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 6 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 6 : meng.select_edge;
				nk_spacing(ctx, 1);
				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 7 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 7 : meng.select_edge;
				nk_spacing(ctx, 1);
				meng.select_edge = nk_button_symbol(ctx, meng.select_edge == 8 ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_RECT_OUTLINE) == 1 ? 8 : meng.select_edge;

				nk_spacing(ctx, 1);

			}
		}

		nk_end(ctx);
	}
}

int main(int argc, char *argv[])
{
	int16 i = 0, test = 0, ch, ch2, ch3, op_file = -1;
	int16 testa = 0;
	char str[64];

	uint8 t1;

	uint32 t3;

	float t2;

	int loops;

	struct nk_color background;

	//_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);

	PreInit("meng", argc, argv);
	

	if(LoadCFG()==0)
		if(MessageBox(NULL,"Error while trying to read or write the configuration file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();

	strcpy(st.LogName, "meng.log");
		
	Init();
	DisplaySplashScreen();

	strcpy(st.WindowTitle,"Engineer");

	OpenFont("Font/Roboto-Regular.ttf", "arial", 0, 128);
	OpenFont("Font/mUI.ttf", "arial bold", 1, 128);
	//OpenFont("font//tt0524m_.ttf","geometry",2,128);

	InitMGG();

	//LoadMGG(&mgg_map[0], "tex01n.mgg");
	
	if(LoadMGG(&mgg_sys[0],"data/mEngUI.mgg")==NULL)
	{
		LogApp("Could not open UI mgg");
		Quit();
	}
	
	UILoadSystem("UI_Sys.cfg");

	if (argc > 0)
	{
		for (i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], "-p") == NULL)
				strcpy(meng.prj_path, argv[i + 1]);
			else if (strcmp(argv[i], "-o") == NULL)
				op_file = i + 1;
		}
	}

	SetCurrentDirectory(meng.prj_path);
	LogApp("Project directory is \"%s\"", meng.prj_path);

	meng.num_mgg=0;
	memset(st.Game_Sprites,0,MAX_SPRITES*sizeof(_SPRITES));
	st.num_sprites=0;
	meng.got_it=-1;
	meng.sprite_selection=-1;
	meng.sprite_frame_selection=-1;
	meng.spr.size.x=2048;
	meng.spr.size.y=2048;
	meng.editview = MIDGROUND_MODE;
	meng.LayerBar = 0;
	

	//SpriteListLoad();

	//BASICBKD(255,255,255);
	//DrawString2UI("Loading assets...", 8192, 4608, 0, 0, 0, 255, 255, 255, 255, ARIAL, 2048, 2048, 6);

	char ico[2];

	ico[0] = 30;
	ico[1] = 0;

	//DrawString2UI(ico, 8192, st.gamey - 2048, 0, 0, 0, 255, 255, 255, 255, 1, 4096, 4096, 6);

	//Renderer(1);
	//SwapBuffer(wn);

	LoadSpriteList("sprite.slist");

	LoadSoundList("Data/Audio/sound.list");

	background = nk_rgb(28, 48, 62);

	for (i = 0; i < st.num_sounds; i++)
		strcpy(meng.soundlist[i], st.sounds[i].path);

	for (i = 0; i < st.num_musics; i++)
		strcpy(meng.musiclist[i], st.musics[i].path);

	st.FPSYes=1;

	st.gt=MAIN_MENU;

	meng.pannel_choice=2;
	meng.command=2;

	st.Developer_Mode=1;

	meng.menu_sel=0;

	meng.path=(char*) malloc(2);
	strcpy(meng.path,".");

	//st.gt=STARTUP;
	st.gt = INGAME;

	memset(&meng.spr.body,0,sizeof(Body));

	curr_tic=GetTicks();
	delta=1;

	st.viewmode=31+64;
	meng.loop_complete=0;
	meng.editor=0;
	meng.hide_ui=0;

	memset(meng.layers, 0, 2048 * 57 * sizeof(int16));

	SetCurrentDirectory(st.CurrPath);

	ctx = nk_sdl_init(wn, MAX_NK_BUFFER, MAX_NK_VERTEX_BUFFER, MAX_NK_ELEMENT_BUFFER);

	struct nk_font_atlas *atlas;
	nk_sdl_font_stash_begin(&atlas);
	fonts[0] = nk_font_atlas_add_from_file(atlas, "Font\\Roboto-Regular.ttf", 16, 0);
	fonts[1] = nk_font_atlas_add_from_file(atlas, "Font\\mUI.ttf", 18, 0);
	fonts[2] = nk_font_atlas_add_from_file(atlas, "Font\\Roboto-Regular.ttf", 14, 0);
	nk_sdl_font_stash_end();
	nk_style_set_font(ctx, &fonts[0]->handle);

	SetCurrentDirectory(meng.prj_path);

	InitEngineWindow();

	NewMap();

	SetSkin(ctx, THEME_BLACK);
	meng.theme = THEME_BLACK;

	SETENGINEPATH;

	if (op_file != -1)
	{
		if (LoadMap(argv[op_file]))
		{
			memset(meng.mgg_list, 0, 32 * 256);
			meng.num_mgg = 0;
			LogApp("Map %s loaded", st.Current_Map.name);

			for (int a = 0, id = 0; a < st.Current_Map.num_mgg; a++)
			{
				DrawUI(8192, 4608, 16384, 8192, 0, 0, 0, 0, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, mgg_sys[0].frames[4], 255, 0);
				sprintf(str, "Loading %d%", (a / st.Current_Map.num_mgg) * 100);
				DrawString2UI(str, 8192, 4608, 1, 1, 0, 255, 255, 255, 255, ARIAL, FONT_SIZE * 2, FONT_SIZE * 2, 0);
				char path2[MAX_PATH];
				//PathRelativePathTo(path2, meng.prj_path, FILE_ATTRIBUTE_DIRECTORY, st.Current_Map.MGG_FILES[a], FILE_ATTRIBUTE_DIRECTORY);
				strcpy(path2, meng.prj_path);
				strcat(path2, "\\");
				strcat(path2, st.Current_Map.MGG_FILES[a]);

				if (CheckMGGFile(path2))
				{
					LoadMGG(&mgg_map[id], path2);
					strcpy(meng.mgg_list[a], mgg_map[id].name);
					meng.num_mgg++;
					id++;
				}
				else
				{
					FreeMap();

					LogApp("Error while loading map's MGG: %s", st.Current_Map.MGG_FILES[a]);
					break;
				}

			}

			st.Camera.position.x = 0;
			st.Camera.position.y = 0;
			meng.scroll = 0;
			meng.tex_selection.data = -1;
			meng.command2 = 0;
			meng.scroll2 = 0;
			meng.mgg_sel = 0;
			meng.pannel_choice = 2;
			meng.command = 2;
			meng.menu_sel = 0;
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
			st.gt = INGAME;
			st.mouse1 = 0;
			//free(path2);
			free(meng.path);
			meng.path = (char*)malloc(2);
			strcpy(meng.path, ".");

			meng.viewmode = 7;

			memset(meng.z_buffer, -1, 2048 * 57 * sizeof(int16));
			memset(meng.z_slot, 0, 57 * sizeof(int16));
			meng.z_used = 0;

			for (int m = 0; m<st.Current_Map.num_obj; m++)
			{
				meng.z_buffer[st.Current_Map.obj[m].position.z][meng.z_slot[st.Current_Map.obj[m].position.z]] = m;
				meng.z_slot[st.Current_Map.obj[m].position.z]++;
				if (st.Current_Map.obj[m].position.z>meng.z_used)
					meng.z_used = st.Current_Map.obj[m].position.z;
			}

			for (int m = 0; m<st.Current_Map.num_sprites; m++)
			{
				meng.z_buffer[st.Current_Map.sprites[m].position.z][meng.z_slot[st.Current_Map.sprites[m].position.z]] = m + 2000;
				meng.z_slot[st.Current_Map.sprites[m].position.z]++;
				if (st.Current_Map.sprites[m].position.z>meng.z_used)
					meng.z_used = st.Current_Map.sprites[m].position.z;
			}

			for (int m = 0; m<st.Current_Map.num_sector; m++)
			{
				meng.z_buffer[24][meng.z_slot[24]] = m + 10000;
				meng.z_slot[24]++;
				if (24>meng.z_used)
					meng.z_used = 24;
			}

			for (int m = 1; m <= st.num_lights; m++)
			{
				int8 lz = st.game_lightmaps[m].falloff[4] + 24;

				meng.z_buffer[lz][meng.z_slot[lz]] = m + 12000;
				meng.z_slot[lz]++;

				if (lz>meng.z_used)
					meng.z_used = lz;
			}

			SetCurrentDirectory(meng.prj_path);

			//break;
		}
	}
	//SetDirContent("mgg");

	while(!st.quit)
	{
		if(st.FPSYes)
			FPSCounter();

		nk_input_begin(ctx);

		while(PollEvent(&events))
		{
			WindowEvents();
			nk_sdl_handle_event(&events);
		}

		nk_input_end(ctx);

		/*
		if(meng.viewmode==LIGHTVIEW_MODE || meng.viewmode==INGAMEVIEW_MODE)
			BASICBKD(st.Current_Map.amb_color.r,st.Current_Map.amb_color.g,st.Current_Map.amb_color.b);
		else
			BASICBKD(255,255,255);
		*/
		loops = 0;

		uint32 c_tick = GetTicks();

		while(c_tick > curr_tic && loops < 10)
		{
			if (meng.scaling == 1 && meng.sub_com != SCALER_SELECT)
			{
				meng.scaling = 0;
				st.cursor_type = CURSOR_DEFAULT;
			}

			//nk_clear(ctx);
			Finish();

			if(st.gt==INGAME)
			{
				if(meng.editor==0)
				{

					/*if(st.keys[SPACE_KEY].state)
					{
						PlayMovie("TEN2.MGV");
						st.keys[SPACE_KEY].state=0;
					//}

					/*if(meng.command==TEX_SEL)
					{
						ImageList();
						if(st.keys[ESC_KEY].state)
						{
							meng.command=meng.pannel_choice;
							st.keys[ESC_KEY].state=0;
						}
					}*/
					/*else
					if(meng.command==SPRITE_SELECTION)
					{
						SpriteList();
						if(st.keys[ESC_KEY].state)
						{
							meng.command=meng.pannel_choice;
							st.keys[ESC_KEY].state=0;
						}
					}*/
					/*else
					if(meng.command==MGG_SEL)
					{
						PannelLeft();
						MGGList();
						if(st.keys[ESC_KEY].state)
						{
							meng.command=meng.pannel_choice;
							st.keys[ESC_KEY].state=0;
						}
					}*/
					//else
					//if(meng.command==MGG_LOAD)
					//{
						//if(MGGLoad()==NULL) meng.command=meng.pannel_choice;
					//}
					//else
					if(meng.command==LOAD_LIGHTMAP)
					{
						if(UISelectFile(str))
						{
							if(LoadLightmapFromFile(str))
							{
								st.mouse1=0;
								meng.command=LOAD_LIGHTMAP2;
								LogApp("Lightmap loaded from %s", str);
							}
						}
					}
					else
					{
						if(meng.viewmode==INGAMEVIEW_MODE && st.keys[TAB_KEY].state)
						{
							if(meng.hide_ui)
								meng.hide_ui=0;
							else
								meng.hide_ui=1;

							st.keys[TAB_KEY].state=0;
						}

						//if((meng.viewmode==INGAMEVIEW_MODE && !meng.hide_ui) || meng.viewmode!=INGAMEVIEW_MODE)
							//PannelLeft();


						//nkrendered = 1;
						if (meng.command != SPRITE_TAG && meng.command != TRANSFORM_BOX && meng.command != SPRITE_SELECTION && meng.command != TEX_SEL && meng.command != MGG_SEL && meng.command != SPRITE_PHY)
							ViewPortCommands();

						/*if(st.keys[ESC_KEY].state && meng.command!=OBJ_EDIT_BOX)
						{
							st.gt=GAME_MENU;
							st.keys[ESC_KEY].state=0;
						}*/
					}
				}


			}
			else
			if(st.gt==MAIN_MENU || st.gt==GAME_MENU)
				Menu();

			for (int sp = 0; sp < st.Current_Map.num_sprites; sp++)
			{
				st.Current_Map.sprites[sp].current_sector = UpdateSector(st.Current_Map.sprites[sp].position, st.Current_Map.sprites[sp].body.size);
				st.Current_Map.sprites[sp].body.sector_id = UpdateSector(st.Current_Map.sprites[sp].position, st.Current_Map.sprites[sp].body.size);
			}

			curr_tic += 1000/TICSPERSECOND;
			loops++;
			//SetTimerM(1);

			
			if(meng.loop_complete)
			{
				loops = 10;
				meng.loop_complete = 0;
			}
			
			if((st.Text_Input && !meng.sub_com && !st.num_uiwindow && UI_Sys.current_option==-1) && (meng.command==ADD_LIGHT_TO_LIGHTMAP && !meng.got_it && st.Text_Input))
				st.Text_Input=0;

			if(meng.viewmode==INGAMEVIEW_MODE)
				LockCamera();

			MainSound();
		}

		DrawSys();

		if(st.gt==INGAME && meng.command!=MGG_LOAD && meng.command!=MGG_SEL && meng.editor==0)
		{
			DrawMap();

			ENGDrawLight();
		}

		if (meng.command == SPRITE_PHY)
			PhysicsBox(meng.sprite_edit_selection);

		if (meng.command == SPRITE_SELECTION)
			SpriteListSelection();

		if (meng.command == TEX_SEL)
			TextureListSelection();

		if (meng.command == TRANSFORM_BOX)
		{
			if (meng.command2 == EDIT_SPRITE)
				TransformBox(&st.Current_Map.sprites[meng.sprite_edit_selection].position, &st.Current_Map.sprites[meng.sprite_edit_selection].body.size,
				&st.Current_Map.sprites[meng.sprite_edit_selection].angle, NULL, NULL);

			if(meng.command2 == EDIT_OBJ)
				TransformBox(&st.Current_Map.obj[meng.obj_edit_selection].position, &st.Current_Map.obj[meng.obj_edit_selection].size,
				&st.Current_Map.obj[meng.obj_edit_selection].angle, &st.Current_Map.obj[meng.obj_edit_selection].texpan, &st.Current_Map.obj[meng.obj_edit_selection].texsize);
		}

		if (meng.command == SPRITE_TAG)
			TagBox(st.Current_Map.sprites[meng.sprite_edit_selection].GameID, meng.sprite_edit_selection);

		NewLeftPannel();
		CoordBar();
		LayerBar();
		MenuBar();

		//MapProperties();
		//CameraProperties();
		//TransformBox(&st.Current_Map.cam_area.area_pos, &st.Current_Map.cam_area.area_pos,&testa);

		UIMain_DrawSystem();
		//MainSound();
		Renderer(0);

		//float bg[4];
		//nk_color_fv(bg, background);

		//if (nkrendered == 0)
			//printf("porra\n");

		nk_sdl_render(NK_ANTI_ALIASING_OFF, MAX_NK_VERTEX_BUFFER, MAX_NK_ELEMENT_BUFFER, MAX_NK_COMMAND_BUFFER);
		//nk_clear(ctx);
		SwapBuffer(wn);

		nkrendered = 0;

		FixZLayers();
	}

	nk_sdl_shutdown();

	StopAllSounds();
	Quit();
	return 1;
}