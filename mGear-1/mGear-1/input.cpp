#include "input.h"

void InputInit()
{
	uint32 num, j;

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

	num=SDL_NumJoysticks();

	//Check if it's a game controller

	if(num>4) num=4; //Max number of joysticks is 4
	j=0;

	for(register uint8 i=0;i<num;i++)
	{
		if(SDL_IsGameController(i))
		{
			if((st.controller[j].device=SDL_GameControllerOpen(i))==NULL)
				LogApp("Game Controller %d could not be initialized: %s",i,SDL_GetError());
			else
			if((st.controller[j].joystick=SDL_JoystickOpen(i))==NULL)
				LogApp("Game Controller %d could not be initialized: %s",i,SDL_GetError());
			else
				j++;
		}
		/*
		else
		if((st.Joy[i]=SDL_JoystickOpen(i))==NULL)
			LogApp("Joystick %d could not be initialized: %s",i,SDL_GetError());
			*/
	}

	LogApp("Found %d controller devices",j);

	if(j>0)
	{

		st.control_num=j;

		for(register uint8 i=0;i<j;i++)
		{
			if((st.controller[i].force=SDL_HapticOpenFromJoystick(st.controller[i].joystick))==NULL)
				LogApp("Controller %d does not support force feedback: %s",j,SDL_GetError());
			else
				if(SDL_HapticRumbleInit(st.controller[i].force)!=0)
					LogApp("Could not initialize rumble: %s",SDL_GetError());

			st.controller[i].axis[0].name=SDL_CONTROLLER_AXIS_LEFTX;
			st.controller[i].axis[1].name=SDL_CONTROLLER_AXIS_LEFTY;
			st.controller[i].axis[2].name=SDL_CONTROLLER_AXIS_RIGHTX;
			st.controller[i].axis[3].name=SDL_CONTROLLER_AXIS_RIGHTY;
			st.controller[i].axis[4].name=SDL_CONTROLLER_AXIS_TRIGGERLEFT;
			st.controller[i].axis[5].name=SDL_CONTROLLER_AXIS_TRIGGERRIGHT;

			st.controller[i].button[0].name=SDL_CONTROLLER_BUTTON_A;
			st.controller[i].button[1].name=SDL_CONTROLLER_BUTTON_B;
			st.controller[i].button[2].name=SDL_CONTROLLER_BUTTON_X;
			st.controller[i].button[3].name=SDL_CONTROLLER_BUTTON_Y;
			st.controller[i].button[4].name=SDL_CONTROLLER_BUTTON_BACK;
			st.controller[i].button[5].name=SDL_CONTROLLER_BUTTON_GUIDE;
			st.controller[i].button[6].name=SDL_CONTROLLER_BUTTON_START;
			st.controller[i].button[7].name=SDL_CONTROLLER_BUTTON_LEFTSTICK;
			st.controller[i].button[8].name=SDL_CONTROLLER_BUTTON_RIGHTSTICK;
			st.controller[i].button[9].name=SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
			st.controller[i].button[10].name=SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
			st.controller[i].button[11].name=SDL_CONTROLLER_BUTTON_DPAD_UP;
			st.controller[i].button[12].name=SDL_CONTROLLER_BUTTON_DPAD_DOWN;
			st.controller[i].button[13].name=SDL_CONTROLLER_BUTTON_DPAD_LEFT;
			st.controller[i].button[14].name=SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
		}
	}
}

void InputProcess()
{
	size_t len;
		while(SDL_PollEvent(&events))
		{
				if(events.type==SDL_QUIT) st.quit=1;

				for(register uint16 i=0;i<MAX_KEYS;i++)
				{
				
					if(events.type==SDL_KEYUP)
					{
						if(st.keys[i].key==events.key.keysym.scancode)
							st.keys[i].state=0;
					}
					else
					if(events.type==SDL_KEYDOWN)
					{
						if(st.keys[i].key==events.key.keysym.scancode)
							st.keys[i].state=1;
					}
				
				}

				if(st.control_num>0)
				{
					SDL_GameControllerUpdate();

					for(register uint8 j=0;j<st.control_num;j++)
					{		
						for(register uint16 i=0;i<15;i++)
						{
							st.controller[j].button[i].state=SDL_GameControllerGetButton(st.controller[j].device,st.controller[j].button[i].name);

							if(i<6)
								st.controller[j].axis[i].state=SDL_GameControllerGetAxis(st.controller[j].device,st.controller[j].axis[i].name);
						}
					}
				}
				
				if(st.keys[BACKSPACE_KEY].state && st.Text_Input)
				{
					len=strlen(st.TextInput);
					st.TextInput[len]='\b';
					st.TextInput[len-1]='\0';
					st.keys[BACKSPACE_KEY].state=0;
				}
				
				if(events.type==SDL_TEXTINPUT)
					strcat(st.TextInput,events.text.text);
				
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
				else
				if(events.type==SDL_MOUSEWHEEL)
					st.mouse_wheel=events.wheel.y;

		}

		if(st.keys[RETURN_KEY].state && st.PlayingVideo)
		{
			st.PlayingVideo=0;
			st.keys[RETURN_KEY].state=0;
		}
}

void InputClose()
{
	if(st.control_num>0)
	{
		for(register uint8 i=0;i<st.control_num;i++)
		{
			SDL_HapticClose(st.controller[i].force);
			SDL_JoystickClose(st.controller[i].joystick);
			SDL_GameControllerClose(st.controller[i].device);
		}
	}
}