#include "input.h"
#include "main.h"
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include "dirent.h"
#include "UI.h"

//extern _MGG mgg_sys[3];

mEng meng;

uint16 WriteCFG()
{
	FILE *file;

	if((file=fopen("settings.cfg","w"))==NULL)
		return 0;

	st.screenx=800;
	st.screeny=600;
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
	char buf[128], str[128];
	int value=0;
	if((file=fopen("settings.cfg","r"))==NULL)
		if(WriteCFG()==0)
			return 0;

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
	DrawUI(8192,4096,2275,8192,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,5);
	if(st.Current_Map.num_mgg>0)
	{
		for(i=227;i<454*st.Current_Map.num_mgg;i+=454)
		{
			if(j==st.Current_Map.num_mgg)
				break;
			else
			{
				if(!CheckColisionMouse(8192,i+meng.scroll2,1365,455,0))
				{
					DrawStringUI(meng.mgg_list[j],8192,i+meng.scroll2,0,0,0,255,128,32,255,ARIAL,2048,2048,0);
				}
				else
				{
					DrawStringUI(meng.mgg_list[j],8192,i+meng.scroll2,0,0,0,255,32,0,255,ARIAL,2048,2048,0);
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

void ImageList(uint8 id)
{
	uint32 m=0, i, j;

	if(mgg_map[id].type==NONE)
		DrawStringUI("Invalid MGG",8192,4096,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
	else
	{
		if(meng.scroll<0)
				m=(meng.scroll/1546)*(-10);

		for(i=728;i<(8192/1546)*1546;i+=1546)
		{
			if(m>=mgg_map[id].num_frames) break;
			for(j=728;j<(16384/1546)*1546;j+=1546)
			{
				if(m<mgg_map[id].num_frames)
				{
					if((CheckColisionMouse(j,i+meng.scroll,1638,1456,0) && st.mouse1) || (meng.tex_selection.data==mgg_map[id].frames[m].data && meng.tex_selection.posx==mgg_map[id].frames[m].posx && meng.tex_selection.posy==mgg_map[id].frames[m].posy))
					{
						DrawUI(j,i+meng.scroll,1638,1456,0,255,128,32,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_map[id].frames[m],255,0);
						meng.tex_selection=mgg_map[id].frames[m];
						meng.tex_ID=m;
						meng.tex_MGGID=id;
						if(((float)mgg_map[id].frames[m].w/(float)mgg_map[id].frames[m].h)>1)
						{
							meng.pre_size.x=2048;
							meng.pre_size.y=2048/((float)mgg_map[id].frames[m].w/(float)mgg_map[id].frames[m].h);
						}
						else
						if(((float)mgg_map[id].frames[m].w/(float)mgg_map[id].frames[m].h)<1)
						{
							meng.pre_size.x=2048/((float)mgg_map[id].frames[m].w/(float)mgg_map[id].frames[m].h);
							meng.pre_size.y=2048;
						}
						else
						if(((float)mgg_map[id].frames[m].w/(float)mgg_map[id].frames[m].h)==1)
						{
							meng.pre_size.x=2048;
							meng.pre_size.y=2048;
						}
					}
					else
					{
						DrawUI(j,i+meng.scroll,1638,1456,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_map[id].frames[m],255,0);
					}

					m++;
				}
				else break;
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
	uint32 m=0, i=728, j=728, k=0, l=0;

	
		if(meng.scroll<0)
				m=(meng.scroll/1546)*(-10);

		for(m=0;m<st.num_sprites;m++)
		{
			if(st.Game_Sprites[m].num_start_frames>0)
			{
				for(k=0;k<st.Game_Sprites[m].num_start_frames;k++)
				{
					if((CheckColisionMouse(j,i+meng.scroll,1638,1638,0) && st.mouse1) || (meng.sprite_frame_selection==st.Game_Sprites[m].frame[k] && meng.sprite_selection==m))
					{
						DrawUI(j,i+meng.scroll,1638,1456,0,255,128,32,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_game[st.Game_Sprites[m].MGG_ID].frames[st.Game_Sprites[m].frame[k]],255,2);
						DrawString2UI(st.Game_Sprites[m].name,j,i+meng.scroll+819,2048,2048,0,255,128,32,255,ARIAL,1024,1024,0);
						meng.sprite_selection=m;
						meng.sprite_frame_selection=st.Game_Sprites[m].frame[k];

						meng.spr.health=st.Game_Sprites[m].health;
						meng.spr.body=st.Game_Sprites[m].body;

						meng.spr.body.size=st.Game_Sprites[m].body.size;
							//meng.tex_selection=mgg_map[id].frames[m];
							//meng.tex_ID=m;
							//meng.tex_MGGID=id;
					}
					else
					{
						DrawUI(j,i+meng.scroll,1638,1456,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_game[st.Game_Sprites[m].MGG_ID].frames[st.Game_Sprites[m].frame[k]],255,2);
						DrawString2UI(st.Game_Sprites[m].name,j,i+meng.scroll+819,2048,2048,0,255,128,32,255,ARIAL,1024,1024,0);
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

		if(CheckColisionMouse(8192,i+meng.scroll,2730,455,0))
		{
			DrawString2UI(files[j],8192,i+meng.scroll,0,0,0,255,128,32,255,ARIAL,FONT_SIZE*2,FONT_SIZE*2,0);

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

						DrawUI(8192,4096,16384,8192,0,0,0,0,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,0);
						DrawString2UI("Loading...",8192,4096,1,1,0,255,255,255,255,ARIAL,FONT_SIZE*3,FONT_SIZE*3,0);
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
			DrawString2UI(files[j],8192,i+meng.scroll,0,0,0,255,255,255,255,ARIAL,FONT_SIZE*2,FONT_SIZE*2,0);
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
	char num[32], str[64];
	uint16 i=0, j=0, xt, yt;
	Pos p;
	PosF p2;

	DrawUI(455,4096,455*2,8192,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,7);

	if(!CheckColisionMouse(227,227,455,455,0))
	{
		DrawUI(227,227,455,455,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[0],255,7);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawUI(227,227,445,445,0,255,128,32,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[0],255,7);
		//time++;
		//if((st.time-time)==1000)
		//{
		DrawStringUI("Draw a sector",8192,8192-455,1820,455,0,255,128,32,255,ARIAL,0,0,0);
		//}

			if(st.mouse1)
			{
				meng.command=meng.pannel_choice=meng.command2=0;
				st.mouse1=0;
			}
	}

	if(!CheckColisionMouse(682,227,448,448,0))
	{
		DrawUI(682,227,448,448,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[2],255,7);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawUI(682,227,448,448,0,255,128,32,0,0,32768,32768,mgg_sys[0].frames[2],255,7);
		//if((st.time-time)==1000)
		//{
		DrawStringUI("Select and edit",8192,8192-455,1820,455,0,255,128,32,255,ARIAL,0,0,0);
		//}

		if(st.mouse1) meng.command=meng.pannel_choice=meng.command2=2;
	}
	
	if(!CheckColisionMouse(227,682,448,448,0))
	{
		DrawUI(227,682,448,448,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[1],255,7);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawUI(227,682,448,448,0,255,128,32,0,0,32768,32768,mgg_sys[0].frames[1],255,7);
		//if((st.time-time)==1000)
		//{
		DrawStringUI("Add an OBJ",8192,8192-455,1820,455,0,255,128,32,255,ARIAL,0,0,0);
		//}

		if(st.mouse1)
		{
			//meng.tex_selection.data=meng.tex2_sel.data;
			//meng.tex_ID=meng.tex2_ID;
			//meng.tex_MGGID=meng.tex2_MGGID;

			meng.command=meng.pannel_choice=meng.command2=3;
		}
	}
	
	if(!CheckColisionMouse(682,682,448,448,0))
	{
		DrawUI(682,682,448,448,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[3],255,7);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawUI(682,682,448,448,0,255,128,32,0,0,32768,32768,mgg_sys[0].frames[3],255,7);
		//if((st.time-time)==1000)
		//{
		DrawStringUI("Add a sprite",8192,8192-455,1820,455,0,255,128,32,255,ARIAL,0,0,0);
		//}

			if(st.mouse1)
			{		
				st.mouse1=0;
				meng.command=meng.pannel_choice=meng.command2=ADD_SPRITE;
			}
	}

	if(!CheckColisionMouse(227,1137,448,488,0))
	{
		DrawUI(227,1137,448,448,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[5],255,7);
	}
	else
	{
		DrawUI(227,1137,448,448,0,255,128,32,0,0,32768,32768,mgg_sys[0].frames[5],255,7);

		DrawStringUI("Add a lightmap",8192,8192-455,1820,455,0,255,128,32,255,ARIAL,0,0,0);

		if(st.mouse1)
		{
			st.mouse1=0;
			meng.command=meng.pannel_choice=meng.command2=ADD_LIGHT;
		}
	}
	
	if(meng.pannel_choice==ADD_OBJ || meng.pannel_choice==ADD_SPRITE)
	{
		if(meng.command!=ADD_SPRITE && meng.command!=SPRITE_TAG)
			sprintf(str,"Tex. Sel.");
		else
			sprintf(str,"Sprite Sel.");

		if(!CheckColisionMouse(458,8192-2275,490,223,0))
		{
			DrawStringUI(str,458,8192-2275,490,223,0,255,255,255,255,ARIAL,0,0,0);
		}
		else
		{
			DrawStringUI(str,458,8192-2275,490,223,0,255,128,32,255,ARIAL,0,0,0);


			if(meng.command!=ADD_SPRITE && meng.command!=SPRITE_TAG)
				DrawStringUI("Texture Selection",8192,8192-455,2730,455,0,255,128,32,255,ARIAL,0,0,0);
			else
				DrawStringUI("Sprite Selection",8192,8192-455,2730,455,0,255,128,32,255,ARIAL,0,0,0);


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
	{
		if(!CheckColisionMouse(458,8192-1820,490,223,0))
		{
			DrawStringUI("MGG Sel.",458,8192-1820,490,223,0,255,255,255,255,ARIAL,0,0,0);
		}
		else
		{
			DrawStringUI("MGG Sel.",458,8192-1820,490,223,0,255,128,32,255,ARIAL,0,0,0);

			DrawStringUI("MGG Selection",8192,8192-455,2270,455,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.scroll2=0;
				meng.command=6;
				meng.command2=0;
			}
		}

		if(!CheckColisionMouse(458,(8192)-227,490,223,0))
		{
			DrawStringUI("Load MGG",458,8192-227,490,223,0,255,255,255,255,ARIAL,0,0,0);
		}
		else
		{
			DrawStringUI("Load MGG",458,(8192)-227,490,223,0,255,128,32,255,ARIAL,0,0,0);

			DrawStringUI("Load an MGG file and adds it to the map list",8192,(8192)-455,5460,455,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.scroll2=0;
				meng.command=7;
				meng.command2=0;
			}
		}

		DrawUI(455,8192-910,910,910,0,255,255,255,0,0,32768,32768,meng.tex_selection,255,0);
	}

	if(meng.pannel_choice==0)
		DrawUI(227,227,455,455,0,128,32,32,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[0],255,0);
	//else
	if(meng.pannel_choice==2)
		DrawUI(682,227,455,455,0,128,32,32,0,0,32768,32768,mgg_sys[0].frames[2],255,0);
	//else

	if(meng.pannel_choice==ADD_LIGHT)
	{
		DrawUI(227,1137,455,455,0,128,32,32,0,0,32768,32768,mgg_sys[0].frames[5],255,0);

		if(CheckColisionMouse(455,1900,810,455,0))
		{
			DrawStringUI("Create Lightmap",455,1900,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				st.mouse1=0;
				meng.command=CREATE_LIGHTMAP;
			}
		}
		else
			DrawStringUI("Create Lightmap",455,1900,810,227,0,255,255,255,255,ARIAL,0,0,0);
		
		
		if(CheckColisionMouse(455,1900+810,810,455,0))
		{
			DrawStringUI("Add Light",455,1900+810,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				st.mouse1=0;
				meng.command=ADD_LIGHT_TO_LIGHTMAP;
			}
		}
		else
			DrawStringUI("Add Light",455,1900+810,810,227,0,255,255,255,255,ARIAL,0,0,0);

		
		if(CheckColisionMouse(455,2710+810,810,455,0))
		{
			DrawStringUI("Lightmap Res.",455,2710+810,810,227,0,255,128,32,255,ARIAL,0,0,0);

			DrawStringUI("Set the lightmap resolution",8192,8192-455,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

			if(st.mouse1)
			{
				st.mouse1=0;
				meng.command=LIGHTMAP_RES;
			}
		}
		else
			DrawStringUI("Lightmap Res.",455,2710+810,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==LIGHTMAP_RES)
		{
			DrawUI(8192,4096,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			sprintf(str,"Res. Width %d",meng.lightmap_res.x);

			if(CheckColisionMouse(8192,4096-512,810,455,0))
			{
				DrawStringUI(str,8192,4096-512,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					st.mouse1=0;
					StartText();
					sprintf(st.TextInput,"%d",meng.lightmap_res.x);
					meng.sub_com=1;
				}
			}
			else
				DrawStringUI(str,8192,4096-512,810,227,0,255,255,255,255,ARIAL,2048,2048,0);

			if(meng.sub_com==1)
			{
				meng.lightmap_res.x=atoi(st.TextInput);

				DrawStringUI(str,8192,4096-512,810,227,0,255,32,32,255,ARIAL,2048,2048,0);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			sprintf(str,"Res. Height %d",meng.lightmap_res.y);

			if(CheckColisionMouse(8192,4096+512,810,455,0))
			{
				DrawStringUI(str,8192,4096+512,810,227,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					st.mouse1=0;
					StartText();
					sprintf(st.TextInput,"%d",meng.lightmap_res.y);
					meng.sub_com=2;
				}
			}
			else
				DrawStringUI(str,8192,4096+512,810,227,0,255,255,255,255,ARIAL,2048,2048,0);

			if(meng.sub_com==2)
			{
				meng.lightmap_res.y=atoi(st.TextInput);

				DrawStringUI(str,8192,4096+512,810,227,0,255,32,32,255,ARIAL,2048,2048,0);

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

		if(CheckColisionMouse(455,3520+810,810,455,0))
		{
			DrawStringUI("Edit Lightmap",455,3520+810,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				st.mouse1=0;
				meng.command=EDIT_LIGHTMAP;
				meng.got_it=-1;
			}
		}
		else
			DrawStringUI("Edit Lightmap",455,3520+810,810,227,0,255,255,255,255,ARIAL,0,0,0);
	}

	if(meng.pannel_choice==4)
	{
		DrawUI(682,682,455,455,0,128,32,32,0,0,32768,32768,mgg_sys[0].frames[3],255,0);

		DrawUI(455,8192-910,910,910,0,255,255,255,0,0,32768,32768,mgg_game[st.Game_Sprites[meng.sprite_selection].MGG_ID].frames[meng.sprite_frame_selection],255,0);

		if(meng.spr.type==MIDGROUND)
		{
			if(CheckColisionMouse(465,1900,880,220,0))
			{
				DrawStringUI("Midground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=16;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Midground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.spr.type==FOREGROUND)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Foreground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=16;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Foreground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.spr.type==BACKGROUND1)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background1",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=16;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background1",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.spr.type==BACKGROUND2)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background2",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=16;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background2",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.spr.type==BACKGROUND3)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background3",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=16;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background3",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.command==ADD_SPRITE_TYPE)
		{
			DrawUI(1365,1715,910,1210,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,0);

			if(CheckColisionMouse(1365,1715,810,227,0))
			{
				DrawStringUI("Background1",1365,1715,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					meng.spr.type=BACKGROUND1;
					//st.Current_Map.obj[meng.com_id].position.z=
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background1",1365,1715,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckColisionMouse(1365,1465,810,227,0))
			{
				DrawStringUI("Background2",1365,1465,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{

					meng.spr.type=BACKGROUND2;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background2",1365,1465,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckColisionMouse(1365,1215,810,227,0))
			{
				DrawStringUI("Background3",1365,1215,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					

					meng.spr.type=BACKGROUND3;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background3",1365,1215,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckColisionMouse(1365,1920,810,227,0))
			{
				DrawStringUI("Midground",1365,1920,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					

					meng.spr.type=MIDGROUND;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Midground",1365,1920,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckColisionMouse(1365,2170,810,227,0))
			{
				DrawStringUI("Foreground",1365,2170,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					

					meng.spr.type=FOREGROUND;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Foreground",1365,2170,810,227,0,255,255,255,255,ARIAL,0,0,0);
		}

		sprintf(str,"color");

		if(CheckColisionMouse(465,2445,810,217,0))
		{
			DrawStringUI(str,465,2445,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

			if(st.mouse1)
			{
				meng.command=RGB_SPRITE;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,2445,810,217,0, 255, 255, 255,255,ARIAL,2048,2048,0);

		if(meng.command==RGB_SPRITE)
		{
			DrawUI(8192,3072,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			DrawStringUI(str,465,2445,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

			sprintf(str,"R %d",meng.spr.color.r);

			if(meng.sub_com!=1)
			{
				if(CheckColisionMouse(8192,2048+455,2048,227,0))
				{
					DrawStringUI(str,8192,2048+455,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.spr.color.r);
						StartText();
						st.mouse1=0;
						meng.sub_com=1;
					}
				}
				else
					DrawStringUI(str,8192,2048+455,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				DrawStringUI(str,8192,2048+455,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,2048+810,2048,227,0))
				{
					DrawStringUI(str,8192,2048+810,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.spr.color.g);
						StartText();
						st.mouse1=0;
						meng.sub_com=2;
					}
				}
				else
					DrawStringUI(str,8192,2048+810,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				DrawStringUI(str,8192,2048+810,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,2048+1265,2048,455,0))
				{
					DrawStringUI(str,8192,2048+1265,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.spr.color.b);
						StartText();
						st.mouse1=0;
						meng.sub_com=3;
					}
				}
				else
					DrawStringUI(str,8192,2048+1265,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==3)
			{
				DrawStringUI(str,8192,2048+1265,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,2048+1720,2048,455,0))
				{
					DrawStringUI(str,8192,2048+1720,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.spr.color.a);
						StartText();
						st.mouse1=0;
						meng.sub_com=4;
					}
				}
				else
					DrawStringUI(str,8192,2048+1720,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==4)
			{
				DrawStringUI(str,8192,2048+1720,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.spr.color.a=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			if(!CheckColisionMouse(465,2445,810,217,0) && st.mouse1)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}


		if(CheckColisionMouse(455,3135,810,217,0))
		{
			DrawStringUI("Tags",465,3135,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

			if(st.mouse1)
			{
				meng.command=SPRITE_TAG;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI("Tags",465,3135,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
		
		if(meng.command==SPRITE_TAG)
		{
			DrawUI(8192,4096,2048,6144,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,0);

			for(i=1, yt=4096-2560;(i-1)<st.Game_Sprites[meng.sprite_selection].num_tags;i++, yt+=512)
			{
				sprintf(str,"%s %d",st.Game_Sprites[meng.sprite_selection].tag_names[i-1],st.Game_Sprites[meng.sprite_selection].tags[i-1]);

				if(meng.sub_com!=i)
				{
					if(CheckColisionMouse(8192,yt,2048,455,0))
					{
						DrawStringUI(str,8192,yt,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							sprintf(st.TextInput,"%d",st.Game_Sprites[meng.sprite_selection].tags[i-1]);
							StartText();
							st.mouse1=0;
							meng.sub_com=i;
						}
					}
					else
						DrawStringUI(str,8192,yt,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
				}
				else
				if(meng.sub_com==i)
				{
					DrawStringUI(str,8192,yt,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					st.Game_Sprites[meng.sprite_selection].tags[i-1]=atoi(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.sub_com=0;
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

		if(CheckColisionMouse(465,3435,810,217,0))
		{
			DrawStringUI(str,465,3435,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1 && !StartText())
			{
				meng.command=SPRITE_HEALTH;
				sprintf(st.TextInput,"%d",meng.spr.health);
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,3435,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==SPRITE_HEALTH)
		{
			DrawStringUI(str,465,3435,810,217,0,255,32,32,255,ARIAL,0,0,0);
			meng.spr.health=atoi(st.TextInput);

			if(st.keys[ESC_KEY].state)
			{
				StopText();
				meng.command=meng.pannel_choice;
				st.keys[ESC_KEY].state=0;
			}
		}

		sprintf(str,"Physics");

		if(CheckColisionMouse(465,3690,810,217,0))
		{
			DrawStringUI(str,465,3690,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.command=SPRITE_PHY;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,3690,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==SPRITE_PHY)
		{
			DrawHud(8192, 4096, 2120,4096,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,7);

			sprintf(str,"Physics? %d",meng.spr.body.physics_on);

			if(CheckColisionMouse(8192, 4096-1420,810,405,0))
			{
				DrawString2UI(str,8192, 4096-1420,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

				if(st.mouse1)
				{
					if(meng.spr.body.physics_on==0) meng.spr.body.physics_on=1;
					else meng.spr.body.physics_on=0;

					st.mouse1=0;
				}
			}
			else
				DrawString2UI(str,8192, 4096-1420,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

			sprintf(str,"Mass %.2f",meng.spr.body.mass);

			if(CheckColisionMouse(8192, 4096-710,810,405,0))
			{
				DrawString2UI(str,8192, 4096-710,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

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
				DrawString2UI(str,8192, 4096-710,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

			if(meng.sub_com==10)
			{
				DrawString2UI(str,8192, 4096-1420,0,0,0,255,32,32,255,ARIAL,512*4,512*4,0);
				meng.spr.body.mass=atof(st.TextInput);
				if(st.keys[ESC_KEY].state)
				{
					StopText();
					meng.sub_com=0;
					st.keys[ESC_KEY].state=0;
				}
			}

			sprintf(str,"Flammable? %d",meng.spr.body.flamable);

			if(CheckColisionMouse(8192, 4096,810,405,0))
			{
				DrawString2UI(str,8192, 4096,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

				if(st.mouse1)
				{
					if(meng.spr.body.flamable==0) meng.spr.body.flamable=1;
					else meng.spr.body.flamable=0;

					st.mouse1=0;
				}
			}
			else
				DrawString2UI(str,8192, 4096,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

			sprintf(str,"Explosive? %d",meng.spr.body.explosive);

			if(CheckColisionMouse(8192, 4096+710,810,405,0))
			{
				DrawString2UI(str,8192, 4096+710,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

				if(st.mouse1)
				{
					if(meng.spr.body.explosive==0) meng.spr.body.explosive=1;
					else meng.spr.body.explosive=0;

					st.mouse1=0;
				}
			}
			else
				DrawString2UI(str,8192, 4096+710,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

			if(st.keys[ESC_KEY].state)
			{
				meng.command=meng.pannel_choice;
				st.keys[ESC_KEY].state=0;
			}
		}
	}

	//Sprite editing

	if(meng.command2==EDIT_SPRITE)
	{
			p.x=st.Current_Map.sprites[meng.sprite_edit_selection].position.x;
			p.y=st.Current_Map.sprites[meng.sprite_edit_selection].position.y-(st.Current_Map.sprites[meng.sprite_edit_selection].body.size.y/2);

			//p.x-=st.Camera.position.x;
			//p.y-=st.Camera.position.y;

			p2.x=1024*st.Camera.dimension.x;
			p2.y=1024*st.Camera.dimension.y;

			sprintf(str,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].body.size.x);
			DrawString2(str,p.x,p.y-227,810,217,0,255,255,255,255,ARIAL,p2.x,p2.y,0);

			p.x=st.Current_Map.sprites[meng.sprite_edit_selection].position.x-(st.Current_Map.sprites[meng.sprite_edit_selection].body.size.x/2);
			p.y=st.Current_Map.sprites[meng.sprite_edit_selection].position.y;

			//p.x-=st.Camera.position.x;
			//p.y-=st.Camera.position.y;

			sprintf(str,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].body.size.y);
			DrawString2(str,p.x-455,p.y,810,217,0,255,255,255,255,ARIAL,p2.x,p2.y,0);

			if(st.keys[RETURN_KEY].state && meng.command!=SPRITE_EDIT_BOX && meng.command!=RGB_SPRITE && meng.command!=SPRITE_PHY && meng.command!=SPRITE_TAG)
			{
				meng.command=SPRITE_EDIT_BOX;
				st.keys[RETURN_KEY].state=0;
			}

			if(meng.command==SPRITE_EDIT_BOX)
			{
				DrawUI(8192,4096,1820,2730,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,0);

				sprintf(str,"X %d",st.Current_Map.sprites[meng.sprite_edit_selection].position.x);

				if(CheckColisionMouse(8192,(4096)-681,1720,455,0) && !meng.sub_com)
				{
					DrawStringUI(str,8192,(4096)-681,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
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
					DrawStringUI(str,8192,(4096)-681,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
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
					DrawStringUI(str,8192,(4096)-681,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

				sprintf(str,"Y %d",st.Current_Map.sprites[meng.sprite_edit_selection].position.y);

				if(CheckColisionMouse(8192,(4096)-227,1720,455,0) && !meng.sub_com)
				{
					DrawStringUI(str,8192,(4096)-227,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
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
					DrawStringUI(str,8192,(4096)-227,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
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
					DrawStringUI(str,8192,(4096)-227,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

				sprintf(str,"SX %d",st.Current_Map.sprites[meng.sprite_edit_selection].body.size.x);

				if(CheckColisionMouse(8192,(4096)+227,1720,455,0) && !meng.sub_com)
				{
					DrawStringUI(str,8192,(4096)+227,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
					if(st.mouse1)
					{
						meng.sub_com=3;
						sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].body.size.x);
						SDL_StartTextInput();
						st.Text_Input=1;
						st.mouse1=0;
					}
				}
				else
				if(meng.sub_com==3)
				{
					DrawStringUI(str,8192,(4096)+227,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
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
					DrawStringUI(str,8192,(4096)+227,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

				sprintf(str,"SY %d",st.Current_Map.sprites[meng.sprite_edit_selection].body.size.y);

				if(CheckColisionMouse(8192,(4096)+681,1720,455,0) && !meng.sub_com)
				{
					DrawStringUI(str,8192,(4096)+681,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
					if(st.mouse1)
					{
						meng.sub_com=4;
						sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].body.size.y);
						SDL_StartTextInput();
						st.Text_Input=1;
						st.mouse1=0;
					}
				}
				else
				if(meng.sub_com==4)
				{
					DrawStringUI(str,8192,(4096)+681,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
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
					DrawStringUI(str,8192,(4096)+681,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

				sprintf(str,"Z %d",st.Current_Map.sprites[meng.sprite_edit_selection].position.z);

				if(CheckColisionMouse(8192,(4096)+1137,1720,455,0) && !meng.sub_com)
				{
					DrawStringUI(str,8192,(4096)+1137,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
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
					DrawStringUI(str,8192,(4096)+1137,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
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
					DrawStringUI(str,8192,(4096)+1137,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

				if(st.keys[ESC_KEY].state && !meng.sub_com)
				{
					meng.command=meng.pannel_choice;
					st.keys[ESC_KEY].state=0;
				}
			}


		if(st.Current_Map.sprites[meng.sprite_edit_selection].type_s==MIDGROUND)
		{
			if(CheckColisionMouse(465,1900,880,220,0))
			{
				DrawStringUI("Midground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=26;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Midground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(st.Current_Map.sprites[meng.sprite_edit_selection].type_s==FOREGROUND)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Foreground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=26;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Foreground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(st.Current_Map.sprites[meng.sprite_edit_selection].type_s==BACKGROUND1)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background1",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=26;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background1",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(st.Current_Map.sprites[meng.sprite_edit_selection].type_s==BACKGROUND2)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background2",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=26;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background2",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(st.Current_Map.sprites[meng.sprite_edit_selection].type_s==BACKGROUND3)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background3",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=26;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background3",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.command==EDIT_SPRITE_TYPE_S)
		{
			DrawUI(1365,1715,910,1210,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,0);

			if(CheckColisionMouse(1365,1715,810,227,0))
			{
				DrawStringUI("Background1",1365,1715,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					st.Current_Map.sprites[meng.sprite_edit_selection].type_s=BACKGROUND1;
					st.Current_Map.sprites[meng.sprite_edit_selection].position.z=32;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background1",1365,1715,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckColisionMouse(1365,1465,810,227,0))
			{
				DrawStringUI("Background2",1365,1465,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{

					st.Current_Map.sprites[meng.sprite_edit_selection].type_s=BACKGROUND2;
					st.Current_Map.sprites[meng.sprite_edit_selection].position.z=40;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background2",1365,1465,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckColisionMouse(1365,1215,810,227,0))
			{
				DrawStringUI("Background3",1365,1215,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					st.Current_Map.sprites[meng.sprite_edit_selection].type_s=BACKGROUND3;
					st.Current_Map.sprites[meng.sprite_edit_selection].position.z=48;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background3",1365,1215,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckColisionMouse(1365,1920,810,227,0))
			{
				DrawStringUI("Midground",1365,1920,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					st.Current_Map.sprites[meng.sprite_edit_selection].type_s=MIDGROUND;
					st.Current_Map.sprites[meng.sprite_edit_selection].position.z=24;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Midground",1365,1920,810,227,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckColisionMouse(1365,2170,810,227,0))
			{
				DrawStringUI("Foreground",1365,2170,810,227,0,255,128,32,255,ARIAL,0,0,0);

				if(st.mouse1)
				{
					st.Current_Map.sprites[meng.sprite_edit_selection].type_s=FOREGROUND;
					st.Current_Map.sprites[meng.sprite_edit_selection].position.z=16;
					meng.command=meng.pannel_choice;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Foreground",1365,2170,810,227,0,255,255,255,255,ARIAL,0,0,0);
		}

		sprintf(str,"color");

		if(CheckColisionMouse(465,2445,810,217,0))
		{
			DrawStringUI(str,465,2445,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

			if(st.mouse1)
			{
				meng.command=RGB_SPRITE;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,2445,810,217,0, 255, 255, 255,255,ARIAL,2048,2048,0);

		if(meng.command==RGB_SPRITE)
		{
			DrawUI(8192,3072,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			DrawStringUI(str,465,2445,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

			sprintf(str,"R %d",st.Current_Map.sprites[meng.sprite_edit_selection].color.r);

			if(meng.sub_com!=1)
			{
				if(CheckColisionMouse(8192,2048+455,2048,227,0))
				{
					DrawStringUI(str,8192,2048+455,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].color.r);
						StartText();
						st.mouse1=0;
						meng.sub_com=1;
					}
				}
				else
					DrawStringUI(str,8192,2048+455,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				DrawStringUI(str,8192,2048+455,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,2048+810,2048,227,0))
				{
					DrawStringUI(str,8192,2048+810,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].color.g);
						StartText();
						st.mouse1=0;
						meng.sub_com=2;
					}
				}
				else
					DrawStringUI(str,8192,2048+810,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				DrawStringUI(str,8192,2048+810,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,2048+1265,2048,455,0))
				{
					DrawStringUI(str,8192,2048+1265,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].color.b);
						StartText();
						st.mouse1=0;
						meng.sub_com=3;
					}
				}
				else
					DrawStringUI(str,8192,2048+1265,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==3)
			{
				DrawStringUI(str,8192,2048+1265,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,2048+1720,2048,455,0))
				{
					DrawStringUI(str,8192,2048+1720,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].color.a);
						StartText();
						st.mouse1=0;
						meng.sub_com=4;
					}
				}
				else
					DrawStringUI(str,8192,2048+1720,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==4)
			{
				DrawStringUI(str,8192,2048+1720,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				st.Current_Map.sprites[meng.sprite_edit_selection].color.a=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			if(!CheckColisionMouse(465,2445,810,217,0) && st.mouse1)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}


		if(CheckColisionMouse(455,3135,810,217,0))
		{
			DrawStringUI("Tags",465,3135,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

			if(st.mouse1)
			{
				meng.command=SPRITE_TAG;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI("Tags",465,3135,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
		
		if(meng.command==SPRITE_TAG)
		{
			DrawUI(8192,4096,2048,6144,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,0);

			for(i=1, yt=4096-2560;(i-1)<st.Game_Sprites[meng.sprite_edit_selection].num_tags;i++, yt+=512)
			{
				sprintf(str,"%s %d",st.Game_Sprites[meng.sprite_edit_selection].tag_names[i-1],st.Game_Sprites[meng.sprite_edit_selection].tags[i-1]);

				if(meng.sub_com!=i)
				{
					if(CheckColisionMouse(8192,yt,2048,455,0))
					{
						DrawStringUI(str,8192,yt,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							sprintf(st.TextInput,"%d",st.Game_Sprites[meng.sprite_edit_selection].tags[i-1]);
							StartText();
							st.mouse1=0;
							meng.sub_com=i;
						}
					}
					else
						DrawStringUI(str,8192,yt,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
				}
				else
				if(meng.sub_com==i)
				{
					DrawStringUI(str,8192,yt,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					st.Game_Sprites[meng.sprite_edit_selection].tags[i-1]=atoi(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.sub_com=0;
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

		if(CheckColisionMouse(465,3435,810,217,0))
		{
			DrawStringUI(str,465,3435,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1 && !StartText())
			{
				meng.command=SPRITE_HEALTH;
				sprintf(st.TextInput,"%d",st.Current_Map.sprites[meng.sprite_edit_selection].health);
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,3435,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==SPRITE_HEALTH)
		{
			DrawStringUI(str,465,3435,810,217,0,255,32,32,255,ARIAL,0,0,0);
			st.Current_Map.sprites[meng.sprite_edit_selection].health=atoi(st.TextInput);

			if(st.keys[ESC_KEY].state)
			{
				StopText();
				meng.command=meng.pannel_choice;
				st.keys[ESC_KEY].state=0;
			}
		}

		sprintf(str,"Physics");

		if(CheckColisionMouse(465,3690,810,217,0))
		{
			DrawStringUI(str,465,3690,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.command=SPRITE_PHY;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,3690,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==SPRITE_PHY)
		{
			DrawHud(8192, 4096, 2120,4096,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,7);

			sprintf(str,"Physics? %d",st.Current_Map.sprites[meng.sprite_edit_selection].body.physics_on);

			if(CheckColisionMouse(8192, 4096-1420,810,405,0))
			{
				DrawString2UI(str,8192, 4096-1420,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

				if(st.mouse1)
				{
					if(meng.spr.body.physics_on==0) st.Current_Map.sprites[meng.sprite_edit_selection].body.physics_on=1;
					else st.Current_Map.sprites[meng.sprite_edit_selection].body.physics_on=0;

					st.mouse1=0;
				}
			}
			else
				DrawString2UI(str,8192, 4096-1420,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

			sprintf(str,"Mass %.2f",st.Current_Map.sprites[meng.sprite_edit_selection].body.mass);

			if(CheckColisionMouse(8192, 4096-710,810,405,0))
			{
				DrawString2UI(str,8192, 4096-710,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

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
				DrawString2UI(str,8192, 4096-710,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

			if(meng.sub_com==10)
			{
				DrawString2UI(str,8192, 4096-1420,0,0,0,255,32,32,255,ARIAL,512*4,512*4,0);
				st.Current_Map.sprites[meng.sprite_edit_selection].body.mass=atof(st.TextInput);
				if(st.keys[ESC_KEY].state)
				{
					StopText();
					meng.sub_com=0;
					st.keys[ESC_KEY].state=0;
				}
			}

			sprintf(str,"Flammable? %d",st.Current_Map.sprites[meng.sprite_edit_selection].body.flamable);

			if(CheckColisionMouse(8192, 4096,810,405,0))
			{
				DrawString2UI(str,8192, 4096,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

				if(st.mouse1)
				{
					if(meng.spr.body.flamable==0) st.Current_Map.sprites[meng.sprite_edit_selection].body.flamable=1;
					else st.Current_Map.sprites[meng.sprite_edit_selection].body.flamable=0;

					st.mouse1=0;
				}
			}
			else
				DrawString2UI(str,8192, 4096,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

			sprintf(str,"Explosive? %d",st.Current_Map.sprites[meng.sprite_edit_selection].body.explosive);

			if(CheckColisionMouse(8192, 4096+710,810,405,0))
			{
				DrawString2UI(str,8192, 4096+710,0,0,0,255,128,32,255,ARIAL,512*4,512*4,0);

				if(st.mouse1)
				{
					if(meng.spr.body.explosive==0) st.Current_Map.sprites[meng.sprite_edit_selection].body.explosive=1;
					else st.Current_Map.sprites[meng.sprite_edit_selection].body.explosive=0;

					st.mouse1=0;
				}
			}
			else
				DrawString2UI(str,8192, 4096+710,0,0,0,255,255,255,255,ARIAL,512*4,512*4,0);

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
		DrawString2(str,p.x,p.y-227,810,217,0,255,255,255,255,ARIAL,p2.x,p2.y,0);

		p.x=st.Current_Map.obj[i].position.x-(st.Current_Map.obj[i].size.x/2);
		p.y=st.Current_Map.obj[i].position.y;

		//p.x-=st.Camera.position.x;
		//p.y-=st.Camera.position.y;

		//p.x*=st.Camera.dimension.x;
		//p.y*=st.Camera.dimension.y;

		sprintf(str,"%d",st.Current_Map.obj[i].size.y);
		DrawString2(str,p.x-455,p.y,810,217,0,255,255,255,255,ARIAL,p2.x,p2.y,0);

		if(st.keys[RETURN_KEY].state && meng.command!=TEX_SIZE_OBJ && meng.command!=TEX_PAN_OBJ && meng.command!=OBJ_EDIT_BOX && meng.command!=RGB_OBJ)
		{
			meng.command=OBJ_EDIT_BOX;
			st.keys[RETURN_KEY].state=0;
		}

		if(meng.command==OBJ_EDIT_BOX)
		{
			DrawUI(8192,4096,1820,2730,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,0);

			sprintf(str,"X %d",st.Current_Map.obj[i].position.x);

			if(CheckColisionMouse(8192,(4096)-681,1720,455,0) && !meng.sub_com)
			{
				DrawStringUI(str,8192,(4096)-681,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)-681,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)-681,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

			sprintf(str,"Y %d",st.Current_Map.obj[i].position.y);

			if(CheckColisionMouse(8192,(4096)-227,1720,455,0) && !meng.sub_com)
			{
				DrawStringUI(str,8192,(4096)-227,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)-227,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)-227,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

			sprintf(str,"SX %d",st.Current_Map.obj[i].size.x);

			if(CheckColisionMouse(8192,(4096)+227,1720,455,0) && !meng.sub_com)
			{
				DrawStringUI(str,8192,(4096)+227,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
				if(st.mouse1)
				{
					meng.sub_com=3;
					sprintf(st.TextInput,"%d",st.Current_Map.obj[i].size.x);
					SDL_StartTextInput();
					st.Text_Input=1;
					st.mouse1=0;
				}
			}
			else
			if(meng.sub_com==3)
			{
				DrawStringUI(str,8192,(4096)+227,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)+227,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

			sprintf(str,"SY %d",st.Current_Map.obj[i].size.y);

			if(CheckColisionMouse(8192,(4096)+681,1720,455,0) && !meng.sub_com)
			{
				DrawStringUI(str,8192,(4096)+681,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
				if(st.mouse1)
				{
					meng.sub_com=4;
					sprintf(st.TextInput,"%d",st.Current_Map.obj[i].size.y);
					SDL_StartTextInput();
					st.Text_Input=1;
					st.mouse1=0;
				}
			}
			else
			if(meng.sub_com==4)
			{
				DrawStringUI(str,8192,(4096)+681,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)+681,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

			sprintf(str,"Z %d",st.Current_Map.obj[i].position.z);

			if(CheckColisionMouse(8192,(4096)+1137,1720,455,0) && !meng.sub_com)
			{
				DrawStringUI(str,8192,(4096)+1137,1720,455,0,255,128,32,255,ARIAL,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)+1137,1720,455,0,255,32,32,255,ARIAL,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)+1137,1720,455,0,255,255,255,255,ARIAL,2048,2048,0);

			if(st.keys[ESC_KEY].state && !meng.sub_com)
			{
				meng.command=meng.pannel_choice;
				st.keys[ESC_KEY].state=0;
			}
		}

		if(meng.obj2.type==MIDGROUND)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Midground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=14;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Midground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj2.type==FOREGROUND)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Foreground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=14;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Foreground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj2.type==BACKGROUND1)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background1",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=14;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background1",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj2.type==BACKGROUND2)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background2",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=14;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background2",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj2.type==BACKGROUND3)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background3",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=14;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background3",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		sprintf(str,"color");

		if(CheckColisionMouse(465,2445,810,217,0))
		{
			DrawStringUI(str,465,2445,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

			if(st.mouse1)
			{
				meng.command=RGB_OBJ;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,2445,810,217,0, 255, 255, 255,255,ARIAL,2048,2048,0);

		if(meng.command==RGB_OBJ)
		{
			DrawUI(8192,3072,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			DrawStringUI(str,465,2445,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

			sprintf(str,"R %d",meng.obj2.color.r);

			if(meng.sub_com!=1)
			{
				if(CheckColisionMouse(8192,2048+455,2048,227,0))
				{
					DrawStringUI(str,8192,2048+455,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj2.color.r);
						StartText();
						st.mouse1=0;
						meng.sub_com=1;
					}
				}
				else
					DrawStringUI(str,8192,2048+455,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				DrawStringUI(str,8192,2048+455,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,2048+810,2048,227,0))
				{
					DrawStringUI(str,8192,2048+810,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj2.color.g);
						StartText();
						st.mouse1=0;
						meng.sub_com=2;
					}
				}
				else
					DrawStringUI(str,8192,2048+810,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				DrawStringUI(str,8192,2048+810,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,2048+1265,2048,455,0))
				{
					DrawStringUI(str,8192,2048+1265,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj2.color.b);
						StartText();
						st.mouse1=0;
						meng.sub_com=3;
					}
				}
				else
					DrawStringUI(str,8192,2048+1265,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==3)
			{
				DrawStringUI(str,8192,2048+1265,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,2048+1720,2048,455,0))
				{
					DrawStringUI(str,8192,2048+1720,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj2.color.a);
						StartText();
						st.mouse1=0;
						meng.sub_com=4;
					}
				}
				else
					DrawStringUI(str,8192,2048+1720,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==4)
			{
				DrawStringUI(str,8192,2048+1720,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj2.color.a=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			if(!CheckColisionMouse(465,2445,810,217,0) && st.mouse1)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}



		sprintf(str,"Light %.2f",meng.obj2.amblight);

		if(CheckColisionMouse(465,2887,810,217,0))
		{
			DrawStringUI(str,465,2887,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.command=OBJ_AMBL;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,2887,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==OBJ_AMBL)
		{
			DrawStringUI(str,465,2887,810,217,0,255,32,32,255,ARIAL,0,0,0);

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

			if(!CheckColisionMouse(465,2887,810,217,0) && st.mouse1)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}

		sprintf(str,"Tex.Size");

		if(CheckColisionMouse(465,3315,810,217,0))
		{
			DrawStringUI(str,465,3315,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1 && meng.command!=TEX_PAN_OBJ)
			{
				meng.command=TEX_SIZE_OBJ;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,3315,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==TEX_SIZE_OBJ)
		{
			DrawUI(8192,4096,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			sprintf(str,"X %d",meng.obj2.texsize.x);

			if(meng.sub_com!=1)
			{
				if(CheckColisionMouse(8192,3641,2048,910,0))
				{
					DrawStringUI(str,8192,3641,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

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
					DrawStringUI(str,8192,3641,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				DrawStringUI(str,8192,3641,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,4551,2048,910,0))
				{
					DrawStringUI(str,8192,4551,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

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
					DrawStringUI(str,8192,4551,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				DrawStringUI(str,8192,4551,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

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

			if(!CheckColisionMouse(465,3315,810,217,0) && st.mouse1 && meng.sub_com==0)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}

		sprintf(str,"Tex.Pan");

		if(CheckColisionMouse(465,3640,810,217,0))
		{
			DrawStringUI(str,465,3640,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1 && meng.command!=TEX_SIZE_OBJ)
			{
				meng.command=TEX_PAN_OBJ;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,3640,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==TEX_PAN_OBJ)
		{
			DrawUI(8192,4096,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			sprintf(str,"X %d",meng.obj2.texpan.x);

			if(meng.sub_com!=1)
			{
				if(CheckColisionMouse(8192,3641,2048,910,0))
				{
					DrawStringUI(str,8192,3641,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

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
					DrawStringUI(str,8192,3641,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				DrawStringUI(str,8192,3641,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,4551,2048,910,0))
				{
					DrawStringUI(str,8192,4551,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

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
					DrawStringUI(str,8192,4551,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				DrawStringUI(str,8192,4551,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

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

			if(!CheckColisionMouse(465,3315,810,217,0) && st.mouse1 && meng.sub_com==0)
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

		st.Current_Map.obj[i].amblight=meng.obj2.amblight;
		st.Current_Map.obj[i].texpan=meng.obj2.texpan;
		st.Current_Map.obj[i].texsize=meng.obj2.texsize;
		st.Current_Map.obj[i].color=meng.obj2.color;
		st.Current_Map.obj[i].type=meng.obj2.type;
	}

	if(meng.pannel_choice==3)
	{
		DrawUI(247,681,435,435,0,128,32,32,0,0,32768,32768,mgg_sys[0].frames[1],255,0);

		if(meng.obj.type==MIDGROUND)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Midground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=8;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Midground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj.type==FOREGROUND)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Foreground",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=8;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Foreground",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj.type==BACKGROUND1)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background1",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=8;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background1",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj.type==BACKGROUND2)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background2",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=8;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background2",465,2010,810,217,0,255,255,255,255,ARIAL,0,0,0);
		}

		if(meng.obj.type==BACKGROUND3)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background3",465,2010,810,217,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.command=8;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background3",465,2010,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
		}

		sprintf(str,"color");

		if(CheckColisionMouse(465,2445,810,217,0))
		{
			DrawStringUI(str,465,2445,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

			if(st.mouse1)
			{
				meng.command=RGB_OBJ;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,2445,810,217,0, 255, 255, 255,255,ARIAL,2048,2048,0);

		if(meng.command==RGB_OBJ)
		{
			DrawUI(8192,3072,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			DrawStringUI(str,465,2445,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

			sprintf(str,"R %d",meng.obj.color.r);

			if(meng.sub_com!=1)
			{
				if(CheckColisionMouse(8192,2048+455,2048,227,0))
				{
					DrawStringUI(str,8192,2048+455,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj.color.r);
						StartText();
						st.mouse1=0;
						meng.sub_com=1;
					}
				}
				else
					DrawStringUI(str,8192,2048+455,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				DrawStringUI(str,8192,2048+455,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,2048+810,2048,227,0))
				{
					DrawStringUI(str,8192,2048+810,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj.color.g);
						StartText();
						st.mouse1=0;
						meng.sub_com=2;
					}
				}
				else
					DrawStringUI(str,8192,2048+810,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				DrawStringUI(str,8192,2048+810,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,2048+1265,2048,455,0))
				{
					DrawStringUI(str,8192,2048+1265,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj.color.b);
						StartText();
						st.mouse1=0;
						meng.sub_com=3;
					}
				}
				else
					DrawStringUI(str,8192,2048+1265,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==3)
			{
				DrawStringUI(str,8192,2048+1265,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,2048+1720,2048,455,0))
				{
					DrawStringUI(str,8192,2048+1720,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj.color.a);
						StartText();
						st.mouse1=0;
						meng.sub_com=4;
					}
				}
				else
					DrawStringUI(str,8192,2048+1720,0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==4)
			{
				DrawStringUI(str,8192,2048+1720,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.obj.color.a=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.sub_com=0;
				}
			}

			if(!CheckColisionMouse(465,2445,810,217,0) && st.mouse1)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}

		sprintf(str,"Light %.2f",meng.obj.amblight);

		if(CheckColisionMouse(465,2887,810,217,0))
		{
			DrawStringUI(str,465,2887,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.command=OBJ_AMBL;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,2887,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==OBJ_AMBL)
		{
			DrawStringUI(str,465,2887,810,217,0,255,32,32,255,ARIAL,0,0,0);

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

			if(!CheckColisionMouse(465,2887,810,217,0) && st.mouse1)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}

		sprintf(str,"Tex.Size");

		if(CheckColisionMouse(465,3315,810,217,0))
		{
			DrawStringUI(str,465,3315,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1 && meng.command!=TEX_PAN_OBJ)
			{
				meng.command=TEX_SIZE_OBJ;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,3315,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==TEX_SIZE_OBJ)
		{
			DrawUI(8192,4096,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			sprintf(str,"X %d",meng.obj.texsize.x);

			if(meng.sub_com!=1)
			{
				if(CheckColisionMouse(8192,3641,2048,910,0))
				{
					DrawStringUI(str,8192,3641,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

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
					DrawStringUI(str,8192,3641,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				DrawStringUI(str,8192,3641,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,4551,2048,910,0))
				{
					DrawStringUI(str,8192,4551,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

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
					DrawStringUI(str,8192,4551,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				DrawStringUI(str,8192,4551,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

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

			if(!CheckColisionMouse(465,3315,810,217,0) && st.mouse1 && meng.sub_com==0)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}

		sprintf(str,"Tex.Pan");

		if(CheckColisionMouse(465,3640,810,217,0))
		{
			DrawStringUI(str,465,3640,810,217,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1 && meng.command!=TEX_SIZE_OBJ)
			{
				meng.command=TEX_PAN_OBJ;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,3640,810,217,0,255,255,255,255,ARIAL,0,0,0);

		if(meng.command==TEX_PAN_OBJ)
		{
			DrawUI(8192,4096,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

			sprintf(str,"X %d",meng.obj.texpan.x);

			if(meng.sub_com!=1)
			{
				if(CheckColisionMouse(8192,3641,2048,910,0))
				{
					DrawStringUI(str,8192,3641,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

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
					DrawStringUI(str,8192,3641,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				DrawStringUI(str,8192,3641,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

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
				if(CheckColisionMouse(8192,4551,2048,910,0))
				{
					DrawStringUI(str,8192,4551,810,217,0,255,128,32,255,ARIAL,2048,2048,0);

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
					DrawStringUI(str,8192,4551,810,217,0,255,255,255,255,ARIAL,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				DrawStringUI(str,8192,4551,810,217,0,255,32,32,255,ARIAL,2048,2048,0);

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

			if(!CheckColisionMouse(465,3315,810,217,0) && st.mouse1 && meng.sub_com==0)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}

	}

	if(meng.command==ADD_OBJ_TYPE)
	{
		DrawUI(1365,1715,910,1185,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,0);

		if(CheckColisionMouse(1365,1715,810,227,0))
		{
			DrawStringUI("Background1",1365,1715,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.obj.type=BACKGROUND1;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI("Background1",1365,1715,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckColisionMouse(1365,1465,810,227,0))
		{
			DrawStringUI("Background2",1365,1465,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.obj.type=BACKGROUND2;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI("Background2",1365,1465,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckColisionMouse(1365,1215,810,227,0))
		{
			DrawStringUI("Background3",1365,1215,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.obj.type=BACKGROUND3;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI("Background3",1365,1215,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckColisionMouse(1365,1920,810,227,0))
		{
			DrawStringUI("Midground",1365,1920,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.obj.type=MIDGROUND;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI("Midground",1365,1920,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckColisionMouse(1365,2170,810,227,0))
		{
			DrawStringUI("Foreground",1365,2170,810,227,0,255,128,32,255,ARIAL,0,0,0);

			if(st.mouse1)
			{
				meng.obj.type=FOREGROUND;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI("Foreground",1365,2170,810,227,0,255,255,255,255,ARIAL,0,0,0);
	}

	if(meng.command==EDIT_OBJ_TYPE)
	{
		DrawUI(1365,1715,910,1210,0,255,255,255,0,0,32768,32768,mgg_sys[0].frames[4],255,0);

		if(CheckColisionMouse(1365,1715,810,227,0))
		{
			DrawStringUI("Background1",1365,1715,810,227,0,255,128,32,255,ARIAL,0,0,0);

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
			DrawStringUI("Background1",1365,1715,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckColisionMouse(1365,1465,810,227,0))
		{
			DrawStringUI("Background2",1365,1465,810,227,0,255,128,32,255,ARIAL,0,0,0);

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
			DrawStringUI("Background2",1365,1465,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckColisionMouse(1365,1215,810,227,0))
		{
			DrawStringUI("Background3",1365,1215,810,227,0,255,128,32,255,ARIAL,0,0,0);

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
			DrawStringUI("Background3",1365,1215,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckColisionMouse(1365,1920,810,227,0))
		{
			DrawStringUI("Midground",1365,1920,810,227,0,255,128,32,255,ARIAL,0,0,0);

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
			DrawStringUI("Midground",1365,1920,810,227,0,255,255,255,255,ARIAL,0,0,0);

		if(CheckColisionMouse(1365,2170,810,227,0))
		{
			DrawStringUI("Foreground",1365,2170,810,227,0,255,128,32,255,ARIAL,0,0,0);

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
			DrawStringUI("Foreground",1365,2170,810,227,0,255,255,255,255,ARIAL,0,0,0);
	}

}

static void ViewPortCommands()
{
	Pos vertextmp[4];
	uint8 got_it=0;
	char str[64];
	uint16 i, j;
	static Pos p, p2;
	uPos16 p3;
	float tmp;
	static int16 temp;

	if(!CheckColisionMouse(455,4096,910,8192,0) && meng.command!=MGG_SEL)
	{
		if(meng.command==DRAW_SECTOR)
		{
			if(st.mouse1)
			{
				if(st.Current_Map.num_sector<MAX_SECTORS)
				{
					for(i=0;i<MAX_SECTORS;i++)
					{
						if(st.Current_Map.sector[i].id==-1)
						{
							st.Current_Map.sector[i].id=i;

							st.Current_Map.sector[i].position.x=st.mouse.x;
							st.Current_Map.sector[i].position.y=st.mouse.y;
							STW(&st.Current_Map.sector[i].position.x,&st.Current_Map.sector[i].position.y);

							st.Current_Map.sector[i].vertex[0].x=st.mouse.x-64;
							st.Current_Map.sector[i].vertex[0].y=st.mouse.y-64;
							STW(&st.Current_Map.sector[i].vertex[0].x,&st.Current_Map.sector[i].vertex[0].y);

							st.Current_Map.sector[i].vertex[1].x=st.mouse.x+64;
							st.Current_Map.sector[i].vertex[1].y=st.mouse.y-64;
							STW(&st.Current_Map.sector[i].vertex[1].x,&st.Current_Map.sector[i].vertex[1].y);

							st.Current_Map.sector[i].vertex[2].x=st.mouse.x+64;
							st.Current_Map.sector[i].vertex[2].y=st.mouse.y+64;
							STW(&st.Current_Map.sector[i].vertex[2].x,&st.Current_Map.sector[i].vertex[2].y);

							st.Current_Map.sector[i].vertex[3].x=st.mouse.x-64;
							st.Current_Map.sector[i].vertex[3].y=st.mouse.y+64;
							STW(&st.Current_Map.sector[i].vertex[3].x,&st.Current_Map.sector[i].vertex[3].y);

							st.Current_Map.num_sector++;
							break;
						}
					}
				}
				LogApp("Sector added");
				st.mouse1=0;
			}
		}
		else
		if(meng.command==SELECT_EDIT)
		{
			if(st.Current_Map.num_sector>0)
			{
				for(i=0;i<st.Current_Map.num_sector;i++)
				{
					if(got_it) break;

					for(j=0;j<5;j++)
					{
						if(j<4 && CheckColisionMouseWorld(st.Current_Map.sector[i].vertex[j].x,st.Current_Map.sector[i].vertex[j].y,256,256,0) && st.mouse1)
						{
							st.Current_Map.sector[i].vertex[j].x=st.mouse.x;
							st.Current_Map.sector[i].vertex[j].y=st.mouse.y;
							STW(&st.Current_Map.sector[i].vertex[j].x,&st.Current_Map.sector[i].vertex[j].y);

							got_it=1;
							break;
						}

						if(j==4 && CheckColisionMouseWorld(st.Current_Map.sector[i].position.x,st.Current_Map.sector[i].position.y,484,484,0) && st.mouse1)
						{
							vertextmp[0].x=(st.Current_Map.sector[i].vertex[0].x-st.Current_Map.sector[i].position.x);
							vertextmp[0].y=(st.Current_Map.sector[i].vertex[0].y-st.Current_Map.sector[i].position.y);

							vertextmp[1].x=(st.Current_Map.sector[i].vertex[1].x-st.Current_Map.sector[i].position.x);
							vertextmp[1].y=(st.Current_Map.sector[i].vertex[1].y-st.Current_Map.sector[i].position.y);

							vertextmp[2].x=(st.Current_Map.sector[i].vertex[2].x-st.Current_Map.sector[i].position.x);
							vertextmp[2].y=(st.Current_Map.sector[i].vertex[2].y-st.Current_Map.sector[i].position.y);

							vertextmp[3].x=(st.Current_Map.sector[i].vertex[3].x-st.Current_Map.sector[i].position.x);
							vertextmp[3].y=(st.Current_Map.sector[i].vertex[3].y-st.Current_Map.sector[i].position.y);

							st.Current_Map.sector[i].position.x=st.mouse.x;
							st.Current_Map.sector[i].position.y=st.mouse.y;
							STW(&st.Current_Map.sector[i].position.x,&st.Current_Map.sector[i].position.y);

							st.Current_Map.sector[i].vertex[0].x=st.Current_Map.sector[i].position.x+vertextmp[0].x;
							st.Current_Map.sector[i].vertex[0].y=st.Current_Map.sector[i].position.y+vertextmp[0].y;

							st.Current_Map.sector[i].vertex[1].x=st.Current_Map.sector[i].position.x+vertextmp[1].x;
							st.Current_Map.sector[i].vertex[1].y=st.Current_Map.sector[i].position.y+vertextmp[1].y;

							st.Current_Map.sector[i].vertex[2].x=st.Current_Map.sector[i].position.x+vertextmp[2].x;
							st.Current_Map.sector[i].vertex[2].y=st.Current_Map.sector[i].position.y+vertextmp[2].y;

							st.Current_Map.sector[i].vertex[3].x=st.Current_Map.sector[i].position.x+vertextmp[3].x;
							st.Current_Map.sector[i].vertex[3].y=st.Current_Map.sector[i].position.y+vertextmp[3].y;

							got_it=1;
							break;
						}
					}
				}
			}
			
			if(st.Current_Map.num_sprites>0)
			{
				for(i=0;i<st.Current_Map.num_sprites;i++)
				{
					if(got_it) break;

					if(CheckColisionMouseWorld(st.Current_Map.sprites[i].position.x,st.Current_Map.sprites[i].position.y,st.Current_Map.sprites[i].body.size.x,st.Current_Map.sprites[i].body.size.y,st.Current_Map.sprites[i].angle))
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

							p.x-=st.Camera.position.x;
							p.y-=st.Camera.position.y;

							sprintf(str,"%d",st.Current_Map.sprites[i].angle);
							DrawString2(str,p.x+227,p.y-227,405,217,0,255,255,255,255,ARIAL,1024,1024,0);

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

			if(st.Current_Map.num_obj>0)
			{
				for(i=0;i<st.Current_Map.num_obj;i++)
				{
					if(got_it) break;

					if(CheckColisionMouseWorld(st.Current_Map.obj[i].position.x,st.Current_Map.obj[i].position.y,st.Current_Map.obj[i].size.x,st.Current_Map.obj[i].size.y,st.Current_Map.obj[i].angle))
					{
						if(st.mouse1)
						{
							if(meng.got_it==-1)
							{
								meng.command2=EDIT_OBJ;
								meng.p=st.mouse;
								meng.got_it=i;

								STW(&meng.p.x, &meng.p.y);

								meng.p.x-=st.Current_Map.obj[i].position.x;
								meng.p.y-=st.Current_Map.obj[i].position.y;
							}
							else
							if(meng.got_it!=-1 && meng.got_it!=i)
								continue;

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

							p.x-=st.Camera.position.x;
							p.y-=st.Camera.position.y;

							sprintf(str,"%d",st.Current_Map.obj[i].angle);
							DrawString2(str,p.x+227,p.y-227,405,217,0,255,255,255,255,ARIAL,1024,1024,0);

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

							got_it=1;
							break;
						}
						else
							meng.got_it=-1;
					}
				}
			}
		}
		else
		if(meng.command==ADD_OBJ)
		{
			if(st.mouse1)
			{
				if(st.Current_Map.num_obj<MAX_OBJS)
				{
					for(i=0;i<MAX_OBJS;i++)
					{
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
							STW(&st.Current_Map.obj[i].position.x,&st.Current_Map.obj[i].position.y);
							st.Current_Map.obj[i].size.x=meng.pre_size.x;
							st.Current_Map.obj[i].size.y=meng.pre_size.y;
							st.Current_Map.obj[i].angle=0;

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

							st.Current_Map.num_obj++;
							break;
						}
					}
				}

				LogApp("Object added");
				st.mouse1=0;
			}
		}
		else
		if(meng.command==ADD_SPRITE)
		{
			if(st.mouse1)
			{
				if(st.Current_Map.num_sprites<MAX_SPRITES)
				{
					for(i=0;i<MAX_SPRITES;i++)
					{
						if(st.Current_Map.sprites[i].stat==0)
						{
							st.Current_Map.sprites[i].stat=1;
							st.Current_Map.sprites[i].color=meng.spr.color;
							st.Current_Map.sprites[i].health=meng.spr.health;
							st.Current_Map.sprites[i].body=meng.spr.body;
							st.Current_Map.sprites[i].GameID=meng.sprite_selection;
							st.Current_Map.sprites[i].frame_ID=meng.sprite_frame_selection;
							st.Current_Map.sprites[i].type_s=meng.spr.type;

							st.Current_Map.sprites[i].position=st.mouse;
							STW(&st.Current_Map.sprites[i].position.x,&st.Current_Map.sprites[i].position.y);

							//st.Current_Map.sprites[i].body.size=meng.spr.size;
							memcpy(st.Current_Map.sprites[i].tags,st.Game_Sprites[meng.sprite_selection].tags,8*sizeof(int16));
							st.Current_Map.sprites[i].num_tags=st.Game_Sprites[meng.sprite_selection].num_tags;
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

							st.Current_Map.num_sprites++;

							LogApp("Sprite Added");
							st.mouse1=0;

							break;
						}
					}
				}
			}
		}
		else
		if(meng.pannel_choice==ADD_LIGHT && meng.command==EDIT_LIGHTMAP)
		{
			for(j=0;j<st.num_lightmap;j++)
			{
				if(CheckColisionMouseWorld(st.game_lightmaps[j].w_pos.x,st.game_lightmaps[j].w_pos.y,st.game_lightmaps[j].W_w,st.game_lightmaps[j].W_h,st.game_lightmaps[j].ang) && st.mouse1)
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
					p3=st.game_lightmaps[i].t_pos[meng.light.light_id];

					p3.x=(st.game_lightmaps[i].W_w*st.game_lightmaps[i].t_pos[meng.light.light_id].x)/st.game_lightmaps[i].T_w;
					p3.y=(st.game_lightmaps[i].W_h*st.game_lightmaps[i].t_pos[meng.light.light_id].y)/st.game_lightmaps[i].T_h;

					p3.x=p3.x+st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2);
					p3.y=p3.y+st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2);

					if(CheckColisionMouseWorld(p3.x,p3.y,455,455,0) && st.mouse1)
					{
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

					if(st.keys[LSHIFT_KEY].state)
					{
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
				}

				if(meng.com_id==0)
				{
					p=st.game_lightmaps[i].w_pos;

					p.x+=(st.game_lightmaps[i].W_w/2)+1920;

					if(CheckColisionMouseWorld(p.x,p.y,455,455,0))
					{
						DrawString2("Done",p.x,p.y,0,0,0,255,128,32,255,ARIAL,1536,1536,0);

						if(st.mouse1)
						{
							meng.got_it=-1;
							meng.command=meng.pannel_choice=meng.command2=ADD_LIGHT;

							st.mouse1=0;
						}
					}
					else
						DrawString2("Done",p.x,p.y,0,0,0,255,255,255,255,ARIAL,1536,1536,0);
				}
				else
				{
					p=st.game_lightmaps[i].w_pos;

					p.x+=(st.game_lightmaps[i].W_w/2)+1920;
					p.y+=810;

					if(CheckColisionMouseWorld(p.x,p.y,455,455,0))
					{
						DrawString2("Done Light",p.x,p.y,0,0,0,255,128,32,255,ARIAL,1536,1536,0);

						if(st.mouse1)
						{
							meng.com_id=0;

							if(st.game_lightmaps[i].alpha)
							{
								AddLightToAlphaLight(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,meng.light.color.r,meng.light.color.g,meng.light.color.b,meng.light.falloff,
									st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,st.game_lightmaps[i].t_pos[meng.light.light_id].z,meng.light.intensity,meng.light.type);
								AddLightToAlphaTexture(&st.game_lightmaps[i].tex,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
							}
							else
							{
								AddLightToLightmap(st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,meng.light.color.r,meng.light.color.g,meng.light.color.b,meng.light.falloff,
									st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,st.game_lightmaps[i].t_pos[meng.light.light_id].z,meng.light.intensity,meng.light.type);
								AddLightToTexture(&st.game_lightmaps[i].tex,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
							}


							st.mouse1=0;
						}
					}
					else
						DrawString2("Done Light",p.x,p.y,0,0,0,255,255,255,255,ARIAL,1536,1536,0);
				}

				p=st.game_lightmaps[i].w_pos;

				p.y-=(st.game_lightmaps[i].W_h/2)+810;

				if(CheckColisionMouseWorld(p.x,p.y,455,455,0))
				{
					DrawString2("Edit light",p.x,p.y,0,0,0,255,128,32,255,ARIAL,1536,1536,0);

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
					DrawString2("Edit light",p.x,p.y,0,0,0,255,255,255,255,ARIAL,1536,1536,0);

				p=st.game_lightmaps[i].w_pos;

				p.y+=(st.game_lightmaps[i].W_h/2)+810;

				if(CheckColisionMouseWorld(p.x,p.y,455,455,0))
				{
					DrawString2("Add Light",p.x,p.y,0,0,0,255,128,32,255,ARIAL,1536,1536,0);

					if(st.mouse1)
					{
						meng.com_id=1;

						st.game_lightmaps[i].num_lights++;
						meng.light.light_id=st.game_lightmaps[i].num_lights-1;

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
						meng.light.intensity=255.0f;
						meng.light.type=POINT_LIGHT_MEDIUM;

						if(st.game_lightmaps[i].alpha)
						{
							meng.tmplightdata=malloc(st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

							memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*4);

							AddLightToAlphaLight(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,meng.light.color.r,meng.light.color.g,meng.light.color.b,meng.light.falloff,
								st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,st.game_lightmaps[i].t_pos[meng.light.light_id].z,meng.light.intensity,meng.light.type);
							AddLightToAlphaTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}
						else
						{
							meng.tmplightdata=malloc(st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

							memcpy(meng.tmplightdata,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w*st.game_lightmaps[i].T_h*3);

							AddLightToLightmap(meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h,meng.light.color.r,meng.light.color.g,meng.light.color.b,meng.light.falloff,
								st.game_lightmaps[i].t_pos[meng.light.light_id].x,st.game_lightmaps[i].t_pos[meng.light.light_id].y,st.game_lightmaps[i].t_pos[meng.light.light_id].z,meng.light.intensity,meng.light.type);
							AddLightToTexture(&st.game_lightmaps[i].tex,meng.tmplightdata,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						}

						//LogApp("Light added");

						st.mouse1=0;
					}
				}
				else
					DrawString2("Add Light",p.x,p.y,0,0,0,255,255,255,255,ARIAL,1536,1536,0);
			}
			else
			if(meng.sub_com==1)
			{
				if(meng.com_id==1)
				{
					//DrawString2("Edit light",p.x,p.y,0,0,0,255,32,32,255,ARIAL,1536,1536,0);

					DrawUI(8192,4300,3072,3900,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,7);

					sprintf(str,"R %d",st.game_lightmaps[i].color[meng.light.light_id].r);

					if(CheckColisionMouse(8192,4096-(341*3),341,341,0))
					{
						DrawStringUI(str,8192,4096-(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							StartText();
							sprintf(st.TextInput,"%d",st.game_lightmaps[i].color[meng.light.light_id].r);
							meng.command2=1;
							st.mouse1=0;
						}
					}
					else
						DrawStringUI(str,8192,4096-(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==1)
					{
						DrawStringUI(str,8192,4096-(341*3),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

						st.game_lightmaps[i].color[meng.light.light_id].r=atoi(st.TextInput);

						if(st.keys[RETURN_KEY].state)
						{
							StopText();
							st.keys[RETURN_KEY].state=0;
							meng.command2=0;
						}
					}

					sprintf(str,"G %d",st.game_lightmaps[i].color[meng.light.light_id].g);

					if(CheckColisionMouse(8192,4096-(341*2),341,341,0))
					{
						DrawStringUI(str,8192,4096-(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							StartText();
							sprintf(st.TextInput,"%d",st.game_lightmaps[i].color[meng.light.light_id].g);
							meng.command2=2;
							st.mouse1=0;
						}
					}
					else
						DrawStringUI(str,8192,4096-(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==2)
					{
						DrawStringUI(str,8192,4096-(341*2),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

						st.game_lightmaps[i].color[meng.light.light_id].g=atoi(st.TextInput);

						if(st.keys[RETURN_KEY].state)
						{
							StopText();
							st.keys[RETURN_KEY].state=0;
							meng.command2=0;
						}
					}

					sprintf(str,"B %d",st.game_lightmaps[i].color[meng.light.light_id].b);

					if(CheckColisionMouse(8192,4096-341,341,341,0))
					{
						DrawStringUI(str,8192,4096-341,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							StartText();
							sprintf(st.TextInput,"%d",st.game_lightmaps[i].color[meng.light.light_id].b);
							meng.command2=3;
							st.mouse1=0;
						}
					}
					else
						DrawStringUI(str,8192,4096-341,0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==3)
					{
						DrawStringUI(str,8192,4096-341,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

						st.game_lightmaps[i].color[meng.light.light_id].b=atoi(st.TextInput);

						if(st.keys[RETURN_KEY].state)
						{
							StopText();
							st.keys[RETURN_KEY].state=0;
							meng.command2=0;
						}
					}

					if(meng.command2!=4)
						sprintf(str,"Intensity %.3f",st.game_lightmaps[i].color[meng.light.light_id].a);
					else
						strcpy(str,st.TextInput);

					if(CheckColisionMouse(8192,4096,341,341,0))
					{
						DrawStringUI(str,8192,4096,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							StartText();
							sprintf(st.TextInput,"%.3f",st.game_lightmaps[i].color[meng.light.light_id].a);
							meng.command2=4;
							st.mouse1=0;
						}
					}
					else
						DrawStringUI(str,8192,4096,0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==4)
					{
						DrawStringUI(str,8192,4096,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

						st.game_lightmaps[i].color[meng.light.light_id].a=atof(st.TextInput);

						if(st.keys[RETURN_KEY].state)
						{
							StopText();
							st.keys[RETURN_KEY].state=0;
							meng.command2=0;
						}
					}

					if(meng.command2!=5)
						sprintf(str,"Fall Off %.3f",st.game_lightmaps[i].falloff[meng.light.light_id]);
					else
						strcpy(str,st.TextInput);

					if(CheckColisionMouse(8192,4096+341,341,341,0))
					{
						DrawStringUI(str,8192,4096+341,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							StartText();
							sprintf(st.TextInput,"%.3f",st.game_lightmaps[i].falloff[meng.light.light_id]);
							meng.command2=5;
							st.mouse1=0;
						}
					}
					else
						DrawStringUI(str,8192,4096+341,0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==5)
					{
						DrawStringUI(str,8192,4096+341,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

						st.game_lightmaps[i].falloff[meng.light.light_id]=atof(st.TextInput);

						if(st.keys[RETURN_KEY].state)
						{
							StopText();
							st.keys[RETURN_KEY].state=0;
							meng.command2=0;
						}
					}

					sprintf(str,"Z %d",st.game_lightmaps[i].t_pos[meng.light.light_id].z);

					if(CheckColisionMouse(8192,4096+(341*2),341,341,0))
					{
						DrawStringUI(str,8192,4096+(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							StartText();
							sprintf(st.TextInput,"%d",st.game_lightmaps[i].t_pos[meng.light.light_id].z);
							meng.command2=6;
							st.mouse1=0;
						}
					}
					else
						DrawStringUI(str,8192,4096+(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==6)
					{
						DrawStringUI(str,8192,4096+(341*2),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

						st.game_lightmaps[i].t_pos[meng.light.light_id].z=atoi(st.TextInput);

						if(st.keys[RETURN_KEY].state)
						{
							StopText();
							st.keys[RETURN_KEY].state=0;
							meng.command2=0;
						}
					}

					if(st.game_lightmaps[i].type[meng.light.light_id]==POINT_LIGHT_MEDIUM) strcpy(str,"Point medium");
					else if(st.game_lightmaps[i].type[meng.light.light_id]==POINT_LIGHT_STRONG) strcpy(str,"Point strong");
					else if(st.game_lightmaps[i].type[meng.light.light_id]==POINT_LIGHT_NORMAL) strcpy(str,"Point normal");
					else if(st.game_lightmaps[i].type[meng.light.light_id]==SPOTLIGHT_MEDIUM) strcpy(str,"Spotlight medium");
					else if(st.game_lightmaps[i].type[meng.light.light_id]==SPOTLIGHT_STRONG) strcpy(str,"Spotlight strong");
					else if(st.game_lightmaps[i].type[meng.light.light_id]==SPOTLIGHT_NORMAL) strcpy(str,"Spotlight normal");

					if(CheckColisionMouse(8192,4096+(341*3),341,341,0))
					{
						DrawStringUI(str,8192,4096+(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.command2=7;
							st.mouse1=0;
						}
					}
					else
						DrawStringUI(str,8192,4096+(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==7)
					{
						DrawUI(11264,4096,3072,3072,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

						if(CheckColisionMouse(11264,4096-(341*3),341,341,0))
						{
							DrawStringUI("Point medium",11264,4096-(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

							if(st.mouse1)
							{
								st.game_lightmaps[i].type[meng.light.light_id]=POINT_LIGHT_MEDIUM;
								meng.command2=-1;
								st.mouse1=0;
							}
						}
						else
							DrawStringUI("Point medium",11264,4096-(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

						if(CheckColisionMouse(11264,4096-(341*2),341,341,0))
						{
							DrawStringUI("Point strong",11264,4096-(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

							if(st.mouse1)
							{
								st.game_lightmaps[i].type[meng.light.light_id]=POINT_LIGHT_STRONG;
								meng.command2=-1;
								st.mouse1=0;
							}
						}
						else
							DrawStringUI("Point strong",11264,4096-(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

						if(CheckColisionMouse(11264,4096-(341),341,341,0))
						{
							DrawStringUI("Point normal",11264,4096-(341),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

							if(st.mouse1)
							{
								st.game_lightmaps[i].type[meng.light.light_id]=POINT_LIGHT_NORMAL;
								meng.command2=-1;
								st.mouse1=0;
							}
						}
						else
							DrawStringUI("Point normal",11264,4096-(341),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

						if(CheckColisionMouse(11264,4096+(341),341,341,0))
						{
							DrawStringUI("Spotlight medium",11264,4096+(341),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

							if(st.mouse1)
							{
								st.game_lightmaps[i].type[meng.light.light_id]=SPOTLIGHT_MEDIUM;
								meng.command2=-1;
								st.mouse1=0;
							}
						}
						else
							DrawStringUI("Spotlight medium",11264,4096+(341),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

						if(CheckColisionMouse(11264,4096+(341*2),341,341,0))
						{
							DrawStringUI("Spotlight strong",11264,4096+(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

							if(st.mouse1)
							{
								st.game_lightmaps[i].type[meng.light.light_id]=SPOTLIGHT_STRONG;
								meng.command2=-1;
								st.mouse1=0;
							}
						}
						else
							DrawStringUI("Spotlight STRONG",11264,4096+(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

						if(CheckColisionMouse(11264,4096+(341*3),341,341,0))
						{
							DrawStringUI("Spotlight normal",11264,4096+(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

							if(st.mouse1)
							{
								st.game_lightmaps[i].type[meng.light.light_id]=SPOTLIGHT_NORMAL;
								meng.command2=-1;
								st.mouse1=0;
							}
						}
						else
							DrawStringUI("Spotlight normal",11264,4096+(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

						if(st.keys[RETURN_KEY].state)
						{
							meng.command2=-1;
							st.keys[RETURN_KEY].state=0;
						}
					}

					sprintf(str,"Spot angle %d",st.game_lightmaps[i].spot_ang[meng.light.light_id]);

					if(CheckColisionMouse(8192,4096+(341*4),341,341,0))
					{
						DrawStringUI(str,8192,4096+(341*4),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							StartText();
							sprintf(st.TextInput,"%d",st.game_lightmaps[i].spot_ang[meng.light.light_id]);
							meng.command=8;
							st.mouse1=0;
						}
					}
					else
						DrawStringUI(str,8192,4096+(341*4),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(meng.command2==8)
					{
						DrawStringUI(str,8192,4096+(341*4),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

						st.game_lightmaps[i].spot_ang[meng.light.light_id]=atoi(st.TextInput);

						if(st.keys[RETURN_KEY].state)
						{
							StopText();
							st.keys[RETURN_KEY].state=0;
							meng.command2=0;
						}
					}


					if(CheckColisionMouse(8192,4096+(341*5),455,455,0))
					{
						DrawStringUI("Done",8192,4096+(341*5),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.sub_com=0;
							meng.command2=-1;

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
						DrawStringUI("Done",8192,4096+(341*5),0,0,0,255,255,255,255,ARIAL,2048,2048,0);
				}
			}
		}
		else
		if(meng.pannel_choice==ADD_LIGHT && meng.command==CREATE_LIGHTMAP)
		{
			if(st.num_lightmap<MAX_LIGHTMAPS)
			{
				if(st.Current_Map.num_obj==0)
				{
					if(((meng.obj_lightmap_sel==-2 && !CheckColisionMouseWorld(meng.lightmappos.x,meng.lightmappos.y,meng.lightmapsize.x+128,meng.lightmapsize.y+128,0)) || meng.obj_lightmap_sel!=-2) && temp<2)
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
					if(CheckColisionMouseWorld(st.Current_Map.obj[j].position.x,st.Current_Map.obj[j].position.y,st.Current_Map.obj[j].size.x,st.Current_Map.obj[j].size.y,st.Current_Map.obj[j].angle))
					{
						if(st.mouse1)
						{
							meng.obj_lightmap_sel=j;
							st.mouse1=0;
							break;
						}
					}
					else
					if(j==st.Current_Map.num_obj-1 && 
						!CheckColisionMouseWorld(st.Current_Map.obj[j].position.x,st.Current_Map.obj[j].position.y,st.Current_Map.obj[j].size.x,st.Current_Map.obj[j].size.y,st.Current_Map.obj[j].angle))
					{
						if(((meng.obj_lightmap_sel==-2 && !CheckColisionMouseWorld(meng.lightmappos.x,meng.lightmappos.y,meng.lightmapsize.x+128,meng.lightmapsize.y+128,0)) || meng.obj_lightmap_sel!=-2) && temp<2)
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

				if(meng.obj_lightmap_sel==-2)
				{
					if(st.mouse1)
					{
						if(CheckColisionMouseWorld(meng.lightmappos.x-(meng.lightmapsize.x/2),meng.lightmappos.y,64,meng.lightmapsize.y,0) && temp==1)
						{
							p=st.mouse;
							temp=2;
						}
						
						else
						if(CheckColisionMouseWorld(meng.lightmappos.x+(meng.lightmapsize.x/2),meng.lightmappos.y,64,meng.lightmapsize.y,0) && temp==1)
						{
							p=st.mouse;
							temp=3;
						}
						else
						if(CheckColisionMouseWorld(meng.lightmappos.x,meng.lightmappos.y-(meng.lightmapsize.y/2),meng.lightmapsize.x,64,0) && temp==1)
						{
							p=st.mouse;
							temp=4;
						}
						else
						if(CheckColisionMouseWorld(meng.lightmappos.x,meng.lightmappos.y+(meng.lightmapsize.y/2),meng.lightmapsize.x,64,0) && temp==1)
						{
							p=st.mouse;
							temp=5;
						}
						else
						if(CheckColisionMouseWorld(meng.lightmappos.x-(meng.lightmapsize.x/2),meng.lightmappos.y-(meng.lightmapsize.y/2),64,64,0) && temp==1)
						{
							p=st.mouse;
							temp=6;
						}
						else
						if(CheckColisionMouseWorld(meng.lightmappos.x+(meng.lightmapsize.x/2),meng.lightmappos.y-(meng.lightmapsize.y/2),64,64,0) && temp==1)
						{
							p=st.mouse;
							temp=7;
						}
						else
						if(CheckColisionMouseWorld(meng.lightmappos.x+(meng.lightmapsize.x/2),meng.lightmappos.y+(meng.lightmapsize.y/2),64,64,0) && temp==1)
						{
							p=st.mouse;
							temp=8;
						}
						else
						if(CheckColisionMouseWorld(meng.lightmappos.x-(meng.lightmapsize.x/2),meng.lightmappos.y+(meng.lightmapsize.y/2),64,64,0) && temp==1)
						{
							p=st.mouse;
							temp=9;
						}

						if(temp==2)
						{
							p2=st.mouse;

							p2.x-=p.x;
							p2.y-=p.y;

							STW(&p2.x,&p2.y);

							meng.lightmapsize.x-=p2.x;

							p=st.mouse;
						}
						else
						if(temp==3)
						{
							p2=st.mouse;

							p2.x-=p.x;
							p2.y-=p.y;

							STW(&p2.x,&p2.y);

							meng.lightmapsize.x+=p2.x;

							p=st.mouse;
						}
						else
						if(temp==4)
						{
							p2=st.mouse;

							p2.x-=p.x;
							p2.y-=p.y;

							STW(&p2.x,&p2.y);

							meng.lightmapsize.y-=p2.y;

							p=st.mouse;
						}
						else
						if(temp==5)
						{
							p2=st.mouse;

							p2.x-=p.x;
							p2.y-=p.y;

							STW(&p2.x,&p2.y);

							meng.lightmapsize.y+=p2.y;

							p=st.mouse;
						}
						else
						if(temp==6)
						{
							p2=st.mouse;

							p2.x-=p.x;
							p2.y-=p.y;

							STW(&p2.x,&p2.y);

							meng.lightmapsize.x-=p2.x;
							meng.lightmapsize.y-=p2.y;

							p=st.mouse;
						}
						else
						if(temp==7)
						{
							p2=st.mouse;

							p2.x-=p.x;
							p2.y-=p.y;

							STW(&p2.x,&p2.y);

							meng.lightmapsize.x+=p2.x;
							meng.lightmapsize.y-=p2.y;

							p=st.mouse;
						}
						else
						if(temp==8)
						{
							p2=st.mouse;

							p2.x-=p.x;
							p2.y-=p.y;

							STW(&p2.x,&p2.y);

							meng.lightmapsize.x+=p2.x;
							meng.lightmapsize.y+=p2.y;

							p=st.mouse;
						}
						else
						if(temp==9)
						{
							p2=st.mouse;

							p2.x-=p.x;
							p2.y-=p.y;

							STW(&p2.x,&p2.y);

							meng.lightmapsize.x-=p2.x;
							meng.lightmapsize.y+=p2.y;

							p=st.mouse;
						}
					}
					else
						temp=1;
				}

				if(st.keys[RETURN_KEY].state && meng.obj_lightmap_sel>-1)
				{
					for(i=0;i<MAX_LIGHTMAPS;i++)
					{
						if(!st.game_lightmaps[i].stat)
						{
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

			DrawUI(8192,4096,2048,2730,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,7);

			if(st.game_lightmaps[i].alpha)
				sprintf(str,"Alpha light [X]");
			else
				sprintf(str,"Alpha light [ ]");

			if(CheckColisionMouse(8192,4096-(341*3),341,341,0))
			{
				DrawStringUI(str,8192,4096-(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					if(st.game_lightmaps[i].alpha)
						st.game_lightmaps[i].alpha=0;
					else
						st.game_lightmaps[i].alpha=1;

					st.mouse1=0;
				}
			}
			else
				DrawStringUI(str,8192,4096-(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

			sprintf(str,"R %d",meng.lightmap_color.r);

			if(CheckColisionMouse(8192,4096-(341*2),341,341,0))
			{
				DrawStringUI(str,8192,4096-(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					StartText();
					sprintf(st.TextInput,"%d",meng.lightmap_color.r);
					meng.got_it=1;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI(str,8192,4096-(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

			if(meng.got_it==1)
			{
				DrawStringUI(str,8192,4096-(341*2),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.lightmap_color.r=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.got_it=0;
				}
			}

			sprintf(str,"G %d",meng.lightmap_color.g);

			if(CheckColisionMouse(8192,4096-(341),341,341,0))
			{
				DrawStringUI(str,8192,4096-(341),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					StartText();
					sprintf(st.TextInput,"%d",meng.lightmap_color.g);
					meng.got_it=2;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI(str,8192,4096-(341),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

			if(meng.got_it==2)
			{
				DrawStringUI(str,8192,4096-(341),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.lightmap_color.g=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.got_it=0;
				}
			}

			sprintf(str,"B %d",meng.lightmap_color.b);

			if(CheckColisionMouse(8192,4096,341,341,0))
			{
				DrawStringUI(str,8192,4096,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					StartText();
					sprintf(st.TextInput,"%d",meng.lightmap_color.b);
					meng.got_it=3;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI(str,8192,4096,0,0,0,255,255,255,255,ARIAL,2048,2048,0);

			if(meng.got_it==3)
			{
				DrawStringUI(str,8192,4096,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				meng.lightmap_color.b=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.got_it=0;
				}
			}

			sprintf(str,"Res. Width %d",st.game_lightmaps[i].T_w);

			if(CheckColisionMouse(8192,4096+(341),341,341,0))
			{
				DrawStringUI(str,8192,4096+(341),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					StartText();
					sprintf(st.TextInput,"%d",st.game_lightmaps[i].T_w);
					meng.got_it=4;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI(str,8192,4096+(341),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

			if(meng.got_it==4)
			{
				DrawStringUI(str,8192,4096+(341),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				st.game_lightmaps[i].T_w=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.got_it=0;
				}
			}

			sprintf(str,"Res. Height %d",st.game_lightmaps[i].T_h);

			if(CheckColisionMouse(8192,4096+(341*2),341,341,0))
			{
				DrawStringUI(str,8192,4096+(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					StartText();
					sprintf(st.TextInput,"%d",st.game_lightmaps[i].T_h);
					meng.got_it=5;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI(str,8192,4096+(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

			if(meng.got_it==5)
			{
				DrawStringUI(str,8192,4096+(341*2),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

				st.game_lightmaps[i].T_h=atoi(st.TextInput);

				if(st.keys[RETURN_KEY].state)
				{
					StopText();
					st.keys[RETURN_KEY].state=0;
					meng.got_it=0;
				}
			}

			if(CheckColisionMouse(8192,4096+(341*3),455,455,0))
			{
				DrawStringUI("Done",8192,4096+(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

				if(st.mouse1)
				{
					meng.command=ADD_LIGHT_TO_LIGHTMAP;

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
				DrawStringUI("Done",8192,4096+(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);
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

			if(CheckColisionMouseWorld(p3.x,p3.y,455,455,0) && st.mouse1)
			{
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

			if(st.keys[LSHIFT_KEY].state)
			{
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
				if(CheckColisionMouseWorld(p.x,p.y,455,455,0))
				{
					DrawString2("Done",p.x,p.y,0,0,0,255,128,32,255,ARIAL,1536,1536,0);

					if(st.mouse1)
					{
						meng.com_id=0;
						meng.got_it=-1;
						st.game_lightmaps[i].stat=1;
						st.num_lights++;
						meng.command=meng.pannel_choice=meng.command2=ADD_LIGHT;

						if(st.game_lightmaps[i].alpha)
							AddLightToAlphaTexture(&st.game_lightmaps[i].tex,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						else
							AddLightToTexture(&st.game_lightmaps[i].tex,st.game_lightmaps[i].data,st.game_lightmaps[i].T_w,st.game_lightmaps[i].T_h);
						free(meng.tmplightdata);
						st.mouse1=0;
					}
				}
				else
					DrawString2("Done",p.x,p.y,0,0,0,255,255,255,255,ARIAL,1536,1536,0);

				p=st.game_lightmaps[i].w_pos;

				p.y-=(st.game_lightmaps[i].W_h/2)+810;

				if(CheckColisionMouseWorld(p.x,p.y,455,455,0))
				{
					DrawString2("Edit light",p.x,p.y,0,0,0,255,128,32,255,ARIAL,1536,1536,0);

					if(st.mouse1)
					{
						meng.com_id=1;
						meng.got_it=0;
						st.mouse1=0;
					}
				}
				else
					DrawString2("Edit light",p.x,p.y,0,0,0,255,255,255,255,ARIAL,1536,1536,0);

				p=st.game_lightmaps[i].w_pos;

				p.y+=(st.game_lightmaps[i].W_h/2)+810;

				if(CheckColisionMouseWorld(p.x,p.y,455,455,0))
				{
					DrawString2("Add Light",p.x,p.y,0,0,0,255,128,32,255,ARIAL,1536,1536,0);

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
						meng.light.intensity=255.0f;
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
					DrawString2("Add Light",p.x,p.y,0,0,0,255,255,255,255,ARIAL,1536,1536,0);
			}

			if(meng.com_id==1)
			{
				//DrawString2("Edit light",p.x,p.y,0,0,0,255,32,32,255,ARIAL,1536,1536,0);

				DrawUI(8192,4300,3072,3900,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,7);

				sprintf(str,"R %d",meng.light.color.r);

				if(CheckColisionMouse(8192,4096-(341*3),341,341,0))
				{
					DrawStringUI(str,8192,4096-(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						StartText();
						sprintf(st.TextInput,"%d",meng.light.color.r);
						meng.got_it=1;
						st.mouse1=0;
					}
				}
				else
					DrawStringUI(str,8192,4096-(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==1)
				{
					DrawStringUI(str,8192,4096-(341*3),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					meng.light.color.r=atoi(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.got_it=0;
					}
				}

				sprintf(str,"G %d",meng.light.color.g);

				if(CheckColisionMouse(8192,4096-(341*2),341,341,0))
				{
					DrawStringUI(str,8192,4096-(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						StartText();
						sprintf(st.TextInput,"%d",meng.light.color.g);
						meng.got_it=2;
						st.mouse1=0;
					}
				}
				else
					DrawStringUI(str,8192,4096-(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==2)
				{
					DrawStringUI(str,8192,4096-(341*2),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					meng.light.color.g=atoi(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.got_it=0;
					}
				}

				sprintf(str,"B %d",meng.light.color.b);

				if(CheckColisionMouse(8192,4096-341,341,341,0))
				{
					DrawStringUI(str,8192,4096-341,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						StartText();
						sprintf(st.TextInput,"%d",meng.light.color.b);
						meng.got_it=3;
						st.mouse1=0;
					}
				}
				else
					DrawStringUI(str,8192,4096-341,0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==3)
				{
					DrawStringUI(str,8192,4096-341,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					meng.light.color.b=atoi(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.got_it=0;
					}
				}

				if(meng.got_it!=4)
					sprintf(str,"Intensity %.3f",meng.light.intensity);
				else
					strcpy(str,st.TextInput);

				if(CheckColisionMouse(8192,4096,341,341,0))
				{
					DrawStringUI(str,8192,4096,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						StartText();
						sprintf(st.TextInput,"%.3f",meng.light.intensity);
						meng.got_it=4;
						st.mouse1=0;
					}
				}
				else
					DrawStringUI(str,8192,4096,0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==4)
				{
					DrawStringUI(str,8192,4096,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					meng.light.intensity=atof(st.TextInput);

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

				if(CheckColisionMouse(8192,4096+341,341,341,0))
				{
					DrawStringUI(str,8192,4096+341,0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						StartText();
						sprintf(st.TextInput,"%.3f",meng.light.falloff);
						meng.got_it=5;
						st.mouse1=0;
					}
				}
				else
					DrawStringUI(str,8192,4096+341,0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==5)
				{
					DrawStringUI(str,8192,4096+341,0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					meng.light.falloff=atof(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.got_it=0;
					}
				}

				sprintf(str,"Z %d",st.game_lightmaps[i].t_pos[meng.light.light_id].z);

				if(CheckColisionMouse(8192,4096+(341*2),341,341,0))
				{
					DrawStringUI(str,8192,4096+(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						StartText();
						sprintf(st.TextInput,"%d",st.game_lightmaps[i].t_pos[meng.light.light_id].z);
						meng.got_it=6;
						st.mouse1=0;
					}
				}
				else
					DrawStringUI(str,8192,4096+(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==6)
				{
					DrawStringUI(str,8192,4096+(341*2),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

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

				if(CheckColisionMouse(8192,4096+(341*3),341,341,0))
				{
					DrawStringUI(str,8192,4096+(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						meng.got_it=7;
						st.mouse1=0;
					}
				}
				else
					DrawStringUI(str,8192,4096+(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==7)
				{
					DrawUI(11264,4096,3072,3072,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,6);

					if(CheckColisionMouse(11264,4096-(341*3),341,341,0))
					{
						DrawStringUI("Point medium",11264,4096-(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.light.type=POINT_LIGHT_MEDIUM;
							meng.got_it=-1;
							st.mouse1=0;
						}
					}
					else
						DrawStringUI("Point medium",11264,4096-(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(CheckColisionMouse(11264,4096-(341*2),341,341,0))
					{
						DrawStringUI("Point strong",11264,4096-(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.light.type=POINT_LIGHT_STRONG;
							meng.got_it=-1;
							st.mouse1=0;
						}
					}
					else
						DrawStringUI("Point strong",11264,4096-(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(CheckColisionMouse(11264,4096-(341),341,341,0))
					{
						DrawStringUI("Point normal",11264,4096-(341),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.light.type=POINT_LIGHT_NORMAL;
							meng.got_it=-1;
							st.mouse1=0;
						}
					}
					else
						DrawStringUI("Point normal",11264,4096-(341),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(CheckColisionMouse(11264,4096+(341),341,341,0))
					{
						DrawStringUI("Spotlight medium",11264,4096+(341),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.light.type=SPOTLIGHT_MEDIUM;
							meng.got_it=-1;
							st.mouse1=0;
						}
					}
					else
						DrawStringUI("Spotlight medium",11264,4096+(341),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(CheckColisionMouse(11264,4096+(341*2),341,341,0))
					{
						DrawStringUI("Spotlight strong",11264,4096+(341*2),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.light.type=SPOTLIGHT_STRONG;
							meng.got_it=-1;
							st.mouse1=0;
						}
					}
					else
						DrawStringUI("Spotlight STRONG",11264,4096+(341*2),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(CheckColisionMouse(11264,4096+(341*3),341,341,0))
					{
						DrawStringUI("Spotlight normal",11264,4096+(341*3),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

						if(st.mouse1)
						{
							meng.light.type=SPOTLIGHT_NORMAL;
							meng.got_it=-1;
							st.mouse1=0;
						}
					}
					else
						DrawStringUI("Spotlight normal",11264,4096+(341*3),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

					if(st.keys[RETURN_KEY].state)
					{
						meng.got_it=-1;
						st.keys[RETURN_KEY].state=0;
					}
				}

				sprintf(str,"Spot angle %d",st.game_lightmaps[i].spot_ang[meng.light.light_id]);

				if(CheckColisionMouse(8192,4096+(341*4),341,341,0))
				{
					DrawStringUI(str,8192,4096+(341*4),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

					if(st.mouse1)
					{
						StartText();
						sprintf(st.TextInput,"%d",st.game_lightmaps[i].spot_ang[meng.light.light_id]);
						meng.got_it=8;
						st.mouse1=0;
					}
				}
				else
					DrawStringUI(str,8192,4096+(341*4),0,0,0,255,255,255,255,ARIAL,2048,2048,0);

				if(meng.got_it==8)
				{
					DrawStringUI(str,8192,4096+(341*4),0,0,0,255,32,32,255,ARIAL,2048,2048,0);

					st.game_lightmaps[i].spot_ang[meng.light.light_id]=atoi(st.TextInput);

					if(st.keys[RETURN_KEY].state)
					{
						StopText();
						st.keys[RETURN_KEY].state=0;
						meng.got_it=0;
					}
				}


				if(CheckColisionMouse(8192,4096+(341*5),455,455,0))
				{
					DrawStringUI("Done",8192,4096+(341*5),0,0,0,255,128,32,255,ARIAL,2048,2048,0);

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
					DrawStringUI("Done",8192,4096+(341*5),0,0,0,255,255,255,255,ARIAL,2048,2048,0);
			}
		}
	}

	if(meng.command!=TEX_SIZE_OBJ && meng.command!=TEX_PAN_OBJ && meng.command!=OBJ_AMBL && meng.command!=RGB_OBJ && meng.command!=OBJ_EDIT_BOX)
	{
		if(st.keys[W_KEY].state)
		{
			st.Camera.position.y-=64;
		}

		if(st.keys[S_KEY].state)
		{
			st.Camera.position.y+=64;
		}

		if(st.keys[D_KEY].state)
		{
			st.Camera.position.x+=64;
		}

		if(st.keys[A_KEY].state)
		{
			st.Camera.position.x-=64;
		}

		if(meng.command!=ADD_LIGHT_TO_LIGHTMAP)
		{
			if(st.mouse_wheel>0 && !st.mouse2)
			{
				if(st.Camera.dimension.x<2) st.Camera.dimension.x+=0.1;
				if(st.Camera.dimension.y<2) st.Camera.dimension.y+=0.1;
				st.mouse_wheel=0;
			}

			if(st.mouse_wheel<0 && !st.mouse2)
			{
				if(st.Camera.dimension.x>0.4) st.Camera.dimension.x-=0.1;
				if(st.Camera.dimension.y>0.4) st.Camera.dimension.y-=0.1;
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
		DrawStringUI("Loading...",400,300,200,50,0,255,255,255,255,ARIAL,0,0,0);
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
	int16 i=0;

	TEX_DATA texture;

	int32 dist2;

	float dist;

	Pos p;
	uPos16 p2;

	if(meng.pannel_choice==ADD_LIGHT && meng.command==CREATE_LIGHTMAP_STEP2)
	{
		i=meng.command2;

		DrawGraphic(st.game_lightmaps[i].w_pos.x,st.game_lightmaps[i].w_pos.y,st.game_lightmaps[i].W_w,st.game_lightmaps[i].W_h,0,255,128,32,mgg_sys[0].frames[5],128,0,0,32768,32768,17);

		DrawLine(st.game_lightmaps[i].w_pos.x-((st.game_lightmaps[i].W_w/2)-((st.game_lightmaps[i].W_w/12))),st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2),
			st.game_lightmaps[i].w_pos.x+((st.game_lightmaps[i].W_w/2)-((st.game_lightmaps[i].W_w/12))),st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2),
			255,128,32,255,st.game_lightmaps[i].W_h/12,16);

		DrawLine(st.game_lightmaps[i].w_pos.x-((st.game_lightmaps[i].W_w/2)-((st.game_lightmaps[i].W_w/12))),st.game_lightmaps[i].w_pos.y+(st.game_lightmaps[i].W_h/2),
			st.game_lightmaps[i].w_pos.x+((st.game_lightmaps[i].W_w/2)-((st.game_lightmaps[i].W_w/12))),st.game_lightmaps[i].w_pos.y+(st.game_lightmaps[i].W_h/2),
			255,128,32,255,st.game_lightmaps[i].W_h/12,16);

		DrawLine(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y+((st.game_lightmaps[i].W_h/2)-(st.game_lightmaps[i].W_h/12)),
			st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y-((st.game_lightmaps[i].W_h/2)-(st.game_lightmaps[i].W_h/12)),
			255,128,32,255,st.game_lightmaps[i].W_w/12,16);

		DrawLine(st.game_lightmaps[i].w_pos.x+(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y+((st.game_lightmaps[i].W_h/2)-(st.game_lightmaps[i].W_h/12)),
			st.game_lightmaps[i].w_pos.x+(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y-((st.game_lightmaps[i].W_h/2)-(st.game_lightmaps[i].W_h/12)),
			255,128,32,255,st.game_lightmaps[i].W_w/12,16);

		DrawGraphic(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2),st.game_lightmaps[i].W_w/6,st.game_lightmaps[i].W_h/6,0,
			255,32,32,mgg_sys[0].frames[4],255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,16);

		DrawGraphic(st.game_lightmaps[i].w_pos.x+(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2),st.game_lightmaps[i].W_w/6,st.game_lightmaps[i].W_h/6,0,
			255,32,32,mgg_sys[0].frames[4],255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,16);

		DrawGraphic(st.game_lightmaps[i].w_pos.x+(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y+(st.game_lightmaps[i].W_h/2),st.game_lightmaps[i].W_w/6,st.game_lightmaps[i].W_h/6,0,
			255,32,32,mgg_sys[0].frames[4],255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,16);

		DrawGraphic(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2),st.game_lightmaps[i].w_pos.y+(st.game_lightmaps[i].W_h/2),st.game_lightmaps[i].W_w/6,st.game_lightmaps[i].W_h/6,0,
			255,32,32,mgg_sys[0].frames[4],255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,16);
	}
	else
	if(meng.pannel_choice==ADD_LIGHT && meng.command==CREATE_LIGHTMAP)
	{
		i=meng.obj_lightmap_sel;

		if(i>-1)
		{
			DrawLine(st.Current_Map.obj[i].position.x-(st.Current_Map.obj[i].size.x/2),st.Current_Map.obj[i].position.y-(st.Current_Map.obj[i].size.y/2),
				st.Current_Map.obj[i].position.x+(st.Current_Map.obj[i].size.x/2)+32,st.Current_Map.obj[i].position.y-(st.Current_Map.obj[i].size.y/2),
				255,255,255,255,64,17);

			DrawLine(st.Current_Map.obj[i].position.x-(st.Current_Map.obj[i].size.x/2)-32,st.Current_Map.obj[i].position.y+(st.Current_Map.obj[i].size.y/2),
				st.Current_Map.obj[i].position.x+(st.Current_Map.obj[i].size.x/2)+32,st.Current_Map.obj[i].position.y+(st.Current_Map.obj[i].size.y/2),
				255,255,255,255,64,17);

			DrawLine(st.Current_Map.obj[i].position.x-(st.Current_Map.obj[i].size.x/2),st.Current_Map.obj[i].position.y+(st.Current_Map.obj[i].size.y/2),
				st.Current_Map.obj[i].position.x-(st.Current_Map.obj[i].size.x/2),st.Current_Map.obj[i].position.y-(st.Current_Map.obj[i].size.y/2)-32,
				255,255,255,255,64,17);

			DrawLine(st.Current_Map.obj[i].position.x+(st.Current_Map.obj[i].size.x/2),st.Current_Map.obj[i].position.y+(st.Current_Map.obj[i].size.y/2),
				st.Current_Map.obj[i].position.x+(st.Current_Map.obj[i].size.x/2),st.Current_Map.obj[i].position.y-(st.Current_Map.obj[i].size.y/2)-32,
				255,255,255,255,64,17);
		}
		else
		if(i==-2)
		{
			//DrawGraphic(meng.lightmappos.x,meng.lightmappos.y,meng.lightmapsize.x,meng.lightmapsize.y,0,255,255,255,mgg_sys[0].frames[4],255,0,0,32768,32768,16);

			DrawLine(meng.lightmappos.x-(meng.lightmapsize.x/2)-32,meng.lightmappos.y-(meng.lightmapsize.y/2),meng.lightmappos.x+(meng.lightmapsize.x/2)+32,meng.lightmappos.y-(meng.lightmapsize.y/2),255,255,255,255,64,17);
			DrawLine(meng.lightmappos.x-(meng.lightmapsize.x/2),meng.lightmappos.y-(meng.lightmapsize.y/2),meng.lightmappos.x-(meng.lightmapsize.x/2),meng.lightmappos.y+(meng.lightmapsize.y/2)+32,255,255,255,255,64,17);
			DrawLine(meng.lightmappos.x-(meng.lightmapsize.x/2),meng.lightmappos.y+(meng.lightmapsize.y/2),meng.lightmappos.x+(meng.lightmapsize.x/2)+32,meng.lightmappos.y+(meng.lightmapsize.y/2),255,255,255,255,64,17);
			DrawLine(meng.lightmappos.x+(meng.lightmapsize.x/2),meng.lightmappos.y-(meng.lightmapsize.y/2),meng.lightmappos.x+(meng.lightmapsize.x/2),meng.lightmappos.y+(meng.lightmapsize.y/2)+32,255,255,255,255,64,17);
		}
	}
	else
	if(meng.pannel_choice==ADD_LIGHT && meng.command==ADD_LIGHT_TO_LIGHTMAP)
	{
		i=meng.command2;

		texture.data=st.game_lightmaps[i].tex;
		texture.normal=0;
		texture.vb_id=-1;

		DrawGraphic(st.game_lightmaps[i].w_pos.x,st.game_lightmaps[i].w_pos.y,st.game_lightmaps[i].W_w,st.game_lightmaps[i].W_h,0,255,255,255,texture,255,0,0,32768,32768,17);

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

		dist=1.0f/(meng.light.falloff*0.001);

		//dist=(dist*st.game_lightmaps[i].W_w)/st.game_lightmaps[i].T_w;

		//dist=((st.game_lightmaps[i].w_pos.x*2)*dist)/st.game_lightmaps[i].W_w;
		
		p2=st.game_lightmaps[i].t_pos[meng.light.light_id];

		//dist2=(int32) p2.x;

		dist2=dist;

		dist2=(st.game_lightmaps[i].W_w*dist2)/st.game_lightmaps[i].T_w;
		p2.x=(st.game_lightmaps[i].W_w*st.game_lightmaps[i].t_pos[meng.light.light_id].x)/st.game_lightmaps[i].T_w;
		p2.y=(st.game_lightmaps[i].W_h*st.game_lightmaps[i].t_pos[meng.light.light_id].y)/st.game_lightmaps[i].T_h;

		dist2=(32768*dist2)/st.game_lightmaps[i].W_w;
		p2.x=p2.x+(st.game_lightmaps[i].w_pos.x-(st.game_lightmaps[i].W_w/2));
		p2.y=p2.y+(st.game_lightmaps[i].w_pos.y-(st.game_lightmaps[i].W_h/2));

		//DrawLine(p2.x-dist2,p2.y,p2.x+dist2,p2.y,255,255,255,255,128,16);

		DrawGraphic(p2.x,p2.y,256,256,0,255,255,255,mgg_sys[0].frames[4],255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,17);

		//DrawLine(p2.x,p2.y,p2.x+dist2,p2.y,255,255,255,255,128,16);
	}
	
}

int main(int argc, char *argv[])
{
	int8 i=0, test=0, ch, ch2, ch3;
	char options[8][16]={"Test 1", "Option 2", "vagina 3", "mGear 4"};

	uint8 t1;

	uint32 t3;

	float t2;

	if(LoadCFG()==0)
		if(MessageBox(NULL,L"Error while trying to read or write the configuration file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();

	strcpy(st.WINDOW_NAME,"Engineer Map Editor ALPHA");

	Init();

	OpenFont("font//arial.ttf","arial",0,128);
	OpenFont("font//arialbd.ttf","arial bould",1,128);
	OpenFont("font//tt0524m_.ttf","geometry",2,128);

	InitMGG();

	if(LoadMGG(&mgg_sys[0],"data//mEngUI.mgg")==NULL)
	{
		LogApp("Could not open UI mgg");
		Quit();
	}

	UILoadSystem("UI_Sys.cfg");

	meng.num_mgg=0;
	memset(st.Game_Sprites,0,MAX_SPRITES*sizeof(_SPRITES));
	st.num_sprites=0;
	meng.got_it=-1;
	meng.sprite_selection=-1;
	meng.sprite_frame_selection=-1;
	meng.spr.size.x=2048;
	meng.spr.size.y=2048;

	//SpriteListLoad();

	BASICBKD();
	DrawStringUI("Loading sprites...",8192,4096,0,0,0,255,255,255,255,ARIAL,2048,2048,6);

	Renderer(1);

	LoadSpriteList("sprite.list");

	st.FPSYes=1;

	st.gt=MAIN_MENU;

	meng.pannel_choice=2;
	meng.command=2;

	st.Developer_Mode=1;

	meng.menu_sel=0;

	meng.path=(char*) malloc(2);
	strcpy(meng.path,".");

	//st.gt=STARTUP;

	memset(&meng.spr.body,0,sizeof(Body));

	while(!st.quit)
	{
		if(st.FPSYes)
			FPSCounter();

		BASICBKD();

		InputProcess();

		/*
		if(st.gt==STARTUP)
		{
			if(test==0)
			{
				if(UIMessageBox(8192,4096,CENTER,"This is a test message box\nmade using the new UI system\n for mGear\n Click OK to proceed.",1,ARIAL,2048,UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_NORMAL)==UI_OK)
				{
					test=1;
					ch2=UICreateWindow2(8192,4096,CENTER,7,3,2048,12,ARIAL);
					ch=128;
					ch3=0;
					t3=32768;
					t2=128.456f;
				}
			}
			else
			{
				UIWin2_NumberBoxf(ch2,0,&t2,"Test",UI_COL_NORMAL,UI_COL_SELECTED,UI_COL_CLICKED);
				UIWin2_NumberBoxi8(ch2,1,&ch,"8 bit",UI_COL_NORMAL,0xFF8020,0XFF2020);
				UIWin2_NumberBoxui32(ch2,2,&t3,"u32 bit",UI_COL_NORMAL,0xFF8020,0XFF2020);
			}
		}
		else
		*/
		if(st.gt==INGAME)
		{
			if(st.keys[SPACE_KEY].state)
			{
				PlayMovie("LOGOHD.MGV");
				st.keys[SPACE_KEY].state=0;
			}

			if(meng.command==TEX_SEL)
			{
				ImageList(meng.mgg_sel);
				if(st.keys[ESC_KEY].state)
				{
					meng.command=meng.pannel_choice;
					st.keys[ESC_KEY].state=0;
				}
			}
			else
			if(meng.command==SPRITE_SELECTION)
			{
				SpriteList();
				if(st.keys[ESC_KEY].state)
				{
					meng.command=meng.pannel_choice;
					st.keys[ESC_KEY].state=0;
				}
			}
			else
			if(meng.command==MGG_SEL)
			{
				PannelLeft();
				MGGList();
				if(st.keys[ESC_KEY].state)
				{
					meng.command=meng.pannel_choice;
					st.keys[ESC_KEY].state=0;
				}
			}
			else
			if(meng.command==MGG_LOAD)
			{
				if(MGGLoad()==NULL) meng.command=meng.pannel_choice;
			}
			else
			{
				ViewPortCommands();
				
				PannelLeft();
				DrawMap();

				ENGDrawLight();

				if(st.keys[ESC_KEY].state && meng.command!=OBJ_EDIT_BOX)
				{
					st.gt=GAME_MENU;
					st.keys[ESC_KEY].state=0;
				}
			}
		}
		else
			if(st.gt==MAIN_MENU || st.gt==GAME_MENU)
			Menu();

		UIMain_DrawSystem();
		MainSound();
		Renderer(0);
		//Timer();
	}

	StopAllSounds();
	Quit();
	return 1;
}