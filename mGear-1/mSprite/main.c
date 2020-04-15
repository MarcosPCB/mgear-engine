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

#define MAX_NK_VERTEX_BUFFER 512 * 1024
#define MAX_NK_ELEMENT_BUFFER 128 * 1024
#define MAX_NK_COMMAND_BUFFER 5000000
#define MAX_NK_BUFFER 16000000

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

int nk_button_icon_set(uint8 icon)
{
	char buf[2];

	buf[0] = icon;
	buf[1] = 0;

	nk_style_set_font(ctx, &fonts[1]->handle);

	int ret = nk_button_label(ctx, buf);

	nk_style_set_font(ctx, &fonts[0]->handle);

	return ret;
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

int nk_button_icon_toggle(struct nk_context *ctx, uint8 icon, int active)
{
	int ret;

	NK_ASSERT(ctx);
	//NK_ASSERT(icon);

	if (active == 1)
	{
		ctx->style.button.normal = ctx->style.button.active;
		ctx->style.button.hover = ctx->style.button.active;
	}

	ret = nk_button_icon_set(icon);

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

	if((file=fopen("mspr_settings.cfg","w"))==NULL)
		return 0;

	st.screenx=1280;
	st.screeny=720;
	st.fullscreen=0;
	st.bpp=32;
	st.audiof=44100;
	st.audioc=2;
	st.vsync=0;

	mspr.max_undo_states = 32;

	fprintf(file,"ScreenX = %d\n",st.screenx);
	fprintf(file,"ScreenY = %d\n",st.screeny);
	fprintf(file,"FullScreen = %d\n",st.fullscreen);
	fprintf(file,"ScreenBPP = %d\n",st.bpp);
	fprintf(file,"AudioFrequency = %d\n",st.audiof);
	fprintf(file,"AudioChannels = %d\n",st.audioc);
	fprintf(file,"VSync = %d\n",st.vsync);
	fprintf(file, "Theme = %d\n", mspr.theme);
	fprintf(file, "FPS = 1\n");
	fprintf(file, "Undo = %d\n", mspr.max_undo_states);

	fclose(file);

	return 1;
}

uint16 SaveCFG()
{
	FILE *file;

	if ((file = fopen(StringFormat("%s\\mspr_settings.cfg", st.CurrPath), "w")) == NULL)
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
	fprintf(file, "Undo = %d\n", mspr.max_undo_states);

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

	if((file=fopen("mspr_settings.cfg","r"))==NULL)
	{
		if (WriteCFG() == 0)
			return 0;

		if ((file = fopen("mspr_settings.cfg", "r")) == NULL)
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
		if (strcmp(str, "Undo") == NULL) mspr.max_undo_states = value;
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

void CheckForChanges()
{
	if (memcmp(mspr.sprbck, st.Game_Sprites, sizeof(_SPRITES) * MAX_SPRITES) != NULL)
	{
		mspr.changes_detected = 1;
		sprintf(st.WindowTitle, "Sprite *%s", GetFileNameOnly(mspr.filepath));
	}
}

void PushUndo(_SPRITES *s, uint8 i)
{
	if (mspr.num_undo_states == 32)
		mspr.num_undo_states = 32;

	if (mspr.num_undo_states > 0)
	{
		for (uint16 j = mspr.num_undo_states; j > 0; j--)
		{
			mspr.undostates[j] = mspr.undostates[j - 1];
			mspr.undostates[j - 1] = 0;

			mspr.sprunstate[j] = mspr.sprunstate[j - 1];
			memset(&mspr.sprunstate[j - 1], 0, sizeof(_SPRITES));
		}
	}

	mspr.undostates[0] = i;
	memcpy(&mspr.sprunstate[0], s, sizeof(_SPRITES));
}

uint8 PopUndo(_SPRITES *s)
{
	uint8 i = mspr.undostates[0];
	memcpy(s, &mspr.sprunstate[0], sizeof(_SPRITES));

	for (uint16 j = 1; j < mspr.num_undo_states; j++)
	{
		mspr.undostates[j - 1] = mspr.undostates[j];
		mspr.undostates[j] = 0;

		mspr.sprunstate[j - 1] = mspr.sprunstate[j];
		memset(&mspr.sprunstate[j], 0, sizeof(_SPRITES));
	}

	mspr.num_undo_states--;

	return i;
}

void UndoState()
{
	for (int i = 0; i < st.num_sprites; i++)
	{
		if (memcmp(&mspr.sprbck[i], &st.Game_Sprites[i], sizeof(_SPRITES)) != NULL)
		{
			PushUndo(&mspr.sprbck[i], i);
			memcpy(&mspr.sprbck[i], &st.Game_Sprites[i], sizeof(_SPRITES));

			if (mspr.num_undo_states < mspr.max_undo_states)
				mspr.num_undo_states++;

			break;
		}
	}
}

int SavePrjFile(const char *path)
{
	FILE *f;
	int16 i, j, k, l, error = 0;
	char str[2048], str2[2048], buf[2048];
	DIR *d;

	if (st.num_sprites == 0)
	{
		MessageBoxRes("Error", MB_OK, "Error: no sprites on the list");
		return NULL;
	}

	strcpy(str, path);

	for (i = strlen(str); i > 0; i--)
	{
		if (path[i] == '\\' || path[i] == '/')
		break;

		str[i] = '\0';
	}
	
	SetCurrentDirectory(str);

	strcat(str, "/Sprites");

	if ((d = opendir(str)) == NULL)
		system("md Sprites");
	else
		closedir(d);

	for (i = 0; i < st.num_sprites; i++)
	{
		strcpy(str2, str);
		strcat(str2, StringFormat("/#%d - %s.cfg", i, st.Game_Sprites[i].name));

		if ((f = fopen(str2, "w")) == NULL)
		{
			GetError;
			error = 1;
			goto SVPRJERROR;
		}

		fprintf(f, "NAME %s\n", st.Game_Sprites[i].name);
		fprintf(f, "MGG %s PART\n", mgg_game[st.Game_Sprites[i].MGG_ID].path);
		fprintf(f, "NUM_FRAMES %d\n", st.Game_Sprites[i].num_frames);

		ZeroMemory(buf, 2048);
		for (j = 0; j < st.Game_Sprites[i].num_start_frames; j++)
			strcat(buf, StringFormat("%d ", st.Game_Sprites[i].frame[j]));

		fprintf(f, "FRAMES %s\n", buf);
		fprintf(f, "HEALTH %d\n", st.Game_Sprites[i].health);
		fprintf(f, "MASS %d\n", st.Game_Sprites[i].body.mass);
		
		switch (st.Game_Sprites[i].body.material)
		{
			case METAL:
				fprintf(f, "MATERIAL METAL\n");
				break;

			case WOOD:
				fprintf(f, "MATERIAL WOOD\n");
				break;
				
			case ORGANIC:
				fprintf(f, "MATERIAL ORGANIC\n");
				break;

			case CONCRETE:
				fprintf(f, "MATERIAL CONCRETE\n");
				break;

			case PLASTIC:
				fprintf(f, "MATERIAL PLASTIC\n");
				break;

			case MATERIAL_END:
				fprintf(f, "MATERIAL NONE\n");
				break;
		}

		switch (st.Game_Sprites[i].type)
		{
			case ENEMY:
				fprintf(f, "TYPE ENEMY\n");
				break;

			case FRIEND:
				fprintf(f, "TYPE FRIEND\n");
				break;

			case GAME_LOGICAL:
				fprintf(f, "TYPE LOGICAL\n");
				break;

			case NORMAL:
				fprintf(f, "TYPE NORMAL\n");
				break;
		}

		if (st.Game_Sprites[i].flags & 4)
			fprintf(f, "ORIGINAL_SIZE\n");
		else
			fprintf(f, "SIZE %d %d\n", st.Game_Sprites[i].body.size.x, st.Game_Sprites[i].body.size.y);

		if (st.Game_Sprites[i].size_m.x != 0 || st.Game_Sprites[i].size_m.y != 0)
			fprintf(f, "SIZE_MUL %d %d\n", st.Game_Sprites[i].size_m.x == 0 ? 1 : st.Game_Sprites[i].size_m.x, st.Game_Sprites[i].size_m.y == 0 ? 1 : st.Game_Sprites[i].size_m.y);

		if (st.Game_Sprites[i].size_a.x != 0 || st.Game_Sprites[i].size_a.y != 0)
			fprintf(f, "SIZE_ADD %d %d\n", st.Game_Sprites[i].size_a.x, st.Game_Sprites[i].size_a.y);

		if (st.Game_Sprites[i].flags & 1)
			fprintf(f, "RESIZEABLE\n");

		if (st.Game_Sprites[i].flags & 2)
			fprintf(f, "HIDDEN\n");

		if (st.Game_Sprites[i].shadow == 1)
			fprintf(f, "SHADOW\n");

		fprintf(f, "FLAMABLE %d\n", st.Game_Sprites[i].body.flamable);
		fprintf(f, "EXPLOSIVE %d\n", st.Game_Sprites[i].body.explosive);

		if (st.Game_Sprites[i].num_states > 0)
		{
			for (j = 0; j < st.Game_Sprites[i].num_states; j++)
			{
				if (st.Game_Sprites[i].states[j].used == 1)
				{
					fprintf(f, "STATE %d - %s %s %s %d %s %d %s %d\n", j, st.Game_Sprites[i].states[j].name, st.Game_Sprites[i].states[j].loop == 1 ? "LOOP" : "NOLOOP",
						st.Game_Sprites[i].states[j].animation == 1 ? "ANIMATED" : "STILL", st.Game_Sprites[i].states[j].main_anim,
						st.Game_Sprites[i].states[j].in == 1 ? "IN" : "NOIN", st.Game_Sprites[i].states[j].in_transition, st.Game_Sprites[i].states[j].out == 1 ? "OUT" : "NOOUT",
						st.Game_Sprites[i].states[j].out_transition);

					ZeroMemory(buf, 2048);

					strcpy(buf, StringFormat("STATE_OUTPUTS %d - ", j));

					for (k = 0; k < 64; k++)
					{
						if (st.Game_Sprites[i].states[j].outputs[k] != -1)
							strcat(buf, StringFormat("%d %d : ", k, st.Game_Sprites[i].states[j].outputs[k]));
					}

					fprintf(f, "%s\n", buf);
				}
			}
		}

		LogApp("Saved Sprite CFG \"%s\"", str2);

		fclose(f);
	}

	if ((f = fopen(path, "w")) == NULL)
	{
		GetError;
		error = 2;
		goto SVPRJERROR;
	}

	for (i = 0; i < st.num_sprites; i++)
	{
		fprintf(f, "SPRITE %s %d \"sprites/#%d - %s.cfg\" %d", st.Game_Sprites[i].name, i, i, st.Game_Sprites[i].name, st.Game_Sprites[i].num_tags);

		for (j = 0; j < st.Game_Sprites[i].num_tags; j++)
			fprintf(f, " %s ", st.Game_Sprites[i].tag_names[j]);

		fprintf(f, "\n");
	}

	LogApp("Saved Sprite List at \"%s\"", path);

	fclose(f);

	memcpy(mspr.sprbck, st.Game_Sprites, MAX_SPRITES * sizeof(_SPRITES));
	strcpy(mspr.filepath, StringFormat("%s/sprite.slist", path));
	
SVPRJERROR:

	if (error == 1)
	{
		SetCurrentDirectory(mspr.program_path);
		MessageBoxRes("Error", MB_OK, StringFormat("Error: could not write the sprite file \"%s\"", str2));
		return NULL;
	}

	if (error == 2)
	{
		SetCurrentDirectory(mspr.program_path);
		MessageBoxRes("Error", MB_OK, StringFormat("Error: could not write the sprite list \"%s\"", str2));
		return NULL;
	}

	return 1;
}

int NewProject()
{
	for (int i = 0; i < st.num_sprites; i++)
	{
		if (st.Game_Sprites[i].num_start_frames > 0)
			free(st.Game_Sprites[i].frame);
	}

	memset(st.Game_Sprites, 0, sizeof(_SPRITES)* st.num_sprites);

	st.num_sprites = 0;

	for (int i = 0; i < st.num_sprites; i++)
	{
		st.Game_Sprites[i].MGG_ID = -1;
		for (int j = 0; j < 64; j++)
		{
			memset(st.Game_Sprites[i].states[j].inputs, -1, sizeof(int16)* 64);
			memset(st.Game_Sprites[i].states[j].outputs, -1, sizeof(int16)* 64);

			st.Game_Sprites[i].states[j].inputs[0] = j;
			st.Game_Sprites[i].states[j].outputs[0] = j;
		}
	}

	mspr.anim_frame = mspr.selected_spr = mspr.state_selected = mspr.curframe = -1;
	mspr.play = 0;

	for (int i = 0; i < st.num_mgg; i++)
		FreeMGG(&mgg_game[i]);

	memcpy(mspr.sprbck, st.Game_Sprites, MAX_SPRITES * sizeof(_SPRITES));

	memset(mspr.undostates, 0, mspr.max_undo_states + 1 * sizeof(uint16));

	memset(mspr.sprunstate, 0, mspr.max_undo_states + 1, sizeof(_SPRITES));

	memcpy(mspr.sprunstate, st.Game_Sprites, mspr.max_undo_states + 1 * sizeof(_SPRITES));

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
	static int temp, temp2;
	nk_size size;

	if (mspr.theme == THEME_WHITE) strcpy(str, "White skin");
	if (mspr.theme == THEME_RED) strcpy(str, "Red skin");
	if (mspr.theme == THEME_BLUE) strcpy(str, "Blue skin");
	if (mspr.theme == THEME_DARK) strcpy(str, "Dark skin");
	//if (mspr.theme == THEME_GWEN) strcpy(str, "GWEN skin");
	if (mspr.theme == THEME_BLACK) strcpy(str, "Default");

	SetCurrentDirectory(mspr.program_path);

	if (nk_begin(ctx, "Preferences", nk_rect(st.screenx / 2 - 160, st.screeny / 2 - 190, 320, 200), NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE))
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

		temp = mspr.max_undo_states;

		mspr.max_undo_states = nk_propertyd(ctx, "Max. undo states:", 8, mspr.max_undo_states, 254, 8, 2);

		if (temp != mspr.max_undo_states + 1)
		{
			mspr.undostates = realloc(mspr.undostates, mspr.max_undo_states + 1 * sizeof(uint16));
			mspr.sprunstate = realloc(mspr.sprunstate, mspr.max_undo_states + 1 * sizeof(_SPRITES));

			if (mspr.num_undo_states > temp)
				mspr.num_undo_states = temp;
		}

		if (nk_button_label(ctx, "Ok"))
		{
			SaveCFG();
			nk_end(ctx);
			return 1;
		}
	}

	nk_end(ctx);

	return NULL;
}

int FrameListSelection(uint8 mode)
{
	register int i, j, k, l = 0, m;
	TEX_DATA data;
	int temp;
	float px, py, sx, sy;
	struct nk_image texid;
	static int16 sel1 = 0, sel2 = 0, msel[32], msel_num = 0, mggl = -1, state = 0;

	_SPRITES *spr = &st.Game_Sprites[mspr.selected_spr];
	_MGG mgg;

	if (mode == 1)
		state = 10;

	if (mggl != spr->MGG_ID)
	{
		if (spr->num_frames == 0 || spr->num_start_frames == 0)
		{
			sel1 = -1;
			sel2 = -1;
			memset(msel, -1, sizeof(int16)* 32);
			msel_num = 0;
			mggl = spr->MGG_ID;
			state = 0;
		}
		else
		{
			sel1 = spr->frame[0];
			sel2 = sel1 + spr->num_frames - 1;

			msel_num = spr->num_start_frames;

			memset(msel, -1, sizeof(int16)* 32);

			for (i = 0; i < spr->num_start_frames; i++)
				msel[i] = spr->frame[i];

			mggl = spr->MGG_ID;
			state = 0;
		}
	}

	if (spr->MGG_ID == -1)
	{
		MessageBoxRes("Error", MB_OK, "Error: no MGG loaded");
		return 1;
	}
	else
	{
		mgg = mgg_game[spr->MGG_ID];

		char wnstr[32];

		if (state == 0)
			strcpy(wnstr, "Select the starting frame");

		if (state == 1)
			strcpy(wnstr, "Select the final frame");

		if (state == 2)
			strcpy(wnstr, "Select the sprite starters");

		if (state == 10)
			strcpy(wnstr, "Select the state frame");

		if (nk_begin(ctx, wnstr, nk_rect(st.screenx / 2 - 300, st.screeny / 2 - 300, 600, 600),
			NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE | NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_SCALABLE))
		{
			nk_layout_row_dynamic(ctx, 515, 1);

			if (nk_group_begin(ctx, "TEXSEL", NK_WINDOW_BORDER))
			{
				ctx->style.selectable.hover = nk_style_item_color(nk_rgb(206, 206, 206));
				ctx->style.selectable.normal_active = nk_style_item_color(nk_rgb(255, 128, 32));
				ctx->style.selectable.hover_active = nk_style_item_color(nk_rgb(255, 128, 32));

				nk_layout_row_dynamic(ctx, 100, 6);

				if (state == 0)
				{
					j = 0;
					l = mgg.num_frames;
				}
				
				if (state == 2)
				{
					j = sel1;
					l = mgg.num_frames;
				}

				if (state == 4)
				{
					l = sel2 + 1;
					j = sel1;
				}

				if (state == 10)
				{
					j = spr->frame[0];
					l = spr->frame[0] + spr->num_frames;
				}

				for (; j < l; j++)
				{
						data = mgg.frames[j];

						if (state == 0)
						{
							if (sel1 == j)
								temp = 1;
							else
								temp = 0;
						}
						
						if (state == 2)
						{
							if (j < sel2 || sel2 == j)
								temp = 1;
							else
								temp = 0;
						}

						if (state == 4)
						{
							temp = 0;
							for (i = 0; i < 32; i++)
							{
								if (msel[i] == j)
									temp = 1;
							}
						}

						if (state == 10)
						{
							if (sel1 == j)
								temp = 1;
							else
								temp = 0;
						}

						if (data.vb_id != -1)
						{
							px = ((float)data.posx / 32768.0f) * data.w;
							//ceil(px);
							//px += data.x_offset;
							py = ((float)data.posy / 32768.0f) * data.h;
							//ceil(py);
							//py += data.y_offset;
							sx = ((float)data.sizex / 32768.0f) * data.w;
							//ceil(sx);
							sy = ((float)data.sizey / 32768.0f) * data.h;
							//ceil(sy);
							texid = nk_subimage_id(data.data, data.w, data.h, nk_rect(px, py, sx, sy));
						}
						else
							texid = nk_image_id(data.data);

						if (nk_selectable_image_label(ctx, texid, " ", NK_TEXT_ALIGN_CENTERED, &temp))
						{
							if (state == 0)
							{
								sel1 = j;
								msel[0] = sel1;

								if (msel_num == 0)
									msel_num = 1;
							}

							if (state == 2)
							{
								sel2 = j;
							}

							if (state == 4)
							{
								k = 0;
								for (i = 0; i < 32; i++)
								{
									if (msel[i] == j)
									{
										msel[i] = -1;
										msel_num--;
										k = 1;
										break;
									}
								}

								if (k == 0)
								{
									for (i = 0; i < 32; i++)
									{
										if (msel[i] == -1)
										{
											msel[i] = j;
											msel_num++;
											break;
										}
									}
								}
							}

							if (state == 10)
								sel1 = j;
						}
					}

				nk_group_end(ctx);
			}

			SetThemeBack(ctx, mspr.theme);

			nk_layout_row_dynamic(ctx, 30, 4);
			nk_spacing(ctx, 2);

			if (nk_button_label(ctx, "Select"))
			{
				if (sel1 == mgg.num_frames - 1 && state == 0)
				{
					switch (MessageBoxRes("Warning", MB_YESNOCANCEL, "You have selected the last frame of the MGG,\nmeaning that, this sprite has only one possible frame.\nDo you wish to proceed?"))
					{
					case IDYES:
						sel2 = sel1;
						msel_num = 1;
						state = 5;
						break;

					case IDCANCEL:
						state = 6;
						break;
					}
				}
				else
					state++;
			}

			if (nk_button_label(ctx, "Cancel"))
				state = 6;
		}

		st.mouse1 = 0;

		nk_end(ctx);
	}

	if (state == 1)
	{
		MessageBoxRes("Next step...", MB_OK, "Select the final frame of the sprite");
		state = 2;
	}

	if (state == 3)
	{
		MessageBoxRes("Next step...", MB_OK, "Select the frames that will start the sprite");
		state = 4;
	}

	if (state == 5)
	{
		spr->num_frames = sel2 - sel1 + 1;

		if (spr->num_start_frames > 0)
		{
			free(spr->frame);
		}

		spr->frame = malloc(sizeof(int32)* msel_num);

		spr->num_start_frames = msel_num;

		for (i = 0, k = 0; i < 32; i++)
		{
			if (msel[i] == -1) continue;

			spr->frame[i] = msel[i];
			k++;

			if (k == msel_num) break;
		}

		state = 0;
		mggl = -1;

		mspr.curframe = spr->frame[0];
		spr->body.size.x = (mgg.frames[spr->frame[0]].sizex / 32768.0f) * mgg.frames[spr->frame[0]].w;
		spr->body.size.y = (mgg.frames[spr->frame[0]].sizey / 32768.0f) * mgg.frames[spr->frame[0]].h;

		STW(&spr->body.size.x, &spr->body.size.y);

		return 1;
	}

	if (state == 6)
	{
		state = 0;
		mggl = -1;

		return 1;
	}

	if (state == 11)
	{
		spr->states[mspr.state_selected].main_anim = sel1;
		state = 0;
		mggl = -1;

		return 1;
	}

	return NULL;
}

void MenuBar()
{
	register int i, a, m;
	char mapname[2048], str[128], filename[2048];
	int id = 0, id2 = 0, check, temp;
	static int state = 0, mggid;
	static char path[MAX_PATH];

	//if (nkrendered==0)
	//{
	if (state == 1)
	{
		NewProject();
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
						NewProject();
						temp = LoadSpriteList(path);

						if (temp == 0)
							MessageBox(NULL, "Error could not open the file", "Error", MB_OK);

						if (temp == -1)
							MessageBox(NULL, "Error invalid file", "Error", MB_OK);

						if (temp == 1)
						{
							strcpy(mspr.filepath, path);
							memcpy(mspr.sprbck, st.Game_Sprites, MAX_SPRITES * sizeof(_SPRITES));

							sprintf(st.WindowTitle, "Sprite %s", GetFileNameOnly(mspr.filepath));
						}
					}

					state = 0;
				}
				
				if (nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT))
				{
					strcpy(path, mspr.filepath);

					for (i = strlen(path); i > 0; i--)
					{
						if (path[i] == '\\' || path[i] == '/')
							break;
						
						path[i] = '\0';
					}

					SavePrjFile(path);

					//strcpy(mspr.filepath, path);
					//memcpy(mspr.sprbck, st.Game_Sprites, MAX_SPRITES * sizeof(_SPRITES));

					sprintf(st.WindowTitle, "Sprite %s", GetFileNameOnly(mspr.filepath));
				}

				if (nk_menu_item_label(ctx, "Save as...", NK_TEXT_LEFT))
				{
					ZeroMemory(&path, sizeof(path));

					OPENFILENAME ofn;

					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
					ofn.lpstrFilter = "Sprite list File\0*.slist\0";
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFile = path;
					ofn.lpstrTitle = "Save the sprite list file";
					//ofn.hInstance = OFN_EXPLORER;
					ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_EXPLORER;

					if (GetSaveFileName(&ofn) && path)
					{
						SavePrjFile(path);

						strcpy(mspr.filepath, path);
						memcpy(mspr.sprbck, st.Game_Sprites, MAX_SPRITES * sizeof(_SPRITES));

						sprintf(st.WindowTitle, "Sprite %s", GetFileNameOnly(mspr.filepath));
					}
				}

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

				if (nk_menu_item_label(ctx, "Undo", NK_TEXT_LEFT) && mspr.num_undo_states > 0)
				{
					_SPRITES s;
					uint8 undo_n = PopUndo(&s);

					memcpy(&st.Game_Sprites[undo_n], &s, sizeof(_SPRITES));
					memcpy(&mspr.sprbck[undo_n], &st.Game_Sprites[undo_n], sizeof(_SPRITES));
				}

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
	static char strbuf[32], entry[512], filename[2048];
	static struct nk_rect bounds;
	static struct nk_rect nodes[64];
	TEX_DATA data;
	int temp, px, py, sx, sy, tmp;
	struct nk_image texid;
	MSG msg;
	char buf[256];

	DWORD error;

	static int state = 0, sel2 = -1;
	DWORD exitcode;

	//static SHELLEXECUTEINFO info;

	if (nk_begin(ctx, "Sprite list", nk_rect(0, 30, st.screenx * 0.15, st.screeny - 30), NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		nk_layout_row_dynamic(ctx, 30, 2);
		if (nk_button_icon_toggle(ctx, BIN_ICON, state == 2 || state == 3) && mspr.selected_spr != -1 && (state != 2 && state != 3))
		{
			if (MessageBoxRes("Delete sprite", MB_YESNO, "Do you want to delete this sprite?") == IDYES)
			{
				i = mspr.selected_spr;

				if (st.Game_Sprites[i].num_start_frames > 0)
					free(st.Game_Sprites[i].frame);

				memset(&st.Game_Sprites[i], 0, sizeof(_SPRITES));

				for (j = i + 1; j < st.num_sprites; j++)
				{
					_SPRITES a;
					a = st.Game_Sprites[j - 1];
					st.Game_Sprites[j - 1] = st.Game_Sprites[j];
					st.Game_Sprites[j] = a;
				}

				st.num_sprites--;
				mspr.selected_spr = -1;
			}
		}

		if (nk_button_icon_toggle(ctx, SWITCH_ICON, state == 2 || state == 3))
		{
			//mspr.selected_spr = -1;

			if (state == 2 || state == 3)
				state = 0;
			else
			if (state == 0)
				state = 2;
		}

		for (i = 0; i < st.num_sprites + 1; i++)
		{
			if (i == st.num_sprites)
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				if (nk_button_symbol(ctx, NK_SYMBOL_PLUS))
				{
					strcpy(st.Game_Sprites[st.num_sprites].name, StringFormat("Sprites_#%d", st.num_sprites));
					st.num_sprites++;
				}

				break;
			}

			nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 2);
			nk_layout_row_push(ctx, 0.15);
			
			if (st.Game_Sprites[i].MGG_ID != -1 && st.Game_Sprites[i].num_start_frames > 0)
			{
				if (mgg_game[st.Game_Sprites[i].MGG_ID].frames[st.Game_Sprites[i].frame[0]].vb_id == -1)
				{
					nk_image(ctx, nk_image_id(mgg_game[st.Game_Sprites[i].MGG_ID].frames[st.Game_Sprites[i].frame[0]].data));
				}
				else
				{
					data = mgg_game[st.Game_Sprites[i].MGG_ID].frames[st.Game_Sprites[i].frame[0]];

					float ax = ((float)data.posx / 32768.0f) * data.w;
					float ay = ((float)data.posy / 32768.0f) * data.h;
					float bx = ((float)data.sizex / 32768.0f) * data.w;
					float by = ((float)data.sizey / 32768.0f) * data.h;
					nk_image(ctx, nk_subimage_id(data.data, data.w, data.h, nk_rect(ax, ay, bx, by)));
				}
			}
			else
				nk_spacing(ctx, 1);

			nk_layout_row_push(ctx, 0.85);

			if (state == 2)
			{
				if (mspr.selected_spr != -1) state = 3;

				if (nk_select_label(ctx, st.Game_Sprites[i].name, NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, mspr.selected_spr == i))
				{
					mspr.selected_spr = i;
					sel2 = -1;
					state = 3;
				}
			}

			if (state == 3)
			{
				if (nk_select_label(ctx, st.Game_Sprites[i].name, NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, mspr.selected_spr == i || sel2 == i))
				{
					if (mspr.selected_spr != i)
					{
						if (MessageBoxRes("Switch sprites", MB_YESNO, "Do you want to switch these sprites?") == IDYES)
						{
							_SPRITES a = st.Game_Sprites[i];
							st.Game_Sprites[i] = st.Game_Sprites[mspr.selected_spr];
							st.Game_Sprites[mspr.selected_spr] = a;
						}

						mspr.selected_spr = sel2 = -1;
						state = 0;
						break;
					}
				}
			}
			
			if (state != 2 && state != 3)
			{
				char name_temp[64];

				if (strlen(st.Game_Sprites[i].name) == 0)
					name_temp[0] = ' ';
				else
					strcpy(name_temp, st.Game_Sprites[i].name);

				if (nk_select_label(ctx, name_temp, NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, mspr.selected_spr == i))
				{
					if (mspr.selected_spr != i)
					{
						mspr.selected_spr = i;
						if (st.Game_Sprites[i].num_start_frames > 0) mspr.curframe = st.Game_Sprites[i].frame[0];
						mspr.state_selected = -1;
					}
				}
			}

			nk_layout_row_end(ctx);
		}

		i = mspr.selected_spr;
	}
	
	nk_end(ctx);

	if (nk_begin(ctx, "Properties", nk_rect(st.screenx * 0.15, 30, st.screenx * 0.55, st.screeny * 0.4 - 30), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		if (mspr.selected_spr != -1)
		{
			if (nk_input_mouse_clicked(&ctx->input, NK_BUTTON_LEFT, nk_window_get_content_region(ctx)) && state == 0)
				mspr.state_selected = -1;

			nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 2);

			nk_layout_row_push(ctx, 0.05);
			nk_label(ctx, "Name:", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);

			nk_layout_row_push(ctx, 0.30);
			nk_edit_string_zero_terminated(ctx, NK_EDIT_SIMPLE, st.Game_Sprites[i].name, 64, nk_filter_ascii_r);
			nk_layout_row_end(ctx);


			nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 3);
			nk_layout_row_push(ctx, 0.05);
			nk_label(ctx, "MGG: ", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);

			nk_layout_row_push(ctx, 0.50);
			nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, mgg_game[st.Game_Sprites[i].MGG_ID].name, 32, nk_filter_ascii);

			nk_layout_row_push(ctx, 0.10);
			if (nk_button_label(ctx, "Browse"))
			{
				for (i = 0; i < MAX_GAME_MGG; i++)
				{
					if (i == MAX_GAME_MGG - 1 && mgg_game[i].type != NONE)
					{
						LogApp("Cannot load MGG, reached max number of game MGGs loaded");
						break;
					}

					if (mgg_game[i].type == 1)
					{
						st.Game_Sprites[mspr.selected_spr].MGG_ID = i;
						OPENFILENAME ofn;
						char path[MAX_PATH];
						ZeroMemory(&path, sizeof(path));
						ZeroMemory(&ofn, sizeof(ofn));
						ofn.lStructSize = sizeof(ofn);
						ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
						ofn.lpstrFilter = "MGG files\0*.mgg\0";
						ofn.nMaxFile = MAX_PATH;
						ofn.lpstrFile = path;
						ofn.lpstrTitle = "Select the MGG file";
						//ofn.hInstance = OFN_EXPLORER;
						ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_EXPLORER;

						if (GetOpenFileName(&ofn))
						{
							strcpy(filename, path);

							int32 lm = CheckMGGInSystem(filename), lm2;

							if (lm > 999 && lm < 1003)
							{
								LogApp("MGG already loaded in system");
								st.Game_Sprites[mspr.selected_spr].MGG_ID = lm - 1000;
							}
							
							if (lm > 9999)
							{
								LogApp("MGG already loaded in system");
								st.Game_Sprites[mspr.selected_spr].MGG_ID = lm - 10000;
							}

							if (lm == -2)
								MessageBoxRes("Error", MB_OK, StringFormat("Error: could not load MGG file - %s\nCheck mspr.log for more information", filename));

							if (lm == -1)
							{
								lm2 = LoadMGG(&mgg_game[i], filename);

								if (lm2 == -2 || lm2 == 0)
								{
									MessageBoxRes("Error", MB_OK, StringFormat("Error: could not load MGG file - %s\nCheck mspr.log for more information", filename));
								}
								else
									st.num_mgg++;
							}
						}
						break;
					}
				}
			}

			nk_layout_row_end(ctx);

			nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 4);
			nk_layout_row_push(ctx, 0.25);
			st.Game_Sprites[i].num_frames = nk_propertyd(ctx, "Number of frames:", 1, st.Game_Sprites[i].num_frames, 16384, 1, 1);

			nk_layout_row_push(ctx, 0.01);
			nk_spacing(ctx, 1);

			nk_layout_row_push(ctx, 0.12);
			if (nk_button_label(ctx, "Select frames"))
			{
				state = 1;
			}

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

			nk_layout_row_push(ctx, 0.20);
			st.Game_Sprites[i].shadow = nk_check_label(ctx, "Soft shadow texture", st.Game_Sprites[i].shadow == 1) ? 1 : 0;

			nk_layout_row_end(ctx);

			nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 4);
			nk_layout_row_push(ctx, 0.15);
			st.Game_Sprites[i].health = nk_propertyd(ctx, "Health:", -16384, st.Game_Sprites[i].health, 16384, 32, 4);

			nk_layout_row_push(ctx, 0.02);
			nk_spacing(ctx, 1);

			nk_layout_row_push(ctx, 0.04);
			nk_label(ctx, "Type:", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);

			nk_layout_row_push(ctx, 0.10);
			st.Game_Sprites[i].type = nk_option_label(ctx, "Logical", st.Game_Sprites[i].type == 0) ? 0 : st.Game_Sprites[i].type;
			st.Game_Sprites[i].type = nk_option_label(ctx, "Enemy", st.Game_Sprites[i].type == 1) ? 1 : st.Game_Sprites[i].type;
			st.Game_Sprites[i].type = nk_option_label(ctx, "Friend", st.Game_Sprites[i].type == 2) ? 2 : st.Game_Sprites[i].type;
			st.Game_Sprites[i].type = nk_option_label(ctx, "Normal", st.Game_Sprites[i].type == 3) ? 3 : st.Game_Sprites[i].type;

			nk_layout_row_end(ctx);

			nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 2);
			nk_layout_row_push(ctx, 0.30);
			st.Game_Sprites[i].body.mass = nk_propertyd(ctx, "Mass:", 0, st.Game_Sprites[i].body.mass, 65535, 10, 1);

			nk_layout_row_end(ctx);

			nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 7);
			nk_layout_row_push(ctx, 0.07);
			nk_label(ctx, "Material:", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);

			nk_layout_row_push(ctx, 0.10);
			st.Game_Sprites[i].body.material = nk_option_label(ctx, "Metal", st.Game_Sprites[i].body.material == 0) ? 0 : st.Game_Sprites[i].body.material;
			st.Game_Sprites[i].body.material = nk_option_label(ctx, "Wood", st.Game_Sprites[i].body.material == 1) ? 1 : st.Game_Sprites[i].body.material;
			st.Game_Sprites[i].body.material = nk_option_label(ctx, "Plastic", st.Game_Sprites[i].body.material == 2) ? 2 : st.Game_Sprites[i].body.material;
			st.Game_Sprites[i].body.material = nk_option_label(ctx, "Concrete", st.Game_Sprites[i].body.material == 3) ? 3 : st.Game_Sprites[i].body.material;
			st.Game_Sprites[i].body.material = nk_option_label(ctx, "Organic", st.Game_Sprites[i].body.material == 4) ? 4 : st.Game_Sprites[i].body.material;
			st.Game_Sprites[i].body.material = nk_option_label(ctx, "None", st.Game_Sprites[i].body.material == 5) ? 5 : st.Game_Sprites[i].body.material;

			nk_layout_row_end(ctx);

			nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 4);

			if (!(st.Game_Sprites[i].flags & 4))
			{
				nk_layout_row_push(ctx, 0.15);
				st.Game_Sprites[i].body.size.x = nk_propertyd(ctx, "Size X:", 0, st.Game_Sprites[i].body.size.x, 32768, 64, 8);
				st.Game_Sprites[i].body.size.y = nk_propertyd(ctx, "Size Y:", 0, st.Game_Sprites[i].body.size.y, 32768, 64, 8);

				nk_layout_row_push(ctx, 0.02);
				nk_label(ctx, "-", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_MIDDLE);
			}

			nk_layout_row_push(ctx, 0.15);
			st.Game_Sprites[i].size_m.x = nk_propertyd(ctx, "Mul X:", 0, st.Game_Sprites[i].size_m.x, 256, 1, 1);
			st.Game_Sprites[i].size_m.y = nk_propertyd(ctx, "Mul Y:", 0, st.Game_Sprites[i].size_m.y, 256, 1, 1);
			st.Game_Sprites[i].size_a.x = nk_propertyd(ctx, "Add X:", 0, st.Game_Sprites[i].size_a.x, 16384, 32, 4);
			st.Game_Sprites[i].size_a.y = nk_propertyd(ctx, "Add Y:", 0, st.Game_Sprites[i].size_a.y, 16384, 32, 4);

			nk_layout_row_end(ctx);

			nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 4);
			nk_layout_row_push(ctx, 0.04);
			nk_label(ctx, "Flags:", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);


			nk_layout_row_push(ctx, 0.10);
			st.Game_Sprites[i].flags = nk_check_flags_label(ctx, "Resizable", st.Game_Sprites[i].flags, 1);
			st.Game_Sprites[i].flags = nk_check_flags_label(ctx, "Hidden", st.Game_Sprites[i].flags, 2);
			st.Game_Sprites[i].flags = nk_check_flags_label(ctx, "Original size", st.Game_Sprites[i].flags, 4);

			nk_layout_row_push(ctx, 0.02);
			nk_spacing(ctx, 1);

			nk_layout_row_push(ctx, 0.20);
			if (nk_combo_begin_label(ctx, "Tag list", nk_vec2(150, 400)))
			{
				for (j = 0; j < st.Game_Sprites[i].num_tags + 2; j++)
				{
					if ((j == st.Game_Sprites[i].num_tags || j == st.Game_Sprites[i].num_tags + 1) && st.Game_Sprites[i].num_tags < 8)
					{
						if (j == st.Game_Sprites[i].num_tags)
						{
							nk_layout_row_dynamic(ctx, 25, 1);
							if (nk_button_label(ctx, "Add Tag", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_MIDDLE))
								st.Game_Sprites[i].num_tags++;
						}
						else
						{
							nk_layout_row_dynamic(ctx, 25, 1);
							if (nk_button_label(ctx, "Add Default Tags", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_MIDDLE))
							{
								strcpy(st.Game_Sprites[i].tag_names[st.Game_Sprites[i].num_tags], "INPUT");
								st.Game_Sprites[i].num_tags++;

								strcpy(st.Game_Sprites[i].tag_names[st.Game_Sprites[i].num_tags], "OUTPUT");
								st.Game_Sprites[i].num_tags++;
							}
						}
					}
					else
					{
						nk_layout_row_begin(ctx, NK_DYNAMIC, 25, 2);
						nk_layout_row_push(ctx, 0.85);
						nk_edit_string_zero_terminated(ctx, NK_EDIT_SIMPLE, st.Game_Sprites[i].tag_names[j], 16, nk_filter_ascii_r);

						nk_layout_row_push(ctx, 0.15);
						if (nk_button_icon_set(BIN_ICON))
						{
							ZeroMemory(st.Game_Sprites[i].tag_names[j], 16);
							
							if (j < st.Game_Sprites[i].num_tags)
							{
								for (k = j + 1; k < st.Game_Sprites[i].num_tags; k++)
								{
									strcpy(st.Game_Sprites[i].tag_names[k - 1], st.Game_Sprites[i].tag_names[k]);
									ZeroMemory(st.Game_Sprites[i].tag_names[k], 16);
								}
							}
							
							st.Game_Sprites[i].num_tags--;
						}

					}
				}

				nk_combo_end(ctx);
			}

			nk_layout_row_end(ctx);
		}
	}

	nk_end(ctx);

	if (nk_begin(ctx, "AI states", nk_rect(st.screenx * 0.15, st.screeny * 0.4, st.screenx * 0.85, st.screeny * 0.6), NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		if (mspr.selected_spr != -1)
		{
			struct nk_command_buffer *canvas;
			struct nk_rect total_space, group_pos;

			canvas = nk_window_get_canvas(ctx);
			total_space = nk_window_get_content_region(ctx);

			nk_menubar_begin(ctx);
			nk_layout_row_begin(ctx, NK_STATIC, 30, 3);

			nk_layout_row_push(ctx, 90);
			if (nk_button_label(ctx, "New state"))
			{
				st.Game_Sprites[i].num_states++;
				st.Game_Sprites[i].states[st.Game_Sprites[i].num_states - 1].used = 1;
				strcpy(st.Game_Sprites[i].states[st.Game_Sprites[i].num_states - 1].name, StringFormat("State_#%d", st.Game_Sprites[i].num_states - 1));
			}

			nk_layout_row_end(ctx);

			nk_menubar_end(ctx);

			nk_layout_space_begin(ctx, NK_STATIC, total_space.h * (64 / 5), 64);
			{
				struct nk_rect size = nk_layout_space_bounds(ctx);

				for (j = 0; j < st.Game_Sprites[i].num_states; j++)
				{
					nk_layout_space_push(ctx, nk_rect(20 + ((j % 5) * 350), (int)((j / 5) * 300) + 20, 300, 250));
					if (nk_group_begin(ctx, StringFormat("#%d - %s", j, st.Game_Sprites[i].states[j].name), NK_WINDOW_BORDER | NK_WINDOW_TITLE))
					{
						struct nk_rect gpos = nk_window_get_content_region(ctx);
						gpos.y -= 40;
						gpos.h += 50;

						if (nk_input_mouse_clicked(&ctx->input, NK_BUTTON_LEFT, gpos) && state == 0)
						{
							mspr.state_selected = j;

							if (st.Game_Sprites[i].MGG_ID != -1 && mgg_game[st.Game_Sprites[i].MGG_ID].num_anims > 0)
								mspr.anim_frame = mgg_game[st.Game_Sprites[i].MGG_ID].anim[st.Game_Sprites[i].states[j].main_anim].startID;

							if (st.Game_Sprites[i].MGG_ID != -1 && mgg_game[st.Game_Sprites[i].MGG_ID].num_anims == 0)
								mspr.curframe = st.Game_Sprites[i].states[j].main_anim;
						}

						nk_layout_row_begin(ctx, NK_DYNAMIC, 25, 2);
						nk_layout_row_push(ctx, 0.8);
						nk_edit_string_zero_terminated(ctx, NK_EDIT_SIMPLE, st.Game_Sprites[i].states[j].name, 32, nk_filter_ascii_r);

						nk_layout_row_push(ctx, 0.1);
						if (nk_button_icon_set(BIN_ICON))
						{
							st.Game_Sprites[i].states[j].used = 0;
							for (k = j + 1; k < st.Game_Sprites[i].num_states; k++)
							{
								if (st.Game_Sprites[i].states[k].used == 1)
								{
									strcpy(st.Game_Sprites[i].states[k - 1].name, st.Game_Sprites[i].states[k].name);
									st.Game_Sprites[i].states[k - 1].main_anim = st.Game_Sprites[i].states[k].main_anim;
									st.Game_Sprites[i].states[k - 1].in_transition = st.Game_Sprites[i].states[k].in_transition;
									st.Game_Sprites[i].states[k - 1].out_transition = st.Game_Sprites[i].states[k].out_transition;
									st.Game_Sprites[i].states[k - 1].in = st.Game_Sprites[i].states[k].in;
									st.Game_Sprites[i].states[k - 1].out = st.Game_Sprites[i].states[k].out;
									memcpy(st.Game_Sprites[i].states[k - 1].inputs, st.Game_Sprites[i].states[k].inputs, sizeof(int16)* 64);
									memcpy(st.Game_Sprites[i].states[k - 1].outputs, st.Game_Sprites[i].states[k].outputs, sizeof(int16)* 64);

									for (int l = 0; l < 64; l++)
									{
										if (st.Game_Sprites[i].states[k - 1].inputs[l] != -1)
											st.Game_Sprites[i].states[k - 1].inputs[l] -= 1;

										if (st.Game_Sprites[i].states[k - 1].outputs[l] != -1)
											st.Game_Sprites[i].states[k - 1].outputs[l] -= 1;
									}

									st.Game_Sprites[i].states[k - 1].used = 1;
									st.Game_Sprites[i].states[k].used = 0;

									memset(&st.Game_Sprites[i].states[k], 0, sizeof(AISTATE));
									memset(st.Game_Sprites[i].states[k].inputs, -1, sizeof(int16)* 64);
									memset(st.Game_Sprites[i].states[k].outputs, -1, sizeof(int16)* 64);

									st.Game_Sprites[i].states[k].inputs[0] = k;
									st.Game_Sprites[i].states[k].outputs[0] = k;
								}
							}

							st.Game_Sprites[i].num_states--;
						}

						nk_layout_row_end(ctx);

						nk_layout_row_dynamic(ctx, 25, 2);
						st.Game_Sprites[i].states[j].loop = nk_check_label(ctx, "Loop state?", st.Game_Sprites[i].states[j].loop == 1) ? 1 : 0;

						if (mgg_game[st.Game_Sprites[i].MGG_ID].num_anims != 0)
						{
							temp = st.Game_Sprites[i].states[j].animation;
							st.Game_Sprites[i].states[j].animation = nk_check_label(ctx, "Animation?", st.Game_Sprites[i].states[j].animation == 1) ? 1 : 0;

							if (temp != st.Game_Sprites[i].states[j].animation)
								st.Game_Sprites[i].states[j].main_anim = st.Game_Sprites[i].states[j].animation;
						}
						else
							nk_spacing(ctx, 1);

						char combobox_str[1024];
						memset(combobox_str, 0, 1024);
						if (st.Game_Sprites[i].MGG_ID != -1)
						{
							uint16 bufseek = 0;
							for (k = 0; k < mgg_game[st.Game_Sprites[i].MGG_ID].num_anims; k++)
							{
								strcpy(combobox_str + bufseek, mgg_game[st.Game_Sprites[i].MGG_ID].anim[k].name);
								bufseek += strlen(mgg_game[st.Game_Sprites[i].MGG_ID].anim[k].name) + 1;
							}
						}

						if (mgg_game[st.Game_Sprites[i].MGG_ID].num_anims != 0)
						{
							nk_layout_row_dynamic(ctx, 25, 1);

							if (st.Game_Sprites[i].states[j].animation == 1)
								st.Game_Sprites[i].states[j].main_anim = nk_combo_string(ctx, combobox_str, st.Game_Sprites[i].states[j].main_anim,
									mgg_game[st.Game_Sprites[i].MGG_ID].num_anims, 15, nk_vec2(100, mgg_game[st.Game_Sprites[i].MGG_ID].num_anims * 30));
							else
							{
								if (nk_button_label(ctx, StringFormat("Frame : %d", st.Game_Sprites[i].states[j].main_anim)))
								{
									mspr.state_selected = j;
									state = 4;
								}
							}

							if (st.Game_Sprites[i].states[j].in == 1)
							{
								nk_layout_row_dynamic(ctx, 25, 2);
								st.Game_Sprites[i].states[j].in = nk_check_label(ctx, "In transition", st.Game_Sprites[i].states[j].in == 1) ? 1 : 0;

								st.Game_Sprites[i].states[j].in_transition = nk_combo_string(ctx, combobox_str, st.Game_Sprites[i].states[j].in_transition,
									mgg_game[st.Game_Sprites[i].MGG_ID].num_anims, 15, nk_vec2(100, mgg_game[st.Game_Sprites[i].MGG_ID].num_anims * 30));
							}
							else
							{
								nk_layout_row_dynamic(ctx, 25, 1);
								st.Game_Sprites[i].states[j].in = nk_check_label(ctx, "In transition", st.Game_Sprites[i].states[j].in == 1) ? 1 : 0;
							}

							if (st.Game_Sprites[i].states[j].out == 1)
							{
								nk_layout_row_dynamic(ctx, 25, 2);
								st.Game_Sprites[i].states[j].out = nk_check_label(ctx, "Out transition", st.Game_Sprites[i].states[j].out == 1) ? 1 : 0;

								st.Game_Sprites[i].states[j].out_transition = nk_combo_string(ctx, combobox_str, st.Game_Sprites[i].states[j].out_transition,
									mgg_game[st.Game_Sprites[i].MGG_ID].num_anims, 15, nk_vec2(100, mgg_game[st.Game_Sprites[i].MGG_ID].num_anims * 30));
							}
							else
							{
								nk_layout_row_dynamic(ctx, 25, 1);
								st.Game_Sprites[i].states[j].out = nk_check_label(ctx, "Out transition", st.Game_Sprites[i].states[j].out == 1) ? 1 : 0;
							}

							nk_layout_row_dynamic(ctx, 25, 2);
						}
						else
						{
							nk_layout_row_dynamic(ctx, 25, 1);
							if (nk_button_label(ctx, StringFormat("Frame : %d", st.Game_Sprites[i].states[j].main_anim)))
							{
								mspr.state_selected = j;
								state = 4;
							}

							nk_layout_row_dynamic(ctx, 25, 2);
						}

						for (k = 0; k < 65; k++)
						{
							if (k > 0 && st.Game_Sprites[i].states[j].outputs[k] == -1) continue;

							if (k == 64)
							{
								if (nk_button_label(ctx, "Add output"))
								{
									for (uint8 l = 1; l < 64; l++)
									{
										if (st.Game_Sprites[i].states[j].outputs[l] == -1)
										{
											st.Game_Sprites[i].states[j].outputs[l] = j;
											break;
										}
									}
								}
								nk_spacing(ctx, 1);
							}
							else
							{
								st.Game_Sprites[i].states[j].outputs[k] = nk_propertyd(ctx, StringFormat("#Output #%d:", k), -1, st.Game_Sprites[i].states[j].outputs[k],
									st.Game_Sprites[i].num_states - 1, 1, 1);

								if (st.Game_Sprites[i].states[j].outputs[k] > -1)
								{
									char output_str[2048 + 64];
									memset(output_str, 0, 2048 + 64);
									for (uint16 l = 0, bufseek = 0; l < st.Game_Sprites[i].num_states; l++)
									{
										strcpy(output_str + bufseek, st.Game_Sprites[i].states[l].name);

										bufseek += strlen(st.Game_Sprites[i].states[l].name) + 1;
									}

									st.Game_Sprites[i].states[j].outputs[k] = nk_combo_string(ctx, output_str, st.Game_Sprites[i].states[j].outputs[k],
										st.Game_Sprites[i].num_states, 15, nk_vec2(200, st.Game_Sprites[i].num_states * 30));
								}
							}
						}

						nk_group_end(ctx);
					}

					//uint8 l = 0;

					struct nk_color col;

					if (mspr.state_selected == j)
						col = nk_rgb(255, 128, 32);
					else
						col = nk_rgb(100, 100, 100);

					for (k = 0; k < 64; k++)
					{
						if (st.Game_Sprites[i].states[j].outputs[k] != -1 && st.Game_Sprites[i].states[j].outputs[k] != j)
						{
							//l++;
							struct nk_vec2 s1, s2;

							s2.x = 20 + (350 * (st.Game_Sprites[i].states[j].outputs[k] % 5)) + 150;

							if (st.Game_Sprites[i].states[j].outputs[k] < j && (st.Game_Sprites[i].states[j].outputs[k] / 5) == (j / 5))
							{
								s1.x = 20 + ((j % 5) * 350);
								s1.y = 20 + ((j / 5) * 300) + 4 + 250;
								s2.y = 20 + ((int)(st.Game_Sprites[i].states[j].outputs[k] / 5) * 300) + 4 + 250;
							}

							if (st.Game_Sprites[i].states[j].outputs[k] > j && (st.Game_Sprites[i].states[j].outputs[k] / 5) == (j / 5))
							{
								s1.x = 20 + ((j % 5) * 350) + 300;
								s1.y = 20 + ((j / 5) * 300) + 4;
								s2.y = 20 + ((int)(st.Game_Sprites[i].states[j].outputs[k] / 5) * 300) + 4;
							}

							if (st.Game_Sprites[i].states[j].outputs[k] < j && (st.Game_Sprites[i].states[j].outputs[k] / 5) < (j / 5))
							{
								s1.x = 20 + ((j % 5) * 350) + 300;
								s1.y = 20 + ((j / 5) * 300) + 4;
								s2.y = 20 + ((int)(st.Game_Sprites[i].states[j].outputs[k] / 5) * 300) + 4 + 250;
							}

							if (st.Game_Sprites[i].states[j].outputs[k] > j && (st.Game_Sprites[i].states[j].outputs[k] / 5) > (j / 5))
							{
								s1.x = 20 + ((j % 5) * 350);
								s1.y = 20 + ((j / 5) * 300) + 4 + 250;
								s2.y = 20 + ((int)(st.Game_Sprites[i].states[j].outputs[k] / 5) * 300) + 4;
							}

							struct nk_rect b = nk_layout_space_rect_to_screen(ctx, nk_rect(s1.x, s1.y, 0, 0));
							struct nk_rect c = nk_layout_space_rect_to_screen(ctx, nk_rect(s2.x, s2.y, 0, 0));

							if (st.Game_Sprites[i].states[j].outputs[k] < j && (st.Game_Sprites[i].states[j].outputs[k] / 5) == (j / 5))
								nk_stroke_curve(canvas, b.x, b.y, b.x, b.y + 25, c.x, c.y + 25, c.x, c.y, 1.0f, col);

							if (st.Game_Sprites[i].states[j].outputs[k] > j && (st.Game_Sprites[i].states[j].outputs[k] / 5) == (j / 5))
								nk_stroke_curve(canvas, b.x, b.y, b.x, b.y - 25, c.x, c.y - 25, c.x, c.y, 1.0f, col);

							if (st.Game_Sprites[i].states[j].outputs[k] < j && (st.Game_Sprites[i].states[j].outputs[k] / 5) < (j / 5))
								nk_stroke_curve(canvas, b.x, b.y, b.x, b.y - 25, c.x, c.y + 25, c.x, c.y, 1.0f, col);

							if (st.Game_Sprites[i].states[j].outputs[k] > j && (st.Game_Sprites[i].states[j].outputs[k] / 5) > (j / 5))
								nk_stroke_curve(canvas, b.x, b.y, b.x, b.y + 25, c.x, c.y - 25, c.x, c.y, 1.0f, col);
						}
					}

					struct nk_vec2 s1, s2;

					s1.x = 20 + ((j % 5) * 350) + 300 - 4;
					s1.y = 20 + ((int)(j / 5) * 300) - 4;

					s2.x = 20 + ((int)(j % 5) * 350) + 150 - 4;
					s2.y = 20 + +((int)(j / 5) * 300) + 250 - 4;

					struct nk_rect b = nk_layout_space_rect_to_screen(ctx, nk_rect(s1.x, s1.y, 0, 0));
					struct nk_rect c = nk_layout_space_rect_to_screen(ctx, nk_rect(s2.x, s2.y, 0, 0));
					struct nk_rect d = nk_layout_space_rect_to_screen(ctx, nk_rect(20 + ((j % 5) * 350) - 4, 0, 0, 0));

					nk_fill_circle(canvas, nk_rect(b.x, b.y, 8, 8), col);
					nk_fill_circle(canvas, nk_rect(c.x, c.y, 8, 8), col);

					nk_fill_circle(canvas, nk_rect(c.x, b.y, 8, 8), col);
					nk_fill_circle(canvas, nk_rect(d.x, c.y, 8, 8), col);
				}
			}
			nk_layout_space_end(ctx);
		}
	}

	nk_end(ctx);

	if (nk_begin(ctx, "Canvas", nk_rect(st.screenx * 0.7, 30, st.screenx * 0.3, st.screeny * 0.4 - 30), NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
	{
		struct nk_rect vec4;
		float x2, y2, x3, y3, x4, y4;
		static float zoom = 2.0f;
		int32 x, y;
		char str[8];
		_MGG mgg = mgg_game[st.Game_Sprites[i].MGG_ID];

		nk_layout_space_begin(ctx, NK_DYNAMIC, st.screeny * 0.28f, 7);

		vec4 = nk_layout_space_bounds(ctx);

		//Grid
		for (i = 0; i < vec4.w; i += 32)
			nk_stroke_line(nk_window_get_canvas(ctx), i + vec4.x, vec4.y, i + vec4.x, vec4.y + vec4.h, 1.0f, nk_rgb(128, 128, 128));
		for (i = 0; i < vec4.h; i += 32)
			nk_stroke_line(nk_window_get_canvas(ctx), vec4.x, i + vec4.y, vec4.w + vec4.x, i + vec4.y, 1.0f, nk_rgb(128, 128, 128));

		nk_stroke_line(nk_window_get_canvas(ctx), vec4.x + (vec4.w / 2), vec4.y, vec4.x + (vec4.w / 2), vec4.y + vec4.h, 3.0f, nk_rgb(255, 0, 0));
		nk_stroke_line(nk_window_get_canvas(ctx), vec4.x, vec4.y + (vec4.h / 2), vec4.x + vec4.w, vec4.y + (vec4.h / 2), 3.0f, nk_rgb(255, 0, 0));

		i = mspr.selected_spr;

		if (mgg.type == MGG_USED)
		{
			if (mspr.state_selected != -1)
			{
				if (st.Game_Sprites[i].states[mspr.state_selected].animation == 1)
					mspr.curframe = mspr.anim_frame;
				else
					mspr.curframe = st.Game_Sprites[i].states[mspr.state_selected].main_anim;
			}

			x = mgg.frames[mspr.curframe].x_offset;
			y = mgg.frames[mspr.curframe].y_offset;

			WTS(&x, &y);

			vec4.x = ((vec4.w / 2));
			vec4.y = ((vec4.h / 2));

			x2 = (vec4.x) / vec4.w;
			y2 = (vec4.y) / vec4.h;

			if (st.Game_Sprites[i].flags & 4)
			{
				float ax, ay;
				ax = ((float)(mgg.frames[mspr.curframe].sizex * (st.Game_Sprites[i].size_m.x == 0 ? 1 : st.Game_Sprites[i].size_m.x)
					+ st.Game_Sprites[i].size_a.x) / 32768.0f) * mgg.frames[mspr.curframe].w;
				ay = ((float)(mgg.frames[mspr.curframe].sizey * (st.Game_Sprites[i].size_m.y == 0 ? 1 : st.Game_Sprites[i].size_m.y)
					+ st.Game_Sprites[i].size_a.y) / 32768.0f) * mgg.frames[mspr.curframe].h;

				STWf(&ax, &ay);

				x3 = (ax * zoom * 0.06f) / vec4.w;
				y3 = (ay * zoom * 0.06f) / vec4.h;
			}
			else
			{
				x3 = ((st.Game_Sprites[i].body.size.x * (st.Game_Sprites[i].size_m.x == 0 ? 1 : st.Game_Sprites[i].size_m.x)
					+ st.Game_Sprites[i].size_a.x) * zoom * 0.06f) / vec4.w;
				y3 = ((st.Game_Sprites[i].body.size.y * (st.Game_Sprites[i].size_m.y == 0 ? 1 : st.Game_Sprites[i].size_m.y)
					+ st.Game_Sprites[i].size_a.y) * zoom * 0.06f) / vec4.h;
			}

			x4 = (x * zoom * 0.06f) / vec4.w;
			y4 = (y * zoom * 0.06f) / vec4.w;

			nk_layout_space_push(ctx, nk_rect(x2 - (x3 / 2) + x4, y2 - (y3 / 2) + y4, x3, y3));

			float ax = ((float)mgg.frames[mspr.curframe].posx / 32768.0f) * mgg.frames[mspr.curframe].w;
			float ay = ((float)mgg.frames[mspr.curframe].posy / 32768.0f) * mgg.frames[mspr.curframe].h;
			float bx = ((float)mgg.frames[mspr.curframe].sizex / 32768.0f) * mgg.frames[mspr.curframe].w;
			float by = ((float)mgg.frames[mspr.curframe].sizey / 32768.0f) * mgg.frames[mspr.curframe].h;

			if (mgg.frames[mspr.curframe].vb_id != -1)
				nk_image(ctx, nk_subimage_id(mgg.frames[mspr.curframe].data, mgg.frames[mspr.curframe].w, mgg.frames[mspr.curframe].h, nk_rect(ax, ay, bx, by)));
			else
				nk_image(ctx, nk_image_id(mgg.frames[mspr.curframe].data));

			nk_layout_space_push(ctx, nk_rect(0.6f, 0.00f, 0.1f, 0.1f));
			if (nk_button_symbol(ctx, NK_SYMBOL_MINUS))
				zoom -= 0.1f;

			nk_layout_space_push(ctx, nk_rect(0.7f, 0.00f, 0.2f, 0.1f));
			sprintf(str, "%.2f%%", (zoom / 2.0f) * 100.0f);
			//ctx->style.button.normal = ctx->style.button.active = ctx->style.button.hover;
			if (nk_button_label(ctx, str))
				zoom = 2.0f;
			//SetThemeBack();

			nk_layout_space_push(ctx, nk_rect(0.9f, 0.00f, 0.1f, 0.1f));
			if (nk_button_symbol(ctx, NK_SYMBOL_PLUS))
				zoom += 0.1f;

			nk_layout_space_push(ctx, nk_rect(0.6f, 0.1f, 0.4f, 0.1f));
			nk_slider_float(ctx, 0.05f, &zoom, 5.0f, 0.01f);

			nk_layout_space_end(ctx);

			if (zoom > 5.0f)
				zoom = 5.0f;

			if (zoom < 0.05f)
				zoom = 0.05f;

			if (mspr.state_selected != -1 && mgg.num_anims > 0 && st.Game_Sprites[i].states[mspr.state_selected].animation == 1)
			{
				nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 5);
				nk_style_set_font(ctx, &fonts[1]->handle);

				nk_layout_row_push(ctx, 0.1f);
				nk_spacing(ctx, 1);
				if (mspr.anim_frame == mgg.anim[st.Game_Sprites[i].states[mspr.state_selected].main_anim].startID)
				{
					ctx->style.button.normal = ctx->style.button.active = ctx->style.button.hover;
					nk_button_label(ctx, "E");
					SetThemeBack();
				}
				else
				{
					if (nk_button_label(ctx, "E"))
					{
						if (mspr.anim_frame > mgg.anim[st.Game_Sprites[i].states[mspr.state_selected].main_anim].startID - 1)
							mspr.anim_frame--;
					}
				}

				nk_layout_row_push(ctx, 0.2f);
				if (nk_button_label(ctx, "D"))
				{
					mspr.play = 0;
					mspr.anim_frame = mgg.anim[st.Game_Sprites[i].states[mspr.state_selected].main_anim].startID;
				}

				nk_layout_row_push(ctx, 0.2f);
				if (mspr.play)
				{
					ctx->style.button.normal = ctx->style.button.active = ctx->style.button.hover;
					nk_button_label(ctx, "B");
				}
				else
				{
					if (nk_button_label(ctx, "B"))
						mspr.play = 1;
				}

				SetThemeBack();

				nk_layout_row_push(ctx, 0.2f);
				if (!mspr.play)
				{
					ctx->style.button.normal = ctx->style.button.active = ctx->style.button.hover;
					nk_button_label(ctx, "C");
				}
				else
				{
					if (nk_button_label(ctx, "C"))
						mspr.play = 0;
				}

				SetThemeBack();

				nk_layout_row_push(ctx, 0.1f);
				if (mspr.anim_frame == mgg.anim[st.Game_Sprites[i].states[mspr.state_selected].main_anim].endID)
				{
					ctx->style.button.normal = ctx->style.button.active = ctx->style.button.hover;
					nk_button_label(ctx, "F");
				}
				else
				{
					if (nk_button_label(ctx, "F"))
					{
						if (mspr.anim_frame < mgg.anim[st.Game_Sprites[i].states[mspr.state_selected].main_anim].endID + 1)
							mspr.anim_frame++;
					}
				}

				SetThemeBack();

				nk_spacing(ctx, 1);

				nk_style_set_font(ctx, &fonts[0]->handle);
				//nk_style_set_font(ctx, &fonts[2]->handle);

				nk_layout_row_end(ctx);
			}
		}
	}

	nk_end(ctx);

	if (state == 1)
	{
		if (FrameListSelection(NULL) == 1)
			state = 0;
	}

	if (state == 4)
	{
		if (FrameListSelection(1) == 1)
			state = 0;
	}
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

	strcpy(st.WindowTitle, "Sprite");

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

	ctx = nk_sdl_init(wn, MAX_NK_BUFFER, MAX_NK_VERTEX_BUFFER, MAX_NK_ELEMENT_BUFFER);
	
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

	mspr.anim_frame = mspr.selected_spr = mspr.state_selected = mspr.curframe = -1;

	memset(st.Game_Sprites, 0, MAX_SPRITES*sizeof(_SPRITES));

	for (int i = 0; i < MAX_SPRITES; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			memset(st.Game_Sprites[i].states[j].inputs, -1, sizeof(int16)* 64);
			memset(st.Game_Sprites[i].states[j].outputs, -1, sizeof(int16)* 64);

			st.Game_Sprites[i].states[j].inputs[0] = j;
			st.Game_Sprites[i].states[j].outputs[0] = j;
		}
	}

	memcpy(mspr.sprbck, st.Game_Sprites, MAX_SPRITES * sizeof(_SPRITES));

	mspr.undostates = calloc(mspr.max_undo_states + 1, 1);

	mspr.sprunstate = calloc(mspr.max_undo_states + 1, sizeof(_SPRITES));

	memcpy(mspr.sprunstate, st.Game_Sprites, mspr.max_undo_states + 1 * sizeof(_SPRITES));

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

		BASICBKD(255, 255, 255);
		
		loops=0;
		while(GetTicks() > curr_tic && loops < 10)
		{
			//nk_clear(ctx);
			Finish();

			if (mspr.play == 1 && mspr.state_selected != -1 && st.Game_Sprites[mspr.selected_spr].states[mspr.state_selected].animation == 1)
			{
				_MGGANIM *mga = &mgg_game[st.Game_Sprites[mspr.selected_spr].MGG_ID].anim[st.Game_Sprites[mspr.selected_spr].states[mspr.state_selected].main_anim];
				if (mga->speed > 0)
				{
					if (mga->current_frame < mga->startID * 100)
						mga->current_frame = mga->startID * 100;
				}
				else
				{
					if (mga->current_frame >mga->startID * 100)
						mga->current_frame = mga->startID * 100;
				}

				mga->current_frame += mga->speed;

				if (mga->speed > 0)
				{
					if (mga->current_frame >=mga->endID * 100)
						mga->current_frame = mga->startID * 100;
				}
				else
				{
					if (mga->current_frame <=mga->endID * 100)
						mga->current_frame = mga->startID * 100;
				}

				mspr.anim_frame = mga->current_frame / 100;
			}

			curr_tic+=1000/TICSPERSECOND;
			loops++;
			SetTimerM(1);
		}

		//DrawSys();

		Pannel();


		MenuBar();

		//UIMain_DrawSystem();
		//MainSound();
		Renderer(0);

		float bg[4];
		nk_color_fv(bg, background);

		nk_sdl_render(NK_ANTI_ALIASING_OFF, MAX_NK_VERTEX_BUFFER, MAX_NK_ELEMENT_BUFFER, MAX_NK_COMMAND_BUFFER);

		SwapBuffer(wn);

		nkrendered = 0;

		if (mspr.changes_detected == 0)
			CheckForChanges();

		UndoState();
	}

	Quit();
	return 1;
}