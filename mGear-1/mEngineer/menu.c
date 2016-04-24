#include "types.h"
#include "main.h"
#include "input.h"
#include "dirent.h"
#include "UI.h"
#include "mggeditor.h"

int16 scroll=0;

void Menu()
{
	char files[512][512];
	uint16 num_files=0, i=0, j, a, m, n;
	FILE *f;
	DIR *dir;
	char path2[2048];
	size_t size;
	int8 id=0;
	uint32 id2=0;
	char lo[32];
	static int8 winid;

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
				StringUIData("Start New Map",8192,4096-1820,1820,455,0,255,128,32,255,ARIAL,0,0,0);
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

					if(st.num_lights>0)
					{
						for(i=1;i<=st.num_lights;i++)
						{
							free(st.game_lightmaps[i].data);
							st.game_lightmaps[i].obj_id=-1;
							st.game_lightmaps[i].stat=0;
							glDeleteTextures(1,&st.game_lightmaps[i].tex);
						}
					}

					st.Current_Map.obj=(_MGMOBJ*) malloc(MAX_OBJS*sizeof(_MGMOBJ));
					st.Current_Map.sector=(_SECTOR*) malloc(MAX_SECTORS*sizeof(_SECTOR));
					st.Current_Map.sprites=(_MGMSPRITE*) malloc(MAX_SPRITES*sizeof(_MGMSPRITE));

					st.Current_Map.num_sector=0;
					st.Current_Map.num_obj=0;
					st.Current_Map.num_sprites=0;
					st.Current_Map.num_lights=0;

					st.num_lights=0;

					for(i=0;i<MAX_SECTORS;i++)
					{
						st.Current_Map.sector[i].id=-1;
						///st.Current_Map.sector[i].layers=1;
						st.Current_Map.sector[i].material=CONCRETE;
						st.Current_Map.sector[i].tag=0;
					}

					for(i=0;i<MAX_OBJS;i++)
					{
						st.Current_Map.obj[i].type=BLANK;
						st.Current_Map.obj[i].lightmapid=-1;
					}

					for(i=0;i<MAX_SPRITES;i++)
						st.Current_Map.sprites[i].stat=0;

					if(st.Current_Map.num_mgg>0)
					{
						for(i=0;i<st.Current_Map.num_mgg;i++)
							FreeMGG(&mgg_map[i]);
					}

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

					meng.lightmapsize.x=0;
					meng.lightmapsize.y=0;

					meng.spr.gid=-1;
					meng.spr2.gid=-1;
					meng.sprite_selection=0;
					meng.sprite_frame_selection=0;
					meng.spr.size.x=2048;
					meng.spr.size.y=2048;

					meng.playing_sound=0;

					meng.lightmap_res.x=meng.lightmap_res.y=256;

					st.Current_Map.bck1_v=BCK1_DEFAULT_VEL;
					st.Current_Map.bck2_v=BCK2_DEFAULT_VEL;
					st.Current_Map.fr_v=FR_DEFAULT_VEL;
					st.Current_Map.bcktex_id=-1;
					st.Current_Map.bcktex_mgg=0;
					memset(&st.Current_Map.amb_color,255,sizeof(Color));

					meng.viewmode=7;

					memset(meng.z_buffer,0,2048*57*sizeof(int16));
					memset(meng.z_slot,0,57*sizeof(int16));
					meng.z_used=0;

					st.Current_Map.cam_area.area_pos.x=st.Current_Map.cam_area.area_pos.y=0;
					st.Current_Map.cam_area.area_size.x=1.0;
					st.Current_Map.cam_area.area_size.y=1.0;
					st.Current_Map.cam_area.horiz_lim=0;
					st.Current_Map.cam_area.vert_lim=0;
					st.Current_Map.cam_area.max_dim.x=6.0;
					st.Current_Map.cam_area.max_dim.y=6.0;
					st.Current_Map.cam_area.limit[0].x=0;
					st.Current_Map.cam_area.limit[1].x=16384;
					st.Current_Map.cam_area.limit[0].y=0;
					st.Current_Map.cam_area.limit[1].y=8192;

					st.Current_Map.bck3_size.x=st.Current_Map.bck3_size.y=TEX_PAN_RANGE;

				}
			}
			else
				StringUI2Data("Start New Map",8192,(4096)-1820,1820,455,0,255,255,255,255,ARIAL,0,0,0);

			if(st.gt==GAME_MENU) 
			{
				if(CheckColisionMouse(8192,(4096)-2275,1365,455,0))
				{
					StringUIData("Save Map",8192,(4096)-2275,1365,455,0,255,128,32,255,ARIAL,0,0,0);
					if(st.mouse1)
					{
						strcpy(UI_Sys.file_name,"newmap");
						meng.menu_sel=1;
						st.mouse1=0;
					}
				}
				else
					StringUI2Data("Save Map",8192,(4096)-2275,1365,455,0,255,255,255,255,ARIAL,0,0,0);
			}

			if(st.gt==MAIN_MENU)
			{
				if(UIStringButton(8192,4096-1365,"MGG Editor",ARIAL,2048,0,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
				{
					meng.menu_sel=4;
				}
			}

			if(CheckColisionMouse(8192,(4096)-910,1365,455,0))
			{
				StringUIData("Load Map",8192,(4096)-910,1365,455,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					meng.menu_sel=2;
					st.mouse1=0;
				}
			}
			else
				StringUI2Data("Load Map",8192,(4096)-910,1365,455,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckColisionMouse(8192,(4096)-455,910,455,0))
			{
				StringUIData("Options",8192,(4096)-455,910,455,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
				{
					winid=UICreateWindow2(0,0,CENTER,5,16,2048,64,ARIAL);
					meng.menu_sel=3;
					st.mouse1=0;
				}
			}
			else
				StringUI2Data("Options",8192,(4096)-455,910,455,0,255,255,255,255,ARIAL,0,0,0);

			if(CheckColisionMouse(8192,(4096),455,455,0))
			{
				StringUIData("Quit",8192,(4096),455,455,0,255,128,32,255,ARIAL,0,0,0);
				if(st.mouse1)
					Quit();
			}
			else
				StringUI2Data("Quit",8192,(4096),455,455,0,255,255,255,255,ARIAL,0,0,0);
		}
		else
		if(meng.menu_sel==2)
		{
			if(UISelectFile("mgm",path2))
			{
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
						DrawString2UI(lo,8192,4096,1,1,0,255,255,255,255,ARIAL,FONT_SIZE*2,FONT_SIZE*2,0);
						if(CheckMGGFile(st.Current_Map.MGG_FILES[a]))
						{
							LoadMGG(&mgg_map[id],st.Current_Map.MGG_FILES[a]);
							strcpy(meng.mgg_list[a],mgg_map[id].name);
							meng.num_mgg++;
							id++;
						}
						else
						{
							FreeMap();
							
							LogApp("Error while loading map's MGG: %s",st.Current_Map.MGG_FILES[a]);
							break;
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

									meng.lightmapsize.x=0;
									meng.lightmapsize.y=0;

									meng.spr.gid=-1;
									meng.spr2.gid=-1;
									meng.sprite_selection=0;
									meng.sprite_frame_selection=0;
									meng.spr.size.x=2048;
									meng.spr.size.y=2048;

									meng.playing_sound=0;

									meng.lightmap_res.x=meng.lightmap_res.y=256;
									st.gt=INGAME;
									st.mouse1=0;
									//free(path2);
									free(meng.path);
									meng.path=(char*) malloc(2);
									strcpy(meng.path,".");
									scroll=0;

									meng.viewmode=7;

									memset(meng.z_buffer,0,2048*57*sizeof(int16));
									memset(meng.z_slot,0,57*sizeof(int16));
									meng.z_used=0;

									for(m=0;m<st.Current_Map.num_obj;m++)
									{
										meng.z_buffer[st.Current_Map.obj[m].position.z][meng.z_slot[st.Current_Map.obj[m].position.z]]=m;
										meng.z_slot[st.Current_Map.obj[m].position.z]++;
										if(st.Current_Map.obj[m].position.z>meng.z_used)
											meng.z_used=st.Current_Map.obj[m].position.z;
									}
									
									for(m=0;m<st.Current_Map.num_sprites;m++)
									{
										meng.z_buffer[st.Current_Map.sprites[m].position.z][meng.z_slot[st.Current_Map.sprites[m].position.z]]=m+2000;
										meng.z_slot[st.Current_Map.sprites[m].position.z]++;
										if(st.Current_Map.sprites[m].position.z>meng.z_used)
											meng.z_used=st.Current_Map.sprites[m].position.z;
									}
									
									for(m=0;m<st.Current_Map.num_sector;m++)
									{
										meng.z_buffer[24][meng.z_slot[24]]=m+10000;
										meng.z_slot[24]++;
										if(24>meng.z_used)
											meng.z_used=24;
									}
									
									//break;
								}
							}
		}
		else
		if(meng.menu_sel==1)
		{
			if(UISavePath("mgm",path2))
			{
				SaveMap(path2);
				meng.menu_sel=0;
			}
		}
		else
		if(meng.menu_sel==3)
		{
			if(UIWin2_StringButton(winid,14,"Apply",UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
				RestartVideo();

			if(UIWin2_StringButton(winid,15,"Back",UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
			{
				UIDestroyWindow(winid);
				meng.menu_sel=0;
			}
		}
		else
		if(meng.menu_sel==4)
		{
			st.gt=INGAME;
			memset(&mged,0,sizeof(MGGEd));
			meng.editor=1;
		}
	}
}

