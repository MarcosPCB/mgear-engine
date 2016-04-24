#include "game.h"
#include "engine.h"
#include"UI.h"

void Menu()
{
	int16 i, j;

	if(UIStringButton(8192,4096-1024,"New Game",FIGHTFONT,4096,6,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
	{
		LoadMap("STAGE02.MGM");

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

		st.Camera.position.x=0;
		st.Camera.position.y=0;

		st.Camera.dimension.x=st.Current_Map.cam_area.area_size.x;
		st.Camera.dimension.y=st.Current_Map.cam_area.area_size.y;

		st.gt=INGAME;

		PreGameEvent();
	}

	if(UIStringButton(8192,4096+1024,"Quit",FIGHTFONT,4096,6,UI_COL_NORMAL,UI_COL_SELECTED)==UI_SEL)
		st.quit=1;
}

