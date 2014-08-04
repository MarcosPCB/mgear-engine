#include "input.h"

void InputInit()
{
	st.keys[ESC_KEY].key=SDL_SCANCODE_ESCAPE;
	st.keys[RETURN_KEY].key=SDL_SCANCODE_RETURN;
	st.keys[2].key=SDL_SCANCODE_RIGHT;
	st.keys[3].key=SDL_SCANCODE_LEFT;
	st.keys[4].key=SDL_SCANCODE_UP;
	st.keys[5].key=SDL_SCANCODE_DOWN;
	st.keys[6].key=SDL_SCANCODE_SPACE;
	st.keys[7].key=SDL_SCANCODE_LCTRL;
	st.keys[8].key=SDL_SCANCODE_RCTRL;
	st.keys[9].key=SDL_SCANCODE_LSHIFT;
	st.keys[10].key=SDL_SCANCODE_RSHIFT;
	st.keys[11].key=SDL_SCANCODE_TAB;
	st.keys[12].key=SDL_SCANCODE_LALT;
	st.keys[13].key=SDL_SCANCODE_RALT;
	st.keys[14].key=SDL_SCANCODE_F1;
	st.keys[15].key=SDL_SCANCODE_F2;
	st.keys[16].key=SDL_SCANCODE_F3;
	st.keys[17].key=SDL_SCANCODE_F4;
	st.keys[18].key=SDL_SCANCODE_F5;
	st.keys[19].key=SDL_SCANCODE_F6;
	st.keys[20].key=SDL_SCANCODE_F7;
	st.keys[21].key=SDL_SCANCODE_F8;
	st.keys[22].key=SDL_SCANCODE_F9;
	st.keys[23].key=SDL_SCANCODE_F10;
	st.keys[24].key=SDL_SCANCODE_F11;
	st.keys[25].key=SDL_SCANCODE_F12;
	st.keys[26].key=SDL_SCANCODE_BACKSPACE;
	st.keys[27].key=SDL_SCANCODE_0;
	st.keys[28].key=SDL_SCANCODE_1;
	st.keys[29].key=SDL_SCANCODE_2;
	st.keys[30].key=SDL_SCANCODE_3;
	st.keys[31].key=SDL_SCANCODE_4;
	st.keys[32].key=SDL_SCANCODE_5;
	st.keys[33].key=SDL_SCANCODE_6;
	st.keys[34].key=SDL_SCANCODE_7;
	st.keys[35].key=SDL_SCANCODE_8;
	st.keys[36].key=SDL_SCANCODE_9;
	st.keys[37].key=SDL_SCANCODE_A;
	st.keys[38].key=SDL_SCANCODE_B;
	st.keys[39].key=SDL_SCANCODE_C;
	st.keys[40].key=SDL_SCANCODE_D;
	st.keys[41].key=SDL_SCANCODE_E;
	st.keys[42].key=SDL_SCANCODE_F;
	st.keys[43].key=SDL_SCANCODE_G;
	st.keys[44].key=SDL_SCANCODE_H;
	st.keys[45].key=SDL_SCANCODE_I;
	st.keys[46].key=SDL_SCANCODE_J;
	st.keys[47].key=SDL_SCANCODE_K;
	st.keys[48].key=SDL_SCANCODE_L;
	st.keys[49].key=SDL_SCANCODE_M;
	st.keys[50].key=SDL_SCANCODE_N;
	st.keys[51].key=SDL_SCANCODE_O;
	st.keys[52].key=SDL_SCANCODE_P;
	st.keys[53].key=SDL_SCANCODE_Q;
	st.keys[54].key=SDL_SCANCODE_R;
	st.keys[55].key=SDL_SCANCODE_S;
	st.keys[56].key=SDL_SCANCODE_T;
	st.keys[57].key=SDL_SCANCODE_U;
	st.keys[58].key=SDL_SCANCODE_V;
	st.keys[59].key=SDL_SCANCODE_W;
	st.keys[60].key=SDL_SCANCODE_X;
	st.keys[61].key=SDL_SCANCODE_Y;
	st.keys[62].key=SDL_SCANCODE_Z;
	st.keys[63].key=SDL_SCANCODE_COMMA;
	st.keys[64].key=SDL_SCANCODE_BACKSLASH;
	st.keys[65].key=SDL_SCANCODE_GRAVE;
	st.keys[66].key=SDL_SCANCODE_SLASH;
	st.keys[67].key=SDL_SCANCODE_APOSTROPHE;
	st.keys[68].key=SDL_SCANCODE_EQUALS;
	st.keys[69].key=SDL_SCANCODE_MINUS;
	st.keys[70].key=SDL_SCANCODE_LEFTBRACKET;
	st.keys[71].key=SDL_SCANCODE_RIGHTBRACKET;
	st.keys[72].key=SDL_SCANCODE_PERIOD;
	st.keys[73].key=SDL_SCANCODE_SEMICOLON;
	st.keys[74].key=SDL_SCANCODE_CAPSLOCK;
}

void InputProcess()
{
		while(SDL_PollEvent(&events))
		{
				if(events.type==SDL_QUIT) st.quit=1;

				for(uint16 i=0;i<MAX_KEYS;i++)
				{
				
					if(events.type==SDL_KEYUP)
					{
						if(st.keys[i].key==events.key.keysym.scancode)
							st.keys[i].state=0;
					}
				
					if(events.type==SDL_KEYDOWN)
					{
						if(st.keys[i].key==events.key.keysym.scancode)
							st.keys[i].state=1;
					}
				
				}

				if(events.type==SDL_MOUSEMOTION)
				{
					st.mouse.x=events.motion.x;
					st.mouse.y=events.motion.y;
				}

				if(events.type==SDL_MOUSEBUTTONDOWN)
				{
					st.mouse.x=events.motion.x;
					st.mouse.y=events.motion.y;

					if(events.button.button==SDL_BUTTON_LEFT)
						st.mouse1=1;

					if(events.button.button==SDL_BUTTON_RIGHT)
						st.mouse2=1;

				}
				else
				if(events.type==SDL_MOUSEBUTTONUP)
				{
					st.mouse.x=events.motion.x;
					st.mouse.y=events.motion.y;

					if(events.button.button==SDL_BUTTON_LEFT)
						st.mouse1=0;

					if(events.button.button==SDL_BUTTON_RIGHT)
						st.mouse2=0;

				}

		}

		if(st.keys[RETURN_KEY].state && st.PlayingVideo)
		{
			st.PlayingVideo=0;
			st.keys[RETURN_KEY].state=0;
		}
}