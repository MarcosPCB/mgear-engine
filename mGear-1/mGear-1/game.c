#include "game.h"
#include "input.h"
#include "UI.h"
#include "actors.h"

PLAYERC playerc;

uint16 WriteCFG()
{
	FILE *file;

	if((file=fopen("settings.cfg","w"))==NULL)
		return 0;

	st.screenx=960;
	st.screeny=540;
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

void Player_BaseCode(int16 id)
{
	register int16 i, j, k;
	int16 tmp, temp;
	static int8 loop=0;
	float speed=0.5;

	if(st.keys[RIGHT_KEY].state)
	{
		st.Current_Map.sprites[id].position.x+=16;
		st.Camera.position.x+=8;
		playerc.current_anim=WALK;
		speed=0.5;
		//SetAnim(WALK,id);
		loop=1;
		//st.keys[RIGHT_KEY].state=0;
	}
	
	if(st.keys[LEFT_KEY].state)
	{
		st.Current_Map.sprites[id].position.x-=16;
		st.Camera.position.x-=8;
		playerc.current_anim=WALK;
		speed=0.5;
		//SetAnim(WALK,id);
		loop=1;
		//st.keys[LEFT_KEY].state=0;
	}

	if(st.keys[S_KEY].state)
	{
		playerc.current_anim=PUNCH1;
		SetAnim(PUNCH1,id);
		loop=0;
		speed=0.02;
		//st.keys[S_KEY].state=0;
	}

	if(st.keys[W_KEY].state)
	{
		playerc.current_anim=PUNCH2;
		SetAnim(PUNCH2,id);
		loop=0;
		speed=0.02;
		//st.keys[S_KEY].state=0;
	}

	tmp=MAnim(playerc.current_anim,speed,id,loop);

	if(tmp==1)
	{
		playerc.current_anim=STAND;
		speed=0.5;
		loop=1;
	}

	if(playerc.current_anim==WALK && !st.keys[LEFT_KEY].state && !st.keys[RIGHT_KEY].state)
	{
		playerc.current_anim=STAND;
		speed=0.5;
		loop=1;
	}

	st.Current_Map.sprites[id].position.y+=2;

	for(i=0;i<st.Current_Map.num_sector;i++)
	{
			temp=st.Current_Map.sector[i].vertex[0].y;

			for(j=1;j<4;j++)
			{
				if(temp>st.Current_Map.sector[i].vertex[j].y)
					temp=st.Current_Map.sector[i].vertex[j].y;
			}

			if(st.Current_Map.sprites[id].position.y+(st.Current_Map.sprites[id].body.size.y/2)>temp)
				st.Current_Map.sprites[id].position.y=temp-(st.Current_Map.sprites[id].body.size.y/2);

			//break;
	}
}

void SpawnPlayer(Pos pos, Pos size, int16 ang)
{
	int16 i=st.Current_Map.num_sprites, j;

	st.Current_Map.sprites[i].GameID=PLAYER1;
	st.Current_Map.sprites[i].position=pos;
	st.Current_Map.sprites[i].angle=ang;

	st.Current_Map.sprites[i].stat=1;

	for(j=0;j<st.num_sprites;j++)
	{
		if(j==SEKTOR)
		{
			st.Current_Map.sprites[i].frame_ID=st.Current_Map.sprites[i].current_frame=st.Game_Sprites[j].frame[0];
			st.Current_Map.sprites[i].MGG_ID=st.Game_Sprites[j].MGG_ID;
			st.Current_Map.sprites[i].body.size.x=st.Game_Sprites[j].body.size.x*2;
			st.Current_Map.sprites[i].body.size.y=st.Game_Sprites[j].body.size.y*2;
			break;
		}
	}

	st.Current_Map.num_sprites++;
}

void SpawnSprite(int16 game_id, Pos pos, Pos size, int16 ang)
{
	int16 i=st.Current_Map.num_sprites;

	st.Current_Map.sprites[i].GameID=game_id;
	st.Current_Map.sprites[i].position=pos;
	st.Current_Map.sprites[i].body.size=size;
	st.Current_Map.sprites[i].angle=ang;

	st.Current_Map.sprites[i].MGG_ID=st.Game_Sprites[game_id].MGG_ID;
	st.Current_Map.sprites[i].frame_ID=st.Current_Map.sprites[i].current_frame=st.Game_Sprites[game_id].frame[0];
	st.Current_Map.sprites[i].stat=1;

	st.Current_Map.num_sprites++;
}

void PreGameEvent()
{
	register int16 i, j;
	int16 tmp, temp, starter=0;

	memset(&playerc,0,sizeof(playerc));

	for(i=0;i<st.Current_Map.num_sprites;i++)
	{
		if(st.Current_Map.sprites[i].GameID==STARTER && !starter)
		{
			SpawnPlayer(st.Current_Map.sprites[i].position,st.Current_Map.sprites[i].body.size,st.Current_Map.sprites[i].angle);
			starter=1;
		}
		else
		if(st.Current_Map.sprites[i].GameID==SOUNDFX)
		{
			for(j=0;j<st.Current_Map.sprites[i].num_tags;j++)
			{
				if(strcmp(st.Game_Sprites[SOUNDFX].tag_names[j],"PATH_S")==NULL && strlen(st.Current_Map.sprites[i].tags_str[j])>5)
					PlaySound(st.Current_Map.sprites[i].tags_str[j],0);
			}
		}
	}
}

void GameEvent()
{
	register int16 i, j;
	int16 tmp, temp;

	for(i=0;i<st.Current_Map.num_sprites;i++)
	{
		//switch(st.Current_Map.sprites[i].GameID)
		//{
			//case PLAYER1:
		if(st.Current_Map.sprites[i].GameID==PLAYER1)
		{
				Player_BaseCode(i);
		}
		//}
	}
}

int main(int argc, char *argv[])
{
	int loops;
	int prev_tic, curr_tic, delta;

	st.FPSYes=1;

	if(LoadCFG()==0)
		if(MessageBox(NULL,L"Error while trying to read or write the configuration file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();

	Init();

	strcpy(st.WindowTitle,"mGear Test Demo");

	OpenFont("font/arial.ttf","arial",ARIAL,128);
	OpenFont("font/ftp.ttf","ftp",FIGHTFONT,128);

	InitMGG();

	if(LoadMGG(&mgg_sys[0],"data/UI.mgg")==NULL)
	{
		LogApp("Could not open UI mgg");
		Quit();
	}

	UILoadSystem("UI_Sys.cfg");

	memset(st.Game_Sprites,0,MAX_SPRITES*sizeof(_SPRITES));
	st.num_sprites=0;

	BASICBKD(255,255,255);
	DrawString2UI("Loading sprites...",8192,4096,0,0,0,255,255,255,255,ARIAL,2048,2048,6);

	Renderer(1);

	LoadSpriteList("sprite.list");

	st.gt=MAIN_MENU;

	curr_tic=GetTicks();
	delta=1;

	st.viewmode=31+32;

	while(!st.quit)
	{
		if(st.FPSYes)
			FPSCounter();

		InputProcess();

		loops=0;
		while(GetTicks() > curr_tic && loops < 10)
		{
			Finish();

			if(st.gt==MAIN_MENU)
				Menu();
			else
			if(st.gt==INGAME)
			{
				GameEvent();
				LockCamera();
			}

			curr_tic+=1000/TICSPERSECOND;
			loops++;
			SetTimerM(1);

		}

		DrawSys();

		if(st.gt==INGAME)
			DrawMap();

		UIMain_DrawSystem();

		Renderer(0);
	}

	StopAllSounds();
	Quit();

	return 0;
}