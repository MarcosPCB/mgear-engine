#include "game.h"
#include "input.h"

extern _MGG mgg[MAX_MGG];
extern SDL_Window *wn;

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

void createmap()
{
	st.Current_Map.num_mgg=1;
	st.Current_Map.num_obj=2;
	st.Current_Map.num_sprites=0;
	strcpy(st.Current_Map.MGG_FILES[0],"TEX00.MGG");
	strcpy(st.Current_Map.name,"TEST");
	st.Current_Map.obj=(_MGMOBJ*) malloc(st.Current_Map.num_obj*sizeof(_MGMOBJ));
	st.Current_Map.sprites=(_MGMSPRITE*) malloc(0*sizeof(_MGMSPRITE));

	st.Current_Map.obj[0].angle=0;
	st.Current_Map.obj[0].block_type=none;
	st.Current_Map.obj[0].color.a=1;
	st.Current_Map.obj[0].color.r=255;
	st.Current_Map.obj[0].color.g=255;
	st.Current_Map.obj[0].color.b=255;
	st.Current_Map.obj[0].position.x=8192;
	st.Current_Map.obj[0].position.y=4096;
	st.Current_Map.obj[0].priority=0;
	st.Current_Map.obj[0].size.x=16384;
	st.Current_Map.obj[0].size.y=200;
	st.Current_Map.obj[0].tag=0;
	st.Current_Map.obj[0].TextureID=0;
	st.Current_Map.obj[0].texsize.x=50;
	st.Current_Map.obj[0].texsize.y=1;
	st.Current_Map.obj[0].texpan.x=0;
	st.Current_Map.obj[0].texpan.y=0;
	st.Current_Map.obj[0].type=FLOOR;

	st.Current_Map.obj[1].angle=0;
	st.Current_Map.obj[1].block_type=none;
	st.Current_Map.obj[1].color.a=1;
	st.Current_Map.obj[1].color.r=255;
	st.Current_Map.obj[1].color.g=255;
	st.Current_Map.obj[1].color.b=255;
	st.Current_Map.obj[1].position.x=8192;
	st.Current_Map.obj[1].position.y=2048;
	st.Current_Map.obj[1].priority=0;
	st.Current_Map.obj[1].size.x=16384;
	st.Current_Map.obj[1].size.y=200;
	st.Current_Map.obj[1].tag=0;
	st.Current_Map.obj[1].TextureID=1;
	st.Current_Map.obj[1].texsize.x=100;
	st.Current_Map.obj[1].texsize.y=1;
	st.Current_Map.obj[1].texpan.x=0;
	st.Current_Map.obj[1].texpan.y=0;
	st.Current_Map.obj[1].type=WALL_BACK;

	SaveMap("TEST.MAP");

	//FreeMap();
}

int main(int argc, char *argv[])
{
	if(LoadCFG()==false)
		if(MessageBox(NULL,L"Error while trying to read or write the configuration file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();

	strcpy(st.WINDOW_NAME,"mGear-1 Engine ALPHA");

	Init();

	InitMGG();

	//createmap();

	//createmgg();
	LoadMGG(&mgg[0],"fulgore.mgg");

	int32 startmovie=1;

	LoadMap("TEST.MAP");
	LoadMGG(&mgg[3],st.Current_Map.MGG_FILES[0]);
	
	st.MapTex[0].ID=mgg[3].frames[0];
	st.MapTex[0].MGG_ID=3;
	st.MapTex[1].ID=mgg[3].frames[2];
	st.MapTex[1].MGG_ID=3;
	
	long long unsigned int time=st.time;

	int32 anim=0;
	double X=6840, Y=4096;

	st.FPSYes=1;

	while(!st.quit)
	{

		InputProcess();

		if(startmovie==0)
		{
			PlayMovie("movie.mgv");
			startmovie=1;
		}
		
		if(st.keys[0].state==1) st.quit=1;
		if(st.keys[1].state==1)
		{
			startmovie=0;
			st.keys[1].state=0;
		}

		if(st.keys[2].state==1)
		{
			if(X<12000) X+=100;
			else if(X>12000) st.Camera.position.x+=100;
			st.keys[2].state=0;
		}

		if(st.keys[3].state==1)
		{
			if(X>4000) X-=100;
			else if(X<4000) st.Camera.position.x-=100;
			st.keys[3].state=0;
		}

		DrawMap();

		MAnim(X,Y,128,128,0,255,255,255,&mgg[0],1,0.3,1);
		//DrawSprite(8192-st.Camera.position.x,4096-st.Camera.position.y,256,256,0,255,250,250,mgg[0].frames[0],0.5);
		//DrawString(COOPER,"menu test",400,300,0.5,0.5,30,250,250,32,0.2);

		MainSound();
		Renderer();
		Timer();

		if(st.FPSYes)
			FPSCounter();
	}
	StopAllSounds();
	FreeMGG(&mgg[0]);
	Quit();
	return true;
}