#include "types.h"
#include "main.h"

void Menu()
{
	char files[512][512];
	uint16 num_files=0, i=0;

	if(st.gt==MAIN_MENU || st.gt==GAME_MENU)
	{
		if(meng.menu_sel==0)
		{
			if(CheckColisionMouse(400,200,200,50,0))
			{
				DrawString("Start New Map",400,200,200,50,0,255,128,32,1,st.fonts[GEOMET].font);
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

					meng.pannel_choice=2;
					meng.command=2;

				}
			}
			else
				DrawString("Start New Map",400,200,200,50,0,255,255,255,1,st.fonts[GEOMET].font);

			if(st.gt==GAME_MENU) 
			{
				if(CheckColisionMouse(400,150,150,50,0))
				{
					DrawString("Save Map",400,150,150,50,0,255,128,32,1,st.fonts[GEOMET].font);
					if(st.mouse1)
						meng.menu_sel=1;
				}
				else
					DrawString("Save Map",400,150,150,50,0,255,255,255,1,st.fonts[GEOMET].font);
			}

			if(CheckColisionMouse(400,250,150,50,0))
			{
				DrawString("Load Map",400,250,150,50,0,255,128,32,1,st.fonts[GEOMET].font);
				if(st.mouse1)
					meng.menu_sel=2;
			}
			else
				DrawString("Load Map",400,250,150,50,0,255,255,255,1,st.fonts[GEOMET].font);

			if(CheckColisionMouse(400,300,100,50,0))
			{
				DrawString("Options",400,300,100,50,0,255,128,32,1,st.fonts[GEOMET].font);
				if(st.mouse1)
					meng.menu_sel=3;
			}
			else
				DrawString("Options",400,300,100,50,0,255,255,255,1,st.fonts[GEOMET].font);

			if(CheckColisionMouse(400,350,50,50,0))
			{
				DrawString("Quit",400,350,50,50,0,255,128,32,1,st.fonts[GEOMET].font);
				if(st.mouse1)
					Quit();
			}
			else
				DrawString("Quit",400,350,50,50,0,255,255,255,1,st.fonts[GEOMET].font);
		}
		else
		if(meng.menu_sel==2)
		{
			num_files=DirFiles(".",files);

			i=0;

			if(num_files>0)
			{
				for(uint16 j=25;j<600;j+=50)
				{
					if(i==num_files) break;

					DrawString2(files[i],400,j,0.5,0.5,0,255,255,255,1,st.fonts[ARIAL].font);
					i++;
				}
			}
		}
	}
}

