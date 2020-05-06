#include "game.h"
#include "input.h"
#include "UI.h"
#include "actors.h"
#include "physics.h"
#include "mgl.h"

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

void GameInput()
{
	register int16 i;
	
	for(i=0;i<8;i++)
	{
		if(playerc.key_pressed[i])
		{
			playerc.key_time2[i]++;

			if(playerc.key_time2[i]>64 && !playerc.key_state[i])
			{
				playerc.key_time2[i]=0;
				playerc.key_pressed[i]=0;
			}
		}
	}
	
	if(st.keys[W_KEY].state)
	{
		playerc.key_state[0]=1;

		if(!playerc.key_pressed[0])
		{
			playerc.key_pressed[0]++;
			playerc.key_time[0]=0;
			playerc.key_p[0]=1;
			playerc.key_time2[0]=0;
		}
		else
		{
			if(!playerc.key_p[0])
			{
				playerc.key_pressed[0]++;
				playerc.key_time2[0]=0;
				playerc.key_p[0]=1;
			}

			playerc.key_time[0]++;
			if(playerc.key_time[0]>32)
				playerc.key_state[0]=2;
		}
	}

	if(st.keys[S_KEY].state)
	{
		playerc.key_state[1]=1;

		if(!playerc.key_pressed[1])
		{
			playerc.key_pressed[1]++;
			playerc.key_time[1]=0;
			playerc.key_p[1]=1;
			playerc.key_time2[1]=0;
		}
		else
		{
			if(!playerc.key_p[1])
			{
				playerc.key_pressed[1]++;
				playerc.key_time2[1]=0;
				playerc.key_p[1]=1;
			}

			playerc.key_time[1]++;
			if(playerc.key_time[1]>32)
				playerc.key_state[1]=2;
		}
	}

	if(st.keys[E_KEY].state)
	{
		playerc.key_state[2]=1;

		if(!playerc.key_pressed[2])
		{
			playerc.key_pressed[2]++;
			playerc.key_time[2]=0;
			playerc.key_p[2]=1;
			playerc.key_time2[2]=0;
		}
		else
		{
			if(!playerc.key_p[2])
			{
				playerc.key_pressed[2]++;
				playerc.key_time2[2]=0;
				playerc.key_p[2]=1;
			}

			playerc.key_time[2]++;
			if(playerc.key_time[2]>32)
				playerc.key_state[2]=2;
		}
	}

	if(st.keys[D_KEY].state)
	{
		playerc.key_state[3]=1;

		if(!playerc.key_pressed[3])
		{
			playerc.key_pressed[3]++;
			playerc.key_time[3]=0;
			playerc.key_p[3]=1;
			playerc.key_time2[3]=0;
		}
		else
		{
			if(!playerc.key_p[3])
			{
				playerc.key_pressed[3]++;
				playerc.key_time2[3]=0;
				playerc.key_p[3]=1;
			}

			playerc.key_time[3]++;
			if(playerc.key_time[3]>32)
				playerc.key_state[3]=2;
		}
	}

	if(st.keys[UP_KEY].state)
	{
		playerc.key_state[4]=1;

		if(!playerc.key_pressed[4])
		{
			playerc.key_pressed[4]++;
			playerc.key_time[4]=0;
			playerc.key_p[4]=1;
			playerc.key_time2[4]=0;
		}
		else
		{
			if(!playerc.key_p[4])
			{
				playerc.key_pressed[4]++;
				playerc.key_time2[4]=0;
				playerc.key_p[4]=1;
			}

			playerc.key_time[4]++;
			if(playerc.key_time[4]>32)
				playerc.key_state[4]=2;
		}
	}

	if(st.keys[DOWN_KEY].state)
	{
		playerc.key_state[5]=1;

		if(!playerc.key_pressed[5])
		{
			playerc.key_pressed[5]++;
			playerc.key_time[5]=0;
			playerc.key_p[5]=1;
			playerc.key_time2[5]=0;
		}
		else
		{
			if(!playerc.key_p[5])
			{
				playerc.key_pressed[5]++;
				playerc.key_time2[5]=0;
				playerc.key_p[5]=1;
			}

			playerc.key_time[5]++;
			if(playerc.key_time[5]>32)
				playerc.key_state[5]=2;
		}
	}

	if(st.keys[LEFT_KEY].state)
	{
		playerc.key_state[6]=1;

		if(!playerc.key_pressed[6])
		{
			playerc.key_pressed[6]++;
			playerc.key_time[6]=0;
			playerc.key_p[6]=1;
			playerc.key_time2[6]=0;
		}
		else
		{
			if(!playerc.key_p[6])
			{
				playerc.key_pressed[6]++;
				playerc.key_time2[6]=0;
				playerc.key_p[6]=1;
			}

			playerc.key_time[6]++;
			if(playerc.key_time[6]>32)
				playerc.key_state[6]=2;
		}
	}

	if(st.keys[RIGHT_KEY].state)
	{
		playerc.key_state[7]=1;

		if(!playerc.key_pressed[7])
		{
			playerc.key_pressed[7]++;
			playerc.key_time[7]=0;
			playerc.key_p[7]=1;
			playerc.key_time2[7]=0;
		}
		else
		{
			if(!playerc.key_p[7])
			{
				playerc.key_pressed[7]++;
				playerc.key_time2[7]=0;
				playerc.key_p[7]=1;
			}

			playerc.key_time[7]++;
			if(playerc.key_time[7]>32)
				playerc.key_state[7]=2;
		}
	}

	if(!st.keys[W_KEY].state && playerc.key_state[0])
	{
		playerc.key_state[0]=0;
		playerc.key_time[0]=0;
		playerc.key_p[0]=0;
	}

	if(!st.keys[S_KEY].state && playerc.key_state[1])
	{
		playerc.key_state[1]=0;
		playerc.key_time[1]=0;
		playerc.key_p[1]=0;
	}

	if(!st.keys[E_KEY].state && playerc.key_state[2])
	{
		playerc.key_state[2]=0;
		playerc.key_time[2]=0;
		playerc.key_p[2]=0;
	}

	if(!st.keys[D_KEY].state && playerc.key_state[3])
	{
		playerc.key_state[3]=0;
		playerc.key_time[3]=0;
		playerc.key_p[3]=0;
	}

	if(!st.keys[UP_KEY].state && playerc.key_state[4])
	{
		playerc.key_state[4]=0;
		playerc.key_time[4]=0;
		playerc.key_p[4]=0;
	}

	if(!st.keys[DOWN_KEY].state && playerc.key_state[5])
	{
		playerc.key_state[5]=0;
		playerc.key_time[5]=0;
		playerc.key_p[5]=0;
	}

	if(!st.keys[LEFT_KEY].state && playerc.key_state[6])
	{
		playerc.key_state[6]=0;
		playerc.key_time[6]=0;
		playerc.key_p[6]=0;
	}

	if(!st.keys[RIGHT_KEY].state && playerc.key_state[7])
	{
		playerc.key_state[7]=0;
		playerc.key_time[7]=0;
		playerc.key_p[7]=0;
	}
}

void SetPlayerAnim(uint16 id, uint16 anim, float speed, uint8 loop)
{
	playerc.current_anim=anim;
	SetAnim(anim,id);
	playerc.speed=speed;
	playerc.anim_loop=loop;
}
/*
int8 DoCombination(uint8 keys[4])
{
	if(playerc.key_pressed[0]>0)
}
*/

void DoPlayerHit(int8 id)
{
	SetPlayerAnim(playerc.i,playerc.hit[id].anim,playerc.hit[id].speed,0);
}

void Player_BaseCode(int16 id)
{
	register int16 i, j, k;
	int16 tmp, temp;
	static int8 loop=0;
	static float speed=2;
	int32 sety;

	if(playerc.key_state[RGK])
	{
		st.Current_Map.sprites[id].body.velxy.x=64;
		st.Camera.position.x+=32;
		SetPlayerAnim(id,WALK,4,1);
		//st.keys[RIGHT_KEY].state=0;
	}
	
	if(playerc.key_state[LFK])
	{
		st.Current_Map.sprites[id].body.velxy.x = -64;
		st.Camera.position.x-=32;
		SetPlayerAnim(id,WALKB,4,1);
		//st.keys[LEFT_KEY].state=0;
	}

	if(playerc.key_state[SK]==1 && !CheckAnim(id,PUNCHS1) && !CheckAnim(id,PUNCHS2))
	{
		if(playerc.state & 2)
		{
			playerc.state-=2;
			DoPlayerHit(2);
		}
		else
		{
			playerc.state+=2;
			DoPlayerHit(3);
		}
		//loop=0;
		
		PlaySound(0,0);
	}

	

	if(playerc.key_state[WK]==1 && !CheckAnim(id,PUNCHM1) && !CheckAnim(id,PUNCHM2))
	{
		if(playerc.state & 1)
		{
			playerc.state-=1;
			DoPlayerHit(1);
		}
		else
		{
			playerc.state+=1;
			DoPlayerHit(0);
		}

		ShootProjectile(0, 0, id, st.Current_Map.sprites[id].position);

		PlaySound(0,0);
	}

	if(playerc.key_state[UPK]==1 && !CheckAnim(id,JUMPING) && ~playerc.state & 4)
	{
		SetPlayerAnim(id,JUMPING,6,0);

		playerc.key_state[4]=1;
		playerc.state+=4;
		st.Current_Map.sprites[id].body.velxy.y=-256;
	}

	if(playerc.key_state[EK]==1 && !CheckAnim(id,KICKM))
	{
		DoPlayerHit(6);
		
		PlaySound(0,0);

	}

	if(playerc.key_state[DK]==1  && !CheckAnim(id,KICKS))
	{
		DoPlayerHit(5);
		
		PlaySound(0,0);
	}

	if(playerc.key_pressed[RGK]==2 && playerc.key_pressed[WK]==1 && (~playerc.state & 16))
	{
		playerc.state=16;
		playerc.combo_t=0;
	}

	if(playerc.state & 16)
	{
		if(playerc.combo_t==0)
		{
			SetPlayerAnim(id,PUNCHM1,7,0);
			PlaySound(0,0);
			playerc.combo_t=1;
		}
		else
		if(playerc.combo_t==1 && !CheckAnim(id,PUNCHM1))
		{
			SetPlayerAnim(id,PUNCHM2,7,0);
			PlaySound(0,0);
			playerc.combo_t=2;
		}
		else
		if(playerc.combo_t==2 && !CheckAnim(id,PUNCHM2))
		{
			SetPlayerAnim(id,PUNCHM1,7,0);
			PlaySound(0,0);
			playerc.combo_t=3;
		}
		else
		if(playerc.combo_t==3 && !CheckAnim(id,PUNCHM1))
		{
			SetPlayerAnim(id,KICKS,5,0);
			PlaySound(0,0);
			playerc.combo_t=4;
		}
		else
		if(playerc.combo_t==4 && !CheckAnim(id,KICKS))
		{
			SetPlayerAnim(id,PUNCHS1,7,0);
			PlaySound(0,0);
			playerc.combo_t=0;
			playerc.state=0;
		}
	}
	
	if(playerc.state & 4)
	{
		if(!CheckAnim(id,JUMPING))
			SetPlayerAnim(id,JUMP,1,1);

		if(st.Current_Map.sprites[id].body.velxy.y>0)
		{
			playerc.state+=4;
			SetPlayerAnim(id, FALLING, 5, 0);
		}
		
	}
	
	if (playerc.state & 8)
		if (OnTheGround(id, NULL) || HitSprite(id))
			playerc.state -= 8;

	tmp=MAnim(playerc.current_anim,playerc.speed,id,playerc.anim_loop);

	if(tmp==1 && ~playerc.state & 4 && ~playerc.state & 8)
	{
		st.Current_Map.sprites[id].body.velxy.x = 0;
		SetPlayerAnim(id,STAND,2,1);
	}

	if((playerc.current_anim==WALK || playerc.current_anim==WALKB) && !st.keys[LEFT_KEY].state && !st.keys[RIGHT_KEY].state && ~playerc.state & 4 && ~playerc.state & 8)
	{
		st.Current_Map.sprites[id].body.velxy.x = 0;
		SetPlayerAnim(id,STAND,2,1);
	}

	if(st.Current_Map.cam_area.horiz_lim)
	{
		if(st.Current_Map.sprites[id].position.x+(st.Current_Map.sprites[id].body.size.x/2)>=st.Current_Map.cam_area.limit[1].x)
			st.Current_Map.sprites[id].position.x=st.Current_Map.cam_area.limit[1].x-(st.Current_Map.sprites[id].body.size.x/2);

		if(st.Current_Map.sprites[id].position.x-(st.Current_Map.sprites[id].body.size.x/2)<=st.Current_Map.cam_area.limit[0].x)
			st.Current_Map.sprites[id].position.x=st.Current_Map.cam_area.limit[0].x+(st.Current_Map.sprites[id].body.size.x/2);
	}
	
}

void SpawnPlayer(Pos pos, Pos size, int16 ang)
{
	int16 i=st.Current_Map.num_sprites, j;

	st.Current_Map.sprites[i].GameID=PLAYER1;
	st.Current_Map.sprites[i].position=pos;
	st.Current_Map.sprites[i].angle=ang;
	st.Current_Map.sprites[i].health=100;
	st.Current_Map.sprites[i].color.r=st.Current_Map.sprites[i].color.g=st.Current_Map.sprites[i].color.b=st.Current_Map.sprites[i].color.a=255;
	st.Current_Map.sprites[i].type_s=MIDGROUND;
	playerc.state=0;

	st.Current_Map.sprites[i].stat=1;

	for(j=0;j<st.num_sprites;j++)
	{
		if(j==GLACIUS)
		{
			st.Current_Map.sprites[i].frame_ID=st.Current_Map.sprites[i].current_frame=st.Game_Sprites[j].frame[0];
			st.Current_Map.sprites[i].MGG_ID=st.Game_Sprites[j].MGG_ID;
			st.Current_Map.sprites[i].body.size.x=st.Game_Sprites[j].body.size.x;
			st.Current_Map.sprites[i].body.size.y=st.Game_Sprites[j].body.size.y;
			st.Current_Map.sprites[i].size_a.x=st.Game_Sprites[j].size_a.x;
			st.Current_Map.sprites[i].size_a.y=st.Game_Sprites[j].size_a.y;
			st.Current_Map.sprites[i].flags=st.Game_Sprites[j].flags;
			st.Current_Map.sprites[i].size_m.x=st.Game_Sprites[j].size_m.x+1;
			st.Current_Map.sprites[i].size_m.y=st.Game_Sprites[j].size_m.y+1;

			playerc.i=i;

			playerc.hit[0].anim=PUNCHM1;
			playerc.hit[0].speed=6;

			playerc.hit[1].anim=PUNCHM2;
			playerc.hit[1].speed=5;

			playerc.hit[2].anim=PUNCHS1;
			playerc.hit[2].speed=5;

			playerc.hit[3].anim=PUNCHS2;
			playerc.hit[3].speed=5;

			playerc.hit[4].anim=PUNCHS3;
			playerc.hit[4].speed=5;

			playerc.hit[5].anim=KICKS;
			playerc.hit[5].speed=5;

			playerc.hit[6].anim=KICKM;
			playerc.hit[6].speed=5;
			
			playerc.hit[7].anim=CPUNCHS;
			playerc.hit[7].speed=5;

			playerc.hit[8].anim=CPUNCHM;
			playerc.hit[8].speed=5;

			playerc.hit[9].anim=CKICKS;
			playerc.hit[9].speed=5;

			playerc.hit[10].anim=CKICKM;
			playerc.hit[10].speed=5;

			playerc.hit[11].anim=JPUNCHM;
			playerc.hit[11].speed=5;

			playerc.hit[12].anim=JPUNCHS;
			playerc.hit[12].speed=5;

			playerc.hit[13].anim=JKICKS;
			playerc.hit[13].speed=5;

			playerc.hit[14].anim=JKICKM;
			playerc.hit[14].speed=5;

			AddDynamicSprite(i);
			

			break;
		}
	}

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

			//SpawnSprite(GLACIUS,st.Current_Map.sprites[i].position,st.Current_Map.sprites[3].body.size,0);
			//st.Current_Map.sprites[i].MGG_ID=3;
			//st.Current_Map.sprites[i].frame_ID=1;
			//st.Current_Map.sprites[i].current_frame=1;

			starter=1;
		}
		else
		if (st.Current_Map.sprites[i].GameID == SEKTOR)
		{
			AddDynamicSprite(i);
		}
		else
		if(st.Current_Map.sprites[i].GameID==MUSICTRACK)
		{
			for(j=0;j<st.Current_Map.sprites[i].num_tags;j++)
			{
				if(strcmp(st.Game_Sprites[MUSICTRACK].tag_names[j],"MUSFX")==NULL)
					PlayMusic(st.Current_Map.sprites[i].tags[j],1);
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

void SparkPart(PARTICLES part)
{
	
}

int main(int argc, char *argv[])
{
	int loops;
	int prev_tic, curr_tic, delta, mx, my;

	Pos mp, ms, mp2, ms2;

	st.FPSYes=1;

	PreInit("mgear", argc, argv);

	if(LoadCFG()==0)
		if(MessageBox(NULL,L"Error while trying to read or write the configuration file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();

	strcpy(st.LogName, "mgear.log");

	Init();

	strcpy(st.WindowTitle,"mGear Test Demo");

	OpenFont("Font/Roboto-Regular.ttf", "arial", 0, 128);

	InitMGG();

	if (LoadMGG(&mgg_sys[0], "data/mEngUI.mgg") == NULL)
	{
		LogApp("Could not open UI mgg");
		Quit();
	}

	UILoadSystem("UI_Sys.cfg");

	if (argc > 0)
	{
		for (uint8 i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], "-p") == NULL)
				strcpy(prj_path, argv[i + 1]);
		}
	}

	SetCurrentDirectory(prj_path);

	memset(st.Game_Sprites,0,MAX_SPRITES*sizeof(_SPRITES));
	st.num_sprites=0;

	BASICBKD(255,255,255);
	DrawString2UI("Loading sprites...",8192,4096,0,0,0,255,255,255,255,ARIAL,2048,2048,6);

	Renderer(1);

	LoadSpriteList("sprite.slist");

	//LoadMGGList("mgg.list");

	LoadSoundList("Data/Audio/sound.list");

	LoadMGG(&mgg_sys[1], "Data/Textures/Menu.mgg");

	OpenFont("UI/Fonts/Bebas-Neue.otf", "Bebas", 1, 128);

	st.gt=MAIN_MENU;
	st.viewmode = 31 + 64;

	curr_tic=GetTicks();
	delta=1;

	st.Developer_Mode = 1;

	InitPhysics(1, DEFAULT_GRAVITY, 900, 0);

	DefineProjectileProperties(0, PROJ_REGULAR | PROJ_SPAWNS_SPRITE | PROJ_TRAIL, 15, 0, 0, 1, 1, GLACIUS);
	DefineProjectilePhysics(0, 0, 0, 0, 0, 50, 2048, 1024,64);
	DefineProjectileVisual(0, 2, 5, 0, 0, 0);

	DefineParticle(0, 0, 5, 0, 0, 0, 64, SparkPart);

	SETENGINEPATH;

	BuildMGL("mgl_test.m", "mgl_test.mgc");

	InitMGLCode("mgl_test.mgc");
	ExecuteMGLCode(0);

	PlayMovie("Data/Movies/SIB_Logo.mgv");
	PlayMovie("Data/Movies/Intro.mgv");

	PlayMusic(3, 1);
	SDL_Delay(1000);
	//int8 ExecuteMGLCode(2);

	while(!st.quit)
	{
		if(st.FPSYes)
			FPSCounter();

		while (PollEvent(&events))
		{
			WindowEvents();
		}

		loops=0;
		while(GetTicks() > curr_tic && loops < 10)
		{
			Finish();

			if (st.gt == MAIN_MENU)
				Menu();
			else
			if(st.gt==INGAME)
			{
				GameInput();
				GameEvent();
				LockCamera();
			}

			curr_tic+=1000/TICSPERSECOND;
			loops++;
			SetTimerM(1);

			MainPhysics();

			MainSound();

		}

		DrawSys();

		if(st.gt==INGAME)
		{
			//BASICBKD(st.Current_Map.amb_color.r,st.Current_Map.amb_color.g,st.Current_Map.amb_color.b);
			DrawMap();
			DrawMisc();
		}

		UIMain_DrawSystem();

		Renderer(0);

		SwapBuffer(wn);
	}

	StopAllSounds();
	Quit();

	return 0;
}