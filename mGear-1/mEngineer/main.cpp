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
	DrawHud(400,300,250,600,0,255,255,255,0,0,1,1,mgg[0].frames[4],1);
	if(meng.num_mgg>0)
	{
		for(uint16 i=25;i<8000;i+=50)
		{
			if(j==meng.num_mgg)
				break;
			else
			{
				if(!CheckColisionMouse(400,i+meng.scroll2,150,50,0))
				{
					DrawString2UI(meng.mgg_list[j],400,i+meng.scroll2,0.5,0.5,0,255,128,32,1,st.fonts[ARIAL].font);
				}
				else
				{
					DrawString2UI(meng.mgg_list[j],400,i+meng.scroll2,0.5,0.5,0,255,32,0,1,st.fonts[ARIAL].font);
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
			if(meng.scroll2<0) meng.scroll2+=50;
			st.mouse_wheel=0;
		}

		if(st.mouse_wheel<0)
		{
			meng.scroll2-=50;
			st.mouse_wheel=0;
		}
	}
	else
		meng.command=meng.pannel_choice;
}

void ImageList(uint8 id)
{
	uint16 m=0;

	for(uint16 i=80;i<160000;i+=160)
	{
		if(m>=mgg[id].num_frames) break;
		for(uint16 j=80;j<800;j+=160)
		{
			if(m<mgg[id].num_frames)
			{
				if((CheckColisionMouse(j,i+meng.scroll,160,160,0) && st.mouse1) || meng.tex_selection==mgg[id].frames[m])
				{
					DrawHud(j,i+meng.scroll,160,160,0,255,128,32,0,0,1,1,mgg[id].frames[m],1);
					meng.tex_selection=mgg[id].frames[m];
					meng.tex_ID=m;
					meng.tex_MGGID=id;
				}
				else
				{
					DrawHud(j,i+meng.scroll,160,160,0,255,255,255,0,0,1,1,mgg[id].frames[m],1);
				}

				m++;
			}
			else break;
		}
	}

	if(st.mouse_wheel>0)
	{
		if(meng.scroll<0) meng.scroll+=160;
		st.mouse_wheel=0;
	}

	if(st.mouse_wheel<0)
	{
		meng.scroll-=160;
		st.mouse_wheel=0;
	}
}

static int16 MGGLoad()
{
	uint16 j=0;
	int16 num_files;
	FILE *f;
	DIR *dir;

	int8 id, loaded=0;
	uint16 id2=meng.num_mgg, id3=0;

	char files[512][512];
	char *path2;

	size_t size;

	for(uint8 i=3;i<MAX_MGG;i++)
	{
		if(i==MAX_MGG-1 && mgg[i].type!=NONE)
		{
			LogApp("Cannot load MGG, reached max number of MGG loaded");
			return 0;
		}

		if(mgg[i].type==NONE)
		{
			id=i;
			break;
		}
	}

	num_files=DirFiles(meng.path,files);

	for(uint16 i=25;i<8000;i+=50)
	{
		if(j==num_files) break;

		if(CheckColisionMouse(400,i+meng.scroll,300,50,0))
		{
			DrawString2UI(files[j],400,i+meng.scroll,0.5,0.5,0,255,128,32,1,st.fonts[ARIAL].font);

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
						DrawUI(400,300,800,600,0,0,0,0,0,0,1,1,mgg[0].frames[4],1);
						DrawString2UI("Loading...",400,300,1,1,1,255,255,255,1,st.fonts[GEOMET].font);
						Renderer();
						LoadMGG(&mgg[id],path2);

						for(uint16 u=0;u<meng.num_mgg;u++)
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
							strcpy(meng.mgg_list[meng.num_mgg],mgg[id].name);
							meng.num_mgg++;
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
			DrawString2UI(files[j],400,i+meng.scroll,0.5,0.5,0,255,255,255,1,st.fonts[ARIAL].font);
		}
		j++;

		if(st.mouse_wheel>0)
		{
			if(meng.scroll<0) meng.scroll+=50;
			st.mouse_wheel=0;
		}

		if(st.mouse_wheel<0)
		{
			meng.scroll-=50;
			st.mouse_wheel=0;
		}
	}
	
	
	if(st.keys[ESC_KEY].state)
	{
		free(meng.path);
		meng.path=(char*) malloc(2);
		strcpy(meng.path,".");

		if(path2) free(path2);

		meng.command=meng.pannel_choice;
		st.keys[ESC_KEY].state=0;
	}

	return 1;
}

static void PannelLeft()
{
	uint8 mouse=0;
	char num[32];

	DrawHud(50,300,100,600,0,255,255,255,0,0,1,1,mgg[0].frames[4],1);

	if(!CheckColisionMouse(27,27,48,48,0))
	{
		DrawHud(27,27,48,48,0,255,255,255,0,0,1,1,mgg[0].frames[0],1);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawHud(27,27,48,48,0,255,128,32,0,0,1,1,mgg[0].frames[0],1);
		//time++;
		//if((st.time-time)==1000)
		//{
			DrawString("Draw a sector",400,550,200,50,0,255,128,32,1,st.fonts[ARIAL].font);
		//}

			if(st.mouse1) meng.command=meng.pannel_choice=0;
	}

	if(!CheckColisionMouse(75,27,48,48,0))
	{
		DrawHud(75,27,48,48,0,255,255,255,0,0,1,1,mgg[0].frames[2],1);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawHud(75,27,48,48,0,255,128,32,0,0,1,1,mgg[0].frames[2],1);
		//if((st.time-time)==1000)
		//{
				DrawString("Select and edit",400,550,200,50,0,255,128,32,1,st.fonts[ARIAL].font);
		//}

		if(st.mouse1) meng.command=meng.pannel_choice=2;
	}
	
	if(!CheckColisionMouse(27,75,48,48,0))
	{
		DrawHud(27,75,48,48,0,255,255,255,0,0,1,1,mgg[0].frames[1],1);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawHud(27,75,48,48,0,255,128,32,0,0,1,1,mgg[0].frames[1],1);
		//if((st.time-time)==1000)
		//{
			DrawString("Add an OBJ",400,550,200,50,0,255,128,32,1,st.fonts[ARIAL].font);
		//}

		if(st.mouse1) meng.command=meng.pannel_choice=3;
	}
	
	if(!CheckColisionMouse(75,75,48,48,0))
	{
		DrawHud(75,75,48,48,0,255,255,255,0,0,1,1,mgg[0].frames[3],1);
		//if(st.mouse1) mouse=0;
	}
	else
	{
		DrawHud(75,75,48,48,0,255,128,32,0,0,1,1,mgg[0].frames[3],1);
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

		if(st.mouse1)
		{
			meng.scroll=0;
			meng.command=5;
		}
	}

	if(!CheckColisionMouse(51,171,60,24,0))
	{
		DrawString("MGG Sel.",51,171,60,24,0,255,255,255,1,st.fonts[ARIAL].font);
	}
	else
	{
		DrawString("MGG Sel.",51,171,60,24,0,255,128,32,1,st.fonts[ARIAL].font);

		DrawString("MGG Selection",400,550,300,50,0,255,128,32,1,st.fonts[ARIAL].font);

		if(st.mouse1)
		{
			meng.scroll2=0;
			meng.command=6;
		}
	}

	if(!CheckColisionMouse(51,575,60,24,0))
	{
		DrawString("Load MGG",51,575,60,24,0,255,255,255,1,st.fonts[ARIAL].font);
	}
	else
	{
		DrawString("Load MGG",51,575,60,24,0,255,128,32,1,st.fonts[ARIAL].font);

		DrawString("Load an MGG file and adds it to the map list",400,550,600,50,0,255,128,32,1,st.fonts[ARIAL].font);

		if(st.mouse1)
		{
			meng.scroll2=0;
			meng.command=7;
		}
	}

	sprintf(num,"%.2f",st.Camera.dimension.x);
	DrawString2UI(num,51,219,0.2,0.2,0,255,255,255,1,st.fonts[ARIAL].font);
	sprintf(num,"%.2f",st.Camera.dimension.y);
	DrawString2UI(num,51,267,0.2,0.2,0,255,255,255,1,st.fonts[ARIAL].font);

	sprintf(num,"%.2f",st.Camera.position.x);
	DrawString2UI(num,51,315,0.2,0.2,0,255,255,255,1,st.fonts[ARIAL].font);
	sprintf(num,"%.2f",st.Camera.position.y);
	DrawString2UI(num,51,363,0.2,0.2,0,255,255,255,1,st.fonts[ARIAL].font);

	DrawHud(50,500,100,100,0,255,255,255,0,0,1,1,meng.tex_selection,1);

	if(meng.pannel_choice==0)
		DrawHud(27,27,48,48,0,128,32,32,0,0,1,1,mgg[0].frames[0],1);
	//else
	if(meng.pannel_choice==2)
		DrawHud(75,27,48,48,0,128,32,32,0,0,1,1,mgg[0].frames[2],1);
	//else
	if(meng.pannel_choice==4)
		DrawHud(75,75,48,48,0,128,32,32,0,0,1,1,mgg[0].frames[3],1);
	//else
	if(meng.pannel_choice==3)
		DrawHud(27,75,48,48,0,128,32,32,0,0,1,1,mgg[0].frames[1],1);

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
				for(uint16 i=0;i<st.Current_Map.num_sector;i++)
				{
					for(uint16 j=0;j<5;j++)
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

					if(got_it) break;
					
				}
			}
		}
	}

	if(st.keys[UP_KEY].state)
	{
		st.Camera.position.y-=100;
	}

	if(st.keys[DOWN_KEY].state)
	{
		st.Camera.position.y+=100;
	}

	if(st.keys[RIGHT_KEY].state)
	{
		st.Camera.position.x+=100;
	}

	if(st.keys[LEFT_KEY].state)
	{
		st.Camera.position.x-=100;
	}

	if(st.mouse_wheel>0)
	{
		if(st.Camera.dimension.x<2) st.Camera.dimension.x+=0.1;
		if(st.Camera.dimension.y<2) st.Camera.dimension.y+=0.1;
		st.mouse_wheel=0;
	}

	if(st.mouse_wheel<0)
	{
		if(st.Camera.dimension.x>0.4) st.Camera.dimension.x-=0.1;
		if(st.Camera.dimension.y>0.4) st.Camera.dimension.y-=0.1;
		st.mouse_wheel=0;
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

	st.FPSYes=1;

	st.gt=MAIN_MENU;

	meng.pannel_choice=2;
	meng.command=2;

	st.Developer_Mode=1;

	meng.menu_sel=0;

	meng.path=(char*) malloc(2);
	strcpy(meng.path,".");

	while(!st.quit)
	{
		if(st.FPSYes)
			FPSCounter();

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

				if(st.keys[ESC_KEY].state)
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
		Timer();
	}

	StopAllSounds();
	Quit();
	return 1;
}