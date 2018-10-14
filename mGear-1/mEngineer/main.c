#include "input.h"
#include "main.h"
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include "dirent.h"
#include "UI.h"
#include "mggeditor.h"
#include <commdlg.h>
#include <Windows.h>

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

//extern _MGG mgg_sys[3];

mEng meng, nmeng;

struct nk_font *fonts[4];

int nkrendered = 0;

struct nk_context *ctx;

int prev_tic, curr_tic, delta;

char *Layers[] = { "Background 1", "Background 2", "Background 3", "Midground", "Foreground" };

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

static void MGGList()
{
	uint16 j=0, i;
	UIData(8192,4608,2275,GAME_HEIGHT,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,5);
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
								meng.pre_size.y=(mgg_map[id].frames[m].h*9216)/st.screeny;
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

static void PannelLeft()
{
	uint8 mouse=0;
	char num[32], str[64], options[8][16]={"Foreground", "Midground", "Background1", "Background2", "Background3", "Light view", "Ingame view", "All"}, filen[2048];
	int32 i=0, j=0, xt, yt, k=0;
	static float asp;
	Pos p;
	PosF p2;
	static int8 winid[4];

	//UIPannelLeft();

	UIData(8192,128,16384,256,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,7);
	UIData(455,4608,455*2,9216,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,7);

	UIData(16384-1024,9216-512,2048,1024,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,7);

	sprintf(str,"MX: %d - MY: %d",(st.mouse.x*16384)/st.screenx, (st.mouse.y*9216)/st.screeny);
	StringUIData(str,16384-1024,9216-512-256,0,0,0,255,255,255,255,ARIAL,1536,1536,0);

	sprintf(str,"X: %d - Y: %d",st.Camera.position.x, st.Camera.position.y);
	StringUIData(str,16384-1024,9216-512,0,0,0,255,255,255,255,ARIAL,1536,1536,6);

	sprintf(str,"W: %0.2f - H: %0.2f",st.Camera.dimension.x, st.Camera.dimension.y);
	StringUIData(str,16384-1024,9216-256,0,0,0,255,255,255,255,ARIAL,1536,1536,6);

	if(UIStringButton(8096,128,"Camera Area",ARIAL,1536,6,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
	{
		meng.pannel_choice=1;
		meng.command=CAM_AREA;
		meng.command2=0;
		winid[1]=UICreateWindow2(8192,4608,CENTER,6,16,2048,32,ARIAL);
	}

	if(meng.command==CAM_AREA)
	{
		UIWin2_NumberBoxi32(winid[1],0,&st.Current_Map.cam_area.area_pos.x,"Area pos. X",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
		UIWin2_NumberBoxi32(winid[1],1,&st.Current_Map.cam_area.area_pos.y,"Area pos. Y",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
		UIWin2_NumberBoxf(winid[1],2,&st.Current_Map.cam_area.area_size.x,"Area size X",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
		UIWin2_NumberBoxf(winid[1],3,&st.Current_Map.cam_area.area_size.y,"Area size Y",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);

		UIWin2_NumberBoxf(winid[1],4,&st.Current_Map.cam_area.max_dim.x,"Max dim. X",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
		UIWin2_NumberBoxf(winid[1],5,&st.Current_Map.cam_area.max_dim.y,"Max dim. y",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);

		if(st.Current_Map.cam_area.horiz_lim)
		{
			if(UIWin2_MarkBox(winid[1],6,1,"Limit X mov.",UI_COL_NORMAL,UI_COL_SELECTED)==1)
				st.Current_Map.cam_area.horiz_lim=0;

			UIWin2_NumberBoxi32(winid[1],7,&st.Current_Map.cam_area.limit[0].x,"Area min X",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
			UIWin2_NumberBoxi32(winid[1],8,&st.Current_Map.cam_area.limit[1].x,"Area max X",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
		}
		else
		{
			if(UIWin2_MarkBox(winid[1],6,0,"Limit X mov.",UI_COL_NORMAL,UI_COL_SELECTED)==2)
				st.Current_Map.cam_area.horiz_lim=1;

			UIWin2_MarkBox(winid[1],7,2,"Area min X",UI_COL_NORMAL,UI_COL_SELECTED);
			UIWin2_MarkBox(winid[1],8,2,"Area max X",UI_COL_NORMAL,UI_COL_SELECTED);
		}

		if(st.Current_Map.cam_area.vert_lim)
		{
			if(UIWin2_MarkBox(winid[1],9,1,"Limit Y mov.",UI_COL_NORMAL,UI_COL_SELECTED)==1)
				st.Current_Map.cam_area.vert_lim=0;

			UIWin2_NumberBoxi32(winid[1],10,&st.Current_Map.cam_area.limit[0].y,"Area min Y",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
			UIWin2_NumberBoxi32(winid[1],11,&st.Current_Map.cam_area.limit[1].y,"Area max Y",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
		}
		else
		{
			if(UIWin2_MarkBox(winid[1],9,0,"Limit Y mov.",UI_COL_NORMAL,UI_COL_SELECTED)==2)
				st.Current_Map.cam_area.vert_lim=1;

			UIWin2_MarkBox(winid[1],10,2,"Area min Y",UI_COL_NORMAL,UI_COL_SELECTED);
			UIWin2_MarkBox(winid[1],11,2,"Area max Y",UI_COL_NORMAL,UI_COL_SELECTED);
		}
		
		if(UIWin2_StringButton(winid[1],12,"Edit area",UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
		{
			meng.command=CAM_AREA_EDIT;
			UIDestroyWindow(winid[1]);
		}
		
		if(UIWin2_StringButton(winid[1],13,"Edit limit X",UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
		{
			meng.command=CAM_LIM_X;
			UIDestroyWindow(winid[1]);
		}

		if(UIWin2_StringButton(winid[1],14,"Edit limit Y",UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
		{
			meng.command=CAM_LIM_Y;
			UIDestroyWindow(winid[1]);
		}

		if(UIWin2_StringButton(winid[1],15,"Done",UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
		{
			meng.command=meng.pannel_choice;
			meng.command2=0;
			UIDestroyWindow(winid[1]);
		}
		
	}

	if(UIStringButton(4096,128,"Load MGG",ARIAL,1536,6,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
	{
		meng.command=MGG_LOAD;
		SetDirContent("mgg");
		meng.command2=0;
	}

	if(meng.command==MGG_LOAD)
	{
		if(UISelectFile(filen))
		{
			if(CheckMGGFile(filen))
			{
				if(CheckMGGInSystem(filen)<99999)
				{
					DrawUI(8192,4608,4096,2048,0,0,0,0,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,0);
					DrawString2UI("Loading...",8192,4608,1,1,0,255,255,255,255,ARIAL,FONT_SIZE*3,FONT_SIZE*3,0);
					Renderer(1);
					LoadMGG(&mgg_map[st.Current_Map.num_mgg],filen);

					j=0;
					for(i=0;i<st.Current_Map.num_mgg;i++)
					{
						if(strcmp(meng.mgg_list[i],mgg_map[st.Current_Map.num_mgg].name)==NULL)
						{
							j=1;
							break;
						}
					}

					if(j==1)
						FreeMGG(&mgg_map[st.Current_Map.num_mgg]);
					else
					{
						strcpy(st.Current_Map.MGG_FILES[st.Current_Map.num_mgg],filen);
						strcpy(meng.mgg_list[st.Current_Map.num_mgg],mgg_map[st.Current_Map.num_mgg].name);
						meng.num_mgg++;
						st.Current_Map.num_mgg++;
						st.num_mgg++;
						LogApp("MGG %s loaded",filen);
						meng.command=meng.pannel_choice;
					}
				}
			}
		}
	}

	if(UIStringButton(2256,128,"Map Properties",ARIAL,1536,6,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
	{
		meng.command=meng.pannel_choice=MAP_PROPERTIES;
		winid[0]=UICreateWindow2(0,0,CENTER,7,16,2048,32,ARIAL);
	}

	if(UIStringButton(6096,128,"Viewmode",ARIAL,1536,6,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
		meng.command=VIEWMODE_BOX;

	if(meng.command==VIEWMODE_BOX)
	{
		switch(UIOptionBox(6096,1024,CUSTOM,options,8,ARIAL,1024,UI_COL_NORMAL,UI_COL_SELECTED))
		{
			case UI_SEL:
				{
					meng.viewmode=FOREGROUND_MODE;
					meng.command=meng.pannel_choice;
					break;
				}
			case UI_SEL+1:
				{
					meng.viewmode=MIDGROUND_MODE;
					meng.command=meng.pannel_choice;
					break;
				}
			case UI_SEL+2:
				{
					meng.viewmode=BACKGROUND1_MODE;
					meng.command=meng.pannel_choice;
					break;
				}
			case UI_SEL+3:
				{
					meng.viewmode=BACKGROUND2_MODE;
					meng.command=meng.pannel_choice;
					break;
				}
			case UI_SEL+4:
				{
					meng.viewmode=BACKGROUND3_MODE;
					meng.command=meng.pannel_choice;
					break;
				}
			case UI_SEL+5:
				{
					meng.viewmode=LIGHTVIEW_MODE;
					meng.command=meng.pannel_choice;
					if(st.viewmode & 64 && meng.pannel_choice!=ADD_LIGHT)
						st.viewmode-=64;
					else
						st.viewmode+=64;
					break;
				}
			case UI_SEL+6:
				{
					meng.viewmode=INGAMEVIEW_MODE;
					st.Camera.dimension=st.Current_Map.cam_area.area_size;
					meng.command=meng.pannel_choice;
					break;
				}
			case UI_SEL+7:
				{
					meng.viewmode=ALLVIEW_MODE;
					meng.command=meng.pannel_choice;
					break;
				}

				meng.command=meng.pannel_choice;
		}

		if(meng.viewmode==0)
			st.viewmode=1;
		else
		if(meng.viewmode==1)
			st.viewmode=2;
		else
		if(meng.viewmode==2)
			st.viewmode=4;
		else
		if(meng.viewmode==3)
			st.viewmode=8;
		else
		if(meng.viewmode==4)
			st.viewmode=16;
		else
		if(meng.viewmode==5)
			st.viewmode=31+32;
		else
		if(meng.viewmode==7)
			st.viewmode=31+64;
	}

	if(meng.pannel_choice==MAP_PROPERTIES)
	{
		if(meng.command==MAP_PROPERTIES && meng.command2==TEX_SEL)
		{
			st.Current_Map.bcktex_id=meng.tex_ID;
			st.Current_Map.bcktex_mgg=meng.tex_MGGID;
			winid[0]=UICreateWindow2(0,0,CENTER,7,16,2048,32,ARIAL);
			meng.command2=0;
		}

		if(!st.Current_Map.num_mgg)
			UIWin2_MarkBox(winid[0],0,2,"Textured Background3",UI_COL_NORMAL,UI_COL_SELECTED);
		else
		{
			if(st.Current_Map.bcktex_id==-1)
			{
				if(UIWin2_MarkBox(winid[0],1,0,"Textured Background3",UI_COL_NORMAL,UI_COL_SELECTED)==2)
				{
					meng.command=TEX_SEL;
					meng.command2=TEX_SEL;
					UIDestroyWindow(winid[0]);
				}
			}
			else
			if(UIWin2_MarkBox(winid[0],0,1,"Textured Background3",UI_COL_NORMAL,UI_COL_SELECTED)==1)
			{
					st.Current_Map.bcktex_id=-1;
					st.Current_Map.bcktex_mgg=0;
			}
		}
		
		if(st.Current_Map.bcktex_id>-1)
		{
			if(UIWin2_StringButton(winid[0],1,"Background3 texture",UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
			{
				meng.command=meng.command2=TEX_SEL;
				UIDestroyWindow(winid[0]);
			}
		}


		UIWin2_NumberBoxf(winid[0],2,&st.Current_Map.bck1_v,"BCK1 parallax vel",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
		UIWin2_NumberBoxf(winid[0],3,&st.Current_Map.bck2_v,"BCK2 parallax vel",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
		UIWin2_NumberBoxf(winid[0],4,&st.Current_Map.fr_v,"FR1 parallax vel",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);

		UIWin2_NumberBoxui8(winid[0],5,&st.Current_Map.amb_color.r,"Ambient Color R:",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
		UIWin2_NumberBoxui8(winid[0],6,&st.Current_Map.amb_color.g,"Ambient Color G:",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
		UIWin2_NumberBoxui8(winid[0],7,&st.Current_Map.amb_color.b,"Ambient Color B:",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);

		sprintf(str,"Map name: %s",st.Current_Map.name);
		UIWin2_TextBox(winid[0],8,str,UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
		sscanf(str,"Map name: %s",st.Current_Map.name);

		UIWin2_NumberBoxi32(winid[0],9,&st.Current_Map.bck3_pan.x,"BCK3 pan X",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
		UIWin2_NumberBoxi32(winid[0],10,&st.Current_Map.bck3_pan.y,"BCK3 pan Y",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
		UIWin2_NumberBoxi32(winid[0],11,&st.Current_Map.bck3_size.x,"BCK3 size X",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
		UIWin2_NumberBoxi32(winid[0],12,&st.Current_Map.bck3_size.y,"BCK3 size Y",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);

		if(UIWin2_StringButton(winid[0],15,"Done",UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
		{
			meng.command=meng.pannel_choice=1;
			UIDestroyWindow(winid[0]);
		}

	}

	if(!CheckCollisionMouse(227,227,455,455,0))
	{
		UIData(227,227,455,455,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[0],255,7);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		UIData(227,227,445,445,0,255,128,32,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[0],255,7);
		//time++;
		//if((st.time-time)==1000)
		//{
		StringUIData("Draw a sector",8192,9216-455,1820,455,0,255,128,32,255,ARIAL,0,0,0);
		//}

			if(st.mouse1 && !meng.current_command)
			{
				meng.command=meng.pannel_choice=meng.command2=0;
				st.mouse1=0;
			}
	}

	if(!CheckCollisionMouse(682,227,448,448,0))
	{
		UIData(682,227,448,448,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[2],255,7);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		UIData(682,227,448,448,0,255,128,32,0,0,32768,32768,mgg_sys[0].frames[2],255,7);
		//if((st.time-time)==1000)
		//{
		StringUIData("Select and edit",8192,9216-455,1820,455,0,255,128,32,255,ARIAL,0,0,0);
		//}

		if(st.mouse1 && !meng.current_command) meng.command=meng.pannel_choice=meng.command2=2;
	}
	
	if(!CheckCollisionMouse(227,682,448,448,0))
	{
		UIData(227,682,448,448,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[1],255,7);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		UIData(227,682,448,448,0,255,128,32,0,0,32768,32768,mgg_sys[0].frames[1],255,7);
		//if((st.time-time)==1000)
		//{
		StringUIData("Add an OBJ",8192,9216-455,1820,455,0,255,128,32,255,ARIAL,0,0,0);
		//}

		if(st.mouse1 && !meng.current_command)
		{
			//meng.tex_selection.data=meng.tex2_sel.data;
			//meng.tex_ID=meng.tex2_ID;
			//meng.tex_MGGID=meng.tex2_MGGID;

			meng.command=meng.pannel_choice=meng.command2=3;
		}
	}
	
	if(!CheckCollisionMouse(682,682,448,448,0))
	{
		UIData(682,682,448,448,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[3],255,7);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		UIData(682,682,448,448,0,255,128,32,0,0,32768,32768,mgg_sys[0].frames[3],255,7);
		//if((st.time-time)==1000)
		//{
		StringUIData("Add a sprite",8192,9216-455,1820,455,0,255,128,32,255,ARIAL,0,0,0);
		//}

			if(st.mouse1 && !meng.current_command)
			{		
				st.mouse1=0;
				meng.command=meng.pannel_choice=meng.command2=ADD_SPRITE;
			}
	}

	if(!CheckCollisionMouse(227,1137,448,488,0))
	{
		UIData(227,1137,448,448,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[5],255,7);
	}
	else
	{
		UIData(227,1137,448,448,0,255,128,32,0,0,32768,32768,mgg_sys[0].frames[5],255,7);

		StringUIData("Add a lightmap",8192,9216-455,1820,455,0,255,128,32,255,ARIAL,0,0,0);

		if(st.mouse1)
		{
			st.mouse1=0;
			meng.command=meng.pannel_choice=meng.command2=ADD_LIGHT;
			
			if(st.viewmode<64)
				st.viewmode+=64;
		}
	}
	
	if(meng.pannel_choice==ADD_OBJ || meng.pannel_choice==ADD_SPRITE)
	{
		if(meng.command!=ADD_SPRITE && meng.command!=SPRITE_TAG)
			sprintf(str,"Tex. Sel.");
		else
			sprintf(str,"Sprite Sel.");

		if(!CheckCollisionMouse(458,9216-2275,490,223,0))
		{
			StringUIData(str,458,9216-2275,490,223,0,255,255,255,255,ARIAL,0,0,0);
		}
		else
		{
			StringUIData(str,458,9216-2275,490,223,0,255,128,32,255,ARIAL,0,0,0);


			if(meng.command!=ADD_SPRITE && meng.command!=SPRITE_TAG)
				StringUIData("Texture Selection",8192,9216-455,2730,455,0,255,128,32,255,ARIAL,0,0,0);
			else
				StringUIData("Sprite Selection",8192,9216-455,2730,455,0,255,128,32,255,ARIAL,0,0,0);


			if(st.mouse1)
			{
				meng.scroll=0;

				if(meng.command!=ADD_SPRITE && meng.command!=SPRITE_TAG)
					meng.command=TEX_SEL;
				else
					meng.command=SPRITE_SELECTION;

				meng.command2=0;
				st.mouse1=0;
			}
		}
	}

	if(meng.pannel_choice==ADD_OBJ)
		UIData(455,9216-910,910,910,0,255,255,255,0,0,32768,32768,meng.tex_selection,255,0);

	if(meng.pannel_choice==0)
		UIData(227,227,455,455,0,128,32,32,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[0],255,0);
	//else
	if(meng.pannel_choice==2)
		UIData(682,227,455,455,0,128,32,32,0,0,32768,32768,mgg_sys[0].frames[2],255,0);
	//else

	if(meng.pannel_choice==ADD_LIGHT)
	{
		UIData(227,1137,455,455,0,128,32,32,0,0,32768,32768,mgg_sys[0].frames[5],255,0);

		if(CheckCollisionMouse(455,1900,810,455,0))
		{
			StringUIData("Create Lightmap",455,1900,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1 && !meng.current_command)
			{
				st.mouse1=0;
				meng.command=CREATE_LIGHTMAP;
			}
		}
		else
			StringUIData("Create Lightmap",455,1900,810,227,0,255,255,255,255,ARIAL,0,0,0);
		
		
		if(CheckCollisionMouse(455,1900+810,810,455,0))
		{
			StringUIData("Remove Lightmap",455,1900+810,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1 && !meng.current_command)
			{
				st.mouse1=0;
				meng.command=REMOVE_LIGHTMAP;
			}
		}
		else
			StringUIData("Remove Lightmap",455,1900+810,810,227,0,255,255,255,255,ARIAL,0,0,0);

		
		if(CheckCollisionMouse(455,2710+810,810,455,0))
		{
			StringUIData("Lightmap Res.",455,2710+810,810,227,0,255,128,32,255,ARIAL,0,0,0);

			StringUIData("Set the lightmap resolution",8192,9216-455,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

			if(st.mouse1 && !meng.current_command)
			{
				st.mouse1=0;
				meng.command=LIGHTMAP_RES;
			}
		}
		else
			StringUIData("Lightmap Res.",455,2710+810,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==LIGHTMAP_RES)
		{
			UIData(8192,4608,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			sprintf(str,"Res. Width %d",meng.lightmap_res.x);

			if(CheckCollisionMouse(8192,4608-512,810,455,0))
			{
				StringUIData(str,8192,4608-512,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					st.mouse1=0;
					StartText();
					sprintf(st.TextInput,"%d",meng.lightmap_res.x);
					meng.sub_com=1;
				}
			}
			else
				StringUIData(str,8192,4608-512,810,227,0,255,255,255,255,ARIAL,2048,2048,0);

			if(meng.sub_com==1)
			{
				meng.lightmap_res.x=atoi(st.TextInput);

				StringUIData(str,8192,4608-512,810,227,0,255,32,32,255,ARIAL,2048,2048,0);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"Res. Height %d",meng.lightmap_res.y);

			if(CheckCollisionMouse(8192,4608+512,810,455,0))
			{
				StringUIData(str,8192,4608+512,810,227,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					st.mouse1=0;
					StartText();
					sprintf(st.TextInput,"%d",meng.lightmap_res.y);
					meng.sub_com=2;
				}
			}
			else
				StringUIData(str,8192,4608+512,810,227,0,255,255,255,255,ARIAL,2048,2048,0);

			if(meng.sub_com==2)
			{
				meng.lightmap_res.y=atoi(st.TextInput);

				StringUIData(str,8192,4608+512,810,227,0,255,32,32,255,ARIAL,2048,2048,0);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			if(st.keys[ESC_KEY].state)
			{
				meng.command=0;
				st.keys[ESC_KEY].state=0;
				meng.sub_com=0;
			}
		}

		if(CheckCollisionMouse(455,3520+810,810,455,0))
		{
			StringUIData("Edit Lightmap",455,3520+810,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1 && !meng.current_command)
			{
				st.mouse1=0;
				meng.command=EDIT_LIGHTMAP;
				meng.got_it=-1;
			}
		}
		else
			StringUIData("Edit Lightmap",455,3520+810,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command!=MOVE_LIGHTMAP)
		{
			if(UIStringButton(455,3520+810+810,"Move lightmap",ARIAL,1024,0,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
			{
				meng.command=MOVE_LIGHTMAP;
				meng.got_it=-1;
			}
		}
		else
		{
			if(UIStringButton(455,3520+810+810,"Move lightmap",ARIAL,1024,0,UI_COL_CLICKED,UI_COL_CLICKED)==UI_SEL)
			{
				meng.command=meng.pannel_choice;;
				meng.got_it=-1;
			}
		}

		if(UIStringButton(455,3520+810+810+810,"Load lightmap",ARIAL,1024,0,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
		{
			meng.command=LOAD_LIGHTMAP;
			SetDirContent("tga");
			meng.got_it=-1;
		}
	}

	if(meng.pannel_choice==4)
	{
		UIData(682,682,455,455,0,128,32,32,0,0,32768,32768,mgg_sys[0].frames[3],255,0);

		UIData(455,9216-910,910,910,0,255,255,255,0,0,32768,32768,mgg_game[st.Game_Sprites[meng.sprite_selection].MGG_ID].frames[meng.sprite_frame_selection],255,0);

		if(meng.spr.type==MIDGROUND)
		{
			if(CheckCollisionMouse(465,1900,880,220,0))
			{
				StringUIData("Midground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=16;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Midground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.spr.type==FOREGROUND)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Foreground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=16;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Foreground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.spr.type==BACKGROUND1)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Background1",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=16;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background1",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.spr.type==BACKGROUND2)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Background2",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=16;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background2",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.spr.type==BACKGROUND3)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Background3",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=16;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background3",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.command==ADD_SPRITE_TYPE)
		{
			UIData(1365,1715,910,1210,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,0);

			if(CheckCollisionMouse(1365,1715,810,227,0))
			{
				StringUIData("Background1",1365,1715,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					meng.spr.type=BACKGROUND1;
					//st.Current_Map.obj[meng.com_id].position.z=
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background1",1365,1715,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckCollisionMouse(1365,1465,810,227,0))
			{
				StringUIData("Background2",1365,1465,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{

					meng.spr.type=BACKGROUND2;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background2",1365,1465,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckCollisionMouse(1365,1215,810,227,0))
			{
				StringUIData("Background3",1365,1215,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					

					meng.spr.type=BACKGROUND3;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background3",1365,1215,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckCollisionMouse(1365,1920,810,227,0))
			{
				StringUIData("Midground",1365,1920,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					

					meng.spr.type=MIDGROUND;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Midground",1365,1920,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckCollisionMouse(1365,2170,810,227,0))
			{
				StringUIData("Foreground",1365,2170,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					

					meng.spr.type=FOREGROUND;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Foreground",1365,2170,810,227,0,255,255,255,255,ARIAL,0,0,0);
		}

		sprintf(str,"color");

		if(CheckCollisionMouse(465,2445,810,217,0))
		{
			StringUIData(str,465,2445,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

			if(st.mouse1)
			{
				meng.command=RGB_SPRITE;
				st.mouse1=0;
			}
		}
		else
			StringUIData(str,465,2445,810,217,0, 255, 255, 255,255,ARIAL,2048,2048,0);

		if(meng.command==RGB_SPRITE)
		{
			UIData(8192,3072,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			Sys_ColorPicker(&meng.spr.color.r,&meng.spr.color.g,&meng.spr.color.b);

			StringUIData(str,465,2445,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

			sprintf(str,"R %d",meng.spr.color.r);

			if(meng.sub_com!=1)
			{
				if(CheckCollisionMouse(8192,2048+455,2048,227,0))
				{
					StringUIData(str,8192,2048+455,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.spr.color.r);
						StartText();
						st.mouse1=0;
						meng.sub_com=1;
					}
				}
				else
					StringUIData(str,8192,2048+455,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				StringUIData(str,8192,2048+455,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.spr.color.r=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"G %d",meng.spr.color.g);

			if(meng.sub_com!=2)
			{
				if(CheckCollisionMouse(8192,2048+810,2048,227,0))
				{
					StringUIData(str,8192,2048+810,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.spr.color.g);
						StartText();
						st.mouse1=0;
						meng.sub_com=2;
					}
				}
				else
					StringUIData(str,8192,2048+810,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				StringUIData(str,8192,2048+810,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.spr.color.g=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"B %d",meng.spr.color.b);

			if(meng.sub_com!=3)
			{
				if(CheckCollisionMouse(8192,2048+1265,2048,455,0))
				{
					StringUIData(str,8192,2048+1265,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.spr.color.b);
						StartText();
						st.mouse1=0;
						meng.sub_com=3;
					}
				}
				else
					StringUIData(str,8192,2048+1265,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==3)
			{
				StringUIData(str,8192,2048+1265,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.spr.color.b=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"A %d",meng.spr.color.a);

			if(meng.sub_com!=4)
			{
				if(CheckCollisionMouse(8192,2048+1720,2048,455,0))
				{
					StringUIData(str,8192,2048+1720,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.spr.color.a);
						StartText();
						st.mouse1=0;
						meng.sub_com=4;
					}
				}
				else
					StringUIData(str,8192,2048+1720,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==4)
			{
				StringUIData(str,8192,2048+1720,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.spr.color.a=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			if(st.keys[RETURN_KEY].state)
			{
				meng.command=meng.pannel_choice;
				st.keys[RETURN_KEY].state=0;
			}
		}


		if(CheckCollisionMouse(455,3135,810,217,0))
		{
			StringUIData("Tags",465,3135,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

			if(st.mouse1)
			{
				meng.command=SPRITE_TAG;
				st.mouse1=0;
			}
		}
		else
			StringUIData("Tags",465,3135,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
		
		if(meng.command==SPRITE_TAG)
		{
			UIData(8192,4608,8192,6144,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,7);

			for(i=1, yt=4608-2560;(i-1)<st.Game_Sprites[meng.sprite_selection].num_tags;i++, yt+=512)
			{
				j=strlen(st.Game_Sprites[meng.sprite_selection].tag_names[i-1]);

				if(st.Game_Sprites[meng.sprite_selection].tag_names[i-1][j-1]=='S' && st.Game_Sprites[meng.sprite_selection].tag_names[i-1][j-2]=='_')
					sprintf(str,"%s %s",st.Game_Sprites[meng.sprite_selection].tag_names[i-1],st.Game_Sprites[meng.sprite_selection].tags_str[i-1]);
				else
					sprintf(str,"%s %d",st.Game_Sprites[meng.sprite_selection].tag_names[i-1],st.Game_Sprites[meng.sprite_selection].tags[i-1]);

				if(meng.sub_com!=i && meng.sub_com<100)
				{
					if(CheckCollisionMouse(8192,yt,2048,455,0))
					{
						StringUIData(str,8192,yt,0,0,0,255,128,32,255,ARIAL,2048,2048,6);

						if(st.mouse1)
						{
							if (strcmp(st.Game_Sprites[meng.sprite_selection].tag_names[i - 1], "SNDFX") == NULL)
							{
								st.mouse1 = 0;
								meng.sub_com = i + 200;
							}
							else
							if (strcmp(st.Game_Sprites[meng.sprite_selection].tag_names[i - 1], "MUSFX") == NULL)
							{
								st.mouse1 = 0;
								meng.sub_com = i + 300;
							}
							else
							{

								if (st.Game_Sprites[meng.sprite_selection].tag_names[i - 1][j - 1] == 'S' && st.Game_Sprites[meng.sprite_selection].tag_names[i - 1][j - 2] == '_')
									sprintf(st.TextInput, "%s", st.Game_Sprites[meng.sprite_selection].tags_str[i - 1]);
								else
									sprintf(st.TextInput, "%d", st.Game_Sprites[meng.sprite_selection].tags[i - 1]);

								StartText();
								st.mouse1 = 0;
								meng.sub_com = i;
							}
						}

						if(st.mouse2 && st.Game_Sprites[meng.sprite_selection].tag_names[i-1][j-1]=='S' && 
							st.Game_Sprites[meng.sprite_selection].tag_names[i-1][j-2]=='_')
						{
							meng.sub_com=i+100;
							SetDirContent(NULL);
							st.mouse2=0;
						}
					}
					else
						StringUIData(str,8192,yt,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
				}
				else
				if(meng.sub_com==i)
				{
					StringUIData(str,8192,yt,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					if(st.Game_Sprites[meng.sprite_selection].tag_names[i-1][j-1]=='S' && st.Game_Sprites[meng.sprite_selection].tag_names[i-1][j-2]=='_')
						strcpy(st.Game_Sprites[meng.sprite_selection].tags_str[i-1],st.TextInput);
					else
						st.Game_Sprites[meng.sprite_selection].tags[i-1]=atoi(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.sub_com=0;
					}
				}
				else
				if(meng.sub_com==i+100)
				{
					if(UISelectFile(filen))
					{
						strcpy(st.Game_Sprites[meng.sprite_selection].tags_str[i-1],filen);
						meng.sub_com=0;
					}

					if(st.keys[RETURN_KEY].state)
					{
						st.keys[RETURN_KEY].state=0;
						meng.sub_com=0;
					}
				}
				else
				if (meng.sub_com == i + 200)
				{
					if ((st.Game_Sprites[meng.sprite_selection].tags[i - 1] = UIMakeList(meng.soundlist, st.num_sounds)) != -1)
						meng.sub_com = 0;

					if (st.keys[RETURN_KEY].state)
					{
						st.keys[RETURN_KEY].state = 0;
						meng.sub_com = 0;
					}
				}
				else
				if (meng.sub_com == i + 300)
				{
					if ((st.Game_Sprites[meng.sprite_selection].tags[i - 1] = UIMakeList(meng.musiclist, st.num_musics)) != -1)
						meng.sub_com = 0;

					if (st.keys[RETURN_KEY].state)
					{
						st.keys[RETURN_KEY].state = 0;
						meng.sub_com = 0;
					}
				}
			}

			if(st.keys[ESC_KEY].state)
			{
				meng.command=meng.pannel_choice;
				st.keys[ESC_KEY].state=0;
			}
		}

		sprintf(str,"Health %d",meng.spr.health);

		if(CheckCollisionMouse(465,3435,810,217,0))
		{
			StringUIData(str,465,3435,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1 && !StartText())
			{
				meng.command=SPRITE_HEALTH;
				sprintf(st.TextInput,"%d",meng.spr.health);
				st.mouse1=0;
			}
		}
		else
			StringUIData(str,465,3435,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==SPRITE_HEALTH)
		{
			StringUIData(str,465,3435,810,217,0,255,32,32,255,ARIAL,0,0,0);
			meng.spr.health=atoi(st.TextInput);

			if(st.keys[ESC_KEY].state)
			{
				StopText();
				meng.command=meng.pannel_choice;
				st.keys[ESC_KEY].state=0;
			}
		}

		sprintf(str,"Physics");

		if(CheckCollisionMouse(465,3690,810,217,0))
		{
			StringUIData(str,465,3690,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.command=SPRITE_PHY;
				st.mouse1=0;
			}
		}
		else
			StringUIData(str,465,3690,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==SPRITE_PHY)
		{
			DrawHud(8192, 4608, 2120,4608,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,7);

			sprintf(str,"Physics? %d",meng.spr.body.physics_on);

			if(CheckCollisionMouse(8192, 4608-1420,810,405,0))
			{
				StringUIData(str,8192, 4608-1420,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

				if(st.mouse1)
				{
					if(meng.spr.body.physics_on==0) meng.spr.body.physics_on=1;
					else meng.spr.body.physics_on=0;

					st.mouse1=0;
				}
			}
			else
				StringUIData(str,8192, 4608-1420,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

			sprintf(str,"Mass %.2f",meng.spr.body.mass);

			if(CheckCollisionMouse(8192, 4608-710,810,405,0))
			{
				StringUIData(str,8192, 4608-710,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

				if(st.mouse1 && !StartText())
				{
					if(meng.spr.body.mass==0)
						sprintf(st.TextInput,"%.0f",meng.spr.body.mass);
					else
						sprintf(st.TextInput,"%.2f",meng.spr.body.mass);
					meng.sub_com=10;
					st.mouse1=0;
				}
			}
			else
				StringUIData(str,8192, 4608-710,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

			if(meng.sub_com==10)
			{
				StringUIData(str,8192, 4608-1420,0,0,0,255,32,32,255,ARIAL,512*4,512*4,0);
				meng.spr.body.mass=atof(st.TextInput);
				if(st.keys[ESC_KEY].state)
				{
					StopText();
					meng.sub_com=0;
					st.keys[ESC_KEY].state=0;
				}
			}

			sprintf(str,"Flammable? %d",meng.spr.body.flamable);

			if(CheckCollisionMouse(8192, 4608,810,405,0))
			{
				StringUIData(str,8192, 4608,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

				if(st.mouse1)
				{
					if(meng.spr.body.flamable==0) meng.spr.body.flamable=1;
					else meng.spr.body.flamable=0;

					st.mouse1=0;
				}
			}
			else
				StringUIData(str,8192, 4608,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

			sprintf(str,"Explosive? %d",meng.spr.body.explosive);

			if(CheckCollisionMouse(8192, 4608+710,810,405,0))
			{
				StringUIData(str,8192, 4608+710,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

				if(st.mouse1)
				{
					if(meng.spr.body.explosive==0) meng.spr.body.explosive=1;
					else meng.spr.body.explosive=0;

					st.mouse1=0;
				}
			}
			else
				StringUIData(str,8192, 4608+710,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

			if(st.keys[ESC_KEY].state)
			{
				meng.command=meng.pannel_choice;
				st.keys[ESC_KEY].state=0;
			}
		}
	}

	//Sprite editing

	if (meng.command2 == EDIT_SPRITE)
	{
		p.x = st.Current_Map.sprites[meng.sprite_edit_selection].position.x;
		p.y = st.Current_Map.sprites[meng.sprite_edit_selection].position.y - (st.Current_Map.sprites[meng.sprite_edit_selection].body.size.y / 2);

		//p.x-=st.Camera.position.x;
		//p.y-=st.Camera.position.y;

		p2.x = 1024 * st.Camera.dimension.x;
		p2.y = 1024 * st.Camera.dimension.y;

		sprintf(str, "%d", st.Current_Map.sprites[meng.sprite_edit_selection].body.size.x);
		String2Data(str, p.x, p.y - 227, 810, 217, 0, 255, 255, 255, 255, ARIAL, p2.x, p2.y, st.Current_Map.sprites[meng.sprite_edit_selection].position.z);

		p.x = st.Current_Map.sprites[meng.sprite_edit_selection].position.x - (st.Current_Map.sprites[meng.sprite_edit_selection].body.size.x / 2);
		p.y = st.Current_Map.sprites[meng.sprite_edit_selection].position.y;

		//p.x-=st.Camera.position.x;
		//p.y-=st.Camera.position.y;

		sprintf(str, "%d", st.Current_Map.sprites[meng.sprite_edit_selection].body.size.y);
		String2Data(str, p.x - 455, p.y, 810, 217, 0, 255, 255, 255, 255, ARIAL, p2.x, p2.y, st.Current_Map.sprites[meng.sprite_edit_selection].position.z);

		//if(strcmp(st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].name,"SOUNDFX")==NULL)
		//{
		for (i = 1; i < st.Current_Map.sprites[meng.sprite_edit_selection].num_tags; i++)
		{
			if (strcmp(st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i], "SNDFX") == NULL)
			{
				if (UIStringButton(465, 4300, "Play sound", ARIAL, 1536, 0, UI_COL_NORMAL, UI_COL_SELECTED) == UI_SEL)
					PlaySound(st.Current_Map.sprites[meng.sprite_edit_selection].tags[i], 0);
			}
			else
				if (strcmp(st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i], "MUSFX") == NULL)
				{
					if (UIStringButton(465, 4300, "Play music", ARIAL, 1536, 0, UI_COL_NORMAL, UI_COL_SELECTED) == UI_SEL)
						PlayMusic(st.Current_Map.sprites[meng.sprite_edit_selection].tags[i], 0);
				}
		}
		//}

		if (UIStringButton(465, 4600, "Stop sound", ARIAL, 1536, 0, UI_COL_NORMAL, UI_COL_SELECTED) == UI_SEL)
		{
			StopAllSounds();
			StopMusic();
		}

			if(st.keys[DELETE_KEY].state)
			{
				st.Current_Map.sprites[meng.sprite_edit_selection].stat=0;

				if(meng.sprite_edit_selection<st.Current_Map.num_sprites-1)
				{
					for(j=meng.sprite_edit_selection;j<st.Current_Map.num_sprites-1;j++)
					{
						memcpy(&st.Current_Map.sprites[j],&st.Current_Map.sprites[j+1],sizeof(_MGMSPRITE));
						memset(&st.Current_Map.sprites[j+1],0,sizeof(_MGMSPRITE));

						st.Current_Map.sprites[j+1].stat=0;
					}
				}
				
				for(k=0;k<meng.z_slot[st.Current_Map.sprites[meng.sprite_edit_selection].position.z];k++)
					if(meng.z_buffer[st.Current_Map.sprites[meng.sprite_edit_selection].position.z][k]==st.Current_Map.num_sprites-1+2000)
						break;

				if(k<meng.z_slot[st.Current_Map.sprites[meng.sprite_edit_selection].position.z]-1)
				{
					for(;k<meng.z_slot[st.Current_Map.sprites[meng.sprite_edit_selection].position.z];k++)
					{
						meng.z_buffer[st.Current_Map.sprites[meng.sprite_edit_selection].position.z][k]=meng.z_buffer[st.Current_Map.sprites[meng.sprite_edit_selection].position.z][k+1];
						meng.z_buffer[st.Current_Map.sprites[meng.sprite_edit_selection].position.z][k+1]=0;
					}
				}
				else
					meng.z_buffer[st.Current_Map.sprites[meng.sprite_edit_selection].position.z][k]=0;
					
				meng.z_slot[st.Current_Map.sprites[meng.sprite_edit_selection].position.z]--;

				st.Current_Map.num_sprites--;
				st.keys[DELETE_KEY].state=0;
			}

			if(UIStringButton(465,4000,"Edit pos",ARIAL,1536,0,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
				meng.command=SPRITE_EDIT_BOX;

			if(meng.command==SPRITE_EDIT_BOX)
			{
				UIData(8192,4608,1820,2730,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,1);

				sprintf(str,"X %d",st.Current_Map.sprites[meng.sprite_edit_selection].position.x);

				if(CheckCollisionMouse(8192,(4608)-681,1720,455,0) && !meng.sub_com)
				{
					StringUIData(str,8192,(4608)-681,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
					if(st.mouse1)
					{
						meng.sub_com=1;
						sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].position.x);
						SDL_StartTextInput();
						st.Text_Input=1;
						st.mouse1=0;
					}
				}
				else
				if(meng.sub_com==1)
				{
					StringUIData(str,8192,(4608)-681,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
					st.Current_Map.sprites[meng.sprite_edit_selection].position.x=atof(st.TextInput);
					if(st.keys[RETURN_KEY].state)
					{
						SDL_StopTextInput();
						st.Text_Input=0;
						meng.sub_com=0;
						st.keys[RETURN_KEY].state=0;
					}
				}
				else
					StringUIData(str,8192,(4608)-681,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

				sprintf(str,"Y %d",st.Current_Map.sprites[meng.sprite_edit_selection].position.y);

				if(CheckCollisionMouse(8192,(4608)-227,1720,455,0) && !meng.sub_com)
				{
					StringUIData(str,8192,(4608)-227,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
					if(st.mouse1)
					{
						meng.sub_com=2;
						sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].position.y);
						SDL_StartTextInput();
						st.Text_Input=1;
						st.mouse1=0;
					}
				}
				else
				if(meng.sub_com==2)
				{
					StringUIData(str,8192,(4608)-227,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
					st.Current_Map.sprites[meng.sprite_edit_selection].position.y=atof(st.TextInput);
					if(st.keys[RETURN_KEY].state)
					{
						SDL_StopTextInput();
						st.Text_Input=0;
						meng.sub_com=0;
						st.keys[RETURN_KEY].state=0;
					}
				}
				else
					StringUIData(str,8192,(4608)-227,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

				sprintf(str,"SX %d",st.Current_Map.sprites[meng.sprite_edit_selection].body.size.x);

				if(CheckCollisionMouse(8192,(4608)+227,1720,455,0) && !meng.sub_com)
				{
					StringUIData(str,8192,(4608)+227,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
					if(st.mouse1 || st.mouse2)
					{
						if(st.mouse1)
							meng.sub_com=3;
						else
						if(st.mouse2)
							meng.sub_com=6;

						asp=(float) st.Current_Map.sprites[meng.sprite_edit_selection].body.size.x/st.Current_Map.sprites[meng.sprite_edit_selection].body.size.y;
						sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].body.size.x);
						SDL_StartTextInput();
						st.Text_Input=1;
						st.mouse1=0;
					}
				}
				else
				if(meng.sub_com==3)
				{
					StringUIData(str,8192,(4608)+227,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
					st.Current_Map.sprites[meng.sprite_edit_selection].body.size.x=atof(st.TextInput);
					if(st.keys[RETURN_KEY].state)
					{
						SDL_StopTextInput();
						st.Text_Input=0;
						meng.sub_com=0;
						st.keys[RETURN_KEY].state=0;
					}
				}
				else
				if(meng.sub_com==6)
				{
					StringUIData(str,8192,(4608)+227,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);

					st.Current_Map.sprites[meng.sprite_edit_selection].body.size.x=atof(st.TextInput);
					
					st.Current_Map.sprites[meng.sprite_edit_selection].body.size.y=(float) st.Current_Map.sprites[meng.sprite_edit_selection].body.size.x/asp;

					if(st.keys[RETURN_KEY].state)
					{
						SDL_StopTextInput();
						st.Text_Input=0;
						meng.sub_com=0;
						st.keys[RETURN_KEY].state=0;
					}
				}
				else
					StringUIData(str,8192,(4608)+227,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

				sprintf(str,"SY %d",st.Current_Map.sprites[meng.sprite_edit_selection].body.size.y);

				if(CheckCollisionMouse(8192,(4608)+681,1720,455,0) && !meng.sub_com)
				{
					StringUIData(str,8192,(4608)+681,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
					if(st.mouse1 || st.mouse2)
					{
						if(st.mouse1)
							meng.sub_com=4;
						else
						if(st.mouse2)
							meng.sub_com=7;

						asp=(float) st.Current_Map.sprites[meng.sprite_edit_selection].body.size.x/st.Current_Map.sprites[meng.sprite_edit_selection].body.size.y;
						sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].body.size.y);
						SDL_StartTextInput();
						st.Text_Input=1;
						st.mouse1=0;
					}
				}
				else
				if(meng.sub_com==4)
				{
					StringUIData(str,8192,(4608)+681,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
					st.Current_Map.sprites[meng.sprite_edit_selection].body.size.y=atof(st.TextInput);
					if(st.keys[RETURN_KEY].state)
					{
						SDL_StopTextInput();
						st.Text_Input=0;
						meng.sub_com=0;
						st.keys[RETURN_KEY].state=0;
					}
				}
				else
				if(meng.sub_com==7)
				{
					StringUIData(str,8192,(4608)+681,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);

					st.Current_Map.sprites[meng.sprite_edit_selection].body.size.y=atof(st.TextInput);
					
					st.Current_Map.sprites[meng.sprite_edit_selection].body.size.x=(float) st.Current_Map.sprites[meng.sprite_edit_selection].body.size.y*asp;

					if(st.keys[RETURN_KEY].state)
					{
						SDL_StopTextInput();
						st.Text_Input=0;
						meng.sub_com=0;
						st.keys[RETURN_KEY].state=0;
					}
				}
				else
					StringUIData(str,8192,(4608)+681,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

				sprintf(str,"Z %d",st.Current_Map.sprites[meng.sprite_edit_selection].position.z);

				if(CheckCollisionMouse(8192,(4608)+1137,1720,455,0) && !meng.sub_com)
				{
					StringUIData(str,8192,(4608)+1137,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
					if(st.mouse1)
					{
						meng.sub_com=5;
						sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].position.z);
						SDL_StartTextInput();
						st.Text_Input=1;
						st.mouse1=0;
					}
				}
				else
				if(meng.sub_com==5)
				{
					StringUIData(str,8192,(4608)+1137,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
					st.Current_Map.sprites[meng.sprite_edit_selection].position.z=atoi(st.TextInput);
					if(st.Current_Map.sprites[meng.sprite_edit_selection].position.z>56) st.Current_Map.sprites[meng.sprite_edit_selection].position.z=56;
					if(st.keys[RETURN_KEY].state)
					{
						SDL_StopTextInput();
						st.Text_Input=0;
						meng.sub_com=0;
						st.keys[RETURN_KEY].state=0;
					}
				}
				else
					StringUIData(str,8192,(4608)+1137,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

				if(st.keys[ESC_KEY].state && !meng.sub_com)
				{
					meng.command=meng.pannel_choice;
					st.keys[ESC_KEY].state=0;
				}
			}


		if(st.Current_Map.sprites[meng.sprite_edit_selection].type_s==MIDGROUND)
		{
			if(CheckCollisionMouse(465,1900,880,220,0))
			{
				StringUIData("Midground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=26;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Midground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(st.Current_Map.sprites[meng.sprite_edit_selection].type_s==FOREGROUND)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Foreground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=26;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Foreground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(st.Current_Map.sprites[meng.sprite_edit_selection].type_s==BACKGROUND1)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Background1",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=26;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background1",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(st.Current_Map.sprites[meng.sprite_edit_selection].type_s==BACKGROUND2)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Background2",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=26;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background2",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(st.Current_Map.sprites[meng.sprite_edit_selection].type_s==BACKGROUND3)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Background3",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=26;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background3",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.command==EDIT_SPRITE_TYPE_S)
		{
			UIData(1365,1715,910,1210,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,0);

			if(CheckCollisionMouse(1365,1715,810,227,0))
			{
				StringUIData("Background1",1365,1715,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					st.Current_Map.sprites[meng.sprite_edit_selection].type_s=BACKGROUND1;
					st.Current_Map.sprites[meng.sprite_edit_selection].position.z=32;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background1",1365,1715,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckCollisionMouse(1365,1465,810,227,0))
			{
				StringUIData("Background2",1365,1465,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{

					st.Current_Map.sprites[meng.sprite_edit_selection].type_s=BACKGROUND2;
					st.Current_Map.sprites[meng.sprite_edit_selection].position.z=40;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background2",1365,1465,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckCollisionMouse(1365,1215,810,227,0))
			{
				StringUIData("Background3",1365,1215,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					st.Current_Map.sprites[meng.sprite_edit_selection].type_s=BACKGROUND3;
					st.Current_Map.sprites[meng.sprite_edit_selection].position.z=48;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background3",1365,1215,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckCollisionMouse(1365,1920,810,227,0))
			{
				StringUIData("Midground",1365,1920,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					st.Current_Map.sprites[meng.sprite_edit_selection].type_s=MIDGROUND;
					st.Current_Map.sprites[meng.sprite_edit_selection].position.z=24;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Midground",1365,1920,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckCollisionMouse(1365,2170,810,227,0))
			{
				StringUIData("Foreground",1365,2170,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					st.Current_Map.sprites[meng.sprite_edit_selection].type_s=FOREGROUND;
					st.Current_Map.sprites[meng.sprite_edit_selection].position.z=16;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Foreground",1365,2170,810,227,0,255,255,255,255,ARIAL,0,0,0);
		}

		sprintf(str,"color");

		if(CheckCollisionMouse(465,2445,810,217,0))
		{
			StringUIData(str,465,2445,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

			if(st.mouse1)
			{
				meng.command=RGB_SPRITE;
				st.mouse1=0;
			}
		}
		else
			StringUIData(str,465,2445,810,217,0, 255, 255, 255,255,ARIAL,2048,2048,0);

		if(meng.command==RGB_SPRITE)
		{
			UIData(8192,3072,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			Sys_ColorPicker(&st.Current_Map.sprites[meng.sprite_edit_selection].color.r,&st.Current_Map.sprites[meng.sprite_edit_selection].color.g,&st.Current_Map.sprites[meng.sprite_edit_selection].color.b);

			StringUIData(str,465,2445,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

			sprintf(str,"R %d",st.Current_Map.sprites[meng.sprite_edit_selection].color.r);

			if(meng.sub_com!=1)
			{
				if(CheckCollisionMouse(8192,2048+455,2048,227,0))
				{
					StringUIData(str,8192,2048+455,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].color.r);
						StartText();
						st.mouse1=0;
						meng.sub_com=1;
					}
				}
				else
					StringUIData(str,8192,2048+455,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				StringUIData(str,8192,2048+455,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				st.Current_Map.sprites[meng.sprite_edit_selection].color.r=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"G %d",st.Current_Map.sprites[meng.sprite_edit_selection].color.g);

			if(meng.sub_com!=2)
			{
				if(CheckCollisionMouse(8192,2048+810,2048,227,0))
				{
					StringUIData(str,8192,2048+810,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].color.g);
						StartText();
						st.mouse1=0;
						meng.sub_com=2;
					}
				}
				else
					StringUIData(str,8192,2048+810,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				StringUIData(str,8192,2048+810,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				st.Current_Map.sprites[meng.sprite_edit_selection].color.g=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"B %d",st.Current_Map.sprites[meng.sprite_edit_selection].color.b);

			if(meng.sub_com!=3)
			{
				if(CheckCollisionMouse(8192,2048+1265,2048,455,0))
				{
					StringUIData(str,8192,2048+1265,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].color.b);
						StartText();
						st.mouse1=0;
						meng.sub_com=3;
					}
				}
				else
					StringUIData(str,8192,2048+1265,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==3)
			{
				StringUIData(str,8192,2048+1265,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				st.Current_Map.sprites[meng.sprite_edit_selection].color.b=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"A %d",st.Current_Map.sprites[meng.sprite_edit_selection].color.a);

			if(meng.sub_com!=4)
			{
				if(CheckCollisionMouse(8192,2048+1720,2048,455,0))
				{
					StringUIData(str,8192,2048+1720,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].color.a);
						StartText();
						st.mouse1=0;
						meng.sub_com=4;
					}
				}
				else
					StringUIData(str,8192,2048+1720,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==4)
			{
				StringUIData(str,8192,2048+1720,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				st.Current_Map.sprites[meng.sprite_edit_selection].color.a=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			if(st.keys[RETURN_KEY].state)
			{
				meng.command=meng.pannel_choice;
				st.keys[RETURN_KEY].state=0;
			}
		}


		if(CheckCollisionMouse(455,3135,810,217,0))
		{
			StringUIData("Tags",465,3135,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

			if(st.mouse1)
			{
				meng.command=SPRITE_TAG;
				st.mouse1=0;
			}
		}
		else
			StringUIData("Tags",465,3135,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
		
		if(meng.command==SPRITE_TAG)
		{

			UIData(8192,4608,9216,6144,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,7);

			for(i=1, yt=4608-2560;(i-1)<st.Current_Map.sprites[meng.sprite_edit_selection].num_tags;i++, yt+=512)
			{
				j=strlen(st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i-1]);

				if(st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i-1][j-1]=='S' && st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i-1][j-2]=='_')
					sprintf(str,"%s %s",st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i-1],st.Current_Map.sprites[meng.sprite_edit_selection].tags_str[i-1]);
				else
					sprintf(str,"%s %d",st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i-1],st.Current_Map.sprites[meng.sprite_edit_selection].tags[i-1]);

				if(meng.sub_com!=i && meng.sub_com<100)
				{
					if(CheckCollisionMouse(8192,yt,2048,455,0))
					{
						StringUIData(str,8192,yt,0,0,0,255,128,32,255,ARIAL,2048,2048,6);

						if(st.mouse1)
						{
							if(strcmp(st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i-1],"SNDFX")==NULL)
							{
								st.mouse1=0;
								meng.sub_com=i+200;
							}
							else
							if(strcmp(st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i - 1], "MUSFX") == NULL)
							{
								st.mouse1 = 0;
								meng.sub_com = i + 300;
							}
							else
							{
								if(st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i-1][j-1]=='S' && 
									st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i-1][j-2]=='_')
									sprintf(st.TextInput,"%s",st.Current_Map.sprites[meng.sprite_edit_selection].tags_str[i-1]);
								else
									sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].tags[i-1]);

								StartText();
								st.mouse1=0;
								meng.sub_com=i;
							}
						}

						if(st.mouse2 && st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i-1][j-1]=='S' && 
							st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i-1][j-2]=='_')
						{
							meng.sub_com=i+100;
							SetDirContent(NULL);
							st.mouse2=0;
						}
					}
					else
						StringUIData(str,8192,yt,0,0,0,255,255,255,255,ARIAL,2048,2048,6);
				}
				else
				if(meng.sub_com==i)
				{
					StringUIData(str,8192,yt,0,0,0,255,32,32,255,ARIAL,2048,2048,6);

					if(st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i-1][j-1]=='S' && st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i-1][j-2]=='_')
						strcpy(st.Current_Map.sprites[meng.sprite_edit_selection].tags_str[i-1],st.TextInput);
					else
						st.Current_Map.sprites[meng.sprite_edit_selection].tags[i-1]=atoi(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.sub_com=0;
					}
				}
				else
				if(meng.sub_com==i+100)
				{
					if(UISelectFile(filen))
					{
						strcpy(st.Current_Map.sprites[meng.sprite_edit_selection].tags_str[i-1],filen);
						meng.sub_com=0;
					}

					if(st.keys[RETURN_KEY].state)
					{
						st.keys[RETURN_KEY].state=0;
						meng.sub_com=0;
					}
				}
				else
				if(meng.sub_com==i+200)
				{
					if((st.Current_Map.sprites[meng.sprite_edit_selection].tags[i-1]=UIMakeList(meng.soundlist,st.num_sounds))!=-1)
						meng.sub_com=0;

					if(st.keys[RETURN_KEY].state)
					{
						st.keys[RETURN_KEY].state=0;
						meng.sub_com=0;
					}
				}
				else
				if(meng.sub_com == i + 300)
				{
					if((st.Current_Map.sprites[meng.sprite_edit_selection].tags[i - 1] = UIMakeList(meng.musiclist, st.num_musics)) != -1)
						meng.sub_com = 0;

					if(st.keys[RETURN_KEY].state)
					{
						st.keys[RETURN_KEY].state = 0;
						meng.sub_com = 0;
					}
				}
			}

			if(st.keys[ESC_KEY].state)
			{
				meng.command=meng.pannel_choice;
				st.keys[ESC_KEY].state=0;
			}
		}

		sprintf(str,"Health %d",st.Current_Map.sprites[meng.sprite_edit_selection].health);

		if(CheckCollisionMouse(465,3435,810,217,0))
		{
			StringUIData(str,465,3435,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1 && !StartText())
			{
				meng.command=SPRITE_HEALTH;
				sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].health);
				st.mouse1=0;
			}
		}
		else
			StringUIData(str,465,3435,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==SPRITE_HEALTH)
		{
			StringUIData(str,465,3435,810,217,0,255,32,32,255,ARIAL,0,0,0);
			st.Current_Map.sprites[meng.sprite_edit_selection].health=atoi(st.TextInput);

			if(st.keys[ESC_KEY].state)
			{
				StopText();
				meng.command=meng.pannel_choice;
				st.keys[ESC_KEY].state=0;
			}
		}



		sprintf(str,"Physics");

		if(CheckCollisionMouse(465,3690,810,217,0))
		{
			StringUIData(str,465,3690,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.command=SPRITE_PHY;
				st.mouse1=0;
			}
		}
		else
			StringUIData(str,465,3690,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==SPRITE_PHY)
		{
			DrawHud(8192, 4608, 2120,4608,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,7);

			sprintf(str,"Physics? %d",st.Current_Map.sprites[meng.sprite_edit_selection].body.physics_on);

			if(CheckCollisionMouse(8192, 4608-1420,810,405,0))
			{
				StringUIData(str,8192, 4608-1420,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

				if(st.mouse1)
				{
					if(meng.spr.body.physics_on==0) st.Current_Map.sprites[meng.sprite_edit_selection].body.physics_on=1;
					else st.Current_Map.sprites[meng.sprite_edit_selection].body.physics_on=0;

					st.mouse1=0;
				}
			}
			else
				StringUIData(str,8192, 4608-1420,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

			sprintf(str,"Mass %.2f",st.Current_Map.sprites[meng.sprite_edit_selection].body.mass);

			if(CheckCollisionMouse(8192, 4608-710,810,405,0))
			{
				StringUIData(str,8192, 4608-710,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

				if(st.mouse1 && !StartText())
				{
					if(meng.spr.body.mass==0)
						sprintf(st.TextInput,"%.0f",st.Current_Map.sprites[meng.sprite_edit_selection].body.mass);
					else
						sprintf(st.TextInput,"%.2f",st.Current_Map.sprites[meng.sprite_edit_selection].body.mass);
					meng.sub_com=10;
					st.mouse1=0;
				}
			}
			else
				StringUIData(str,8192, 4608-710,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

			if(meng.sub_com==10)
			{
				StringUIData(str,8192, 4608-1420,0,0,0,255,32,32,255,ARIAL,512*4,512*4,0);
				st.Current_Map.sprites[meng.sprite_edit_selection].body.mass=atof(st.TextInput);
				if(st.keys[ESC_KEY].state)
				{
					StopText();
					meng.sub_com=0;
					st.keys[ESC_KEY].state=0;
				}
			}

			sprintf(str,"Flammable? %d",st.Current_Map.sprites[meng.sprite_edit_selection].body.flamable);

			if(CheckCollisionMouse(8192, 4608,810,405,0))
			{
				StringUIData(str,8192, 4608,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

				if(st.mouse1)
				{
					if(meng.spr.body.flamable==0) st.Current_Map.sprites[meng.sprite_edit_selection].body.flamable=1;
					else st.Current_Map.sprites[meng.sprite_edit_selection].body.flamable=0;

					st.mouse1=0;
				}
			}
			else
				StringUIData(str,8192, 4608,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

			sprintf(str,"Explosive? %d",st.Current_Map.sprites[meng.sprite_edit_selection].body.explosive);

			if(CheckCollisionMouse(8192, 4608+710,810,405,0))
			{
				StringUIData(str,8192, 4608+710,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

				if(st.mouse1)
				{
					if(meng.spr.body.explosive==0) st.Current_Map.sprites[meng.sprite_edit_selection].body.explosive=1;
					else st.Current_Map.sprites[meng.sprite_edit_selection].body.explosive=0;

					st.mouse1=0;
				}
			}
			else
				StringUIData(str,8192, 4608+710,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

			if(st.keys[ESC_KEY].state)
			{
				meng.command=meng.pannel_choice;
				st.keys[ESC_KEY].state=0;
			}
		}
	}

	//OBJ editing

	if(meng.command2==EDIT_OBJ)
	{
		i=meng.com_id;
		meng.obj2.amblight=st.Current_Map.obj[i].amblight;
		meng.obj2.texpan=st.Current_Map.obj[i].texpan;
		meng.obj2.texsize=st.Current_Map.obj[i].texsize;
		meng.obj2.type=st.Current_Map.obj[i].type;
		meng.obj2.color=st.Current_Map.obj[i].color;
		meng.obj2.flag=st.Current_Map.obj[i].flag;

		p.x=st.Current_Map.obj[i].position.x;
		p.y=st.Current_Map.obj[i].position.y-(st.Current_Map.obj[i].size.y/2);
		//WTS(&p.x,&p.y);

		//p.x-=st.Camera.position.x;
		//p.y-=st.Camera.position.y;

		//p.x*=st.Camera.dimension.x;
		//p.y*=st.Camera.dimension.y;

		p2.x=1024*st.Camera.dimension.x;
		p2.y=1024*st.Camera.dimension.y;

		sprintf(str,"%d",st.Current_Map.obj[i].size.x);
		String2Data(str,p.x,p.y-227,810,217,0,255,255,255,255,ARIAL,p2.x,p2.y,st.Current_Map.obj[i].position.z);

		p.x=st.Current_Map.obj[i].position.x-(st.Current_Map.obj[i].size.x/2);
		p.y=st.Current_Map.obj[i].position.y;

		//p.x-=st.Camera.position.x;
		//p.y-=st.Camera.position.y;

		//p.x*=st.Camera.dimension.x;
		//p.y*=st.Camera.dimension.y;

		sprintf(str,"%d",st.Current_Map.obj[i].size.y);

		if(meng.obj2.flag & 1)
			String2Data(str,st.Current_Map.obj[i].position.x,p.y+(st.Current_Map.obj[i].size.y/2)+227,810,217,0,255,255,255,255,ARIAL,p2.x,p2.y,st.Current_Map.obj[i].position.z);
		else
			String2Data(str,p.x-455,p.y,810,217,0,255,255,255,255,ARIAL,p2.x,p2.y,st.Current_Map.obj[i].position.z);

		if(UIStringButton(465,4000,"Edit pos.",ARIAL,1536,0,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
			meng.command=OBJ_EDIT_BOX;
		

		if(st.keys[DELETE_KEY].state)
		{
			st.Current_Map.obj[i].type=BLANK;

			if(i<st.Current_Map.num_obj-1)
			{
				for(j=i;j<st.Current_Map.num_obj-1;j++)
				{
					memcpy(&st.Current_Map.obj[j],&st.Current_Map.obj[j+1],sizeof(_MGMOBJ));
					st.Current_Map.obj[j+1].type=BLANK;
				}
			}

			for(k=0;k<meng.z_slot[st.Current_Map.obj[i].position.z];k++)
				if(meng.z_buffer[st.Current_Map.obj[i].position.z][k]==st.Current_Map.num_obj-1)
					break;

			if(k<meng.z_slot[st.Current_Map.obj[i].position.z]-1)
			{
				for(;k<meng.z_slot[st.Current_Map.obj[i].position.z];k++)
				{
					meng.z_buffer[st.Current_Map.obj[i].position.z][k]=meng.z_buffer[st.Current_Map.obj[i].position.z][k+1];
					meng.z_buffer[st.Current_Map.obj[i].position.z][k+1]=0;
				}
			}
			else
				meng.z_buffer[st.Current_Map.obj[i].position.z][k]=0;

			meng.z_slot[st.Current_Map.obj[i].position.z]--;
			
			meng.obj2.amblight=st.Current_Map.obj[i].amblight;
			meng.obj2.texpan=st.Current_Map.obj[i].texpan;
			meng.obj2.texsize=st.Current_Map.obj[i].texsize;
			meng.obj2.type=st.Current_Map.obj[i].type;
			meng.obj2.color=st.Current_Map.obj[i].color;
			meng.obj2.flag=st.Current_Map.obj[i].flag;
			
			st.Current_Map.num_obj--;

			if(!st.Current_Map.num_obj)
				meng.command2=0;

			st.keys[DELETE_KEY].state=0;
		}

		if(meng.obj2.type==BACKGROUND1 || meng.obj2.type==BACKGROUND2 || meng.obj2.type==FOREGROUND)
		{
			if(!(meng.obj2.flag & 1))
			{
				if(UIStringButton(465,3640+315+315,"Tex. Mov [ ]",ARIAL,1024,0,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
					st.Current_Map.obj[i].flag |=1;
			}
			else
			if(meng.obj2.flag & 1)
			{
				if(UIStringButton(465,3640+315+315,"Tex. Mov [X]",ARIAL,1024,0,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
					st.Current_Map.obj[i].flag-=1;
			}
		}

		if(meng.obj2.type==MIDGROUND)
		{
			if(!(meng.obj2.flag & OBJF_ANIMATED_TEXTURE_MOV_CAM))
			{
				if(UIStringButton(465,3640+315+315+315,"ATMC [ ]",ARIAL,1024,0,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
					st.Current_Map.obj[i].flag |=2;
			}
			else
			if(meng.obj2.flag & OBJF_ANIMATED_TEXTURE_MOV_CAM)
			{
				if(UIStringButton(465,3640+315+315+315,"ATMC [X]",ARIAL,1024,0,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
					st.Current_Map.obj[i].flag-=2;
			}
		}

		if(meng.command==OBJ_EDIT_BOX)
		{
			UIData(8192,4608,1820,2730,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,0);

			sprintf(str,"X %d",st.Current_Map.obj[i].position.x);

			if(CheckCollisionMouse(8192,(4608)-681,1720,455,0) && !meng.sub_com)
			{
				StringUIData(str,8192,(4608)-681,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
				if(st.mouse1)
				{
					meng.sub_com=1;
					sprintf(st.TextInput,"%d",st.Current_Map.obj[i].position.x);
					SDL_StartTextInput();
					st.Text_Input=1;
					st.mouse1=0;
				}
			}
			else
			if(meng.sub_com==1)
			{
				StringUIData(str,8192,(4608)-681,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
				st.Current_Map.obj[i].position.x=atof(st.TextInput);
				if(st.keys[RETURN_KEY].state)
				{
					SDL_StopTextInput();
					st.Text_Input=0;
					meng.sub_com=0;
					st.keys[RETURN_KEY].state=0;
				}
			}
			else
				StringUIData(str,8192,(4608)-681,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

			sprintf(str,"Y %d",st.Current_Map.obj[i].position.y);

			if(CheckCollisionMouse(8192,(4608)-227,1720,455,0) && !meng.sub_com)
			{
				StringUIData(str,8192,(4608)-227,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
				if(st.mouse1)
				{
					meng.sub_com=2;
					sprintf(st.TextInput,"%d",st.Current_Map.obj[i].position.y);
					SDL_StartTextInput();
					st.Text_Input=1;
					st.mouse1=0;
				}
			}
			else
			if(meng.sub_com==2)
			{
				StringUIData(str,8192,(4608)-227,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
				st.Current_Map.obj[i].position.y=atof(st.TextInput);
				if(st.keys[RETURN_KEY].state)
				{
					SDL_StopTextInput();
					st.Text_Input=0;
					meng.sub_com=0;
					st.keys[RETURN_KEY].state=0;
				}
			}
			else
				StringUIData(str,8192,(4608)-227,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

			sprintf(str,"SX %d",st.Current_Map.obj[i].size.x);

			if(CheckCollisionMouse(8192,(4608)+227,1720,455,0) && !meng.sub_com)
			{
				StringUIData(str,8192,(4608)+227,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
				if(st.mouse1 || st.mouse2)
				{
					if(st.mouse1)
						meng.sub_com=3;
					else
					if(st.mouse2)
						meng.sub_com=6;

					asp=(float) st.Current_Map.obj[i].size.x/st.Current_Map.obj[i].size.y;

					sprintf(st.TextInput,"%d",st.Current_Map.obj[i].size.x);
					SDL_StartTextInput();
					st.Text_Input=1;
					st.mouse1=0;
				}
			}
			else
			if(meng.sub_com==3)
			{
				StringUIData(str,8192,(4608)+227,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
				st.Current_Map.obj[i].size.x=atof(st.TextInput);
				if(st.keys[RETURN_KEY].state)
				{
					SDL_StopTextInput();
					st.Text_Input=0;
					meng.sub_com=0;
					st.keys[RETURN_KEY].state=0;
				}
			}
			else
			if(meng.sub_com==6)
			{
				StringUIData(str,8192,(4608)+227,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);

				st.Current_Map.obj[i].size.x=atof(st.TextInput);
				//asp=(float) st.Current_Map.obj[i].size.x/st.Current_Map.obj[i].size.y;
				st.Current_Map.obj[i].size.y=(float) st.Current_Map.obj[i].size.x/asp;

				if(st.keys[RETURN_KEY].state)
				{
					SDL_StopTextInput();
					st.Text_Input=0;
					meng.sub_com=0;
					st.keys[RETURN_KEY].state=0;
				}
			}
			else
				StringUIData(str,8192,(4608)+227,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

			sprintf(str,"SY %d",st.Current_Map.obj[i].size.y);

			if(CheckCollisionMouse(8192,(4608)+681,1720,455,0) && !meng.sub_com)
			{
				StringUIData(str,8192,(4608)+681,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
				if(st.mouse1 || st.mouse2)
				{
					if(st.mouse1)
						meng.sub_com=4;
					else
					if(st.mouse2)
						meng.sub_com=7;

					asp=(float) st.Current_Map.obj[i].size.x/st.Current_Map.obj[i].size.y;

					sprintf(st.TextInput,"%d",st.Current_Map.obj[i].size.y);
					SDL_StartTextInput();
					st.Text_Input=1;
					st.mouse1=0;
				}
			}
			else
			if(meng.sub_com==4)
			{
				StringUIData(str,8192,(4608)+681,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
				st.Current_Map.obj[i].size.y=atof(st.TextInput);
				if(st.keys[RETURN_KEY].state)
				{
					SDL_StopTextInput();
					st.Text_Input=0;
					meng.sub_com=0;
					st.keys[RETURN_KEY].state=0;
				}
			}
			else
			if(meng.sub_com==7)
			{
				StringUIData(str,8192,(4608)+681,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);

				st.Current_Map.obj[i].size.y=atof(st.TextInput);

				st.Current_Map.obj[i].size.x=(float) st.Current_Map.obj[i].size.y*asp;
				if(st.keys[RETURN_KEY].state)
				{
					SDL_StopTextInput();
					st.Text_Input=0;
					meng.sub_com=0;
					st.keys[RETURN_KEY].state=0;
				}
			}
			else
				StringUIData(str,8192,(4608)+681,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

			sprintf(str,"Z %d",st.Current_Map.obj[i].position.z);

			if(CheckCollisionMouse(8192,(4608)+1137,1720,455,0) && !meng.sub_com)
			{
				StringUIData(str,8192,(4608)+1137,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
				if(st.mouse1)
				{
					meng.sub_com=5;
					sprintf(st.TextInput,"%d",st.Current_Map.obj[i].position.z);
					SDL_StartTextInput();
					st.Text_Input=1;
					st.mouse1=0;
				}
			}
			else
			if(meng.sub_com==5)
			{
				StringUIData(str,8192,(4608)+1137,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
				st.Current_Map.obj[i].position.z=atoi(st.TextInput);
				if(st.Current_Map.obj[i].position.z>56) st.Current_Map.obj[i].position.z=56;
				if(st.keys[RETURN_KEY].state)
				{
					SDL_StopTextInput();
					st.Text_Input=0;
					meng.sub_com=0;
					st.keys[RETURN_KEY].state=0;
				}
			}
			else
				StringUIData(str,8192,(4608)+1137,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

			if(st.keys[ESC_KEY].state && !meng.sub_com)
			{
				meng.command=meng.pannel_choice;
				st.keys[ESC_KEY].state=0;
			}
		}

		if(meng.obj2.type==MIDGROUND)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Midground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=14;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Midground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj2.type==FOREGROUND)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Foreground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=14;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Foreground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj2.type==BACKGROUND1)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Background1",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=14;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background1",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj2.type==BACKGROUND2)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Background2",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=14;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background2",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj2.type==BACKGROUND3)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Background3",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=14;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background3",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		sprintf(str,"color");

		if(CheckCollisionMouse(465,2445,810,217,0))
		{
			StringUIData(str,465,2445,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

			if(st.mouse1)
			{
				meng.command=RGB_OBJ;
				st.mouse1=0;
			}
		}
		else
			StringUIData(str,465,2445,810,217,0, 255, 255, 255,255,ARIAL,2048,2048,0);

		if(meng.command==RGB_OBJ)
		{
			UIData(8192,3072,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			StringUIData(str,465,2445,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

			Sys_ColorPicker(&meng.obj2.color.r,&meng.obj2.color.g,&meng.obj2.color.b);

			sprintf(str,"R %d",meng.obj2.color.r);

			if(meng.sub_com!=1)
			{
				if(CheckCollisionMouse(8192,2048+455,2048,227,0))
				{
					StringUIData(str,8192,2048+455,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj2.color.r);
						StartText();
						st.mouse1=0;
						meng.sub_com=1;
					}
				}
				else
					StringUIData(str,8192,2048+455,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				StringUIData(str,8192,2048+455,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj2.color.r=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"G %d",meng.obj2.color.g);

			if(meng.sub_com!=2)
			{
				if(CheckCollisionMouse(8192,2048+810,2048,227,0))
				{
					StringUIData(str,8192,2048+810,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj2.color.g);
						StartText();
						st.mouse1=0;
						meng.sub_com=2;
					}
				}
				else
					StringUIData(str,8192,2048+810,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				StringUIData(str,8192,2048+810,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj2.color.g=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"B %d",meng.obj2.color.b);

			if(meng.sub_com!=3)
			{
				if(CheckCollisionMouse(8192,2048+1265,2048,455,0))
				{
					StringUIData(str,8192,2048+1265,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj2.color.b);
						StartText();
						st.mouse1=0;
						meng.sub_com=3;
					}
				}
				else
					StringUIData(str,8192,2048+1265,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==3)
			{
				StringUIData(str,8192,2048+1265,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj2.color.b=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"A %d",meng.obj2.color.a);

			if(meng.sub_com!=4)
			{
				if(CheckCollisionMouse(8192,2048+1720,2048,455,0))
				{
					StringUIData(str,8192,2048+1720,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj2.color.a);
						StartText();
						st.mouse1=0;
						meng.sub_com=4;
					}
				}
				else
					StringUIData(str,8192,2048+1720,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==4)
			{
				StringUIData(str,8192,2048+1720,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj2.color.a=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			if(st.keys[RETURN_KEY].state)
			{
				meng.command=meng.pannel_choice;
				st.keys[RETURN_KEY].state=0;
			}
		}

		sprintf(str,"Light %.2f",meng.obj2.amblight);

		if(CheckCollisionMouse(465,2887,810,217,0))
		{
			StringUIData(str,465,2887,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.command=OBJ_AMBL;
				st.mouse1=0;
			}
		}
		else
			StringUIData(str,465,2887,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==OBJ_AMBL)
		{
			StringUIData(str,465,2887,810,217,0,255,32,32,255,ARIAL,0,0,0);

			if(st.keys[RIGHT_KEY].state)
			{
				meng.obj2.amblight+=0.01;
				st.keys[RIGHT_KEY].state=0;
			}

			if(st.keys[LEFT_KEY].state)
			{
				meng.obj2.amblight-=0.01;
				st.keys[LEFT_KEY].state=0;
			}

			if(!CheckCollisionMouse(465,2887,810,217,0) && st.mouse1)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}

		sprintf(str,"Tex.Size");

		if(CheckCollisionMouse(465,3315,810,217,0))
		{
			StringUIData(str,465,3315,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1 && meng.command!=TEX_PAN_OBJ)
			{
				meng.command=TEX_SIZE_OBJ;
				st.mouse1=0;
			}
		}
		else
			StringUIData(str,465,3315,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==TEX_SIZE_OBJ)
		{
			UIData(8192,4608,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			sprintf(str,"X %d",meng.obj2.texsize.x);

			if(meng.sub_com!=1)
			{
				if(CheckCollisionMouse(8192,3641,2048,910,0))
				{
					StringUIData(str,8192,3641,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						st.mouse1=0;
						sprintf(st.TextInput,"%d",meng.obj2.texsize.x);
						st.Text_Input=1;
						SDL_StartTextInput();
						meng.sub_com=1;
					}
				}
				else
					StringUIData(str,8192,3641,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				StringUIData(str,8192,3641,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj2.texsize.x=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					SDL_StopTextInput();
					st.Text_Input=0;
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"Y %d",meng.obj2.texsize.y);

			if(meng.sub_com!=2)
			{
				if(CheckCollisionMouse(8192,4551,2048,910,0))
				{
					StringUIData(str,8192,4551,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						st.mouse1=0;
						sprintf(st.TextInput,"%d",meng.obj2.texsize.y);
						st.Text_Input=1;
						SDL_StartTextInput();
						meng.sub_com=2;
					}
				}
				else
					StringUIData(str,8192,4551,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				StringUIData(str,8192,4551,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj2.texsize.y=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					SDL_StopTextInput();
					st.Text_Input=0;
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}
			

			if(st.keys[UP_KEY].state)
			{
				meng.obj2.texsize.y+=128;
				st.keys[UP_KEY].state=0;
			}

			if(st.keys[DOWN_KEY].state)
			{
				meng.obj2.texsize.y-=128;
				st.keys[DOWN_KEY].state=0;
			}

			if(st.keys[RIGHT_KEY].state)
			{
				meng.obj2.texsize.x+=128;
				st.keys[RIGHT_KEY].state=0;
			}

			if(st.keys[LEFT_KEY].state)
			{
				meng.obj2.texsize.x-=128;
				st.keys[LEFT_KEY].state=0;
			}

			if(!CheckCollisionMouse(465,3315,810,217,0) && st.mouse1 && meng.sub_com==0)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}

		sprintf(str,"Tex.Pan");

		if(CheckCollisionMouse(465,3640,810,217,0))
		{
			StringUIData(str,465,3640,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1 && meng.command!=TEX_SIZE_OBJ)
			{
				meng.command=TEX_PAN_OBJ;
				st.mouse1=0;
			}
		}
		else
			StringUIData(str,465,3640,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==TEX_PAN_OBJ)
		{
			UIData(8192,4608,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			sprintf(str,"X %d",meng.obj2.texpan.x);

			if(meng.sub_com!=1)
			{
				if(CheckCollisionMouse(8192,3641,2048,910,0))
				{
					StringUIData(str,8192,3641,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						st.mouse1=0;
						sprintf(st.TextInput,"%d",meng.obj2.texpan.x);
						st.Text_Input=1;
						SDL_StartTextInput();
						meng.sub_com=1;
					}
				}
				else
					StringUIData(str,8192,3641,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				StringUIData(str,8192,3641,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj2.texpan.x=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					SDL_StopTextInput();
					st.Text_Input=0;
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"Y %d",meng.obj2.texpan.y);

			if(meng.sub_com!=2)
			{
				if(CheckCollisionMouse(8192,4551,2048,910,0))
				{
					StringUIData(str,8192,4551,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						st.mouse1=0;
						sprintf(st.TextInput,"%d",meng.obj2.texpan.y);
						st.Text_Input=1;
						SDL_StartTextInput();
						meng.sub_com=2;
					}
				}
				else
					StringUIData(str,8192,4551,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				StringUIData(str,8192,4551,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj2.texpan.y=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					SDL_StopTextInput();
					st.Text_Input=0;
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}
			

			if(st.keys[UP_KEY].state)
			{
				meng.obj2.texpan.y+=128;
				st.keys[UP_KEY].state=0;
			}

			if(st.keys[DOWN_KEY].state)
			{
				meng.obj2.texpan.y-=128;
				st.keys[DOWN_KEY].state=0;
			}

			if(st.keys[RIGHT_KEY].state)
			{
				meng.obj2.texpan.x+=128;
				st.keys[RIGHT_KEY].state=0;
			}

			if(st.keys[LEFT_KEY].state)
			{
				meng.obj2.texpan.x-=128;
				st.keys[LEFT_KEY].state=0;
			}

			if(!CheckCollisionMouse(465,3315,810,217,0) && st.mouse1 && meng.sub_com==0)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}

		if(meng.command!=TEX_SIZE_OBJ && meng.command!=TEX_PAN_OBJ && meng.command!=OBJ_AMBL && meng.command!=RGB_OBJ && meng.command!=OBJ_EDIT_BOX)
		{
							if(st.keys[UP_KEY].state)
							{
								st.Current_Map.obj[meng.com_id].size.y+=1165;
								st.keys[UP_KEY].state=0;
							}

							if(st.keys[DOWN_KEY].state)
							{
								st.Current_Map.obj[meng.com_id].size.y-=1165;
								st.keys[DOWN_KEY].state=0;
							}

							if(st.keys[RIGHT_KEY].state)
							{
								st.Current_Map.obj[meng.com_id].size.x+=1165;
								st.keys[RIGHT_KEY].state=0;
							}

							if(st.keys[LEFT_KEY].state)
							{
								st.Current_Map.obj[meng.com_id].size.x-=1165;
								st.keys[LEFT_KEY].state=0;
							}

							if(st.mouse_wheel>0 && st.mouse2)
							{
								st.Current_Map.obj[meng.com_id].size.x+=2330;
								st.Current_Map.obj[meng.com_id].size.y+=2330;
								st.mouse_wheel=0;
							}

							if(st.mouse_wheel<0 && st.mouse2)
							{
								st.Current_Map.obj[meng.com_id].size.x-=2330;
								st.Current_Map.obj[meng.com_id].size.y-=2330;
								st.mouse_wheel=0;
							}
		}

		if(st.Current_Map.obj[i].type!=BLANK)
		{
			st.Current_Map.obj[i].amblight=meng.obj2.amblight;
			st.Current_Map.obj[i].texpan=meng.obj2.texpan;
			st.Current_Map.obj[i].texsize=meng.obj2.texsize;
			st.Current_Map.obj[i].color=meng.obj2.color;
			st.Current_Map.obj[i].type=meng.obj2.type;
		}
	}

	if(meng.pannel_choice==ADD_OBJ)
	{
		UIData(247,681,435,435,0,128,32,32,0,0,32768,32768,mgg_sys[0].frames[1],255,0);

		if(meng.obj.type==MIDGROUND)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Midground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=8;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Midground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj.type==FOREGROUND)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Foreground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=8;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Foreground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj.type==BACKGROUND1)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Background1",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=8;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background1",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj.type==BACKGROUND2)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Background2",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=8;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background2",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj.type==BACKGROUND3)
		{
			if(CheckCollisionMouse(465,2010,810,217,0))
			{
				StringUIData("Background3",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=8;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Background3",465,2010,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
		}

		sprintf(str,"color");

		if(CheckCollisionMouse(465,2445,810,217,0))
		{
			StringUIData(str,465,2445,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

			if(st.mouse1)
			{
				meng.command=RGB_OBJ;
				st.mouse1=0;
			}
		}
		else
			StringUIData(str,465,2445,810,217,0, 255, 255, 255,255,ARIAL,2048,2048,0);

		if(meng.command==RGB_OBJ)
		{
			UIData(8192,3072,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			StringUIData(str,465,2445,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

			Sys_ColorPicker(&meng.obj.color.r,&meng.obj.color.g,&meng.obj.color.b);

			sprintf(str,"R %d",meng.obj.color.r);

			if(meng.sub_com!=1)
			{
				if(CheckCollisionMouse(8192,2048+455,2048,227,0))
				{
					StringUIData(str,8192,2048+455,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj.color.r);
						StartText();
						st.mouse1=0;
						meng.sub_com=1;
					}
				}
				else
					StringUIData(str,8192,2048+455,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				StringUIData(str,8192,2048+455,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj.color.r=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"G %d",meng.obj.color.g);

			if(meng.sub_com!=2)
			{
				if(CheckCollisionMouse(8192,2048+810,2048,227,0))
				{
					StringUIData(str,8192,2048+810,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj.color.g);
						StartText();
						st.mouse1=0;
						meng.sub_com=2;
					}
				}
				else
					StringUIData(str,8192,2048+810,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				StringUIData(str,8192,2048+810,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj.color.g=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"B %d",meng.obj.color.b);

			if(meng.sub_com!=3)
			{
				if(CheckCollisionMouse(8192,2048+1265,2048,455,0))
				{
					StringUIData(str,8192,2048+1265,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj.color.b);
						StartText();
						st.mouse1=0;
						meng.sub_com=3;
					}
				}
				else
					StringUIData(str,8192,2048+1265,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==3)
			{
				StringUIData(str,8192,2048+1265,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj.color.b=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"A %d",meng.obj.color.a);

			if(meng.sub_com!=4)
			{
				if(CheckCollisionMouse(8192,2048+1720,2048,455,0))
				{
					StringUIData(str,8192,2048+1720,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj.color.a);
						StartText();
						st.mouse1=0;
						meng.sub_com=4;
					}
				}
				else
					StringUIData(str,8192,2048+1720,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==4)
			{
				StringUIData(str,8192,2048+1720,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj.color.a=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			if(st.keys[RETURN_KEY].state)
			{
				meng.command=meng.pannel_choice;
				st.keys[RETURN_KEY].state=0;
			}
		}

		sprintf(str,"Light %.2f",meng.obj.amblight);

		if(CheckCollisionMouse(465,2887,810,217,0))
		{
			StringUIData(str,465,2887,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.command=OBJ_AMBL;
				st.mouse1=0;
			}
		}
		else
			StringUIData(str,465,2887,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==OBJ_AMBL)
		{
			StringUIData(str,465,2887,810,217,0,255,32,32,255,ARIAL,0,0,0);

			if(st.keys[RIGHT_KEY].state)
			{
				meng.obj.amblight+=0.01;
				st.keys[RIGHT_KEY].state=0;
			}

			if(st.keys[LEFT_KEY].state)
			{
				meng.obj.amblight-=0.01;
				st.keys[LEFT_KEY].state=0;
			}

			if(!CheckCollisionMouse(465,2887,810,217,0) && st.mouse1)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}

		sprintf(str,"Tex.Size");

		if(CheckCollisionMouse(465,3315,810,217,0))
		{
			StringUIData(str,465,3315,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1 && meng.command!=TEX_PAN_OBJ)
			{
				meng.command=TEX_SIZE_OBJ;
				st.mouse1=0;
			}
		}
		else
			StringUIData(str,465,3315,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.obj.type==BACKGROUND1 || meng.obj.type==BACKGROUND2 || meng.obj.type==FOREGROUND)
		{
			if(!(meng.obj.flag & 1))
			{
				if(UIStringButton(465,3640+315,"Tex. Mov [ ]",ARIAL,1024,0,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
					meng.obj.flag |=1;
			}
			else
			if(meng.obj.flag & 1)
			{
				if(UIStringButton(465,3640+315,"Tex. Mov [X]",ARIAL,1024,0,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
					meng.obj.flag-=1;
			}
		}

		if(meng.obj.type==MIDGROUND)
		{
			if(!(meng.obj.flag & 2))
			{
				if(UIStringButton(465,3640+315+315,"ATMC [ ]",ARIAL,1024,0,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
					meng.obj.flag |=2;
			}
			else
			if(meng.obj.flag & 2)
			{
				if(UIStringButton(465,3640+315+315,"ATMC [X]",ARIAL,1024,0,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
					meng.obj.flag-=2;
			}
		}
			
			
		if(meng.command==TEX_SIZE_OBJ)
		{
			UIData(8192,4608,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			sprintf(str,"X %d",meng.obj.texsize.x);

			if(meng.sub_com!=1)
			{
				if(CheckCollisionMouse(8192,3641,2048,910,0))
				{
					StringUIData(str,8192,3641,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						st.mouse1=0;
						sprintf(st.TextInput,"%d",meng.obj.texsize.x);
						st.Text_Input=1;
						SDL_StartTextInput();
						meng.sub_com=1;
					}
				}
				else
					StringUIData(str,8192,3641,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				StringUIData(str,8192,3641,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj.texsize.x=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					SDL_StopTextInput();
					st.Text_Input=0;
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"Y %d",meng.obj.texsize.y);

			if(meng.sub_com!=2)
			{
				if(CheckCollisionMouse(8192,4551,2048,910,0))
				{
					StringUIData(str,8192,4551,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						st.mouse1=0;
						sprintf(st.TextInput,"%d",meng.obj.texsize.y);
						st.Text_Input=1;
						SDL_StartTextInput();
						meng.sub_com=2;
					}
				}
				else
					StringUIData(str,8192,4551,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				StringUIData(str,8192,4551,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj.texsize.y=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					SDL_StopTextInput();
					st.Text_Input=0;
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}
			

			if(st.keys[UP_KEY].state)
			{
				meng.obj2.texsize.y+=128;
				st.keys[UP_KEY].state=0;
			}

			if(st.keys[DOWN_KEY].state)
			{
				meng.obj2.texsize.y-=128;
				st.keys[DOWN_KEY].state=0;
			}

			if(st.keys[RIGHT_KEY].state)
			{
				meng.obj2.texsize.x+=128;
				st.keys[RIGHT_KEY].state=0;
			}

			if(st.keys[LEFT_KEY].state)
			{
				meng.obj2.texsize.x-=128;
				st.keys[LEFT_KEY].state=0;
			}

			if(!CheckCollisionMouse(465,3315,810,217,0) && st.mouse1 && meng.sub_com==0)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}

		sprintf(str,"Tex.Pan");

		if(CheckCollisionMouse(465,3640,810,217,0))
		{
			StringUIData(str,465,3640,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1 && meng.command!=TEX_SIZE_OBJ)
			{
				meng.command=TEX_PAN_OBJ;
				st.mouse1=0;
			}
		}
		else
			StringUIData(str,465,3640,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==TEX_PAN_OBJ)
		{
			UIData(8192,4608,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			sprintf(str,"X %d",meng.obj.texpan.x);

			if(meng.sub_com!=1)
			{
				if(CheckCollisionMouse(8192,3641,2048,910,0))
				{
					StringUIData(str,8192,3641,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						st.mouse1=0;
						sprintf(st.TextInput,"%d",meng.obj.texpan.x);
						st.Text_Input=1;
						SDL_StartTextInput();
						meng.sub_com=1;
					}
				}
				else
					StringUIData(str,8192,3641,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				StringUIData(str,8192,3641,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj.texpan.x=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					SDL_StopTextInput();
					st.Text_Input=0;
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"Y %d",meng.obj.texpan.y);

			if(meng.sub_com!=2)
			{
				if(CheckCollisionMouse(8192,4551,2048,910,0))
				{
					StringUIData(str,8192,4551,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						st.mouse1=0;
						sprintf(st.TextInput,"%d",meng.obj.texpan.y);
						st.Text_Input=1;
						SDL_StartTextInput();
						meng.sub_com=2;
					}
				}
				else
					StringUIData(str,8192,4551,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				StringUIData(str,8192,4551,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj.texpan.y=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					SDL_StopTextInput();
					st.Text_Input=0;
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}
			

			if(st.keys[UP_KEY].state)
			{
				meng.obj.texpan.y+=128;
				st.keys[UP_KEY].state=0;
			}

			if(st.keys[DOWN_KEY].state)
			{
				meng.obj.texpan.y-=128;
				st.keys[DOWN_KEY].state=0;
			}

			if(st.keys[RIGHT_KEY].state)
			{
				meng.obj.texpan.x+=128;
				st.keys[RIGHT_KEY].state=0;
			}

			if(st.keys[LEFT_KEY].state)
			{
				meng.obj.texpan.x-=128;
				st.keys[LEFT_KEY].state=0;
			}

			if(!CheckCollisionMouse(465,3315,810,217,0) && st.mouse1 && meng.sub_com==0)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}

	}

	if(meng.command==ADD_OBJ_TYPE)
	{
		UIData(1365,1715,910,1185,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,0);

		if(CheckCollisionMouse(1365,1715,810,227,0))
		{
			StringUIData("Background1",1365,1715,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.obj.type=BACKGROUND1;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			StringUIData("Background1",1365,1715,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckCollisionMouse(1365,1465,810,227,0))
		{
			StringUIData("Background2",1365,1465,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.obj.type=BACKGROUND2;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			StringUIData("Background2",1365,1465,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckCollisionMouse(1365,1215,810,227,0))
		{
			StringUIData("Background3",1365,1215,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.obj.type=BACKGROUND3;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			StringUIData("Background3",1365,1215,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckCollisionMouse(1365,1920,810,227,0))
		{
			StringUIData("Midground",1365,1920,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.obj.type=MIDGROUND;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			StringUIData("Midground",1365,1920,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckCollisionMouse(1365,2170,810,227,0))
		{
			StringUIData("Foreground",1365,2170,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.obj.type=FOREGROUND;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			StringUIData("Foreground",1365,2170,810,227,0,255,255,255,255,ARIAL,0,0,0);
	}

	if(meng.command==EDIT_OBJ_TYPE)
	{
		UIData(1365,1715,910,1210,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,0);

		if(CheckCollisionMouse(1365,1715,810,227,0))
		{
			StringUIData("Background1",1365,1715,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				if(st.Current_Map.obj[meng.com_id].type==BACKGROUND3)
					st.Current_Map.obj[meng.com_id].position.z-=16;
				else
				if(st.Current_Map.obj[meng.com_id].type==BACKGROUND2)
					st.Current_Map.obj[meng.com_id].position.z-=8;
				else
				if(st.Current_Map.obj[meng.com_id].type==MIDGROUND)
					st.Current_Map.obj[meng.com_id].position.z+=8;
				else
				if(st.Current_Map.obj[meng.com_id].type==FOREGROUND)
					st.Current_Map.obj[meng.com_id].position.z+=16;

				st.Current_Map.obj[meng.com_id].type=BACKGROUND1;
				//st.Current_Map.obj[meng.com_id].position.z=
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			StringUIData("Background1",1365,1715,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckCollisionMouse(1365,1465,810,227,0))
		{
			StringUIData("Background2",1365,1465,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				if(st.Current_Map.obj[meng.com_id].type==BACKGROUND3)
					st.Current_Map.obj[meng.com_id].position.z-=8;
				else
				if(st.Current_Map.obj[meng.com_id].type==BACKGROUND1)
					st.Current_Map.obj[meng.com_id].position.z+=8;
				else
				if(st.Current_Map.obj[meng.com_id].type==MIDGROUND)
					st.Current_Map.obj[meng.com_id].position.z+=16;
				else
				if(st.Current_Map.obj[meng.com_id].type==FOREGROUND)
					st.Current_Map.obj[meng.com_id].position.z+=32;

				st.Current_Map.obj[meng.com_id].type=BACKGROUND2;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			StringUIData("Background2",1365,1465,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckCollisionMouse(1365,1215,810,227,0))
		{
			StringUIData("Background3",1365,1215,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				if(st.Current_Map.obj[meng.com_id].type==BACKGROUND1)
					st.Current_Map.obj[meng.com_id].position.z+=16;
				else
				if(st.Current_Map.obj[meng.com_id].type==BACKGROUND2)
					st.Current_Map.obj[meng.com_id].position.z+=8;
				else
				if(st.Current_Map.obj[meng.com_id].type==MIDGROUND)
					st.Current_Map.obj[meng.com_id].position.z+=32;
				else
				if(st.Current_Map.obj[meng.com_id].type==FOREGROUND)
					st.Current_Map.obj[meng.com_id].position.z+=40;

				st.Current_Map.obj[meng.com_id].type=BACKGROUND3;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			StringUIData("Background3",1365,1215,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckCollisionMouse(1365,1920,810,227,0))
		{
			StringUIData("Midground",1365,1920,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				if(st.Current_Map.obj[meng.com_id].type==BACKGROUND3)
					st.Current_Map.obj[meng.com_id].position.z-=32;
				else
				if(st.Current_Map.obj[meng.com_id].type==BACKGROUND2)
					st.Current_Map.obj[meng.com_id].position.z-=16;
				else
				if(st.Current_Map.obj[meng.com_id].type==BACKGROUND1)
					st.Current_Map.obj[meng.com_id].position.z-=8;
				else
				if(st.Current_Map.obj[meng.com_id].type==FOREGROUND)
					st.Current_Map.obj[meng.com_id].position.z+=8;

				st.Current_Map.obj[meng.com_id].type=MIDGROUND;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			StringUIData("Midground",1365,1920,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckCollisionMouse(1365,2170,810,227,0))
		{
			StringUIData("Foreground",1365,2170,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				if(st.Current_Map.obj[meng.com_id].type==BACKGROUND3)
					st.Current_Map.obj[meng.com_id].position.z-=40;
				else
				if(st.Current_Map.obj[meng.com_id].type==BACKGROUND2)
					st.Current_Map.obj[meng.com_id].position.z-=32;
				else
				if(st.Current_Map.obj[meng.com_id].type==BACKGROUND1)
					st.Current_Map.obj[meng.com_id].position.z-=16;
				else
				if(st.Current_Map.obj[meng.com_id].type==MIDGROUND)
					st.Current_Map.obj[meng.com_id].position.z-=8;

				st.Current_Map.obj[meng.com_id].type=FOREGROUND;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			StringUIData("Foreground",1365,2170,810,227,0,255,255,255,255,ARIAL,0,0,0);
	}

}

static void ViewPortCommands()
{
	Pos vertextmp[4];
	uint8 got_it=0;
	char str[64];
	const char options[5][16]={"Metal", "Wood", "Plastic", "Concrete", "Organic"};
	int16 i, j, k, l, m;
	static Pos p, p2;
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

	if(!CheckCollisionMouse(2458/2,4885,2458,8939,0) && meng.command!=MGG_SEL && !CheckCollisionMouse(8192,128,16384,256,0))
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
			
			if(st.mouse2)
			{
				if(st.Current_Map.num_sector<MAX_SECTORS)
				{
					//i=st.Current_Map.num_sector;
					for(l=meng.z_slot[24]-1;l>-1;l--)
					{
						i=meng.z_buffer[24][l];

						if(i<10000) continue;

						i-=10000;

						if(st.Current_Map.sector[i].id!=-1 && CheckCollisionMouseWorld(st.Current_Map.sector[i].vertex[0].x,st.Current_Map.sector[i].vertex[0].y,256,256,0,0,0,0))
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

							for(k=l;k<meng.z_slot[24];k++)
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

							break;
						}
					}
				}
				LogApp("Sector Removed");
				st.mouse2=0;
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
						st.Current_Map.sector[i].sloped=1;
					}
					else
					{
						st.Current_Map.sector[i].vertex[meng.sub_com].x=st.mouse.x;
						STW(&st.Current_Map.sector[i].vertex[meng.sub_com].x,&st.Current_Map.sector[i].vertex[meng.sub_com].y);
						st.Current_Map.sector[i].vertex[meng.sub_com].y=st.Current_Map.sector[i].vertex[0].y;
						st.Current_Map.sector[i].base_y=st.Current_Map.sector[i].vertex[0].y;
						st.Current_Map.sector[i].sloped=0;
					}

					st.Current_Map.sector[i].num_vertexadded++;

					meng.command=SELECT_EDIT;
					meng.pannel_choice=SELECT_EDIT;
					meng.sub_com=0;
					meng.com_id=0;

					st.mouse1=0;

					meng.z_buffer[24][meng.z_slot[24]]=i+10000;
					meng.z_slot[24]++;

					meng.current_command=0;

					LogApp("Sector Added");
				}
			}
			
			if(st.mouse2)
			{
				if(meng.sub_com)
				{
					st.Current_Map.sector[i].num_vertexadded--;

					if(meng.sub_com==1)
					{
						st.Current_Map.sector[i].id=-1;

						LogApp("Sector removed");

						meng.sub_com=0;
						meng.com_id=0;
						meng.command=DRAW_SECTOR;

					}
					else
					{
						LogApp("Sector vertex %d removed",meng.sub_com);

						meng.sub_com--;

						meng.current_command=0;
					}

					st.mouse2=0;
				}
			}
		}
		else
		if(meng.command==SELECT_EDIT)
		{
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

			if(st.Current_Map.num_sector>0)
			{
				l=24;
				//for(l=16;l<58;l++)
				//{
					for(k=meng.z_slot[l];k>-1;k--)
					{
						if(meng.viewmode!=1 && meng.viewmode<5)
							break;

						if (meng.curlayer == 1 && l<24 || meng.curlayer == 1 && l>31)
							break;

						i=meng.z_buffer[l][k];

						if(i<10000) continue;

						i-=10000;

						if((CheckCollisionMouseWorld(st.Current_Map.sector[i].vertex[0].x,st.Current_Map.sector[i].vertex[0].y,484,484,0,0) || 
							CheckCollisionMouseWorld(st.Current_Map.sector[i].vertex[1].x,st.Current_Map.sector[i].vertex[1].y,484,484,0,0)))
						{
							if(st.mouse1)
							{
								if (meng.got_it == -1)
									meng.got_it = i;
								else
								if (meng.got_it != -1 && meng.got_it != i)
									continue;

								p=st.mouse;

								STW(&p.x,&p.y);

								if(CheckCollisionMouseWorld(st.Current_Map.sector[i].vertex[0].x,st.Current_Map.sector[i].vertex[0].y,484,484,0,0))
								{
									got_it = 1;

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
							else
								meng.got_it=-1;

							if(st.mouse2)
							{
								meng.command2=EDIT_SECTOR;
								meng.com_id=i;
								winid=UICreateWindow2(0,0,CENTER,6,4,2048,32,ARIAL);
								st.mouse2=0;
								break;
							}
						}

						if(got_it) break;
					}
				//}
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

						if(CheckCollisionMouseWorld(st.Current_Map.sprites[i].position.x,st.Current_Map.sprites[i].position.y,st.Current_Map.sprites[i].body.size.x,st.Current_Map.sprites[i].body.size.y,
							st.Current_Map.sprites[i].angle,st.Current_Map.sprites[i].position.z))
						{
							if(st.mouse1)
							{
								if(meng.got_it==-1)
								{
									meng.command2=EDIT_SPRITE;
									meng.p=st.mouse;
									meng.got_it=i;

									meng.sprite_edit_selection=i;

									STW(&meng.p.x, &meng.p.y);

									meng.p.x-=st.Current_Map.sprites[i].position.x;
									meng.p.y-=st.Current_Map.sprites[i].position.y;
								}
								else
								if(meng.got_it!=-1 && meng.got_it!=i)
									continue;

								p=st.mouse;

								STW(&p.x, &p.y);

								meng.com_id=i;

								st.Current_Map.sprites[i].position.x=p.x;
								st.Current_Map.sprites[i].position.y=p.y;

								st.Current_Map.sprites[i].position.x-=meng.p.x;
								st.Current_Map.sprites[i].position.y-=meng.p.y;

								p.x=st.Current_Map.sprites[i].position.x+(st.Current_Map.sprites[i].body.size.x/2);
								p.y=st.Current_Map.sprites[i].position.y-(st.Current_Map.sprites[i].body.size.y/2);

								sprintf(str,"%d",st.Current_Map.sprites[i].angle);
								String2Data(str,p.x+227,p.y-227,405,217,0,255,255,255,255,ARIAL,1024,1024,st.Current_Map.sprites[i].position.z);

								if(st.mouse_wheel>0 && !st.mouse2)
								{
									if(st.keys[RSHIFT_KEY].state) st.Current_Map.sprites[i].angle+=1;
									else st.Current_Map.sprites[i].angle+=100;
									st.mouse_wheel=0;
								}

								if(st.mouse_wheel<0 && !st.mouse2)
								{
									if(st.keys[RSHIFT_KEY].state) st.Current_Map.sprites[i].angle-=1;
									else st.Current_Map.sprites[i].angle-=100;
									st.mouse_wheel=0;
								}

								got_it=1;
								break;
							}
							else
								meng.got_it=-1;
						}
					}
				}
			}

			if(st.Current_Map.num_obj>0)
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

						if(i>1099) continue;
						/*
						if(meng.command2==EDIT_OBJ && meng.got_it==i)
							if(Sys_ResizeController(st.Current_Map.obj[i].position.x,st.Current_Map.obj[i].position.y,&st.Current_Map.obj[i].size.x,&st.Current_Map.obj[i].size.y,0,st.Current_Map.obj[i].angle,
								st.Current_Map.obj[i].position.z))
								break;
								*/

						if(got_it) break;

						if(CheckCollisionMouseWorld(st.Current_Map.obj[i].position.x,st.Current_Map.obj[i].position.y,st.Current_Map.obj[i].size.x,st.Current_Map.obj[i].size.y,st.Current_Map.obj[i].angle,
							st.Current_Map.obj[i].position.z))
						{
							if(st.mouse1)
							{
								if(meng.got_it==-1)
								{
									meng.command2=EDIT_OBJ;
									meng.p=st.mouse;
									meng.got_it=i;

									meng.obj_edit_selection = i;

									STW(&meng.p.x, &meng.p.y);

									meng.p.x-=st.Current_Map.obj[i].position.x;
									meng.p.y-=st.Current_Map.obj[i].position.y;
								}
								else
								if(meng.got_it!=-1 && meng.got_it!=i)
									continue;
								else
								{
									p=st.mouse;

									STW(&p.x, &p.y);

									meng.com_id=i;

									st.Current_Map.obj[i].position.x=p.x;
									st.Current_Map.obj[i].position.y=p.y;

									st.Current_Map.obj[i].position.x-=meng.p.x;
									st.Current_Map.obj[i].position.y-=meng.p.y;

									if(st.Current_Map.obj[i].lightmapid!=-1)
									{
										st.game_lightmaps[st.Current_Map.obj[i].lightmapid].w_pos.x=st.Current_Map.obj[i].position.x;
										st.game_lightmaps[st.Current_Map.obj[i].lightmapid].w_pos.y=st.Current_Map.obj[i].position.y;
									}

									p.x=st.Current_Map.obj[i].position.x+(st.Current_Map.obj[i].size.x/2);
									p.y=st.Current_Map.obj[i].position.y-(st.Current_Map.obj[i].size.y/2);

									sprintf(str,"%d",st.Current_Map.obj[i].angle);
									String2Data(str,p.x+227,p.y-227,405,217,0,255,255,255,255,ARIAL,1024,1024,st.Current_Map.obj[i].position.z);
									/*
									if(st.mouse_wheel>0 && !st.mouse2)
									{
										if(st.keys[RSHIFT_KEY].state) st.Current_Map.obj[i].angle+=1;
										else st.Current_Map.obj[i].angle+=100;

										if(st.Current_Map.obj[i].lightmapid!=-1)
											st.game_lightmaps[st.Current_Map.obj[i].lightmapid].ang=st.Current_Map.obj[i].angle;

										st.mouse_wheel=0;
									}

									if(st.mouse_wheel<0 && !st.mouse2)
									{
										if(st.keys[RSHIFT_KEY].state) st.Current_Map.obj[i].angle-=1;
										else st.Current_Map.obj[i].angle-=100;

										if(st.Current_Map.obj[i].lightmapid!=-1)
											st.game_lightmaps[st.Current_Map.obj[i].lightmapid].ang=st.Current_Map.obj[i].angle;

										st.mouse_wheel=0;
									}
									*/
									got_it=1;
									break;
								}
							}
							else
								meng.got_it=-1;
						}
					}
				}
			}
		}
		else
		if(meng.command==ADD_OBJ && st.Current_Map.num_mgg > 0)
		{
			if(st.mouse1)
			{
				if(st.Current_Map.num_obj<MAX_OBJS)
				{
					i=st.Current_Map.num_obj;
					//for(i=0;i<MAX_OBJS;i++)
					//{
						if(st.Current_Map.obj[i].type==BLANK)
						{
							st.Current_Map.obj[i].type=meng.obj.type;
								
							st.Current_Map.obj[i].amblight=meng.obj.amblight;
							st.Current_Map.obj[i].color=meng.obj.color;
							st.Current_Map.obj[i].tex.ID=meng.tex_ID;
							st.Current_Map.obj[i].tex.MGG_ID=meng.tex_MGGID;
							st.Current_Map.obj[i].texsize=meng.obj.texsize;
							st.Current_Map.obj[i].texpan=meng.obj.texpan;
							st.Current_Map.obj[i].position=st.mouse;
							STWci(&st.Current_Map.obj[i].position.x,&st.Current_Map.obj[i].position.y);

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

								st.Current_Map.obj[i].position.x+=(float) st.Camera.position.x*st.Current_Map.fr_v;
								st.Current_Map.obj[i].position.y+=(float) st.Camera.position.y*st.Current_Map.fr_v;
							}

							st.Current_Map.obj[i].size.x=meng.pre_size.x;
							st.Current_Map.obj[i].size.y=meng.pre_size.y;
							st.Current_Map.obj[i].angle=0;
							st.Current_Map.obj[i].flag=meng.obj.flag;

							if(meng.obj.type==BACKGROUND3)
								st.Current_Map.obj[i].position.z=48;
							else
							if(meng.obj.type==BACKGROUND2)
								st.Current_Map.obj[i].position.z=40;
							else
							if(meng.obj.type==BACKGROUND1)
								st.Current_Map.obj[i].position.z=32;
							else
							if(meng.obj.type==MIDGROUND)
								st.Current_Map.obj[i].position.z=24;
							else
							if(meng.obj.type==FOREGROUND)
								st.Current_Map.obj[i].position.z=16;

							meng.z_buffer[st.Current_Map.obj[i].position.z][meng.z_slot[st.Current_Map.obj[i].position.z]]=i;
							meng.z_slot[st.Current_Map.obj[i].position.z]++;
							if(st.Current_Map.obj[i].position.z>meng.z_used)
								meng.z_used=st.Current_Map.obj[i].position.z;

							st.Current_Map.num_obj++;
							//break;
						}
					//}
				}

				LogApp("Object added");
				st.mouse1=0;
			}
			
			if(st.mouse2)
			{
				if(st.Current_Map.num_obj<MAX_OBJS)
				{
					for(l=16;l<57;l++)
					{
						for(k=meng.z_slot[l];k>-1;k--)
						{
							i=meng.z_buffer[l][k];

							if(i>1099) continue;

							if(st.Current_Map.obj[i].type!=BLANK && CheckCollisionMouseWorld(st.Current_Map.obj[i].position.x,st.Current_Map.obj[i].position.y,st.Current_Map.obj[i].size.x,st.Current_Map.obj[i].size.y,
								st.Current_Map.obj[i].angle,st.Current_Map.obj[i].position.z))
							{
								st.Current_Map.obj[i].type=BLANK;

								if(i<st.Current_Map.num_obj-1)
								{
									for(j=i;j<st.Current_Map.num_obj-1;j++)
									{
										memcpy(&st.Current_Map.obj[j],&st.Current_Map.obj[j+1],sizeof(_MGMOBJ));
										st.Current_Map.obj[j+1].type=BLANK;
									}
								}

								for(m=k;m<meng.z_slot[st.Current_Map.obj[i].position.z];m++)
									if(meng.z_buffer[st.Current_Map.obj[i].position.z][m]==st.Current_Map.num_obj-1)
										break;

								if(m<meng.z_slot[st.Current_Map.obj[i].position.z]-1)
								{
									for(;m<meng.z_slot[st.Current_Map.obj[i].position.z];m++)
									{
										meng.z_buffer[st.Current_Map.obj[i].position.z][m]=meng.z_buffer[st.Current_Map.obj[i].position.z][m+1];
										meng.z_buffer[st.Current_Map.obj[i].position.z][m+1]=0;
									}
								}
								else
									meng.z_buffer[st.Current_Map.obj[i].position.z][m]=0;

								meng.z_slot[l]--;

								st.Current_Map.num_obj--;
								break;
							}
						}
					}
				}

				LogApp("Object removed");
				st.mouse2=0;
			}
		}
		else
		if(meng.command==ADD_SPRITE)
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

							if(meng.spr.type==BACKGROUND3)
								st.Current_Map.sprites[i].position.z=48;
							else
							if(meng.spr.type==BACKGROUND2)
								st.Current_Map.sprites[i].position.z=40;
							else
							if(meng.spr.type==BACKGROUND1)
								st.Current_Map.sprites[i].position.z=32;
							else
							if(meng.spr.type==MIDGROUND)
								st.Current_Map.sprites[i].position.z=24;
							else
							if(meng.spr.type==FOREGROUND)
								st.Current_Map.sprites[i].position.z=16;

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
			
			if(st.mouse2)
			{
				if(st.Current_Map.num_sprites<MAX_SPRITES)
				{
					for(l=16;l<57;l++)
					{
						for(k=meng.z_slot[l];k>-1;k--)
						{
							i=meng.z_buffer[l][k];

							if(i<2000 || i>3000) continue;

							i-=2000;

							if(st.Current_Map.sprites[i].stat!=0 && CheckCollisionMouseWorld(st.Current_Map.sprites[i].position.x,st.Current_Map.sprites[i].position.y,st.Current_Map.sprites[i].body.size.x,st.Current_Map.sprites[i].body.size.y,
								st.Current_Map.sprites[i].body.ang,st.Current_Map.sprites[i].position.z))
							{
								st.Current_Map.sprites[i].stat=0;

								if(i<st.Current_Map.num_sprites-1)
								{
									for(j=i;j<st.Current_Map.num_sprites-1;j++)
									{
										memcpy(&st.Current_Map.sprites[j],&st.Current_Map.sprites[j+1],sizeof(_MGMSPRITE));
										st.Current_Map.sprites[j+1].stat=0;
									}
								}

								for(m=k;m<meng.z_slot[l];m++)
									if(meng.z_buffer[l][m]==st.Current_Map.num_sprites-1+2000)
										break;

								if(m<meng.z_slot[l]-1)
								{
									for(;m<meng.z_slot[l];m++)
									{
										meng.z_buffer[l][m]=meng.z_buffer[l][m+1];
										meng.z_buffer[l][m+1]=0;
									}
								}
								else
									meng.z_buffer[l][m]=0;

								meng.z_slot[l]--;

								st.Current_Map.num_sprites--;

								LogApp("Sprite removed");
								st.mouse2=0;

								break;
							}
						}
					}
				}
			}
		}
		else
		if(meng.pannel_choice==ADD_LIGHT && meng.command==MOVE_LIGHTMAP)
		{
			if(st.num_lights>0)
			{
				for(i=st.num_lights;i>=0;i--)
				{
					if(CheckCollisionMouseWorld(st.game_lightmaps[i].w_pos.x,st.game_lightmaps[i].w_pos.y,st.game_lightmaps[i].W_w,st.game_lightmaps[i].W_h,0,24))
					{
						if(st.mouse1)
						{
							if(meng.got_it==-1)
							{
								meng.p=st.mouse;
								meng.got_it=i;

								STW(&meng.p.x, &meng.p.y);

								meng.p.x-=st.game_lightmaps[i].w_pos.x;
								meng.p.y-=st.game_lightmaps[i].w_pos.y;
							}
							else
							if(meng.got_it!=-1 && meng.got_it!=i)
								continue;

							if(st.keys[LSHIFT_KEY].state)
							{
								if(st.mouse_wheel>0)
								{
									st.game_lightmaps[i].W_w+=256;
									st.game_lightmaps[i].W_h+=256;
									st.mouse_wheel=0;
								}
								else
								if(st.mouse_wheel<0)
								{
									st.game_lightmaps[i].W_w-=256;
									st.game_lightmaps[i].W_h-=256;
									st.mouse_wheel=0;
								}
							}

							if(st.keys[LCTRL_KEY].state)
							{
								if(st.mouse_wheel>0)
								{
									st.game_lightmaps[i].ang+=100;
									st.mouse_wheel=0;
								}
								else
								if(st.mouse_wheel<0)
								{
									st.game_lightmaps[i].ang-=100;
									st.mouse_wheel=0;
								}
							}

							p=st.mouse;

							STW(&p.x, &p.y);

							st.game_lightmaps[i].w_pos.x=p.x;
							st.game_lightmaps[i].w_pos.y=p.y;

							st.game_lightmaps[i].w_pos.x-=meng.p.x;
							st.game_lightmaps[i].w_pos.y-=meng.p.y;

							p.x=st.game_lightmaps[i].w_pos.x+(st.game_lightmaps[i].W_w/2);
							p.y=st.game_lightmaps[i].w_pos.y+(st.game_lightmaps[i].W_h/2);

							break;
						}
						else
							meng.got_it=-1;
					}
				}
			}
		}
		else
		if(meng.pannel_choice==ADD_LIGHT && meng.command==REMOVE_LIGHTMAP)
		{
			if(st.num_lights>0)
			{
				StringUIData("Select a lightmap to be deleted",8192,9216-455,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				for(j=st.num_lights;j>=1;j--)
				{
					if(CheckCollisionMouseWorld(st.game_lightmaps[j].w_pos.x,st.game_lightmaps[j].w_pos.y,st.game_lightmaps[j].W_w,st.game_lightmaps[j].W_h,st.game_lightmaps[j].ang,0) && st.mouse1)
					{
						st.game_lightmaps[j].num_lights=0;
						free(st.game_lightmaps[j].data);
						glDeleteTextures(1,&st.game_lightmaps[j].tex);
						st.Current_Map.obj[st.game_lightmaps[j].obj_id].lightmapid=-1;
						st.game_lightmaps[j].obj_id=-1;
						st.game_lightmaps[j].stat=0;

						if(j<=st.num_lights-1)
						{
							for(k=j;k<=st.num_lights-1;k++)
							{
								memcpy(&st.game_lightmaps[k],&st.game_lightmaps[k+1],sizeof(_GAME_LIGHTMAPS));
								free(st.game_lightmaps[k+1].data);
								st.game_lightmaps[k+1].stat=0;
								st.Current_Map.obj[st.game_lightmaps[k].obj_id].lightmapid=k;
							}
						}

						st.num_lights--;

						LogApp("Removed lightmap %d", j);

						st.mouse1=0;

						meng.command=meng.pannel_choice;

						break;
					}
				}
			}
			else
				meng.command=meng.pannel_choice;
		}
		else
		if(meng.pannel_choice==ADD_LIGHT && meng.command==EDIT_LIGHTMAP)
		{
			for(j=st.num_lights;j>=1;j--)
			{
				if(CheckCollisionMouseWorld(st.game_lightmaps[j].w_pos.x,st.game_lightmaps[j].w_pos.y,st.game_lightmaps[j].W_w,st.game_lightmaps[j].W_h,st.game_lightmaps[j].ang,0) && st.mouse1)
				{
					meng.got_it=j;

					st.mouse1=0;

					break;
				}
			}

			if(meng.got_it!=-1)
			{
				if(st.keys[RETURN_KEY].state)
				{
					meng.command=EDIT_LIGHTMAP2;
					meng.sub_com=0;
					st.keys[RETURN_KEY].state=0;
				}
			}
		}
		else
		if(meng.pannel_choice==ADD_LIGHT && meng.command==EDIT_LIGHTMAP2)
		{
			i=meng.got_it;

			if(!meng.sub_com)
			{
				if(meng.com_id==1)
				{
					p3=st.game_lightmaps[i].t_pos[temp];

					p3.x=(st.game_lightmaps[i].W_w*st.game_lightmaps[i].t_pos[temp].x)/st.game_lightmaps[i].T_w;
					p3.y=(st.game_lightmaps[i].W_h*st.game_lightmaps[i].t_pos[temp].y)/st.game_lightmaps[i].T_h;

					p3.x=p3.x+st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2);
					p3.y=p3.y+st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2);

					if(CheckCollisionMouseWorld(st.game_lightmaps[i].w_pos.x,st.game_lightmaps[i].w_pos.y,st.game_lightmaps[i].W_w,st.game_lightmaps[i].W_h,st.game_lightmaps[i].ang,0) && st.mouse1)
					{
						meng.loop_complete=1;

						p=st.mouse;

						STW(&p.x,&p.y);

						p.x=p.x-(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2));
						p.y=p.y-(st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2));

						p.x=(st.game_lightmaps[i].T_w*p.x)/st.game_lightmaps[i].W_w;
						p.y=(st.game_lightmaps[i].T_h*p.y)/st.game_lightmaps[i].W_h;

						st.game_lightmaps[i].t_pos[temp].x=p.x;
						st.game_lightmaps[i].t_pos[temp].y=p.y;

						if(st.game_lightmaps[i].alpha)
						{
							memcpy(meng.tmplightdata,meng.tmplightdata2,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

							if(st.game_lightmaps[i].type[temp]<4)
								AddLightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
										st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
										st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp]);
								else
									AddSpotlightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
										st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
										st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp],
										st.game_lightmaps[i].t_pos2[temp].x,st.game_lightmaps[i].t_pos2[temp].y,st.game_lightmaps[i].spot_ang[temp]);

							AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}
						else
						{
							memcpy(meng.tmplightdata,meng.tmplightdata2,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

							if(st.game_lightmaps[i].type[meng.light.light_id]<4)
								AddLightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
										st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
										st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp]);
								else
									AddSpotlightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
										st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
										st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp],
										st.game_lightmaps[i].t_pos2[temp].x,st.game_lightmaps[i].t_pos2[temp].y,st.game_lightmaps[i].spot_ang[temp]);

							AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}
					}

					if(st.mouse1)
					{
						meng.loop_complete=1;

						if(st.mouse_wheel>0)
						{
							st.mouse_wheel=0;
							st.game_lightmaps[i].falloff[temp]+=0.1;
						}
						else
						if(st.mouse_wheel<0)
						{
							st.mouse_wheel=0;
							st.game_lightmaps[i].falloff[temp]-=0.1;
						}


						if(st.game_lightmaps[i].alpha)
						{
							memcpy(meng.tmplightdata,meng.tmplightdata2,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

							if(st.game_lightmaps[i].type[temp]<4)
								AddLightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
										st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
										st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp]);
								else
									AddSpotlightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
										st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
										st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp],
										st.game_lightmaps[i].t_pos2[temp].x,st.game_lightmaps[i].t_pos2[temp].y,st.game_lightmaps[i].spot_ang[temp]);

							AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}
						else
						{
							memcpy(meng.tmplightdata,meng.tmplightdata2,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

							if(st.game_lightmaps[i].type[temp]<4)
								AddLightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
										st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
										st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp]);
								else
									AddSpotlightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
										st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
										st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp],
										st.game_lightmaps[i].t_pos2[temp].x,st.game_lightmaps[i].t_pos2[temp].y,st.game_lightmaps[i].spot_ang[temp]);

							AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}

					}

					if(st.keys[LSHIFT_KEY].state && st.game_lightmaps[i].type[temp]>3)
					{
						meng.loop_complete=1;

						if(st.mouse_wheel>0)
						{
							st.game_lightmaps[i].spot_ang[temp]++;
							st.mouse_wheel=0;
						}
						else
						if(st.mouse_wheel<0)
						{
							st.game_lightmaps[i].spot_ang[temp]--;
							st.mouse_wheel=0;
						}


						if(st.game_lightmaps[i].alpha)
						{
							memcpy(meng.tmplightdata,meng.tmplightdata2,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

							AddSpotlightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
										st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
										st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp],
										st.game_lightmaps[i].t_pos2[temp].x,st.game_lightmaps[i].t_pos2[temp].y,st.game_lightmaps[i].spot_ang[temp]);

							AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}
						else
						{
							memcpy(meng.tmplightdata,meng.tmplightdata2,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

							AddSpotlightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
										st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
										st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp],
										st.game_lightmaps[i].t_pos2[temp].x,st.game_lightmaps[i].t_pos2[temp].y,st.game_lightmaps[i].spot_ang[temp]);

							AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}

						st.keys[LSHIFT_KEY].state=0;
					}

					if(st.mouse2)
					{
						meng.loop_complete=1;

						if(st.mouse_wheel>0)
						{
							st.mouse_wheel=0;
							st.game_lightmaps[i].t_pos[temp].z+=2;
						}
						else
						if(st.mouse_wheel<0)
						{
							st.mouse_wheel=0;
							st.game_lightmaps[i].t_pos[temp].z-=2;
						}

						if(st.game_lightmaps[i].type[temp]>3)
						{
							p=st.mouse;

							STW(&p.x,&p.y);

							p.x=p.x-(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2));
							p.y=p.y-(st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2));

							p.x=(st.game_lightmaps[i].T_w*p.x)/st.game_lightmaps[i].W_w;
							p.y=(st.game_lightmaps[i].T_h*p.y)/st.game_lightmaps[i].W_h;

							st.game_lightmaps[i].t_pos2[temp].x=p.x;
							st.game_lightmaps[i].t_pos2[temp].y=p.y;
						}

						if(st.game_lightmaps[i].alpha)
						{
							memcpy(meng.tmplightdata,meng.tmplightdata2,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

							if(st.game_lightmaps[i].type[temp]<4)
								AddLightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
										st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
										st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp]);
								else
									AddSpotlightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
										st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
										st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp],
										st.game_lightmaps[i].t_pos2[temp].x,st.game_lightmaps[i].t_pos2[temp].y,st.game_lightmaps[i].spot_ang[temp]);

							AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}
						else
						{
							memcpy(meng.tmplightdata,meng.tmplightdata2,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

							if(st.game_lightmaps[i].type[temp]<4)
								AddLightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
										st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
										st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp]);
								else
									AddSpotlightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
										st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
										st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp],
										st.game_lightmaps[i].t_pos2[temp].x,st.game_lightmaps[i].t_pos2[temp].y,st.game_lightmaps[i].spot_ang[temp]);

							AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}
					}
				}

				if(meng.com_id==0)
				{
					p=st.game_lightmaps[i].w_pos;

					p.x+=(st.game_lightmaps[i].W_w/2)+1920;

					if(CheckCollisionMouseWorld(p.x,p.y,455,455,0,24))
					{
						String2Data("Done",p.x,p.y,0,0,0,255,128,32,255,ARIAL,1536,1536,24);

						if(st.mouse1)
						{
							meng.got_it=-1;
							meng.command=meng.pannel_choice=meng.command2=ADD_LIGHT;

							st.mouse1=0;
						}
					}
					else
						String2Data("Done",p.x,p.y,0,0,0,255,255,255,255,ARIAL,1536,1536,24);

					p=st.game_lightmaps[i].w_pos;

					p.y+=(st.game_lightmaps[i].W_h/2)+810;

					if(CheckCollisionMouseWorld(p.x,p.y,455,455,0,24))
					{
						String2Data("Add Light",p.x,p.y,0,0,0,255,128,32,255,ARIAL,1536,1536,24);

						if(st.mouse1)
						{
							meng.com_id=1;

							st.game_lightmaps[i].num_lights++;
							meng.light.light_id=st.game_lightmaps[i].num_lights-1;
							temp=meng.light.light_id;
							meng.temp=temp;

							st.game_lightmaps[i].t_pos[meng.light.light_id].x=st.game_lightmaps[i].T_w/2;
							st.game_lightmaps[i].t_pos[meng.light.light_id].y=st.game_lightmaps[i].T_h/2;
							st.game_lightmaps[i].t_pos[meng.light.light_id].z=0;
							st.game_lightmaps[i].t_pos2[meng.light.light_id].x=0;
							st.game_lightmaps[i].t_pos2[meng.light.light_id].y=0;
							st.game_lightmaps[i].color[meng.light.light_id].r=255;
							st.game_lightmaps[i].color[meng.light.light_id].g=255;
							st.game_lightmaps[i].color[meng.light.light_id].b=255;
							st.game_lightmaps[i].color[meng.light.light_id].a=255;
							st.game_lightmaps[i].falloff[meng.light.light_id]=16.0f;
							st.game_lightmaps[i].spot_ang[meng.light.light_id]=30;
							st.game_lightmaps[i].type[meng.light.light_id]=POINT_LIGHT_MEDIUM;

							meng.light.falloff=16.0f;
							meng.light.color.r=255;
							meng.light.color.g=255;
							meng.light.color.b=255;
							meng.light.intensity=255;
							meng.light.type=POINT_LIGHT_MEDIUM;

							if(st.game_lightmaps[i].alpha)
							{
								meng.tmplightdata=malloc(st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);
								meng.tmplightdata2=malloc(st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

								memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);
								memcpy(meng.tmplightdata2,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

								AddLightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,meng.light.color.r,meng.light.color.g,meng.light.color.b,meng.light.falloff,
									st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,st.game_lightmaps[i].t_pos[meng.light.light_id].z,meng.light.intensity,meng.light.type);
								AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
							}
							else
							{
								meng.tmplightdata=malloc(st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);
								meng.tmplightdata2=malloc(st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

								memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);
								memcpy(meng.tmplightdata2,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

								AddLightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,meng.light.color.r,meng.light.color.g,meng.light.color.b,meng.light.falloff,
									st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,st.game_lightmaps[i].t_pos[meng.light.light_id].z,meng.light.intensity,meng.light.type);
								AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
							}

							//LogApp("Light added");

							st.mouse1=0;
						}
					}
					else
						String2Data("Add Light",p.x,p.y,0,0,0,255,255,255,255,ARIAL,1536,1536,24);

				}
				else
				{
					p=st.game_lightmaps[i].w_pos;

					p.x+=(st.game_lightmaps[i].W_w/2)+1920;
					p.y+=810;

					if(CheckCollisionMouseWorld(p.x,p.y,455,455,0,24))
					{
						String2Data("Done Light",p.x,p.y,0,0,0,255,128,32,255,ARIAL,1536,1536,24);

						if(st.mouse1)
						{
							meng.com_id=0;

							if(st.game_lightmaps[i].alpha)
							{
								memcpy(st.game_lightmaps[i].data,meng.tmplightdata,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

								//AddLightToAlphaLight(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,meng.light.color.r,meng.light.color.g,meng.light.color.b,meng.light.falloff,
									//st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,st.game_lightmaps[i].t_pos[temp].z,meng.light.intensity,meng.light.type);
								AddLightToAlphaTexture(&st.game_lightmaps[i].tex,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
							}
							else
							{
								memcpy(st.game_lightmaps[i].data,meng.tmplightdata,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

								//AddLightToLightmap(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,meng.light.color.r,meng.light.color.g,meng.light.color.b,meng.light.falloff,
									//st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,st.game_lightmaps[i].t_pos[temp].z,meng.light.intensity,meng.light.type);
								AddLightToTexture(&st.game_lightmaps[i].tex,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
							}

							free(meng.tmplightdata);
							free(meng.tmplightdata2);


							st.mouse1=0;
						}
					}
					else
						String2Data("Done Light",p.x,p.y,0,0,0,255,255,255,255,ARIAL,1536,1536,24);


					p=st.game_lightmaps[i].w_pos;

					p.x-=(st.game_lightmaps[i].W_w/2)+1920;
					p.y+=810;

					if(UIStringButtonWorld(p.x,p.y,"Remove Light",ARIAL,1536,24,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
					{
						st.game_lightmaps[i].num_lights--;

						if(st.game_lightmaps[i].type[temp]<4)
						{
							memcpy(st.game_lightmaps[i].data,meng.tmplightdata2,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);
							AddLightToTexture(&st.game_lightmaps[i].tex,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}
						else
						{
							memcpy(st.game_lightmaps[i].data,meng.tmplightdata2,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);
							AddLightToAlphaTexture(&st.game_lightmaps[i].tex,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}

						free(meng.tmplightdata);
						free(meng.tmplightdata2);

						meng.com_id=0;
					}
				}

				p=st.game_lightmaps[i].w_pos;

				p.y-=(st.game_lightmaps[i].W_h/2)+810;

				if(CheckCollisionMouseWorld(p.x,p.y,455,455,0,24))
				{
					String2Data("Edit light",p.x,p.y,0,0,0,255,128,32,255,ARIAL,1536,1536,24);

					if(st.mouse1)
					{
						if(meng.com_id==1)
							meng.sub_com=1;
						else
							meng.sub_com=2;

						st.mouse1=0;
					}
				}
				else
					String2Data("Edit light",p.x,p.y,0,0,0,255,255,255,255,ARIAL,1536,1536,24);
			}
			else
			if(meng.sub_com==1)
			{
				if(meng.com_id==1)
				{
					//String2Data("Edit light",p.x,p.y,0,0,0,255,32,32,255,ARIAL,1536,1536,0);

					Sys_ColorPicker(&st.game_lightmaps[i].color[temp].r,&st.game_lightmaps[i].color[temp].g,&st.game_lightmaps[i].color[temp].b);

					UIData(8192,4300,3072,3900,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,7);

					sprintf(str,"R %d",st.game_lightmaps[i].color[temp].r);

					if(CheckCollisionMouse(8192,4608-(341*3),341,341,0))
					{
						StringUIData(str,8192,4608-(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							StartText();
							sprintf(st.TextInput,"%d",st.game_lightmaps[i].color[temp].r);
							meng.command2=1;
							st.mouse1=0;
						}
					}
					else
						StringUIData(str,8192,4608-(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==1)
					{
						StringUIData(str,8192,4608-(341*3),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

						st.game_lightmaps[i].color[temp].r=atoi(st.TextInput);

						if(st.keys[RETURN_KEY].state)
						{
							StopText();
							st.keys[RETURN_KEY].state=0;
							meng.command2=0;
						}
					}

					sprintf(str,"G %d",st.game_lightmaps[i].color[temp].g);

					if(CheckCollisionMouse(8192,4608-(341*2),341,341,0))
					{
						StringUIData(str,8192,4608-(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							StartText();
							sprintf(st.TextInput,"%d",st.game_lightmaps[i].color[temp].g);
							meng.command2=2;
							st.mouse1=0;
						}
					}
					else
						StringUIData(str,8192,4608-(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==2)
					{
						StringUIData(str,8192,4608-(341*2),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

						st.game_lightmaps[i].color[temp].g=atoi(st.TextInput);

						if(st.keys[RETURN_KEY].state)
						{
							StopText();
							st.keys[RETURN_KEY].state=0;
							meng.command2=0;
						}
					}

					sprintf(str,"B %d",st.game_lightmaps[i].color[temp].b);

					if(CheckCollisionMouse(8192,4608-341,341,341,0))
					{
						StringUIData(str,8192,4608-341,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							StartText();
							sprintf(st.TextInput,"%d",st.game_lightmaps[i].color[temp].b);
							meng.command2=3;
							st.mouse1=0;
						}
					}
					else
						StringUIData(str,8192,4608-341,0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==3)
					{
						StringUIData(str,8192,4608-341,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

						st.game_lightmaps[i].color[temp].b=atoi(st.TextInput);

						if(st.keys[RETURN_KEY].state)
						{
							StopText();
							st.keys[RETURN_KEY].state=0;
							meng.command2=0;
						}
					}

					if(meng.command2!=4)
						sprintf(str,"Intensity %d",st.game_lightmaps[i].color[temp].a);
					else
						strcpy(str,st.TextInput);

					if(CheckCollisionMouse(8192,4608,341,341,0))
					{
						StringUIData(str,8192,4608,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							StartText();
							sprintf(st.TextInput,"%d",st.game_lightmaps[i].color[temp].a);
							meng.command2=4;
							st.mouse1=0;
						}
					}
					else
						StringUIData(str,8192,4608,0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==4)
					{
						StringUIData(str,8192,4608,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

						st.game_lightmaps[i].color[temp].a=atoi(st.TextInput);

						if(st.keys[RETURN_KEY].state)
						{
							StopText();
							st.keys[RETURN_KEY].state=0;
							meng.command2=0;
						}
					}

					if(meng.command2!=5)
						sprintf(str,"Fall Off %.3f",st.game_lightmaps[i].falloff[temp]);
					else
						strcpy(str,st.TextInput);

					if(CheckCollisionMouse(8192,4608+341,341,341,0))
					{
						StringUIData(str,8192,4608+341,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							StartText();
							sprintf(st.TextInput,"%.3f",st.game_lightmaps[i].falloff[temp]);
							meng.command2=5;
							st.mouse1=0;
						}
					}
					else
						StringUIData(str,8192,4608+341,0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==5)
					{
						StringUIData(str,8192,4608+341,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

						st.game_lightmaps[i].falloff[temp]=atof(st.TextInput);

						if(st.keys[RETURN_KEY].state)
						{
							StopText();
							st.keys[RETURN_KEY].state=0;
							meng.command2=0;
						}
					}

					sprintf(str,"Z %d",st.game_lightmaps[i].t_pos[temp].z);

					if(CheckCollisionMouse(8192,4608+(341*2),341,341,0))
					{
						StringUIData(str,8192,4608+(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							StartText();
							sprintf(st.TextInput,"%d",st.game_lightmaps[i].t_pos[temp].z);
							meng.command2=6;
							st.mouse1=0;
						}
					}
					else
						StringUIData(str,8192,4608+(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==6)
					{
						StringUIData(str,8192,4608+(341*2),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

						st.game_lightmaps[i].t_pos[temp].z=atoi(st.TextInput);

						if(st.keys[RETURN_KEY].state)
						{
							StopText();
							st.keys[RETURN_KEY].state=0;
							meng.command2=0;
						}
					}

					if(st.game_lightmaps[i].type[temp]==POINT_LIGHT_MEDIUM) strcpy(str,"Point medium");
					else if(st.game_lightmaps[i].type[temp]==POINT_LIGHT_STRONG) strcpy(str,"Point strong");
					else if(st.game_lightmaps[i].type[temp]==POINT_LIGHT_NORMAL) strcpy(str,"Point normal");
					else if(st.game_lightmaps[i].type[temp]==SPOTLIGHT_MEDIUM) strcpy(str,"Spotlight medium");
					else if(st.game_lightmaps[i].type[temp]==SPOTLIGHT_STRONG) strcpy(str,"Spotlight strong");
					else if(st.game_lightmaps[i].type[temp]==SPOTLIGHT_NORMAL) strcpy(str,"Spotlight normal");

					if(CheckCollisionMouse(8192,4608+(341*3),341,341,0))
					{
						StringUIData(str,8192,4608+(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.command2=7;
							st.mouse1=0;
						}
					}
					else
						StringUIData(str,8192,4608+(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==7)
					{
						UIData(11264,4608,3072,3072,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

						if(CheckCollisionMouse(11264,4608-(341*3),341,341,0))
						{
							StringUIData("Point medium",11264,4608-(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

							if(st.mouse1)
							{
								st.game_lightmaps[i].type[temp]=POINT_LIGHT_MEDIUM;
								meng.command2=-1;
								st.mouse1=0;
							}
						}
						else
							StringUIData("Point medium",11264,4608-(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

						if(CheckCollisionMouse(11264,4608-(341*2),341,341,0))
						{
							StringUIData("Point strong",11264,4608-(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

							if(st.mouse1)
							{
								st.game_lightmaps[i].type[temp]=POINT_LIGHT_STRONG;
								meng.command2=-1;
								st.mouse1=0;
							}
						}
						else
							StringUIData("Point strong",11264,4608-(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

						if(CheckCollisionMouse(11264,4608-(341),341,341,0))
						{
							StringUIData("Point normal",11264,4608-(341),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

							if(st.mouse1)
							{
								st.game_lightmaps[i].type[temp]=POINT_LIGHT_NORMAL;
								meng.command2=-1;
								st.mouse1=0;
							}
						}
						else
							StringUIData("Point normal",11264,4608-(341),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

						if(CheckCollisionMouse(11264,4608+(341),341,341,0))
						{
							StringUIData("Spotlight medium",11264,4608+(341),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

							if(st.mouse1)
							{
								st.game_lightmaps[i].type[temp]=SPOTLIGHT_MEDIUM;
								meng.command2=-1;
								st.mouse1=0;
							}
						}
						else
							StringUIData("Spotlight medium",11264,4608+(341),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

						if(CheckCollisionMouse(11264,4608+(341*2),341,341,0))
						{
							StringUIData("Spotlight strong",11264,4608+(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

							if(st.mouse1)
							{
								st.game_lightmaps[i].type[temp]=SPOTLIGHT_STRONG;
								meng.command2=-1;
								st.mouse1=0;
							}
						}
						else
							StringUIData("Spotlight Strong",11264,4608+(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

						if(CheckCollisionMouse(11264,4608+(341*3),341,341,0))
						{
							StringUIData("Spotlight normal",11264,4608+(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

							if(st.mouse1)
							{
								st.game_lightmaps[i].type[temp]=SPOTLIGHT_NORMAL;
								meng.command2=-1;
								st.mouse1=0;
							}
						}
						else
							StringUIData("Spotlight normal",11264,4608+(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

						if(st.keys[RETURN_KEY].state)
						{
							meng.command2=-1;
							st.keys[RETURN_KEY].state=0;
						}
					}

					sprintf(str,"Spot angle %d",st.game_lightmaps[i].spot_ang[temp]);

					if(CheckCollisionMouse(8192,4608+(341*4),341,341,0))
					{
						StringUIData(str,8192,4608+(341*4),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							StartText();
							sprintf(st.TextInput,"%d",st.game_lightmaps[i].spot_ang[temp]);
							meng.command2=8;
							st.mouse1=0;
						}
					}
					else
						StringUIData(str,8192,4608+(341*4),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==8)
					{
						StringUIData(str,8192,4608+(341*4),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

						st.game_lightmaps[i].spot_ang[temp]=atoi(st.TextInput);

						if(st.keys[RETURN_KEY].state)
						{
							StopText();
							st.keys[RETURN_KEY].state=0;
							meng.command2=0;
						}
					}


					if(CheckCollisionMouse(8192,4608+(341*5),455,455,0))
					{
						StringUIData("Done",8192,4608+(341*5),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.sub_com=0;
							meng.command2=-1;

							if(st.game_lightmaps[i].alpha)
							{
								memcpy(meng.tmplightdata,meng.tmplightdata2,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

								if(st.game_lightmaps[i].type[temp]<4)
									AddLightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
											st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
											st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
											st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp]);
									else
										AddSpotlightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
											st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
											st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
											st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp],
											st.game_lightmaps[i].t_pos2[temp].x,st.game_lightmaps[i].t_pos2[temp].y,st.game_lightmaps[i].spot_ang[temp]);

								AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
							}
							else
							{
								memcpy(meng.tmplightdata,meng.tmplightdata2,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

								if(st.game_lightmaps[i].type[temp]<4)
									AddLightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
											st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
											st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
											st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp]);
									else
										AddSpotlightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
											st.game_lightmaps[i].color[temp].r,st.game_lightmaps[i].color[temp].g,st.game_lightmaps[i].color[temp].b,
											st.game_lightmaps[i].falloff[temp],st.game_lightmaps[i].t_pos[temp].x,st.game_lightmaps[i].t_pos[temp].y,
											st.game_lightmaps[i].t_pos[temp].z,st.game_lightmaps[i].color[temp].a,st.game_lightmaps[i].type[temp],
											st.game_lightmaps[i].t_pos2[temp].x,st.game_lightmaps[i].t_pos2[temp].y,st.game_lightmaps[i].spot_ang[temp]);

								AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
							}

							st.mouse1=0;
						}
					}
					else
						StringUIData("Done",8192,4608+(341*5),0,0,0,255,255,255,255,ARIAL,2048,2048,0);
				}
			}
			else
			if(meng.sub_com==2)
			{
				StringUIData("Select the light",8192,9216-455,0,0,0,255,128,32,255,ARIAL,4608,4608,0);

				for(j=0;j<st.game_lightmaps[i].num_lights;j++)
				{
					p3=st.game_lightmaps[i].t_pos[j];

					p3.x=(st.game_lightmaps[i].W_w*st.game_lightmaps[i].t_pos[j].x)/st.game_lightmaps[i].T_w;
					p3.y=(st.game_lightmaps[i].W_h*st.game_lightmaps[i].t_pos[j].y)/st.game_lightmaps[i].T_h;

					p3.x=p3.x+st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2);
					p3.y=p3.y+st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2);

					if(CheckCollisionMouseWorld(p3.x,p3.y,455,455,0,0) && st.mouse1)
					{
						temp=j;
						meng.temp=temp;
						st.mouse1=0;
						meng.sub_com=0;
						meng.com_id=1;

						if(!st.game_lightmaps[i].alpha)
						{
							meng.tmplightdata=malloc(st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

							meng.tmplightdata2=malloc(st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

							FillLightmap(meng.tmplightdata2,st.game_lightmaps[i].ambient_color.r,st.game_lightmaps[i].ambient_color.g,st.game_lightmaps[i].ambient_color.b,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);

							for(k=0;k<st.game_lightmaps[i].num_lights;k++)
							{
								if(k==temp) continue;
								else
								{
									if(st.game_lightmaps[i].type[k]<4)
										AddLightToLightmap(meng.tmplightdata2,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[k].r,st.game_lightmaps[i].color[k].g,st.game_lightmaps[i].color[k].b,
										st.game_lightmaps[i].falloff[k],st.game_lightmaps[i].t_pos[k].x,st.game_lightmaps[i].t_pos[k].y,
										st.game_lightmaps[i].t_pos[k].z,st.game_lightmaps[i].color[k].a,st.game_lightmaps[i].type[k]);
									else
										AddSpotlightToLightmap(meng.tmplightdata2,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[k].r,st.game_lightmaps[i].color[k].g,st.game_lightmaps[i].color[k].b,
										st.game_lightmaps[i].falloff[k],st.game_lightmaps[i].t_pos[k].x,st.game_lightmaps[i].t_pos[k].y,
										st.game_lightmaps[i].t_pos[k].z,st.game_lightmaps[i].color[k].a,st.game_lightmaps[i].type[k],
										st.game_lightmaps[i].t_pos2[k].x,st.game_lightmaps[i].t_pos2[k].y,st.game_lightmaps[i].spot_ang[k]);

									AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata2,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
								}
							}

							k=temp;

							memcpy(meng.tmplightdata,meng.tmplightdata2,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

							if(st.game_lightmaps[i].type[k]<4)
								AddLightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[k].r,st.game_lightmaps[i].color[k].g,st.game_lightmaps[i].color[k].b,
										st.game_lightmaps[i].falloff[k],st.game_lightmaps[i].t_pos[k].x,st.game_lightmaps[i].t_pos[k].y,
										st.game_lightmaps[i].t_pos[k].z,st.game_lightmaps[i].color[k].a,st.game_lightmaps[i].type[k]);
							else
								AddSpotlightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[k].r,st.game_lightmaps[i].color[k].g,st.game_lightmaps[i].color[k].b,
										st.game_lightmaps[i].falloff[k],st.game_lightmaps[i].t_pos[k].x,st.game_lightmaps[i].t_pos[k].y,
										st.game_lightmaps[i].t_pos[k].z,st.game_lightmaps[i].color[k].a,st.game_lightmaps[i].type[k],
										st.game_lightmaps[i].t_pos2[k].x,st.game_lightmaps[i].t_pos2[k].y,st.game_lightmaps[i].spot_ang[k]);

							AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}
						else
						{
							meng.tmplightdata=malloc(st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

							meng.tmplightdata2=malloc(st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

							FillAlphaLight(meng.tmplightdata2,st.game_lightmaps[i].ambient_color.r,st.game_lightmaps[i].ambient_color.g,st.game_lightmaps[i].ambient_color.b,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);

							for(k=0;k<st.game_lightmaps[i].num_lights;k++)
							{
								if(k==temp) continue;
								else
								{
									if(st.game_lightmaps[i].type[k]<4)
										AddLightToAlphaLight(meng.tmplightdata2,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[k].r,st.game_lightmaps[i].color[k].g,st.game_lightmaps[i].color[k].b,
										st.game_lightmaps[i].falloff[k],st.game_lightmaps[i].t_pos[k].x,st.game_lightmaps[i].t_pos[k].y,
										st.game_lightmaps[i].t_pos[k].z,st.game_lightmaps[i].color[k].a,st.game_lightmaps[i].type[k]);
									else
										AddSpotlightToAlphaLight(meng.tmplightdata2,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[k].r,st.game_lightmaps[i].color[k].g,st.game_lightmaps[i].color[k].b,
										st.game_lightmaps[i].falloff[k],st.game_lightmaps[i].t_pos[k].x,st.game_lightmaps[i].t_pos[k].y,
										st.game_lightmaps[i].t_pos[k].z,st.game_lightmaps[i].color[k].a,st.game_lightmaps[i].type[k],
										st.game_lightmaps[i].t_pos2[k].x,st.game_lightmaps[i].t_pos2[k].y,st.game_lightmaps[i].spot_ang[k]);

									AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata2,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
								}
							}

							k=temp;

							memcpy(meng.tmplightdata,meng.tmplightdata2,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

							if(st.game_lightmaps[i].type[k]<4)
								AddLightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[k].r,st.game_lightmaps[i].color[k].g,st.game_lightmaps[i].color[k].b,
										st.game_lightmaps[i].falloff[k],st.game_lightmaps[i].t_pos[k].x,st.game_lightmaps[i].t_pos[k].y,
										st.game_lightmaps[i].t_pos[k].z,st.game_lightmaps[i].color[k].a,st.game_lightmaps[i].type[k]);
							else
								AddSpotlightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[k].r,st.game_lightmaps[i].color[k].g,st.game_lightmaps[i].color[k].b,
										st.game_lightmaps[i].falloff[k],st.game_lightmaps[i].t_pos[k].x,st.game_lightmaps[i].t_pos[k].y,
										st.game_lightmaps[i].t_pos[k].z,st.game_lightmaps[i].color[k].a,st.game_lightmaps[i].type[k],
										st.game_lightmaps[i].t_pos2[k].x,st.game_lightmaps[i].t_pos2[k].y,st.game_lightmaps[i].spot_ang[k]);

							AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}

						break;
					}
				}
			}
		}
		else
		if(meng.pannel_choice==ADD_LIGHT && meng.command==CREATE_LIGHTMAP)
		{
			if(st.num_lightmap<MAX_LIGHTMAPS)
			{
				if(st.Current_Map.num_obj==0 || st.keys[LALT_KEY].state)
				{
					if(((meng.obj_lightmap_sel==-2 && !CheckCollisionMouseWorld(meng.lightmappos.x,meng.lightmappos.y,meng.lightmapsize.x+128,meng.lightmapsize.y+128,0,0)) || meng.obj_lightmap_sel!=-2) && temp<2)
					{
						if(st.mouse1)
						{
							meng.lightmappos.x=st.mouse.x;
							meng.lightmappos.y=st.mouse.y;

							if(meng.lightmapsize.x==0 && meng.lightmapsize.y==0)
							{
								meng.lightmapsize.x=2048;
								meng.lightmapsize.y=2048;
							}

							STW(&meng.lightmappos.x,&meng.lightmappos.y);

							meng.obj_lightmap_sel=-2;

							temp=1;

							st.mouse1=0;
						}
					}
				}

				for(j=0;j<st.Current_Map.num_obj;j++)
				{
					if(CheckCollisionMouseWorld(st.Current_Map.obj[j].position.x,st.Current_Map.obj[j].position.y,st.Current_Map.obj[j].size.x,st.Current_Map.obj[j].size.y,st.Current_Map.obj[j].angle,
						st.Current_Map.obj[j].position.z) && !st.keys[LALT_KEY].state)
					{
						if(st.mouse1 && st.Current_Map.obj[j].lightmapid==-1 && st.Current_Map.obj[j].type==MIDGROUND)
						{
							meng.obj_lightmap_sel=j;
							st.mouse1=0;
							break;
						}
					}
					else
					if(j==st.Current_Map.num_obj-1 && 
						!CheckCollisionMouseWorld(st.Current_Map.obj[j].position.x,st.Current_Map.obj[j].position.y,st.Current_Map.obj[j].size.x,st.Current_Map.obj[j].size.y,
						st.Current_Map.obj[j].angle,st.Current_Map.obj[j].position.z))
					{
						if(((meng.obj_lightmap_sel==-2 && !CheckCollisionMouseWorld(meng.lightmappos.x,meng.lightmappos.y,meng.lightmapsize.x+128,meng.lightmapsize.y+128,0,0)) || meng.obj_lightmap_sel!=-2) && temp<2)
						{
							if(st.mouse1)
							{
								meng.lightmappos.x=st.mouse.x;
								meng.lightmappos.y=st.mouse.y;

								if(meng.lightmapsize.x==0 && meng.lightmapsize.y==0)
								{
									meng.lightmapsize.x=2048;
									meng.lightmapsize.y=2048;
								}

								STW(&meng.lightmappos.x,&meng.lightmappos.y);

								meng.obj_lightmap_sel=-2;

								temp=1;

								st.mouse1=0;
								break;
							}
						}
					}
				}

				//if(meng.obj_lightmap_sel==-2)
				//{
					if(st.mouse1 && st.keys[LALT_KEY].state)
					{
						if(CheckCollisionMouseWorld(meng.lightmappos.x-(meng.lightmapsize.x/2),meng.lightmappos.y,128,meng.lightmapsize.y,0,24) && temp==1)
						{
							p=st.mouse;
							temp=2;
						}
						
						else
						if(CheckCollisionMouseWorld(meng.lightmappos.x+(meng.lightmapsize.x/2),meng.lightmappos.y,128,meng.lightmapsize.y,0,24) && temp==1)
						{
							p=st.mouse;
							temp=3;
						}
						else
						if(CheckCollisionMouseWorld(meng.lightmappos.x,meng.lightmappos.y-(meng.lightmapsize.y/2),meng.lightmapsize.x,128,0,24) && temp==1)
						{
							p=st.mouse;
							temp=4;
						}
						else
						if(CheckCollisionMouseWorld(meng.lightmappos.x,meng.lightmappos.y+(meng.lightmapsize.y/2),meng.lightmapsize.x,128,0,24) && temp==1)
						{
							p=st.mouse;
							temp=5;
						}
						else
						if(CheckCollisionMouseWorld(meng.lightmappos.x-(meng.lightmapsize.x/2),meng.lightmappos.y-(meng.lightmapsize.y/2),128,128,0,24) && temp==1)
						{
							p=st.mouse;
							asp=meng.lightmapsize.x/meng.lightmapsize.y;
							temp=6;
						}
						else
						if(CheckCollisionMouseWorld(meng.lightmappos.x+(meng.lightmapsize.x/2),meng.lightmappos.y-(meng.lightmapsize.y/2),128,128,0,24) && temp==1)
						{
							p=st.mouse;
							asp=meng.lightmapsize.x/meng.lightmapsize.y;
							temp=7;
						}
						else
						if(CheckCollisionMouseWorld(meng.lightmappos.x+(meng.lightmapsize.x/2),meng.lightmappos.y+(meng.lightmapsize.y/2),128,128,0,24) && temp==1)
						{
							p=st.mouse;
							asp=meng.lightmapsize.x/meng.lightmapsize.y;
							temp=8;
						}
						else
						if(CheckCollisionMouseWorld(meng.lightmappos.x-(meng.lightmapsize.x/2),meng.lightmappos.y+(meng.lightmapsize.y/2),128,128,0,24) && temp==1)
						{
							p=st.mouse;
							asp=meng.lightmapsize.x/meng.lightmapsize.y;
							temp=9;
						}

						if(temp==2)
						{
							p2=st.mouse;

							STW(&p2.x,&p2.y);
							STW(&p.x,&p.y);

							p2.x-=p.x;
							p2.y-=p.y;

							meng.lightmapsize.x-=p2.x*2;

							p=st.mouse;
						}
						else
						if(temp==3)
						{
							p2=st.mouse;

							STW(&p2.x,&p2.y);
							STW(&p.x,&p.y);

							p2.x-=p.x;
							p2.y-=p.y;

							meng.lightmapsize.x+=p2.x*2;

							p=st.mouse;
						}
						else
						if(temp==4)
						{
							p2=st.mouse;

							STW(&p2.x,&p2.y);
							STW(&p.x,&p.y);

							p2.x-=p.x;
							p2.y-=p.y;

							meng.lightmapsize.y-=p2.y*2;

							p=st.mouse;
						}
						else
						if(temp==5)
						{
							p2=st.mouse;

							STW(&p2.x,&p2.y);
							STW(&p.x,&p.y);

							p2.x-=p.x;
							p2.y-=p.y;

							meng.lightmapsize.y+=p2.y*2;

							p=st.mouse;
						}
						else
						if(temp==6)
						{
							p2=st.mouse;

							STW(&p2.x,&p2.y);
							STW(&p.x,&p.y);

							p2.x-=p.x;
							p2.y-=p.y;

							
							meng.lightmapsize.x-=p2.x*2;
							meng.lightmapsize.y-=p2.y*2;
							
							meng.lightmapsize.y=(float) meng.lightmapsize.x/asp;

							p=st.mouse;
						}
						else
						if(temp==7)
						{
							p2=st.mouse;

							STW(&p2.x,&p2.y);
							STW(&p.x,&p.y);

							p2.x-=p.x;
							p2.y-=p.y;

							
							meng.lightmapsize.x+=p2.x*2;
							meng.lightmapsize.y-=p2.y*2;
						
							meng.lightmapsize.y=(float) meng.lightmapsize.x/asp;

							p=st.mouse;
						}
						else
						if(temp==8)
						{
							p2=st.mouse;

							STW(&p2.x,&p2.y);
							STW(&p.x,&p.y);

							p2.x-=p.x;
							p2.y-=p.y;

						
							meng.lightmapsize.x+=p2.x*2;
							meng.lightmapsize.y+=p2.y*2;
							
							meng.lightmapsize.y=(float) meng.lightmapsize.x/asp;

							p=st.mouse;
						}
						else
						if(temp==9)
						{
							p2=st.mouse;

							STW(&p2.x,&p2.y);
							STW(&p.x,&p.y);

							p2.x-=p.x;
							p2.y-=p.y;

							
							meng.lightmapsize.x-=p2.x*2;
							meng.lightmapsize.y+=p2.y*2;

							meng.lightmapsize.y=(float) meng.lightmapsize.x/asp;

							p=st.mouse;
						}
					}
					else
						temp=1;
				//}

				if(st.keys[RETURN_KEY].state && meng.obj_lightmap_sel>-1)
				{
					for(i=0;i<MAX_LIGHTMAPS;i++)
					{
						if(!st.game_lightmaps[i].stat)
						{
							meng.current_command=2;

							st.game_lightmaps[i].stat=2;
							
							meng.command=RGB_LIGHTMAP;
							meng.command2=i;

							st.Current_Map.obj[meng.obj_lightmap_sel].lightmapid=i;

							st.game_lightmaps[i].obj_id=meng.obj_lightmap_sel;

							st.game_lightmaps[i].w_pos=st.Current_Map.obj[meng.obj_lightmap_sel].position;

							st.game_lightmaps[i].W_w=st.Current_Map.obj[meng.obj_lightmap_sel].size.x;
							st.game_lightmaps[i].W_h=st.Current_Map.obj[meng.obj_lightmap_sel].size.y;

							tmp=(float) st.Current_Map.obj[meng.obj_lightmap_sel].size.x/st.Current_Map.obj[meng.obj_lightmap_sel].size.y;

							if(tmp>2) 
							{
								tmp/=2.0f;
								st.game_lightmaps[i].T_w=meng.lightmap_res.x*tmp;
								st.game_lightmaps[i].T_h=meng.lightmap_res.x;
							}
							else
							{
								st.game_lightmaps[i].T_w=meng.lightmap_res.x;
								st.game_lightmaps[i].T_h=meng.lightmap_res.x/tmp;
							}

							st.game_lightmaps[i].num_lights=0;
							meng.lightmap_color.r=meng.lightmap_color.g=meng.lightmap_color.b=0;
							st.game_lightmaps[i].alpha=0;

							LogApp("Lightmap Creation Step 2");
							st.mouse1=0;
							temp=0;

							break;
						}
					}
				}
				else
				if(st.keys[RETURN_KEY].state && meng.obj_lightmap_sel==-2)
				{
					for(i=0;i<MAX_LIGHTMAPS;i++)
					{
						if(!st.game_lightmaps[i].stat)
						{
							st.game_lightmaps[i].stat=2;

							meng.current_command=2;
							
							meng.command=RGB_LIGHTMAP;
							meng.command2=i;

							st.game_lightmaps[i].obj_id=-1;

							st.game_lightmaps[i].w_pos=meng.lightmappos;

							st.game_lightmaps[i].W_w=meng.lightmapsize.x;
							st.game_lightmaps[i].W_h=meng.lightmapsize.y;

							tmp=(float) meng.lightmapsize.x/meng.lightmapsize.y;

							if(tmp>1) 
							{
								st.game_lightmaps[i].T_w=meng.lightmap_res.x*tmp;
								st.game_lightmaps[i].T_h=meng.lightmap_res.x;
							}
							else
							{
								st.game_lightmaps[i].T_w=meng.lightmap_res.x;
								st.game_lightmaps[i].T_h=meng.lightmap_res.x*tmp;
							}

							st.game_lightmaps[i].num_lights=0;
							meng.lightmap_color.r=meng.lightmap_color.g=meng.lightmap_color.b=0;
							st.game_lightmaps[i].alpha=0;

							LogApp("Lightmap Creation Step 2");
							st.mouse1=0;
							temp=0;

							break;
						}
					}
				}
			}
		}
		else
		if(meng.pannel_choice==ADD_LIGHT && meng.command==RGB_LIGHTMAP)
		{
			i=meng.command2;

			UIData(8192,4608,2048,2730,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,7);

			Sys_ColorPicker(&meng.lightmap_color.r,&meng.lightmap_color.g,&meng.lightmap_color.b);

			sprintf(str,"R %d",meng.lightmap_color.r);

			if(CheckCollisionMouse(8192,4608-(341*2),341,341,0))
			{
				StringUIData(str,8192,4608-(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					StartText();
					sprintf(st.TextInput,"%d",meng.lightmap_color.r);
					meng.got_it=1;
					st.mouse1=0;
				}
			}
			else
				StringUIData(str,8192,4608-(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

			if(meng.got_it==1)
			{
				StringUIData(str,8192,4608-(341*2),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.lightmap_color.r=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.got_it=0;
				}
			}

			sprintf(str,"G %d",meng.lightmap_color.g);

			if(CheckCollisionMouse(8192,4608-(341),341,341,0))
			{
				StringUIData(str,8192,4608-(341),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					StartText();
					sprintf(st.TextInput,"%d",meng.lightmap_color.g);
					meng.got_it=2;
					st.mouse1=0;
				}
			}
			else
				StringUIData(str,8192,4608-(341),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

			if(meng.got_it==2)
			{
				StringUIData(str,8192,4608-(341),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.lightmap_color.g=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.got_it=0;
				}
			}

			sprintf(str,"B %d",meng.lightmap_color.b);

			if(CheckCollisionMouse(8192,4608,341,341,0))
			{
				StringUIData(str,8192,4608,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					StartText();
					sprintf(st.TextInput,"%d",meng.lightmap_color.b);
					meng.got_it=3;
					st.mouse1=0;
				}
			}
			else
				StringUIData(str,8192,4608,0,0,0,255,255,255,255,ARIAL,2048,2048,0);

			if(meng.got_it==3)
			{
				StringUIData(str,8192,4608,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.lightmap_color.b=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.got_it=0;
				}
			}

			sprintf(str,"Res. Width %d",st.game_lightmaps[i].T_w);

			if(CheckCollisionMouse(8192,4608+(341),341,341,0))
			{
				StringUIData(str,8192,4608+(341),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					StartText();
					sprintf(st.TextInput,"%d",st.game_lightmaps[i].T_w);
					meng.got_it=4;
					st.mouse1=0;
				}
			}
			else
				StringUIData(str,8192,4608+(341),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

			if(meng.got_it==4)
			{
				StringUIData(str,8192,4608+(341),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				st.game_lightmaps[i].T_w=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.got_it=0;
				}
			}

			sprintf(str,"Res. Height %d",st.game_lightmaps[i].T_h);

			if(CheckCollisionMouse(8192,4608+(341*2),341,341,0))
			{
				StringUIData(str,8192,4608+(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					StartText();
					sprintf(st.TextInput,"%d",st.game_lightmaps[i].T_h);
					meng.got_it=5;
					st.mouse1=0;
				}
			}
			else
				StringUIData(str,8192,4608+(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

			if(meng.got_it==5)
			{
				StringUIData(str,8192,4608+(341*2),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				st.game_lightmaps[i].T_h=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.got_it=0;
				}
			}

			if(CheckCollisionMouse(8192,4608+(341*3),455,455,0))
			{
				StringUIData("Done",8192,4608+(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					meng.command=ADD_LIGHT_TO_LIGHTMAP;

					st.game_lightmaps[i].ambient_color=meng.lightmap_color;

					if(st.game_lightmaps[i].alpha)
					{
						st.game_lightmaps[i].data=GenerateAlphaLight(st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						FillAlphaLight(st.game_lightmaps[i].data,meng.lightmap_color.r,meng.lightmap_color.g,meng.lightmap_color.b,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						st.game_lightmaps[i].tex=GenerateAlphaLightTexture(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);

						meng.tmplightdata=malloc(st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);
						memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);
					}
					else
					{
						st.game_lightmaps[i].data=GenerateLightmap(st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						FillLightmap(st.game_lightmaps[i].data,meng.lightmap_color.r,meng.lightmap_color.g,meng.lightmap_color.b,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						st.game_lightmaps[i].tex=GenerateLightmapTexture(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);

						meng.tmplightdata=malloc(st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);
						memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);
					}

					st.game_lightmaps[i].num_lights++;
					st.game_lightmaps[i].t_pos[0].x=st.game_lightmaps[i].T_w/2;
					st.game_lightmaps[i].t_pos[0].y=st.game_lightmaps[i].T_h/2;
					st.game_lightmaps[i].t_pos[0].z=0;

					meng.light.light_id=0;
					meng.light.falloff=16.0f;
					meng.light.color.r=255;
					meng.light.color.g=255;
					meng.light.color.b=255;
					meng.light.intensity=255.0f;
					meng.light.type=POINT_LIGHT_MEDIUM;
					st.game_lightmaps[i].t_pos2[meng.light.light_id].x=0;
					st.game_lightmaps[i].t_pos2[meng.light.light_id].y=0;
					st.game_lightmaps[i].color[meng.light.light_id].r=255;
					st.game_lightmaps[i].color[meng.light.light_id].g=255;
					st.game_lightmaps[i].color[meng.light.light_id].b=255;
					st.game_lightmaps[i].color[meng.light.light_id].a=255;
					st.game_lightmaps[i].falloff[meng.light.light_id]=16.0f;
					st.game_lightmaps[i].spot_ang[meng.light.light_id]=30;
					st.game_lightmaps[i].type[meng.light.light_id]=POINT_LIGHT_MEDIUM;

					st.game_lightmaps[i].stat=1;
					st.num_lights++;

					if(st.game_lightmaps[i].alpha)
					{
						AddLightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,meng.light.color.r,meng.light.color.g,meng.light.color.b,meng.light.falloff,
							st.game_lightmaps[i].t_pos[0].x,st.game_lightmaps[i].t_pos[0].y,st.game_lightmaps[i].t_pos[meng.light.light_id].z,meng.light.intensity,meng.light.type);

						AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
					}
					else
					{
						AddLightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,meng.light.color.r,meng.light.color.g,meng.light.color.b,meng.light.falloff,
							st.game_lightmaps[i].t_pos[0].x,st.game_lightmaps[i].t_pos[0].y,st.game_lightmaps[i].t_pos[meng.light.light_id].z,meng.light.intensity,meng.light.type);

						AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
					}

					meng.com_id=0;
					meng.got_it=-1;
					st.mouse1=0;
				}
			}
			else
				StringUIData("Done",8192,4608+(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);
		}
		else
		if(meng.pannel_choice==ADD_LIGHT && meng.command==ADD_LIGHT_TO_LIGHTMAP)
		{
			i=meng.command2;

			st.game_lightmaps[i].falloff[meng.light.light_id]=meng.light.falloff;
			st.game_lightmaps[i].color[meng.light.light_id].a=meng.light.intensity;
			st.game_lightmaps[i].color[meng.light.light_id].r=meng.light.color.r;
			st.game_lightmaps[i].color[meng.light.light_id].g=meng.light.color.g;
			st.game_lightmaps[i].color[meng.light.light_id].b=meng.light.color.b;
			st.game_lightmaps[i].type[meng.light.light_id]=meng.light.type;

			p3=st.game_lightmaps[i].t_pos[meng.light.light_id];

			p3.x=(st.game_lightmaps[i].W_w*st.game_lightmaps[i].t_pos[meng.light.light_id].x)/st.game_lightmaps[i].T_w;
			p3.y=(st.game_lightmaps[i].W_h*st.game_lightmaps[i].t_pos[meng.light.light_id].y)/st.game_lightmaps[i].T_h;

			p3.x=p3.x+st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2);
			p3.y=p3.y+st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2);

			if(CheckCollisionMouseWorld(st.game_lightmaps[i].w_pos.x,st.game_lightmaps[i].w_pos.y,st.game_lightmaps[i].W_w,st.game_lightmaps[i].W_h,st.game_lightmaps[i].ang,24) && st.mouse1)
			{
				meng.loop_complete=1;

				p=st.mouse;

				STW(&p.x,&p.y);

				p.x=p.x-(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2));
				p.y=p.y-(st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2));

				p.x=(st.game_lightmaps[i].T_w*p.x)/st.game_lightmaps[i].W_w;
				p.y=(st.game_lightmaps[i].T_h*p.y)/st.game_lightmaps[i].W_h;

				st.game_lightmaps[i].t_pos[meng.light.light_id].x=p.x;
				st.game_lightmaps[i].t_pos[meng.light.light_id].y=p.y;

				if(st.game_lightmaps[i].alpha)
				{
					memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

					if(st.game_lightmaps[i].type[meng.light.light_id]<4)
						AddLightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
								st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
								st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
								st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id]);
						else
							AddSpotlightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
								st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
								st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
								st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id],
								st.game_lightmaps[i].t_pos2[meng.light.light_id].x,st.game_lightmaps[i].t_pos2[meng.light.light_id].y,st.game_lightmaps[i].spot_ang[meng.light.light_id]);

					AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
				}
				else
				{
					memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

					if(st.game_lightmaps[i].type[meng.light.light_id]<4)
						AddLightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
								st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
								st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
								st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id]);
						else
							AddSpotlightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
								st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
								st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
								st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id],
								st.game_lightmaps[i].t_pos2[meng.light.light_id].x,st.game_lightmaps[i].t_pos2[meng.light.light_id].y,st.game_lightmaps[i].spot_ang[meng.light.light_id]);

					AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
				}
			}

			if(st.mouse1)
			{
				meng.loop_complete=1;

				if(st.mouse_wheel>0)
				{
					st.mouse_wheel=0;
					meng.light.falloff+=0.1;
				}
				else
				if(st.mouse_wheel<0)
				{
					st.mouse_wheel=0;
					meng.light.falloff-=0.1;
				}

				if(st.game_lightmaps[i].alpha)
				{
					memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

					if(st.game_lightmaps[i].type[meng.light.light_id]<4)
						AddLightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
								st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
								st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
								st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id]);
						else
							AddSpotlightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
								st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
								st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
								st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id],
								st.game_lightmaps[i].t_pos2[meng.light.light_id].x,st.game_lightmaps[i].t_pos2[meng.light.light_id].y,st.game_lightmaps[i].spot_ang[meng.light.light_id]);

					AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
				}
				else
				{
					memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

					if(st.game_lightmaps[i].type[meng.light.light_id]<4)
						AddLightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
								st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
								st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
								st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id]);
						else
							AddSpotlightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
								st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
								st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
								st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id],
								st.game_lightmaps[i].t_pos2[meng.light.light_id].x,st.game_lightmaps[i].t_pos2[meng.light.light_id].y,st.game_lightmaps[i].spot_ang[meng.light.light_id]);

					AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
				}

			}

			if(st.keys[LSHIFT_KEY].state && st.game_lightmaps[i].type[meng.light.light_id]>3)
			{
				meng.loop_complete=1;

				if(st.mouse_wheel>0)
				{
					st.game_lightmaps[i].spot_ang[meng.light.light_id]++;
					st.mouse_wheel=0;
				}
				else
				if(st.mouse_wheel<0)
				{
					st.game_lightmaps[i].spot_ang[meng.light.light_id]--;
					st.mouse_wheel=0;
				}


				if(st.game_lightmaps[i].alpha)
				{
					memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

					AddSpotlightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
								st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
								st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
								st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id],
								st.game_lightmaps[i].t_pos2[meng.light.light_id].x,st.game_lightmaps[i].t_pos2[meng.light.light_id].y,st.game_lightmaps[i].spot_ang[meng.light.light_id]);

					AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
				}
				else
				{
					memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

					AddSpotlightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
								st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
								st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
								st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id],
								st.game_lightmaps[i].t_pos2[meng.light.light_id].x,st.game_lightmaps[i].t_pos2[meng.light.light_id].y,st.game_lightmaps[i].spot_ang[meng.light.light_id]);

					AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
				}

				st.keys[LSHIFT_KEY].state=0;
			}

			if(st.mouse2)
			{
				meng.loop_complete=1;

				if(st.mouse_wheel>0)
				{
					st.mouse_wheel=0;
					st.game_lightmaps[i].t_pos[meng.light.light_id].z+=2;
				}
				else
				if(st.mouse_wheel<0)
				{
					st.mouse_wheel=0;
					st.game_lightmaps[i].t_pos[meng.light.light_id].z-=2;
				}

				if(st.game_lightmaps[i].type[meng.light.light_id]>3)
				{
					p=st.mouse;

					STW(&p.x,&p.y);

					p.x=p.x-(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2));
					p.y=p.y-(st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2));

					p.x=(st.game_lightmaps[i].T_w*p.x)/st.game_lightmaps[i].W_w;
					p.y=(st.game_lightmaps[i].T_h*p.y)/st.game_lightmaps[i].W_h;

					st.game_lightmaps[i].t_pos2[meng.light.light_id].x=p.x;
					st.game_lightmaps[i].t_pos2[meng.light.light_id].y=p.y;
				}

				if(st.game_lightmaps[i].alpha)
				{
					memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

					if(st.game_lightmaps[i].type[meng.light.light_id]<4)
						AddLightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
								st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
								st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
								st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id]);
						else
							AddSpotlightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
								st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
								st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
								st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id],
								st.game_lightmaps[i].t_pos2[meng.light.light_id].x,st.game_lightmaps[i].t_pos2[meng.light.light_id].y,st.game_lightmaps[i].spot_ang[meng.light.light_id]);

					AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
				}
				else
				{
					memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

					if(st.game_lightmaps[i].type[meng.light.light_id]<4)
						AddLightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
								st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
								st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
								st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id]);
						else
							AddSpotlightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
								st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
								st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
								st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id],
								st.game_lightmaps[i].t_pos2[meng.light.light_id].x,st.game_lightmaps[i].t_pos2[meng.light.light_id].y,st.game_lightmaps[i].spot_ang[meng.light.light_id]);

					AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
				}
			}

			p=st.game_lightmaps[i].w_pos;

			p.x+=(st.game_lightmaps[i].W_w/2)+1920;

			if(meng.com_id!=1)
			{
				if(CheckCollisionMouseWorld(p.x,p.y,455,455,0,0))
				{
					String2Data("Done",p.x,p.y,0,0,0,255,128,32,255,ARIAL,1536,1536,24);

					if(st.mouse1)
					{
						meng.com_id=0;
						meng.got_it=-1;
						st.game_lightmaps[i].num_lights--;
						meng.command=meng.pannel_choice=meng.command2=ADD_LIGHT;

						meng.current_command=0;

						if(st.game_lightmaps[i].alpha)
							AddLightToAlphaTexture(&st.game_lightmaps[i].tex,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						else
							AddLightToTexture(&st.game_lightmaps[i].tex,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						free(meng.tmplightdata);
						st.mouse1=0;
					}
				}
				else
					String2Data("Done",p.x,p.y,0,0,0,255,255,255,255,ARIAL,1536,1536,24);

				p=st.game_lightmaps[i].w_pos;

				p.y-=(st.game_lightmaps[i].W_h/2)+810;

				if(CheckCollisionMouseWorld(p.x,p.y,455,455,0,0))
				{
					String2Data("Edit light",p.x,p.y,0,0,0,255,128,32,255,ARIAL,1536,1536,24);

					if(st.mouse1)
					{
						meng.com_id=1;
						meng.got_it=0;
						st.mouse1=0;
					}
				}
				else
					String2Data("Edit light",p.x,p.y,0,0,0,255,255,255,255,ARIAL,1536,1536,24);

				p=st.game_lightmaps[i].w_pos;

				p.y+=(st.game_lightmaps[i].W_h/2)+810;

				if(CheckCollisionMouseWorld(p.x,p.y,455,455,0,0))
				{
					String2Data("Add Light",p.x,p.y,0,0,0,255,128,32,255,ARIAL,1536,1536,24);

					if(st.mouse1)
					{
						if(st.game_lightmaps[i].alpha)
						{
							if(st.game_lightmaps[i].type[meng.light.light_id]<4)
								AddLightToAlphaLight(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
										st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
										st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id]);
								else
									AddSpotlightToAlphaLight(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
										st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
										st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id],
										st.game_lightmaps[i].t_pos2[meng.light.light_id].x,st.game_lightmaps[i].t_pos2[meng.light.light_id].y,st.game_lightmaps[i].spot_ang[meng.light.light_id]);

							AddLightToAlphaTexture(&st.game_lightmaps[i].tex,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}
						else
						{
							if(st.game_lightmaps[i].type[meng.light.light_id]<4)
								AddLightToLightmap(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
										st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
										st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id]);
								else
									AddSpotlightToLightmap(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
										st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
										st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id],
										st.game_lightmaps[i].t_pos2[meng.light.light_id].x,st.game_lightmaps[i].t_pos2[meng.light.light_id].y,st.game_lightmaps[i].spot_ang[meng.light.light_id]);

							AddLightToTexture(&st.game_lightmaps[i].tex,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}

						meng.light.light_id++;

						st.game_lightmaps[i].num_lights++;
						st.game_lightmaps[i].t_pos[meng.light.light_id].x=st.game_lightmaps[i].T_w/2;
						st.game_lightmaps[i].t_pos[meng.light.light_id].y=st.game_lightmaps[i].T_h/2;
						st.game_lightmaps[i].t_pos[meng.light.light_id].z=0;
						st.game_lightmaps[i].t_pos2[meng.light.light_id].x=0;
						st.game_lightmaps[i].t_pos2[meng.light.light_id].y=0;
						st.game_lightmaps[i].color[meng.light.light_id].r=255;
						st.game_lightmaps[i].color[meng.light.light_id].g=255;
						st.game_lightmaps[i].color[meng.light.light_id].b=255;
						st.game_lightmaps[i].color[meng.light.light_id].a=255;
						st.game_lightmaps[i].falloff[meng.light.light_id]=16.0f;
						st.game_lightmaps[i].spot_ang[meng.light.light_id]=30;
						st.game_lightmaps[i].type[meng.light.light_id]=POINT_LIGHT_MEDIUM;

						meng.light.falloff=16.0f;
						meng.light.color.r=255;
						meng.light.color.g=255;
						meng.light.color.b=255;
						meng.light.intensity=255;
						meng.light.type=POINT_LIGHT_MEDIUM;

						if(st.game_lightmaps[i].alpha)
						{
							memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

							AddLightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,meng.light.color.r,meng.light.color.g,meng.light.color.b,meng.light.falloff,
								st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,st.game_lightmaps[i].t_pos[meng.light.light_id].z,meng.light.intensity,meng.light.type);
							AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}
						else
						{
							memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

							AddLightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,meng.light.color.r,meng.light.color.g,meng.light.color.b,meng.light.falloff,
								st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,st.game_lightmaps[i].t_pos[meng.light.light_id].z,meng.light.intensity,meng.light.type);
							AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}

						LogApp("Light added");

						st.mouse1=0;
					}
				}
				else
					String2Data("Add Light",p.x,p.y,0,0,0,255,255,255,255,ARIAL,1536,1536,24);
			}

			if(meng.com_id==1)
			{
				//String2Data("Edit light",p.x,p.y,0,0,0,255,32,32,255,ARIAL,1536,1536,0);

				Sys_ColorPicker(&meng.light.color.r,&meng.light.color.g,&meng.light.color.b);

				UIData(8192,4300,3072,3900,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,7);

				sprintf(str,"R %d",meng.light.color.r);

				if(CheckCollisionMouse(8192,4608-(341*3),341,341,0))
				{
					StringUIData(str,8192,4608-(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						StartText();
						sprintf(st.TextInput,"%d",meng.light.color.r);
						meng.got_it=1;
						st.mouse1=0;
					}
				}
				else
					StringUIData(str,8192,4608-(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==1)
				{
					StringUIData(str,8192,4608-(341*3),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					meng.light.color.r=atoi(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.got_it=0;
					}
				}

				sprintf(str,"G %d",meng.light.color.g);

				if(CheckCollisionMouse(8192,4608-(341*2),341,341,0))
				{
					StringUIData(str,8192,4608-(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						StartText();
						sprintf(st.TextInput,"%d",meng.light.color.g);
						meng.got_it=2;
						st.mouse1=0;
					}
				}
				else
					StringUIData(str,8192,4608-(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==2)
				{
					StringUIData(str,8192,4608-(341*2),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					meng.light.color.g=atoi(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.got_it=0;
					}
				}

				sprintf(str,"B %d",meng.light.color.b);

				if(CheckCollisionMouse(8192,4608-341,341,341,0))
				{
					StringUIData(str,8192,4608-341,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						StartText();
						sprintf(st.TextInput,"%d",meng.light.color.b);
						meng.got_it=3;
						st.mouse1=0;
					}
				}
				else
					StringUIData(str,8192,4608-341,0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==3)
				{
					StringUIData(str,8192,4608-341,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					meng.light.color.b=atoi(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.got_it=0;
					}
				}

				if(meng.got_it!=4)
					sprintf(str,"Intensity %d",meng.light.intensity);
				else
					strcpy(str,st.TextInput);

				if(CheckCollisionMouse(8192,4608,341,341,0))
				{
					StringUIData(str,8192,4608,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						StartText();
						sprintf(st.TextInput,"%d",meng.light.intensity);
						meng.got_it=4;
						st.mouse1=0;
					}
				}
				else
					StringUIData(str,8192,4608,0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==4)
				{
					StringUIData(str,8192,4608,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					meng.light.intensity=atoi(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.got_it=0;
					}
				}

				if(meng.got_it!=5)
					sprintf(str,"Fall Off %.3f",meng.light.falloff);
				else
					strcpy(str,st.TextInput);

				if(CheckCollisionMouse(8192,4608+341,341,341,0))
				{
					StringUIData(str,8192,4608+341,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						StartText();
						sprintf(st.TextInput,"%.3f",meng.light.falloff);
						meng.got_it=5;
						st.mouse1=0;
					}
				}
				else
					StringUIData(str,8192,4608+341,0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==5)
				{
					StringUIData(str,8192,4608+341,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					meng.light.falloff=atof(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.got_it=0;
					}
				}

				sprintf(str,"Z %d",st.game_lightmaps[i].t_pos[meng.light.light_id].z);

				if(CheckCollisionMouse(8192,4608+(341*2),341,341,0))
				{
					StringUIData(str,8192,4608+(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						StartText();
						sprintf(st.TextInput,"%d",st.game_lightmaps[i].t_pos[meng.light.light_id].z);
						meng.got_it=6;
						st.mouse1=0;
					}
				}
				else
					StringUIData(str,8192,4608+(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==6)
				{
					StringUIData(str,8192,4608+(341*2),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					st.game_lightmaps[i].t_pos[meng.light.light_id].z=atoi(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.got_it=0;
					}
				}

				if(meng.light.type==POINT_LIGHT_MEDIUM) strcpy(str,"Point medium");
				else if(meng.light.type==POINT_LIGHT_STRONG) strcpy(str,"Point strong");
				else if(meng.light.type==POINT_LIGHT_NORMAL) strcpy(str,"Point normal");
				else if(meng.light.type==SPOTLIGHT_MEDIUM) strcpy(str,"Spotlight medium");
				else if(meng.light.type==SPOTLIGHT_STRONG) strcpy(str,"Spotlight strong");
				else if(meng.light.type==SPOTLIGHT_NORMAL) strcpy(str,"Spotlight normal");

				if(CheckCollisionMouse(8192,4608+(341*3),341,341,0))
				{
					StringUIData(str,8192,4608+(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						meng.got_it=7;
						st.mouse1=0;
					}
				}
				else
					StringUIData(str,8192,4608+(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==7)
				{
					UIData(11264,4608,3072,3072,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

					if(CheckCollisionMouse(11264,4608-(341*3),341,341,0))
					{
						StringUIData("Point medium",11264,4608-(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.light.type=POINT_LIGHT_MEDIUM;
							meng.got_it=-1;
							st.mouse1=0;
						}
					}
					else
						StringUIData("Point medium",11264,4608-(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(CheckCollisionMouse(11264,4608-(341*2),341,341,0))
					{
						StringUIData("Point strong",11264,4608-(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.light.type=POINT_LIGHT_STRONG;
							meng.got_it=-1;
							st.mouse1=0;
						}
					}
					else
						StringUIData("Point strong",11264,4608-(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(CheckCollisionMouse(11264,4608-(341),341,341,0))
					{
						StringUIData("Point normal",11264,4608-(341),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.light.type=POINT_LIGHT_NORMAL;
							meng.got_it=-1;
							st.mouse1=0;
						}
					}
					else
						StringUIData("Point normal",11264,4608-(341),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(CheckCollisionMouse(11264,4608+(341),341,341,0))
					{
						StringUIData("Spotlight medium",11264,4608+(341),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.light.type=SPOTLIGHT_MEDIUM;
							meng.got_it=-1;
							st.mouse1=0;
						}
					}
					else
						StringUIData("Spotlight medium",11264,4608+(341),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(CheckCollisionMouse(11264,4608+(341*2),341,341,0))
					{
						StringUIData("Spotlight strong",11264,4608+(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.light.type=SPOTLIGHT_STRONG;
							meng.got_it=-1;
							st.mouse1=0;
						}
					}
					else
						StringUIData("Spotlight Strong",11264,4608+(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(CheckCollisionMouse(11264,4608+(341*3),341,341,0))
					{
						StringUIData("Spotlight normal",11264,4608+(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.light.type=SPOTLIGHT_NORMAL;
							meng.got_it=-1;
							st.mouse1=0;
						}
					}
					else
						StringUIData("Spotlight normal",11264,4608+(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(st.keys[RETURN_KEY].state)
					{
						meng.got_it=-1;
						st.keys[RETURN_KEY].state=0;
					}
				}

				sprintf(str,"Spot angle %d",st.game_lightmaps[i].spot_ang[meng.light.light_id]);

				if(CheckCollisionMouse(8192,4608+(341*4),341,341,0))
				{
					StringUIData(str,8192,4608+(341*4),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						StartText();
						sprintf(st.TextInput,"%d",st.game_lightmaps[i].spot_ang[meng.light.light_id]);
						meng.got_it=8;
						st.mouse1=0;
					}
				}
				else
					StringUIData(str,8192,4608+(341*4),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==8)
				{
					StringUIData(str,8192,4608+(341*4),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					st.game_lightmaps[i].spot_ang[meng.light.light_id]=atoi(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.got_it=0;
					}
				}


				if(CheckCollisionMouse(8192,4608+(341*5),455,455,0))
				{
					StringUIData("Done",8192,4608+(341*5),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						meng.com_id=0;
						meng.got_it=-1;

						if(st.game_lightmaps[i].alpha)
						{
							memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

							if(st.game_lightmaps[i].type[meng.light.light_id]<4)
								AddLightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
										st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
										st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id]);
								else
									AddSpotlightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
										st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
										st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id],
										st.game_lightmaps[i].t_pos2[meng.light.light_id].x,st.game_lightmaps[i].t_pos2[meng.light.light_id].y,st.game_lightmaps[i].spot_ang[meng.light.light_id]);

							AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}
						else
						{
							memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

							if(st.game_lightmaps[i].type[meng.light.light_id]<4)
								AddLightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
										st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
										st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id]);
								else
									AddSpotlightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,
										st.game_lightmaps[i].color[meng.light.light_id].r,st.game_lightmaps[i].color[meng.light.light_id].g,st.game_lightmaps[i].color[meng.light.light_id].b,
										st.game_lightmaps[i].falloff[meng.light.light_id],st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,
										st.game_lightmaps[i].t_pos[meng.light.light_id].z,st.game_lightmaps[i].color[meng.light.light_id].a,st.game_lightmaps[i].type[meng.light.light_id],
										st.game_lightmaps[i].t_pos2[meng.light.light_id].x,st.game_lightmaps[i].t_pos2[meng.light.light_id].y,st.game_lightmaps[i].spot_ang[meng.light.light_id]);

							AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}

						st.mouse1=0;
					}
				}
				else
					StringUIData("Done",8192,4608+(341*5),0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
		}
	}

	if(meng.command!=TEX_SIZE_OBJ && meng.command!=TEX_PAN_OBJ && meng.command!=OBJ_AMBL && meng.command!=RGB_OBJ && meng.command!=OBJ_EDIT_BOX && !st.Text_Input)
	{
		if(st.keys[W_KEY].state)
		{
			st.Camera.position.y-=64*delta;
		}

		if(st.keys[S_KEY].state)
		{
			st.Camera.position.y+=64*delta;
		}

		if(st.keys[D_KEY].state)
		{
			st.Camera.position.x+=64*delta;
		}

		if(st.keys[A_KEY].state)
		{
			st.Camera.position.x-=64*delta;
		}

		if(meng.command!=ADD_LIGHT_TO_LIGHTMAP && meng.command!=EDIT_LIGHTMAP2  && meng.command!=MOVE_LIGHTMAP && meng.command!=MGG_LOAD && meng.sub_com<100
			&& meng.command != SPRITE_SELECTION && meng.command != TEX_SEL)
		{
			if(st.mouse_wheel>0 && st.keys[LALT_KEY].state)
			{
				if(st.Camera.dimension.x<6) st.Camera.dimension.x+=0.1;
				if(st.Camera.dimension.y<6) st.Camera.dimension.y+=0.1;
				st.mouse_wheel=0;
			}

			if (st.mouse_wheel<0 && st.keys[LALT_KEY].state)
			{
				if(st.Camera.dimension.x>0.2) st.Camera.dimension.x-=0.1;
				if(st.Camera.dimension.y>0.2) st.Camera.dimension.y-=0.1;
				st.mouse_wheel=0;
			}
		}
		else
		if(meng.command==ADD_LIGHT_TO_LIGHTMAP || meng.command==EDIT_LIGHTMAP2 || meng.command==MOVE_LIGHTMAP)
		{
			if(st.mouse_wheel>0 && !st.mouse1 && !st.mouse2 && !st.keys[LSHIFT_KEY].state && !st.keys[LCTRL_KEY].state)
			{
				if(st.Camera.dimension.x<6) st.Camera.dimension.x+=0.1;
				if(st.Camera.dimension.y<6) st.Camera.dimension.y+=0.1;
				st.mouse_wheel=0;
			}

			if(st.mouse_wheel<0 && !st.mouse1 && !st.mouse2 && !st.keys[LSHIFT_KEY].state && !st.keys[LCTRL_KEY].state)
			{
				if(st.Camera.dimension.x>0.2) st.Camera.dimension.x-=0.1;
				if(st.Camera.dimension.y>0.2) st.Camera.dimension.y-=0.1;
				st.mouse_wheel=0;
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
	int16 i=0, j=0;

	TEX_DATA texture;

	int32 dist2;

	float dist;

	Pos p;
	uPos16 p2;

	if(meng.viewmode!=INGAMEVIEW_MODE)
	{
		if(st.Current_Map.cam_area.horiz_lim)
		{
			DrawLine((float)(st.Current_Map.cam_area.limit[0].x-st.Camera.position.x)*st.Camera.dimension.x,0,(float)(st.Current_Map.cam_area.limit[0].x-st.Camera.position.x)*st.Camera.dimension.x,9216,230,255,0,255,64,15);
			DrawLine((float)(st.Current_Map.cam_area.limit[1].x-st.Camera.position.x)*st.Camera.dimension.x,0,(float)(st.Current_Map.cam_area.limit[1].x-st.Camera.position.x)*st.Camera.dimension.x,9216,230,255,0,255,64,15);
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
				230,255,0,255,256,24);

			DrawLine(st.Current_Map.cam_area.area_pos.x-128,st.Current_Map.cam_area.area_pos.y+st.Current_Map.cam_area.area_size.y,st.Current_Map.cam_area.area_pos.x+st.Current_Map.cam_area.area_size.x+128,
				st.Current_Map.cam_area.area_pos.y+st.Current_Map.cam_area.area_size.y,230,255,0,255,256,24);

			DrawLine(st.Current_Map.cam_area.area_pos.x,st.Current_Map.cam_area.area_pos.y,st.Current_Map.cam_area.area_pos.x,st.Current_Map.cam_area.area_pos.y+st.Current_Map.cam_area.area_size.y,230,255,0,255,256,24);

			DrawLine(st.Current_Map.cam_area.area_pos.x+st.Current_Map.cam_area.area_size.x,st.Current_Map.cam_area.area_pos.y,st.Current_Map.cam_area.area_pos.x+st.Current_Map.cam_area.area_size.x,
				st.Current_Map.cam_area.area_pos.y+st.Current_Map.cam_area.area_size.y,230,255,0,255,256,24);
		}
	}

	if(meng.pannel_choice==ADD_LIGHT && meng.command==EDIT_LIGHTMAP)
	{
		if(meng.got_it!=-1)
		{
			i=meng.got_it;

			DrawLine(st.game_lightmaps[i].w_pos.x-((st.game_lightmaps[i].W_w/2)),st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2),
				st.game_lightmaps[i].w_pos.x+((st.game_lightmaps[i].W_w/2)),st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2),
				255,255,255,255,64,24);

			DrawLine(st.game_lightmaps[i].w_pos.x-((st.game_lightmaps[i].W_w/2)),st.game_lightmaps[i].w_pos.y+(st.game_lightmaps[i].W_h/2),
				st.game_lightmaps[i].w_pos.x+((st.game_lightmaps[i].W_w/2)),st.game_lightmaps[i].w_pos.y+(st.game_lightmaps[i].W_h/2),
				255,255,255,255,64,24);

			DrawLine(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y+((st.game_lightmaps[i].W_h/2)),
				st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y-((st.game_lightmaps[i].W_h/2)),
				255,255,255,255,64,24);

			DrawLine(st.game_lightmaps[i].w_pos.x+(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y+((st.game_lightmaps[i].W_h/2)),
				st.game_lightmaps[i].w_pos.x+(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y-((st.game_lightmaps[i].W_h/2)),
				255,255,255,255,64,24);

		}
	}
	else
	if(meng.pannel_choice==ADD_LIGHT && meng.command==EDIT_LIGHTMAP2)
	{
		i=meng.got_it;

		DrawLine(st.game_lightmaps[i].w_pos.x-((st.game_lightmaps[i].W_w/2)),st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2),
			st.game_lightmaps[i].w_pos.x+((st.game_lightmaps[i].W_w/2)),st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2),
			255,255,255,255,64,24);

		DrawLine(st.game_lightmaps[i].w_pos.x-((st.game_lightmaps[i].W_w/2)),st.game_lightmaps[i].w_pos.y+(st.game_lightmaps[i].W_h/2),
			st.game_lightmaps[i].w_pos.x+((st.game_lightmaps[i].W_w/2)),st.game_lightmaps[i].w_pos.y+(st.game_lightmaps[i].W_h/2),
			255,255,255,255,64,24);

		DrawLine(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y+((st.game_lightmaps[i].W_h/2)),
			st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y-((st.game_lightmaps[i].W_h/2)),
			255,255,255,255,64,24);

		DrawLine(st.game_lightmaps[i].w_pos.x+(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y+((st.game_lightmaps[i].W_h/2)),
			st.game_lightmaps[i].w_pos.x+(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y-((st.game_lightmaps[i].W_h/2)),
			255,255,255,255,64,24);

		if(meng.sub_com==2)
		{
			for(j=0;j<st.game_lightmaps[i].num_lights;j++)	
			{
				p2.x=(st.game_lightmaps[i].W_w*st.game_lightmaps[i].t_pos[j].x)/st.game_lightmaps[i].T_w;
				p2.y=(st.game_lightmaps[i].W_h*st.game_lightmaps[i].t_pos[j].y)/st.game_lightmaps[i].T_h;

				p2.x=p2.x+(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2));
				p2.y=p2.y+(st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2));
				
				DrawGraphic(p2.x,p2.y,256,256,0,255,255,255,mgg_sys[0].frames[4],255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,24,0);
			}
		}

		if(meng.com_id==1)
		{
			j=meng.temp;
			p2.x=(st.game_lightmaps[i].W_w*st.game_lightmaps[i].t_pos[j].x)/st.game_lightmaps[i].T_w;
			p2.y=(st.game_lightmaps[i].W_h*st.game_lightmaps[i].t_pos[j].y)/st.game_lightmaps[i].T_h;

			p2.x=p2.x+(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2));
			p2.y=p2.y+(st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2));
				
			DrawGraphic(p2.x,p2.y,256,256,0,255,255,255,mgg_sys[0].frames[4],255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,24,0);
		}

	}
	else
	if(meng.pannel_choice==ADD_LIGHT && meng.command==CREATE_LIGHTMAP_STEP2)
	{
		i=meng.command2;

		DrawGraphic(st.game_lightmaps[i].w_pos.x,st.game_lightmaps[i].w_pos.y,st.game_lightmaps[i].W_w,st.game_lightmaps[i].W_h,0,255,128,32,mgg_sys[0].frames[5],128,0,0,32768,32768,17,0);

		DrawLine(st.game_lightmaps[i].w_pos.x-((st.game_lightmaps[i].W_w/2)-((st.game_lightmaps[i].W_w/12))),st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2),
			st.game_lightmaps[i].w_pos.x+((st.game_lightmaps[i].W_w/2)-((st.game_lightmaps[i].W_w/12))),st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2),
			255,128,32,255,st.game_lightmaps[i].W_h/12,24);

		DrawLine(st.game_lightmaps[i].w_pos.x-((st.game_lightmaps[i].W_w/2)-((st.game_lightmaps[i].W_w/12))),st.game_lightmaps[i].w_pos.y+(st.game_lightmaps[i].W_h/2),
			st.game_lightmaps[i].w_pos.x+((st.game_lightmaps[i].W_w/2)-((st.game_lightmaps[i].W_w/12))),st.game_lightmaps[i].w_pos.y+(st.game_lightmaps[i].W_h/2),
			255,128,32,255,st.game_lightmaps[i].W_h/12,24);

		DrawLine(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y+((st.game_lightmaps[i].W_h/2)-(st.game_lightmaps[i].W_h/12)),
			st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y-((st.game_lightmaps[i].W_h/2)-(st.game_lightmaps[i].W_h/12)),
			255,128,32,255,st.game_lightmaps[i].W_w/12,24);

		DrawLine(st.game_lightmaps[i].w_pos.x+(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y+((st.game_lightmaps[i].W_h/2)-(st.game_lightmaps[i].W_h/12)),
			st.game_lightmaps[i].w_pos.x+(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y-((st.game_lightmaps[i].W_h/2)-(st.game_lightmaps[i].W_h/12)),
			255,128,32,255,st.game_lightmaps[i].W_w/12,24);

		DrawGraphic(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2),st.game_lightmaps[i].W_w/6,st.game_lightmaps[i].W_h/6,0,
			255,32,32,mgg_sys[0].frames[4],255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,24,0);

		DrawGraphic(st.game_lightmaps[i].w_pos.x+(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2),st.game_lightmaps[i].W_w/6,st.game_lightmaps[i].W_h/6,0,
			255,32,32,mgg_sys[0].frames[4],255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,24,0);

		DrawGraphic(st.game_lightmaps[i].w_pos.x+(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y+(st.game_lightmaps[i].W_h/2),st.game_lightmaps[i].W_w/6,st.game_lightmaps[i].W_h/6,0,
			255,32,32,mgg_sys[0].frames[4],255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,16,0);

		DrawGraphic(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y+(st.game_lightmaps[i].W_h/2),st.game_lightmaps[i].W_w/6,st.game_lightmaps[i].W_h/6,0,
			255,32,32,mgg_sys[0].frames[4],255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,16,0);
	}
	else
	if(meng.pannel_choice==ADD_LIGHT && meng.command==CREATE_LIGHTMAP)
	{
		i=meng.obj_lightmap_sel;

		if(i>-1)
		{
			DrawLine(st.Current_Map.obj[i].position.x-(st.Current_Map.obj[i].size.x/2),st.Current_Map.obj[i].position.y-(st.Current_Map.obj[i].size.y/2),
				st.Current_Map.obj[i].position.x+(st.Current_Map.obj[i].size.x/2)+32,st.Current_Map.obj[i].position.y-(st.Current_Map.obj[i].size.y/2),
				255,255,255,255,64,24);

			DrawLine(st.Current_Map.obj[i].position.x-(st.Current_Map.obj[i].size.x/2)-32,st.Current_Map.obj[i].position.y+(st.Current_Map.obj[i].size.y/2),
				st.Current_Map.obj[i].position.x+(st.Current_Map.obj[i].size.x/2)+32,st.Current_Map.obj[i].position.y+(st.Current_Map.obj[i].size.y/2),
				255,255,255,255,64,24);

			DrawLine(st.Current_Map.obj[i].position.x-(st.Current_Map.obj[i].size.x/2),st.Current_Map.obj[i].position.y+(st.Current_Map.obj[i].size.y/2),
				st.Current_Map.obj[i].position.x-(st.Current_Map.obj[i].size.x/2),st.Current_Map.obj[i].position.y-(st.Current_Map.obj[i].size.y/2)-32,
				255,255,255,255,64,24);

			DrawLine(st.Current_Map.obj[i].position.x+(st.Current_Map.obj[i].size.x/2),st.Current_Map.obj[i].position.y+(st.Current_Map.obj[i].size.y/2),
				st.Current_Map.obj[i].position.x+(st.Current_Map.obj[i].size.x/2),st.Current_Map.obj[i].position.y-(st.Current_Map.obj[i].size.y/2)-32,
				255,255,255,255,64,24);
		}
		else
		if(i==-2)
		{
			//DrawGraphic(meng.lightmappos.x,meng.lightmappos.y,meng.lightmapsize.x,meng.lightmapsize.y,0,255,255,255,mgg_sys[0].frames[4],255,0,0,32768,32768,16);

			DrawLine(meng.lightmappos.x-(meng.lightmapsize.x/2)-32,meng.lightmappos.y-(meng.lightmapsize.y/2),meng.lightmappos.x+(meng.lightmapsize.x/2)+32,meng.lightmappos.y-(meng.lightmapsize.y/2),255,255,255,255,64,24);
			DrawLine(meng.lightmappos.x-(meng.lightmapsize.x/2),meng.lightmappos.y-(meng.lightmapsize.y/2),meng.lightmappos.x-(meng.lightmapsize.x/2),meng.lightmappos.y+(meng.lightmapsize.y/2)+32,255,255,255,255,64,24);
			DrawLine(meng.lightmappos.x-(meng.lightmapsize.x/2),meng.lightmappos.y+(meng.lightmapsize.y/2),meng.lightmappos.x+(meng.lightmapsize.x/2)+32,meng.lightmappos.y+(meng.lightmapsize.y/2),255,255,255,255,64,24);
			DrawLine(meng.lightmappos.x+(meng.lightmapsize.x/2),meng.lightmappos.y-(meng.lightmapsize.y/2),meng.lightmappos.x+(meng.lightmapsize.x/2),meng.lightmappos.y+(meng.lightmapsize.y/2)+32,255,255,255,255,64,24);
		}
	}
	else
	if(meng.pannel_choice==ADD_LIGHT && meng.command==ADD_LIGHT_TO_LIGHTMAP)
	{
		i=meng.command2;

		texture.data=st.game_lightmaps[i].tex;
		texture.normal=0;
		texture.vb_id=-1;

		//DrawGraphic(st.game_lightmaps[i].w_pos.x,st.game_lightmaps[i].w_pos.y,st.game_lightmaps[i].W_w,st.game_lightmaps[i].W_h,0,255,255,255,texture,128,0,0,32768,32768,24,0);

		DrawLine(st.game_lightmaps[i].w_pos.x-((st.game_lightmaps[i].W_w/2)),st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2),
			st.game_lightmaps[i].w_pos.x+((st.game_lightmaps[i].W_w/2)),st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2),
			255,255,255,255,64,17);

		DrawLine(st.game_lightmaps[i].w_pos.x-((st.game_lightmaps[i].W_w/2)),st.game_lightmaps[i].w_pos.y+(st.game_lightmaps[i].W_h/2),
			st.game_lightmaps[i].w_pos.x+((st.game_lightmaps[i].W_w/2)),st.game_lightmaps[i].w_pos.y+(st.game_lightmaps[i].W_h/2),
			255,255,255,255,64,17);

		DrawLine(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y+((st.game_lightmaps[i].W_h/2)),
			st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y-((st.game_lightmaps[i].W_h/2)),
			255,255,255,255,64,17);

		DrawLine(st.game_lightmaps[i].w_pos.x+(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y+((st.game_lightmaps[i].W_h/2)),
			st.game_lightmaps[i].w_pos.x+(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y-((st.game_lightmaps[i].W_h/2)),
			255,255,255,255,64,17);
		
		p2=st.game_lightmaps[i].t_pos[meng.light.light_id];

		
		p2.x=(st.game_lightmaps[i].W_w*st.game_lightmaps[i].t_pos[meng.light.light_id].x)/st.game_lightmaps[i].T_w;
		p2.y=(st.game_lightmaps[i].W_h*st.game_lightmaps[i].t_pos[meng.light.light_id].y)/st.game_lightmaps[i].T_h;

		p2.x=p2.x+(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2));
		p2.y=p2.y+(st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2));

		DrawGraphic(p2.x,p2.y,256,256,0,255,255,255,mgg_sys[0].frames[4],255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,24,0);
	}
	
}

struct nk_color ColorPicker(struct nk_color color)
{
	if (nk_combo_begin_color(ctx, color, nk_vec2(200, 250)))
	{
		nk_layout_row_dynamic(ctx, 120, 1);
		color = nk_color_picker(ctx, color, NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		color.r = (nk_byte)nk_propertyi(ctx, "R:", 0, color.r, 255, 1, 1);
		color.g = (nk_byte)nk_propertyi(ctx, "G:", 0, color.g, 255, 1, 1);
		color.b = (nk_byte)nk_propertyi(ctx, "B:", 0, color.b, 255, 1, 1);

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

void TransformBox(Pos *pos, Pos *size, int16 *ang)
{
	static int chained = 1;
	float aspect;

	if (nk_begin(ctx, "Transform Box", nk_rect((st.screenx / 2) - (300 / 2), (st.screeny / 2) - (300 / 2), 300, 300), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
	{
		nk_layout_row_dynamic(ctx, 100, 1);
		if (nk_group_begin(ctx, "Position", NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
		{
			nk_layout_row_dynamic(ctx, 25, 2);

			pos->x = nk_propertyi(ctx, "X:", GAME_UNIT_MIN, pos->x, GAME_UNIT_MAX, 1024, 128);
			pos->y = nk_propertyi(ctx, "Y:", GAME_UNIT_MIN, pos->y, GAME_UNIT_MAX, 1024, 128);
			pos->z = nk_propertyi(ctx, "Z:", 0, pos->z, 7 * 8, 2, 1);
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

			if (chained)
				size->y = (float) size->x / aspect;

			nk_layout_row_push(ctx, 0.1f);
			if (chained)
			{
				ctx->style.button.normal = ctx->style.button.active;
				ctx->style.button.hover = ctx->style.button.active;
				if (nk_button_symbol(ctx, NK_SYMBOL_X))
					chained = 0;

				nk_style_default(ctx);
			}
			else
				if (nk_button_symbol(ctx, NK_SYMBOL_X))
					chained = 1;


			nk_layout_row_push(ctx, 0.45f);
			size->y = nk_propertyi(ctx, "H:", GAME_UNIT_MIN, size->y, GAME_UNIT_MAX, 1024, 128);

			if (chained)
				size->x = (float) size->y * aspect;

			nk_group_end(ctx);
		}

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, " ", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 25, 1);
		if (nk_button_label(ctx, "OK"))
			meng.command = meng.pannel_choice;
	}

	nk_end(ctx);
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
					st.Current_Map.num_lights = 0;

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
						st.Current_Map.obj[i].lightmapid = -1;
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

					memset(meng.z_buffer, 0, 2048 * 57 * sizeof(int16));
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
					SetDirContent("mgm");
					state = 1;
				}

				nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "Save as...", NK_TEXT_LEFT);
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
			if (nk_menu_begin_label(ctx, "Map", NK_TEXT_LEFT, nk_vec2(150, 200)))
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
				strcpy(st.Current_Map.MGG_FILES[st.Current_Map.num_mgg], filename);
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
		check = FileBrowser(mapname);

		if (check == -1)
			state = 0;

		if (check == 1)
		{
			if (LoadMap(mapname))
			{
				memset(meng.mgg_list, 0, 32 * 256);
				meng.num_mgg = 0;
				LogApp("Map %s loaded", mapname);

				for (a = 0; a<st.Current_Map.num_mgg; a++)
				{
					DrawUI(8192, 4608, 16384, 8192, 0, 0, 0, 0, 0, 0, TEX_PAN_RANGE, TEX_PAN_RANGE, mgg_sys[0].frames[4], 255, 0);
					sprintf(str, "Loading %d%", (a / st.Current_Map.num_mgg) * 100);
					DrawString2UI(str, 8192, 4608, 1, 1, 0, 255, 255, 255, 255, ARIAL, FONT_SIZE * 2, FONT_SIZE * 2, 0);
					if (CheckMGGFile(st.Current_Map.MGG_FILES[a]))
					{
						LoadMGG(&mgg_map[id], st.Current_Map.MGG_FILES[a]);
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

				memset(meng.z_buffer, 0, 2048 * 57 * sizeof(int16));
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

				state = 0;
				//break;
			}
		}
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
				if (st.Game_Sprites[j].num_start_frames>0)
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
							px += data.x_offset;
							py = ((float)data.posy / 32768) * data.h;
							ceil(py);
							py += data.y_offset;
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

		nk_style_default(ctx);

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
								meng.pre_size.x = (mgg_map[i].frames[j].w * 16384) / st.screenx;
								meng.pre_size.y = (mgg_map[i].frames[j].h * 9216) / st.screeny;
							}
						}
					}
				}

				nk_group_end(ctx);
			}

			nk_style_default(ctx);

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

				nk_layout_row_dynamic(ctx, 30, 1);
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

	if (nk_begin(ctx, "Coord bar", nk_rect(st.screenx - 200, st.screeny - 130, 200, 130),NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER))
	{
		nk_layout_row_dynamic(ctx, 20, 1);
		nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "M X: %d M Y: %d", st.mouse.x, st.mouse.y);
		nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "MG X: %d MG Y: %d", x, y);
		nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "X: %d Y: %d", st.Camera.position.x, st.Camera.position.y);
		nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "Zoom: %f", st.Camera.dimension.x);
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_button_label(ctx, "Layers");
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

			nk_style_default(ctx);

			if (meng.pannel_choice == DRAW_SECTOR)
				ctx->style.button.normal = ctx->style.button.hover;

			if (nk_button_label(ctx, "Add sector"))
			{
				meng.command = DRAW_SECTOR;
				meng.pannel_choice = DRAW_SECTOR;
			}

			nk_style_default(ctx);

			if (meng.pannel_choice == ADD_OBJ)
				ctx->style.button.normal = ctx->style.button.hover;

			if (nk_button_label(ctx, "Add scene"))
			{
				meng.command = ADD_OBJ;
				meng.pannel_choice = ADD_OBJ;
			}

			nk_style_default(ctx);

			if (meng.pannel_choice == ADD_SPRITE)
				ctx->style.button.normal = ctx->style.button.hover;

			if (nk_button_label(ctx, "Add Sprite"))
				meng.command = meng.pannel_choice = ADD_SPRITE;

			nk_style_default(ctx);

			if (meng.pannel_choice == ADD_LIGHT)
				ctx->style.button.normal = ctx->style.button.hover;

			nk_button_label(ctx, "Lighting");

			nk_style_default(ctx);

			nk_spacing(ctx, 1);

			if (meng.pannel_choice == SELECT_EDIT)
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				char *layers2[] = { "Foreground", "Midground", "Background 1", "Background 2", "Background 3" };

				nk_label(ctx, "Current layer", NK_TEXT_ALIGN_LEFT);
				meng.curlayer = nk_combo(ctx, layers2, 5, meng.curlayer, 25, nk_vec2(110, 200));

				if (meng.command2 == EDIT_SPRITE)
				{
					editcolor.r = st.Current_Map.sprites[meng.sprite_edit_selection].color.r;
					editcolor.g = st.Current_Map.sprites[meng.sprite_edit_selection].color.g;
					editcolor.b = st.Current_Map.sprites[meng.sprite_edit_selection].color.b;

					nk_layout_row_dynamic(ctx, 30, 1);

					nk_label(ctx, "Current layer of the sprite", NK_TEXT_ALIGN_LEFT);
					meng.spr2.type = nk_combo(ctx, Layers, 5, st.Current_Map.sprites[meng.sprite_edit_selection].type_s, 25, nk_vec2(110, 200));

					st.Current_Map.sprites[meng.sprite_edit_selection].position.z = GetZLayer(st.Current_Map.sprites[meng.sprite_edit_selection].position.z,
						st.Current_Map.sprites[meng.sprite_edit_selection].type_s, meng.spr2.type);

					st.Current_Map.sprites[meng.sprite_edit_selection].type_s = meng.spr2.type;

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

					for (i = 0; i < st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].num_tags; i++)
					{
						if (strcmp(st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i], "MUSFX") == NULL)
						{
							nk_layout_row_dynamic(ctx, 30, 2);

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
						}

						if (strcmp(st.Game_Sprites[st.Current_Map.sprites[meng.sprite_edit_selection].GameID].tag_names[i], "SNDFX") == NULL)
						{
							nk_layout_row_dynamic(ctx, 30, 2);

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
						}
					}

					nk_layout_row_dynamic(ctx, 30, 1);
				}

				if (meng.command2 == EDIT_OBJ)
				{
					editcolor.r = st.Current_Map.obj[meng.obj_edit_selection].color.r;
					editcolor.g = st.Current_Map.obj[meng.obj_edit_selection].color.g;
					editcolor.b = st.Current_Map.obj[meng.obj_edit_selection].color.b;

					nk_layout_row_dynamic(ctx, 30, 1);

					nk_label(ctx, "Current layer of the scenario", NK_TEXT_ALIGN_LEFT);
					meng.obj2.type = nk_combo(ctx, Layers, 5, st.Current_Map.obj[meng.obj_edit_selection].type, 25, nk_vec2(110, 200));

					st.Current_Map.obj[meng.obj_edit_selection].position.z = GetZLayer(st.Current_Map.obj[meng.obj_edit_selection].position.z,
						st.Current_Map.obj[meng.obj_edit_selection].type, meng.obj2.type);

					st.Current_Map.obj[meng.obj_edit_selection].type = meng.obj2.type;

					editcolor = ColorPicker(editcolor);

					st.Current_Map.obj[meng.obj_edit_selection].color.r = editcolor.r;
					st.Current_Map.obj[meng.obj_edit_selection].color.g = editcolor.g;
					st.Current_Map.obj[meng.obj_edit_selection].color.b = editcolor.b;

					st.Current_Map.obj[meng.obj_edit_selection].amblight = nk_propertyf(ctx, "Amb. Light", 0, st.Current_Map.obj[meng.obj_edit_selection].amblight, 1, 0.2f, 0.01f);

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

					if (nk_button_label(ctx, "Transform"))
						meng.command = TRANSFORM_BOX;
				}
			}

			if (meng.pannel_choice == ADD_OBJ)
			{
				editcolor.r = meng.obj.color.r;
				editcolor.g = meng.obj.color.g;
				editcolor.b = meng.obj.color.b;

				nk_layout_row_dynamic(ctx, 30, 1);

				meng.obj.type = nk_combo(ctx, Layers, 5, meng.obj.type, 25, nk_vec2(110, 200));

				editcolor = ColorPicker(editcolor);

				meng.obj.color.r = editcolor.r;
				meng.obj.color.g = editcolor.g;
				meng.obj.color.b = editcolor.b;

				meng.obj.amblight = nk_propertyf(ctx, "Amb. Light", 0, meng.obj.amblight, 1, 0.2f, 0.01f);

				nk_layout_row_dynamic(ctx, (st.screenx * 0.15f) / 2, 2);

				data = meng.tex_selection;

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
					texid = nk_image_id(ctx, data.data);

				nk_image(ctx, texid);

				//nk_layout_row_dynamic(ctx, 30, 1);

				if (nk_button_label(ctx, "Select Sprite"))
					meng.command = SPRITE_SELECTION;

				nk_label(ctx, st.Game_Sprites[meng.sprite_selection].name, NK_TEXT_ALIGN_CENTERED);

			}
		}

		nk_end(ctx);
	}
}

int main(int argc, char *argv[])
{
	int16 i=0, test=0, ch, ch2, ch3;
	int16 testa = 0;
	char options[8][16]={"Test 1", "Option 2", "vagina 3", "mGear 4"}, str[64];

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

	strcpy(st.WindowTitle,"Engineer");

	OpenFont("Font/Roboto-Regular.ttf","arial",0,128);
	OpenFont("Font/Roboto-Bold.ttf","arial bold",1,128);
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
		}
	}

	SetCurrentDirectory(meng.prj_path);

	meng.num_mgg=0;
	memset(st.Game_Sprites,0,MAX_SPRITES*sizeof(_SPRITES));
	st.num_sprites=0;
	meng.got_it=-1;
	meng.sprite_selection=-1;
	meng.sprite_frame_selection=-1;
	meng.spr.size.x=2048;
	meng.spr.size.y=2048;
	meng.editview = MIDGROUND_MODE;

	//SpriteListLoad();

	BASICBKD(255,255,255);
	DrawString2UI("Loading sprites...",8192,4608,0,0,0,255,255,255,255,ARIAL,2048,2048,6);

	Renderer(1);

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

	SetCurrentDirectory(st.CurrPath);

	ctx = nk_sdl_init(wn);

	struct nk_font_atlas *atlas;
	nk_sdl_font_stash_begin(&atlas);
	fonts[0] = nk_font_atlas_add_from_file(atlas, "Font\\Roboto-Regular.ttf", 16, 0);
	fonts[1] = nk_font_atlas_add_from_file(atlas, "Font\\mUI.ttf", 18, 0);
	fonts[2] = nk_font_atlas_add_from_file(atlas, "Font\\Roboto-Regular.ttf", 14, 0);
	nk_sdl_font_stash_end();
	nk_style_set_font(ctx, &fonts[0]->handle);

	SetCurrentDirectory(meng.prj_path);

	NewMap();

	SetSkin(ctx, THEME_BLACK);

	SETENGINEPATH;
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

		if(meng.viewmode==LIGHTVIEW_MODE || meng.viewmode==INGAMEVIEW_MODE)
			BASICBKD(st.Current_Map.amb_color.r,st.Current_Map.amb_color.g,st.Current_Map.amb_color.b);
		else
			BASICBKD(255,255,255);
		
		loops=0;
		while(GetTicks() > curr_tic && loops < 10)
		{
			//nk_clear(ctx);
			Finish();

			if(st.gt==INGAME)
			{
				if(meng.editor==1)
				{
					MGGEditorMain();
				}
				else
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

			

			curr_tic+=1000/TICSPERSECOND;
			loops++;
			SetTimerM(1);

			if(meng.loop_complete)
			{
				loops=10;
				meng.loop_complete=0;
			}

			if((st.Text_Input && !meng.sub_com && !st.num_uiwindow && UI_Sys.current_option==-1) && (meng.command==ADD_LIGHT_TO_LIGHTMAP && !meng.got_it && st.Text_Input))
				st.Text_Input=0;

			if(meng.viewmode==INGAMEVIEW_MODE)
				LockCamera();
		}

		DrawSys();

		if(st.gt==INGAME && meng.command!=MGG_LOAD && meng.command!=MGG_SEL && meng.editor==0)
		{
			DrawMap();

			ENGDrawLight();
		}

		MenuBar();
		NewLeftPannel();
		CoordBar();

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
				&st.Current_Map.sprites[meng.sprite_edit_selection].angle);

			if(meng.command2 == EDIT_OBJ)
				TransformBox(&st.Current_Map.obj[meng.obj_edit_selection].position, &st.Current_Map.obj[meng.obj_edit_selection].size,
				&st.Current_Map.obj[meng.obj_edit_selection].angle);
		}

		if (meng.command == SPRITE_TAG)
			TagBox(st.Current_Map.sprites[meng.sprite_edit_selection].GameID, meng.sprite_edit_selection);

		//MapProperties();
		//CameraProperties();
		//TransformBox(&st.Current_Map.cam_area.area_pos, &st.Current_Map.cam_area.area_pos,&testa);

		//FileBrowser("mgm");

		/*
		if (nk_begin(ctx, "Demo", nk_rect(50, 50, 200, 200),
			NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
			NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
		{
			nk_menubar_begin(ctx);
			nk_layout_row_begin(ctx, NK_STATIC, 25, 2);

			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "FILE", NK_TEXT_LEFT, nk_vec2(120, 200))) {
				nk_layout_row_dynamic(ctx, 30, 1);
				nk_menu_item_label(ctx, "OPEN", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "CLOSE", NK_TEXT_LEFT);
				nk_menu_end(ctx);
			}
			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "EDIT", NK_TEXT_LEFT, nk_vec2(120, 200))) {
				nk_layout_row_dynamic(ctx, 30, 1);
				nk_menu_item_label(ctx, "COPY", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "CUT", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "PASTE", NK_TEXT_LEFT);
				nk_menu_end(ctx);
			}
			nk_layout_row_end(ctx);
			nk_menubar_end(ctx);

			enum { EASY, HARD };
			static int op = EASY;
			static int property = 20;
			nk_layout_row_static(ctx, 30, 80, 1);
			if (nk_button_label(ctx, "button"))
				fprintf(stdout, "button pressed\n");
			nk_layout_row_dynamic(ctx, 30, 2);
			if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
			if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
			nk_layout_row_dynamic(ctx, 25, 1);
			nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);
		}
		nk_end(ctx);
		*/

		UIMain_DrawSystem();
		MainSound();
		Renderer(0);

		float bg[4];
		nk_color_fv(bg, background);

		//if (nkrendered == 0)
			//printf("porra\n");

		nk_sdl_render(NK_ANTI_ALIASING_OFF, 512 * 1024, 128 * 1024);

		SwapBuffer(wn);

		nkrendered = 0;
	}

	StopAllSounds();
	Quit();
	return 1;
}