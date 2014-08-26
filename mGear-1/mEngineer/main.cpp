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
		return false;

	st.screenx=800;
	st.screeny=600;
	st.fullscreen=0;
	st.bpp=32;
	st.audiof=44100;
	st.audioc=2;

	fprintf(file,"ScreenX = %d\n",st.screenx);
	fprintf(file,"ScreenY = %d\n",st.screeny);
	fprintf(file,"FullScreen = %d\n",st.fullscreen);
	fprintf(file,"ScreenBPP = %d\n",st.bpp);
	fprintf(file,"AudioFrequency = %d\n",st.audiof);
	fprintf(file,"AudioChannels = %d\n",st.audioc);

	fclose(file);

	return true;
}

uint16 LoadCFG()
{
	FILE *file;
	char buf[128], str[128];
	int value=0;
	if((file=fopen("settings.cfg","r"))==NULL)
		if(WriteCFG()==false)
			return false;

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
	}

	if(!st.screenx || !st.screeny || !st.bpp || !st.audioc || !st.audioc)
	{
		fclose(file);
		if(WriteCFG()==false)
			return false;
	}

	fclose(file);

	return true;

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
	uint16 j=0;
	DrawHud(400,300,250,600,0,255,255,255,0,0,1,1,st.UiTex[4].ID,1);
	if(meng.num_mgg>0)
	{
		for(uint16 i=25;i<800;i+=50)
		{
			if(j==meng.num_mgg)
				break;
			else
			{
				if(!CheckColisionMouse(400,i-meng.scroll2,150,50,0))
				{
					DrawString2(meng.mgg_list[j],400,i-meng.scroll2,0.5,0.5,0,255,128,32,1,st.fonts[ARIAL].font);
				}
				else
				{
					DrawString2(meng.mgg_list[j],400,i-meng.scroll2,0.5,0.5,0,255,32,0,1,st.fonts[ARIAL].font);
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

		if(st.keys[UP_KEY].state)
		{
			meng.scroll2+=25;
			st.keys[UP_KEY].state=0;
		}

		if(st.keys[DOWN_KEY].state)
		{
			meng.scroll2-=25;
			st.keys[DOWN_KEY].state=0;
		}
	}
	else
		meng.command=meng.pannel_choice;
}

void ImageList(_MGG mggs)
{
	uint16 m=0;

	for(uint16 i=80;i<160000;i+=160)
	{
		if(m>=mggs.num_frames) break;
		for(uint16 j=80;j<800;j+=160)
		{
			if(m<mggs.num_frames)
			{
				if((CheckColisionMouse(j,i-meng.scroll,160,160,0) && st.mouse1) || meng.tex_selection==mggs.frames[m])
				{
					DrawHud(j,i-meng.scroll,160,160,0,255,128,32,0,0,1,1,mggs.frames[m],1);
					meng.tex_selection=mggs.frames[m];
				}
				else
				{
					DrawHud(j,i-meng.scroll,160,160,0,255,255,255,0,0,1,1,mggs.frames[m],1);
				}

				m++;
			}
			else break;
		}
	}

	if(st.keys[DOWN_KEY].state)
	{
		meng.scroll+=160;
		st.keys[DOWN_KEY].state=0;
	}

	if(st.keys[UP_KEY].state)
	{
		meng.scroll-=160;
		st.keys[UP_KEY].state=0;
	}
}

static void PannelLeft()
{
	uint8 mouse=0;

	DrawHud(50,300,100,600,0,255,255,255,0,0,1,1,st.UiTex[4].ID,1);

	if(!CheckColisionMouse(27,27,48,48,0))
	{
		DrawHud(27,27,48,48,0,255,255,255,0,0,1,1,st.UiTex[0].ID,1);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawHud(27,27,48,48,0,255,128,32,0,0,1,1,st.UiTex[0].ID,1);
		//time++;
		//if((st.time-time)==1000)
		//{
			DrawString("Draw a sector",400,550,200,50,0,255,128,32,1,st.fonts[ARIAL].font);
		//}

			if(st.mouse1) meng.command=meng.pannel_choice=0;
	}

	if(!CheckColisionMouse(75,27,48,48,0))
	{
		DrawHud(75,27,48,48,0,255,255,255,0,0,1,1,st.UiTex[2].ID,1);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawHud(75,27,48,48,0,255,128,32,0,0,1,1,st.UiTex[2].ID,1);
		//if((st.time-time)==1000)
		//{
				DrawString("Select and edit",400,550,200,50,0,255,128,32,1,st.fonts[ARIAL].font);
		//}

		if(st.mouse1) meng.command=meng.pannel_choice=2;
	}
	
	if(!CheckColisionMouse(27,75,48,48,0))
	{
		DrawHud(27,75,48,48,0,255,255,255,0,0,1,1,st.UiTex[1].ID,1);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawHud(27,75,48,48,0,255,128,32,0,0,1,1,st.UiTex[1].ID,1);
		//if((st.time-time)==1000)
		//{
			DrawString("Add an OBJ",400,550,200,50,0,255,128,32,1,st.fonts[ARIAL].font);
		//}

		if(st.mouse1) meng.command=meng.pannel_choice=3;
	}
	
	if(!CheckColisionMouse(75,75,48,48,0))
	{
		DrawHud(75,75,48,48,0,255,255,255,0,0,1,1,st.UiTex[3].ID,1);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawHud(75,75,48,48,0,255,128,32,0,0,1,1,st.UiTex[3].ID,1);
		//if((st.time-time)==1000)
		//{
			DrawString("Add a sprite",400,550,200,50,0,255,128,32,1,st.fonts[ARIAL].font);
		//}

			if(st.mouse1) meng.command=meng.pannel_choice=4;
	}
	
	if(!CheckColisionMouse(51,123,60,24,0))
	{
		DrawString("Tex. Sel.",51,123,60,24,0,255,255,255,1,st.fonts[ARIAL].font);
	}
	else
	{
		DrawString("Tex. Sel.",51,123,60,24,0,255,128,32,1,st.fonts[ARIAL].font);

		DrawString("Texture Selection",400,550,300,50,0,255,128,32,1,st.fonts[ARIAL].font);

		if(st.mouse1) meng.command=5;
	}

	if(!CheckColisionMouse(51,171,60,24,0))
	{
		DrawString("MGG Sel.",51,171,60,24,0,255,255,255,1,st.fonts[ARIAL].font);
	}
	else
	{
		DrawString("MGG Sel.",51,171,60,24,0,255,128,32,1,st.fonts[ARIAL].font);

		DrawString("MGG Selection",400,550,300,50,0,255,128,32,1,st.fonts[ARIAL].font);

		if(st.mouse1) meng.command=6;
	}

	DrawHud(50,550,100,100,0,255,255,255,0,0,1,1,meng.tex_selection,1);

	if(meng.pannel_choice==0)
		DrawHud(27,27,48,48,0,128,32,32,0,0,1,1,st.UiTex[0].ID,1);
	//else
	if(meng.pannel_choice==2)
		DrawHud(75,27,48,48,0,128,32,32,0,0,1,1,st.UiTex[2].ID,1);
	//else
	if(meng.pannel_choice==4)
		DrawHud(75,75,48,48,0,128,32,32,0,0,1,1,st.UiTex[3].ID,1);
	//else
	if(meng.pannel_choice==3)
		DrawHud(27,75,48,48,0,128,32,32,0,0,1,1,st.UiTex[1].ID,1);

}

static void ViewPortCommands()
{
	Pos vertextmp[4];
	uint8 got_it=0;

	if(!CheckColisionMouse(50,300,100,600,0))
	{
		if(meng.command==DRAW_SECTOR)
		{
			if(st.mouse1)
			{
				if(st.Current_Map.num_sector<MAX_SECTORS)
				{
					for(uint16 i=0;i<MAX_SECTORS;i++)
					{
						if(st.Current_Map.sector[i].id==-1)
						{
							st.Current_Map.sector[i].id=i;
							st.Current_Map.sector[i].position.x=(st.mouse.x*16384)/st.screenx;
							st.Current_Map.sector[i].position.y=(st.mouse.y*8192)/st.screeny;
							st.Current_Map.sector[i].vertex[0].x=((st.mouse.x-64)*16384)/st.screenx;
							st.Current_Map.sector[i].vertex[0].y=((st.mouse.y-64)*8192)/st.screeny;
							st.Current_Map.sector[i].vertex[1].x=((st.mouse.x+64)*16384)/st.screenx;
							st.Current_Map.sector[i].vertex[1].y=((st.mouse.y-64)*8192)/st.screeny;
							st.Current_Map.sector[i].vertex[2].x=((st.mouse.x+64)*16384)/st.screenx;
							st.Current_Map.sector[i].vertex[2].y=((st.mouse.y+64)*8192)/st.screeny;
							st.Current_Map.sector[i].vertex[3].x=((st.mouse.x-64)*16384)/st.screenx;
							st.Current_Map.sector[i].vertex[3].y=((st.mouse.y+64)*8192)/st.screeny;
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
				for(uint16 i=0;i<st.Current_Map.num_sector;i++)
				{
					for(uint16 j=0;j<5;j++)
					{
						if(j<4 && CheckColisionMouse((st.Current_Map.sector[i].vertex[j].x*st.screenx)/16384,(st.Current_Map.sector[i].vertex[j].y*st.screeny)/8192,(256*st.screenx)/16384,(256*st.screenx)/8192,0) && st.mouse1)
						{
							st.Current_Map.sector[i].vertex[j].x=(st.mouse.x*16384)/st.screenx;
							st.Current_Map.sector[i].vertex[j].y=(st.mouse.y*8192)/st.screeny;
							got_it=1;
							break;
						}

						if(j==4 && CheckColisionMouse((st.Current_Map.sector[i].position.x*st.screenx)/16384,(st.Current_Map.sector[i].position.y*st.screeny)/8192,(484*st.screenx)/16384,(484*st.screeny)/8192,0) && st.mouse1)
						{
							vertextmp[0].x=(st.Current_Map.sector[i].vertex[0].x-st.Current_Map.sector[i].position.x);
							vertextmp[0].y=(st.Current_Map.sector[i].vertex[0].y-st.Current_Map.sector[i].position.y);
							vertextmp[1].x=(st.Current_Map.sector[i].vertex[1].x-st.Current_Map.sector[i].position.x);
							vertextmp[1].y=(st.Current_Map.sector[i].vertex[1].y-st.Current_Map.sector[i].position.y);
							vertextmp[2].x=(st.Current_Map.sector[i].vertex[2].x-st.Current_Map.sector[i].position.x);
							vertextmp[2].y=(st.Current_Map.sector[i].vertex[2].y-st.Current_Map.sector[i].position.y);
							vertextmp[3].x=(st.Current_Map.sector[i].vertex[3].x-st.Current_Map.sector[i].position.x);
							vertextmp[3].y=(st.Current_Map.sector[i].vertex[3].y-st.Current_Map.sector[i].position.y);

							st.Current_Map.sector[i].position.x=(st.mouse.x*16384)/st.screenx;
							st.Current_Map.sector[i].position.y=(st.mouse.y*8192)/st.screeny;

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

					if(got_it) break;
					
				}
			}
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
		DrawString("Loading...",400,300,200,50,0,255,255,255,1,st.fonts[GEOMET].font);
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

int main(int argc, char *argv[])
{
	if(LoadCFG()==false)
		if(MessageBox(NULL,L"Error while trying to read or write the configuration file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();

	strcpy(st.WINDOW_NAME,"Engineer Map Editor ALPHA");

	Init();

	OpenFont("font//arial.ttf","arial",0);
	OpenFont("font//arialbd.ttf","arial bould",1);
	OpenFont("font//tt0524m_.ttf","geometry",2);

	InitMGG();

	if(LoadMGG(&mgg[0],"data//mEngUI.mgg")==NULL)
	{
		LogApp("Could not open UI mgg");
		Quit();
	}

	for(register uint16 i=0;i<mgg[0].num_frames;i++)
	{
		st.UiTex[i].ID=mgg[0].frames[i];
		st.UiTex[i].MGG_ID=0;
	}

	MGGListLoad();

	st.FPSYes=1;

	st.gt=MAIN_MENU;

	meng.pannel_choice=2;
	meng.command=2;

	st.Developer_Mode=1;

	meng.menu_sel=0;

	while(!st.quit)
	{
		if(st.FPSYes)
			FPSCounter();

		InputProcess();

		if(st.keys[ESC_KEY].state) 
		{
			if(st.gt!=GAME_MENU)
				st.gt=GAME_MENU;
			else
				st.gt=INGAME;

			st.keys[ESC_KEY].state=0;
		}

		if(st.gt==INGAME)
		{
			if(meng.command==TEX_SEL)
			{
				ImageList(mgg[meng.mgg_sel]);
				if(st.keys[BACKSPACE_KEY].state)
				{
					meng.command=meng.pannel_choice;
				}
			}
			else
			if(meng.command==MGG_SEL)
			{
				PannelLeft();
				MGGList();
				if(st.keys[BACKSPACE_KEY].state)
				{
					meng.command=meng.pannel_choice;
				}
			}
			else
			{
				PannelLeft();
				ViewPortCommands();
				DrawMap();
			}
		}
		else
			if(st.gt==MAIN_MENU || st.gt==GAME_MENU)
			Menu();

		MainSound();
		Renderer();
		Timer();
	}

	StopAllSounds();
	Quit();
	return 1;
}