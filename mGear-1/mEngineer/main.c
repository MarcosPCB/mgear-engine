#include "input.h"
#include "main.h"
#include <stdio.h> 
#include <stdlib.h>
#include "dirent.h"

extern _MGG mgg[MAX_MGG];

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
	DrawUI(8192,4096,2275,8192,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg[0].frames[4],255,5);
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
					DrawStringUI(meng.mgg_list[j],8192,i+meng.scroll2,0,0,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);
				}
				else
				{
					DrawStringUI(meng.mgg_list[j],8192,i+meng.scroll2,0,0,0,255,32,0,255,st.fonts[ARIAL].font,2048,2048,0);
					if(st.mouse1)
					{
						meng.mgg_sel=j+MGG_MAP_START;
						meng.command=meng.pannel_choice;
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

	if(meng.scroll<0)
			m=(meng.scroll/1546)*(-10);

	for(i=728;i<(8192/1546)*1546;i+=1546)
	{
		if(m>=mgg[id].num_frames) break;
		for(j=728;j<(16384/1546)*1546;j+=1546)
		{
			if(m<mgg[id].num_frames)
			{
				if((CheckColisionMouse(j,i+meng.scroll,1638,1456,0) && st.mouse1) || (meng.tex_selection.data==mgg[id].frames[m].data && meng.tex_selection.posx==mgg[id].frames[m].posx && meng.tex_selection.posy==mgg[id].frames[m].posy))
				{
					DrawUI(j,i+meng.scroll,1638,1456,0,255,128,32,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg[id].frames[m],255,0);
					meng.tex_selection=mgg[id].frames[m];
					meng.tex_ID=m;
					meng.tex_MGGID=id;
				}
				else
				{
					DrawUI(j,i+meng.scroll,1638,1456,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg[id].frames[m],255,0);
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

static int16 MGGLoad()
{
	uint16 j=0, i, u;
	int16 num_files;
	FILE *f;
	DIR *dir;

	int16 id, loaded=0;
	uint16 id2=st.Current_Map.num_mgg, id3=0;

	char files[512][512];
	char *path2;

	size_t size;

	for(i=MGG_MAP_START+st.num_mgg;i<MGG_MAP_START+MAX_MAPMGG;i++)
	{
		if(i==MGG_MAP_START+MAX_MAPMGG && mgg[i].type!=NONE)
		{
			LogApp("Cannot load MGG, reached max number of map MGGs loaded");
			return 0;
		}

		if(mgg[i].type==NONE)
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
			DrawString2UI(files[j],8192,i+meng.scroll,0,0,0,255,128,32,255,st.fonts[GEOMET].font,FONT_SIZE*2,FONT_SIZE*2,0);

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
						DrawUI(8192,4096,16384,8192,0,0,0,0,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg[0].frames[4],255,0);
						DrawString2UI("Loading...",8192,4096,1,1,0,255,255,255,255,st.fonts[GEOMET].font,FONT_SIZE*3,FONT_SIZE*3,0);
						Renderer();
						LoadMGG(&mgg[id],path2);

						for(u=0;u<st.Current_Map.num_mgg;u++)
						{
							if(strcmp(meng.mgg_list[u],mgg[id].name)==NULL)
							{
								loaded=1;
								break;
							}

						}

						if(loaded==1)
						{
							FreeMGG(&mgg[id]);
							st.mouse1=0;
							j++;
							free(path2);
							continue;
						}
						else if(loaded==0);
						{
							strcpy(st.Current_Map.MGG_FILES[st.Current_Map.num_mgg],path2);
							strcpy(meng.mgg_list[st.Current_Map.num_mgg],mgg[id].name);
							meng.num_mgg++;
							st.Current_Map.num_mgg++;
							st.num_mgg++;
							LogApp("MGG %s loaded",path2);
							meng.scroll=0;
							meng.pannel_choice=2;
							meng.command=2;
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
			DrawString2UI(files[j],8192,i+meng.scroll,0,0,0,255,255,255,255,st.fonts[GEOMET].font,FONT_SIZE*2,FONT_SIZE*2,0);
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
	uint16 i=0, j=0;
	Pos p;
	PosF p2;

	DrawUI(455,4096,455*2,8192,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg[0].frames[4],255,7);

	if(!CheckColisionMouse(227,227,455,455,0))
	{
		DrawUI(227,227,455,455,0,255,255,255,0,0,32768,32768,mgg[0].frames[0],255,7);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawUI(227,227,445,445,0,255,128,32,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg[0].frames[0],255,7);
		//time++;
		//if((st.time-time)==1000)
		//{
		DrawStringUI("Draw a sector",8192,8192-455,1820,455,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
		//}

			if(st.mouse1)
			{
				meng.command=meng.pannel_choice=meng.command2=0;
				st.mouse1=0;
			}
	}

	if(!CheckColisionMouse(682,227,448,448,0))
	{
		DrawUI(682,227,448,448,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg[0].frames[2],255,7);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawUI(682,227,448,448,0,255,128,32,0,0,32768,32768,mgg[0].frames[2],255,7);
		//if((st.time-time)==1000)
		//{
		DrawStringUI("Select and edit",8192,8192-455,1820,455,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
		//}

		if(st.mouse1) meng.command=meng.pannel_choice=meng.command2=2;
	}
	
	if(!CheckColisionMouse(227,682,448,448,0))
	{
		DrawUI(227,682,448,448,0,255,255,255,0,0,32768,32768,mgg[0].frames[1],255,7);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawUI(227,682,448,448,0,255,128,32,0,0,32768,32768,mgg[0].frames[1],255,7);
		//if((st.time-time)==1000)
		//{
		DrawStringUI("Add an OBJ",8192,8192-455,1820,455,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
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
		DrawUI(682,682,448,448,0,255,255,255,0,0,32768,32768,mgg[0].frames[3],255,7);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawUI(682,682,448,448,0,255,128,32,0,0,32768,32768,mgg[0].frames[3],255,7);
		//if((st.time-time)==1000)
		//{
		DrawStringUI("Add a sprite",8192,8192-455,1820,455,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
		//}

			if(st.mouse1)
			{
				if(meng.spr.gid!=-1)
				{
					/*
					meng.tex2_sel.data=meng.tex_selection.data;
					meng.tex2_ID=meng.tex_ID;
					meng.tex2_MGGID=meng.tex_MGGID;

					meng.tex_selection=mgg[st.Game_Sprites[meng.spr.gid].MGG_ID].frames[st.Game_Sprites[meng.spr.gid].frame];
					meng.tex_ID=st.Game_Sprites[meng.spr.gid].frame;
					meng.tex_MGGID=st.Game_Sprites[meng.spr.gid].MGG_ID;
					*/
				}

				meng.command=meng.pannel_choice=4;
			}
	}
	
	if(!CheckColisionMouse(458,1137,490,223,0))
	{
		DrawStringUI("Tex. Sel.",458,1137,490,223,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
	}
	else
	{
		DrawStringUI("Tex. Sel.",458,1137,490,223,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

		DrawStringUI("Texture Selection",8192,8192-455,2730,455,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

		if(st.mouse1)
		{
			meng.scroll=0;
			meng.command=5;
			meng.command2=0;
			st.mouse1=0;
		}
	}

	if(!CheckColisionMouse(458,1400,490,223,0))
	{
		DrawStringUI("MGG Sel.",458,1400,490,223,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
	}
	else
	{
		DrawStringUI("MGG Sel.",458,1400,490,223,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

		DrawStringUI("MGG Selection",8192,8192-455,2270,455,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

		if(st.mouse1)
		{
			meng.scroll2=0;
			meng.command=6;
			meng.command2=0;
		}
	}

	if(!CheckColisionMouse(458,(8192)-227,490,223,0))
	{
		DrawStringUI("Load MGG",458,8192-227,490,223,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
	}
	else
	{
		DrawStringUI("Load MGG",458,(8192)-227,490,223,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

		DrawStringUI("Load an MGG file and adds it to the map list",8192,(8192)-455,5460,455,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

		if(st.mouse1)
		{
			meng.scroll2=0;
			meng.command=7;
			meng.command2=0;
		}
	}

	DrawUI(455,8192-910,910,910,0,255,255,255,0,0,32768,32768,meng.tex_selection,255,0);

	if(meng.pannel_choice==0)
		DrawUI(227,227,455,455,0,128,32,32,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg[0].frames[0],255,0);
	//else
	if(meng.pannel_choice==2)
		DrawUI(682,227,455,455,0,128,32,32,0,0,32768,32768,mgg[0].frames[2],255,0);
	//else
	if(meng.pannel_choice==4)
	{
		DrawUI(682,682,455,455,0,128,32,32,0,0,32768,32768,mgg[0].frames[3],255,0);

		if(meng.obj.type==MIDGROUND)
		{
			if(CheckColisionMouse(465,1900,880,220,0))
			{
				DrawStringUI("Midground",465,2010,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
				if(st.mouse1)
				{
					meng.command=16;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Midground",465,2010,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		}

		if(meng.obj.type==FOREGROUND)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Foreground",465,2010,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
				if(st.mouse1)
				{
					meng.command=16;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Foreground",465,2010,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		}

		if(meng.obj.type==BACKGROUND1)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background1",465,2010,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
				if(st.mouse1)
				{
					meng.command=16;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background1",465,2010,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		}

		if(meng.obj.type==BACKGROUND2)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background2",465,2010,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
				if(st.mouse1)
				{
					meng.command=16;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background2",465,2010,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		}

		if(meng.obj.type==BACKGROUND3)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background3",465,2010,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
				if(st.mouse1)
				{
					meng.command=16;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background3",465,2010,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		}

		sprintf(str,"alpha %d", meng.spr.color.a);

		if(CheckColisionMouse(465,2445,810,217,0))
		{
			DrawStringUI(str,465,2445,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1)
			{
				meng.command=RGB_SPRITE;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,2445,810,217,0, 255, 255, 255,255,st.fonts[ARIAL].font,0,0,0);

		if(meng.command==RGB_SPRITE)
		{
			DrawStringUI(str,465,2445,810,217,0,255,32,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.keys[RIGHT_KEY].state)
			{
				meng.spr.color.a+=1;
				st.keys[RIGHT_KEY].state=0;
			}

			if(st.keys[LEFT_KEY].state)
			{
				meng.spr.color.a-=1;
				st.keys[LEFT_KEY].state=0;
			}

			if(!CheckColisionMouse(465,2445,810,217,0) && st.mouse1)
			{
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}

		sprintf(str,"Actor %d",meng.spr.gid);

		if(CheckColisionMouse(465,2880,810,217,0))
		{
			DrawStringUI(str,465,2887,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1)
			{
				meng.command=ACTOR_SPRITE;
				meng.scroll=0;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,2887,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		
		if(meng.command==ACTOR_SPRITE)
		{
			DrawUI(8192,4096,100,8192,0,255,255,255,0,0,32768,32768,mgg[0].frames[4],255,0);

			i=0;
			for(j=227+meng.scroll;j<8192;j+=455)
			{
				if(i==st.num_sprites)
					break;
				else
				if(CheckColisionMouse(8192,j+meng.scroll,810,455,0))
				{
					DrawStringUI(st.Game_Sprites[i].name,8192,j+meng.scroll,810,455,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

					if(st.mouse1)
					{
						meng.spr.gid=i;
						/*
						if(st.Game_Sprites[i].frame>-1)
						{
							meng.tex2_sel=meng.tex_selection;
							meng.tex2_ID=meng.tex_ID;
							meng.tex2_MGGID=meng.tex_MGGID;
							meng.tex_selection=mgg[st.Game_Sprites[i].MGG_ID].frames[st.Game_Sprites[i].frame];
							meng.tex_ID=st.Game_Sprites[i].frame;
							meng.tex_MGGID=st.Game_Sprites[i].MGG_ID;
						}
						*/
						st.mouse1=0;
						meng.command=meng.pannel_choice;
					}
				}
				else
					DrawStringUI(st.Game_Sprites[i].name,8192,j+meng.scroll,810,455,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

				i++;
			}

			if(st.mouse_wheel>0)
			{
				meng.scroll+=455;
				st.mouse_wheel=0;
			}

			if(st.mouse_wheel<0)
			{
				meng.scroll-=455;
				st.mouse_wheel=0;
			}

			if(st.keys[ESC_KEY].state)
			{
				meng.command=meng.pannel_choice;
				meng.scroll=0;
				st.keys[ESC_KEY].state=0;
			}
		}

		sprintf(str,"Tag %d",meng.spr.tag);

		if(CheckColisionMouse(455,3135,810,217,0))
		{
			DrawStringUI(str,465,3135,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1 && !StartText())
			{
				meng.command=SPRITE_TAG;
				sprintf(st.TextInput,"%d",meng.spr.tag);
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,3135,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		
		if(meng.command==SPRITE_TAG)
		{
			DrawStringUI(str,465,3135,810,217,0,255,32,32,255,st.fonts[ARIAL].font,0,0,0);
			meng.spr.tag=atoi(st.TextInput);

			if(st.keys[ESC_KEY].state)
			{
				StopText();
				meng.command=meng.pannel_choice;
				st.keys[ESC_KEY].state=0;
			}
		}

		sprintf(str,"Health %d",meng.spr.health);

		if(CheckColisionMouse(465,3435,810,217,0))
		{
			DrawStringUI(str,465,3435,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1 && !StartText())
			{
				meng.command=SPRITE_HEALTH;
				sprintf(st.TextInput,"%d",meng.spr.health);
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,3435,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(meng.command==SPRITE_HEALTH)
		{
			DrawStringUI(str,465,3435,810,217,0,255,32,32,255,st.fonts[ARIAL].font,0,0,0);
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
			DrawStringUI(str,465,3690,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1)
			{
				meng.command=SPRITE_PHY;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,3690,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(meng.command==SPRITE_PHY)
		{
			DrawUI(8192, 4096, 2120,1820,0,255,255,255,0,0,32768,32768,mgg[0].frames[4],255,0);

			sprintf(str,"Physics? %d",meng.spr.body.physics_on);

			if(CheckColisionMouse(8192, 4096-710,810,200,0))
			{
				DrawString2UI(str,8192, 4096-710,0,0,0,255,128,32,255,st.fonts[ARIAL].font,512*4,512*4,0);

				if(st.mouse1)
				{
					if(meng.spr.body.physics_on==0) meng.spr.body.physics_on=1;
					else meng.spr.body.physics_on=0;

					st.mouse1=0;
				}
			}
			else
				DrawString2UI(str,8192, 4096-710,0,0,0,255,255,255,255,st.fonts[ARIAL].font,512*4,512*4,0);

			sprintf(str,"Mass %.2f",meng.spr.body.mass);

			if(CheckColisionMouse(8192, 4096-1420,810,1420,0))
			{
				DrawString2UI(str,8192, 4096-1420,0,0,0,255,128,32,255,st.fonts[ARIAL].font,512*4,512*4,0);

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
				DrawString2UI(str,8192, 4096-1420,0,0,0,255,255,255,255,st.fonts[ARIAL].font,512*4,512*4,0);

			if(meng.sub_com==10)
			{
				DrawString2UI(str,8192, 4096-1420,0,0,0,255,32,32,255,st.fonts[ARIAL].font,512*4,512*4,0);
				meng.spr.body.mass=atof(st.TextInput);
				if(st.keys[ESC_KEY].state)
				{
					StopText();
					meng.sub_com=0;
					st.keys[ESC_KEY].state=0;
				}
			}

			sprintf(str,"Flammable? %d",meng.spr.body.flamable);

			if(CheckColisionMouse(8192, 4096,810,1420,0))
			{
				DrawString2UI(str,8192, 4096,0,0,0,255,128,32,255,st.fonts[ARIAL].font,512*4,512*4,0);

				if(st.mouse1)
				{
					if(meng.spr.body.flamable==0) meng.spr.body.flamable=1;
					else meng.spr.body.flamable=0;

					st.mouse1=0;
				}
			}
			else
				DrawString2UI(str,8192, 4096,0,0,0,255,255,255,255,st.fonts[ARIAL].font,512*4,512*4,0);

			sprintf(str,"Explosive? %d",meng.spr.body.explosive);

			if(CheckColisionMouse(8192, 4096+1420,90,40,0))
			{
				DrawString2UI(str,8192, 4096+1420,0,0,0,255,128,32,255,st.fonts[ARIAL].font,512*4,512*4,0);

				if(st.mouse1)
				{
					if(meng.spr.body.explosive==0) meng.spr.body.explosive=1;
					else meng.spr.body.explosive=0;

					st.mouse1=0;
				}
			}
			else
				DrawString2UI(str,8192, 4096+1420,0,0,0,255,255,255,255,st.fonts[ARIAL].font,512*4,512*4,0);
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

		p.x-=st.Camera.position.x;
		p.y-=st.Camera.position.y;

		//p.x*=st.Camera.dimension.x;
		//p.y*=st.Camera.dimension.y;

		p2.x=1024*st.Camera.dimension.x;
		p2.y=1024*st.Camera.dimension.y;

		sprintf(str,"%d",st.Current_Map.obj[i].size.y);
		DrawString(str,p.x,p.y-227,810,217,0,255,255,255,255,st.fonts[ARIAL].font,p2.x,p2.y,0);

		p.x=st.Current_Map.obj[i].position.x-(st.Current_Map.obj[i].size.x/2);
		p.y=st.Current_Map.obj[i].position.y;

		p.x-=st.Camera.position.x;
		p.y-=st.Camera.position.y;

		//p.x*=st.Camera.dimension.x;
		//p.y*=st.Camera.dimension.y;

		sprintf(str,"%d",st.Current_Map.obj[i].size.x);
		DrawString(str,p.x-455,p.y,810,217,0,255,255,255,255,st.fonts[ARIAL].font,p2.x,p2.y,0);

		if(st.keys[RETURN_KEY].state && meng.command!=TEX_SIZE_OBJ && meng.command!=TEX_PAN_OBJ && meng.command!=OBJ_EDIT_BOX && meng.command!=RGB_OBJ)
		{
			meng.command=OBJ_EDIT_BOX;
			st.keys[RETURN_KEY].state=0;
		}

		if(meng.command==OBJ_EDIT_BOX)
		{
			DrawUI(8192,4096,1820,2730,0,255,255,255,0,0,32768,32768,mgg[0].frames[4],255,0);

			sprintf(str,"X %d",st.Current_Map.obj[i].position.x);

			if(CheckColisionMouse(8192,(4096)-681,1720,455,0) && !meng.sub_com)
			{
				DrawStringUI(str,8192,(4096)-681,1720,455,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)-681,1720,455,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)-681,1720,455,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);

			sprintf(str,"Y %d",st.Current_Map.obj[i].position.y);

			if(CheckColisionMouse(8192,(4096)-227,1720,455,0) && !meng.sub_com)
			{
				DrawStringUI(str,8192,(4096)-227,1720,455,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)-227,1720,455,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)-227,1720,455,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);

			sprintf(str,"SX %d",st.Current_Map.obj[i].size.x);

			if(CheckColisionMouse(8192,(4096)+227,1720,455,0) && !meng.sub_com)
			{
				DrawStringUI(str,8192,(4096)+227,1720,455,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)+227,1720,455,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)+227,1720,455,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);

			sprintf(str,"SY %d",st.Current_Map.obj[i].size.y);

			if(CheckColisionMouse(8192,(4096)+681,1720,455,0) && !meng.sub_com)
			{
				DrawStringUI(str,8192,(4096)+681,1720,455,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)+681,1720,455,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)+681,1720,455,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);

			sprintf(str,"Z %d",st.Current_Map.obj[i].position.z);

			if(CheckColisionMouse(8192,(4096)+1137,1720,455,0) && !meng.sub_com)
			{
				DrawStringUI(str,8192,(4096)+1137,1720,455,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)+1137,1720,455,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);
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
				DrawStringUI(str,8192,(4096)+1137,1720,455,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);

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
				DrawStringUI("Midground",465,2010,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
				if(st.mouse1)
				{
					meng.command=14;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Midground",465,2010,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		}

		if(meng.obj2.type==FOREGROUND)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Foreground",465,2010,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
				if(st.mouse1)
				{
					meng.command=14;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Foreground",465,2010,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		}

		if(meng.obj2.type==BACKGROUND1)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background1",465,2010,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
				if(st.mouse1)
				{
					meng.command=14;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background1",465,2010,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		}

		if(meng.obj2.type==BACKGROUND2)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background2",465,2010,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
				if(st.mouse1)
				{
					meng.command=14;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background2",465,2010,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		}

		if(meng.obj2.type==BACKGROUND3)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background3",465,2010,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
				if(st.mouse1)
				{
					meng.command=14;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background3",465,2010,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		}

		sprintf(str,"color");

		if(CheckColisionMouse(465,2445,810,217,0))
		{
			DrawStringUI(str,465,2445,810,217,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

			if(st.mouse1)
			{
				meng.command=RGB_OBJ;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,2445,810,217,0, 255, 255, 255,255,st.fonts[ARIAL].font,2048,2048,0);

		if(meng.command==RGB_OBJ)
		{
			DrawUI(8192,3072,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg[0].frames[4],255,6);

			DrawStringUI(str,465,2445,810,217,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

			sprintf(str,"R %d",meng.obj2.color.r);

			if(meng.sub_com!=1)
			{
				if(CheckColisionMouse(8192,2048+455,2048,227,0))
				{
					DrawStringUI(str,8192,2048+455,0,0,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj2.color.r);
						StartText();
						st.mouse1=0;
						meng.sub_com=1;
					}
				}
				else
					DrawStringUI(str,8192,2048+455,0,0,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				DrawStringUI(str,8192,2048+455,0,0,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,2048+810,0,0,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj2.color.g);
						StartText();
						st.mouse1=0;
						meng.sub_com=2;
					}
				}
				else
					DrawStringUI(str,8192,2048+810,0,0,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				DrawStringUI(str,8192,2048+810,0,0,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,2048+1265,0,0,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj2.color.b);
						StartText();
						st.mouse1=0;
						meng.sub_com=3;
					}
				}
				else
					DrawStringUI(str,8192,2048+1265,0,0,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==3)
			{
				DrawStringUI(str,8192,2048+1265,0,0,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,2048+1720,0,0,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj2.color.a);
						StartText();
						st.mouse1=0;
						meng.sub_com=4;
					}
				}
				else
					DrawStringUI(str,8192,2048+1720,0,0,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==4)
			{
				DrawStringUI(str,8192,2048+1720,0,0,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
			DrawStringUI(str,465,2887,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1)
			{
				meng.command=OBJ_AMBL;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,2887,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(meng.command==OBJ_AMBL)
		{
			DrawStringUI(str,465,2887,810,217,0,255,32,32,255,st.fonts[ARIAL].font,0,0,0);

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
			DrawStringUI(str,465,3315,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1 && meng.command!=TEX_PAN_OBJ)
			{
				meng.command=TEX_SIZE_OBJ;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,3315,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(meng.command==TEX_SIZE_OBJ)
		{
			DrawUI(8192,4096,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg[0].frames[4],255,6);

			sprintf(str,"X %d",meng.obj2.texsize.x);

			if(meng.sub_com!=1)
			{
				if(CheckColisionMouse(8192,3641,2048,910,0))
				{
					DrawStringUI(str,8192,3641,810,217,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,3641,810,217,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				DrawStringUI(str,8192,3641,810,217,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,4551,810,217,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,4551,810,217,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				DrawStringUI(str,8192,4551,810,217,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
			DrawStringUI(str,465,3640,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1 && meng.command!=TEX_SIZE_OBJ)
			{
				meng.command=TEX_PAN_OBJ;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,3640,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(meng.command==TEX_PAN_OBJ)
		{
			DrawUI(8192,4096,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg[0].frames[4],255,6);

			sprintf(str,"X %d",meng.obj2.texpan.x);

			if(meng.sub_com!=1)
			{
				if(CheckColisionMouse(8192,3641,2048,910,0))
				{
					DrawStringUI(str,8192,3641,810,217,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,3641,810,217,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				DrawStringUI(str,8192,3641,810,217,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,4551,810,217,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,4551,810,217,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				DrawStringUI(str,8192,4551,810,217,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
		DrawUI(247,681,435,435,0,128,32,32,0,0,32768,32768,mgg[0].frames[1],255,0);

		if(meng.obj.type==MIDGROUND)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Midground",465,2010,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
				if(st.mouse1)
				{
					meng.command=8;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Midground",465,2010,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		}

		if(meng.obj.type==FOREGROUND)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Foreground",465,2010,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
				if(st.mouse1)
				{
					meng.command=8;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Foreground",465,2010,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		}

		if(meng.obj.type==BACKGROUND1)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background1",465,2010,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
				if(st.mouse1)
				{
					meng.command=8;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background1",465,2010,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		}

		if(meng.obj.type==BACKGROUND2)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background2",465,2010,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
				if(st.mouse1)
				{
					meng.command=8;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background2",465,2010,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		}

		if(meng.obj.type==BACKGROUND3)
		{
			if(CheckColisionMouse(465,2010,810,217,0))
			{
				DrawStringUI("Background3",465,2010,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);
				if(st.mouse1)
				{
					meng.command=8;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Background3",465,2010,810,217,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
		}

		sprintf(str,"color");

		if(CheckColisionMouse(465,2445,810,217,0))
		{
			DrawStringUI(str,465,2445,810,217,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

			if(st.mouse1)
			{
				meng.command=RGB_OBJ;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,2445,810,217,0, 255, 255, 255,255,st.fonts[ARIAL].font,2048,2048,0);

		if(meng.command==RGB_OBJ)
		{
			DrawUI(8192,3072,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg[0].frames[4],255,6);

			DrawStringUI(str,465,2445,810,217,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

			sprintf(str,"R %d",meng.obj.color.r);

			if(meng.sub_com!=1)
			{
				if(CheckColisionMouse(8192,2048+455,2048,227,0))
				{
					DrawStringUI(str,8192,2048+455,0,0,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj.color.r);
						StartText();
						st.mouse1=0;
						meng.sub_com=1;
					}
				}
				else
					DrawStringUI(str,8192,2048+455,0,0,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				DrawStringUI(str,8192,2048+455,0,0,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,2048+810,0,0,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj.color.g);
						StartText();
						st.mouse1=0;
						meng.sub_com=2;
					}
				}
				else
					DrawStringUI(str,8192,2048+810,0,0,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				DrawStringUI(str,8192,2048+810,0,0,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,2048+1265,0,0,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj.color.b);
						StartText();
						st.mouse1=0;
						meng.sub_com=3;
					}
				}
				else
					DrawStringUI(str,8192,2048+1265,0,0,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==3)
			{
				DrawStringUI(str,8192,2048+1265,0,0,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,2048+1720,0,0,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

					if(st.mouse1)
					{
						sprintf(st.TextInput,"%d",meng.obj.color.a);
						StartText();
						st.mouse1=0;
						meng.sub_com=4;
					}
				}
				else
					DrawStringUI(str,8192,2048+1720,0,0,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==4)
			{
				DrawStringUI(str,8192,2048+1720,0,0,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
			DrawStringUI(str,465,2887,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1)
			{
				meng.command=OBJ_AMBL;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,2887,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(meng.command==OBJ_AMBL)
		{
			DrawStringUI(str,465,2887,810,217,0,255,32,32,255,st.fonts[ARIAL].font,0,0,0);

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
			DrawStringUI(str,465,3315,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1 && meng.command!=TEX_PAN_OBJ)
			{
				meng.command=TEX_SIZE_OBJ;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,3315,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(meng.command==TEX_SIZE_OBJ)
		{
			DrawUI(8192,4096,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg[0].frames[4],255,6);

			sprintf(str,"X %d",meng.obj.texsize.x);

			if(meng.sub_com!=1)
			{
				if(CheckColisionMouse(8192,3641,2048,910,0))
				{
					DrawStringUI(str,8192,3641,810,217,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,3641,810,217,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				DrawStringUI(str,8192,3641,810,217,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,4551,810,217,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,4551,810,217,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				DrawStringUI(str,8192,4551,810,217,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
			DrawStringUI(str,465,3640,810,217,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1 && meng.command!=TEX_SIZE_OBJ)
			{
				meng.command=TEX_PAN_OBJ;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI(str,465,3640,810,217,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(meng.command==TEX_PAN_OBJ)
		{
			DrawUI(8192,4096,2048,2048,0,255,255,255,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg[0].frames[4],255,6);

			sprintf(str,"X %d",meng.obj.texpan.x);

			if(meng.sub_com!=1)
			{
				if(CheckColisionMouse(8192,3641,2048,910,0))
				{
					DrawStringUI(str,8192,3641,810,217,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,3641,810,217,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==1)
			{
				DrawStringUI(str,8192,3641,810,217,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,4551,810,217,0,255,128,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
					DrawStringUI(str,8192,4551,810,217,0,255,255,255,255,st.fonts[ARIAL].font,2048,2048,0);
			}
			else
			if(meng.sub_com==2)
			{
				DrawStringUI(str,8192,4551,810,217,0,255,32,32,255,st.fonts[ARIAL].font,2048,2048,0);

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
		DrawUI(1365,1715,910,1185,0,255,255,255,0,0,32768,32768,mgg[0].frames[4],255,0);

		if(CheckColisionMouse(1365,1715,810,227,0))
		{
			DrawStringUI("Background1",1365,1715,810,227,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1)
			{
				meng.obj.type=BACKGROUND1;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI("Background1",1365,1715,810,227,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(CheckColisionMouse(1365,1465,810,227,0))
		{
			DrawStringUI("Background2",1365,1465,810,227,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1)
			{
				meng.obj.type=BACKGROUND2;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI("Background2",1365,1465,810,227,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(CheckColisionMouse(1365,1215,810,227,0))
		{
			DrawStringUI("Background3",1365,1215,810,227,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1)
			{
				meng.obj.type=BACKGROUND3;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI("Background3",1365,1215,810,227,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(CheckColisionMouse(1365,1920,810,227,0))
		{
			DrawStringUI("Midground",1365,1920,810,227,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1)
			{
				meng.obj.type=MIDGROUND;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI("Midground",1365,1920,810,227,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(CheckColisionMouse(1365,2170,810,227,0))
		{
			DrawStringUI("Foreground",1365,2170,810,227,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

			if(st.mouse1)
			{
				meng.obj.type=FOREGROUND;
				meng.command=meng.pannel_choice;
				st.mouse1=0;
			}
		}
		else
			DrawStringUI("Foreground",1365,2170,810,227,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
	}

	if(meng.command==EDIT_OBJ_TYPE)
	{
		DrawUI(1365,1715,910,1210,0,255,255,255,0,0,32768,32768,mgg[0].frames[4],255,0);

		if(CheckColisionMouse(1365,1715,810,227,0))
		{
			DrawStringUI("Background1",1365,1715,810,227,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

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
			DrawStringUI("Background1",1365,1715,810,227,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(CheckColisionMouse(1365,1465,810,227,0))
		{
			DrawStringUI("Background2",1365,1465,810,227,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

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
			DrawStringUI("Background2",1365,1465,810,227,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(CheckColisionMouse(1365,1215,810,227,0))
		{
			DrawStringUI("Background3",1365,1215,810,227,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

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
			DrawStringUI("Background3",1365,1215,810,227,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(CheckColisionMouse(1365,1920,810,227,0))
		{
			DrawStringUI("Midground",1365,1920,810,227,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

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
			DrawStringUI("Midground",1365,1920,810,227,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);

		if(CheckColisionMouse(1365,2170,810,227,0))
		{
			DrawStringUI("Foreground",1365,2170,810,227,0,255,128,32,255,st.fonts[ARIAL].font,0,0,0);

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
			DrawStringUI("Foreground",1365,2170,810,227,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
	}

}

static void ViewPortCommands()
{
	Pos vertextmp[4];
	uint8 got_it=0;
	char str[64];
	uint16 i, j;
	Pos p, p2;

	if(!CheckColisionMouse(455,4096,910,8192,0))
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

							p.x=st.Current_Map.obj[i].position.x+(st.Current_Map.obj[i].size.x/2);
							p.y=st.Current_Map.obj[i].position.y-(st.Current_Map.obj[i].size.y/2);

							p.x-=st.Camera.position.x;
							p.y-=st.Camera.position.y;

							sprintf(str,"%d",st.Current_Map.obj[i].angle);
							DrawString(str,p.x+227,p.y-227,405,217,0,255,255,255,255,st.fonts[ARIAL].font,1024,1024,0);

							if(st.mouse_wheel>0 && !st.mouse2)
							{
								if(st.keys[RSHIFT_KEY].state) st.Current_Map.obj[i].angle+=1;
								else st.Current_Map.obj[i].angle+=100;
								st.mouse_wheel=0;
							}

							if(st.mouse_wheel<0 && !st.mouse2)
							{
								if(st.keys[RSHIFT_KEY].state) st.Current_Map.obj[i].angle-=1;
								else st.Current_Map.obj[i].angle-=100;
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
							st.Current_Map.obj[i].size.x=2048;
							st.Current_Map.obj[i].size.y=2048;
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

static void MGGListLoad()
{
	FILE *file;
	char str[512], str2[512];
	uint16 j=MGG_MAP_START, i=0;

	if((file=fopen("mgg.list","r"))==NULL)
	{
		LogApp("Could not open mgg list file");
		Quit();
	}

	while(!feof(file))
	{
		DrawStringUI("Loading...",400,300,200,50,0,255,255,255,255,st.fonts[ARIAL].font,0,0,0);
		memset(str,0,sizeof(str));
		fgets(str,512,file);
		sscanf(str,"%s",str2);
		
		if(LoadMGG(&mgg[j],str2)!=NULL)
		{
			strcpy(meng.mgg_list[i],mgg[j].name);
			meng.num_mgg++;
			j++;
			i++;
			LogApp("Loaded: %s",str2);
		}

		Renderer();
	}

	fclose(file);

	LogApp("MGGs loaded");

}
/*
static void SpriteListLoad()
{
	FILE *file;
	char str[3][64], tmp[1024];
	int value[2];
	register uint16 i=MGG_SPRITE_START, line=0;

	if((file=fopen("sprite.list","r"))==NULL)
	{
		LogApp("Could not open the sprite list");
		Quit();
	}

	while(!feof(file))
	{
		BASICBKD();
		DrawStringUI("Loading...",8192,4096,1820,455,0,255,255,255,255,st.fonts[GEOMET].font,0,0,0);
		memset(tmp,0,sizeof(str));
		fgets(tmp,1024,file);
		sscanf(tmp,"%s %s %d %s %d",str[0], str[1], &value[0], str[2], &value[1]);
		if(strcmp(str[0],"\0")==NULL)
		{
			line++;
			continue;
		}
		else
		if(strcmp(str[0],"SPRITE")==NULL)
		{
			strcpy(st.Game_Sprites[value[0]].name,str[1]);
			if(strcmp(str[2],"NONE")==NULL)
			{
				st.Game_Sprites[value[0]].MGG_ID=-1;
				st.Game_Sprites[value[0]].frame=-1;
				st.num_sprites++;
				line++;
			}
			else
			{
				if(CheckMGGFile(str[2]))
				{
					DrawString2UI(str[2],8192,(4096)+455,0,0,0,255,255,255,255,st.fonts[ARIAL_BOULD].font,2048,2048,0);
					Renderer();
					if(LoadMGG(&mgg[i],str[2]))
					{
						st.Game_Sprites[value[0]].MGG_ID=i;
						st.Game_Sprites[value[0]].frame=value[1];
						st.num_sprites++;
						meng.num_mgg++;
						line++;
					}
					else
					{
						LogApp("Failed to load sprite MGG: %s at slot %d",str[2], i);
						line++;
						continue;
					}
				}
				else
				{
					LogApp("Invalid sprite MGG file: %s",str[2]);
					line++;
					continue;
				}
			}
		}
		else
		{
			LogApp("Invalid entry at line %d", line);
			line++;
			continue;
		}
		
		Renderer();
	}

	LogApp("%d Sprites loaded",st.num_sprites);
}
*/
int main(int argc, char *argv[])
{

	if(LoadCFG()==0)
		if(MessageBox(NULL,L"Error while trying to read or write the configuration file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();

	strcpy(st.WINDOW_NAME,"Engineer Map Editor ALPHA");

	Init();

	OpenFont("font//arial.ttf","arial",0,128);
	OpenFont("font//arialbd.ttf","arial bould",1,128);
	OpenFont("font//tt0524m_.ttf","geometry",2,128);

	InitMGG();

	if(LoadMGG(&mgg[0],"data//mEngUI.mgg")==NULL)
	{
		LogApp("Could not open UI mgg");
		Quit();
	}

	meng.num_mgg=0;
	memset(st.Game_Sprites,0,MAX_SPRITES*sizeof(_SPRITES));
	st.num_sprites=0;
	meng.got_it=-1;

	//SpriteListLoad();

	BASICBKD();
	DrawStringUI("Loading sprites...",8192,4096,0,0,0,255,255,255,255,st.fonts[GEOMET].font,2048,2048,6);

	LoadSpriteList("sprite.list");

	st.FPSYes=1;

	st.gt=MAIN_MENU;

	meng.pannel_choice=2;
	meng.command=2;

	st.Developer_Mode=1;

	meng.menu_sel=0;

	meng.path=(char*) malloc(2);
	strcpy(meng.path,".");

	memset(&meng.spr.body,0,sizeof(Body));

	while(!st.quit)
	{
		if(st.FPSYes)
			FPSCounter();

		BASICBKD();

		InputProcess();

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

		MainSound();
		Renderer();
		//Timer();
	}

	StopAllSounds();
	Quit();
	return 1;
}