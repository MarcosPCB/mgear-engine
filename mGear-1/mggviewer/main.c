#include <Windows.h>
#include <commdlg.h>
#include "input.h"
#include "main.h"
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include "dirent.h"
#include "UI.h"
#include "funcs.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_sdl_gl3.h"

#define MAX_NK_VERTEX_BUFFER 512 * 1024
#define MAX_NK_ELEMENT_BUFFER 128 * 1024
#define MAX_NK_COMMAND_BUFFER 5000000
#define MAX_NK_BUFFER 16000000

int nkrendered = 0;

struct nk_context *ctx;

int prev_tic, curr_tic, delta;

Mggv mggv;

uint16 WriteCFG()
{
	FILE *file;

	if((file=fopen("mggv_settings.cfg","w"))==NULL)
		return 0;

	st.screenx=1024;
	st.screeny=576;
	st.fullscreen=0;
	st.bpp=32;
	st.audiof=44100;
	st.audioc=2;
	st.vsync=0;

	fprintf(file,"ScreenX = %d\n",st.screenx);
	fprintf(file,"ScreenY = %d\n",st.screeny);
	fprintf(file,"FullScreen = %d\n",st.fullscreen);
	fprintf(file,"ScreenBPP = %d\n",st.bpp);
	fprintf(file,"AudioFrequency = %d\n",st.audiof);
	fprintf(file,"AudioChannels = %d\n",st.audioc);
	fprintf(file,"VSync = %d\n",st.vsync);

	fclose(file);

	return 1;
}

uint16 LoadCFG()
{
	FILE *file;
	char buf[2048], str[128], str2[2048], *buf2, buf3[2048];
	int value=0;
	if((file=fopen("mggv_settings.cfg","r"))==NULL)
	{
		if (WriteCFG() == 0)
			return 0;

		if ((file = fopen("mggv_settings.cfg", "r")) == NULL)
			return 0;
	}

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
		if(strcmp(str,"VSync")==NULL) st.vsync=value;
		if (strcmp(str, "CurrentPath") == NULL)
		{
			memcpy(buf3, buf, 2048);
			buf2 = strtok(buf3, "\"");
			buf2 = strtok(NULL, "\"");
			strcpy(st.CurrPath, buf2);
			continue;
		}
	}

	if(!st.screenx || !st.screeny || !st.bpp || !st.audioc || !st.audioc || st.vsync>1)
	{
		fclose(file);
		if(WriteCFG()==0)
			return 0;
	}

	fclose(file);

	return 1;

}

void MenuBar()
{
	register int i, a, m;
	char str[128], filename[MAX_PATH];
	static int state = 0;

	OPENFILENAME ofn;
	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	ofn.lpstrFilter = "Master Gear Graphics file\0*.mgg\0";
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFile = filename;
	ofn.lpstrTitle = "Select the file";
	//ofn.hInstance = OFN_EXPLORER;
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_EXPLORER;

	if (nkrendered==0)
	{
		if (nk_begin(ctx, "Menu", nk_rect(0, 0, st.screenx, 30), NK_WINDOW_NO_SCROLLBAR))
		{
			ctx->current->flags = NK_WINDOW_NO_SCROLLBAR;

			nk_menubar_begin(ctx);
			nk_layout_row_begin(ctx, NK_STATIC, 25, 2);

			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(120, 210)))
			{
				nk_layout_row_dynamic(ctx, 30, 1);

				if (nk_menu_item_label(ctx, "Open MGG file", NK_TEXT_LEFT))
				{
					GetOpenFileName(&ofn);

					if (filename)
					{
						if (mggv.mgg.num_frames)
							FreeMGG(&mggv.mgg);

						if (!LoadMGG(&mggv.mgg, filename))
							MessageBox(NULL, "Could not open MGG file", "Error", MB_OK);
					}
					
				}

				if (nk_menu_item_label(ctx, "Exit", NK_TEXT_LEFT)) st.quit = 1;
				nk_menu_end(ctx);
			}

			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "Help", NK_TEXT_LEFT, nk_vec2(120, 100)))
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				//nk_menu_item_label(ctx, "Help", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "About", NK_TEXT_LEFT);
				nk_menu_end(ctx);
			}

			nk_layout_row_end(ctx);
			nk_menubar_end(ctx);
		}

		nk_end(ctx);
	}
}

void Canvas()
{
	register int i, j;
	float px, py, sx, sy, aspect;
	TEX_DATA data;
	struct nk_image texid;
	int rt = 0;
	static frame;

	if (nk_begin(ctx, "Viewer", nk_rect(0, 30, st.screenx, st.screeny - 30), NK_WINDOW_BORDER | NK_WINDOW_BACKGROUND))
	{
		nk_menubar_begin(ctx);

		nk_layout_row_dynamic(ctx, 25, 8);
		mggv.state = nk_option_label(ctx, "Diffuse", mggv.state == 0) ? 0 : mggv.state;
		mggv.state = nk_option_label(ctx, "Normal", mggv.state == 1) ? 1 : mggv.state;
		mggv.state = nk_option_label(ctx, "Animations", mggv.state == 2) ? 2 : mggv.state;

		nk_menubar_end(ctx);

		if (mggv.state != 2)
		{
			nk_layout_row_begin(ctx, NK_STATIC, (st.screeny - 30) / 6, 12);
			for (i = 0; i < mggv.mgg.num_frames + mggv.mgg.num_atlas; i++)
			{
				if (i < mggv.mgg.num_atlas)
				{
					if (!mggv.state)
						data.data = mggv.mgg.atlas[i];
					else
						data.data = mggv.mgg.frames[i + mggv.mgg.num_frames].Ndata;

					data.normal = mggv.mgg.frames[i + mggv.mgg.num_frames].normal;

					sx = mggv.mgg.frames[i + mggv.mgg.num_frames].w;
					sy = mggv.mgg.frames[i + mggv.mgg.num_frames].h;
					texid = nk_image_id(data.data);
				}
				else
				{
					data = mggv.mgg.frames[i - mggv.mgg.num_atlas];

					if (data.vb_id != -1)
					{
						px = ((float)data.posx / 32768.0f) * data.w + 2;
						//ceil(px);
						//px += data.x_offset;
						py = ((float)data.posy / 32768.0f) * data.h;
						//ceil(py);
						//floor(py);
						//py += data.y_offset;
						sx = ((float)data.sizex / 32768.0f) * data.w;
						//floor(sx);
						//ceil(sx);
						sy = ((float)data.sizey / 32768.0f) * data.h;
						//floor(sy);
						//ceil(sy);
						if (!mggv.state)
							texid = nk_subimage_id(data.data, data.w, data.h, nk_rect(px, py, sx, sy));
						else
							texid = nk_subimage_id(data.Ndata, data.w, data.h, nk_rect(px, py, sx, sy));
					}
					else
					{
						sx = data.w;
						sy = data.h;
						if (!mggv.state)
							texid = nk_subimage_id(data.data, sx, sy, nk_rect(0, 0, sx, sy - 2));
						else
							texid = nk_subimage_id(data.Ndata, sx, sy, nk_rect(0, 0, sx, sy - 2));
					}
				}

				if (sy > sx)
					aspect = ((st.screeny - 30) / 5) * (sx / sy);
				else
					aspect = ((st.screeny - 30) / 5) / (sy / sx);

				nk_layout_row_push(ctx, aspect);

				if (mggv.state && !data.normal)
					nk_select_label(ctx, "No normal map", NK_TEXT_ALIGN_LEFT, 1);
				else
					nk_image(ctx, texid);
			}

			nk_layout_row_end(ctx);
		}
		else
		{
			for (i = 0; i < mggv.mgg.num_anims; i++)
			{
				nk_layout_row_begin(ctx, NK_STATIC, (st.screeny - 30) / 6, 1);
				frame = mggv.mgg.anim[i].current_frame / 100;

				data = mggv.mgg.frames[frame];

				if (data.vb_id != -1)
				{
					px = ((float)data.posx / 32768.0f) * data.w + 2;
					//ceil(px);
					//px += data.x_offset;
					py = ((float)data.posy / 32768.0f) * data.h;
					//ceil(py);
					//floor(py);
					//py += data.y_offset;
					sx = ((float)data.sizex / 32768.0f) * data.w;
					//floor(sx);
					//ceil(sx);
					sy = ((float)data.sizey / 32768.0f) * data.h;
					//floor(sy);
					//ceil(sy);
					texid = nk_subimage_id(data.data, data.w, data.h, nk_rect(px, py, sx, sy));
				}
				else
				{
					sx = data.w;
					sy = data.h;
					texid = nk_subimage_id(data.data, sx, sy, nk_rect(0, 0, sx, sy - 2));
				}

				if (sy > sx)
					aspect = ((st.screeny - 30) / 5) * (sx / sy);
				else
					aspect = ((st.screeny - 30) / 5) / (sy / sx);

				nk_layout_row_push(ctx, aspect);

				nk_image(ctx, texid);

				nk_layout_row_end(ctx);
			}
		}
	}

	nk_end(ctx);
}

int main(int argc, char *argv[])
{
	int i;
	char str[64];
	int loops;

	struct nk_color background;

	PreInit("mggv",argc, argv);

	if(LoadCFG()==0)
		if(MessageBox(NULL,"Error while trying to read or write the configuration file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();


	strcpy(st.LogName, "mggviewer.log");
	Init();

	strcpy(st.WindowTitle,"MGG Viewer");

	//OpenFont("font/Roboto-Regular.ttf","arial",0,128);
	//OpenFont("font/Roboto-Bold.ttf","arial bold",1,128);
	//OpenFont("font//tt0524m_.ttf","geometry",2,128);

	InitMGG();

	//st.FPSYes=1;

	st.Developer_Mode=1;

	curr_tic=GetTicks();

	ctx = nk_sdl_init(wn, MAX_NK_BUFFER, MAX_NK_VERTEX_BUFFER, MAX_NK_ELEMENT_BUFFER);

	struct nk_font_atlas *atlas;
	nk_sdl_font_stash_begin(&atlas);
	struct nk_font *fonts = nk_font_atlas_add_from_file(atlas, "Font\\Roboto-Regular.ttf", 16, 0);
	nk_sdl_font_stash_end();
	nk_style_set_font(ctx, &fonts->handle);

	background = nk_rgb(28, 48, 62);

	SETENGINEPATH;

	memset(&mggv, 0, sizeof(Mggv));

	if (argc > 1)
	{
		if (strcmp(argv[1], "-o") == NULL)
		{
			if (!LoadMGG(&mggv.mgg, argv[2]))
				MessageBox(NULL, "Could not open MGG file", "Error", MB_OK);
		}
	}

	while(!st.quit)
	{
		//if(st.FPSYes)
		FPSCounter();

		nk_input_begin(ctx);

		int8 AC;
		char *ACdata = CheckAppComm(&AC);

		if (ACdata != NULL && AC >= 0)
		{
			if (AC == IA_OPENFILE)
			{
				SDL_RestoreWindow(wn);
				SDL_ShowWindow(wn);
				SDL_RaiseWindow(wn);

				if (!LoadMGG(&mggv.mgg, ACdata))
					MessageBox(NULL, "Could not open MGG file", "Error", MB_OK);
				else
					ResetAppComm();
			}
		}

		while(PollEvent(&events))
		{
			WindowEvents();
			nk_sdl_handle_event(&events);
		}

		nk_input_end(ctx);

		BASICBKD(255,255,255);
		
		loops=0;
		while(GetTicks() > curr_tic && loops < 10)
		{
			//nk_clear(ctx);
			Finish();

			for (i = 0; i < mggv.mgg.num_anims; i++)
			{
				if (mggv.mgg.anim[i].speed > 0)
				{
					if (mggv.mgg.anim[i].current_frame < mggv.mgg.anim[i].startID * 100)
						mggv.mgg.anim[i].current_frame = mggv.mgg.anim[i].startID * 100;
				}
				else
				{
					if (mggv.mgg.anim[i].current_frame > mggv.mgg.anim[i].startID * 100)
						mggv.mgg.anim[i].current_frame = mggv.mgg.anim[i].startID * 100;
				}

				mggv.mgg.anim[i].current_frame += mggv.mgg.anim[i].speed;

				if (mggv.mgg.anim[i].speed > 0)
				{
					if (mggv.mgg.anim[i].current_frame >= mggv.mgg.anim[i].endID * 100)
						mggv.mgg.anim[i].current_frame = mggv.mgg.anim[i].startID * 100;
				}
				else
				{
					if (mggv.mgg.anim[i].current_frame <= mggv.mgg.anim[i].endID * 100)
						mggv.mgg.anim[i].current_frame = mggv.mgg.anim[i].startID * 100;
				}
			}

			curr_tic+=1000/TICSPERSECOND;
			loops++;
			SetTimerM(1);
		}

		DrawSys();
		if (mggv.mgg.frames)
			Canvas();

		MenuBar();

		UIMain_DrawSystem();
		//MainSound();
		Renderer(0);

		float bg[4];
		nk_color_fv(bg, background);

		nk_sdl_render(NK_ANTI_ALIASING_OFF, MAX_NK_VERTEX_BUFFER, MAX_NK_ELEMENT_BUFFER, MAX_NK_COMMAND_BUFFER);

		SwapBuffer(wn);

		nkrendered = 0;
	}

	if (mggv.mgg.num_frames)
		FreeMGG(&mggv.mgg);

	nk_sdl_shutdown();

	Quit();
	return 1;
}