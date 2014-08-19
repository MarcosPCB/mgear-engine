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

	if(!CheckColisionMouse(27,27,48,48))
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
			DrawString("Draw a path or use it to block something",400,550,400,50,0,255,128,32,1,st.fonts[ARIAL].font);
		//}

			if(st.mouse1) meng.pannel_choice=1;
	}

	if(!CheckColisionMouse(75,27,48,48))
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
	
	if(!CheckColisionMouse(27,75,48,48))
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
	
	if(!CheckColisionMouse(75,75,48,48))
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

	if(!CheckColisionMouse(75,75,48,48) && !CheckColisionMouse(27,75,48,48) && !CheckColisionMouse(75,27,48,48) && !CheckColisionMouse(27,27,48,48) && st.mouse1)
		meng.pannel_choice=0;

	if(meng.pannel_choice==1)
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