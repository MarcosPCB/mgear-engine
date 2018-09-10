#include <Windows.h>
#include <commdlg.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include "input.h"
#include "main.h"
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
//#include <atlstr.h>
#include "dirent.h"
#include "UI.h"
#include <crtdbg.h>
#include "funcs.h"
#include <SDL_syswm.h>

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
struct nk_font *fonts[3];

int prev_tic, curr_tic, delta;

mTex mtex;

int16 LoadIntApps()
{
	FILE *f;
	char buf[2048], str[128], str2[2048], *buf2, buf3[2048];

	memset(mtex.intg_app, 0, sizeof(struct IntApps) * 16);

	if ((f = fopen("msdk_settings.cfg", "r")) == NULL)
	{
		LogApp("Could not load integrated apps");
		return NULL;
	}

	while (!feof(f))
	{
		fgets(buf, sizeof(buf), f);
		sscanf(buf, "%s = %d", str);
		if (strcmp(str, "PS") == NULL)
		{
			memcpy(buf3, buf, 2048);
			buf2 = strtok(buf3, "\"");
			buf2 = strtok(NULL, "\"");
			alloc_mem(mtex.intg_app[INTG_PS].path, MAX_PATH);
			strcpy(mtex.intg_app[INTG_PS].path, buf2);

			continue;
		}

		if (strcmp(str, "PS_a") == NULL)
		{
			memcpy(buf3, buf, 2048);
			buf2 = strtok(buf3, "\"");
			buf2 = strtok(NULL, "\"");
			alloc_mem(mtex.intg_app[INTG_PS].args, MAX_PATH * 2);
			strcpy(mtex.intg_app[INTG_PS].args, buf2);

			continue;
		}

		if (strcmp(str, "IL") == NULL)
		{
			memcpy(buf3, buf, 2048);
			buf2 = strtok(buf3, "\"");
			buf2 = strtok(NULL, "\"");
			alloc_mem(mtex.intg_app[INTG_IL].path, MAX_PATH);
			strcpy(mtex.intg_app[INTG_IL].path, buf2);

			continue;
		}

		if (strcmp(str, "IL_a") == NULL)
		{
			memcpy(buf3, buf, 2048);
			buf2 = strtok(buf3, "\"");
			buf2 = strtok(NULL, "\"");
			alloc_mem(mtex.intg_app[INTG_IL].args, MAX_PATH * 2);
			strcpy(mtex.intg_app[INTG_IL].args, buf2);

			continue;
		}

		if (strcmp(str, "GP") == NULL)
		{
			memcpy(buf3, buf, 2048);
			buf2 = strtok(buf3, "\"");
			buf2 = strtok(NULL, "\"");
			alloc_mem(mtex.intg_app[INTG_GP].path, MAX_PATH);
			strcpy(mtex.intg_app[INTG_GP].path, buf2);

			continue;
		}

		if (strcmp(str, "GP_a") == NULL)
		{
			memcpy(buf3, buf, 2048);
			buf2 = strtok(buf3, "\"");
			buf2 = strtok(NULL, "\"");
			alloc_mem(mtex.intg_app[INTG_GP].args, MAX_PATH * 2);
			strcpy(mtex.intg_app[INTG_GP].args, buf2);

			continue;
		}
	}

	if (mtex.intg_app[INTG_PS].path != NULL && mtex.intg_app[INTG_PS].args != NULL)
	{
		strcpy(mtex.intg_app[INTG_PS].name, INTG_PS_NAME);
		mtex.intg_app[INTG_PS].valid = 1;
	}

	if (mtex.intg_app[INTG_IL].path != NULL && mtex.intg_app[INTG_IL].args != NULL)
	{
		strcpy(mtex.intg_app[INTG_IL].name, INTG_IL_NAME);
		mtex.intg_app[INTG_IL].valid = 1;
	}

	if (mtex.intg_app[INTG_GP].path != NULL && mtex.intg_app[INTG_GP].args != NULL)
	{
		strcpy(mtex.intg_app[INTG_GP].name, INTG_GP_NAME);
		mtex.intg_app[INTG_GP].valid = 1;
	}
}

uint16 WriteCFG()
{
	FILE *file;

	if((file=fopen("mtex_settings.cfg","w"))==NULL)
		return 0;

	st.screenx=1280;
	st.screeny=720;
	st.fullscreen=0;
	st.bpp=32;
	st.audiof=44100;
	st.audioc=2;
	st.vsync=0;
	mtex.theme = 0;

	fprintf(file,"ScreenX = %d\n",st.screenx);
	fprintf(file,"ScreenY = %d\n",st.screeny);
	fprintf(file,"FullScreen = %d\n",st.fullscreen);
	fprintf(file,"ScreenBPP = %d\n",st.bpp);
	fprintf(file,"AudioFrequency = %d\n",st.audiof);
	fprintf(file,"AudioChannels = %d\n",st.audioc);
	fprintf(file,"VSync = %d\n",st.vsync);
	fprintf(file, "Theme = %d\n", mtex.theme);

	fclose(file);

	return 1;
}

uint16 SaveCFG()
{
	FILE *file;

	if ((file = fopen("mtex_settings.cfg", "w")) == NULL)
		return 0;

	fprintf(file, "ScreenX = %d\n", st.screenx);
	fprintf(file, "ScreenY = %d\n", st.screeny);
	fprintf(file, "FullScreen = %d\n", st.fullscreen);
	fprintf(file, "ScreenBPP = %d\n", st.bpp);
	fprintf(file, "AudioFrequency = %d\n", st.audiof);
	fprintf(file, "AudioChannels = %d\n", st.audioc);
	fprintf(file, "VSync = %d\n", st.vsync);
	fprintf(file, "Theme = %d\n", mtex.theme);

	fclose(file);

	return 1;
}

uint16 LoadCFG()
{
	FILE *file;
	char buf[2048], str[128], str2[2048], *buf2, buf3[2048];
	int value=0;
	if ((file = fopen("mtex_settings.cfg", "r")) == NULL)
	{
		if (WriteCFG() == 0)
			return 0;

		if ((file = fopen("mtex_settings.cfg", "r")) == NULL)
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
		if (strcmp(str, "Theme") == NULL) mtex.theme = value;
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

void SetThemeBack()
{
	switch (mtex.theme)
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

void OpenWithIntApp(const char *file, int8 app)
{
	//char directory[MAX_PATH];
	char exepath[MAX_PATH];
	char args[MAX_PATH * 3];
	char str[512];
	static path2[MAX_PATH];
	MSG msg;

	DWORD error;

	static int state = 0;
	DWORD exitcode;

	static SHELLEXECUTEINFO info;
	strcpy(exepath, mtex.intg_app[app].path);

	strcpy(args, mtex.intg_app[app].args);

	char *buf2 = strstr(args, "%1");
	ZeroMemory(buf2, strlen(buf2));

	strcat(args, StringFormat(" \"%s\"",file));

	ZeroMemory(&info, sizeof(info));
	info.cbSize = sizeof(info);
	info.lpVerb = ("open");
	info.fMask = SEE_MASK_NOCLOSEPROCESS; //| SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC;
	info.lpFile = exepath;
	info.lpParameters = args;
	//info.lpDirectory = directory;
	info.nShow = SW_SHOW;
	if (!ShellExecuteEx(&info))
		MessageBoxRes("Error", MB_OK, "Could not execute %",mtex.intg_app[app].name);
}

FILETIME *GetFileLastModification(const char *filename)
{
	FILETIME ft;
	HANDLE f;

	if ((f = CreateFile(filename, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		return NULL;

	if (GetFileTime(f, NULL, NULL, &ft) == NULL)
	{
		CloseHandle(f);
		return NULL;
	}

	CloseHandle(f);

	return &ft;
}

int8 CheckFilesForModification(int16 file, int8 normal)
{
	HANDLE f;
	FILETIME ft;
	SYSTEMTIME st, st2;

	if (normal == NULL)
	{
		if ((f = CreateFile(mtex.mgg.files[mtex.mgg.fn[file]], GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
			return NULL;
	}
	else
	{
		if ((f = CreateFile(mtex.mgg.files[mtex.mgg.fnn[file]], GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
			return NULL;
	}

	if (GetFileTime(f, NULL, NULL, &ft) == NULL)
	{
		CloseHandle(f);
		return NULL;
	}

	CloseHandle(f);

	if (FileTimeToSystemTime(&ft, &st) == NULL)
		return NULL;

	if (normal == NULL)
	{
		if (FileTimeToSystemTime(&mtex.mgg.ftime[mtex.mgg.fn[file]], &st2) == NULL)
			return NULL;
	}
	else
	{
		if (FileTimeToSystemTime(&mtex.mgg.ftime_n[mtex.mgg.fnn[file]], &st2) == NULL)
			return NULL;
	}

	if (st.wYear != st2.wYear || st.wMonth != st2.wMonth || st.wDay != st2.wDay || st.wHour != st2.wHour || st.wMinute != st2.wMinute ||
		st.wSecond != st2.wSecond || st.wMilliseconds != st2.wMilliseconds)
	{
		if (normal == NULL) mtex.mgg.ftime[mtex.mgg.fn[file]] = ft;
		else mtex.mgg.ftime_n[mtex.mgg.fnn[file]] = ft;

		return 1;
	}
	else
		return -1;

}

void UpdateFiles()
{
	register int16 i, j;

	for (i = 0; i < mtex.mgg.num_frames; i++)
	{
		if (CheckFilesForModification(i, 0) == 1)
		{
			glDeleteTextures(1, &mtex.textures[mtex.mgg.fn[i]]);
			if ((mtex.textures[mtex.mgg.fn[i]] = LoadTexture(mtex.mgg.files[mtex.mgg.fn[i]], mtex.mgg.mipmap, &mtex.size[mtex.mgg.fn[i]])) == -1)
			{
				MessageBoxRes("Refresh error", MB_OK, "Error while refreshing texture: %s", mtex.mgg.files[mtex.mgg.fn[i]]);
				continue;
			}
		}

		if (CheckFilesForModification(i, 1) == 1)
		{
			glDeleteTextures(1, &mtex.textures[mtex.mgg.fnn[i]]);
			if ((mtex.textures_n[mtex.mgg.fnn[i]] = LoadTexture(mtex.mgg.files[mtex.mgg.fnn[i]], mtex.mgg.mipmap, &mtex.size[mtex.mgg.fnn[i]])) == -1)
			{
				MessageBoxRes("Refresh error", MB_OK, "Error while refreshing texture: %s", mtex.mgg.files[mtex.mgg.fnn[i]]);
				continue;
			}
		}
	}
}

void UnloadmTexMGG()
{
	register int i;
	if (mtex.mgg.num_frames > 0)
	{

		for (i = 0; i < mtex.mgg.num_frames;i++)
		{
			if (mtex.mgg.fn[i] != -1)
				glDeleteTextures(1, &mtex.textures[i]);
		}

		if (mtex.textures)
			free(mtex.textures);

		for (i = 0; i < mtex.mgg.num_frames; i++)
		{
			if (mtex.mgg.fnn[i] != -1)
				glDeleteTextures(1, &mtex.textures_n[i]);
		}

		if (mtex.textures_n)
			free(mtex.textures_n);

		if (mtex.size)
			free(mtex.size);

		if (mtex.mgg.fn)
			free(mtex.mgg.fn);

		if (mtex.mgg.fnn)
			free(mtex.mgg.fnn);

		if (mtex.mgg.frames_atlas)
			free(mtex.mgg.frames_atlas);

		if (mtex.mgg.num_anims > 0)
		{
			if (mtex.mgg.mga)
				free(mtex.mgg.mga);

			if (mtex.mgg.an)
				free(mtex.mgg.an);
		}

		if (mtex.mgg.num_c_atlas > 0)
		{
			if (mtex.mgg.num_f_a)
				free(mtex.mgg.num_f_a);

			if (mtex.mgg.num_f0)
				free(mtex.mgg.num_f0);

			if (mtex.mgg.num_ff)
				free(mtex.mgg.num_ff);
		}

		memset(&mtex.mgg, 0, sizeof(mtex.mgg));

		memset(mtex.selection, 0, 512 * sizeof(int));

		mtex.selected = -1;
		mtex.anim_selected = -1;
		mtex.mult_selection = 0;
		mtex.dn_mode = 0;
		mtex.canvas = 0;
	}
}

void UpdateTexList()
{
	int16 new_fn[512], new_fnn[512], frames = 0, framesn = 0;
	int8 fa[512];
	register int i, j, k;

	memset(new_fn, -1, 512 * sizeof(int16));
	memset(fa, -1, 512 * sizeof(int8));

	for (i = 0, k = 0, j = 0; k < mtex.mgg.num_frames; i++, k++, j++)
	{
		if (mtex.mgg.fn[i] < 1024)
		{
			new_fn[i] = mtex.mgg.fn[k];
			new_fnn[i] = mtex.mgg.fnn[k];

			if (new_fnn[i] != -1 && new_fnn[i] < 1024) framesn++;

			fa[i] = mtex.mgg.frames_atlas[k];
			frames++;
		}
		else
		{
			for (; j < mtex.mgg.num_frames; j++)
			{
				if (mtex.mgg.fn[j] < 1024)
				{
					new_fn[i] = mtex.mgg.fn[j];
					new_fnn[i] = mtex.mgg.fnn[j];

					if (new_fnn[i] != -1 && new_fnn[i] < 1024) framesn++;

					fa[i] = mtex.mgg.frames_atlas[j];
					//i = j - 1;
					k = j;
					frames++;
					break;
				}
			}
		}
	}

	memcpy(mtex.mgg.fnn, new_fnn, mtex.mgg.num_frames * sizeof(int16));
	memcpy(mtex.mgg.fn, new_fn, mtex.mgg.num_frames * sizeof(int16));
	memcpy(mtex.mgg.frames_atlas, fa, mtex.mgg.num_frames * sizeof(int8));

	for (i = 0; i < mtex.mgg.num_anims; i++)
	{
		if (mtex.mgg.mga[i].startID == mtex.mgg.mga[i].endID)
			mtex.mgg.an[i] += 1024;
	}

	for (i = 0; i < mtex.mgg.num_c_atlas; i++)
	{
		for (j = 0, k = 0; j < mtex.mgg.num_frames; j++)
		{
			if (mtex.mgg.frames_atlas[j] == i && mtex.mgg.frames_atlas[j - 1] != i)
			{
				mtex.mgg.num_f0[i] = j;
				k++;
			}

			if (mtex.mgg.frames_atlas[j] == i && mtex.mgg.frames_atlas[j + 1] != i)
			{
				mtex.mgg.num_ff[i] = j;
				if (mtex.mgg.num_ff[i] != mtex.mgg.num_f0[i]) k++;
				else mtex.mgg.frames_atlas[j] = -1;
				break;
			}
		}

		if (k == 1 || !k)
		{
			//This is an invalid atlas
			//Pull all the other atlas one slot down
			//But before doing this, check if there are any atlasses left

			if (i == mtex.mgg.num_c_atlas - 1 && mtex.mgg.num_c_atlas == 1)
			{
				free(mtex.mgg.num_f_a);
				free(mtex.mgg.num_f0);
				free(mtex.mgg.num_ff);

				mtex.mgg.num_c_atlas--;
			}
			
			if (i == mtex.mgg.num_c_atlas - 1 && mtex.mgg.num_c_atlas > 1)
			{
				mtex.mgg.num_f_a = realloc(mtex.mgg.num_f_a, (mtex.mgg.num_c_atlas - 1) * sizeof(int16));
				mtex.mgg.num_f0 = realloc(mtex.mgg.num_f0, (mtex.mgg.num_c_atlas - 1) * sizeof(int16));
				mtex.mgg.num_ff = realloc(mtex.mgg.num_ff, (mtex.mgg.num_c_atlas - 1) * sizeof(int16));

				mtex.mgg.num_c_atlas--;
			}
			
			if (i < mtex.mgg.num_c_atlas)
			{
				for (j = i + 1; j < mtex.mgg.num_c_atlas; j++)
				{
					mtex.mgg.num_f_a[j - 1] = mtex.mgg.num_f_a[j];
					mtex.mgg.num_f0[j - 1] = mtex.mgg.num_f0[j];
					mtex.mgg.num_ff[j - 1] = mtex.mgg.num_ff[j];

					for (k = 0; k < mtex.mgg.num_frames; k++)
					{
						if (mtex.mgg.frames_atlas[k] == i + 1)
							mtex.mgg.frames_atlas[k] = i;
					}
				}

				mtex.mgg.num_f_a = realloc(mtex.mgg.num_f_a, (mtex.mgg.num_c_atlas - 1) * sizeof(int16));
				mtex.mgg.num_f0 = realloc(mtex.mgg.num_f0, (mtex.mgg.num_c_atlas - 1) * sizeof(int16));
				mtex.mgg.num_ff = realloc(mtex.mgg.num_ff, (mtex.mgg.num_c_atlas - 1) * sizeof(int16));

				mtex.mgg.num_c_atlas--;
			}

		}
	}

	/*
	for (i = 0, k = 0, j = 0; k < mtex.mgg.num_frames; i++, k++, j++)
	{
		if (mtex.mgg.fnn[i] < 1024)
		{
			new_fn[i] = mtex.mgg.fnn[k];
			framesn++;
		}
		else
		{
			for (; j < mtex.mgg.num_frames; j++)
			{
				if (mtex.mgg.fnn[j] < 1024)
				{
					new_fn[i] = mtex.mgg.fnn[j];
					//i = j - 1;
					k = j;
					framesn++;
					break;
				}
			}
		}
	}
	*/

	mtex.mgg.num_frames = frames;
	mtex.mgg.num_f_n = framesn;

	LogApp("Updated texture list");
}

void UpdateAtlasses()
{
	register int i, j, k;

	for (i = 0; i < mtex.mgg.num_c_atlas; i++)
	{
		for (j = 0, k = 0; j < mtex.mgg.num_frames; j++)
		{
			if (mtex.mgg.frames_atlas[j] == i && mtex.mgg.frames_atlas[j - 1] != i)
			{
				mtex.mgg.num_f0[i] = j;
				k++;
			}

			if (mtex.mgg.frames_atlas[j] == i && mtex.mgg.frames_atlas[j + 1] != i)
			{
				mtex.mgg.num_ff[i] = j;
				if (mtex.mgg.num_ff[i] != mtex.mgg.num_f0[i]) k++;
				else mtex.mgg.frames_atlas[j] = -1;
				break;
			}
		}

		if (k == 1 || !k)
		{
			//This is an invalid atlas
			//Pull all the other atlas one slot down
			//But before doing this, check if there are any atlasses left

			if (i == mtex.mgg.num_c_atlas - 1 && mtex.mgg.num_c_atlas == 1)
			{
				free(mtex.mgg.num_f_a);
				free(mtex.mgg.num_f0);
				free(mtex.mgg.num_ff);

				mtex.mgg.num_c_atlas--;
			}

			if (i == mtex.mgg.num_c_atlas - 1 && mtex.mgg.num_c_atlas > 1)
			{
				mtex.mgg.num_f_a = realloc(mtex.mgg.num_f_a, (mtex.mgg.num_c_atlas - 1) * sizeof(int16));
				mtex.mgg.num_f0 = realloc(mtex.mgg.num_f0, (mtex.mgg.num_c_atlas - 1) * sizeof(int16));
				mtex.mgg.num_ff = realloc(mtex.mgg.num_ff, (mtex.mgg.num_c_atlas - 1) * sizeof(int16));

				mtex.mgg.num_c_atlas--;
			}

			if (i < mtex.mgg.num_c_atlas)
			{
				for (j = i + 1; j < mtex.mgg.num_c_atlas; j++)
				{
					mtex.mgg.num_f_a[j - 1] = mtex.mgg.num_f_a[j];
					mtex.mgg.num_f0[j - 1] = mtex.mgg.num_f0[j];
					mtex.mgg.num_ff[j - 1] = mtex.mgg.num_ff[j];

					for (k = 0; k < mtex.mgg.num_frames; k++)
					{
						if (mtex.mgg.frames_atlas[k] == i + 1)
							mtex.mgg.frames_atlas[k] = i;
					}
				}

				mtex.mgg.num_f_a = realloc(mtex.mgg.num_f_a, (mtex.mgg.num_c_atlas - 1) * sizeof(int16));
				mtex.mgg.num_f0 = realloc(mtex.mgg.num_f0, (mtex.mgg.num_c_atlas - 1) * sizeof(int16));
				mtex.mgg.num_ff = realloc(mtex.mgg.num_ff, (mtex.mgg.num_c_atlas - 1) * sizeof(int16));

				mtex.mgg.num_c_atlas--;
			}

		}
	}
}

void UpdateAnims()
{
	register int i, j;
	int counter;

	for (i = 0; i < mtex.mgg.num_anims; i++)
	{
		//if (mtex.mgg.fn[mtex.mgg.mga[i].startID] > 1024)
		//{
			for (j = 0, counter = 0; j < mtex.mgg.num_frames; j++)
			{
				if (j == mtex.mgg.mga[i].startID)
				{
					mtex.mgg.mga[i].startID = counter;
					mtex.mgg.mga[i].current_frame = mtex.mgg.mga[i].startID;
					break;
				}

				if (mtex.mgg.fn[j] < 1024) counter++;
			}
		//}

		//if (mtex.mgg.fn[mtex.mgg.mga[i].endID] > 1024)
		//{
			for (j = 0, counter = 0; j < mtex.mgg.num_frames; j++)
			{
				if (j == mtex.mgg.mga[i].endID)
				{
					mtex.mgg.mga[i].endID = counter;
					break;
				}

				if (mtex.mgg.fn[j] < 1024) counter++;
			}
		//}

		if (mtex.mgg.mga[i].startID == mtex.mgg.mga[i].endID)
			mtex.mgg.an[i] += 1024;
	}
}

void CheckForChanges()
{
	if (memcmp(&mtex.mgg, &mtex.mgg2, sizeof(mtex.mgg2)) != NULL)
	{
		mtex.changes_detected = 1;
		sprintf(st.WindowTitle, "Tex *%s", mtex.filename);
	}
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

char *CheckForNormal(char *filename)
{
	register int i, j;
	int len;
	static char normal[MAX_PATH];
	char str[8];
	FILE *f;

	len = strlen(filename);

	for (i = len; i > 0; i--)
	{
		if (filename[i] == '.')
			break;
	}

	strcpy(str, filename + i);

	strcpy(normal, filename);
	strcpy(normal + i, "_n");
	strcpy(normal + i + 2, str);

	if ((f = fopen(normal, "rb")) == NULL)
		return NULL;
	
	fclose(f);

	return normal;
}

const char *CopyThisFile(char *filepath, char *newpath)
{
	FILE *f, *f2;
	size_t size;
	int len, i;
	static char newfile[MAX_PATH];
	char *data;

	if ((f = fopen(filepath, "rb")) == NULL)
		return 0;

	len = strlen(filepath);

	for (i = len; i > 0; i--)
	{
		if (filepath[i] == '\\' || filepath[i] == '/')
		{
			i++;
			break;
		}
	}

	memset(newfile, 0, MAX_PATH);

	strcpy(newfile, newpath);

	strcat(newfile, "\\");

	strcat(newfile, filepath + i);
	strcat(newfile, "\0");

	if (strcmp(newfile, filepath) == NULL)
	{
		fclose(f);
		return -2;
	}

	if ((f2 = fopen(newfile, "wb")) == NULL)
	{
		fclose(f);
		return -1;
	}

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);

	data = malloc(size);

	fread(data, size, 1, f);

	fwrite(data, size, 1, f2);

	fclose(f);
	fclose(f2);

	free(data);

	return newfile;
}

char *StrTokNull(char *string)
{
	register int j;
	static int i, len;
	int state = 0;
	static char str[2048];
	static char *buf = NULL;

	if (string != NULL)
	{
		buf = string;
		i = 0;

		len = 0;
		while (buf[len] != '\0' || buf[len + 1] != '\0') len++;
	}

	if (!buf)
		return NULL;

	if (i == len - 1 || i >= len)
		return NULL;

	j = 0;
	while (buf[i] != '\0')
	{
		str[j] = buf[i];
		i++;
		j++;
	}

	str[j] = '\0';
	i++;

	return str;
}

int8 SavePrjFile(char *filepath)
{
	FILE *f;
	register int i, j;
	int anims = 0;

	if ((f = fopen(filepath, "w")) == NULL)
	{
		LogApp("Error: Could not create file %s", filepath);
		return NULL;
	}

	for (i = 0; i < mtex.mgg.num_anims; i++)
	{
		if (mtex.mgg.an[i] < 1024)
			anims++;
	}

	fprintf(f, "MGGNAME %s\n", mtex.mgg.name);
	fprintf(f, "FRAMES %d\n", mtex.mgg.num_frames);
	fprintf(f, "ANIMS %d\n", anims);
	
	if (mtex.mgg.RLE)
		fprintf(f, "RLE\n");

	if (mtex.mgg.mipmap)
		fprintf(f, "MIPMAP\n");

	if (mtex.mgg.num_c_atlas > 0)
	{
		for (i = 0; i < mtex.mgg.num_c_atlas; i++)
		{
			fprintf(f, "CONSTRUCT_ATLAS %d %d %d\n", i, mtex.mgg.num_f0[i], mtex.mgg.num_ff[i]);
			fprintf(f, "FRAMENAMES_CUSTOM_ATLAS %d ", i);
			for (j = mtex.mgg.num_f0[i]; j < mtex.mgg.num_ff[i] + 1; j++)
			{
				if (mtex.mgg.frames_atlas[j] != -1)
				{
					fprintf(f, "\"%s\"", mtex.mgg.texnames[mtex.mgg.fn[j]]);
				}

				if (j == mtex.mgg.num_ff[i])
					fprintf(f, "\n");
				else
					fprintf(f, ", ");
			}
		}
	}

	if (mtex.mgg.num_c_atlas > 0 && mtex.mgg.num_f_n > 0)
	{
		for (i = 0; i < mtex.mgg.num_c_atlas; i++)
		{
			//fprintf(f, "CONSTRUCT_ATLAS_NORMAL %d %d %d\n", i, mtex.mgg.num_f0[i], mtex.mgg.num_ff[i]);
			fprintf(f, "FRAMENAMES_CUSTOM_ATLAS_NORMALS %d ", i);
			for (j = mtex.mgg.num_f0[i]; j < mtex.mgg.num_ff[i] + 1; j++)
			{
				if (mtex.mgg.frames_atlas[j] != -1)
				{
					if (mtex.mgg.fnn[j] != -1 && mtex.mgg.fnn[j] < 1024)
						fprintf(f, "\"%s\"", mtex.mgg.texnames_n[mtex.mgg.fnn[j]]);
					else
						fprintf(f, "\"null\"");
				}

				if (j == mtex.mgg.num_ff[i])
					fprintf(f, "\n");
				else
					fprintf(f, ", ");
			}
		}
	}

	if (mtex.mgg.num_c_atlas == 0)
	{
		fprintf(f, "FRAMESFILES 0 %d ", mtex.mgg.num_frames);

		for (i = 0; i < mtex.mgg.num_frames; i++)
		{
			fprintf(f, "\"%s\"", mtex.mgg.texnames[mtex.mgg.fn[i]]);

			if (i == mtex.mgg.num_frames - 1)
				fprintf(f, "\n");
			else
				fprintf(f, ", ");
		}
	}
	else
	{
		for (i = 0; i < mtex.mgg.num_frames; i++)
		{
			if (mtex.mgg.frames_atlas[i] == -1)
				fprintf(f, "FRAMEFILE %d \"%s\"\n", i, mtex.mgg.texnames[mtex.mgg.fn[i]]);
		}
	}

	if (mtex.mgg.num_c_atlas == 0)
	{
		if (mtex.mgg.num_f_n > 0)
		{
			fprintf(f, "FRAMESFILES_NORMALS 0 %d ", mtex.mgg.num_frames);

			for (i = 0; i < mtex.mgg.num_frames; i++)
			{
				if (mtex.mgg.fnn[i] != -1 && mtex.mgg.fnn[i] < 1024)
					fprintf(f, "\"%s\"", mtex.mgg.texnames_n[mtex.mgg.fnn[i]]);
				else
					fprintf(f, "\"null\"");

				if (i == mtex.mgg.num_frames - 1)
					fprintf(f, "\n");
				else
					fprintf(f, ", ");
			}
		}
	}
	else
	{
		if (mtex.mgg.num_f_n > 0)
		{
			for (i = 0; i < mtex.mgg.num_frames; i++)
			{
				if (mtex.mgg.frames_atlas[i] == -1 && mtex.mgg.fnn[i] != -1 && mtex.mgg.fnn[i] < 1024)
					fprintf(f, "FRAMEFILE_NORMAL %d \"%s\"\n", i, mtex.mgg.texnames_n[mtex.mgg.fnn[i]]);
			}
		}
	}

	if (mtex.mgg.num_f_n > 0)
	{
		for (i = 0; i < mtex.mgg.num_frames; i++)
		{
			if (mtex.mgg.fnn[i] < 1024 && mtex.mgg.fnn[i] != -1)
				fprintf(f, "NORMALMAP %d\n", i);
		}
	}

	for (i = 0; i < mtex.mgg.num_frames; i++)
	{
		if (mtex.mgg.frameoffset_x[i] != 0 || mtex.mgg.frameoffset_y[i] != 0)
			fprintf(f, "FRAMEOFFSET %d %d %d\n", i, mtex.mgg.frameoffset_x[mtex.mgg.fn[i]], mtex.mgg.frameoffset_y[mtex.mgg.fn[i]]);
	}

	if (anims > 0)
	{
		for (i = 0, j = 0; i < mtex.mgg.num_anims; i++)
		{
			if (mtex.mgg.an[i] < 1024)
			{
				fprintf(f, "BEGIN\nANIM %d\nNAME %s\nFRAMESA %d\nSTARTF %d\nENDF %d\nSPEED %d\nENDA\n", j, mtex.mgg.mga[mtex.mgg.an[i]].name,
					mtex.mgg.mga[mtex.mgg.an[i]].num_frames, mtex.mgg.mga[mtex.mgg.an[i]].startID, mtex.mgg.mga[mtex.mgg.an[i]].endID, mtex.mgg.mga[mtex.mgg.an[i]].speed);
				j++;
			}
		}
	}

	fprintf(f, "\0\0");

	fclose(f);

	return 1;
}

char *LoadPrjFile(const char *filepath)
{
	FILE *f;
	char buf[512 * 64], str[MAX_PATH + 128], str2[64], buf2[2048], *tok, prj_path[MAX_PATH], error_string[2048], *tok2;
	register int i, j, k, l;
	int v1, v2, v3, v4, v5, readerror = 0, re[128], error_line[128], line = -1, anim = 0, anims = 0;

	memset(re, 0, 128 * sizeof(int));
	memset(prj_path, 0, MAX_PATH);
	memset(error_string, 0, 2048);

	if ((f = fopen(filepath, "r")) == NULL)
	{
		sprintf(error_string, "Could not open project file %s", filepath);
		LogApp(error_string);
		return error_string;
	}
	
	UnloadmTexMGG();
	
	v1 = strlen(filepath);

	for (i = v1; i > 0; i--)
	{
		if (filepath[i] == '\\' || filepath[i] == '/')
			break;
	}

	for (j = 0; j < i; j++)
		prj_path[j] = filepath[j];

	strcat(prj_path, "\\");

	LogApp("Reading project file...");

	while (!feof(f))
	{
		line++;
		memset(buf, 0, 512 * 64);
		fgets(buf, 512 * 64, f);

		if (!buf)
			continue;

		tok2 = strtok(buf, " \n");

		if (!tok2)
			continue;

		if (anim)
		{
			if (anim == 1)
			{
				if (strcmp(tok2, "ANIM") == NULL)
				{
					tok = strtok(NULL, " ");

					if (tok)
					{
						v1 = atoi(tok);
						anim++;
						continue;
					}
					else
					{
						re[readerror] = RE_NOTOK;
						error_line[readerror] = line;
						readerror++;
						anim = 0;
					}
				}
				else
				{
					LogApp("Line %d: Undefined command %s after BEGIN", line, tok);
					re[readerror] = RE_UNCOM;
					error_line[readerror] = line;
					readerror++;
				}
			}

			if (anim == 2)
			{
				if (strcmp(tok2, "NAME") == NULL)
				{
					tok = strtok(NULL, " ");

					if (tok)
					{
						v2 = strlen(tok);
						tok[v2 - 1] = '\0';

						strcpy(mtex.mgg.mga[v1].name, tok);
					}
					else
					{
						re[readerror] = RE_NOTOK;
						error_line[readerror] = line;
						readerror++;
					}
				}

				if (strcmp(tok2, "FRAMESA") == NULL)
				{
					tok = strtok(NULL, " ");

					if (tok)
						mtex.mgg.mga[v1].num_frames = atoi(tok);
					else
					{
						re[readerror] = RE_NOTOK;
						error_line[readerror] = line;
						readerror++;
					}
				}

				if (strcmp(tok2, "STARTF") == NULL)
				{
					tok = strtok(NULL, " ");

					if (tok)
						mtex.mgg.mga[v1].startID = atoi(tok);
					else
					{
						re[readerror] = RE_NOTOK;
						error_line[readerror] = line;
						readerror++;
					}
				}

				if (strcmp(tok2, "ENDF") == NULL)
				{
					tok = strtok(NULL, " ");

					if (tok)
						mtex.mgg.mga[v1].endID = atoi(tok);
					else
					{
						re[readerror] = RE_NOTOK;
						error_line[readerror] = line;
						readerror++;
					}
				}

				if (strcmp(tok2, "SPEED") == NULL)
				{
					tok = strtok(NULL, " ");

					if (tok)
						mtex.mgg.mga[v1].speed = atoi(tok);
					else
					{
						re[readerror] = RE_NOTOK;
						error_line[readerror] = line;
						readerror++;
					}
				}

				if (strcmp(tok2, "ENDA") == NULL)
				{
					if (!mtex.mgg.mga[v1].name || mtex.mgg.mga[v1].startID < -1 || !mtex.mgg.mga[v1].endID || !mtex.mgg.mga[v1].num_frames
						|| (mtex.mgg.mga[v1].endID - mtex.mgg.mga[v1].startID != mtex.mgg.mga[v1].num_frames - 1) || mtex.mgg.mga[v1].endID < mtex.mgg.mga[v1].startID)
					{
						LogApp("Animation %d: missing commands or invalid values before ENDA", v1);
						re[readerror] = RE_MISCOMS;
						error_line[readerror] = line;
						readerror++;
					}

					mtex.mgg.an[v1] = v1;

					anim = 0;
				}

				continue;
			}
		}

		if (strcmp(tok2, "MGGNAME") == NULL)
		{
			tok = strtok(NULL, " ");
			
			if (tok)
			{
				v1 = strlen(tok);
				tok[v1 - 1] = '\0';
				strcpy(mtex.mgg.name, tok);
			}
			else
			{
				re[readerror] = RE_NOTOK;
				error_line[readerror] = line;
				readerror++;
			}
		}

		if (strcmp(tok2, "FRAMEOFFSET") == NULL)
		{
			tok = strtok(NULL, " ");

			if (tok)
			{
				v2 = atoi(tok);

				if (v2 > mtex.mgg.num_frames || v2 < 0)
				{
					re[readerror] = RE_WRVAL;
					error_line[readerror] = line;
					readerror++;
					continue;
				}

				tok = strtok(NULL, " ");

				if (tok)
				{
					mtex.mgg.frameoffset_x[v2] = atoi(tok);
					
					tok = strtok(NULL, " ");

					if (tok)
						mtex.mgg.frameoffset_y[v2] = atoi(tok);
					else
					{
						re[readerror] = RE_NOTOK;
						error_line[readerror] = line;
						readerror++;
					}
				}
				else
				{
					re[readerror] = RE_NOTOK;
					error_line[readerror] = line;
					readerror++;
				}

			}
			else
			{
				re[readerror] = RE_NOTOK;
				error_line[readerror] = line;
				readerror++;
			}
		}

		if (strcmp(tok2, "FRAMES") == NULL)
		{
			tok = strtok(NULL, " ");

			if (tok)
			{
				mtex.mgg.num_frames = atoi(tok);

				if (mtex.mgg.num_frames)
				{
					mtex.mgg.fn = malloc(mtex.mgg.num_frames * sizeof(int16));
					memset(mtex.mgg.fn, -1, mtex.mgg.num_frames * sizeof(int16));
					mtex.mgg.fnn = malloc(mtex.mgg.num_frames * sizeof(int16));
					memset(mtex.mgg.fnn, -1, mtex.mgg.num_frames * sizeof(int16));
					mtex.mgg.frames_atlas = malloc(mtex.mgg.num_frames * sizeof(int8));
					memset(mtex.mgg.frames_atlas, -1, mtex.mgg.num_frames * sizeof(int8));
					mtex.textures = malloc(mtex.mgg.num_frames * sizeof(GLuint));
					mtex.size = malloc(mtex.mgg.num_frames * sizeof(Pos));
					mtex.textures_n = malloc(mtex.mgg.num_frames * sizeof(GLuint));
				}
				else
				{
					LogApp("Line %d: Invalid number of frames: %d", line, mtex.mgg.num_frames);
					re[readerror] = RE_WRVAL;
					error_line[readerror] = line;
					readerror++;
				}
			}
			else
			{
				re[readerror] = RE_NOTOK;
				error_line[readerror] = line;
				readerror++;
			}
		}

		if (strcmp(tok2, "ANIMS") == NULL)
		{
			tok = strtok(NULL, " ");

			if (tok)
			{
				mtex.mgg.num_anims = atoi(tok);

				LogApp("%d anims", mtex.mgg.num_anims);

				if (mtex.mgg.num_anims)
					mtex.mgg.mga = malloc(mtex.mgg.num_anims * sizeof(_MGGANIM));

				if (mtex.mgg.num_anims)
					mtex.mgg.an = malloc(mtex.mgg.num_anims * sizeof(int16));
			}
			else
			{
				re[readerror] = RE_NOTOK;
				error_line[readerror] = line;
				readerror++;
			}
		}

		if (strcmp(tok2, "BEGIN") == NULL)
		{
			if (!mtex.mgg.num_anims)
			{
				LogApp("Line %d: ANIMS not defined before BEGIN", line);
				re[readerror] = RE_MISCOM;
				error_line[readerror] = line;
				readerror++;
				continue;
			}
			else
			{
				anim++;
				anims++;
			}
		}

		if (strcmp(tok2, "MIPMAP") == NULL)
		{
			mtex.mgg.mipmap = 1;
			continue;
		}

		if (strcmp(tok2, "RLE") == NULL)
		{
			mtex.mgg.RLE = 1;
			continue;
		}

		if (strcmp(tok2, "CONSTRUCT_ATLAS") == NULL)
		{
			if (!mtex.mgg.num_frames)
			{
				LogApp("Line %d: FRAMES not defined before FRAMESNAMES", line);
				re[readerror] = RE_MISCOM;
				error_line[readerror] = line;
				readerror++;
				continue;
			}

			tok = strtok(NULL, " ");

			if (tok)
			{
				i = atoi(tok);

				tok = strtok(NULL, " ");

				if (tok)
				{
					j = atoi(tok);

					tok = strtok(NULL, " ");

					if (tok)
					{
						k = atoi(tok);

						if (j < 0)
						{
							LogApp("Line %d: atlas first frame is lower than 0", line);
							re[readerror] = RE_WRVAL;
							error_line[readerror] = line;
							readerror++;
						}

						if (k < j)
						{
							LogApp("Line %d: atlas final frame is lower than the initial", line);
							re[readerror] = RE_WRVAL;
							error_line[readerror] = line;
							readerror++;
						}

						if (mtex.mgg.num_c_atlas > 0)
						{
							for (l = 0; l < mtex.mgg.num_c_atlas; l++)
							{
								if (j == mtex.mgg.num_f0 || j == mtex.mgg.num_ff || (j > mtex.mgg.num_f0 && j < mtex.mgg.num_ff))
								{
									LogApp("Line %d: atlas first frame is part of atlas %d", line, l);
									re[readerror] = RE_WRVAL;
									error_line[readerror] = line;
									readerror++;
								}

								if (k == mtex.mgg.num_f0 || k == mtex.mgg.num_ff || (k > mtex.mgg.num_f0 && k < mtex.mgg.num_ff))
								{
									LogApp("Line %d: atlas last frame is part of atlas %d", line, l);
									re[readerror] = RE_WRVAL;
									error_line[readerror] = line;
									readerror++;
								}
							}

							mtex.mgg.num_f_a = realloc(mtex.mgg.num_f_a, (mtex.mgg.num_c_atlas + 1) * sizeof(int16));
							mtex.mgg.num_f0 = realloc(mtex.mgg.num_f0, (mtex.mgg.num_c_atlas + 1) * sizeof(int16));
							mtex.mgg.num_ff = realloc(mtex.mgg.num_ff, (mtex.mgg.num_c_atlas + 1) * sizeof(int16));

							mtex.mgg.num_f_a[mtex.mgg.num_c_atlas] = k - j + 1;
							mtex.mgg.num_f0[mtex.mgg.num_c_atlas] = j;
							mtex.mgg.num_ff[mtex.mgg.num_c_atlas] = k;

							mtex.mgg.num_c_atlas++;
						}
						else
						{
							mtex.mgg.num_f_a = malloc((mtex.mgg.num_c_atlas + 1) * sizeof(int16));
							mtex.mgg.num_f0 = malloc((mtex.mgg.num_c_atlas + 1) * sizeof(int16));
							mtex.mgg.num_ff = malloc((mtex.mgg.num_c_atlas + 1) * sizeof(int16));

							mtex.mgg.num_f_a[mtex.mgg.num_c_atlas] = k - j + 1;
							mtex.mgg.num_f0[mtex.mgg.num_c_atlas] = j;
							mtex.mgg.num_ff[mtex.mgg.num_c_atlas] = k;

							mtex.mgg.num_c_atlas++;
						}
					}
					else
					{
						re[readerror] = RE_NOTOK;
						error_line[readerror] = line;
						readerror++;
					}
				}
				else
				{
					re[readerror] = RE_NOTOK;
					error_line[readerror] = line;
					readerror++;
				}
			}
			else
			{
				re[readerror] = RE_NOTOK;
				error_line[readerror] = line;
				readerror++;
			}
		}

		if (strcmp(tok2, "FRAMENAMES_CUSTOM_ATLAS") == NULL)
		{
			if (!mtex.mgg.num_frames)
			{
				LogApp("Line %d: FRAMES not defined before FRAMESNAMES", line);
				re[readerror] = RE_MISCOM;
				error_line[readerror] = line;
				readerror++;
				continue;
			}

			if (!mtex.mgg.num_c_atlas)
			{
				LogApp("Line %d: CONSTRUCT_ATLAS not defined before FRAMESNAMES", line);
				re[readerror] = RE_MISCOM;
				error_line[readerror] = line;
				readerror++;
				continue;
			}

			tok = strtok(NULL, " ");

			if (tok)
			{
				j = atoi(tok);

				if (j < mtex.mgg.num_c_atlas - 1 || j > mtex.mgg.num_c_atlas)
				{
					LogApp("Line %d: atlas number not defined before with CONSTRUCT_ATLAS", line);
					re[readerror] = RE_WRVAL;
					error_line[readerror] = line;
					readerror++;
					continue;
				}

				for (k = mtex.mgg.num_f0[j]; k < mtex.mgg.num_ff[j] + 1; k++)
				{
					tok = strtok(NULL, ",\"");

					if (strcmp(tok, " ")==NULL)
						tok = strtok(NULL, ",\"");

					if (tok)
					{
						strcpy(mtex.mgg.texnames[k], tok);
						strcpy(mtex.mgg.files[k], prj_path);
						strcat(mtex.mgg.files[k], tok);
						mtex.mgg.fn[k] = k;
						mtex.mgg.frames_atlas[k] = j;
					}
					else
					{
						LogApp("Line %d: missing frame file name %d", line, k);
						re[readerror] = RE_NOTOK;
						error_line[readerror] = line;
						readerror++;

						continue;
					}
				}
			}
			else
			{
				re[readerror] = RE_NOTOK;
				error_line[readerror] = line;
				readerror++;
			}
		}

		if (strcmp(tok2, "FRAMENAMES_CUSTOM_ATLAS_NORMALS") == NULL)
		{
			if (!mtex.mgg.num_frames)
			{
				LogApp("Line %d: FRAMES not defined before FRAMESNAMES", line);
				re[readerror] = RE_MISCOM;
				error_line[readerror] = line;
				readerror++;
				continue;
			}

			if (!mtex.mgg.num_c_atlas)
			{
				LogApp("Line %d: CONSTRUCT_ATLAS not defined before FRAMESNAMES", line);
				re[readerror] = RE_MISCOM;
				error_line[readerror] = line;
				readerror++;
				continue;
			}

			tok = strtok(NULL, " ");

			if (tok)
			{
				j = atoi(tok);

				if (j < mtex.mgg.num_c_atlas - 1 || j > mtex.mgg.num_c_atlas)
				{
					LogApp("Line %d: atlas number not defined before with CONSTRUCT_ATLAS", line);
					re[readerror] = RE_WRVAL;
					error_line[readerror] = line;
					readerror++;
					continue;
				}

				for (k = mtex.mgg.num_f0[j]; k < mtex.mgg.num_ff[j] + 1; k++)
				{
					tok = strtok(NULL, ",\"");

					if (strcmp(tok, " ") == NULL)
						tok = strtok(NULL, ",\"");

					if (tok)
					{
						if (strcmp(tok, "null") == NULL)
							mtex.mgg.fnn[k] = -1;
						else
						{
							strcpy(mtex.mgg.texnames_n[k], tok);
							strcpy(mtex.mgg.files_n[k], prj_path);
							strcat(mtex.mgg.files_n[k], tok);
							mtex.mgg.fnn[k] = k;
							mtex.mgg.num_f_n++;
							//mtex.mgg.frames_atlas[k] = j;
						}
					}
					else
					{
						LogApp("Line %d: missing frame file name %d", line, k);
						re[readerror] = RE_NOTOK;
						error_line[readerror] = line;
						readerror++;

						continue;
					}
				}
			}
			else
			{
				re[readerror] = RE_NOTOK;
				error_line[readerror] = line;
				readerror++;
			}
		}

		if (strcmp(tok2, "FRAMESFILES") == NULL)
		{
			if (!mtex.mgg.num_frames)
			{
				LogApp("Line %d: FRAMES not defined before FRAMESFILES", line);
				re[readerror] = RE_MISCOM;
				error_line[readerror] = line;
				readerror++;
				continue;
			}

			tok = strtok(NULL, " ");

			if (tok)
			{
				i = atoi(tok);

				tok = strtok(NULL, " ");

				if (tok)
				{
					j = atoi(tok);

					if (!j || j < i)
					{
						LogApp("Line %d: final frame is zero or less than initial frame", line, j);
						re[readerror] = RE_WRVAL;
						error_line[readerror] = line;
						readerror++;
					}
					else
					{
						for (k = i; k < j; k++)
						{
							tok = strtok(NULL, ",\"");

							if (strcmp(tok, " ") == NULL)
								tok = strtok(NULL, ",\"");

							if (tok)
							{
								strcpy(mtex.mgg.texnames[k], tok);
								strcpy(mtex.mgg.files[k], prj_path);
								strcat(mtex.mgg.files[k], tok);
								mtex.mgg.fn[k] = k;
							}
							else
							{
								LogApp("Line %d: missing frame file name %d", line, k);
								re[readerror] = RE_NOTOK;
								error_line[readerror] = line;
								readerror++;

								continue;
							}
						}
					}
				}
				else
				{
					re[readerror] = RE_NOTOK;
					error_line[readerror] = line;
					readerror++;
				}
			}
			else
			{
				re[readerror] = RE_NOTOK;
				error_line[readerror] = line;
				readerror++;
			}
		}

		if (strcmp(tok2, "FRAMEFILE") == NULL)
		{
			if (!mtex.mgg.num_frames)
			{
				LogApp("Line %d: FRAMES not defined before FRAMEFILE", line);
				re[readerror] = RE_MISCOM;
				error_line[readerror] = line;
				readerror++;
				continue;
			}

			tok = strtok(NULL, " ");

			if (tok)
			{
				i = atoi(tok);

				tok = strtok(NULL, "\"\n");

				if (strcmp(tok, " ") == NULL)
					tok = strtok(NULL, ",\"");

				if (tok)
				{
					strcpy(mtex.mgg.texnames[i], tok);
					strcpy(mtex.mgg.files[i], prj_path);
					strcat(mtex.mgg.files[i], tok);
					mtex.mgg.fn[i] = i;
				}
				else
				{
					LogApp("Line %d: missing frame file name %d", line, k);
					re[readerror] = RE_NOTOK;
					error_line[readerror] = line;
					readerror++;

					continue;
				}
			}
			else
			{
				re[readerror] = RE_NOTOK;
				error_line[readerror] = line;
				readerror++;
			}
		}

		if (strcmp(tok2, "FRAMESFILES_NORMALS") == NULL)
		{
			if (!mtex.mgg.num_frames)
			{
				LogApp("Line %d: FRAMES not defined before FRAMESFILES_NORMAL", line);
				re[readerror] = RE_MISCOM;
				error_line[readerror] = line;
				readerror++;
				continue;
			}

			tok = strtok(NULL, " ");

			if (tok)
			{
				i = atoi(tok);

				tok = strtok(NULL, " ");

				if (tok)
				{
					j = atoi(tok);

					if (!j || j < i)
					{
						LogApp("Line %d: final frame is zero or less than initial frame", line, j);
						re[readerror] = RE_WRVAL;
						error_line[readerror] = line;
						readerror++;
					}
					else
					{
						for (k = i; k < j; k++)
						{
							tok = strtok(NULL, ",\"");

							if (strcmp(tok, " ") == NULL)
								tok = strtok(NULL, ",\"");

							if (tok)
							{
								if (strcmp(tok, "null") == NULL)
									mtex.mgg.fnn[k] = -1;
								else
								{
									strcpy(mtex.mgg.texnames_n[k], tok);
									strcpy(mtex.mgg.files_n[k], prj_path);
									strcat(mtex.mgg.files_n[k], tok);
									mtex.mgg.fnn[k] = k;
									mtex.mgg.num_f_n++;
								}
							}
							else
							{
								LogApp("Line %d: missing frame file name %d", line, k);
								re[readerror] = RE_NOTOK;
								error_line[readerror] = line;
								readerror++;

								continue;
							}
						}
					}
				}
				else
				{
					re[readerror] = RE_NOTOK;
					error_line[readerror] = line;
					readerror++;
				}
			}
			else
			{
				re[readerror] = RE_NOTOK;
				error_line[readerror] = line;
				readerror++;
			}
		}

		if (strcmp(tok2, "FRAMEFILE_NORMAL") == NULL)
		{
			if (!mtex.mgg.num_frames)
			{
				LogApp("Line %d: FRAMES not defined before FRAMEFILE_NORMAL", line);
				re[readerror] = RE_MISCOM;
				error_line[readerror] = line;
				readerror++;
				continue;
			}

			tok = strtok(NULL, " ");

			if (tok)
			{
				i = atoi(tok);

				tok = strtok(NULL, "\"\n");

				if (strcmp(tok, " ") == NULL)
					tok = strtok(NULL, ",\"");

				if (tok)
				{
					strcpy(mtex.mgg.texnames_n[i], tok);
					strcpy(mtex.mgg.files_n[i], prj_path);
					strcat(mtex.mgg.files_n[i], tok);
					mtex.mgg.fnn[i] = i;
					mtex.mgg.num_f_n++;
				}
				else
				{
					LogApp("Line %d: missing frame file name %d", line, k);
					re[readerror] = RE_NOTOK;
					error_line[readerror] = line;
					readerror++;

					continue;
				}
			}
			else
			{
				re[readerror] = RE_NOTOK;
				error_line[readerror] = line;
				readerror++;
			}
		}

		/*
		if (strcmp(tok, "NORMALMAP") == NULL)
		{
			if (!mtex.mgg.num_frames)
			{
				LogApp("Line %d: FRAMES not defined before NORMALMAP", line);
				re[readerror] = RE_MISCOM;
				error_line[readerror] = line;
				readerror++;
				continue;
			}

			tok = strtok(NULL, " ");

			if (tok)
			{
				mtex.mgg.num_f_n++;

				mtex.mgg.fnn[atoi]
			}
		}
		*/
	}

	if (anim)
	{
		LogApp("Error: animation definition %d not ended", v1);
		re[readerror] = RE_MISCOM;
		error_line[readerror] = line;
		readerror++;
	}

	if (anims < mtex.mgg.num_anims)
	{
		LogApp("Error: number defined animations is less than the number of declared animations");
		LogApp("Declared animations: %d", mtex.mgg.num_anims);
		LogApp("Defined animations: %d", anims);

		re[readerror] = RE_WRVAL;
		error_line[readerror] = line;
		readerror++;
	}

	if (readerror > 0)
	{
		for (i = 0; i < readerror; i++)
		{
			switch (re[i])
			{
				case RE_NOTOK:
					sprintf(str, "Missing value/name in line %d\n", error_line[i]);
					strcat(error_string, str);
					break;

				case RE_WRVAL:
					sprintf(str, "Invalid value in line %d\n", error_line[i]);
					strcat(error_string, str);
					break;

				case RE_MISCOM:
					sprintf(str, "Missing command before function in line %d\n", error_line[i]);
					strcat(error_string, str);
					break;

				case RE_UNTOK_N:
					sprintf(str, "Undefined value in line %d\n", error_line[i]);
					strcat(error_string, str);
					break;

				case RE_UNTOK_W:
					sprintf(str, "Undefined name in line %d\n", error_line[i]);
					strcat(error_string, str);
					break;

				case RE_UNCOM:
					sprintf(str, "Undefined command in line %d\n", error_line[i]);
					strcat(error_string, str);
					break;

				case RE_SEVERR:
					sprintf(str, "Multiple errors found before or in line %d\n", error_line[i]);
					strcat(error_string, str);
					break;

				case RE_MISCOMS:
					sprintf(str, "Missing commands or definitions before function in line %d\n", error_line[i]);
					strcat(error_string, str);
					break;
			}
		}

		sprintf(str, "Found %d errors in %s\n Check %s for more information", readerror, filepath, st.LogName);
		strcat(error_string, str);
		LogApp("%s", error_string);
		UnloadmTexMGG();
		return error_string;
	}
	else
	{
		LogApp("No errors found");
		LogApp("Loading textures...");

		for (i = 0; i < mtex.mgg.num_frames; i++)
		{
			if ((mtex.textures[i] = LoadTexture(mtex.mgg.files[i], mtex.mgg.mipmap, &mtex.size[i])) == -1)
			{
				sprintf(buf2, "Error: Texture %s could not be loaded", mtex.mgg.files[i]);
				MessageBox(NULL, buf2, "Error", MB_OK);
			}

			mtex.mgg.ftime[i] =*GetFileLastModification(mtex.mgg.files[i]);

			if (mtex.mgg.fnn[i] != -1 && mtex.mgg.fnn[i] < 1024)
			{
				if ((mtex.textures_n[i] = LoadTexture(mtex.mgg.files_n[i], mtex.mgg.mipmap, NULL)) == -1)
				{
					sprintf(buf2, "Error: Texture %s could not be loaded", mtex.mgg.files_n[i]);
					MessageBox(NULL, buf2, "Error", MB_OK);
				}

				mtex.mgg.ftime_n[i] = *GetFileLastModification(mtex.mgg.files_n[i]);
			}
		}
	}

	fclose(f);

	return NULL;
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

int NewMGGBox(const char path[MAX_PATH])
{
	register int i, j, k;
	static char path2[MAX_PATH], files[MAX_PATH + (32 * 512)], tex[512][MAX_PATH], tex_n[512][MAX_PATH], path3[MAX_PATH], str[32], tex_names[512][32], tex_n_names[512][32],
		prj_name[32] = { 0 }, mgg_name[32] = { 0 }, command[MAX_PATH + 32];
	static int len, state = 0, num_files_t = 0, num_files_n = 0, len_prj_n = 0, len_in_n = 0, fls[512], flsn[512];
	int len2;
	char *tok = NULL, strerror[1024];
	Pos size;

	OPENFILENAME ofn;
	ZeroMemory(&files, sizeof(files));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.tga;*.bmp\0Any File\0*.*\0";
	ofn.nMaxFile = MAX_PATH + (32 * 512);
	ofn.lpstrFile = files;
	ofn.lpstrTitle = "Select textures to import";
	//ofn.hInstance = OFN_EXPLORER;
	ofn.lpstrInitialDir = path;

	BROWSEINFO bi;

	ZeroMemory(&bi, sizeof(bi));

	static LPITEMIDLIST pidl;

	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	if (!state)
	{
		memset(&fls, -1, 512 * sizeof(int));
		memset(&flsn, -1, 512 * sizeof(int));

		strcpy(path2, path);
		len = strlen(path2);
		
		state = 1;
	}

	if (nk_begin(ctx, "Create new MGG project", nk_rect(st.screenx / 2 - 256, st.screeny / 2 - (564/2), 512, 564), NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		nk_layout_row_begin(ctx, NK_DYNAMIC, 20, 2);

		nk_layout_row_push(ctx, 0.85f);
		nk_edit_string(ctx, NK_EDIT_SIMPLE, path2, &len, MAX_PATH, nk_filter_default);

		nk_layout_row_push(ctx, 0.15f);
		if (nk_button_label(ctx, "Browse"))
		{
			bi.lpszTitle = ("Select a folder to create the project");

			pidl = SHBrowseForFolder(&bi);

			if (pidl)
			{
				SHGetPathFromIDList(pidl, path);

				strcpy(path2, path);
				len = strlen(path2);
			}
		}

		nk_layout_row_end(ctx);

		nk_layout_row_dynamic(ctx, 10, 1);
		nk_label(ctx, "Project name", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 20, 1);
		nk_edit_string(ctx, NK_EDIT_SIMPLE, prj_name, &len_prj_n, 32, nk_filter_default);

		nk_layout_row_dynamic(ctx, 10, 1);
		nk_label(ctx, "MGG internal name", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 20, 1);
		nk_edit_string(ctx, NK_EDIT_SIMPLE, mgg_name, &len_in_n, 32, nk_filter_default);

		//nk_layout_row_dynamic(ctx, 10, 1);
		//nk_spacing(ctx, 1);

		nk_layout_row_dynamic(ctx, 30, 2);
		if (nk_button_label(ctx, "Import textures"))
		{
			if (GetOpenFileName(&ofn))
			{
				state = 1;
				i = 0;
				while (state == 1)
				{
					if (i == 0)
					{
						tok = StrTokNull(files);
						strcpy(path3, tok);
					}
					else
					{
						tok = StrTokNull(NULL);

						if (!tok)
						{
							if (i == 1)
							{
								strcpy(tex[num_files_t], path3);
								len2 = strlen(path3);

								for (j = len2; j > 0; j--)
								{
									if (path3[j] == '\\' || path3[j] == '/')
									{
										j++;
										break;
									}
								}

								strcpy(tex_names[num_files_t], path3 + j);

								tok = CheckForNormal(tex[num_files_t]);

								if (tok)
								{
									strcpy(tex_n[num_files_n], tok);

									len2 = strlen(tok);

									for (j = len2; j > 0; j--)
									{
										if (tok[j] == '\\' || tok[j] == '/')
										{
											j++;
											break;
										}
									}

									strcpy(tex_n_names[num_files_n], tok + j);

									flsn[num_files_n] = num_files_n;

									num_files_n++;
								}

								fls[num_files_t] = num_files_t;

								num_files_t++;
							}
							state = 2;
							i = 0;
							break;
						}
						else
						{
							strcpy(tex[num_files_t], path3);
							strcat(tex[num_files_t], "\\");
							strcat(tex[num_files_t], tok);
							strcpy(tex_names[num_files_t], tok);

							tok = CheckForNormal(tex[num_files_t]);

							if (tok)
							{
								strcpy(tex_n[num_files_n], tok);

								len2 = strlen(tok);

								for (j = len2; j > 0; j--)
								{
									if (tok[j] == '\\' || tok[j] == '/')
									{
										j++;
										break;
									}
								}

								strcpy(tex_n_names[num_files_n], tok + j);

								flsn[num_files_n] = num_files_n;

								num_files_n++;
							}

							fls[num_files_t] = num_files_t;

							num_files_t++;
						}
					}

					i++;
				}
				
			}
		}

		if (nk_button_label(ctx, "Import normal mapping textures"))
		{
			ZeroMemory(&files, sizeof(files));
			ofn.lpstrTitle = "Select normal mapping textures to import";

			if (GetOpenFileName(&ofn))
			{
				state = 1;
				i = 0;
				while (state)
				{
					if (i == 0)
					{
						tok = StrTokNull(files);
						strcpy(path3, tok);
					}
					else
					{
						tok = StrTokNull(NULL);

						if (!tok)
						{
							if (i == 1)
							{
								strcpy(tex_n[num_files_n], path3);
								len2 = strlen(path3);

								for (j = len2; j > 0; j--)
								{
									if (path3[j] == '\\' || path3[j] == '/')
									{
										j++;
										break;
									}
								}

								strcpy(tex_n_names[num_files_n], path3 + j);

								flsn[num_files_n] = num_files_n;

								num_files_n++;
							}
							state = 2;
							i = 0;
							break;
						}
						else
						{
							strcpy(tex_n[num_files_n], path3);
							strcat(tex[num_files_n], "\\");
							strcat(tex_n[num_files_n], tok);
							strcpy(tex_n_names[num_files_n], tok);

							flsn[num_files_n] = num_files_n;

							num_files_n++;
						}
					}

					i++;
				}

			}
		}

		nk_layout_row_dynamic(ctx, 256, 2);

		if (nk_group_begin(ctx, "Selected textures", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
		{
			for (i = 0; i < num_files_t; i++)
			{
				nk_layout_row_begin(ctx, NK_DYNAMIC, 15, 4);
				nk_layout_row_push(ctx, 0.1f);
				sprintf(str, "%d", i);
				nk_label(ctx, str, NK_TEXT_ALIGN_LEFT);

				nk_layout_row_push(ctx, 0.60f);

				nk_label(ctx, tex_names[fls[i]], NK_TEXT_ALIGN_LEFT);

				SetThemeBack();

				nk_layout_row_push(ctx, 0.1f);

				if (fls[i] < 1024)
				{
					if (nk_button_symbol(ctx, NK_SYMBOL_MINUS))
						fls[i] += 1024;
				}
				else
				{
					if (nk_button_symbol(ctx, NK_SYMBOL_PLUS))
						fls[i] -= 1024;
				}

				nk_layout_row_push(ctx, 0.1f);
				if (i > 0)
				{
					if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_UP))
					{
						j = fls[i];
						fls[i] = fls[i - 1];
						fls[i - 1] = j;
					}
				}
				else
					nk_spacing(ctx, 1);

				nk_layout_row_push(ctx, 0.1f);
				if (i < num_files_t - 1)
				{
					if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_DOWN))
					{
						j = fls[i];
						fls[i] = fls[i + 1];
						fls[i + 1] = j;
					}
				}
				else
					nk_spacing(ctx, 1);

				nk_layout_row_end(ctx);
			}

			nk_group_end(ctx);
		}

		if (nk_group_begin(ctx, "Selected normal maps", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
		{
			for (i = 0; i < num_files_t; i++)
			{
				nk_layout_row_begin(ctx, NK_DYNAMIC, 15, 4);
				nk_layout_row_push(ctx, 0.1f);
				sprintf(str, "%d", i);
				nk_label(ctx, str, NK_TEXT_ALIGN_LEFT);

				nk_layout_row_push(ctx, 0.60f);

				nk_label(ctx, tex_n_names[flsn[i]], NK_TEXT_ALIGN_LEFT);

				SetThemeBack();

				nk_layout_row_push(ctx, 0.1f);

				if (flsn[i] < 1024 && flsn[i] != -1)
				{
					if (nk_button_symbol(ctx, NK_SYMBOL_MINUS))
						flsn[i] += 1024;
				}
				else
				{
					if (nk_button_symbol(ctx, NK_SYMBOL_PLUS))
						flsn[i] -= 1024;
				}

				nk_layout_row_push(ctx, 0.1f);
				if (i > 0)
				{
					if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_UP))
					{
						j = flsn[i];
						flsn[i] = flsn[i - 1];
						flsn[i - 1] = j;
					}
				}
				else
					nk_spacing(ctx, 1);

				nk_layout_row_push(ctx, 0.1f);
				if (i < num_files_t - 1)
				{
					if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_DOWN) && i)
					{
						j = flsn[i];
						flsn[i] = flsn[i + 1];
						flsn[i + 1] = j;
					}
				}
				else
					nk_spacing(ctx, 1);

				nk_layout_row_end(ctx);
			}

			nk_group_end(ctx);
		}

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "Texture compression", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 15, 2);

		mtex.mgg.RLE = nk_option_label(ctx, "None", mtex.mgg.RLE == 0) ? 0 : mtex.mgg.RLE;
		mtex.mgg.RLE = nk_option_label(ctx, "RLE (faster)", mtex.mgg.RLE == 1) ? 1 : mtex.mgg.RLE;

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "Texture mipmap (filter)", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 15, 2);

		mtex.mgg.mipmap = nk_option_label(ctx, "Nearest", mtex.mgg.mipmap == 1) ? 1 : mtex.mgg.mipmap;
		mtex.mgg.mipmap = nk_option_label(ctx, "Linear", mtex.mgg.mipmap == 0) ? 0 : mtex.mgg.mipmap;

		nk_layout_row_dynamic(ctx, 25, 6);
		nk_spacing(ctx, 4);

		if (nk_button_label(ctx, "Create"))
		{
			state = 3;

			if (!len_prj_n)
			{
				MessageBox(NULL, "Error: Project name is empty", "Error", MB_OK);
				state = 2;
			}
			
			if (!len_in_n)
			{
				MessageBox(NULL, "Error: MGG internal name is empty", "Error", MB_OK);
				state = 2;
			}

			for (i = 0, j = 0; i < num_files_t; i++)
			{
				if (fls[i] < 1024)
					j++;
			}

			if (!num_files_t || j == 0)
			{
				MessageBox(NULL, "Error: No textures selected", "Error", MB_OK);
				state = 2;
			}

			if (state == 3)
			{
				strcat(path2, "\\");
				strcat(path2, prj_name);
				sprintf(command, "mkdir \"%s\"", path2);
				system(command);

				strcpy(mtex.mgg.name, mgg_name);

				strcpy(mtex.mgg.path, path2);

				for (i = 0; i < num_files_t; i++)
				{
					if (fls[i] < 1024)
						mtex.mgg.num_frames++;
				}

				for (i = 0; i < num_files_n; i++)
				{
					if (flsn[i] < 1024)
						mtex.mgg.num_f_n++;
				}

				mtex.mgg.fn = malloc(mtex.mgg.num_frames * sizeof(int16));
				memset(mtex.mgg.fn, -1, mtex.mgg.num_frames * sizeof(int16));
				mtex.mgg.fnn = malloc(mtex.mgg.num_frames * sizeof(int16));
				memset(mtex.mgg.fnn, -1, mtex.mgg.num_frames * sizeof(int16));
				mtex.mgg.frames_atlas = malloc(mtex.mgg.num_frames * sizeof(int8));
				memset(mtex.mgg.frames_atlas, -1, mtex.mgg.num_frames * sizeof(int8));
				mtex.textures = malloc(mtex.mgg.num_frames * sizeof(GLuint));
				mtex.textures_n = malloc(mtex.mgg.num_f_n * sizeof(GLuint));
				mtex.size = malloc(mtex.mgg.num_frames * sizeof(Pos));

				for (i = 0, j = 0; i < num_files_t; i++)
				{
					if (fls[i] < 1024)
					{
						tok = CopyThisFile(tex[fls[i]], path2);
						if (tok == NULL)
						{
							sprintf(strerror, "Error: Texture %s could not be opened", tex[fls[i]]);
							MessageBox(NULL, strerror, "Error", MB_OK);
						}

						if (tok == -1)
						{
							sprintf(strerror, "Error: Texture %s could not be copied", tex[fls[i]]);
							MessageBox(NULL, strerror, "Error", MB_OK);
						}

						if (tok == -2)
							tok = tex[fls[i]];

						if (tok)
						{
							mtex.mgg.fn[j] = j;
							strcpy(mtex.mgg.files[j], tok);
							strcpy(mtex.mgg.texnames[j], tex_names[fls[i]]);

							if ((mtex.textures[j] = LoadTexture(mtex.mgg.files[j], mtex.mgg.mipmap, &mtex.size[j])) == -1)
							{
								sprintf(strerror, "Error: Texture %s could not be loaded", mtex.mgg.files[j]);
								MessageBox(NULL, strerror, "Error", MB_OK);
								continue;
							}

							mtex.mgg.ftime[j] = *GetFileLastModification(mtex.mgg.files[j]);

							j++;
						}
					}
				}

				for (i = 0, j = 0; i < num_files_t; i++)
				{
					if (flsn[i] < 1024 && flsn[i] != -1)
					{
						tok = CopyThisFile(tex_n[flsn[i]], path2);
						if (tok == NULL)
						{
							sprintf(strerror, "Error: Texture %s could not be opened", tex_n[flsn[i]]);
							MessageBox(NULL, strerror, "Error", MB_OK);
						}

						if (tok == -1)
						{
							sprintf(strerror, "Error: Texture %s could not be copied", tex_n[flsn[i]]);
							MessageBox(NULL, strerror, "Error", MB_OK);
						}

						if (tok == -2)
							tok = tex[flsn[i]];

						if (tok)
						{
							mtex.mgg.fnn[j] = j;
							strcpy(mtex.mgg.files_n[j], tok);
							strcpy(mtex.mgg.texnames_n[j], tex_n_names[flsn[i]]);

							if ((mtex.textures_n[j] = LoadTexture(mtex.mgg.files_n[j], mtex.mgg.mipmap, &size)) == -1)
							{
								sprintf(strerror, "Error: Texture %s could not be loaded", mtex.mgg.files_n[j]);
								MessageBox(NULL, strerror, "Error", MB_OK);
								continue;
							}

							mtex.mgg.ftime_n[j] = *GetFileLastModification(mtex.mgg.files_n[j]);

							if (size.x != mtex.size[j].x || size.y != mtex.size[j].y)
							{
								sprintf(strerror, "Error: Normal texture %s width and height are different from the diffuse", mtex.mgg.files_n[j]);
								MessageBox(NULL, strerror, "Error", MB_OK);
								continue;
							}

							j++;
						}
					}
				}

				strcpy(path3, path2);
				strcat(path3, "\\");
				strcat(path3, prj_name);
				strcat(path3, ".texprj");
				SavePrjFile(path3);

				len = strlen(path3);

				for (i = len; i > 0; i--)
				{
					if (path3[i] == '\\' || path3[i] == '/') break;
				}

				i++;

				strcpy(mtex.filename, path3 + i);

				sprintf(st.WindowTitle, "Tex %s", mtex.filename);

				memcpy(&mtex.mgg2, &mtex.mgg, sizeof(mtex.mgg));

				strcpy(mtex.prj_path, path3);

				state = 4;
			}
		}

		if (nk_button_label(ctx, "Cancel"))
			state = 5;
	}

	nk_end(ctx);

	if (state == 4)
	{
		num_files_n = 0;
		num_files_t = 0;
		memset(tex_names, 0, sizeof(tex_names));
		memset(tex_n_names, 0, sizeof(tex_n_names));
		memset(tex, 0, sizeof(tex));
		memset(tex_n, 0, sizeof(tex_n));
		memset(fls, 0, sizeof(512));
		memset(flsn, 0, sizeof(512));
		memset(path3, 0, sizeof(512));
		state = 0;
		return 1;
	}

	if (state == 5)
	{
		num_files_n = 0;
		num_files_t = 0;
		memset(tex_names, 0, sizeof(tex_names));
		memset(tex_n_names, 0, sizeof(tex_n_names));
		memset(tex, 0, sizeof(tex));
		memset(tex_n, 0, sizeof(tex_n));
		memset(fls, 0, sizeof(512));
		memset(flsn, 0, sizeof(512));
		memset(path3, 0, sizeof(512));
		state = 0;
		return -1;
	}

	return NULL;
}

int MGGProperties()
{
	if (nk_begin(ctx, "MGG Properties", nk_rect(st.screenx / 2 - 140, st.screeny / 2 - 115, 280, 230), NK_WINDOW_TITLE | NK_WINDOW_BORDER))
	{
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "MGG Name",NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 25, 1);
		nk_edit_string_zero_terminated(ctx, NK_EDIT_SIMPLE, mtex.mgg.name, 32, nk_filter_ascii);

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "Texture filter (requires reloading)", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 25, 2);
		mtex.mgg.mipmap = nk_option_label(ctx, "Linear", mtex.mgg.mipmap == 0) ? 0 : mtex.mgg.mipmap;
		mtex.mgg.mipmap = nk_option_label(ctx, "Nearest", mtex.mgg.mipmap == 1) ? 1 : mtex.mgg.mipmap;

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "Compression", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 25, 2);
		mtex.mgg.RLE = nk_option_label(ctx, "None", mtex.mgg.RLE == 0) ? 0 : mtex.mgg.RLE;
		mtex.mgg.RLE = nk_option_label(ctx, "RLE", mtex.mgg.RLE == 1) ? 1 : mtex.mgg.RLE;

		nk_layout_row_dynamic(ctx, 25, 1);
		if (nk_button_label(ctx, "Ok"))
		{
			nk_end(ctx);
			return 1;
		}
	}

	nk_end(ctx);

	return NULL;
}

int Preferences()
{
	static char str[32];
	register int i;

	if (mtex.theme == THEME_WHITE) strcpy(str, "White skin");
	if (mtex.theme == THEME_RED) strcpy(str, "Red skin");
	if (mtex.theme == THEME_BLUE) strcpy(str, "Blue skin");
	if (mtex.theme == THEME_DARK) strcpy(str, "Dark skin");
	if (mtex.theme == THEME_GWEN) strcpy(str, "GWEN skin");
	if (mtex.theme == THEME_BLACK) strcpy(str, "Default");

	if (nk_begin(ctx, "Preferences", nk_rect(st.screenx / 2 - 100, st.screeny / 2 - 100, 200, 128), NK_WINDOW_TITLE | NK_WINDOW_BORDER))
	{
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "UI skin", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 25, 1);
		if (nk_combo_begin_label(ctx, str, nk_vec2(nk_widget_width(ctx), 225)))
		{
			nk_layout_row_dynamic(ctx, 25, 1);

			if (nk_combo_item_label(ctx, "White skin", NK_TEXT_ALIGN_LEFT))
			{
				mtex.theme = THEME_WHITE;
				SetSkin(ctx, THEME_WHITE);
				strcpy(str, "White skin");
			}

			if (nk_combo_item_label(ctx, "Red skin", NK_TEXT_ALIGN_LEFT))
			{
				mtex.theme = THEME_RED;
				SetSkin(ctx, THEME_RED);
				strcpy(str, "Red skin");
			}

			if (nk_combo_item_label(ctx, "Blue skin", NK_TEXT_ALIGN_LEFT))
			{
				mtex.theme = THEME_BLUE;
				SetSkin(ctx, THEME_BLUE);
				strcpy(str, "Blud skin");
			}

			if (nk_combo_item_label(ctx, "Dark skin", NK_TEXT_ALIGN_LEFT))
			{
				mtex.theme = THEME_DARK;
				SetSkin(ctx, THEME_DARK);
				strcpy(str, "Dark skin");
			}

			if (nk_combo_item_label(ctx, "GWEN skin", NK_TEXT_ALIGN_LEFT))
			{
				mtex.theme = THEME_GWEN;
				SetSkin(ctx, THEME_GWEN);
				strcpy(str, "GWEN skin");
			}

			if (nk_combo_item_label(ctx, "Default", NK_TEXT_ALIGN_LEFT))
			{
				mtex.theme = THEME_BLACK;
				SetSkin(ctx, THEME_BLACK);
				strcpy(str, "Default skin");
			}

			nk_combo_end(ctx);
		}

		nk_layout_row_dynamic(ctx, 25, 1);
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

int Compiler(const char *path)
{
	char directory[MAX_PATH];
	char exepath[MAX_PATH];
	char args[MAX_PATH * 3];
	char str[512];
	static path2[MAX_PATH];
	MSG msg;

	DWORD error;

	static int state = 0;
	DWORD exitcode;

	static SHELLEXECUTEINFO info;

	if (state == 0)
	{
		strcpy(exepath, st.CurrPath);
		strcat(exepath,"\\Tools\\");
		strcat(exepath, "mggcreator.exe");
		strcpy(directory, exepath);
		strcpy(path2, path);
		PathRemoveFileSpec(directory);

		sprintf(args, "-o \"%s\" -p \"%s\" -i \"%s\"", path, mtex.mgg.path, mtex.filename);

		ZeroMemory(&info, sizeof(info));
		info.cbSize = sizeof(info);
		info.lpVerb = ("open");
		info.fMask = SEE_MASK_NOCLOSEPROCESS; //| SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC;
		info.lpFile = exepath;
		info.lpParameters = args;
		//info.lpDirectory = directory;
		info.nShow = SW_SHOW;
		if (!ShellExecuteEx(&info))
		{
			error = GetLastError();
			MessageBox(NULL, "Could not execute compiler file", "Error", MB_OK);
			state = 0;
			return -1;
		}

		state = 1;
	}
	
	if (state == 1)
	{
		if (nk_begin(ctx, "Compiler", nk_rect(st.screenx / 2 - 128, st.screeny / 2 - 32, 256, 64), NK_WINDOW_BORDER))
		{
			nk_layout_row_dynamic(ctx, 15, 1);

			nk_label(ctx, "Compiling project...", NK_TEXT_ALIGN_LEFT);

			// Wait for process to finish.
			error = WaitForSingleObject(info.hProcess, 0);


			if (error == WAIT_OBJECT_0)
			{
				// Return exit code from process
				GetExitCodeProcess(info.hProcess, &exitcode);

				CloseHandle(info.hProcess);

				if (exitcode)
					MessageBox(NULL, "MGGcreator failed to compile your MGG file", "Error", MB_OK);
				else
				{
					if (MessageBox(NULL, "Success compiling file\nWould you like to test it?", "Success", MB_YESNO) == IDYES)
					{
						ZeroMemory(&info, sizeof(info));
						info.cbSize = sizeof(info);
						info.lpVerb = ("open");
						strcpy(exepath, st.CurrPath);
						strcat(exepath, "\\mggviewer.exe");
						info.lpFile = exepath;
						sprintf(args, "-o \"%s\"", path2);
						info.lpParameters = args;
						info.lpDirectory = st.CurrPath;
						info.nShow = SW_SHOW;
						if (!ShellExecuteEx(&info))
						{
							error = GetLastError();
							MessageBox(NULL, "Could not execute MGGViewer", "Error", MB_OK);
						}
					}
				}

				state = 0;

				nk_end(ctx);
				return 1;
			}
		}

		nk_end(ctx);
	}

	return NULL;
}

void MenuBar()
{
	register int i, a, m, j;
	static char str[128], path[MAX_PATH];
	int id = 0, id2 = 0, check;
	static int state = 0, mggid;
	FILE *f;
	char *buf, path2[MAX_PATH], str2[512], path3[MAX_PATH];

	OPENFILENAME ofn;
	ZeroMemory(&path, sizeof(path));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	ofn.lpstrFilter = "mTex project Files\0*.texprj\0";
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFile = path;
	ofn.lpstrTitle = "Select the project file";
	//ofn.hInstance = OFN_EXPLORER;
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_EXPLORER;

	BROWSEINFO bi;

	ZeroMemory(&bi, sizeof(bi));

	static LPITEMIDLIST pidl;

	//if (nkrendered==0)
	//{
		if (nk_begin(ctx, "MenuBar", nk_rect(0, 0, st.screenx, 30), NK_WINDOW_NO_SCROLLBAR))
		{
			nk_menubar_begin(ctx);
			nk_layout_row_begin(ctx, NK_STATIC, 25, 3);

			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(210, 250)))
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				if (nk_menu_item_label(ctx, "New project", NK_TEXT_LEFT))
					state = 1;

				if (nk_menu_item_label(ctx, "Open project file", NK_TEXT_LEFT))
				{
					if (GetOpenFileName(&ofn) && path)
					{
						buf = LoadPrjFile(path);
						if (buf)
							MessageBox(NULL, buf, "Error", MB_OK);
						else
						{
							strcpy(mtex.prj_path, path);

							a = strlen(path);

							for (i = a; i > 0; i--)
							{
								if (path[i] == '\\' || path[i] == '/') break;
							}

							i++;

							strcpy(mtex.filename, path + i);

							sprintf(st.WindowTitle, "Tex %s", mtex.filename);

							memcpy(&mtex.mgg2, &mtex.mgg, sizeof(mtex.mgg));

							for (m = 0; m < i - 1; m++)
								mtex.mgg.path[m] = path[m];
						}
					}
				}

				if (nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT))
				{
					SavePrjFile(mtex.prj_path);

					sprintf(st.WindowTitle, "Tex %s", mtex.filename);

					memcpy(&mtex.mgg2, &mtex.mgg, sizeof(mtex.mgg));
				}

				if (nk_menu_item_label(ctx, "Save as...", NK_TEXT_LEFT))
				{
					ZeroMemory(&path, sizeof(path));
					
					ofn.lpstrFilter = "mTex project File\0*.texprj\0";
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFile = path;
					ofn.lpstrTitle = "Save project file";
					//ofn.hInstance = OFN_EXPLORER;
					ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_EXPLORER;

					if (GetSaveFileName(&ofn) && path)
					{
						strcpy(path2, path);

						for (j = strlen(path2); j > 0; j--)
						{
							if (path2[j] == '\\' || path2[j] == '/') break;
						}

						for (a = 0, m = 0; a < j; a++)
						{
							if (path2[a] != mtex.mgg.path[a])
							{
								m = 1;
								break;
							}
						}

						for (j = strlen(path2); j > 0; j--)
						{
							if (path2[j] == '.') break;
							if (path2[j] == '\\' || path2[j] == '/')
							{
								j = -1;
								break;
							}
						}

						if (j != -1)
							strcpy(path2 + j, ".texprj");
						else
							strcat(path2, ".texprj");

						SavePrjFile(path2);

						for (j = strlen(path2); j > 0; j--)
						{
							if (path2[j] == '\\' || path2[j] == '/') break;
						}

						//j--;
						
						strcpy(path3, mtex.mgg.path);

						for (i = 0; i < j; i++)
							mtex.mgg.path[i] = path2[i];

						strcpy(mtex.filename, path2 + j + 1);

						strcpy(mtex.prj_path, path2);

						sprintf(st.WindowTitle, "Tex %s", mtex.filename);

						memcpy(&mtex.mgg2, &mtex.mgg, sizeof(mtex.mgg));

						if (m == 1)
						{
							if (MessageBox(NULL, "Would you like to copy the texture files to this folder?", NULL, MB_YESNO) == IDYES)
							{
								for (i = 0; i < mtex.mgg.num_frames; i++)
								{
									if (mtex.mgg.fn[i] < 1024)
									{
										//strcpy(path2, path3);
										//strcat(path2, "\\");
										strcpy(path2, mtex.mgg.files[i]);
										buf = CopyThisFile(path2, mtex.mgg.path);

										if (buf == NULL || buf == -1)
										{
											sprintf(str2, "Error: Texture %d could not be copied", path2);
											MessageBox(NULL, str2, "Error", MB_OK);
										}

										if (mtex.mgg.fnn[i] < 1024 && mtex.mgg.fnn[i] != -1)
										{
											//strcpy(path2, path3);
											//strcat(path2, "\\");
											strcpy(path2, mtex.mgg.files_n[i]);
											buf = CopyThisFile(path2, mtex.mgg.path);

											if (buf == NULL || buf == -1)
											{
												sprintf(str2, "Error: Texture %d could not be copied", path2);
												MessageBox(NULL, str2, "Error", MB_OK);
											}
										}
									}
								}
							}
						}
					}
				}

				if (nk_menu_item_label(ctx, "Close project", NK_TEXT_LEFT))
				{
					switch (MessageBox(NULL, "Would you like to save your project first?", NULL, MB_YESNOCANCEL))
					{
						case IDYES:
							SavePrjFile(mtex.prj_path);
							UnloadmTexMGG();
							break;

						case IDNO:
							UnloadmTexMGG();
							break;
					}
				}

				if (nk_menu_item_label(ctx, "Compile MGG", NK_TEXT_LEFT))
				{
					ofn.lpstrFilter = "MGG File\0*.mgg\0";
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFile = path;
					ofn.lpstrTitle = "Save MGG file";
					//ofn.hInstance = OFN_EXPLORER;
					ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_EXPLORER;

					SavePrjFile(mtex.prj_path);

					sprintf(st.WindowTitle, "Tex %s", mtex.filename);

					memcpy(&mtex.mgg2, &mtex.mgg, sizeof(mtex.mgg));

					if (GetSaveFileName(&ofn) && path)
					{
						strcpy(path2, path);

						for (j = strlen(path2); j > 0; j--)
						{
							if (path2[j] == '.') break;
							if (path2[j] == '\\' || path2[j] == '/')
							{
								j = -1;
								break;
							}
						}

						if (j != -1)
							strcpy(path2 + j, ".mgg");
						else
							strcat(path2, ".mgg");

						state = 6;
					}
				}

				if (nk_menu_item_label(ctx, "Exit", NK_TEXT_LEFT)) st.quit = 1;
				nk_menu_end(ctx);
			}

			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "Edit", NK_TEXT_LEFT, nk_vec2(120, 200)))
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				if (nk_menu_item_label(ctx, "Preferences", NK_TEXT_LEFT))
					state = 5;

				if (nk_menu_item_label(ctx, "MGG properties", NK_TEXT_LEFT))
					state = 4;

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

		if (state == 1)
		{
			bi.lpszTitle = ("Select a folder to create the project");
			bi.ulFlags = BIF_USENEWUI;

			pidl = SHBrowseForFolder(&bi);

			if (pidl)
			{
				state = 2;
				UnloadmTexMGG();
				//memset(&tmgg, 0, sizeof(_MGGFORMAT));
			}
			else
				state = 0;
		}

		if (state == 2)
		{
			if (pidl)
			{
				SHGetPathFromIDList(pidl, path);
				
				a = NewMGGBox(path);

				if (a != 0)
					state = 0;
			}
		}

		if (state == 4)
		{
			if (MGGProperties())
				state = 0;
		}

		if (state == 5)
		{
			if (Preferences())
				state = 0;
		}

		if (state == 6)
		{
			if (Compiler(path2)) state = 0;
		}
	//}
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

		SetThemeBack();

		nk_layout_row_dynamic(ctx, 30, 3);
		nk_spacing(ctx, 2);

		//if (nk_button_label(ctx, "Select"))
			//meng.command = ADD_SPRITE;
	}

	st.mouse1 = 0;

	nk_end(ctx);
}

void LeftPannel()
{
	register int i, j, k;
	static int sl, pannel_state = 0, len;
	static struct nk_color editcolor = { 255, 255, 255, 255 };
	static char strbuf[32], anim_name[32];
	static struct nk_rect bounds;
	TEX_DATA data;
	Pos size;
	int temp, px, py, sx, sy, state;
	GLuint tex;
	static int8 anim_speed = 1, atlas;
	struct nk_image texid;
	char str[128], files[(512 + 32) * MAX_PATH], *tok, path3[MAX_PATH], *path2, path[MAX_PATH], strerror[512];
	int16 tmp;

	OPENFILENAME ofn;
	ZeroMemory(&files, sizeof(files));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.tga;*.bmp;*.psd\0Any File\0*.*\0";
	ofn.nMaxFile = MAX_PATH + (32 * 512);
	ofn.lpstrFile = files;
	ofn.lpstrTitle = "Select textures to import";
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	if (nkrendered == 0)
	{
		if (nk_begin(ctx, "Tool pannel", nk_rect(0, 30, st.screenx * 0.20f, (st.screeny - 30) / 2), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER | NK_WINDOW_TITLE))
		{
			nk_layout_row_dynamic(ctx, 30, 2);
			mtex.dn_mode = nk_option_label(ctx, "Diffuse texture", mtex.dn_mode == 0) ? 0 : mtex.dn_mode;
			mtex.dn_mode = nk_option_label(ctx, "Normal map", mtex.dn_mode == 1) ? 1 : mtex.dn_mode;

			temp = 1;

			if (mtex.mult_selection > 0)
			{
				for (i = mtex.first_sel; i < mtex.last_sel + 1; i++)
				{
					if (mtex.mgg.fnn[i] < 1024 && mtex.mgg.fnn[i] != -1)
					{
						temp = 0;
						break;
					}
				}
			}

			nk_layout_row_dynamic(ctx, 30, 1);

			if (mtex.dn_mode && (mtex.selected == -1 || (mtex.mgg.fnn[mtex.selected] != -1 && mtex.mgg.fnn[mtex.selected] < 1024) || temp == 0))
			{
				ctx->style.button.normal = ctx->style.button.hover = ctx->style.button.active;
				nk_button_label(ctx, "Import textures");
				SetThemeBack();
			}
			else
			{
				if (nk_button_label(ctx, "Import textures"))
				{
					if (GetOpenFileName(&ofn))
					{
						state = 1;
						i = 0;
						if (!mtex.dn_mode)
						{
							while (state == 1)
							{
								if (i == 0)
								{
									tok = StrTokNull(files);
									strcpy(path3, tok);
								}
								else
								{
									tok = StrTokNull(NULL);

									if (!tok)
									{
										if (i == 1)
										{
											path2 = CopyThisFile(files, mtex.mgg.path);

											if (path2 == NULL)
											{
												sprintf(strerror, "Error: Texture %s could not be opened", files);
												MessageBox(NULL, strerror, "Error", MB_OK);
											}

											if (path2 == -1)
											{
												sprintf(strerror, "Error: Texture %s could not be copied", files);
												MessageBox(NULL, strerror, "Error", MB_OK);
											}

											if (path2 == -2)
												path2 = files;

											if ((tex = LoadTexture(path2, mtex.mgg.mipmap, &size)) == -1)
											{
												sprintf(strerror, "Error: Texture %s could not be loaded", files);
												MessageBox(NULL, strerror, "Error", MB_OK);
												break;
											}

											mtex.mgg.ftime[mtex.mgg.num_frames] = *GetFileLastModification(path2);

											mtex.textures = realloc(mtex.textures, (mtex.mgg.num_frames + 1) * sizeof(GLuint));
											mtex.textures_n = realloc(mtex.textures_n, (mtex.mgg.num_frames + 1) * sizeof(GLuint));

											mtex.textures[mtex.mgg.num_frames] = tex;

											mtex.size = realloc(mtex.size, (mtex.mgg.num_frames + 1) * sizeof(Pos));
											memcpy(&mtex.size[mtex.mgg.num_frames], &size, sizeof(Pos));

											len = strlen(path2);

											for (j = len; j > 0; j--)
											{
												if (path2[j] == '\\' || path2[j] == '/')
												{
													j++;
													break;
												}
											}

											strcpy(mtex.mgg.texnames[mtex.mgg.num_frames], path2 + j);
											strcpy(mtex.mgg.files[mtex.mgg.num_frames], path2 + j);

											mtex.mgg.fn = realloc(mtex.mgg.fn, (mtex.mgg.num_frames + 1) * sizeof(int16));
											mtex.mgg.fnn = realloc(mtex.mgg.fnn, (mtex.mgg.num_frames + 1) * sizeof(int16));
											mtex.mgg.frames_atlas = realloc(mtex.mgg.frames_atlas, (mtex.mgg.num_frames + 1) * sizeof(int8));

											mtex.mgg.frames_atlas[mtex.mgg.num_frames] = -1;

											mtex.mgg.fn[mtex.mgg.num_frames] = mtex.mgg.num_frames;
											mtex.mgg.fnn[mtex.mgg.num_frames] = -1;

											mtex.mgg.num_frames++;

											tok = CheckForNormal(files);

											if (tok)
											{
												path2 = CopyThisFile(tok, mtex.mgg.path);

												if (path2 == NULL)
												{
													sprintf(strerror, "Error: Texture %s could not be opened", tok);
													MessageBox(NULL, strerror, "Error", MB_OK);
													goto FINALIZE_LOOP;
												}

												if (path2 == -1)
												{
													sprintf(strerror, "Error: Texture %s could not be copied", tok);
													MessageBox(NULL, strerror, "Error", MB_OK);
													goto FINALIZE_LOOP;
												}

												if (path2 == -2)
													path2 = tok;

												if ((tex = LoadTexture(path2, mtex.mgg.mipmap, &size)) == -1)
												{
													sprintf(strerror, "Error: Texture %s could not be loaded", tok);
													MessageBox(NULL, strerror, "Error", MB_OK);
													goto FINALIZE_LOOP;
												}

												mtex.mgg.ftime_n[mtex.mgg.num_frames - 1] =*GetFileLastModification(path2);

												if (size.x != mtex.size[mtex.mgg.fn[mtex.mgg.num_frames - 1]].x || size.y != mtex.size[mtex.mgg.fn[mtex.mgg.num_frames - 1]].y)
												{
													sprintf(strerror, "Error: Normal texture %s has a different width and height from the diffuse texture", files);
													MessageBox(NULL, strerror, "Error", MB_OK);
													glDeleteTextures(1, &tex);
													goto FINALIZE_LOOP;
												}

												//mtex.textures_n = realloc(mtex.textures_n, (mtex.mgg.num_frames) * sizeof(GLuint));

												mtex.textures_n[mtex.mgg.num_frames - 1] = tex;

												len = strlen(tok);

												for (j = len; j > 0; j--)
												{
													if (tok[j] == '\\' || tok[j] == '/')
													{
														j++;
														break;
													}
												}

												strcpy(mtex.mgg.texnames_n[mtex.mgg.num_frames - 1], tok + j);
												strcpy(mtex.mgg.files_n[mtex.mgg.num_frames - 1], tok + j);

												mtex.mgg.fnn[mtex.mgg.num_frames - 1] = mtex.mgg.num_frames - 1;

												mtex.mgg.num_f_n++;
											}
										}
									FINALIZE_LOOP:
										state = 2;
										i = 0;
										break;
									}
									else
									{
										strcpy(path, path3);
										strcat(path, "\\");
										strcat(path, tok);

										path2 = CopyThisFile(path, mtex.mgg.path);

										if (path2 == NULL)
										{
											sprintf(strerror, "Error: Texture %s could not be opened", tok);
											MessageBox(NULL, strerror, "Error", MB_OK);
											break;
										}

										if (path2 == -1)
										{
											sprintf(strerror, "Error: Texture %s could not be copied", tok);
											MessageBox(NULL, strerror, "Error", MB_OK);
											break;
										}

										if (path2 == -2)
											path2 = path;

										if ((tex = LoadTexture(path2, mtex.mgg.mipmap, &size)) == -1)
										{
											sprintf(strerror, "Error: Texture %s could not be loaded", tok);
											MessageBox(NULL, strerror, "Error", MB_OK);
											break;
										}

										mtex.mgg.ftime[mtex.mgg.num_frames] = *GetFileLastModification(path2);

										mtex.textures = realloc(mtex.textures, (mtex.mgg.num_frames + 1) * sizeof(GLuint));
										mtex.textures_n = realloc(mtex.textures_n, (mtex.mgg.num_frames + 1) * sizeof(GLuint));

										mtex.textures[mtex.mgg.num_frames] = tex;

										mtex.size = realloc(mtex.size, (mtex.mgg.num_frames + 1) * sizeof(Pos));
										memcpy(&mtex.size[mtex.mgg.num_frames], &size, sizeof(Pos));

										mtex.mgg.fn = realloc(mtex.mgg.fn, (mtex.mgg.num_frames + 1) * sizeof(int16));
										mtex.mgg.fnn = realloc(mtex.mgg.fnn, (mtex.mgg.num_frames + 1) * sizeof(int16));
										mtex.mgg.frames_atlas = realloc(mtex.mgg.frames_atlas, (mtex.mgg.num_frames + 1) * sizeof(int8));
										mtex.mgg.frames_atlas[mtex.mgg.num_frames] = -1;

										strcpy(mtex.mgg.files[mtex.mgg.num_frames], tok);
										strcpy(mtex.mgg.texnames[mtex.mgg.num_frames], tok);

										mtex.mgg.fn[mtex.mgg.num_frames] = mtex.mgg.num_frames;
										mtex.mgg.fnn[mtex.mgg.num_frames] = -1;

										mtex.mgg.num_frames++;

										tok = CheckForNormal(path);

										if (tok)
										{
											path2 = CopyThisFile(tok, mtex.mgg.path);

											if (path2 == NULL)
											{
												sprintf(strerror, "Error: Texture %s could not be opened", tok);
												MessageBox(NULL, strerror, "Error", MB_OK);
												goto CONTINUE_LOOP;
											}

											if (path2 == -1)
											{
												sprintf(strerror, "Error: Texture %s could not be copied", tok);
												MessageBox(NULL, strerror, "Error", MB_OK);
												goto CONTINUE_LOOP;
											}

											if (path2 == -2)
												path2 = tok;

											if ((tex = LoadTexture(path2, mtex.mgg.mipmap, &size)) == -1)
											{
												sprintf(strerror, "Error: Texture %s could not be loaded", tok);
												MessageBox(NULL, strerror, "Error", MB_OK);
												goto CONTINUE_LOOP;
											}

											mtex.mgg.ftime[mtex.mgg.num_frames - 1] = *GetFileLastModification(path2);

											if (size.x != mtex.size[mtex.mgg.fn[mtex.mgg.num_frames - 1]].x || size.y != mtex.size[mtex.mgg.fn[mtex.mgg.num_frames - 1]].y)
											{
												sprintf(strerror, "Error: Normal texture %s has a different width and height from the diffuse texture", files);
												MessageBox(NULL, strerror, "Error", MB_OK);
												glDeleteTextures(1, &tex);
												goto CONTINUE_LOOP;
											}

											//mtex.textures_n = realloc(mtex.textures_n, (mtex.mgg.num_frames) * sizeof(GLuint));

											mtex.textures_n[mtex.mgg.num_frames - 1] = tex;

											len = strlen(tok);

											for (j = len; j > 0; j--)
											{
												if (tok[j] == '\\' || tok[j] == '/')
												{
													j++;
													break;
												}
											}

											strcpy(mtex.mgg.texnames_n[mtex.mgg.num_frames - 1], tok + j);
											strcpy(mtex.mgg.files_n[mtex.mgg.num_frames - 1], tok + j);

											mtex.mgg.fnn[mtex.mgg.num_frames - 1] = mtex.mgg.num_frames - 1;

											mtex.mgg.num_f_n++;
										}
									}
								}
							CONTINUE_LOOP:
								i++;
							}
						}
						else
						if (mtex.dn_mode && mtex.selected != -1 && temp == 1)
						{
							while (state == 1)
							{
								if (i == 0)
								{
									tok = StrTokNull(files);
									strcpy(path3, tok);
								}
								else
								{
									tok = StrTokNull(NULL);

									if (!tok)
									{
										if (i == 1)
										{
											path2 = CopyThisFile(files, mtex.mgg.path);

											if (path2 == NULL)
											{
												sprintf(strerror, "Error: Texture %s could not be opened", files);
												MessageBox(NULL, strerror, "Error", MB_OK);
											}

											if (path2 == -1)
											{
												sprintf(strerror, "Error: Texture %s could not be copied", files);
												MessageBox(NULL, strerror, "Error", MB_OK);
											}

											if (path2 == -2)
												path2 = files;

											if ((tex = LoadTexture(path2, mtex.mgg.mipmap, &size)) == -1)
											{
												sprintf(strerror, "Error: Texture %s could not be loaded", files);
												MessageBox(NULL, strerror, "Error", MB_OK);
												break;
											}

											mtex.mgg.ftime_n[mtex.selected] = *GetFileLastModification(mtex.mgg.fn[mtex.selected]);

											if (size.x != mtex.size[mtex.mgg.fn[mtex.selected]].x || size.y != mtex.size[mtex.mgg.fn[mtex.selected]].y)
											{
												sprintf(strerror, "Error: Normal texture %s has a different width and height from the diffuse texture", files);
												MessageBox(NULL, strerror, "Error", MB_OK);
												glDeleteTextures(1, &tex);
												break;
											}

											mtex.textures_n[mtex.selected] = tex;

											len = strlen(path2);

											for (j = len; j > 0; j--)
											{
												if (path2[j] == '\\' || path2[j] == '/')
												{
													j++;
													break;
												}
											}

											strcpy(mtex.mgg.texnames_n[mtex.selected], path2 + j);
											strcpy(mtex.mgg.files_n[mtex.selected], path2 + j);

											mtex.mgg.fnn[mtex.selected] = mtex.mgg.fn[mtex.selected];

											mtex.mgg.num_f_n++;

										}
										//FINALIZE_LOOP:
										state = 2;
										i = 0;
										break;
									}
									else
									{
										strcpy(path, path3);
										strcat(path, "\\");
										strcat(path, tok);

										path2 = CopyThisFile(path, mtex.mgg.path);

										if (path2 == NULL)
										{
											sprintf(strerror, "Error: Texture %s could not be opened", tok);
											MessageBox(NULL, strerror, "Error", MB_OK);
											break;
										}

										if (path2 == -1)
										{
											sprintf(strerror, "Error: Texture %s could not be copied", tok);
											MessageBox(NULL, strerror, "Error", MB_OK);
											break;
										}

										if (path2 == -2)
											path2 = path;

										if ((tex = LoadTexture(path2, mtex.mgg.mipmap, &size)) == -1)
										{
											sprintf(strerror, "Error: Texture %s could not be loaded", tok);
											MessageBox(NULL, strerror, "Error", MB_OK);
											break;
										}

										mtex.mgg.ftime_n[mtex.selected + i - 1] =*GetFileLastModification(path2);

										if (size.x != mtex.size[mtex.mgg.fn[mtex.selected + i - 1]].x || size.y != mtex.size[mtex.mgg.fn[mtex.selected + i - 1]].y)
										{
											sprintf(strerror, "Error: Normal texture %s has a different width and height from the diffuse texture", files);
											MessageBox(NULL, strerror, "Error", MB_OK);
											glDeleteTextures(1, &tex);
											break;
										}

										mtex.textures_n[mtex.selected + i - 1] = tex;

										len = strlen(path2);

										for (j = len; j > 0; j--)
										{
											if (path2[j] == '\\' || path2[j] == '/')
											{
												j++;
												break;
											}
										}

										strcpy(mtex.mgg.texnames_n[mtex.selected + i - 1], path2 + j);
										strcpy(mtex.mgg.files_n[mtex.selected + i - 1], path2 + j);

										mtex.mgg.fnn[mtex.selected + i - 1] = mtex.mgg.fn[mtex.selected + i - 1];

										mtex.mgg.num_f_n++;
									}
								}
								//CONTINUE_LOOP_2:
								i++;
							}
						}

					}

					state = 0;
				}
			}

			if (mtex.mult_selection == 0 || mtex.mult_selection == 1)
			{
				//if (!mtex.dn_mode)
				//{
					nk_layout_row_dynamic(ctx, 30, 2);

					ctx->style.button.normal = ctx->style.button.hover = ctx->style.button.active = nk_style_item_color(nk_rgb(64, 64, 64));
					nk_button_label(ctx, "Create atlas");
					nk_button_label(ctx, "Create animation");
					SetThemeBack();
				//}

				if (mtex.selected != -1)
				{
					nk_layout_row_dynamic(ctx, 30, 1);

					if (!mtex.dn_mode || (mtex.dn_mode && mtex.mgg.fnn[mtex.selected] != -1))
					{
						if (nk_button_label(ctx, "Move texture"))
						{
							mtex.command = MOV_TEX;
							atlas = mtex.mgg.frames_atlas[mtex.selected];
						}
					}

					if (!mtex.dn_mode || (mtex.dn_mode && mtex.mgg.fnn[mtex.selected] != -1))
					{
						if (nk_button_label(ctx, "Remove texture"))
						{
							if(!mtex.dn_mode) sprintf(str, "Are you sure you want to remove this texture \"%s\"?", mtex.mgg.texnames[mtex.mgg.fn[mtex.selected]]);
							else sprintf(str, "Are you sure you want to remove this normal texture \"%s\"?", mtex.mgg.texnames_n[mtex.mgg.fn[mtex.selected]]);
							if (MessageBox(NULL, str, "Warning", MB_YESNO) == IDYES)
							{
								if (!mtex.dn_mode) mtex.mgg.fn[mtex.selected] += 1024;
								mtex.mgg.fnn[mtex.selected] += 1024;
								if (!mtex.dn_mode) glDeleteTextures(1, &mtex.textures[mtex.mgg.fn[mtex.selected]]);
								glDeleteTextures(1, &mtex.textures[mtex.mgg.fnn[mtex.selected]]);
								mtex.selected = -1;
								mtex.sel_slot = -1;
								memset(mtex.selection, 0, 512 * sizeof(int));
								if (!mtex.dn_mode) UpdateAnims();
								if (!mtex.dn_mode) UpdateTexList();
							}
						}
					}
				}
			}
			
			if (mtex.mult_selection > 1)
			{
				if (mtex.dn_mode)
				{
					nk_layout_row_dynamic(ctx, 30, 2);

					ctx->style.button.normal = ctx->style.button.hover = ctx->style.button.active = nk_style_item_color(nk_rgb(64, 64, 64));
					nk_button_label(ctx, "Create atlas");
					nk_button_label(ctx, "Create animation");
					SetThemeBack();
				}
				else
				{
					nk_layout_row_dynamic(ctx, 30, 2);
					if (nk_button_label(ctx, "Create atlas"))
					{
						pannel_state = 3;
						if (mtex.mgg.num_c_atlas < 32)
						{
							if (mtex.mgg.num_c_atlas > 0)
							{
								for (i = mtex.first_sel; i < mtex.last_sel + 1; i++)
								{
									if (mtex.mgg.frames_atlas[i] != -1)
									{
										sprintf(str, "Error: frame number %d, is already in atlas %d", i, mtex.mgg.frames_atlas[i]);
										MessageBox(NULL, str, "Atlas creation error", MB_OK);
										pannel_state = 2;
										break;
									}
								}
							}

							if (pannel_state == 3)
							{
								for (i = mtex.first_sel; i < mtex.last_sel + 1; i++)
									mtex.mgg.frames_atlas[i] = mtex.mgg.num_c_atlas;

								if (mtex.mgg.num_c_atlas > 0)
								{
									mtex.mgg.num_f_a = realloc(mtex.mgg.num_f_a, (mtex.mgg.num_c_atlas + 1) * sizeof(int16));
									mtex.mgg.num_f0 = realloc(mtex.mgg.num_f0, (mtex.mgg.num_c_atlas + 1) * sizeof(int16));
									mtex.mgg.num_ff = realloc(mtex.mgg.num_ff, (mtex.mgg.num_c_atlas + 1) * sizeof(int16));
								}
								else
								{
									mtex.mgg.num_f_a = malloc((mtex.mgg.num_c_atlas + 1) * sizeof(int16));
									mtex.mgg.num_f0 = malloc((mtex.mgg.num_c_atlas + 1) * sizeof(int16));
									mtex.mgg.num_ff = malloc((mtex.mgg.num_c_atlas + 1) * sizeof(int16));
								}

								mtex.mgg.num_f0[mtex.mgg.num_c_atlas] = mtex.first_sel;
								mtex.mgg.num_ff[mtex.mgg.num_c_atlas] = mtex.last_sel;

								mtex.mgg.num_f_a[mtex.mgg.num_c_atlas] = mtex.last_sel - mtex.first_sel + 1;

								mtex.mgg.num_c_atlas++;

								pannel_state = 0;
							}

							if (pannel_state == 2) pannel_state = 0;
						}
					}

					if (nk_button_label(ctx, "Create animation"))
						pannel_state = 1;

					nk_layout_row_dynamic(ctx, 30, 1);
				}

				for (i = mtex.first_sel; i < mtex.last_sel + 1; i++)
				{
					if (mtex.mgg.fnn[i] < 1024 && mtex.mgg.fnn[i] != -1)
					{
						temp = 0;
						break;
					}
				}

				if (!mtex.dn_mode || (mtex.dn_mode && !temp))
				{

					if (nk_button_label(ctx, "Move textures"))
					{
						mtex.command = MOV_TEX;
						//mtex.command2 = mtex.anim_selected;
					}
				}

				if (!mtex.dn_mode || (mtex.dn_mode && !temp))
				{
					if (nk_button_label(ctx, "Remove textures"))
					{
						sprintf(str, "Are you sure you want to remove these textures %d to %d?", mtex.first_sel, mtex.last_sel);
						if (MessageBox(NULL, str, "Warning", MB_YESNO) == IDYES)
						{
							for (i = mtex.first_sel; i < mtex.last_sel + 1; i++)
							{
								if (!mtex.dn_mode) mtex.mgg.fn[i] += 1024;
								mtex.mgg.fnn[i] += 1024;

								if (!mtex.dn_mode) glDeleteTextures(1, &mtex.textures[mtex.mgg.fn[i]]);
								glDeleteTextures(1, &mtex.textures[mtex.mgg.fnn[i]]);

								mtex.selected = -1;
								mtex.sel_slot = -1;
								memset(mtex.selection, 0, 512 * sizeof(int));
							}
							if (!mtex.dn_mode) UpdateAnims();
							if (!mtex.dn_mode) UpdateTexList();
						}
					}
				}
			}

			if (mtex.command == MOV_TEX && !mtex.dn_mode)
			{
				nk_layout_row_dynamic(ctx, 190, 1);
				if (nk_group_begin(ctx, "Move texture(s)", NK_WINDOW_TITLE | NK_WINDOW_BORDER))
				{
				/*
					nk_layout_row_begin(ctx, NK_DYNAMIC, 25, 3);
					nk_layout_row_push(ctx, 0.1f);
					if (mtex.mgg.frames_atlas[mtex.selected] > -1)
					{
						if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_LEFT))
						{
							mtex.mgg.num_f_a[mtex.mgg.frames_atlas[mtex.selected]]--;
							mtex.mgg.frames_atlas[mtex.selected]--;
							mtex.canvas = mtex.mgg.frames_atlas[mtex.selected];
						}
					}
					else
						nk_spacing(ctx, 1);

					nk_layout_row_push(ctx, 0.8f);
					if (mtex.mgg.frames_atlas[mtex.selected] == -1)
						sprintf(str, "No atlas");
					else
						sprintf(str, "Atlas %d", mtex.mgg.frames_atlas[mtex.selected] + 1);

					ctx->style.button.normal = ctx->style.button.hover = ctx->style.button.active;
					nk_button_label(ctx, str);
					SetThemeBack();

					nk_layout_row_push(ctx, 0.1f);
					if (mtex.mgg.frames_atlas[mtex.selected] < mtex.mgg.num_c_atlas - 1)
					{
						if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT))
						{
							mtex.mgg.frames_atlas[mtex.selected]++;
							mtex.canvas = mtex.mgg.frames_atlas[mtex.selected];
						}
					}
					else
						nk_spacing(ctx, 1);

					nk_layout_row_end(ctx);
					*/

					if (mtex.canvas < mtex.mgg.num_c_atlas && mtex.mgg.num_c_atlas > 0)
					{
						nk_layout_row_dynamic(ctx, 25, 1);

						if (nk_button_label(ctx, "Move out of the atlas") && mtex.mgg.frames_atlas[mtex.selected] != -1)
						{
							if (mtex.mult_selection == 0)
							{
								if (MessageBox(NULL, "Are you sure you want to move this texture out of this atlas?\nIt will be positioned as the last texture!",
									"Warning", MB_YESNO) == IDYES)
								{
									tmp = mtex.mgg.fn[mtex.selected];

									mtex.mgg.fn[mtex.selected] += 1024;

									for (i = mtex.selected; i < mtex.mgg.num_frames; i++)
										mtex.mgg.fn[i] = mtex.mgg.fn[i + 1];

									mtex.mgg.fn[mtex.mgg.num_frames - 1] = tmp;

									tmp = mtex.mgg.fnn[mtex.selected];

									for (i = mtex.selected; i < mtex.mgg.num_frames; i++)
										mtex.mgg.fnn[i] = mtex.mgg.fnn[i + 1];

									mtex.mgg.fnn[mtex.mgg.num_frames - 1] = tmp;

									mtex.mgg.num_ff[mtex.mgg.frames_atlas[mtex.selected]]--;
									mtex.mgg.num_f_a[mtex.mgg.frames_atlas[mtex.selected]]--;

									for (i = mtex.selected; i < mtex.mgg.num_frames; i++)
										mtex.mgg.frames_atlas[i] = mtex.mgg.frames_atlas[i + 1];

									mtex.mgg.frames_atlas[mtex.mgg.num_frames - 1] = -1;

									for (i = mtex.mgg.frames_atlas[mtex.selected] + 1; i < mtex.mgg.num_c_atlas; i++)
									{
										if (mtex.mgg.num_f0[i] > 0) mtex.mgg.num_f0[i]--;
										mtex.mgg.num_ff[i]--;
									}

									UpdateAnims();
									UpdateAtlasses();

									mtex.command = NONE;
								}
							}
							else
							{
								if (MessageBox(NULL, "Are you sure you want to move these textures out of this atlas?\nThey will be positioned as the last textures!",
									"Warning", MB_YESNO) == IDYES)
								{
									for (j = mtex.first_sel; j < mtex.last_sel + 1; j++)
									{
										tmp = mtex.mgg.fn[mtex.selected];

										mtex.mgg.fn[mtex.selected] += 1024;
										//UpdateAnims();

										for (i = mtex.selected; i < mtex.mgg.num_frames; i++)
											mtex.mgg.fn[i] = mtex.mgg.fn[i + 1];

										mtex.mgg.fn[mtex.mgg.num_frames - 1] = tmp;

										tmp = mtex.mgg.fnn[mtex.selected];

										for (i = mtex.selected; i < mtex.mgg.num_frames; i++)
											mtex.mgg.fnn[i] = mtex.mgg.fnn[i + 1];

										mtex.mgg.fnn[mtex.mgg.num_frames - 1] = tmp;

										mtex.mgg.num_ff[mtex.mgg.frames_atlas[mtex.selected]]--;
										mtex.mgg.num_f_a[mtex.mgg.frames_atlas[mtex.selected]]--;

										for (i = mtex.selected; i < mtex.mgg.num_frames; i++)
											mtex.mgg.frames_atlas[i] = mtex.mgg.frames_atlas[i + 1];

										mtex.mgg.frames_atlas[mtex.mgg.num_frames - 1] = -1;

										for (i = mtex.mgg.frames_atlas[mtex.selected] + 1; i < mtex.mgg.num_c_atlas; i++)
										{
											if (mtex.mgg.num_f0[i] > 0) mtex.mgg.num_f0[i]--;
											mtex.mgg.num_ff[i]--;
										}
									}

									UpdateAnims();
									UpdateAtlasses();

									mtex.command = NONE;
								}
							}
						}

						if (mtex.mgg.num_c_atlas > 1)
						{

							nk_layout_row_begin(ctx, NK_DYNAMIC, 25, 4);

							nk_layout_row_push(ctx, 0.1f);
							if (atlas > 0)
							{
								if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_LEFT))
									atlas--;
							}
							else
								nk_spacing(ctx, 1);

							nk_layout_row_push(ctx, 0.3f);
							ctx->style.button.normal = ctx->style.button.hover = ctx->style.button.active;
							sprintf(str, "Atlas %d", atlas + 1);
							nk_button_label(ctx, str);
							SetThemeBack();

							nk_layout_row_push(ctx, 0.1f);
							if (atlas < mtex.mgg.num_c_atlas - 1)
							{
								if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT))
									atlas++;
							}
							else
								nk_spacing(ctx, 1);

							nk_layout_row_push(ctx, 0.5f);
							if (atlas == mtex.mgg.frames_atlas[mtex.selected])
							{
								ctx->style.button.normal = ctx->style.button.hover = ctx->style.button.active;
								nk_button_label(ctx, "Move to atlas...");
								SetThemeBack();
							}
							else
							{
								if (nk_button_label(ctx, "Move to atlas..."))
								{
									if (mtex.mult_selection == 0)
									{
										if (MessageBox(NULL, "Are you sure you want to move this texture to other atlas?\nIt will be positioned as the last texture!",
											"Warning", MB_YESNO) == IDYES)
										{
											tmp = mtex.mgg.fn[mtex.selected];

											mtex.mgg.fn[mtex.selected] += 1024;
											//UpdateAnims();

											if (mtex.mgg.num_ff[atlas] > mtex.selected)
											{
												for (i = mtex.selected; i < mtex.mgg.num_ff[atlas] + 1; i++)
													mtex.mgg.fn[i] = mtex.mgg.fn[i + 1];

												mtex.mgg.fn[mtex.mgg.num_ff[atlas]] = tmp;

												//mtex.mgg.num_ff[atlas]++;

												mtex.mgg.num_f0[atlas]--;
												mtex.mgg.num_f_a[atlas]++;
											}
											else
											{
												for (i = mtex.selected; i > mtex.mgg.num_ff[atlas] + 1; i--)
													mtex.mgg.fn[i] = mtex.mgg.fn[i - 1];

												mtex.mgg.fn[mtex.mgg.num_ff[atlas] + 1] = tmp;

												mtex.mgg.num_ff[atlas]++;

												//mtex.mgg.num_f0[atlas]--;
												mtex.mgg.num_f_a[atlas]++;
											}

											mtex.mgg.num_ff[mtex.mgg.frames_atlas[mtex.selected]]--;
											mtex.mgg.num_f_a[mtex.mgg.frames_atlas[mtex.selected]]--;

											tmp = mtex.mgg.fnn[mtex.selected];

											if (mtex.mgg.num_ff[atlas] > mtex.selected)
											{
												for (i = mtex.selected; i < mtex.mgg.num_ff[atlas]; i++)
													mtex.mgg.fnn[i] = mtex.mgg.fnn[i + 1];
											}
											else
											{
												for (i = mtex.selected; i > mtex.mgg.num_ff[atlas] + 1; i--)
													mtex.mgg.fnn[i] = mtex.mgg.fn[i - 1];
											}

											mtex.mgg.fnn[mtex.mgg.num_ff[atlas]] = tmp;

											if (mtex.mgg.num_ff[atlas] > mtex.selected)
											{
												for (i = mtex.selected; i < mtex.mgg.num_ff[atlas]; i++)
													mtex.mgg.frames_atlas[i] = mtex.mgg.frames_atlas[i + 1];
											}
											else
											{
												for (i = mtex.selected; i > mtex.mgg.num_ff[atlas] + 1; i--)
													mtex.mgg.frames_atlas[i] = mtex.mgg.frames_atlas[i - 1];
											}

											mtex.mgg.frames_atlas[mtex.mgg.num_ff[atlas]] = atlas;

											if (mtex.mgg.num_ff[atlas] > mtex.selected)
											{
												for (i = mtex.mgg.frames_atlas[mtex.selected] + 1; i < atlas + 1; i++)
												{
													if (mtex.mgg.num_f0[i] > 0) mtex.mgg.num_f0[i]--;
													mtex.mgg.num_ff[i]--;
												}
											}
											else
											{
												for (i = mtex.mgg.frames_atlas[mtex.selected] + 1; i >= atlas; i--)
												{
													if (mtex.mgg.num_f0[i] > 0) mtex.mgg.num_f0[i]--;
													mtex.mgg.num_ff[i]--;
												}
											}

											UpdateAnims();
											UpdateAtlasses();

											mtex.command = NONE;
										}
									}
									else
									{
										if (MessageBox(NULL, "Are you sure you want to move these textures to other atlas?\nThey will be positioned as the last textures!",
											"Warning", MB_YESNO) == IDYES)
										{
											for (j = mtex.first_sel, k = 0; k < mtex.mult_selection + 1; k++)
											{
												tmp = mtex.mgg.fn[j];

												mtex.mgg.fn[j] += 1024;
												//UpdateAnims();

												if (mtex.mgg.num_ff[atlas] > j)
												{
													for (i = j; i < mtex.mgg.num_ff[atlas] + 1; i++)
														mtex.mgg.fn[i] = mtex.mgg.fn[i + 1];

													mtex.mgg.fn[mtex.mgg.num_ff[atlas]] = tmp;

													//mtex.mgg.num_ff[atlas]++;

													mtex.mgg.num_f0[atlas]--;
													mtex.mgg.num_f_a[atlas]++;
												}
												else
												{
													for (i = j; i > mtex.mgg.num_ff[atlas] + 1; i--)
														mtex.mgg.fn[i] = mtex.mgg.fn[i - 1];

													mtex.mgg.fn[mtex.mgg.num_ff[atlas] + 1] = tmp;

													mtex.mgg.num_ff[atlas]++;

													//mtex.mgg.num_f0[atlas]--;
													mtex.mgg.num_f_a[atlas]++;
												}

												mtex.mgg.num_ff[mtex.mgg.frames_atlas[mtex.selected]]--;
												mtex.mgg.num_f_a[mtex.mgg.frames_atlas[mtex.selected]]--;

												tmp = mtex.mgg.fnn[j];

												if (mtex.mgg.num_ff[atlas] > j)
												{
													for (i = j; i < mtex.mgg.num_ff[atlas]; i++)
														mtex.mgg.fnn[i] = mtex.mgg.fnn[i + 1];
												}
												else
												{
													for (i = j; i > mtex.mgg.num_ff[atlas] + 1; i--)
														mtex.mgg.fnn[i] = mtex.mgg.fn[i - 1];
												}

												mtex.mgg.fnn[mtex.mgg.num_ff[atlas]] = tmp;

												if (mtex.mgg.num_ff[atlas] > j)
												{
													for (i = j; i < mtex.mgg.num_ff[atlas]; i++)
														mtex.mgg.frames_atlas[i] = mtex.mgg.frames_atlas[i + 1];
												}
												else
												{
													for (i = j; i > mtex.mgg.num_ff[atlas] + 1; i--)
														mtex.mgg.frames_atlas[i] = mtex.mgg.frames_atlas[i - 1];
												}

												mtex.mgg.frames_atlas[mtex.mgg.num_ff[atlas]] = atlas;

												if (mtex.mgg.num_ff[atlas] > j)
												{
													for (i = mtex.mgg.frames_atlas[j] + 1; i < atlas + 1; i++)
													{
														if (mtex.mgg.num_f0[i] > 0) mtex.mgg.num_f0[i]--;
														mtex.mgg.num_ff[i]--;
													}
												}
												else
												{
													for (i = mtex.mgg.frames_atlas[j] + 1; i >= atlas; i--)
													{
														if (mtex.mgg.num_f0[i] > 0) mtex.mgg.num_f0[i]--;
														mtex.mgg.num_ff[i]--;
													}

													j++;
												}
											}

											UpdateAnims();
											UpdateAtlasses();

											mtex.command = NONE;
										}
									}
								}
							}

							nk_layout_row_end(ctx);
						}
					}
					//else
					if (mtex.canvas == mtex.mgg.num_c_atlas && mtex.mgg.num_c_atlas > 0)
					{
						if (mtex.mgg.num_c_atlas > 0)
						{

							nk_layout_row_begin(ctx, NK_DYNAMIC, 25, 4);

							nk_layout_row_push(ctx, 0.1f);
							if (atlas > 0)
							{
								if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_LEFT))
									atlas--;
							}
							else
								nk_spacing(ctx, 1);

							nk_layout_row_push(ctx, 0.3f);
							ctx->style.button.normal = ctx->style.button.hover = ctx->style.button.active;
							if(atlas == -1)
								sprintf(str, "No atlas");
							else
								sprintf(str, "Atlas %d", atlas + 1);
							nk_button_label(ctx, str);
							SetThemeBack();

							nk_layout_row_push(ctx, 0.1f);
							if (atlas < mtex.mgg.num_c_atlas - 1)
							{
								if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT))
									atlas++;
							}
							else
								nk_spacing(ctx, 1);

							nk_layout_row_push(ctx, 0.5f);
							
							if (atlas == mtex.mgg.frames_atlas[mtex.selected])
							{
								ctx->style.button.normal = ctx->style.button.hover = ctx->style.button.active;
								nk_button_label(ctx, "Move to atlas...");
								SetThemeBack();
							}
							else
							{
							
								if (nk_button_label(ctx, "Move to atlas..."))
								{
									if (mtex.mult_selection == 0)
									{
										if (MessageBox(NULL, "Are you sure you want to move this texture to an atlas?\nIt will be positioned as the last texture!",
											"Warning", MB_YESNO) == IDYES)
										{
											tmp = mtex.mgg.fn[mtex.selected];

											mtex.mgg.fn[mtex.selected] += 1024;
											//UpdateAnims();

											if (mtex.mgg.num_ff[atlas] > mtex.selected)
											{
												for (i = mtex.selected; i < mtex.mgg.num_ff[atlas] + 1; i++)
													mtex.mgg.fn[i] = mtex.mgg.fn[i + 1];

												mtex.mgg.fn[mtex.mgg.num_ff[atlas]] = tmp;

												//mtex.mgg.num_ff[atlas]++;

												mtex.mgg.num_f0[atlas]--;
												mtex.mgg.num_f_a[atlas]++;
											}
											else
											{
												for (i = mtex.selected; i > mtex.mgg.num_ff[atlas] + 1; i--)
													mtex.mgg.fn[i] = mtex.mgg.fn[i - 1];

												mtex.mgg.fn[mtex.mgg.num_ff[atlas] + 1] = tmp;

												mtex.mgg.num_ff[atlas]++;

												//mtex.mgg.num_f0[atlas]--;
												mtex.mgg.num_f_a[atlas]++;
											}

											//mtex.mgg.num_ff[mtex.mgg.frames_atlas[mtex.selected]]--;
											//mtex.mgg.num_f_a[mtex.mgg.frames_atlas[mtex.selected]]--;

											tmp = mtex.mgg.fnn[mtex.selected];

											if (mtex.mgg.num_ff[atlas] > mtex.selected)
											{
												for (i = mtex.selected; i < mtex.mgg.num_ff[atlas]; i++)
													mtex.mgg.fnn[i] = mtex.mgg.fnn[i + 1];
											}
											else
											{
												for (i = mtex.selected; i > mtex.mgg.num_ff[atlas] + 1; i--)
													mtex.mgg.fnn[i] = mtex.mgg.fn[i - 1];
											}

											mtex.mgg.fnn[mtex.mgg.num_ff[atlas]] = tmp;

											if (mtex.mgg.num_ff[atlas] > mtex.selected)
											{
												for (i = mtex.selected; i < mtex.mgg.num_ff[atlas]; i++)
													mtex.mgg.frames_atlas[i] = mtex.mgg.frames_atlas[i + 1];
											}
											else
											{
												for(i = mtex.selected; i > mtex.mgg.num_ff[atlas] + 1; i--)
													mtex.mgg.frames_atlas[i] = mtex.mgg.frames_atlas[i - 1];
											}

											mtex.mgg.frames_atlas[mtex.mgg.num_ff[atlas]] = atlas;

											if (mtex.mgg.num_ff[atlas] > mtex.selected)
											{
												for (i = mtex.mgg.frames_atlas[mtex.selected] + 1; i < atlas + 1; i++)
												{
													if(mtex.mgg.num_f0[i] > 0) mtex.mgg.num_f0[i]--;
													mtex.mgg.num_ff[i]--;
												}
											}
											else
											{
												for (i = mtex.mgg.frames_atlas[mtex.selected] + 1; i >= atlas; i--)
												{
													if (mtex.mgg.num_f0[i] > 0) mtex.mgg.num_f0[i]--;
													mtex.mgg.num_ff[i]--;
												}
											}

											UpdateAnims();
											UpdateAtlasses();

											mtex.command = NONE;
										}
									}
									else
									{
										if (MessageBox(NULL, "Are you sure you want to move these textures to an atlas?\nThey will be positioned as the last textures!",
											"Warning", MB_YESNO) == IDYES)
										{
											for (j = mtex.first_sel, k = 0; k < mtex.mult_selection + 1; k++)
											{
												tmp = mtex.mgg.fn[j];

												mtex.mgg.fn[j] += 1024;
												//UpdateAnims();

												if (mtex.mgg.num_ff[atlas] > j)
												{
													for (i = j; i < mtex.mgg.num_ff[atlas] + 1; i++)
														mtex.mgg.fn[i] = mtex.mgg.fn[i + 1];

													mtex.mgg.fn[mtex.mgg.num_ff[atlas]] = tmp;

													//mtex.mgg.num_ff[atlas]++;

													mtex.mgg.num_f0[atlas]--;
													mtex.mgg.num_f_a[atlas]++;
												}
												else
												{
													for (i = j; i > mtex.mgg.num_ff[atlas] + 1; i--)
														mtex.mgg.fn[i] = mtex.mgg.fn[i - 1];

													mtex.mgg.fn[mtex.mgg.num_ff[atlas] + 1] = tmp;

													mtex.mgg.num_ff[atlas]++;

													//mtex.mgg.num_f0[atlas]--;
													mtex.mgg.num_f_a[atlas]++;
												}

												//mtex.mgg.num_ff[mtex.mgg.frames_atlas[mtex.selected]]--;
												//mtex.mgg.num_f_a[mtex.mgg.frames_atlas[mtex.selected]]--;

												tmp = mtex.mgg.fnn[j];

												if (mtex.mgg.num_ff[atlas] > j)
												{
													for (i = j; i < mtex.mgg.num_ff[atlas]; i++)
														mtex.mgg.fnn[i] = mtex.mgg.fnn[i + 1];
												}
												else
												{
													for (i = j; i > mtex.mgg.num_ff[atlas] + 1; i--)
														mtex.mgg.fnn[i] = mtex.mgg.fn[i - 1];
												}

												mtex.mgg.fnn[mtex.mgg.num_ff[atlas]] = tmp;

												if (mtex.mgg.num_ff[atlas] > j)
												{
													for (i = j; i < mtex.mgg.num_ff[atlas]; i++)
														mtex.mgg.frames_atlas[i] = mtex.mgg.frames_atlas[i + 1];
												}
												else
												{
													for (i = j; i > mtex.mgg.num_ff[atlas] + 1; i--)
														mtex.mgg.frames_atlas[i] = mtex.mgg.frames_atlas[i - 1];
												}

												mtex.mgg.frames_atlas[mtex.mgg.num_ff[atlas]] = atlas;

												if (mtex.mgg.num_ff[atlas] > j)
												{
													for (i = mtex.mgg.frames_atlas[j] + 1; i < atlas + 1; i++)
													{
														if (mtex.mgg.num_f0[i] > 0) mtex.mgg.num_f0[i]--;
														mtex.mgg.num_ff[i]--;
													}
												}
												else
												{
													for (i = mtex.mgg.frames_atlas[j] + 1; i >= atlas; i--)
													{
														if (mtex.mgg.num_f0[i] > 0) mtex.mgg.num_f0[i]--;
														mtex.mgg.num_ff[i]--;
													}

													j++;
												}
											}

											UpdateAnims();
											UpdateAtlasses();

											mtex.command = NONE;
										}
									}
								}
							}

							nk_layout_row_end(ctx);
						}
					}

					nk_layout_row_dynamic(ctx, 25, 1);

					nk_checkbox_label(ctx, "Switch places", &mtex.switch_place);

					nk_group_end(ctx);
				}
			}

			if (mtex.anim_selected != -1 && !mtex.dn_mode)
			{
				nk_layout_row_dynamic(ctx, 200, 1);
				if (nk_group_begin(ctx, "Animation", NK_WINDOW_TITLE | NK_WINDOW_BORDER))
				{
					nk_layout_row_dynamic(ctx, 25, 1);
					nk_edit_string_zero_terminated(ctx, NK_EDIT_SIMPLE, mtex.mgg.mga[mtex.anim_selected].name, 32, nk_filter_default);

					nk_layout_row_dynamic(ctx, 25, 2);
					mtex.mgg.mga[mtex.anim_selected].startID = nk_propertyi(ctx, "First", 0, mtex.mgg.mga[mtex.anim_selected].startID, mtex.mgg.num_frames - 1, 1, 1);
					mtex.mgg.mga[mtex.anim_selected].endID = nk_propertyi(ctx, "Last", mtex.mgg.mga[mtex.anim_selected].startID,
						mtex.mgg.mga[mtex.anim_selected].endID, mtex.mgg.num_frames - 1, 1, 1);
					nk_layout_row_dynamic(ctx, 25, 1);
					mtex.mgg.mga[mtex.anim_selected].speed = nk_propertyi(ctx, "Speed", -127, mtex.mgg.mga[mtex.anim_selected].speed, 127, 1, 1);

					if (nk_button_label(ctx, "Move animation"))
					{
						mtex.command = MOV_ANIM;
						mtex.command2 = mtex.anim_selected;
					}

					if (nk_button_label(ctx, "Remove animation"))
					{
						sprintf(str, "Are you sure you want to remove this animation \"%s\"?", mtex.mgg.mga[mtex.anim_selected].name);
						if (MessageBox(NULL, str, "Warning",MB_YESNO)==IDYES)
						{
							mtex.mgg.an[mtex.anim_slot] += 1024;
							mtex.anim_selected = -1;
							mtex.anim_slot = -1;
						}
					}

					memset(mtex.selection, 0, 512 * sizeof(int));
					for (j = mtex.mgg.mga[mtex.anim_selected].startID; j < mtex.mgg.mga[mtex.anim_selected].endID + 1; j++)
						mtex.selection[j] = 1;

					nk_group_end(ctx);
				}
			}

			if (pannel_state == 1 && !mtex.dn_mode)
			{
				nk_layout_row_dynamic(ctx, 140, 1);
				if (nk_group_begin(ctx, "New animation",NK_WINDOW_TITLE | NK_WINDOW_BORDER))
				{
					nk_layout_row_dynamic(ctx, 25, 1);
					nk_edit_string(ctx, NK_EDIT_SIMPLE, anim_name, &len, 32, nk_filter_default);
					anim_speed = nk_propertyi(ctx, "Speed", -127, anim_speed, 127, 1, 1);

					if (!len)
					{
						ctx->style.button.normal = ctx->style.button.hover = ctx->style.button.active = nk_style_item_color(nk_rgb(64, 64, 64));
						nk_button_label(ctx, "Create");
					}
					else
					{
						if (nk_button_label(ctx, "Create"))
						{
							mtex.mgg.num_anims++;
							mtex.mgg.mga = realloc(mtex.mgg.mga, mtex.mgg.num_anims * sizeof(_MGGANIM));
							mtex.mgg.an = realloc(mtex.mgg.an, mtex.mgg.num_anims * sizeof(int16));
							mtex.mgg.an[mtex.mgg.num_anims - 1] = mtex.mgg.num_anims - 1;
							strcpy(mtex.mgg.mga[mtex.mgg.num_anims - 1].name, anim_name);
							mtex.mgg.mga[mtex.mgg.num_anims - 1].speed = anim_speed;
							mtex.mgg.mga[mtex.mgg.num_anims - 1].startID = mtex.first_sel;
							mtex.mgg.mga[mtex.mgg.num_anims - 1].endID = mtex.last_sel;
							mtex.mgg.mga[mtex.mgg.num_anims - 1].current_frame = mtex.first_sel;
							mtex.mgg.mga[mtex.mgg.num_anims - 1].num_frames = mtex.last_sel - mtex.first_sel + 1;
							anim_speed = 1;
							memset(anim_name, 0, 32);
							len = 0;
							pannel_state = 0;
						}
					}

					nk_group_end(ctx);
				}
			}
		}

		nk_end(ctx);
	}
}

void ViewerBox()
{
	struct nk_rect vec4;
	float x2, y2, x3, y3, x4, y4;
	static float zoom = 2.0f;
	int32 x, y, i;
	char str[8];

	if (nk_begin(ctx, "Texture viewer", nk_rect(0, 30 + ((st.screeny - 30) / 2), st.screenx * 0.20f, (st.screeny - 30) / 2),
		NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		nk_layout_space_begin(ctx, NK_DYNAMIC, st.screenx * 0.20f, 5);

		vec4 = nk_layout_space_bounds(ctx);

		//Grid
		for (i = 0; i < vec4.w; i += 32)
			nk_stroke_line(nk_window_get_canvas(ctx), i + vec4.x, vec4.y, i + vec4.x, vec4.y + vec4.h, 1.0f, nk_rgb(128, 128, 128));
		for (i = 0; i < vec4.h; i += 32)
			nk_stroke_line(nk_window_get_canvas(ctx), vec4.x, i + vec4.y, vec4.w + vec4.x, i + vec4.y, 1.0f, nk_rgb(128, 128, 128));

		nk_stroke_line(nk_window_get_canvas(ctx), vec4.x + (vec4.w / 2), vec4.y, vec4.x + (vec4.w / 2), vec4.y + vec4.h, 3.0f, nk_rgb(255, 0, 0));
		nk_stroke_line(nk_window_get_canvas(ctx), vec4.x, vec4.y + (vec4.h / 2), vec4.x + vec4.w, vec4.y + (vec4.h / 2), 3.0f, nk_rgb(255, 0, 0));

		if (mtex.anim_selected != -1)
			mtex.selected = mtex.anim_frame;
		
		x = mtex.mgg.frameoffset_x[mtex.mgg.fn[mtex.selected]];
		y = mtex.mgg.frameoffset_y[mtex.mgg.fn[mtex.selected]];

		WTS(&x, &y);

		vec4.x = ((vec4.w / 2));
		vec4.y = ((vec4.h / 2));

		x2 = (vec4.x) / vec4.w;
		y2 = (vec4.y) / vec4.h;

		x3 = (mtex.size[mtex.mgg.fn[mtex.selected]].x * zoom) / vec4.w;
		y3 = (mtex.size[mtex.mgg.fn[mtex.selected]].y * zoom) / vec4.h;

		x4 = (x * zoom) / vec4.w;
		y4 = (y * zoom) / vec4.w;
		
		nk_layout_space_push(ctx, nk_rect(x2 - (x3/2) + x4, y2 - (y3/2) + y4, x3, y3));

		nk_image(ctx, nk_image_id(mtex.textures[mtex.mgg.fn[mtex.selected]]));
		
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

		nk_layout_row_dynamic(ctx, 20, 2);

		mtex.mgg.frameoffset_x[mtex.mgg.fn[mtex.selected]] = nk_propertyi(ctx, "X offset", -16384, mtex.mgg.frameoffset_x[mtex.mgg.fn[mtex.selected]], 16384, 4, 1);
		mtex.mgg.frameoffset_y[mtex.mgg.fn[mtex.selected]] = nk_propertyi(ctx, "Y offset", -16384, mtex.mgg.frameoffset_y[mtex.mgg.fn[mtex.selected]], 16384, 4, 1);

		if (mtex.anim_selected != -1)
		{
			nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 7);
			nk_style_set_font(ctx, &fonts[1]->handle);

			nk_layout_row_push(ctx, 0.1f);
			if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_LEFT))
				mtex.selected = mtex.mgg.mga[mtex.anim_selected].startID;

			nk_layout_row_push(ctx, 0.1f);
			if (mtex.selected == mtex.mgg.mga[mtex.anim_selected].startID)
			{
				ctx->style.button.normal = ctx->style.button.active = ctx->style.button.hover;
				nk_button_label(ctx, "E");
				SetThemeBack();
			}
			else
			{
				if (nk_button_label(ctx, "E"))
				{
					if (mtex.selected > mtex.mgg.mga[mtex.anim_selected].startID - 1)
						mtex.selected--;
				}
			}

			nk_layout_row_push(ctx, 0.2f);
			if (nk_button_label(ctx, "D"))
			{
				mtex.play = 0;
				mtex.selected = mtex.mgg.mga[mtex.anim_selected].startID;
			}

			nk_layout_row_push(ctx, 0.2f);
			if (mtex.play)
			{
				ctx->style.button.normal = ctx->style.button.active = ctx->style.button.hover;
				nk_button_label(ctx, "B");
			}
			else
			{
				if (nk_button_label(ctx, "B"))
					mtex.play = 1;
			}

			SetThemeBack();

			nk_layout_row_push(ctx, 0.2f);
			if (!mtex.play)
			{
				ctx->style.button.normal = ctx->style.button.active = ctx->style.button.hover;
				nk_button_label(ctx, "C");
			}
			else
			{
				if (nk_button_label(ctx, "C"))
					mtex.play = 0;
			}

			SetThemeBack();

			nk_layout_row_push(ctx, 0.1f);
			if (mtex.selected == mtex.mgg.mga[mtex.anim_selected].endID)
			{
				ctx->style.button.normal = ctx->style.button.active = ctx->style.button.hover;
				nk_button_label(ctx, "F");
			}
			else
			{
				if (nk_button_label(ctx, "F"))
				{
					if (mtex.selected < mtex.mgg.mga[mtex.anim_selected].endID + 1)
						mtex.selected++;
				}
			}

			SetThemeBack();

			nk_layout_row_push(ctx, 0.1f);
			if(nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT))
				mtex.selected = mtex.mgg.mga[mtex.anim_selected].endID;

			nk_style_set_font(ctx, &fonts[0]->handle);
			//nk_style_set_font(ctx, &fonts[2]->handle);

			nk_layout_row_end(ctx);
		}
		else
		{
			nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 3);

			nk_layout_row_push(ctx, 0.1f);
			if (mtex.selected > 0)
			{
				if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_LEFT))
				{
					mtex.selected--;
					memset(mtex.selection, 0, 512 * sizeof(int));
					mtex.mult_selection = 0;
					mtex.selection[mtex.selected] = 1;
				}
			}
			else
				nk_spacing(ctx, 1);

			nk_layout_row_push(ctx, 0.8f);
			sprintf(str, "%d", mtex.selected);
			ctx->style.button.normal = ctx->style.button.active = ctx->style.button.hover;
			nk_button_label(ctx, str);
			SetThemeBack();

			nk_layout_row_push(ctx, 0.1f);
			if (mtex.selected < mtex.mgg.num_frames - 1)
			{
				if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT))
				{
					mtex.selected++;
					memset(mtex.selection, 0, 512 * sizeof(int));
					mtex.mult_selection = 0;
					mtex.selection[mtex.selected] = 1;
				}
			}
			else
				nk_spacing(ctx, 1);

			nk_layout_row_end(ctx);
		}

	}

	nk_end(ctx);
}

void Canvas()
{
	register int i, j, k, l, m, n;
	int names[8], mult_sel_count = 0, nm[8], counter = 0, texu, temp;
	char str[128];
	static int option = 0;
	struct nk_rect bounds;
	ctx->style.window.fixed_background = nk_style_item_color(nk_rgb(16, 16, 16));
	
	ctx->style.selectable.hover = nk_style_item_color(nk_rgb(206, 206, 206));
	ctx->style.selectable.normal_active = nk_style_item_color(nk_rgb(255, 128, 32));
	ctx->style.selectable.hover_active = nk_style_item_color(nk_rgb(255, 128, 32));

	if (nk_begin(ctx, mtex.mgg.name, nk_rect(0.20f * st.screenx, 30, 0.70f * st.screenx, st.screeny - 30), NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		option = mtex.canvas;

		if (!mtex.dn_mode)
		{

			if (option == mtex.mgg.num_c_atlas)
				sprintf(str, "Single textures");

			if (option == mtex.mgg.num_c_atlas + 1)
				sprintf(str, "All textures");

			if (option < mtex.mgg.num_c_atlas)
				sprintf(str, "Atlas %d", option + 1);

			nk_menubar_begin(ctx);

			nk_layout_row_dynamic(ctx, 25, 1);

			if (nk_combo_begin_label(ctx, str, nk_vec2(nk_widget_width(ctx), 128 * mtex.mgg.num_c_atlas + 2)))
			{
				for (i = 0; i < mtex.mgg.num_c_atlas + 2; i++)
				{
					nk_layout_row_dynamic(ctx, 25, 1);
					if (i == mtex.mgg.num_c_atlas)
					{
						if (nk_combo_item_label(ctx, "Single textures", NK_TEXT_ALIGN_LEFT))
							option = i;
					}

					if (i == mtex.mgg.num_c_atlas + 1)
					{
						if (nk_combo_item_label(ctx, "All textures", NK_TEXT_ALIGN_LEFT))
							option = i;
					}

					if (i < mtex.mgg.num_c_atlas)
					{
						sprintf(str, "Atlas %d", i + 1);
						if (nk_combo_item_label(ctx, str, NK_TEXT_ALIGN_LEFT))
							option = i;
					}
				}

				nk_combo_end(ctx);
			}

			nk_menubar_end(ctx);

			//if (option == mtex.mgg.num_c_atlas)
			//{
			nk_layout_row_dynamic(ctx, (0.70f * st.screenx) / 8.0f, 8);
			for (i = 0, j = 0; i < mtex.mgg.num_frames; i++, counter++)
			{
				if (mtex.mgg.fn[i] < 1024)
				{
					if ((option == mtex.mgg.num_c_atlas && mtex.mgg.frames_atlas[i] == -1) || (option == mtex.mgg.num_c_atlas + 1)
						|| (option < mtex.mgg.num_c_atlas && mtex.mgg.frames_atlas[i] == option))
					{
						if (j == 8)
						{
							nk_layout_row_dynamic(ctx, 20, 8);

							for (k = 0; k < 8; k++)
							{
								sprintf(str, "%d - %s", nm[k], mtex.mgg.texnames[names[k]]);
								nk_label(ctx, str, NK_TEXT_ALIGN_CENTERED);
							}

							j = 0;

							nk_layout_row_dynamic(ctx, (0.70f * st.screenx) / 8.0f, 8);
						}

						if (mtex.command == MOV_TEX)
						{
							if (mtex.mult_selection == 0)
							{
								bounds = nk_widget_bounds(ctx);
								bounds.h += 20;
								if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds))
								{
									nk_button_symbol(ctx, NK_SYMBOL_PLUS);

									if (nk_input_has_mouse_click(ctx, NK_BUTTON_LEFT))
									{
										if (mtex.switch_place)
										{
											l = mtex.mgg.fn[i];
											mtex.mgg.fn[i] = mtex.mgg.fn[mtex.selected];
											mtex.mgg.fn[mtex.selected] = l;

											l = mtex.mgg.fnn[i];
											mtex.mgg.fnn[i] = mtex.mgg.fnn[mtex.selected];
											mtex.mgg.fnn[mtex.selected] = l;
										}
										else
										{
											l = mtex.mgg.fn[mtex.selected];
											m = mtex.mgg.fnn[mtex.selected];

											if (i < mtex.selected)
											{
												for (k = mtex.selected - 1; k > i - 1; k--)
													mtex.mgg.fn[k + 1] = mtex.mgg.fn[k];

												for (k = mtex.selected - 1; k > i - 1; k--)
													mtex.mgg.fnn[k + 1] = mtex.mgg.fnn[k];

											}
											else
											{
												for (k = mtex.selected; k < i + 1; k++)
													mtex.mgg.fn[k] = mtex.mgg.fn[k + 1];

												for (k = mtex.selected; k < i + 1; k++)
													mtex.mgg.fnn[k] = mtex.mgg.fnn[k + 1];
											}

											mtex.mgg.fn[i] = l;
											mtex.mgg.fnn[i] = m;

											UpdateAnims();
											UpdateAtlasses();
										}

										mtex.selected = -1;
										mtex.sel_slot = -1;
										memset(mtex.selection, 0, 512 * sizeof(int));

										mtex.command = NONE;
									}

									names[j] = mtex.mgg.fn[i];
									nm[j] = counter;

									if (j < 8)
										j++;

									continue;
								}
							}
							else
							{
								bounds = nk_widget_bounds(ctx);
								bounds.h += 20;

								if (mult_sel_count > 0)
								{
									nk_button_symbol(ctx, NK_SYMBOL_PLUS);

									names[j] = mtex.mgg.fn[i];

									if (j < 8)
										j++;

									mult_sel_count--;

									continue;
								}

								temp = 0;
								if (i + mtex.last_sel - mtex.first_sel + 1 > mtex.mgg.num_frames)
									temp = 1;

								if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds) && !temp)
								{
									nk_button_symbol(ctx, NK_SYMBOL_PLUS);

									if (nk_input_has_mouse_click(ctx, NK_BUTTON_LEFT))
									{
										//l = mtex.mgg.fn[i];
										//mtex.mgg.fn[i] = mtex.mgg.fn[mtex.selected];
										//mtex.mgg.fn[mtex.selected] = l;
										if (mtex.switch_place)
										{
											for (k = mtex.first_sel, m = 0; k < mtex.last_sel + 1; k++, m++)
											{
												l = mtex.mgg.fn[i + m];
												mtex.mgg.fn[i + m] = mtex.mgg.fn[k];
												mtex.mgg.fn[k] = l;
											}

											for (k = mtex.first_sel, m = 0; k < mtex.last_sel + 1; k++, m++)
											{
												l = mtex.mgg.fnn[i + m];
												mtex.mgg.fnn[i + m] = mtex.mgg.fnn[k];
												mtex.mgg.fnn[k] = l;
											}
										}
										else
										{
											if (i < mtex.selected)
											{
												for (n = 0; n < mtex.last_sel - mtex.first_sel + 1; n++)
												{
													l = mtex.mgg.fn[mtex.first_sel + n];
													m = mtex.mgg.fnn[mtex.first_sel + n];

													for (k = mtex.first_sel + n - 1; k > i + n - 1; k--)
														mtex.mgg.fn[k + 1] = mtex.mgg.fn[k];

													for (k = mtex.first_sel + n - 1; k > i + n - 1; k--)
														mtex.mgg.fnn[k + 1] = mtex.mgg.fnn[k];

													mtex.mgg.fn[i + n] = l;
													mtex.mgg.fnn[i + n] = m;
												}
											}
											else
											{
												for (n = mtex.last_sel - mtex.first_sel + 1; n > 0; n--)
												{
													l = mtex.mgg.fn[mtex.first_sel + n - 1];
													m = mtex.mgg.fnn[mtex.first_sel + n - 1];

													for (k = mtex.first_sel + n - 1; k < i + n -1; k++)
														mtex.mgg.fn[k] = mtex.mgg.fn[k + 1];

													for (k = mtex.first_sel + n - 1; k < i + n - 1; k++)
														mtex.mgg.fnn[k] = mtex.mgg.fnn[k + 1];

													mtex.mgg.fn[i + n - 1] = l;
													mtex.mgg.fnn[i + n - 1] = m;
												}
											}

											UpdateAnims();
											UpdateAtlasses();
										}

										mtex.selected = -1;
										mtex.sel_slot = -1;
										memset(mtex.selection, 0, 512 * sizeof(int));

										mtex.command = NONE;
									}
									else
										mult_sel_count = mtex.last_sel - mtex.first_sel;

									names[j] = mtex.mgg.fn[i];
									nm[j] = counter;

									if (j < 8)
										j++;

									continue;
								}
							}
						}

						struct nk_rect coords = nk_widget_bounds(ctx);

						if (nk_selectable_image_label(ctx, nk_image_id(mtex.textures[mtex.mgg.fn[i]]), " ", NK_TEXT_ALIGN_MIDDLE, &mtex.selection[i]))
						{
							if (st.keys[LSHIFT_KEY].state)
							{
								mtex.anim_selected = -1;
								memset(mtex.selection, 0, 512 * sizeof(int));
								//memset(mtex.sel_slots, 0, 512 * sizeof(int));

								mtex.mult_selection = abs(i - mtex.selected);

								if (mtex.selected < i)
								{
									mtex.first_sel = mtex.selected;
									mtex.last_sel = i;

									mtex.first_sel_slot = mtex.sel_slot;
									mtex.last_sel_slot = i;

									for (k = mtex.selected; k < i + 1; k++)
									{
										mtex.selection[k] = 1;
										mtex.sel_slots[k] = 1;
									}
								}
								else
								{
									mtex.first_sel = i;
									mtex.last_sel = mtex.selected;

									mtex.first_sel_slot = i;
									mtex.last_sel_slot = mtex.sel_slot;

									for (k = i; k < mtex.selected + 1; k++)
									{
										mtex.selection[k] = 1;
										mtex.sel_slots[k] = 1;
									}
								}
							}
							else
							{
								mtex.anim_selected = -1;
								mtex.mult_selection = 0;
								memset(mtex.selection, 0, 512 * sizeof(int));
								memset(mtex.sel_slots, 0, 512 * sizeof(int));
								mtex.selection[i] = 1;
								mtex.selected = i;
								mtex.sel_slot = i;
							}

						}

						if (mtex.intg_app[INTG_PS].valid == 1 || mtex.intg_app[INTG_GP].valid == 1 || mtex.intg_app[INTG_GP].valid == 1)
						{
							if (nk_contextual_begin(ctx, NULL, nk_vec2(300, 300), coords))
							{
								nk_layout_row_dynamic(ctx, 20, 1);
								for (int o = 0; o < 3; o++)
								{
									if (mtex.intg_app[o].valid == 1)
									{
										if (nk_button_label(ctx, StringFormat("Open with %s", mtex.intg_app[o].name)))
										{
											OpenWithIntApp(mtex.mgg.files[mtex.mgg.fn[i]], o);
											nk_contextual_close(ctx);
											break;
										}
									}
								}

								nk_contextual_end(ctx);
							}
						}

						names[j] = mtex.mgg.fn[i];
						nm[j] = counter;

						if (j < 8)
							j++;
					}
					else
						continue;
				}
				else
					counter--;
			}

			if (j > 0)
			{
				nk_layout_row_dynamic(ctx, 20, 8);

				for (k = 0; k < j; k++)
				{
					sprintf(str, "%d - %s", nm[k], mtex.mgg.texnames[names[k]]);
					nk_label(ctx, str, NK_TEXT_ALIGN_CENTERED);
				}

				j = 0;

			}
			//}
			/*
			else
			{
			nk_layout_row_dynamic(ctx, (0.70f * st.screenx) / 8.0f, 8);
			for (i = mtex.mgg.num_f0[option], j = 0; i < mtex.mgg.num_ff[option] + 1; i++)
			{
			if (mtex.mgg.frames_atlas[i] == option)
			{
			if (mtex.mgg.fn[i] < 1024)
			{
			if (j == 8)
			{
			nk_layout_row_dynamic(ctx, 20, 8);

			for (k = 0; k < 8; k++)
			{
			sprintf(str, "%d - %s", names[k], mtex.mgg.texnames[names[k]]);
			nk_label(ctx, str, NK_TEXT_ALIGN_CENTERED);
			}

			j = 0;

			nk_layout_row_dynamic(ctx, (0.70f * st.screenx) / 8.0f, 8);
			}

			if (nk_selectable_image_label(ctx, nk_image_id(mtex.textures[i]), " ", NK_TEXT_ALIGN_MIDDLE, &mtex.selection[i]))
			{
			if (st.keys[LSHIFT_KEY].state)
			{
			mtex.anim_selected = -1;
			memset(mtex.selection, 0, 512 * sizeof(int));

			mtex.mult_selection = abs(i - mtex.selected);

			if (mtex.selected < i)
			{
			mtex.first_sel = mtex.selected;
			mtex.last_sel = i;

			for (k = mtex.selected; k < i + 1; k++)
			mtex.selection[k] = 1;
			}
			else
			{
			mtex.first_sel = i;
			mtex.last_sel = mtex.selected;

			for (k = i; k < mtex.selected + 1; k++)
			mtex.selection[k] = 1;
			}
			}
			else
			{
			mtex.anim_selected = -1;
			mtex.mult_selection = 0;
			memset(mtex.selection, 0, 512 * sizeof(int));
			mtex.selection[i] = 1;
			mtex.selected = i;
			}

			}

			names[j] = i;

			if (j < 8)
			j++;
			}
			}
			else
			continue;
			}

			if (j > 0)
			{
			nk_layout_row_dynamic(ctx, 20, 8);

			for (k = 0; k < j; k++)
			{
			sprintf(str, "%d - %s", names[k], mtex.mgg.texnames[names[k]]);
			nk_label(ctx, str, NK_TEXT_ALIGN_CENTERED);
			}

			j = 0;

			}
			}
			*/
		}
		else
		{
			nk_layout_row_dynamic(ctx, (0.70f * st.screenx) / 8.0f, 8);
			for (i = 0, j = 0; i < mtex.mgg.num_frames; i++, counter++)
			{
				//if (mtex.mgg.fnn[i] < 1024)
				//{
				//if ((option == mtex.mgg.num_c_atlas && mtex.mgg.frames_atlas[i] == -1) || (option == mtex.mgg.num_c_atlas + 1)
				//|| (option < mtex.mgg.num_c_atlas && mtex.mgg.frames_atlas[i] == option))
				//{
				if (j == 8)
				{
					nk_layout_row_dynamic(ctx, 20, 8);

					for (k = 0; k < 8; k++)
					{
						sprintf(str, "%d - %s", nm[k], mtex.mgg.texnames_n[names[k]]);
						nk_label(ctx, str, NK_TEXT_ALIGN_CENTERED);
					}

					j = 0;

					nk_layout_row_dynamic(ctx, (0.70f * st.screenx) / 8.0f, 8);
				}

				if (mtex.command == MOV_TEX)
				{
					if (mtex.mult_selection == 0)
					{
						bounds = nk_widget_bounds(ctx);
						bounds.h += 20;
						if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds) && (mtex.size[i].x == mtex.size[mtex.selected].x && mtex.size[i].y == mtex.size[mtex.selected].y))
						{
							nk_button_symbol(ctx, NK_SYMBOL_PLUS);

							if (nk_input_has_mouse_click(ctx, NK_BUTTON_LEFT))
							{
								l = mtex.mgg.fnn[i];
								mtex.mgg.fnn[i] = mtex.mgg.fnn[mtex.selected];
								mtex.mgg.fnn[mtex.selected] = l;

								mtex.selected = -1;
								mtex.sel_slot = -1;
								memset(mtex.selection, 0, 512 * sizeof(int));

								mtex.command = NONE;
							}

							names[j] = mtex.mgg.fn[i];
							nm[j] = counter;

							if (j < 8)
								j++;

							continue;
						}
					}
					else
					{
						bounds = nk_widget_bounds(ctx);
						bounds.h += 20;

						if (mult_sel_count > 0)
						{
							nk_button_symbol(ctx, NK_SYMBOL_PLUS);

							names[j] = mtex.mgg.fnn[i];

							if (j < 8)
								j++;

							mult_sel_count--;

							continue;
						}

						temp = 1;

						for (k = mtex.first_sel, l = 0; k < mtex.last_sel + 1; k++, l++)
						{
							if (mtex.size[i + l].x != mtex.size[k].x || mtex.size[i + l].y != mtex.size[k].y)
							{
								temp = 0;
								break;
							}
						}

						if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds) && temp)
						{
							nk_button_symbol(ctx, NK_SYMBOL_PLUS);

							if (nk_input_has_mouse_click(ctx, NK_BUTTON_LEFT))
							{
								//l = mtex.mgg.fn[i];
								//mtex.mgg.fn[i] = mtex.mgg.fn[mtex.selected];
								//mtex.mgg.fn[mtex.selected] = l;

								for (k = mtex.first_sel, m = 0; k < mtex.last_sel + 1; k++, m++)
								{
									l = mtex.mgg.fnn[i + m];
									mtex.mgg.fnn[i + m] = mtex.mgg.fnn[k];
									mtex.mgg.fnn[k] = l;
								}

								mtex.selected = -1;
								mtex.sel_slot = -1;
								memset(mtex.selection, 0, 512 * sizeof(int));

								mtex.command = NONE;
							}
							else
								mult_sel_count = mtex.last_sel - mtex.first_sel;

							names[j] = mtex.mgg.fnn[i];
							nm[j] = counter;

							if (j < 8)
								j++;

							continue;
						}
					}
				}

				struct nk_rect coords;

				if (mtex.mgg.fnn[i] < 1024 && mtex.mgg.fnn[i] != -1)
				{
					coords = nk_widget_bounds(ctx);
					texu = nk_selectable_image_label(ctx, nk_image_id(mtex.textures_n[mtex.mgg.fnn[i]]), " ", NK_TEXT_ALIGN_MIDDLE, &mtex.selection[i]);
				}
				else
					texu = nk_selectable_label(ctx, "No normal map", NK_TEXT_ALIGN_LEFT, &mtex.selection[i]);

				if (texu)
				{
					if (st.keys[LSHIFT_KEY].state)
					{
						mtex.anim_selected = -1;
						memset(mtex.selection, 0, 512 * sizeof(int));
						//memset(mtex.sel_slots, 0, 512 * sizeof(int));

						mtex.mult_selection = abs(i - mtex.selected);

						if (mtex.selected < i)
						{
							mtex.first_sel = mtex.selected;
							mtex.last_sel = i;

							mtex.first_sel_slot = mtex.sel_slot;
							mtex.last_sel_slot = i;

							for (k = mtex.selected; k < i + 1; k++)
							{
								mtex.selection[k] = 1;
								mtex.sel_slots[k] = 1;
							}
						}
						else
						{
							mtex.first_sel = i;
							mtex.last_sel = mtex.selected;

							mtex.first_sel_slot = i;
							mtex.last_sel_slot = mtex.sel_slot;

							for (k = i; k < mtex.selected + 1; k++)
							{
								mtex.selection[k] = 1;
								mtex.sel_slots[k] = 1;
							}
						}
					}
					else
					{
						mtex.anim_selected = -1;
						mtex.mult_selection = 0;
						memset(mtex.selection, 0, 512 * sizeof(int));
						memset(mtex.sel_slots, 0, 512 * sizeof(int));
						mtex.selection[i] = 1;
						mtex.selected = i;
						mtex.sel_slot = i;
					}

					texu = 0;

				}

				if (mtex.mgg.fnn[i] < 1024 && mtex.mgg.fnn[i] != -1)
				{
					if (mtex.intg_app[INTG_PS].valid == 1 || mtex.intg_app[INTG_GP].valid == 1 || mtex.intg_app[INTG_GP].valid == 1)
					{
						if (nk_contextual_begin(ctx, NULL, nk_vec2(300, 300), coords))
						{
							nk_layout_row_dynamic(ctx, 20, 1);
							for (int o = 0; o < 3; o++)
							{
								if (mtex.intg_app[o].valid == 1)
								{
									if (nk_button_label(ctx, StringFormat("Open with %s", mtex.intg_app[o].name)))
									{
										OpenWithIntApp(mtex.mgg.files_n[mtex.mgg.fnn[i]], o);
										nk_contextual_close(ctx);
										break;
									}
								}
							}

							nk_contextual_end(ctx);
						}
					}
				}

				names[j] = mtex.mgg.fnn[i];
				nm[j] = counter;

				if (j < 8)
					j++;
				//}
				//else
				//continue;
				//}
				//else
				//counter--;
			}

			if (j > 0)
			{
				nk_layout_row_dynamic(ctx, 20, 8);

				for (k = 0; k < j; k++)
				{
					sprintf(str, "%d - %s", nm[k], mtex.mgg.texnames_n[names[k]]);
					nk_label(ctx, str, NK_TEXT_ALIGN_CENTERED);
				}

				j = 0;

			}
		}
	}

	mtex.canvas = option;

	nk_end(ctx);

	SetThemeBack();
}

void AnimBox()
{
	register int i, j, k, l;
	int an[2];
	static int inrect = 0;
	struct nk_rect bounds;

	if (nk_begin(ctx, "Animation Box", nk_rect(0.90f * st.screenx, 30, 0.10f * st.screenx, st.screeny - 30), NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		nk_layout_row_dynamic(ctx, 0.07f * st.screenx, 1);
		if (nk_button_label(ctx, "None"))
		{
			mtex.anim_selected = -1;
			mtex.anim_slot = -1;
			memset(mtex.selection, 0, 512 * sizeof(int));
		}

		if (mtex.mgg.num_anims > 0)
		{
			nk_layout_row_dynamic(ctx, 0.035f * st.screenx, 2);
			for (i = 0, k = 0; i < mtex.mgg.num_anims; i++)
			{
				if (mtex.mgg.an[i] < 1024)
				{
					if (k == 2)
					{
						nk_layout_row_dynamic(ctx, 15, 2);
						for (l = 0; l < 2; l++)
							nk_label(ctx, mtex.mgg.mga[an[l]].name, NK_TEXT_ALIGN_CENTERED);

						k = 0;

						nk_layout_row_dynamic(ctx, 0.035f * st.screenx, 2);
					}

					if (mtex.command == MOV_ANIM)
					{
						bounds = nk_widget_bounds(ctx);
						bounds.h += 20;
						if (nk_input_is_mouse_hovering_rect(&ctx->input, bounds))
						{
							nk_button_symbol(ctx, NK_SYMBOL_PLUS);

							if (nk_input_has_mouse_click(ctx, NK_BUTTON_LEFT))
							{
								l = mtex.mgg.an[i];
								mtex.mgg.an[i] = mtex.mgg.an[mtex.anim_slot];
								mtex.mgg.an[mtex.anim_slot] = l;

								mtex.anim_selected = mtex.mgg.an[i];
								mtex.anim_slot = i;

								mtex.command = NONE;
							}

							an[k] = mtex.mgg.an[i];

							if (k < 2)
								k++;

							continue;
						}
					}

					if (mtex.anim_selected == mtex.mgg.an[i])
						ctx->style.button.normal = ctx->style.button.hover;

					if (nk_button_image(ctx, nk_image_id(mtex.textures[mtex.mgg.mga[mtex.mgg.an[i]].startID])))
					{
						mtex.selected = mtex.mgg.mga[mtex.mgg.an[i]].startID;
						mtex.anim_selected = mtex.mgg.an[i];
						mtex.anim_slot = i;
						memset(mtex.selection, 0, 512 * sizeof(int));
						mtex.mult_selection = 0;
						for (j = mtex.mgg.mga[mtex.mgg.an[i]].startID; j < mtex.mgg.mga[mtex.mgg.an[i]].endID + 1; j++)
							mtex.selection[j] = 1;

						mtex.first_sel = mtex.mgg.mga[mtex.mgg.an[i]].startID;
						mtex.last_sel = mtex.mgg.mga[mtex.mgg.an[i]].endID;
					}

					an[k] = mtex.mgg.an[i];

					if (k < 2)
						k++;

					SetThemeBack();
				}
			}

			if (k > 0)
			{
				nk_layout_row_dynamic(ctx, 15, 2);
				for (l = 0; l < k; l++)
					nk_label(ctx, mtex.mgg.mga[an[l]].name, NK_TEXT_ALIGN_CENTERED);

				k = 0;
			}
		}


	}

	nk_end(ctx);
}

int main(int argc, char *argv[])
{
	char str[64];
	int loops, i;

	struct nk_color background;

	PreInit("mtex",argc,argv);

	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);

	memset(&mtex, 0, sizeof(mTex));

	if(LoadCFG()==0)
		if(MessageBox(NULL,"Error while trying to read or write the configuration file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();

	LoadIntApps();
	
	strcpy(st.LogName, "mtex.log");

	Init();

	DisplaySplashScreen();

	strcpy(st.WindowTitle,"Tex");

	//OpenFont("font/Roboto-Regular.ttf","arial",0,128);
	//OpenFont("font/Roboto-Bold.ttf","arial bold",1,128);
	//OpenFont("font//tt0524m_.ttf","geometry",2,128);

	InitMGG();
	/*
	if(LoadMGG(&mgg_sys[0],"data/mEngUI.mgg")==NULL)
	{
		MessageBox(NULL, "Could not open UI MGG", "Error", MB_OK);
		LogApp("Could not open UI mgg");
		Quit();
	}
	
	UILoadSystem("UI_Sys.cfg");
	*/

	st.FPSYes=1;

	st.Developer_Mode=0;

	curr_tic=GetTicks();

	ctx = nk_sdl_init(wn);

	struct nk_font_atlas *atlas;
	struct nk_font_config cfg = nk_font_config(0);
	cfg.oversample_h = 3;
	cfg.oversample_v = 2;

	nk_sdl_font_stash_begin(&atlas);
	fonts[0] = nk_font_atlas_add_from_file(atlas, "Font\\Roboto-Regular.ttf", 16, 0);
	fonts[1] = nk_font_atlas_add_from_file(atlas, "Font\\mUI.ttf", 18, 0);
	nk_sdl_font_stash_end();
	nk_style_set_font(ctx, &fonts[0]->handle);
	background = nk_rgb(28, 48, 62);

	SETENGINEPATH;

	mtex.selected = -1;
	mtex.anim_selected = -1;

	InitGWEN();
	SetSkin(ctx, mtex.theme);

	if (argc > 0)
	{
		for (i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], "-o") == NULL)
			{
				char *buf = LoadPrjFile(argv[i + 1]);

				if (buf)
					MessageBox(NULL, buf, "Error", MB_OK);
				else
				{
					strcpy(mtex.prj_path, argv[i+1]);

					int a = strlen(argv[i + 1]);

					int j;

					for (j = a; j > 0; j--)
					{
						if (argv[i + 1][j] == '\\' || argv[i + 1][j] == '/') break;
					}

					j++;

					strcpy(mtex.filename, argv[i + 1] + j);

					sprintf(st.WindowTitle, "Tex %s", mtex.filename);

					memcpy(&mtex.mgg2, &mtex.mgg, sizeof(mtex.mgg));

					for (int m = 0; m < j - 1; m++)
						mtex.mgg.path[m] = argv[i + 1][m];
				}
			}
		}
	}

	InitEngineWindow();

BACKLOOP:

	while(!st.quit)
	{
		if(st.FPSYes)
			FPSCounter();

		nk_input_begin(ctx);
		
		int8 AC;
		char *ACdata = CheckAppComm(&AC);

		if (ACdata != NULL && AC >= 0)
		{
			char *buf;

			if (AC == IA_OPENFILE)
			{
				SDL_RestoreWindow(wn);
				SDL_ShowWindow(wn);
				SDL_RaiseWindow(wn);

				switch (MessageBoxRes("Warning", MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST, "Would you like to save this project before opening another file?"))
				{
				case IDYES:
					SavePrjFile(mtex.prj_path);
					UnloadmTexMGG();
					buf = LoadPrjFile(ACdata);

					if (buf)
						MessageBox(NULL, buf, "Error", MB_OK);
					else
					{
						strcpy(mtex.prj_path, ACdata);

						int a = strlen(ACdata);

						int j;

						for (j = a; j > 0; j--)
						{
							if (ACdata[j] == '\\' || ACdata[j] == '/') break;
						}

						j++;

						strcpy(mtex.filename, ACdata + j);

						sprintf(st.WindowTitle, "Tex %s", mtex.filename);

						memcpy(&mtex.mgg2, &mtex.mgg, sizeof(mtex.mgg));

						for (int m = 0; m < j - 1; m++)
							mtex.mgg.path[m] = ACdata[m];
					}

					break;

				case IDNO:
					UnloadmTexMGG();
					buf = LoadPrjFile(ACdata);

					if (buf)
						MessageBox(NULL, buf, "Error", MB_OK);
					else
					{
						strcpy(mtex.prj_path, ACdata);

						int a = strlen(ACdata);

						int j;

						for (j = a; j > 0; j--)
						{
							if (ACdata[j] == '\\' || ACdata[j] == '/') break;
						}

						j++;

						strcpy(mtex.filename, ACdata + j);

						sprintf(st.WindowTitle, "Tex %s", mtex.filename);

						memcpy(&mtex.mgg2, &mtex.mgg, sizeof(mtex.mgg));

						for (int m = 0; m < j - 1; m++)
							mtex.mgg.path[m] = ACdata[m];
					}

					break;
				}
			}

			ResetAppComm();
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

			if (mtex.anim_selected != -1 && mtex.play)
			{
				if (mtex.mgg.mga[mtex.anim_selected].speed > 0)
				{
					if (mtex.mgg.mga[mtex.anim_selected].current_frame < mtex.mgg.mga[mtex.anim_selected].startID * 100)
						mtex.mgg.mga[mtex.anim_selected].current_frame = mtex.mgg.mga[mtex.anim_selected].startID * 100;
				}
				else
				{
					if (mtex.mgg.mga[mtex.anim_selected].current_frame > mtex.mgg.mga[mtex.anim_selected].startID * 100)
						mtex.mgg.mga[mtex.anim_selected].current_frame = mtex.mgg.mga[mtex.anim_selected].startID * 100;
				}

				mtex.mgg.mga[mtex.anim_selected].current_frame += mtex.mgg.mga[mtex.anim_selected].speed;

				if (mtex.mgg.mga[mtex.anim_selected].speed > 0)
				{
					if (mtex.mgg.mga[mtex.anim_selected].current_frame >= mtex.mgg.mga[mtex.anim_selected].endID * 100)
						mtex.mgg.mga[mtex.anim_selected].current_frame = mtex.mgg.mga[mtex.anim_selected].startID * 100;
				}
				else
				{
					if (mtex.mgg.mga[mtex.anim_selected].current_frame <= mtex.mgg.mga[mtex.anim_selected].endID * 100)
						mtex.mgg.mga[mtex.anim_selected].current_frame = mtex.mgg.mga[mtex.anim_selected].startID * 100;
				}

				mtex.anim_frame = mtex.mgg.mga[mtex.anim_selected].current_frame / 100;
			}

			curr_tic+=1000/TICSPERSECOND;
			loops++;
			SetTimerM(1);
		}

		if (curr_tic % 30 == 0) 
			UpdateFiles();

		DrawSys();

		if (mtex.mgg.num_frames > 0)
		{
			LeftPannel();
			ViewerBox();
			Canvas();
			AnimBox();
		}

		MenuBar();

		UIMain_DrawSystem();
		//MainSound();
		Renderer(0);

		float bg[4];
		nk_color_fv(bg, background);

		nk_sdl_render(NK_ANTI_ALIASING_OFF, 512 * 1024, 128 * 1024);

		SwapBuffer(wn);

		nkrendered = 0;

		CheckForChanges();
	}

	if (mtex.mgg.num_frames > 0)
	{
		switch (MessageBoxRes("Warning", MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST, "Would you like to save this project before quitting?"))
		{
			case IDYES:
				SavePrjFile(mtex.prj_path);
				break;

			case IDCANCEL:
				st.quit = 0;
				goto BACKLOOP;
		}
	}

	UnloadmTexMGG();

	Quit();
	return 1;
}