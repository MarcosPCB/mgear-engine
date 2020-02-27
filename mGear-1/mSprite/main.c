#include "input.h"
#include "main.h"
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include "dirent.h"
#include "UI.h"
#include "funcs.h"
#include <time.h>
#include <SDL_video.h>
#include <Windows.h>
#include <commdlg.h>

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
#define _NKUI_SKINS
#include "skins.h"

int nkrendered = 0;

struct nk_context *ctx;
struct nk_font *fonts[5];

int prev_tic, curr_tic, delta, idx;

mSpr mspr;

void SetThemeBack()
{
	switch (mspr.theme)
	{
	case THEME_BLACK:
		SetSkin(ctx, THEME_BLACK);
		break;

	case THEME_RED:
		SetSkin(ctx, THEME_RED);
		break;

	case THEME_WHITE:
		SetSkin(ctx, THEME_WHITE);
		break;

	case THEME_BLUE:
		SetSkin(ctx, THEME_BLUE);
		break;

	case THEME_DARK:
		SetSkin(ctx, THEME_DARK);
	}
}

char *its(uint8 icon)
{
	static char buf[2];

	buf[0] = icon;

	return buf;
}

int nk_button_toggle(struct nk_context *ctx, const char *str, int active)
{
	int ret;

	NK_ASSERT(ctx);
	NK_ASSERT(str);

	if (active == 1)
	{
		ctx->style.button.normal = ctx->style.button.active;
		ctx->style.button.hover = ctx->style.button.active;
	}

	ret = nk_button_label(ctx, str);

	if (ret == 1 && active == 0)
		active = 1;
	
	if (ret == 1 && active == 1)
		active = 0;

	SetThemeBack();

	return ret;
}

int nk_button_image_toggle(struct nk_context *ctx, struct nk_image img, int active)
{
	int ret;

	NK_ASSERT(ctx);
	//NK_ASSERT(str);

	if (active == 1)
	{
		ctx->style.button.normal = ctx->style.button.active;
		ctx->style.button.hover = ctx->style.button.active;
	}

	ret = nk_button_image(ctx, img);

	if (ret == 1 && active == 0)
		active = 1;

	if (ret == 1 && active == 1)
		active = 0;

	SetThemeBack();

	return ret;
}

char *GetRootDir(const char *path)
{
	char final_path[MAX_PATH];
	int16 i = 0, j, k;

	if (path[0] == '.' && (path[1] == '\\' || path[1] == '/'))
		i = 1;
	else if (path[0] == '.' && path[1] == "." && (path[2] == '\\' || path[2] == '/'))
		i = 2;

	j = i;

	for (; i < strlen(path); i++)
	{
		if (path[i] == '\\' || path[i] == '/')
			break;
	}

	for (k = 0; j < i; j++, k++)
		final_path[k] = path[j];

	return final_path;
}

uint16 WriteCFG()
{
	FILE *file;

	if((file=fopen("msdk_settings.cfg","w"))==NULL)
		return 0;

	st.screenx=1280;
	st.screeny=720;
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
	fprintf(file, "Theme = %d\n", mspr.theme);
	fprintf(file, "FPS = 1\n");

	fclose(file);

	return 1;
}

uint16 SaveCFG()
{
	FILE *file;

	if ((file = fopen(StringFormat("%s\\msdk_settings.cfg", st.CurrPath), "w")) == NULL)
		return 0;

	fprintf(file, "ScreenX = %d\n", st.screenx);
	fprintf(file, "ScreenY = %d\n", st.screeny);
	fprintf(file, "FullScreen = %d\n", st.fullscreen);
	fprintf(file, "ScreenBPP = %d\n", st.bpp);
	fprintf(file, "AudioFrequency = %d\n", st.audiof);
	fprintf(file, "AudioChannels = %d\n", st.audioc);
	fprintf(file, "VSync = %d\n", st.vsync);
	fprintf(file, "Theme = %d\n", mspr.theme);
	fprintf(file, "FPS = %d\n", st.FPSYes);

	fclose(file);

	return 1;
}

uint16 LoadCFG()
{
	FILE *file;
	char buf[2048], str[128], str2[2048], *buf2, buf3[2048];
	int value = 0, recent = 0;

	for (int i = 0; i < 10; i++)
		mspr.open_recent[i][0] = '\0';

	if((file=fopen("msdk_settings.cfg","r"))==NULL)
	{
		if (WriteCFG() == 0)
			return 0;

		if ((file = fopen("msdk_settings.cfg", "r")) == NULL)
			return 0;
	}

	while(!feof(file))
	{
		fgets(buf,sizeof(buf),file);

		if (recent == 1)
		{
			if (recent < 11)
			{
				buf2 = strtok(buf, "\"");
				strcpy(mspr.open_recent[recent - 1], buf2);
				recent++;
				continue;
			}
			else
			{
				recent = 0;
				break;
			}
		}

		sscanf(buf,"%s = %d", str, &value);
		if(strcmp(str,"ScreenX")==NULL) st.screenx=value;
		if(strcmp(str,"ScreenY")==NULL) st.screeny=value;
		if(strcmp(str,"ScreenBPP")==NULL) st.bpp=value;
		if(strcmp(str,"FullScreen")==NULL) st.fullscreen=value;
		if(strcmp(str,"AudioFrequency")==NULL) st.audiof=value;
		if(strcmp(str,"AudioChannels")==NULL) st.audioc=value;
		if(strcmp(str,"VSync")==NULL) st.vsync=value;
		if (strcmp(str, "Theme") == NULL) mspr.theme = value;
		if (strcmp(str, "FPS") == NULL) st.FPSYes = value;
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

int SavePrjFile(const char *filename)
{
	FILE *f;
	char header[21] = { "mGear Project V1.0" };


	if ((f = fopen(filename, "wb")) == NULL)
	{
		GetError;
		return 0;
	}

	fwrite(header, 21, 1, f);
	

	fclose(f);

	return 1;
}

int LoadPrjFile(const char *filename)
{
	FILE *f;
	char header[21];

	if (filename == NULL)
	{
		MessageBoxRes("Error", MB_OK, "Invalid file NULL");
		return -1;
	}
	
	if ((f = fopen(filename, "rb")) == NULL)
	{
		GetError;
		return 0;
	}

	fread(header, 21, 1, f);

	if (strcmp(header, "mGear Project V1.0") != NULL)
	{
		fclose(f);
		return -1;
	}

	

	fclose(f);

	//LoadTDL();
	//LoadBaseLog();

	//if (mspr.prj.curr_rev > 0)
	//	LoadRevLog();

	return 1;
}

int16 DirFiles(const char *path, char content[512][MAX_PATH])
{
	DIR *dir;
	dirent *ent;
	uint16 i = 0;
	int16 filenum = 0;

	if ((dir = opendir(path)) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			if (content != NULL)
				strcpy(content[i], ent->d_name);

			i++;
			filenum++;
		}

		closedir(dir);
	}
	else
	{
		LogApp("Could not open directory");
		return -1;
	}

	return filenum;
}

int16 DirFrames(const char *path)
{
	DIR *dir;
	dirent *ent;
	register uint16 i = 0, j = 0;
	int16 filenum = 0, len;
	char frame[32];

	if ((dir = opendir(path)) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			len = strlen(ent->d_name);

			if (ent->d_name[len - 1] == 'g' && ent->d_name[len - 2] == 'p' && ent->d_name[len - 3] == 'j' && ent->d_name[len - 4] == '.')
				filenum++;
		}

		closedir(dir);
	}
	else
	{
		LogApp("Could not open directory");
		return -1;
	}

	return filenum;
}

struct nk_color ColorPicker(struct nk_color color)
{
	if (nk_combo_begin_color(ctx, color, nk_vec2(200, 250)))
	{
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf c = nk_color_picker(ctx, nk_color_cf(color), NK_RGB);
		color = nk_rgb_cf(c);
		nk_layout_row_dynamic(ctx, 25, 1);
		color.r = (nk_byte)nk_propertyi(ctx, "R:", 0, color.r, 255, 1, 1);
		color.g = (nk_byte)nk_propertyi(ctx, "G:", 0, color.g, 255, 1, 1);
		color.b = (nk_byte)nk_propertyi(ctx, "B:", 0, color.b, 255, 1, 1);

		nk_combo_end(ctx);
	}

	return color;
}

int Preferences()
{
	static char str[32];
	register int i;
	static FILE *f;
	static int state = 0;
	int temp, temp2;
	nk_size size;

	if (mspr.theme == THEME_WHITE) strcpy(str, "White skin");
	if (mspr.theme == THEME_RED) strcpy(str, "Red skin");
	if (mspr.theme == THEME_BLUE) strcpy(str, "Blue skin");
	if (mspr.theme == THEME_DARK) strcpy(str, "Dark skin");
	//if (mspr.theme == THEME_GWEN) strcpy(str, "GWEN skin");
	if (mspr.theme == THEME_BLACK) strcpy(str, "Default");

	SetCurrentDirectory(mspr.program_path);

	if (nk_begin(ctx, "Preferences", nk_rect(st.screenx / 2 - 160, st.screeny / 2 - 190, 320, 380), NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE))
	{
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "UI skin", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 25, 1);
		if (nk_combo_begin_label(ctx, str, nk_vec2(nk_widget_width(ctx), 225)))
		{
			nk_layout_row_dynamic(ctx, 25, 1);

			if (nk_combo_item_label(ctx, "White skin", NK_TEXT_ALIGN_LEFT))
			{
				mspr.theme = THEME_WHITE;
				SetSkin(ctx, THEME_WHITE);
				strcpy(str, "White skin");
			}

			if (nk_combo_item_label(ctx, "Red skin", NK_TEXT_ALIGN_LEFT))
			{
				mspr.theme = THEME_RED;
				SetSkin(ctx, THEME_RED);
				strcpy(str, "Red skin");
			}

			if (nk_combo_item_label(ctx, "Blue skin", NK_TEXT_ALIGN_LEFT))
			{
				mspr.theme = THEME_BLUE;
				SetSkin(ctx, THEME_BLUE);
				strcpy(str, "Blud skin");
			}

			if (nk_combo_item_label(ctx, "Dark skin", NK_TEXT_ALIGN_LEFT))
			{
				mspr.theme = THEME_DARK;
				SetSkin(ctx, THEME_DARK);
				strcpy(str, "Dark skin");
			}
			/*
			if (nk_combo_item_label(ctx, "GWEN skin", NK_TEXT_ALIGN_LEFT))
			{
			mspr.theme = THEME_GWEN;
			SetSkin(ctx, THEME_GWEN);
			strcpy(str, "GWEN skin");
			}
			*/
			if (nk_combo_item_label(ctx, "Default", NK_TEXT_ALIGN_LEFT))
			{
				mspr.theme = THEME_BLACK;
				SetSkin(ctx, THEME_BLACK);
				strcpy(str, "Default skin");
			}

			nk_combo_end(ctx);
		}

		nk_layout_row_dynamic(ctx, 25, 1);
		st.FPSYes = nk_check_label(ctx, "FPS counter", st.FPSYes == 1);
	}

	nk_end(ctx);

	return NULL;
}

void MenuBar()
{
	register int i, a, m;
	char mapname[2048], str[128], filename[2048];
	int id = 0, id2 = 0, check, temp;
	static int state = 0, mggid, path[MAX_PATH];

	//if (nkrendered==0)
	//{
	if (state == 1)
	{
		//temp = NewProject();

		if (temp == 1 || temp == -1)
			state = 0;
	}

	if (state == 10)
	{
		if (Preferences())
			state = 0;
	}

	if (state == 6)
	{
		//temp = ExportProject();

		if (temp == -1 || temp == 1)
			state = 0;
	}

		if (nk_begin(ctx, "Menu", nk_rect(0, 0, st.screenx, 30), NK_WINDOW_NO_SCROLLBAR))
		{
			ctx->current->flags = NK_WINDOW_NO_SCROLLBAR;

			nk_menubar_begin(ctx);
			nk_layout_row_begin(ctx, NK_STATIC, 25, 3);

			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(120, 310)))
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				if (nk_menu_item_label(ctx, "New sprite list", NK_TEXT_LEFT))
				{
					//UnloadPrj();
					state = 1;
				}

				if (nk_menu_item_label(ctx, "Open sprite list", NK_TEXT_LEFT))
				{
					//UnloadPrj();

					OPENFILENAME ofn;
					ZeroMemory(&path, sizeof(path));
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
					ofn.lpstrFilter = "Sprite List Files\0*.slist\0";
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFile = path;
					ofn.lpstrTitle = "Select the sprite list file";
					//ofn.hInstance = OFN_EXPLORER;
					ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_EXPLORER;

					if (GetOpenFileName(&ofn))
					{
						temp = LoadSpriteList(path);

						if (temp == 0)
							MessageBox(NULL, "Error could not open the file", "Error", MB_OK);

						if (temp == -1)
							MessageBox(NULL, "Error invalid file", "Error", MB_OK);

						if (temp == 1)
						{
							
						}
					}

					state = 0;
				}
				
				if (nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT))
				{
					SavePrjFile(mspr.filepath);

					
				}

				nk_menu_item_label(ctx, "Save as...", NK_TEXT_LEFT);
				//nk_menu_item_label(ctx, "Import", NK_TEXT_LEFT);
				//if (nk_menu_item_label(ctx, "Export", NK_TEXT_LEFT))
				//	state = 6;

				if (nk_menu_item_label(ctx, "Exit", NK_TEXT_LEFT)) st.quit = 1;
				nk_menu_end(ctx);
			}

			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "Edit", NK_TEXT_LEFT, nk_vec2(120, 200)))
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				if (nk_menu_item_label(ctx, "Preferences", NK_TEXT_LEFT))
					state = 10;

				nk_menu_end(ctx);
			}


			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "Help", NK_TEXT_LEFT, nk_vec2(120, 200)))
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				nk_menu_item_label(ctx, "Help", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "About", NK_TEXT_LEFT);
				nk_menu_end(ctx);
			}

			nk_layout_row_end(ctx);
			nk_menubar_end(ctx);
		}

		nk_end(ctx);

		
	//}
}

void Pannel()
{
	register int i, j, k;
	static int sl, pannel_state = 0;
	static struct nk_color editcolor = { 255, 255, 255, 255 };
	static char strbuf[32], entry[512];
	static struct nk_rect bounds;
	TEX_DATA data;
	int temp, px, py, sx, sy, tmp;
	struct nk_image texid;
	MSG msg;
	char buf[256];

	DWORD error;

	static int state = 0;
	DWORD exitcode;

	//static SHELLEXECUTEINFO info;

	if (nk_begin(ctx, "Sprite list", nk_rect(0, 30, st.screenx * 0.15, st.screeny - 30), NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE
		| NK_WINDOW_SCALABLE))
	{
		for (i = 0; i < st.num_sprites; i++)
		{
			nk_layout_row_dynamic(ctx, 30, 1);
			if (nk_select_label(ctx, st.Game_Sprites[i].name, NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, mspr.selected_spr == i))
				mspr.selected_spr = i;
		}
	}
	
	nk_end(ctx);

	if (nk_begin(ctx, "Properties", nk_rect(st.screenx * 0.15, 30, st.screenx * 0.55, st.screeny * 0.6 - 30), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER | NK_WINDOW_TITLE
		| NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE))
	{
		nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 2);

		nk_layout_row_push(ctx, 0.05);
		nk_label(ctx, "Name:", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);

		nk_layout_row_push(ctx, 0.30);
		nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, st.Game_Sprites[i].name, 64, nk_filter_ascii);
		nk_layout_row_end(ctx);


		nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 3);
		nk_layout_row_push(ctx, 0.05);
		nk_label(ctx, "MGG: ", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);

		nk_layout_row_push(ctx, 0.50);
		nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, mgg_game[st.Game_Sprites[i].MGG_ID].name, 32, nk_filter_ascii);

		nk_layout_row_push(ctx, 0.10);
		nk_button_label(ctx, "Browse");
		nk_layout_row_end(ctx);

		nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 4);
		nk_layout_row_push(ctx, 0.25);
		st.Game_Sprites[i].num_frames = nk_propertyd(ctx, "Number of frames:", 1, st.Game_Sprites[i].num_frames, 16384, 1, 1);

		nk_layout_row_push(ctx, 0.01);
		nk_spacing(ctx, 1);

		nk_layout_row_push(ctx, 0.12);
		nk_button_label(ctx, "Select frames");

		nk_layout_row_push(ctx, 0.01);
		nk_spacing(ctx, 1);

		nk_layout_row_push(ctx, 0.30);
		strcpy(buf, "Starting frames: ");
		if (st.Game_Sprites[i].frame)
		{
			for (j = 0; j < st.Game_Sprites[i].num_start_frames; j++)
			{
				if (st.Game_Sprites[i].frame[j] > -1)
					strcat(buf, StringFormat("%d, ", st.Game_Sprites[i].frame[j]));
			}
		}

		nk_label(ctx, buf, NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);

		nk_layout_row_end(ctx);

		nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 4);

		nk_layout_row_end(ctx);
	}

	nk_end(ctx);

	if (nk_begin(ctx, "AI states", nk_rect(st.screenx * 0.15, st.screeny * 0.6, st.screenx * 0.85, st.screeny * 0.4), NK_WINDOW_BORDER | NK_WINDOW_TITLE
		| NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE))
	{

	}

	nk_end(ctx);

	if (nk_begin(ctx, "Canvas", nk_rect(st.screenx * 0.7, 30, st.screenx * 0.3, st.screeny * 0.6 - 30), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER | NK_WINDOW_TITLE
		| NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE))
	{

	}

	nk_end(ctx);
}

int main(int argc, char *argv[])
{
	char str[64];
	int loops;

	struct nk_color background;

	memset(&mspr, 0, sizeof(mSpr));

	PreInit("msprite",argc,argv);

	if(LoadCFG()==0)
		if(MessageBox(NULL,"Error while trying to read or write the configuration file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();

	strcpy(st.LogName, "msprite.log");

	Init();

	//DisplaySplashScreen();

	strcpy(st.WindowTitle, "mSprite");

	//OpenFont("font/Roboto-Regular.ttf","arial",0,128);
	//OpenFont("font/Roboto-Bold.ttf","arial bold",1,128);
	//OpenFont("font//tt0524m_.ttf","geometry",2,128);

	InitMGG();

	/*
	if(LoadMGG(&mgg_sys[0],"data/mEngUI.mgg")==NULL)
	{
		LogApp("Could not open UI mgg");
		Quit();
	}
	*/
	UILoadSystem("UI_Sys.cfg");

	//st.FPSYes=1;

	st.Developer_Mode=1;

	curr_tic=GetTicks();

	ctx = nk_sdl_init(wn);
	
	struct nk_font_atlas *atlas;
	nk_sdl_font_stash_begin(&atlas);
	fonts[0] = nk_font_atlas_add_from_file(atlas, "Font\\Roboto-Regular.ttf", 16, 0);
	fonts[1] = nk_font_atlas_add_from_file(atlas, "Font\\mUI.ttf", 18, 0);
	fonts[2] = nk_font_atlas_add_from_file(atlas, "Font\\Roboto-Regular.ttf", 14, 0);
	nk_sdl_font_stash_end();
	nk_style_set_font(ctx, &fonts[0]->handle);
	
	background = nk_rgb(28, 48, 62);
	
	SETENGINEPATH;

	//InitGWEN();
	SetSkin(ctx, mspr.theme);

	GetCurrentDirectory(MAX_PATH, mspr.program_path);

	//InitEngineWindow();

	while(!st.quit)
	{
		//if(st.FPSYes)
		FPSCounter();

		nk_input_begin(ctx);

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

			curr_tic+=1000/TICSPERSECOND;
			loops++;
			SetTimerM(1);
		}

		DrawSys();


		//if (mspr.prj.loaded == 1)
		Pannel();


		MenuBar();

		UIMain_DrawSystem();
		//MainSound();
		Renderer(0);

		float bg[4];
		nk_color_fv(bg, background);

		nk_sdl_render(NK_ANTI_ALIASING_OFF, 512 * 1024, 128 * 1024);

		SwapBuffer(wn);

		nkrendered = 0;
	}

	Quit();
	return 1;
}