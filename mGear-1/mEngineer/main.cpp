#include "input.h"
#include "main.h"
#include <stdio.h> 
#include <stdlib.h>

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
/*
uint8 LoadList()
{
	FILE *file;

	if((file=fopen("mgg.list","r"))==NULL)
	{
		LogApp("Unable to load MGG list");
		Quit();
	}

}
*/
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

			if(st.mouse1) meng.pannel_choice=0;
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

		if(st.mouse1) meng.pannel_choice=2;
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

		if(st.mouse1) meng.pannel_choice=4;
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

		if(st.mouse1) meng.pannel_choice=3;
	}
	/*
	if(!CheckColisionMouse(75,75,48,48) && !CheckColisionMouse(27,75,48,48) && !CheckColisionMouse(75,27,48,48) && !CheckColisionMouse(27,27,48,48) && st.mouse1)
		meng.pannel_choice=0;
		*/

	if(meng.pannel_choice==0)
		DrawHud(27,27,48,48,0,128,32,32,0,0,1,1,st.UiTex[0].ID,1);
	//else
	if(meng.pannel_choice==2)
		DrawHud(75,27,48,48,0,128,32,32,0,0,1,1,st.UiTex[2].ID,1);
	//else
	if(meng.pannel_choice==3)
		DrawHud(75,75,48,48,0,128,32,32,0,0,1,1,st.UiTex[3].ID,1);
	//else
	if(meng.pannel_choice==4)
		DrawHud(27,75,48,48,0,128,32,32,0,0,1,1,st.UiTex[1].ID,1);

	meng.command=meng.pannel_choice;
}

static void ViewPortCommands()
{
	Pos vertextmp[4];

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
				printf("Sector Added\n");
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
						if(j<4 && CheckColisionMouse(st.Current_Map.sector[i].vertex[j].x,st.Current_Map.sector[i].vertex[j].y,16,16,0) && st.mouse1)
							st.Current_Map.sector[i].vertex[j]=st.mouse;
						else if(j==4 && CheckColisionMouse(st.Current_Map.sector[i].position.x,st.Current_Map.sector[i].position.y,32,32,0) && st.mouse1)
						{
							vertextmp[0].x=(st.Current_Map.sector[i].vertex[0].x-st.Current_Map.sector[i].position.x);
							vertextmp[0].y=(st.Current_Map.sector[i].vertex[0].y-st.Current_Map.sector[i].position.y);
							vertextmp[1].x=(st.Current_Map.sector[i].vertex[1].x-st.Current_Map.sector[i].position.x);
							vertextmp[1].y=(st.Current_Map.sector[i].vertex[1].y-st.Current_Map.sector[i].position.y);
							vertextmp[2].x=(st.Current_Map.sector[i].vertex[2].x-st.Current_Map.sector[i].position.x);
							vertextmp[2].y=(st.Current_Map.sector[i].vertex[2].y-st.Current_Map.sector[i].position.y);
							vertextmp[3].x=(st.Current_Map.sector[i].vertex[3].x-st.Current_Map.sector[i].position.x);
							vertextmp[3].y=(st.Current_Map.sector[i].vertex[3].y-st.Current_Map.sector[i].position.y);

							st.Current_Map.sector[i].position=st.mouse;

							st.Current_Map.sector[i].vertex[0].x=st.Current_Map.sector[i].position.x+vertextmp[0].x;
							st.Current_Map.sector[i].vertex[0].y=st.Current_Map.sector[i].position.y+vertextmp[0].y;
							st.Current_Map.sector[i].vertex[1].x=st.Current_Map.sector[i].position.x+vertextmp[1].x;
							st.Current_Map.sector[i].vertex[1].y=st.Current_Map.sector[i].position.y+vertextmp[1].y;
							st.Current_Map.sector[i].vertex[2].x=st.Current_Map.sector[i].position.x+vertextmp[2].x;
							st.Current_Map.sector[i].vertex[2].y=st.Current_Map.sector[i].position.y+vertextmp[2].y;
							st.Current_Map.sector[i].vertex[3].x=st.Current_Map.sector[i].position.x+vertextmp[3].x;
							st.Current_Map.sector[i].vertex[3].y=st.Current_Map.sector[i].position.y+vertextmp[3].y;
						}
					}
					
				}
			}
		}
	}
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

	st.FPSYes=1;

	st.gt=MAIN_MENU;

	meng.pannel_choice=2;
	meng.command=2;

	st.Developer_Mode=1;

	st.Current_Map.obj=(_MGMOBJ*) malloc(MAX_OBJS*sizeof(_MGMOBJ));
	st.Current_Map.sector=(_SECTOR*) malloc(MAX_SECTORS*sizeof(_SECTOR));
	st.Current_Map.sprites=(_MGMSPRITE*) malloc(MAX_SPRITES*sizeof(_MGMSPRITE));

	st.Current_Map.num_sector=0;
	st.Current_Map.num_obj=0;
	st.Current_Map.num_sprites=0;

	for(register uint16 i=0;i<MAX_SECTORS;i++)
	{
		st.Current_Map.sector[i].id=-1;
		st.Current_Map.sector[i].layers=1;
		st.Current_Map.sector[i].material=CONCRETE;
		st.Current_Map.sector[i].tag=0;
	}

	for(register uint16 i=0;i<MAX_OBJS;i++)
		st.Current_Map.obj[i].type=BLANK;

	for(register uint16 i=0;i<MAX_SPRITES;i++)
		st.Current_Map.sprites[i].type=non;

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

		PannelLeft();
		ViewPortCommands();
		//Menu();
		
		DrawMap();
		MainSound();
		Renderer();
		Timer();
	}

	StopAllSounds();
	Quit();
	return 1;
}