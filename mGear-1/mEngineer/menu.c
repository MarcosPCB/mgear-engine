#include "types.h"
#include "main.h"
#include "input.h"
#include "dirent.h"

int16 scroll=0;

void Menu()
{
	char files[512][512];
	uint16 num_files=0, i=0, j, a;
	FILE *f;
	DIR *dir;
	char *path2;
	size_t size;
	int8 id=0;
	uint32 id2=0;
	char lo[32];

	if(st.gt==MAIN_MENU || st.gt==GAME_MENU)
	{
		if(meng.menu_sel==0)
		{
			if(st.keys[ESC_KEY].state && st.gt==GAME_MENU)
			{
				st.gt=INGAME;
				st.keys[ESC_KEY].state=0;
			}

			if(CheckColisionMouse(8192,4096-1820,1820,455,0))
			{
				DrawStringUI("Start New Map",8192,4096-1820,1820,455,0,255,128,32,255,st.fonts[GEOMET].font,0,0,0);
				if(st.mouse1)
				{
					meng.scroll=0;
					meng.tex_selection.data=-1;
					meng.command2=0;
					meng.scroll2=0;
					meng.mgg_sel=0;

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

					for(i=0;i<MAX_SECTORS;i++)
					{
						st.Current_Map.sector[i].id=-1;
						st.Current_Map.sector[i].layers=1;
						st.Current_Map.sector[i].material=CONCRETE;
						st.Current_Map.sector[i].tag=0;
					}

					for(i=0;i<MAX_OBJS;i++)
					{
						st.Current_Map.obj[i].type=BLANK;
						st.Current_Map.obj[i].lightmapid=-1;
					}

					for(i=0;i<MAX_SPRITES;i++)
						st.Current_Map.sprites[i].stat=0;;

					memset(st.Current_Map.MGG_FILES,0,32*256);
					meng.num_mgg-=st.Current_Map.num_mgg;
					st.Current_Map.num_mgg=0;

					memset(meng.mgg_list,0,64*256);

					st.gt=INGAME;

					st.Camera.position.x=0;
					st.Camera.position.y=0;

					meng.pannel_choice=2;
					meng.command=2;

					memset(&meng.spr,0,sizeof(meng.spr));

					meng.obj.amblight=1;
					meng.obj.color.r=meng.spr.color.r=255;
					meng.obj.color.g=meng.spr.color.g=255;
					meng.obj.color.b=meng.spr.color.b=255;
					meng.obj.color.a=meng.spr.color.a=255;
					meng.obj.texsize.x=32768;
					meng.obj.texsize.y=32768;
					meng.obj.texpan.x=0;
					meng.obj.texpan.y=0;
					meng.obj.type=meng.spr.type=MIDGROUND;
					meng.obj_lightmap_sel=-1;

					meng.spr.gid=-1;
					meng.spr2.gid=-1;
					meng.sprite_selection=0;
					meng.sprite_frame_selection=0;
					meng.spr.size.x=2048;
					meng.spr.size.y=2048;

					meng.lightmap_res.x=meng.lightmap_res.y=256;

				}
			}
			else
				DrawString2UI("Start New Map",8192,(4096)-1820,1820,455,0,255,255,255,255,st.fonts[GEOMET].font,0,0,0);

			if(st.gt==GAME_MENU) 
			{
				if(CheckColisionMouse(8192,(4096)-2275,1365,455,0))
				{
					DrawStringUI("Save Map",8192,(4096)-2275,1365,455,0,255,128,32,255,st.fonts[GEOMET].font,0,0,0);
					if(st.mouse1)
					{
						meng.menu_sel=1;
						st.mouse1=0;
					}
				}
				else
					DrawString2UI("Save Map",8192,(4096)-2275,1365,455,0,255,255,255,255,st.fonts[GEOMET].font,0,0,0);
			}

			if(CheckColisionMouse(8192,(4096)-1365,1365,455,0))
			{
				DrawStringUI("Load Map",8192,(4096)-1365,1365,455,0,255,128,32,255,st.fonts[GEOMET].font,0,0,0);
				if(st.mouse1)
				{
					meng.menu_sel=2;
					st.mouse1=0;
				}
			}
			else
				DrawString2UI("Load Map",8192,(4096)-1365,1365,455,0,255,255,255,255,st.fonts[GEOMET].font,0,0,0);

			if(CheckColisionMouse(8192,(4096)-910,910,455,0))
			{
				DrawStringUI("Options",8192,(4096)-910,910,455,0,255,128,32,255,st.fonts[GEOMET].font,0,0,0);
				if(st.mouse1)
				{
					meng.menu_sel=3;
					st.mouse1=0;
				}
			}
			else
				DrawString2UI("Options",8192,(4096)-910,910,455,0,255,255,255,255,st.fonts[GEOMET].font,0,0,0);

			if(CheckColisionMouse(8192,(4096)-455,455,455,0))
			{
				DrawStringUI("Quit",8192,(4096)-455,455,455,0,255,128,32,255,st.fonts[GEOMET].font,0,0,0);
				if(st.mouse1)
					Quit();
			}
			else
				DrawString2UI("Quit",8192,(4096)-455,455,455,0,255,255,255,255,st.fonts[GEOMET].font,0,0,0);
		}
		else
		if(meng.menu_sel==2)
		{
			num_files=DirFiles(meng.path,files);

			i=0;

			if(num_files>0)
			{
				for(j=227;j<227*20;j+=454)
				{
					if(i==num_files) break;

					if(CheckColisionMouse(8192,j+scroll,2730,455,0))
					{
						DrawString2UI(files[i],8192,j+scroll,0,0,0,255,128,32,255,st.fonts[GEOMET].font,2048,2048,0);
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
									memset(meng.mgg_list,0,64*256);
									meng.num_mgg=0;
									LogApp("Map %s loaded",path2);

									id=0;
									id2=0;

									for(a=0;a<st.Current_Map.num_mgg;a++)
									{
										DrawUI(8192,4096,16384,8192,0,0,0,0,0,0,TEX_PAN_RANGE,TEX_PAN_RANGE,mgg_sys[0].frames[4],255,0);
										sprintf(lo,"Loading %d%",(a/st.Current_Map.num_mgg)*100);
										DrawString2UI(lo,8192,4096,1,1,0,255,255,255,255,st.fonts[GEOMET].font,FONT_SIZE*2,FONT_SIZE*2,0);
										if(CheckMGGFile(st.Current_Map.MGG_FILES[a]))
										{
											LoadMGG(&mgg_map[id],st.Current_Map.MGG_FILES[a]);
											strcpy(meng.mgg_list[a],mgg_map[id].name);
											meng.num_mgg++;
											id++;
										}

									}

									st.Camera.position.x=0;
									st.Camera.position.y=0;
									meng.scroll=0;
									meng.tex_selection.data=-1;
									meng.command2=0;
									meng.scroll2=0;
									meng.mgg_sel=0;
									meng.pannel_choice=2;
									meng.command=2;
									meng.menu_sel=0;
									meng.obj.amblight=1;
									meng.obj.color.r=255;
									meng.obj.color.g=255;
									meng.obj.color.b=255;
									meng.obj.color.a=255;
									meng.obj.texsize.x=32768;
									meng.obj.texsize.y=32768;
									meng.obj.texpan.x=0;
									meng.obj.texpan.y=0;
									meng.obj.type=MIDGROUND;
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
						DrawString2UI(files[i],8192,j+scroll,0,0,0,255,255,255,255,st.fonts[GEOMET].font,2048,2048,0);
					}

					i++;
				}

				if(st.mouse_wheel>0)
				{
					if(scroll<0) scroll+=454;
					st.mouse_wheel=0;
				}

				if(st.mouse_wheel<0)
				{
					scroll-=454;
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

