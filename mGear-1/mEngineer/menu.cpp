#include "types.h"
#include "main.h"

void Menu()
{
	if(st.gt==MAIN_MENU || st.gt==GAME_MENU)
	{
		if(CheckColisionMouse(400,200,200,50))
			DrawString("Start New Map",400,200,200,50,0,255,128,32,1,st.fonts[GEOMET].font);
		else
			DrawString("Start New Map",400,200,200,50,0,255,255,255,1,st.fonts[GEOMET].font);

		if(st.gt==GAME_MENU) 
		{
			if(CheckColisionMouse(400,150,150,50))
				DrawString("Save Map",400,150,150,50,0,255,128,32,1,st.fonts[GEOMET].font);
			else
				DrawString("Save Map",400,150,150,50,0,255,255,255,1,st.fonts[GEOMET].font);
		}

		if(CheckColisionMouse(400,250,150,50))
			DrawString("Load Map",400,250,150,50,0,255,128,32,1,st.fonts[GEOMET].font);
		else
			DrawString("Load Map",400,250,150,50,0,255,255,255,1,st.fonts[GEOMET].font);

		if(CheckColisionMouse(400,300,100,50))
			DrawString("Options",400,300,100,50,0,255,128,32,1,st.fonts[GEOMET].font);
		else
			DrawString("Options",400,300,100,50,0,255,255,255,1,st.fonts[GEOMET].font);

		if(CheckColisionMouse(400,350,50,50))
			DrawString("Quit",400,350,50,50,0,255,128,32,1,st.fonts[GEOMET].font);
		else
			DrawString("Quit",400,350,50,50,0,255,255,255,1,st.fonts[GEOMET].font);
	}
}

