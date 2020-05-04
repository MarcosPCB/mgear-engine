#include "game.h"
#include "engine.h"
#include"UI.h"

void Menu()
{
	int16 i, j;

	static float a_anim = 0.00f, rgb_anim = 0.0f;

	PlayBGVideo("Data/Movies/Menu.mgv", 1);

	if (a_anim < 255.0f && rgb_anim < 255.0f)
	{
		a_anim += 0.5f;
		rgb_anim += 0.5f;
	}

	UIezData(2048 + 1536, 1536, 0.3f, 0, rgb_anim, rgb_anim, rgb_anim, mgg_sys[1].frames[0], a_anim, 0);

	if(UIStringButton(1024,4096-1024,"New Game",FIGHTFONT,6000,6,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
	{
		PlayBGVideo("Data/Movies/Menu.mgv", 0);
		StopMusic();
		LoadMap(StringFormat("%s/test.mgm", prj_path));

		for(i=0,j=0;i<st.Current_Map.num_mgg;i++)
		{
			DrawUI(8192,4096,16384,8192,0,0,0,0,0,0,32768,32768,mgg_sys[0].frames[4],255,6);
			DrawStringUI("Loading map...",8192,4096,0,0,0,255,255,255,255,FIGHTFONT,4096,4096,5);
			Renderer(1);
			if(CheckMGGFile(st.Current_Map.MGG_FILES[i]))
			{
				LoadMGG(&mgg_map[j],st.Current_Map.MGG_FILES[i]);
				j++;
			}
			else
			{
				FreeMap();

				LogApp("Error while loading map's MGG: %s",st.Current_Map.MGG_FILES[i]);
				break;
			}
		}

		st.Camera.position=st.Current_Map.cam_area.area_pos;

		st.Camera.dimension.x=st.Current_Map.cam_area.area_size.x;
		st.Camera.dimension.y=st.Current_Map.cam_area.area_size.y;

		st.gt=INGAME;

		PreGameEvent();
	}

	if (UIStringButton(1024, 4096, "Load Game", FIGHTFONT, 6000, 6, UI_COL_NORMAL, UI_COL_SELECTED) == UI_SEL)
	{
	}

	if (UIStringButton(1024, 4096 + 1024, "Options", FIGHTFONT, 6000, 6, UI_COL_NORMAL, UI_COL_SELECTED) == UI_SEL)
	{
	}

	if(UIStringButton(1024, 4096 + 2048,"Quit",FIGHTFONT,6000,6,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
		st.quit=1;
}

