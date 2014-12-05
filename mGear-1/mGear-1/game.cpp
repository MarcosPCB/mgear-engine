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
	st.vsync=0;

	fprintf(file,"ScreenX = %d\n",st.screenx);
	fprintf(file,"ScreenY = %d\n",st.screeny);
	fprintf(file,"FullScreen = %d\n",st.fullscreen);
	fprintf(file,"ScreenBPP = %d\n",st.bpp);
	fprintf(file,"AudioFrequency = %d\n",st.audiof);
	fprintf(file,"AudioChannels = %d\n",st.audioc);
	fprintf(file,"VSync = %d\n",st.vsync);

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
		if(strcmp(str,"VSync")==NULL) st.vsync=value;
	}

	if(!st.screenx || !st.screeny || !st.bpp || !st.audioc || !st.audioc || st.vsync>1)
	{
		fclose(file);
		if(WriteCFG()==false)
			return false;
	}

	fclose(file);

	return true;

}
/*
void createmap()
{
	st.Current_Map.num_mgg=2;
	st.Current_Map.num_obj=1;
	st.Current_Map.num_sprites=0;
	st.Current_Map.num_lights=2;
	strcpy(st.Current_Map.MGG_FILES[0],"STAGE.MGG");
	strcpy(st.Current_Map.MGG_FILES[1],"LIGHT_MAPS.MGG");
	strcpy(st.Current_Map.name,"TEST");
	st.Current_Map.obj=(_MGMOBJ*) malloc(st.Current_Map.num_obj*sizeof(_MGMOBJ));
	st.Current_Map.sprites=(_MGMSPRITE*) malloc(0*sizeof(_MGMSPRITE));
	st.Current_Map.light=(_MGMLIGHT*) malloc(2*sizeof(_MGMLIGHT));

	st.Current_Map.obj[0].angle=0;
	st.Current_Map.obj[0].block_type=none;
	st.Current_Map.obj[0].color.a=1;
	st.Current_Map.obj[0].color.r=255;
	st.Current_Map.obj[0].color.g=255;
	st.Current_Map.obj[0].color.b=255;
	st.Current_Map.obj[0].position.x=8192;
	st.Current_Map.obj[0].position.y=3096;
	st.Current_Map.obj[0].priority=0;
	st.Current_Map.obj[0].size.x=18384;
	st.Current_Map.obj[0].size.y=10192;
	st.Current_Map.obj[0].tag=0;
	st.Current_Map.obj[0].TextureID=0;
	st.Current_Map.obj[0].texsize.x=-1;
	st.Current_Map.obj[0].texsize.y=-1;
	st.Current_Map.obj[0].texpan.x=0;
	st.Current_Map.obj[0].texpan.y=0;
	st.Current_Map.obj[0].type=MIDGROUND;
	st.Current_Map.obj[0].amblight=1;
	
	st.Current_Map.light[0].angle=0;
	st.Current_Map.light[0].color.r=155;
	st.Current_Map.light[0].color.g=155;
	st.Current_Map.light[0].color.b=155;
	st.Current_Map.light[0].color.a=1;
	st.Current_Map.light[0].position.x=2048;
	st.Current_Map.light[0].position.y=4096;
	st.Current_Map.light[0].size.x=8192;
	st.Current_Map.light[0].size.y=8192;
	st.Current_Map.light[0].tag=0;
	st.Current_Map.light[0].TextureID=1;

	st.Current_Map.light[1].angle=0;
	st.Current_Map.light[1].color.r=255;
	st.Current_Map.light[1].color.g=255;
	st.Current_Map.light[1].color.b=255;
	st.Current_Map.light[1].color.a=1;
	st.Current_Map.light[1].position.x=12096;
	st.Current_Map.light[1].position.y=4096;
	st.Current_Map.light[1].size.x=8192;
	st.Current_Map.light[1].size.y=8192;
	st.Current_Map.light[1].tag=0;
	st.Current_Map.light[1].TextureID=1;
	
	SaveMap("TEST.MAP");

	//FreeMap();
}
*/
int main(int argc, char *argv[])
{

	register uint32 i=0, j=0;

	if(LoadCFG()==false)
		if(MessageBox(NULL,L"Error while trying to read or write the configuration file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();

	strcpy(st.WINDOW_NAME,"mGear-1 Engine PRE-ALPHA");

	Init();

	OpenFont("font01.ttf","font01",0);

	InitMGG();

	//createmap();

	//createmgg();
	LoadMGG(&mgg[0],"fulgore.mgg");
	//LoadMGG(&mgg[1],"mEngUI.mgg");

	int32 startmovie=1;

	//LoadMap("TEST.MAP");
	//LoadMGG(&mgg[3],st.Current_Map.MGG_FILES[0]);
	//LoadMGG(&mgg[4],st.Current_Map.MGG_FILES[1]);
	
	long long unsigned int time=st.time;

	int32 anim=0;

	Pos fulg;
	fulg.x=6840;
	fulg.y=4096;
	Pos fulgsize;
	fulgsize.x=3048;
	fulgsize.y=3048;

	Pos fulg1;
	fulg1.x=14000;
	fulg1.y=4096;
	fulgsize.x=3048;
	fulgsize.y=3048;

	st.FPSYes=1;

	char num[64];

	while(!st.quit)
	{
		if(st.FPSYes)
			FPSCounter();

		InputProcess();

		if(startmovie==0)
		{
			PlayMovie("LOGOHD.MGV");
			//PlayMusic("castle.ogg",1);
			//st.screenx=1280;
			//st.screeny=720;
			//RestartVideo();
			startmovie=1;
		}
		
		if(st.keys[0].state==1) st.quit=1;
		if(st.keys[1].state==1)
		{
			startmovie=0;
			st.keys[1].state=0;
		}
		/*
		if(st.control_num>0)
		{

			//printf("%f\n",st.controller[0].axis[0].state);
			//printf("%f\n",st.controller[0].axis[1].state);

			if(st.controller[0].axis[0].state>10000)
			{
				//SDL_HapticRumblePlay(st.controller[0].force,1,100);

				if(fulg.x<=st.Camera.position.x+12000 && fulg.x>=st.Camera.position.x+4000) fulg.x+=(100.0f/32768.0f)*st.controller[0].axis[0].state;
				else if(fulg.x>st.Camera.position.x+4000)
				{
					fulg.x+=(100.0f/32768.0f)*st.controller[0].axis[0].state;
					st.Camera.position.x+=150;
				}
				//else if(X<4000) st.Camera.position.x-=100;
				//PlayMovie("LOGOHD.MGV");
				//st.controller[0].axis[0].state=0;
			}

			if(st.controller[0].axis[0].state<-10000)
			{
				//SDL_HapticRumblePlay(st.controller[0].force,1,500);

				if(fulg.x>=st.Camera.position.x+4000) fulg.x+=(100.0f/32768.0f)*st.controller[0].axis[0].state;
				//else if(X>12000) st.Camera.position.x+=100;
				else if(fulg.x<=st.Camera.position.x+4000) 
				{
					fulg.x+=(100.0f/32768.0f)*st.controller[0].axis[0].state;
					st.Camera.position.x-=150;
				}
				//PlayMovie("LOGOHD.MGV");
				//st.controller[0].axis[0].state=0;
			}

			if(st.controller[0].axis[1].state>10000)
			{
				//SDL_HapticRumblePlay(st.controller[0].force,1,100);

				if(fulg.y<=st.Camera.position.y+6000 && fulg.y>=st.Camera.position.y+2000) fulg.y+=(100.0f/32768.0f)*st.controller[0].axis[1].state;
				else if(fulg.y>=st.Camera.position.y+2000)
				{
					fulg.y+=(100.0f/32768.0f)*st.controller[0].axis[1].state;
					st.Camera.position.y+=150;
				}
				//else if(X<4000) st.Camera.position.x-=100;
				//PlayMovie("LOGOHD.MGV");
				//st.controller[0].axis[0].state=0;
			}

			if(st.controller[0].axis[1].state<-10000)
			{
				//SDL_HapticRumblePlay(st.controller[0].force,1,500);

				if(fulg.y>=st.Camera.position.y+2000) fulg.y+=(100.0f/32768.0f)*st.controller[0].axis[1].state;
				//else if(X>12000) st.Camera.position.x+=100;
				else if(fulg.y<=st.Camera.position.y+2000) 
				{
					st.Camera.position.y-=150;
					fulg.y+=(100.0f/32768.0f)*st.controller[0].axis[1].state;
				}
				//PlayMovie("LOGOHD.MGV");
				//st.controller[0].axis[0].state=0;
			}

			if(st.controller[0].button[13].state==1)
			{
				if(fulg.x>4000) fulg.x-=110;
				else if(fulg.x<4000) st.Camera.position.x-=100;
				st.keys[3].state=0;
			}
			
		}
		*/
		//DrawMap();
		

		//DrawMap();

		//MAnim(fulg.x-st.Camera.position.x,fulg.y-st.Camera.position.y,3048,3048,0,255,255,255,&mgg[0],1,0.3,2);

	
	uint32 timej, timel;

	//timej=GetTicks();

		//for(i=0;i<100;i++)
			//for(j=0;j<64;j++)
				//DrawSprite(j*256,4096,256,256,0,255,255,255,mgg[0].frames[52],255,0);
	//st.Camera.dimension.x=2;
	//st.Camera.dimension.y=2;
				DrawSprite(2048,4096,2048,2048,0,255,255,255,mgg[0].frames[49],255,0);
				DrawSprite(8192,4096,2048,2048,0,255,255,255,mgg[0].frames[59],255,2048);
				//MAnim(8192,4096,2048,2048,0,255,255,255,mgg,1,1,255);
				//DrawSprite(4096,4096,2048,2048,0,255,255,255,mgg[0].frames[52],255,2048);
				//DrawSprite(6200,4096,2048,2048,0,255,255,255,mgg[0].frames[53],255,2048);
				//DrawSprite(8248,4096,2048,2048,0,255,255,255,mgg[0].frames[54],255,2048);
				//DrawSprite(10500,4096,2048,2048,0,255,255,255,mgg[0].frames[55],255,2048);
				//DrawSprite(12548,4096,2048,2048,0,255,255,255,mgg[0].frames[56],255,2048);
				//DrawSprite(14600,4096,2048,2048,0,255,255,255,mgg[0].frames[57],255,2048);
				//DrawSprite(16648,4096,2048,2048,0,255,255,255,mgg[0].frames[57],255,2048);
				//DrawSprite(8192,4096,1024,1024,0,255,255,255,mgg[0].frames[52],255,0);
				//DrawSprite(1024,4096,1024,1024,0,255,255,255,mgg[0].frames[1],1,0);
				//DrawSprite(250,950,250,250,0,255,255,255,0,1);
				//DrawSprite(250,1200,250,250,0,255,255,255,0,1);
				//DrawSprite(250,1500,250,250,0,255,255,255,0,1);
				//DrawSprite(850,1500,250,250,0,255,255,255,0,1);

		//timel=GetTicks();
		//LogApp("For Time: %d",timel-timej);
			//}
		
		//for(register uint16 i=0;i<st.Current_Map.num_lights;i++)
			//DrawLight(st.Current_Map.light[i].position.x-st.Camera.position.x,st.Current_Map.light[i].position.y-st.Camera.position.y,st.Current_Map.light[i].size.x,st.Current_Map.light[i].size.y,
			//st.Current_Map.light[i].angle,st.Current_Map.light[i].color.r,st.Current_Map.light[i].color.g,st.Current_Map.light[i].color.b,st.MapTex[st.Current_Map.light[i].TextureID].ID,st.Current_Map.light[i].color.a);

		

		//MainSound();

		//timej=GetTicks();

		Renderer();

		//timel=GetTicks();
		//LogApp("Render Time: %d",timel-timej);
		//Timer();
	}
	StopAllSounds();
	//FreeMGG(&mgg[0]);
	Quit();
	return true;
}