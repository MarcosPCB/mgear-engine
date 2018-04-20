#include <Windows.h>
#include <commdlg.h>
#include "input.h"
#include "main.h"
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include "dirent.h"
#include "UI.h"

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
		if(WriteCFG()==0)
			return 0;

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

int16 DirFiles(const char *path, char content[512][512])
{
	DIR *dir;
	dirent *ent;
	uint16 i=0;
	int16 filenum=0;

	if((dir=opendir(path))!=NULL)
	{
		while((ent=readdir(dir))!=NULL)
		{
			strcpy(content[i],ent->d_name);
			i++;
			filenum++;
		}

		closedir(dir);
	}
	else
	{
		LogApp("Coulnd not open directory");
		return -1;
	}

	return filenum;
}

struct nk_color ColorPicker(struct nk_color color)
{
	if (nk_combo_begin_color(ctx, color, nk_vec2(200, 250)))
	{
		nk_layout_row_dynamic(ctx, 120, 1);
		color = nk_color_picker(ctx, color, NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		color.r = (nk_byte)nk_propertyi(ctx, "R:", 0, color.r, 255, 1, 1);
		color.g = (nk_byte)nk_propertyi(ctx, "G:", 0, color.g, 255, 1, 1);
		color.b = (nk_byte)nk_propertyi(ctx, "B:", 0, color.b, 255, 1, 1);

		nk_combo_end(ctx);
	}

	return color;
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

void SpriteListSelection()
{
	register int i, j, k, l = 0, m;
	TEX_DATA data;
	int temp;
	float px, py, sx, sy;
	struct nk_image texid;
	char labels[6][32], label_active[6] = { 0, 0, 0, 0, 0, 0 };

	if (nk_begin(ctx, "Sprite Selection", nk_rect(st.screenx / 2 - 300, st.screeny / 2 - 300, 600, 600),
		NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_SCALABLE))
	{
		nk_layout_row_dynamic(ctx, 515, 1);

		if (nk_group_begin(ctx, "SPRSEL", NK_WINDOW_BORDER))
		{
			/*
			ctx->style.selectable.text_normal = nk_rgb(255, 128, 32);
			ctx->style.selectable.text_hover = nk_rgb(255, 128, 8);
			ctx->style.selectable.text_pressed = nk_rgb(255, 128, 8);
			ctx->style.selectable.text_normal_active = nk_rgb(255, 32, 32);
			ctx->style.selectable.text_hover_active = nk_rgb(255, 32, 32);
			ctx->style.selectable.text_pressed_active = nk_rgb(255, 32, 32);
			*/

			ctx->style.selectable.hover = nk_style_item_color(nk_rgb(206, 206, 206));
			ctx->style.selectable.normal_active = nk_style_item_color(nk_rgb(255, 128, 32));
			ctx->style.selectable.hover_active = nk_style_item_color(nk_rgb(255, 128, 32));

			nk_layout_row_dynamic(ctx, 100, 6);
			//l = 0;
			for (i = 0; i < st.num_sprites; i++)
			{
				j = st.sprite_id_list[i];
				if (st.Game_Sprites[j].num_start_frames>0)
				{
					for (k = 0; k < st.Game_Sprites[j].num_start_frames; k++)
					{
						data = mgg_game[st.Game_Sprites[j].MGG_ID].frames[st.Game_Sprites[j].frame[k]];

					//	if (meng.sprite_selection == j && meng.sprite_frame_selection == st.Game_Sprites[j].frame[k])
							temp = 1;
						//else
							temp = 0;

						if (data.vb_id != -1)
						{
							px = ((float)data.posx / 32768) * data.w;
							ceil(px);
							px += data.x_offset;
							py = ((float)data.posy / 32768) * data.h;
							ceil(py);
							py += data.y_offset;
							sx = ((float)data.sizex / 32768) * data.w;
							ceil(sx);
							sy = ((float)data.sizey / 32768) * data.h;
							ceil(sy);
							texid = nk_subimage_id(data.data, data.w, data.h, nk_recta(nk_vec2(px, py), nk_vec2(sx, sy)));
						}
						else
							texid = nk_image_id(data.data);

						if (nk_selectable_image_label(ctx, texid," ", NK_TEXT_ALIGN_CENTERED, &temp))
						{
							//meng.sprite_selection = j;
							//meng.sprite_frame_selection = st.Game_Sprites[j].frame[k];

							//meng.spr.health = st.Game_Sprites[j].health;
							//meng.spr.body = st.Game_Sprites[j].body;
							//meng.spr.flags = st.Game_Sprites[j].flags;

							//meng.spr.body.size = st.Game_Sprites[j].body.size;
						}

						strcpy(labels[l], st.Game_Sprites[j].name);

						//nk_label(ctx, labels[l], NK_TEXT_ALIGN_CENTERED);

						label_active[l] = temp;

						l++;

						if (l == 6)
						{
							nk_layout_row_dynamic(ctx, 15, 6);
							for (m = 0; m < l; m++)
							{
								if (label_active[m])
									ctx->style.text.color = nk_rgb(255, 255, 255);
								else
									ctx->style.text.color = nk_rgb(128, 128, 128);

								nk_label(ctx, labels[m], NK_TEXT_ALIGN_CENTERED);
							}

							memset(label_active, 6, 0);

							nk_layout_row_dynamic(ctx, 100, 6);
							l = 0;
						}
					}
				}
			}

			if (l != 0)
			{
				nk_layout_row_dynamic(ctx, 15, 6);
				for (m = 0; m < l; m++)
					nk_label(ctx, labels[m], NK_TEXT_ALIGN_CENTERED);

				l = 0;
			}

			nk_group_end(ctx);
		}

		nk_style_default(ctx);

		nk_layout_row_dynamic(ctx, 30, 3);
		nk_spacing(ctx, 2);

		//if (nk_button_label(ctx, "Select"))
			//meng.command = ADD_SPRITE;
	}

	st.mouse1 = 0;

	nk_end(ctx);
}

void Canvas()
{
	register int i, j;
	float px, py, sx, sy;
	TEX_DATA data;
	struct nk_image texid;
	int rt = 0;

	if (nk_begin(ctx, mggv.mgg.name, nk_rect(0, 30, st.screenx, st.screeny - 30), NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_BACKGROUND))
	{
		nk_layout_row_dynamic(ctx, st.screenx / 12.0f, 12);

		for (i = 0; i < mggv.mgg.num_frames + mggv.mgg.num_atlas; i++)
		{
			if (i < mggv.mgg.num_atlas)
			{
				data.data = mggv.mgg.atlas[i];
				texid = nk_image_id(data.data);
			}
			else
			{
				data = mggv.mgg.frames[i - mggv.mgg.num_atlas];

				if (data.vb_id != -1)
				{
					px = ((float)data.posx / 32768) * data.w;
					//floor(px);
					//px += data.x_offset;
					py = ((float)data.posy / 32768) * data.h;
					//ceil(py);
					//py += data.y_offset;
					sx = ((float)data.sizex / 32768) * data.w;
					//ceil(sx);
					sy = ((float)data.sizey / 32768) * data.h;
					//ceil(sy);
					texid = nk_subimage_id(data.data, data.w, data.h, nk_rect(px, py, sx, sy));
				}
				else
					texid = nk_image_id(data.data);
			}

			nk_image(ctx, texid);
			//nk_selectable_image_label(ctx, texid, " ", NK_TEXT_ALIGN_LEFT, &rt);
			//nk_selectable_image_label(ctx, nk_image_id(NULL), "test", NK_TEXT_ALIGN_LEFT, &rt);

			//break;
		}
	}

	nk_end(ctx);
}

int main(int argc, char *argv[])
{
	char str[64];
	int loops;

	struct nk_color background;

	if(LoadCFG()==0)
		if(MessageBox(NULL,"Error while trying to read or write the configuration file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();


	strcpy(st.LogName, "mggviewer.log");
	Init();

	strcpy(st.WindowTitle,"MGG Viewer");

	OpenFont("font/Roboto-Regular.ttf","arial",0,128);
	OpenFont("font/Roboto-Bold.ttf","arial bold",1,128);
	//OpenFont("font//tt0524m_.ttf","geometry",2,128);

	InitMGG();

	st.FPSYes=1;

	st.Developer_Mode=1;

	curr_tic=GetTicks();

	ctx = nk_sdl_init(wn);

	struct nk_font_atlas *atlas;
	nk_sdl_font_stash_begin(&atlas);
	nk_sdl_font_stash_end();
	background = nk_rgb(28, 48, 62);

	SETENGINEPATH;

	memset(&mggv, 0, sizeof(Mggv));

	while(!st.quit)
	{
		if(st.FPSYes)
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
		if (mggv.mgg.frames)
			Canvas();

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