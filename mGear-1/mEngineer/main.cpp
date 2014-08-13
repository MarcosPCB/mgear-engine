#include "input.h"
#include "main.h"
#include <stdio.h> 
#include <stdlib.h>

extern _MGG mgg[MAX_MGG];

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
	
	long long unsigned int time=st.time;

	while(!st.quit)
	{
		if(st.FPSYes)
			FPSCounter();

		InputProcess();
		
		if(st.keys[0].state==1) st.quit=1;
		
		DrawMap();
		MainSound();
		Renderer();
		Timer();
	}

	StopAllSounds();
	Quit();
	return 1;
}