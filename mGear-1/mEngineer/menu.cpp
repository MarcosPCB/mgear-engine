#include "types.h"
#include "main.h"
#include "input.h"
#include "dirent.h"

int16 scroll=0;

void Menu()
{
	char files[512][512];
	uint16 num_files=0, i=0;
	FILE *f;
	DIR *dir;
	char *path2;
	size_t size;

	if(st.gt==MAIN_MENU || st.gt==GAME_MENU)
	{
		if(meng.menu_sel==0)
		{
			if(st.keys[ESC_KEY].state && st.gt==GAME_MENU)
			{
				st.gt=INGAME;
				st.keys[ESC_KEY].state=0;
			}

			if(CheckColisionMouse(400,200,200,50,0))
			{
				DrawStringUI("Start New Map",400,200,200,50,0,255,128,32,1,st.fonts[GEOMET].font);
				if(st.mouse1)
				{
					meng.scroll=0;
					meng.tex_selection=-1;
					meng.command2=0;
					meng.scroll2=0;
					meng.mgg_sel=MGG_MAP_START;

					if(st.Current_Map.obj)
						free(st.Current_Map.obj);

					if(st.Current_Map.sprites)
						free(st.Current_Map.sprites);

					if(st.Current_Map.sector)
						free(st.Current_Map.sector);

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

					st.gt=INGAME;

					st.Camera.position.x=0;
					st.Camera.position.y=0;

					meng.pannel_choice=2;
					meng.command=2;

				}
			}
			else
				DrawStringUI("Start New Map",400,200,200,50,0,255,255,255,1,st.fonts[GEOMET].font);

			if(st.gt==GAME_MENU) 
			{
				if(CheckColisionMouse(400,150,150,50,0))
				{
					DrawStringUI("Save Map",400,150,150,50,0,255,128,32,1,st.fonts[GEOMET].font);
					if(st.mouse1)
					{
						meng.menu_sel=1;
						st.mouse1=0;
					}
				}
				else
					DrawStringUI("Save Map",400,150,150,50,0,255,255,255,1,st.fonts[GEOMET].font);
			}

			if(CheckColisionMouse(400,250,150,50,0))
			{
				DrawStringUI("Load Map",400,250,150,50,0,255,128,32,1,st.fonts[GEOMET].font);
				if(st.mouse1)
				{
					meng.menu_sel=2;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Load Map",400,250,150,50,0,255,255,255,1,st.fonts[GEOMET].font);

			if(CheckColisionMouse(400,300,100,50,0))
			{
				DrawStringUI("Options",400,300,100,50,0,255,128,32,1,st.fonts[GEOMET].font);
				if(st.mouse1)
				{
					meng.menu_sel=3;
					st.mouse1=0;
				}
			}
			else
				DrawStringUI("Options",400,300,100,50,0,255,255,255,1,st.fonts[GEOMET].font);

			if(CheckColisionMouse(400,350,50,50,0))
			{
				DrawStringUI("Quit",400,350,50,50,0,255,128,32,1,st.fonts[GEOMET].font);
				if(st.mouse1)
					Quit();
			}
			else
				DrawStringUI("Quit",400,350,50,50,0,255,255,255,1,st.fonts[GEOMET].font);
		}
		else
		if(meng.menu_sel==2)
		{
			num_files=DirFiles(meng.path,files);

			i=0;

			if(num_files>0)
			{
				for(uint16 j=25;j<8000;j+=50)
				{
					if(i==num_files) break;

					if(CheckColisionMouse(400,j+scroll,300,50,0))
					{
						DrawString2UI(files[i],400,j+scroll,0.5,0.5,0,255,128,32,1,st.fonts[ARIAL].font);
						if(st.mouse1)
						{
							size=strlen(files[i]);
							size+=4;
							size+=strlen(meng.path);
							path2=(char*) malloc(size);
							strcpy(path2,meng.path);
							strcat(path2,"//");
							strcat(path2,files[i]);
							//Check if it's a file
							if((f=fopen(path2,"rb"))==NULL)
							{
								//Check if it's a directory
								if((dir=opendir(path2))==NULL)
								{
									LogApp("Error invalid file or directory: %s",files[i]);
									st.mouse1=0;
									i++;
									free(path2);
									continue;
								}
								else
								{
									meng.path=(char*) realloc(meng.path,size);
									strcpy(meng.path,path2);
									closedir(dir);
									scroll=0;
									st.mouse1=0;
									free(path2);
									break;
								}
							}
							else
							{
								fclose(f);
								if(LoadMap(path2))
								{
									LogApp("Map %s loaded",path2);
									st.Camera.position.x=0;
									st.Camera.position.y=0;
									meng.scroll=0;
									meng.tex_selection=-1;
									meng.command2=0;
									meng.scroll2=0;
									meng.mgg_sel=MGG_MAP_START;
									meng.pannel_choice=2;
									meng.command=2;
									meng.menu_sel=0;
									st.gt=INGAME;
									st.mouse1=0;
									free(path2);
									free(meng.path);
									meng.path=(char*) malloc(2);
									strcpy(meng.path,".");
									scroll=0;
									break;
								}
								else
								{
									st.mouse1=0;
									i++;
									free(path2);
									continue;
								}
							}
						}
					}
					else
					{
						DrawString2UI(files[i],400,j+scroll,0.5,0.5,0,255,255,255,1,st.fonts[ARIAL].font);
					}

					i++;
				}

				if(st.mouse_wheel>0)
				{
					if(scroll<0) scroll+=50;
					st.mouse_wheel=0;
				}

				if(st.mouse_wheel<0)
				{
					scroll-=50;
					st.mouse_wheel=0;
				}
			}

			if(st.keys[ESC_KEY].state)
			{
				meng.menu_sel=0;
				free(meng.path);
				meng.path=(char*) malloc(2);
				strcpy(meng.path,".");
				st.keys[ESC_KEY].state=0;
			}
		}
	}
}

