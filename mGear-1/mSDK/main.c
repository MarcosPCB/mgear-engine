#include <Windows.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <commdlg.h>
#include "input.h"
#include "main.h"
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
#include "dirent.h"
#include "UI.h"
#include <curl.h>
#include <tomcrypt.h>
#include "funcs.h"
#include <time.h>

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
int64 FileSize, TransBytes;

mSdk msdk;

prng_state prng;
hash_state hash;

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

struct File_sys *GetFolderTreeContent(const char path[MAX_PATH], int16 *num_files)
{
	struct File_sys *files;
	FILE *f;
	DIR *d, *d2;
	dirent *dir;

	register uint32 i, j, k, l;
	uint8 state = 0;

	int16 filenum;

	char content[2048][32], curr_path[MAX_PATH], tmp[MAX_PATH], str[512], *buffer;

	i = 0;
	strcpy(curr_path, path);
	while (state == 0)
	{
		if (i > 1)
		{
			for (k = 0, l = 0; k < i; k++)
			{
				if (files[k].type == 2)
				{
					strcpy(curr_path, files[k].path);
					strcat(curr_path, "/");
					strcat(curr_path, files[k].file);

					filenum = NumDirFile(curr_path, content);

					for (j = 0; j < filenum; j++)
					{
						if (strcmp(content[j], ".") == NULL || strcmp(content[j], "..") == NULL || strcmp(content[j], "desktop.ini") == NULL
							|| strcmp(content[j], "thumbs.db") == NULL || strcmp(content[j], "Thumbs.db") == NULL) continue;
						else
						{
							strcpy(tmp, curr_path);
							strcat(tmp, "/");
							strcat(tmp, content[j]);
							
							if ((f = fopen(tmp, "r")) == NULL)
							{
								//strcpy(tmp, curr_path);
								//strcat(tmp, "/");
								//strcat(tmp, content[j]);
								if ((d2 = opendir(tmp)) == NULL)
								{
									LogApp("Invalid file or folder %s", tmp);
								}
								else
								{
									i++;
									files = realloc(files, sizeof(struct File_sys) * (i + 1));

									strcpy(files[i].path, curr_path);
									strcpy(files[i].file, content[j]);
									strcpy(files[i].parent, ".");
									strcat(files[i].parent, curr_path + strlen(path));
									files[i].type = 2;
									LogApp("%s/%s", files[i].path, files[i].file);
									closedir(d2);
								}
							}
							else
							{
								i++;
								files = realloc(files, sizeof(struct File_sys) * (i + 1));

								strcpy(files[i].path, curr_path);
								strcpy(files[i].file, content[j]);
								strcpy(files[i].parent, ".");
								strcat(files[i].parent, curr_path + strlen(path));
								files[i].type = 0;
								LogApp("%s/%s", files[i].path, files[i].file);
								
								fseek(f, SEEK_END, 0);
								files[i].size = ftell(f);
								
								rewind(f);
							
								alloc_mem(buffer, files[i].size);
							
								fread(buffer, files[i].size, 1, f);
							
								sha256_init(&hash);
								sha256_process(&hash, buffer, files[i].size);
								sha256_done(&hash, files[i].hash);
								
								free(buffer);
								fclose(f);
								l++;
							}
						}
					}

					files[k].type = 1;
					files[k].filenum = l;
				}
			}

			state = 1;
			break;
		}
		
		

		filenum = NumDirFile(curr_path, content);


		for (j = 0; j < filenum; j++)
		{
			if (strcmp(content[j], ".") == NULL || strcmp(content[j], "..") == NULL || strcmp(content[j], "desktop.ini") == NULL
				|| strcmp(content[j], "thumbs.db") == NULL || strcmp(content[j], "Thumbs.db") == NULL) continue;
			else
			{
				if ((f = fopen(content[j], "r")) == NULL)
				{
					strcpy(tmp, curr_path);
					strcat(tmp, "/");
					strcat(tmp, content[j]);
					if ((d2 = opendir(tmp)) == NULL)
					{
						LogApp("Invalid file or folder %s", tmp);
					}
					else
					{
						i++;
						if (i == 1)
							files = malloc(sizeof(struct File_sys) * 2);
						else
							files = realloc(files, sizeof(struct File_sys) * (i + 1));

						strcpy(files[i].path, curr_path);
						strcpy(files[i].file, content[j]);
						strcpy(files[i].parent, ".");
						files[i].type = 2;
						LogApp("%s/%s", files[i].path, files[i].file);
						closedir(d2);
					}
				}
				else
				{
					i++;
					if (i == 1)
						files = malloc(sizeof(struct File_sys) * 2);
					else
						files = realloc(files, sizeof(struct File_sys) * (i + 1));

					strcpy(files[i].path, curr_path);
					strcpy(files[i].file, content[j]);
					strcpy(files[i].parent, ".");
					files[i].type = 0;
					LogApp("%s/%s", files[i].path, files[i].file);
					
					fseek(f, SEEK_END, 0);
					files[i].size = ftell(f);
					
					rewind(f);
				
					alloc_mem(buffer, files[i].size);
				
					fread(buffer, files[i].size, 1, f);
				
					sha256_init(&hash);
					sha256_process(&hash, buffer, files[i].size);
					sha256_done(&hash, files[i].hash);
					
					free(buffer);
					
					fclose(f);
				}
			}
		}
	}

	*num_files = i;

	return files;
}

size_t CURLWriteData(void *ptr, size_t size, size_t new_mem, FILE *stream)
{
	size_t written;
	return fwrite(ptr, size, new_mem, stream);
}

int CURLProgress(void *bar, double t, /* dltotal */ double d, /* dlnow */ double ultotal, double ulnow)
{
	double pr;
	msdk.download_now = d;
	msdk.download_total = t;

	pr = (d / t) * 100.00f;

	printf("\r%0.2f", pr);

	return 0;
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
	fprintf(file, "Theme = %d\n", msdk.theme);

	fclose(file);

	return 1;
}

uint16 SaveCFG()
{
	FILE *file;

	if ((file = fopen("msdk_settings.cfg", "w")) == NULL)
		return 0;

	fprintf(file, "ScreenX = %d\n", st.screenx);
	fprintf(file, "ScreenY = %d\n", st.screeny);
	fprintf(file, "FullScreen = %d\n", st.fullscreen);
	fprintf(file, "ScreenBPP = %d\n", st.bpp);
	fprintf(file, "AudioFrequency = %d\n", st.audiof);
	fprintf(file, "AudioChannels = %d\n", st.audioc);
	fprintf(file, "VSync = %d\n", st.vsync);
	fprintf(file, "Theme = %d\n", msdk.theme);

	fclose(file);

	return 1;
}

uint16 LoadCFG()
{
	FILE *file;
	char buf[2048], str[128], str2[2048], *buf2, buf3[2048];
	int value=0;
	if((file=fopen("msdk_settings.cfg","r"))==NULL)
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
		if (strcmp(str, "Theme") == NULL) msdk.theme = value;
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
	switch (msdk.theme)
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

int SavePrjFile(const char *filename)
{
	FILE *f;
	char header[21] = { "mGear Project V1.0" };


	if ((f = fopen(filename, "wb")) == NULL)
		return 0;

	fwrite(header, 21, 1, f);
	fwrite(msdk.prj.name, 32, 1, f);
	fwrite(&msdk.prj.audio, 1, 1, f);
	fwrite(&msdk.prj.code, 1, 1, f);
	fwrite(&msdk.prj.code_type, 1, 1, f);
	fwrite(&msdk.prj.map, 1, 1, f);
	fwrite(&msdk.prj.sprites, 1, 1, f);
	fwrite(&msdk.prj.ui, 1, 1, f);
	fwrite(&msdk.prj.tex, 1, 1, f);
	fwrite(&msdk.prj.curr_rev, sizeof(int16), 1, f);
	fwrite(&msdk.prj.revisions, sizeof(int16), 1, f);
	fwrite(msdk.prj.user_name, 16, 1, f);
	fwrite(&msdk.prj.num_users, sizeof(int8), 1, f);
	fwrite(msdk.prj.users, 16, 8, f);

	if (msdk.prj.curr_rev > 0)
		fwrite(msdk.prj.exp_path, MAX_PATH, 1, f);

	fclose(f);

	return 1;
}

int SaveRevFile(const char *filename, char *log, size_t len, int16 num_files, _Files *files)
{
	FILE *f;

	time_t t;

	struct tm *tmv;

	uint32 timet;

	char header[13] = { "REV_FILE_SDK" };

	int16 rev = msdk.prj.curr_rev + 1;

	openfile_cmd(f, filename, "wb", MessageBoxRes("Error", MB_OK, "Could not save revision file %d", msdk.prj.curr_rev + 1); return NULL;);

	fwrite(header, 13, 1, f);

	fwrite(&rev, 2, 1, f);

	t = time(NULL);

	tmv = localtime(&t);

	timet = tmv->tm_year << 20;
	timet |= tmv->tm_mon << 16;
	timet |= tmv->tm_mday << 11;
	timet |= tmv->tm_hour << 5;
	timet |= tmv->tm_min;

	fwrite(&timet, 4, 1, f);

	fwrite(&len, 2, 1, f);
	fwrite(log, len, 1, f);
	fwrite(&num_files, 2, 1, f);
	fwrite(files, sizeof(_Files)* num_files, 1, f);

	fclose(f);

	return 1;
}

int LoadPrjFile(const char *filename)
{
	FILE *f;
	char header[21];
	
	if ((f = fopen(filename, "rb")) == NULL)
		return 0;

	fread(header, 21, 1, f);

	if (strcmp(header, "mGear Project V1.0") != NULL)
	{
		fclose(f);
		return -1;
	}

	fread(msdk.prj.name, 32, 1, f);
	fread(&msdk.prj.audio, 1, 1, f);
	fread(&msdk.prj.code, 1, 1, f);
	fread(&msdk.prj.code_type, 1, 1, f);
	fread(&msdk.prj.map, 1, 1, f);
	fread(&msdk.prj.sprites, 1, 1, f);
	fread(&msdk.prj.ui, 1, 1, f);
	fread(&msdk.prj.tex, 1, 1, f);
	fread(&msdk.prj.curr_rev, sizeof(int16), 1, f);
	fread(&msdk.prj.revisions, sizeof(int16), 1, f);
	fread(msdk.prj.user_name, 16, 1, f);
	fread(&msdk.prj.num_users, sizeof(int8), 1, f);
	fread(msdk.prj.users, 16, 8, f);

	if (msdk.prj.curr_rev > 0)
		fread(msdk.prj.exp_path, MAX_PATH, 1, f);

	fclose(f);

	//LoadTDL();
	//LoadBaseLog();

	//if (msdk.prj.curr_rev > 0)
	//	LoadRevLog();

	GetCurrentDirectory(MAX_PATH, msdk.prj.prj_path);
	strcpy(msdk.prj.prj_raw_path, msdk.prj.prj_path);
	strcat(msdk.prj.prj_raw_path, "\\_prj_raw");

	return 1;
}

int CommitPrj()
{
	static int state = 0, pct_inc, pct;
	static int16 num_files, num_files_exp, num_files_commited = 0;
	
	char fhash[512], log[1024];
	
	static struct File_sys *files;
	static _Files *exp_files;
	_Files *prj_files;

	char str[256], filepath[MAX_PATH], *buf;

	static char exp_state = 0, steps = 0, dots[3], pct_str[32], timed = 0;

	static int64 f_timer;

	int i = 0, j = 0;
	
	FILE *f;
	
	if(state == 0)
	{
		files = GetFolderTreeContent(msdk.prj.prj_path, &num_files);
		
		OPENFILE_D(f, StringFormat("%s/index",msdk.prj.exp_path),"rb");
		
		fread(&num_files_exp, sizeof(int16), 1, f);
		
		alloc_mem(exp_files, num_files_exp * sizeof(_Files));
		
		fread(exp_files, sizeof(_Files) * num_files_exp, 1, f);
		
		fclose(f);

		num_files_commited = 0;

		state = 1;
	}
	
	if (state == 1)
	{
		if (nk_begin(ctx, "Commit", nk_rect(st.screenx / 2 - 256, st.screeny / 2 - 256, 512, 512), NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE))
		{
			nk_layout_row_dynamic(ctx, 25, 1);
			nk_label(ctx, "Exported project path:", NK_TEXT_ALIGN_LEFT);

			nk_label_wrap(ctx, msdk.prj.exp_path);

			nk_layout_row_dynamic(ctx, 250, 1);

			if (nk_group_begin(ctx, "Files", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
			{
				nk_layout_row_dynamic(ctx, 15, 1);
				for (i = 0; i < num_files; i++)
				{
					if (files[i].type == 0)
					{
						for (j = 0; j < num_files_exp; j++)
						{
							if (strcmp(StringFormat("%s/%s", files[i].parent, files[i].file), exp_files[j].path) == NULL) break;
						}

						if (j == num_files_exp)
							j = -1;

						if (j != -1)
						{
							if (memcmp(files[i].hash, exp_files[j].hash, 512) != NULL)
								files[i].commit = nk_check_label(ctx, files[i].file, files[i].commit == 1);
						}
						else
							files[i].commit = nk_check_label(ctx, files[i].file, files[i].commit == 1);
					}
				}


				nk_group_end(ctx);
			}

			nk_layout_row_dynamic(ctx, 100, 1);
			nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, log, 1024, nk_filter_default);

			nk_layout_row_dynamic(ctx, 25, 4);
			nk_spacing(ctx, 2);
			if (nk_button_label(ctx, "Commit"))
				state = 2;

			if (nk_button_label(ctx, "Cancel"))
				state = 3;
		}

		nk_end(ctx);
	}

	if (state == 2)
	{
		if (nk_begin(ctx, "Exporting", nk_rect(st.screenx / 2 - 125, st.screeny / 2 - 48, 350, 96), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
		{
			nk_layout_row_dynamic(ctx, 25, 2);

			if (timed == 0)
			{
				f_timer = st.time;
				timed = 1;
			}

			if (timed == 1)
			{
				if (st.time - f_timer > TICSPERSECOND / 2)
				{

					if (strcmp(dots, "...") == NULL)
						strcpy(dots, ".");
					else
						strcat(dots, ".");

					timed = 0;
					f_timer = 0;
				}
			}

			switch (exp_state)
			{
			case 0:
				sprintf(pct_str, "Creating user system%s", dots);
				break;

			case 1:
				sprintf(pct_str, "Copying files%s", dots);
				break;

			case 2:
				sprintf(pct_str, "Encrypting%s", dots);
				break;

			case 3:
				sprintf(pct_str, "Pushing to storage%s", dots);
				break;
			}

			nk_label(ctx, pct_str, NK_TEXT_ALIGN_LEFT);

			if (exp_state == 0)
			{
				if (steps < num_files)
				{
					SetCurrentDirectory(msdk.prj.exp_path);
					SetCurrentDirectory("versions");
					SetCurrentDirectory(StringFormat("%d",msdk.prj.revisions));

					if (files[steps].type != 0 || files[steps].commit == 0) steps++;
					else
					{
						strcpy(filepath, files[steps].path);
						strcat(filepath, "/");
						strcat(filepath, files[steps].file);

						char extension = strrchr(files[steps].file, '.'), newfilepath[MAX_PATH];

						sprintf(newfilepath, "f%04dr%04d.%s", steps, extension);

						buf = GetRootDir(files[steps].parent);

						if (strcmp(buf, "_prj_raw") == NULL)
							SetCurrentDirectory("raw");
						else
							SetCurrentDirectory("prj");
				
						if (CopyFile(filepath, newfilepath, FALSE) == NULL)
						{
							sprintf(str, "Error %x when copying file: %s", GetLastError(), files[steps].file);
							MessageBox(NULL, str, "Error", MB_OK);
						}

						SetCurrentDirectory("..");

						steps++;
					}
				}
				else
				{
					exp_state = 1;

					SetCurrentDirectory(StringFormat("%s/users/%03d",msdk.prj.exp_path, msdk.prj.user_id));

					prj_files = malloc(sizeof(_Files)* num_files);

					for (i = 0, j = 0; i < num_files; i++)
					{
						if (files[i].commit == 1 && files[i].type == 0)
						{
							strcpy(prj_files[j].path, files[i].parent);
							prj_files[j].rev = msdk.prj.curr_rev;
							prj_files[i].f_rev = msdk.prj.revisions;
							prj_files[j].size = files[i].size;
							memcpy(prj_files[j].hash, files[i].hash, 512);
							j++;
						}
					}

					SaveRevFile(StringFormat("%04d.rif", msdk.prj.curr_rev + 1), log, strlen(log), j, prj_files);

					steps = 0;
				}
			}
			else
			if (exp_state == 1)
			{
				if (steps == 0)
				{
					if (MessageBoxRes("First Commit", MB_YESNO, "Would you like to push the commit now?") == IDYES)
					{

					}
					else
					{
						exp_state = 4;
						steps = 0;
					}
				}
			}
			else
			if (exp_state == 3)
			{
				MessageBoxRes("Export complete", MB_OK, "Exporting complete");
				state = 4;
			}

			if (exp_state == 0)
					pct = steps + 1;

			if (exp_state == 1)
				pct = steps + (num_files * 2) + 1;

			if (exp_state == 2)
				pct = 100 / pct_inc;

			if (exp_state == 3)
				pct = 100 / pct_inc;

			nk_label(ctx, StringFormat("%d%%", pct * pct_inc), NK_TEXT_ALIGN_RIGHT);
		}

		nk_end(ctx);
	}
	
	if(state == 3)
	{
		free(exp_files);
		free(files);
		return 1;
	}

	return NULL;
}

int ExportProject()
{
	static uint8 encrypted = 0, num_users = 1, usernames[8][16], passwords[8][16], state = 0, selected_str[12], exp_state = 0;
	static int storage = 0, admin = 0, timed = 0, steps = 0, can_exp = 0, user_ready = 0, select_all = 0, select_all_2 = 0, num_files_commited;
	static enum USER_TYPE user_perm[8];
	char str[128], str2[128], dir[MAX_PATH];
	int pct, pct_inc, err, salt_len, hash2_len, ck_len, master_key_len, tmp_len, tmp2_len;
	static int16 num_files;
	static uint64 f_timer, cur_timer, hexcode;
	static char dots[3] = { "." }, path[MAX_PATH], path_cur[MAX_PATH], hashed_password[MAXBLOCKSIZE], hash2[MAXBLOCKSIZE], mac[MAXBLOCKSIZE], salt[MAXBLOCKSIZE],
		master_key[MAXBLOCKSIZE], ck[MAXBLOCKSIZE], recover_keys[8][16], tmp[MAXBLOCKSIZE], tmp2[MAXBLOCKSIZE], user_salt[8][MAXBLOCKSIZE], filepath[MAX_PATH],
		newfilepath[MAX_PATH], *extension, *buf, pct_str[128];

	static struct File_sys *files;

	static FILE *fc;

	static symmetric_key key;

	FILE *f, *f2;
	DIR *d;
	dirent *di;

	register int16 i, j;

	BOOL cpf;

	struct SDKEXP exporter;

	_Files *prj_files;

	BROWSEINFO bi;

	static LPITEMIDLIST pidl;

	char directory[MAX_PATH];
	char exepath[MAX_PATH];
	char args[MAX_PATH * 3];
	static path2[MAX_PATH];
	MSG msg;

	DWORD error;

	static DWORD exitcode;

	static SHELLEXECUTEINFO info;
	
	static struct indexer *indexed_files;
	static struct index_f *index_array;

	ZeroMemory(&bi, sizeof(bi));

	if (state == 0)
	{
		encrypted = 0;
		num_users = msdk.prj.num_users;
		memcpy(usernames, msdk.prj.users, 8 * 16);
		ZeroMemory(passwords, 8 * 16);
		admin = 0;
		memset(user_perm, 0, sizeof(enum USERTYPE) * 8);
		user_perm[0] = ADMIN;
		storage = 0;
		state = 1;
		strcpy(dots, ".");
		steps = 1;
		can_exp = 0;
		ZeroMemory(path, MAX_PATH);
		ZeroMemory(path_cur, MAX_PATH);
		files = GetFolderTreeContent(msdk.prj.prj_path, &num_files);
		exp_state = steps = 0;

		fc = NULL;
	}

	if (state == 1)
	{
		if (nk_begin(ctx, "Export Project", nk_rect(st.screenx / 2 - 256, st.screeny / 2 - 375, 512, 750), NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE))
		{
			nk_layout_row_dynamic(ctx, 25, 1);
			nk_label(ctx, "Export path", NK_TEXT_ALIGN_LEFT);

			nk_layout_row_begin(ctx, NK_DYNAMIC, 25, 2);
			nk_layout_row_push(ctx, 0.80f);
			nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, path, MAX_PATH, nk_filter_ascii);

			nk_layout_row_push(ctx, 0.20f);
			if (nk_button_label(ctx, "Browse"))
			{
				bi.lpszTitle = ("Select a folder to export the project");
				bi.ulFlags = BIF_USENEWUI;

				pidl = SHBrowseForFolder(&bi);

				if (pidl)
					SHGetPathFromIDList(pidl, path);
			}
			nk_layout_row_end(ctx);

			nk_layout_row_dynamic(ctx, 25, 3);

			encrypted = nk_option_label(ctx, "No encryption", encrypted == 0) ? 0 : encrypted;
			encrypted = nk_option_label(ctx, "Vital encryption", encrypted == 1) ? 1 : encrypted;
			encrypted = nk_option_label(ctx, "Full encryption", encrypted == 2) ? 2 : encrypted;

			nk_layout_row_dynamic(ctx, 25, 1);
			num_users = nk_propertyi(ctx, "Number of users", 1, num_users, 8, 1, 1);

			nk_layout_row_dynamic(ctx, 196, 1);
			if (nk_group_begin(ctx, "Users", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
			{
				for (i = 0; i < num_users; i++)
				{
					nk_layout_row_begin(ctx, NK_DYNAMIC, 25, 2);
					if (encrypted == 0)
						nk_layout_row_push(ctx, 0.80f);
					else
						nk_layout_row_push(ctx, 0.40f);

					nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, usernames[i], 16, nk_filter_ascii);

					if (encrypted != 0)
					{
						nk_layout_row_push(ctx, 0.40f);
						nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, passwords[i], 16, nk_filter_ascii);
					}

					nk_layout_row_push(ctx, 0.20f);
					switch (user_perm[i])
					{
					case ADMIN: strcpy(selected_str, "Admin");
						break;

					case ART: strcpy(selected_str, "Artist");
						break;

					case CODE: strcpy(selected_str, "Coder");
						break;

					case SOUND: strcpy(selected_str, "Sound Des.");
						break;

					case MAP: strcpy(selected_str, "Mapper");
						break;
					}

					if (nk_combo_begin_label(ctx, selected_str, nk_vec2(nk_widget_width(ctx), 5 * 20 + 45)))
					{
						nk_layout_row_dynamic(ctx, 20, 1);
						if (admin == -1 || admin == i)
						{
							if (nk_combo_item_label(ctx, "Admin", NK_TEXT_ALIGN_LEFT))
							{
								admin = i;
								user_perm[i] = ADMIN;
							}
						}

						if (nk_combo_item_label(ctx, "Artist", NK_TEXT_ALIGN_LEFT))
							user_perm[i] = ART;

						if (nk_combo_item_label(ctx, "Coder", NK_TEXT_ALIGN_LEFT))
							user_perm[i] = CODE;

						if (nk_combo_item_label(ctx, "Sound Des.", NK_TEXT_ALIGN_LEFT))
							user_perm[i] = SOUND;

						if (nk_combo_item_label(ctx, "Mapper", NK_TEXT_ALIGN_LEFT))
							user_perm[i] = MAP;

						if (admin == i && user_perm != ADMIN)
							admin = -1;

						nk_combo_end(ctx);
					}

					nk_layout_row_end(ctx);
				}

				nk_group_end(ctx);
			}

			nk_layout_row_dynamic(ctx, 15, 1);
			if (nk_button_label(ctx, "Select all"))
			{
				for (i = 0; i < num_files; i++)
				{
					if (files[i].type == 0)
						files[i].commit = 1;
				}
			}

			nk_layout_row_dynamic(ctx, 256, 1);
			if (nk_group_begin(ctx, "Files", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
			{
				nk_layout_row_dynamic(ctx, 15, 1);
				for (i = 0; i < num_files; i++)
				{
					//nk_layout_row_dynamic(ctx, 20, 1);
					if (strcmp(files[i].parent, ".") == NULL && files[i].type == 0)
						files[i].commit = nk_check_label(ctx, files[i].file, files[i].commit == 1);
				}

				//	nk_tree_pop(ctx);
				//}

				for (i = 0; i < num_files; i++)
				{
					if (files[i].type == 1 && files[i].filenum > 0)
					{
						//nk_layout_row_dynamic(ctx, 20, 1);
						sprintf(str, "%s/%s", files[i].parent, files[i].file);
						if (nk_tree_push_id(ctx, NK_TREE_NODE, str, NK_MINIMIZED, i))
						{
							nk_layout_row_dynamic(ctx, 15, 1);
							for (j = 0; j < num_files; j++)
							{
								if (strcmp(files[j].parent, str) == NULL && files[j].type == 0)
									files[j].commit = nk_check_label(ctx, files[j].file, files[j].commit == 1);
							}

							nk_tree_pop(ctx);
						}
					}	
				}

				nk_group_end(ctx);
			}

			nk_layout_row_dynamic(ctx, 25, 1);
			nk_combobox_string(ctx, "Google Drive\0OneDrive\0Drop Box\0Other Storage", &storage, 4, 25, nk_vec2(90, 256));

			can_exp = 0;
			if (strlen(path) == 0) can_exp++;
			
			for (i = 0; i < num_users; i++)
			{
				if (strlen(usernames[i]) == 0)
				{
					can_exp++;
					break;
				}

				if (encrypted > 0 && strlen(passwords[i]) == 0)
				{
					can_exp++;
					break;
				}
			}

			if (can_exp != 0)
			{
				ctx->style.button.normal = ctx->style.button.hover = ctx->style.button.active;
				nk_button_label(ctx, "Export");
				SetThemeBack();
			}
			else
			{
				if (nk_button_label(ctx, "Export"))
					state = 2;
			}

			if (nk_button_label(ctx, "Cancel"))
				state = 3;

		}

		nk_end(ctx);
	}

	if (state == 2)
	{
		if (nk_begin(ctx, "Exporting", nk_rect(st.screenx / 2 - 125, st.screeny / 2 - 48, 350, 96), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE))
		{
			nk_layout_row_dynamic(ctx, 25, 2);

			if (timed == 0)
			{
				f_timer = st.time;
				timed = 1;
			}

			if (timed == 1)
			{
				if (st.time - f_timer > TICSPERSECOND / 2)
				{
					
					if (strcmp(dots, "...") == NULL)
						strcpy(dots, ".");
					else
						strcat(dots, ".");
						
					timed = 0;
					f_timer = 0;
				}
			}

			switch (exp_state)
			{
				case 0:
					sprintf(pct_str, "Creating base system%s",dots);
					break;

				case 1:
					sprintf(pct_str, "Copying files%s", dots);
					break;

				case 2:
					sprintf(pct_str, "Encrypting%s", dots);
					break;

				case 3:
					sprintf(pct_str, "Pushing to storage%s", dots);
					break;
			}
			
			nk_label(ctx, pct_str, NK_TEXT_ALIGN_LEFT);

			if (exp_state == 0)
			{
				if (steps == 0)
				{
					pct_inc = (num_files * 2) + 1 + num_users + 1;

					SetCurrentDirectory(path);
					CreateDirectory(msdk.prj.name, NULL);
					SetCurrentDirectory(msdk.prj.name);
					GetCurrentDirectory(MAX_PATH, msdk.prj.exp_path);

					CreateDirectory("versions", NULL);

					steps++;
				}
				else
				if (steps == 1)
				{
					SetCurrentDirectory(msdk.prj.exp_path);
					//SetCurrentDirectory("versions");
					/*
					if (encrypted != 0)
					{
					sha256_init(&hash);
					sha256_process(&hash, passwords[0], 16);
					sha256_done(&hash, hashed_password);

					if ((err = rng_make_prng(128, find_prng("yarrow"), &prng, NULL)) != CRYPT_OK)
					{
					sprintf(str2, "PRNG Error: %s", error_to_string(err));
					MessageBox(NULL, str2, "Error", MB_OK);
					}

					yarrow_read(salt, 128, &prng);

					hash2_len = 128;

					pkcs_5_alg2(hashed_password, strlen(hashed_password), salt, 128, 50000, find_hash("sha512"), hash2, &hash2_len);

					master_key_len = 32;

					aes_keysize(&master_key_len);

					if ((err = rng_make_prng(128, find_prng("yarrow"), &prng, NULL)) != CRYPT_OK)
					{
					sprintf(str2, "PRNG Error: %s", error_to_string(err));
					MessageBox(NULL, str2, "Error", MB_OK);
					}

					yarrow_read(master_key, master_key_len, &prng);

					if ((err = rc5_setup(hash2, hash2_len, 24, &key)) != CRYPT_OK)
					{
					sprintf(str2, "RC5 Error: %s", error_to_string(err));
					MessageBox(NULL, str2, "Error", MB_OK);
					}

					i = 0;
					do
					{
					rc5_ecb_encrypt(master_key + i, ck + i, &key);
					i += 8;
					} while (i < master_key_len);

					if ((err = rng_make_prng(128, find_prng("yarrow"), &prng, NULL)) != CRYPT_OK)
					{
					sprintf(str2, "PRNG Error: %s", error_to_string(err));
					MessageBox(NULL, str2, "Error", MB_OK);
					}

					yarrow_read(user_salt[0], 128, &prng);

					tmp2_len = 4;

					pkcs_5_alg2(usernames[0], strlen(usernames[0]), user_salt, 128, 50000, find_hash("sha512"), tmp2, &tmp2_len);

					for (i = 0; i < 4; i++)
					{
					if (i == 0) hexcode = abs(tmp2[i]);
					else hexcode |= abs(tmp2[i]);
					hexcode = hexcode << 8;
					}

					sprintf(str, "ID_%d.UA", hexcode);

					if ((f = fopen(str, "w")) == NULL)
					{
					sprintf(str, "User creation error: unable to create user %s file", usernames[0]);
					MessageBox(NULL, str, "UA Error", MB_OK);
					}

					ck_len = 32;
					salt_len = 128;
					fwrite(&salt_len, sizeof(int), 1, f);
					fwrite(&ck_len, sizeof(int), 1, f);
					fwrite(&salt, salt_len, 1, f);
					fwrite(&ck, ck_len, 1, f);

					fclose(f);

					sprintf(str, "%d", hexcode);

					SetCurrentDirectory(msdk.prj.exp_path);
					SetCurrentDirectory("users");
					CreateDirectory(str, NULL);

					SetCurrentDirectory(msdk.prj.exp_path);
					SetCurrentDirectory("mgear_sys");

					/*
					if ((err = rng_make_prng(128, find_prng("yarrow"), &prng, NULL)) != CRYPT_OK)
					{
					sprintf(str2, "PRNG Error: %s", error_to_string(err));
					MessageBox(NULL, str2, "Error", MB_OK);
					}

					yarrow_read(tmp, 1, &prng);
					*/
					/*
						for (i = 0; i < 16; i++)
						{
						tmp[0] = 0;
						while (((tmp[0] < 65 || tmp[0] > 90) && (tmp[0] < 97 || tmp[0] > 122) && (tmp[0] < 48 || tmp[0] > 57)))
						{
						if ((err = rng_make_prng(128, find_prng("yarrow"), &prng, NULL)) != CRYPT_OK)
						{
						sprintf(str2, "PRNG Error: %s", error_to_string(err));
						MessageBox(NULL, str2, "Error", MB_OK);
						}

						yarrow_read(tmp, 1, &prng);
						}
						recover_keys[0][i] = tmp[0];
						}

						sprintf(str, "rs_%d.UA", hexcode);

						if ((f = fopen(str, "w")) == NULL)
						{
						sprintf(str, "User creation error: unable to create recover system %s file", usernames[0]);
						MessageBox(NULL, str, "UA Error", MB_OK);
						}

						if ((err = rc5_setup(recover_keys[0], 16, 24, &key)) != CRYPT_OK)
						{
						sprintf(str2, "RC5 Error: %s", error_to_string(err));
						MessageBox(NULL, str2, "Error", MB_OK);
						}

						i = 0;
						do
						{
						rc5_ecb_encrypt(master_key + i, ck + i, &key);
						i += 8;
						} while (i < master_key_len);

						ck_len = 32;
						fwrite(&ck_len, sizeof(int), 1, f);
						fwrite(&ck, ck_len, 1, f);

						fclose(f);

						steps++;
						}
						else
						*/
					//{
					if ((f = fopen("user_list.ua", "w")) == NULL)
					{
						sprintf(str, "User creation error: unable to create user list file");
						MessageBox(NULL, str, "UA Error", MB_OK);
					}

					for (i = 0; i < num_users; i++)
					{
						sprintf(str, "%s;", usernames[i]);
						fwrite(str, 128, 1, f);
					}

					fclose(f);

					//SetCurrentDirectory(msdk.prj.exp_path);
					//SetCurrentDirectory("users");
					/*
					for (i = 0; i < num_users; i++)
					{
					sprintf(str, "%03d", i);
					CreateDirectory(str, NULL);
					}

					SetCurrentDirectory(msdk.prj.exp_path);
					SetCurrentDirectory("mgear_sys");
					*/
					exp_state = 1;
					steps = 0;

					//}
				}
			}
				/*
				else
				if (steps >= 1 && num_users - steps != 0)
				{
					sha256_init(&hash);
					sha256_process(&hash, passwords[steps - 1], 16);
					sha256_done(&hash, hashed_password);

					if ((err = rng_make_prng(128, find_prng("yarrow"), &prng, NULL)) != CRYPT_OK)
					{
						sprintf(str2, "PRNG Error: %s", error_to_string(err));
						MessageBox(NULL, str2, "Error", MB_OK);
					}

					yarrow_read(salt, 128, &prng);

					hash2_len = sizeof(hash2);

					pkcs_5_alg2(hashed_password, strlen(hashed_password), salt, 128, 50000, find_hash("sha512"), hash2, &hash2_len);

					if ((err = rc5_setup(hash2, hash2_len, 24, &key)) != CRYPT_OK)
					{
						sprintf(str2, "RC5 Error: %s", error_to_string(err));
						MessageBox(NULL, str2, "Error", MB_OK);
					}

					i = 0;
					do
					{
						rc5_ecb_encrypt(master_key + i, ck + i, &key);
						i += 8;
					} while (i < master_key_len);

					if ((err = rng_make_prng(128, find_prng("yarrow"), &prng, NULL)) != CRYPT_OK)
					{
						sprintf(str2, "PRNG Error: %s", error_to_string(err));
						MessageBox(NULL, str2, "Error", MB_OK);
					}

					yarrow_read(user_salt[steps - 1], 128, &prng);

					tmp2_len = 4;

					pkcs_5_alg2(usernames[steps - 1], strlen(usernames[steps - 1]), user_salt, 128, 50000, find_hash("sha512"), tmp2, &tmp2_len);

					for (i = 0; i < 4; i++)
					{
						if (i == 0) hexcode = abs(tmp2[i]);
						else hexcode |= abs(tmp2[i]);
						hexcode = hexcode << 8;
					}

					sprintf(str, "ID_%d.UA", hexcode);

					if ((f = fopen(str, "w")) == NULL)
					{
						sprintf(str, "User creation error: unable to create user %s file", usernames[i]);
						MessageBox(NULL, str, "UA Error", MB_OK);
					}

					salt_len = 128;
					ck_len = 32;
					fwrite(&salt_len, sizeof(int), 1, f);
					fwrite(&ck_len, sizeof(int), 1, f);
					fwrite(&salt, salt_len, 1, f);
					fwrite(&ck, ck_len, 1, f);

					fclose(f);

					sprintf(str, "%d", hexcode);

					SetCurrentDirectory(msdk.prj.exp_path);
					SetCurrentDirectory("users");
					CreateDirectory(str, NULL);

					SetCurrentDirectory(msdk.prj.exp_path);
					SetCurrentDirectory("mgear_sys");

					for (i = 0; i < 16; i++)
					{
						tmp[0] = 0;
						while (((tmp[0] < 65 || tmp[0] > 90) && (tmp[0] < 97 || tmp[0] > 122) && (tmp[0] < 48 || tmp[0] > 57)))
						{
							if ((err = rng_make_prng(128, find_prng("yarrow"), &prng, NULL)) != CRYPT_OK)
							{
								sprintf(str2, "PRNG Error: %s", error_to_string(err));
								MessageBox(NULL, str2, "Error", MB_OK);
							}

							yarrow_read(tmp, 1, &prng);
						}
						recover_keys[steps - 1][i] = tmp[0];
					}

					sprintf(str, "rs_%d.UA", hexcode);

					if ((f = fopen(str, "w")) == NULL)
					{
						sprintf(str, "User creation error: unable to create recover system %s file", usernames[0]);
						MessageBox(NULL, str, "UA Error", MB_OK);
					}

					if ((err = rc5_setup(recover_keys[steps - 1], 16, 24, &key)) != CRYPT_OK)
					{
						sprintf(str2, "RC5 Error: %s", error_to_string(err));
						MessageBox(NULL, str2, "Error", MB_OK);
					}

					i = 0;
					do
					{
						rc5_ecb_encrypt(master_key + i, ck + i, &key);
						i += 8;
					} while (i < master_key_len);

					ck_len = 32;
					fwrite(&ck_len, sizeof(int), 1, f);
					fwrite(&ck, ck_len, 1, f);

					fclose(f);

					steps++;
				}
				
				if (steps > 1 && num_users - steps == 0)
				{
					exp_state = 1;
					steps = 0;
				}
			}
			*/
			else
			if (exp_state == 1)
			{
				if (steps < num_files)
				{
					//SetCurrentDirectory(msdk.prj.prj_path);

					//ToDoBaseListSave();

					if (fc == NULL)
					{
						openfile_d(fc, "v0000", "wb");
						fwrite("REVFILE", 7, 1, fc);
						fwrite(&msdk.prj.curr_rev, 2, 1, fc);

						prj_files = malloc(sizeof(_Files)* num_files);

						for (i = 0, j = 0; i < num_files; i++)
						{
							if (files[i].commit == 1 && files[i].type == 0)
							{
								strcpy(prj_files[j].path, files[i].parent);
								strcat(prj_files[j].path, StringFormat("/%s", files[i].file);
								prj_files[j].f_rev = prj_files[i].f_rev = 0;
								prj_files[j].size = files[i].size;
								memcpy(prj_files[j].hash, files[i].hash, 512);
								j++;
							}
						}

						num_files_commited = j;

						fwrite(&j, sizeof(int16), 1, fc);
						fwrite(prj_files, sizeof(_Files)* j, 1, fc);
						fwrite("First Commit", 1024, 1, fc);

						memcpy(msdk.prj.users, usernames, 8 * 16);
						msdk.prj.num_users = num_users;

						msdk.prj.revisions = 0;
						msdk.prj.curr_rev = 0;

						msdk.prj.user_id = 0;

						SavePrjFile(msdk.filepath);

						SetCurrentDirectory(msdk.prj.exp_path);
						SetCurrentDirectory("versions");
						CreateDirectory("0", NULL);
						SetCurrentDirectory("0");
					}
					
					if (files[steps].type != 0 || files[steps].commit == 0) steps++;
					else
					{
						strcpy(filepath, files[steps].path);
						strcat(filepath, "/");
						strcat(filepath, files[steps].file);
						
						//extension = strrchr(files[steps].file, '.');
						
						//sprintf(newfilepath, "f%04dr0000.%s", steps, extension);
						
						//buf = GetRootDir(files[steps].parent);

						//if (strcmp(buf, "_prj_raw") == NULL)
							//SetCurrentDirectory("raw");
						//else
							//SetCurrentDirectory("prj");
						/*
						if (encrypted == 1)
						{
							if (strstr(files[steps].file, ".sdkprj") != NULL || strstr(files[steps].file, ".cfg") != NULL || strstr(files[steps].file, ".list") != NULL
								|| strstr(files[steps].file, ".texprj") != NULL || strstr(files[steps].file, ".mgm") != NULL || strstr(files[steps].file, ".MGM") != NULL)
							{
								if ((err = aes_setup(master_key, master_key_len, NULL, &key)) != CRYPT_OK)
								{
									MessageBoxRes("Error", MB_OK, "Error when encrypting file %s: %s", files[steps].file, error_to_string(err));
								}

								if ((f = fopen(filepath, "r")) == NULL)
								{
									MessageBoxRes("Error", MB_OK, "Error while opening file %s for encryption", filepath);
								}

								if ((f = fopen(newfilepath, "w")) == NULL)
								{
									MessageBoxRes("Error", MB_OK, "Error while creating file %s", newfilepath);
								}

								i = 0;
								do
								{
									fread(tmp, 16, 1, f);
									aes_ecb_encrypt(tmp, ck, &key);
									fwrite(ck, 16, 1, f2);

								} while (!feof(f));

								files[steps].size = ftell(f);

								fclose(f);
								fclose(f2);
							}
							else
							{
								/*
								if ((f = fopen(filepath, "r")) == NULL)
								{
									MessageBoxRes("Error", MB_OK, "Error while opening file %s for copy", filepath);
								}

								fseek(f, SEEK_END, 0);
								files[steps].size = ftell(f);
								
								rewind(f);
							
								char *buffer;
							
								alloc_mem(buffer, files[steps].size);
							
								fread(buffer, files[steps].size, 1, f);
							
								sha256_init(&hash);
								sha256_process(&hash, buffer, files[steps].size);
								sha256_done(&hash, files[steps].hash);

								fclose(f);
								
								free(buffer);
								*/
						/*
								if (CopyFile(filepath, newfilepath, FALSE) == NULL)
								{
									sprintf(str, "Error %x when copying file: %s", GetLastError(), files[steps].file);
									MessageBox(NULL, str, "Error", MB_OK);
								}
							}
						}
						else
						*/
						//{
						/*
							if (CopyFile(filepath, newfilepath, FALSE) == NULL)
							{
								sprintf(str, "Error %x when copying file: %s", GetLastError(), files[steps].file);
								MessageBox(NULL, str, "Error", MB_OK);
							}
							*/
						//}

						if ((f = fopen(filepath, "rb")) == NULL)
						{
							MessageBoxRes("Error", MB_OK, "Could not open file %s", filepath);
						}
						else
						{
							size_t filesize;
							getfilesize(f, filesize);

							void *data;

							alloc_mem(data, filesize);

							fread(data, filesize, 1, f);
							fclose(f);

							fwrite(&filesize, sizeof(size_t), 1, fc);
							fwrite(data, filesize, 1, fc);

							fflush(fc);

							free_mem(data);
						}

						//SetCurrentDirectory("..");

						steps++;
					}
				}
				else
				{
					fclose(fc);

					if (encrypted != 0)
					{
						exp_state = 2;
					}
					else
						exp_state = 3;

					SetCurrentDirectory(msdk.prj.exp_path);
					
					openfile_d(f, "log_journal", "wb");
					
					
					
					fclose(f);

					if ((f = fopen("importer", "wb")) == NULL)
					{
						MessageBoxRes("Error", MB_OK, "Error: Could not create importer file");
					}

					strcpy(exporter.name, msdk.prj.name);
					exporter.rev = 0;
					exporter.curr_rev = 0;
					exporter.encrypted = encrypted;
					exporter.num_users = num_users;
					
					fwrite(&exporter, sizeof(struct SDKEXP), 1, f);

					fclose(f);

					if ((f = fopen("index", "wb")) == NULL)
					{
						MessageBoxRes("Error", MB_OK, "Error: Could not create index file");
					}
					
					
					fwrite(&num_files_commited, sizeof(int), 1, f);
					fwrite(prj_files, sizeof(_Files) * num_files_commited, 1, f);

					fclose(f);

					steps = 0;
				}
			}
			else
			if (exp_state == 3)
			{
				if (steps == 0)
				{
					if (MessageBoxRes("First Commit", MB_YESNO, "Would you like to push the commit now?") == IDYES)
					{

					}
					else
					{
						exp_state = 4;
						steps = 0;
					}
				}
			}
			else
			if (exp_state == 4)
			{
				MessageBoxRes("Export complete", MB_OK, "Exporting complete");
				state = 4;
			}

			if (exp_state == 0)
				pct = steps;
			
			if (exp_state == 1)
			{
				if (encrypted != 2)
					pct = (steps * 2) + num_users + 1;
				else
					pct = steps + num_users + 1;
			}

			if (exp_state == 2)
				pct = (steps * 2) + num_users + 1;

			if (exp_state == 3)
				pct = steps + (num_files * 2) + num_users + 1;

			if (exp_state == 4)
				pct = 100/pct_inc;

			nk_label(ctx, StringFormat("%d%%",pct * pct_inc), NK_TEXT_ALIGN_RIGHT);
		}

		nk_end(ctx);
	}

	if (state == 3)
	{
		state = 0;
		return -1;
	}

	if (state == 4)
	{
		state = 0;
		return 1;
	}

	return NULL;
}

void UnloadPrj()
{
	if (msdk.prj.log)
		free(msdk.prj.log);

	if (msdk.prj.log_list)
		free(msdk.prj.log_list);

	//if (msdk.prj.TDList_entries > 0)
		//free(msdk.prj.TDList);

	memset(&msdk.prj, 0, sizeof(SDKPRJ));
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

int8 CheckForFFmpeg()
{
	FILE *f;
	DIR *d;

	char path[MAX_PATH];

	strcpy(path, st.CurrPath);
	strcat(path, "\\Tools\\FFmpeg\\");

	strcat(path, "ffmpeg.exe");
	if ((f = fopen(path, "rb")) != NULL)
	{
		fclose(f);
		return 1;
	}

	strcpy(path, st.CurrPath);
	strcat(path, "\\..\\temp\\ffmpeg-32-bit.zip");

	if ((f = fopen(path, "rb")) != NULL)
	{
		fclose(f);
		return 0;
	}

	return -1;
}

void CheckFFmpegConversionFolder()
{
	DIR *dir;
	char path[MAX_PATH], str[MAX_PATH];

	strcpy(path, st.CurrPath);
	strcat(path, "\\Tools\\FFmpeg\\MGV");

	if ((dir = opendir(path)) != NULL)
	{
		SetCurrentDirectory(st.CurrPath);
		system("del Tools\\FFmpeg\\MGV\\*.* /q");
		strcpy(str, st.CurrPath);
		strcat(str, "\\Tools\\FFmpeg");
		SetCurrentDirectory(str);

		system("rd MGV /s /q");

		SetCurrentDirectory(st.CurrPath);

		closedir(dir);
	}
	
}

int Preferences()
{
	static char str[32];
	register int i;
	static FILE *f;
	static int state = 0;
	int temp, temp2;

	if (msdk.theme == THEME_WHITE) strcpy(str, "White skin");
	if (msdk.theme == THEME_RED) strcpy(str, "Red skin");
	if (msdk.theme == THEME_BLUE) strcpy(str, "Blue skin");
	if (msdk.theme == THEME_DARK) strcpy(str, "Dark skin");
	//if (msdk.theme == THEME_GWEN) strcpy(str, "GWEN skin");
	if (msdk.theme == THEME_BLACK) strcpy(str, "Default");

	if (nk_begin(ctx, "Preferences", nk_rect(st.screenx / 2 - 100, st.screeny / 2 - 100, 200, 170), NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE))
	{
		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "UI skin", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 25, 1);
		if (nk_combo_begin_label(ctx, str, nk_vec2(nk_widget_width(ctx), 225)))
		{
			nk_layout_row_dynamic(ctx, 25, 1);

			if (nk_combo_item_label(ctx, "White skin", NK_TEXT_ALIGN_LEFT))
			{
				msdk.theme = THEME_WHITE;
				SetSkin(ctx, THEME_WHITE);
				strcpy(str, "White skin");
			}

			if (nk_combo_item_label(ctx, "Red skin", NK_TEXT_ALIGN_LEFT))
			{
				msdk.theme = THEME_RED;
				SetSkin(ctx, THEME_RED);
				strcpy(str, "Red skin");
			}

			if (nk_combo_item_label(ctx, "Blue skin", NK_TEXT_ALIGN_LEFT))
			{
				msdk.theme = THEME_BLUE;
				SetSkin(ctx, THEME_BLUE);
				strcpy(str, "Blud skin");
			}

			if (nk_combo_item_label(ctx, "Dark skin", NK_TEXT_ALIGN_LEFT))
			{
				msdk.theme = THEME_DARK;
				SetSkin(ctx, THEME_DARK);
				strcpy(str, "Dark skin");
			}
			/*
			if (nk_combo_item_label(ctx, "GWEN skin", NK_TEXT_ALIGN_LEFT))
			{
				msdk.theme = THEME_GWEN;
				SetSkin(ctx, THEME_GWEN);
				strcpy(str, "GWEN skin");
			}
			*/
			if (nk_combo_item_label(ctx, "Default", NK_TEXT_ALIGN_LEFT))
			{
				msdk.theme = THEME_BLACK;
				SetSkin(ctx, THEME_BLACK);
				strcpy(str, "Default skin");
			}

			nk_combo_end(ctx);
		}

		nk_layout_row_dynamic(ctx, 25, 2);
		if (msdk.ffmpeg_downloaded == 1)
			sprintf(str, "FFmpeg: Yes");
		else
			sprintf(str, "FFmpeg: No");

		nk_label(ctx, str,NK_TEXT_ALIGN_LEFT);

		if (state == 1 || msdk.ffmpeg_installed == 1)
		{
			ctx->style.button.normal = ctx->style.button.hover = ctx->style.button.active;
			nk_button_label(ctx, "Install FFmpeg");
			SetThemeBack();
		}
		else
		{
			if (nk_button_label(ctx, "Install FFmpeg"))
			{
				if (msdk.ffmpeg_downloaded == 0)
				{
					if ((f = fopen("..\\temp\\ffmpeg-32-bit.zip", "wb")) == NULL)
						MessageBox(NULL, "Error: could not create ZIP file", "Error", MB_OK);
					else
					{
						msdk.curl = curl_easy_init();
						msdk.curlm = curl_multi_init();

						curl_easy_setopt(msdk.curl, CURLOPT_URL, "https://ffmpeg.zeranoe.com/builds/win32/static/ffmpeg-20180424-d06b01f-win32-static.zip");
						curl_easy_setopt(msdk.curl, CURLOPT_TIMEOUT, 120L);
						curl_easy_setopt(msdk.curl, CURLOPT_CAINFO, "./ca-bundle.crt");
						curl_easy_setopt(msdk.curl, CURLOPT_SSL_VERIFYPEER, 0);
						curl_easy_setopt(msdk.curl, CURLOPT_SSL_VERIFYHOST, 0);

						curl_easy_setopt(msdk.curl, CURLOPT_WRITEFUNCTION, CURLWriteData);

						curl_easy_setopt(msdk.curl, CURLOPT_WRITEDATA, f);

						curl_easy_setopt(msdk.curl, CURLOPT_VERBOSE, 0);

						curl_easy_setopt(msdk.curl, CURLOPT_NOPROGRESS, 0);
						curl_easy_setopt(msdk.curl, CURLOPT_PROGRESSFUNCTION, CURLProgress);

						curl_multi_add_handle(msdk.curlm, msdk.curl);

						msdk.downloading = 1;

						state = 1;
					}
				}
				
				if (msdk.ffmpeg_downloaded == 1)
				{
					system("Tools\\7z.exe e \"..\\temp\\ffmpeg-32-bit.zip\" \"-o..\\bin\\Tools\\FFmpeg\" ffmpeg.exe -r");
					system("Tools\\7z.exe e \"..\\temp\\ffmpeg-32-bit.zip\" \"-o..\\bin\\Tools\\FFmpeg\" LICENSE.TXT -r");
					system("Tools\\7z.exe e \"..\\temp\\ffmpeg-32-bit.zip\" \"-o..\\bin\\Tools\\FFmpeg\" README.TXT -r");
					system("pause");

					temp = CheckForFFmpeg();

					if (temp == 1)
					{
						MessageBox(NULL, "FFmpeg was installed successfully", NULL, MB_OK);
						msdk.ffmpeg_installed = 1;
					}

					if (temp == 0)
					{
						MessageBox(NULL, "Error: could not install FFmpeg\nTry downloading it again", "Error", MB_OK);
						msdk.ffmpeg_installed = msdk.ffmpeg_downloaded = 0;
					}

					if (temp == -1)
					{
						MessageBox(NULL, "Error: FFmpeg not installed", "Error", MB_OK);
						msdk.ffmpeg_installed = msdk.ffmpeg_downloaded = 0;
					}
				}
			}
		}

		nk_layout_row_dynamic(ctx, 25, 1);

		if (msdk.downloading == 1)
		{
			sprintf(str, "Downloading %0.2f", (msdk.download_now / msdk.download_total)*100.0f);
			nk_label(ctx, str, NK_TEXT_ALIGN_LEFT);
		}

		if (msdk.downloading == 0)
		{
			if (nk_button_label(ctx, "Ok"))
			{
				SaveCFG();
				nk_end(ctx);
				return 1;
			}
		}
	}

	nk_end(ctx);

	if (state == 1)
	{
		msdk.resm = curl_multi_perform(msdk.curlm, &temp);

		msdk.curlmsg = curl_multi_info_read(msdk.curlm, &temp2);

		if (msdk.curlmsg && msdk.curlmsg->msg == CURLMSG_DONE)
		{
			curl_multi_remove_handle(msdk.curlm, msdk.curl);
			curl_easy_cleanup(msdk.curl);
			curl_multi_cleanup(msdk.curlm);
			msdk.downloading = 0;
			state = 0;
			fclose(f);

			temp = CheckForFFmpeg();

			if (temp == 0 || temp == 1)
			{
				system("Tools\\7z.exe e \"..\\temp\\ffmpeg-32-bit.zip\" \"-o..\\bin\\Tools\\FFmpeg\" ffmpeg.exe -r");
				system("Tools\\7z.exe e \"..\\temp\\ffmpeg-32-bit.zip\" \"-o..\\bin\\Tools\\FFmpeg\" LICENSE.TXT -r");
				system("Tools\\7z.exe e \"..\\temp\\ffmpeg-32-bit.zip\" \"-o..\\bin\\Tools\\FFmpeg\" README.TXT -r");
				system("pause");

				if (CheckForFFmpeg() != 1)
				{
					MessageBox(NULL, "Error: could not install FFmpeg\nTry downloading it again", "Error", MB_OK);
					msdk.ffmpeg_installed = msdk.ffmpeg_downloaded = 0;
				}

				msdk.ffmpeg_downloaded = 1;
				msdk.ffmpeg_installed = 1;
			}

			if (temp == -1)
			{
				MessageBox(NULL, "Error: could not download FFmpeg", "Error", MB_OK);
				msdk.ffmpeg_installed = msdk.ffmpeg_downloaded = 0;
			}
			
		}
	}

	return NULL;
}

int NewProject()
{
	static int state = 0;
	static char path[MAX_PATH];
	char str[MAX_PATH * 3], filepath[MAX_PATH];

	BROWSEINFO bi;

	static LPITEMIDLIST pidl;

	char directory[MAX_PATH];
	char exepath[MAX_PATH];
	char args[MAX_PATH * 3];
	static path2[MAX_PATH];
	MSG msg;

	DWORD error;

	static DWORD exitcode;

	static SHELLEXECUTEINFO info;

	ZeroMemory(&bi, sizeof(bi));

	if (state == 0)
	{
		ZeroMemory(path, MAX_PATH);
		pidl = NULL;

		state = 1;
	}

	if (nk_begin(ctx, "New project", nk_rect((st.screenx / 2) - 256, (st.screeny / 2) - 260, 512, 320), NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE))
	{
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_label(ctx, "Project name", NK_TEXT_ALIGN_LEFT);
		nk_edit_string_zero_terminated(ctx, NK_EDIT_SIMPLE, msdk.prj.name, 32, nk_filter_ascii);

		nk_layout_row_dynamic(ctx, 10, 1);
		nk_spacing(ctx, 1);

		nk_layout_row_dynamic(ctx, 25, 1);
		nk_label(ctx, "Project path", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_begin(ctx, NK_DYNAMIC, 25, 2);
		nk_layout_row_push(ctx, 0.7f);
		nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, path, MAX_PATH, nk_filter_ascii);

		nk_layout_row_push(ctx, 0.3f);
		if (nk_button_label(ctx, "Browse"))
		{
			bi.lpszTitle = ("Select a folder to create the project");
			bi.ulFlags = BIF_USENEWUI;

			pidl = SHBrowseForFolder(&bi);

			if (pidl)
				SHGetPathFromIDList(pidl, path);

		}

		nk_layout_row_end(ctx);

		nk_layout_row_dynamic(ctx, 25, 1);
		nk_label(ctx, "Code type", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 25, 2);
		msdk.prj.code_type = nk_option_label(ctx, "C\\C++", msdk.prj.code_type == 0) ? 0 : msdk.prj.code_type;
		msdk.prj.code_type = nk_option_label(ctx, "MGL", msdk.prj.code_type == 1) ? 1 : msdk.prj.code_type;


		nk_layout_row_dynamic(ctx, 25, 7);
		nk_spacing(ctx, 2);

		if (path[0] == NULL || msdk.prj.name[0] == NULL)
		{
			ctx->style.button.normal = ctx->style.button.hover = ctx->style.button.active;
			nk_button_label(ctx, "Create");
			nk_style_default(ctx);
		}
		else
		{
			if (nk_button_label(ctx, "Create"))
			{
				SetCurrentDirectory(st.CurrPath);

				strcpy(exepath, st.CurrPath);
				strcat(exepath, "\\Tools\\");
				strcat(exepath, "7z.exe");
				strcpy(directory, st.CurrPath);
				//strcpy(path2, path);

				sprintf(args, "x \"data\\project.zip\"  \"-o%s\"", path);

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
					MessageBox(NULL, "Could not execute 7zip", "Error", MB_OK);
					UnloadPrj();
					state = 0;
					nk_end(ctx);
					return -1;
				}

				error = WaitForSingleObject(info.hProcess, INFINITE);

				if (error == WAIT_OBJECT_0)
				{
					// Return exit code from process
					GetExitCodeProcess(info.hProcess, &exitcode);

					if (exitcode != 0)
					{
						MessageBox(NULL, "Error while creating project folder 7ZERROR", "Error", MB_OK);
						LogApp("Error while creating project folder 7ZERROR");
						UnloadPrj();
						state = 0;
						nk_end(ctx);
						return -1;
					}

					CloseHandle(info.hProcess);
				}

				//sprintf(str, "Tools\\7z.exe x \"data\\project.zip\"  \"-o%s\"", path);
				//system(str);

				SetCurrentDirectory(path);
				if (rename("untitled", msdk.prj.name) != NULL)
				{
					MessageBox(NULL, "Error while creating project folder", "Error", MB_OK);
					LogApp("Error while creating project folder");
					UnloadPrj();
					state = 0;
					nk_end(ctx);
					return -1;
				}

				strcpy(msdk.prj.prj_path, path);
				strcat(msdk.prj.prj_path, "\\");
				strcat(msdk.prj.prj_path, msdk.prj.name);

				SetCurrentDirectory(msdk.prj.prj_path);

				strcpy(msdk.prj.prj_raw_path, path);
				strcat(msdk.prj.prj_raw_path, "\\");
				strcat(msdk.prj.prj_raw_path, "_prj_raw");

				strcpy(msdk.prj.code_path, msdk.prj.prj_raw_path);
				strcat(msdk.prj.code_path, "\\");
				strcat(msdk.prj.code_path, "CODE");

				msdk.prj.audio = msdk.prj.code = msdk.prj.map = msdk.prj.sprites = msdk.prj.ui = msdk.prj.tex = 1;
				msdk.prj.curr_rev = 0;
				msdk.prj.revisions = 0;
				//msdk.prj.TDList_entries = 0;
				msdk.prj.log = NULL;
				//msdk.prj.TDList = NULL;

				strcpy(msdk.filepath, msdk.prj.prj_path);
				strcat(msdk.filepath, "\\");
				strcat(msdk.filepath, msdk.prj.name);
				strcat(msdk.filepath, ".sdkprj");

				if (!SavePrjFile(msdk.filepath))
				{
					MessageBox(NULL, "Error while creating project file", "Error", MB_OK);
					LogApp("Error while creating project file");
					UnloadPrj();
					state = 0;
					nk_end(ctx);
					return -1;
				}

				msdk.prj.loaded = 1;

				nk_end(ctx);
				return 1;
			}
		}

		nk_spacing(ctx, 2);
		if (nk_button_label(ctx, "Cancel"))
		{
			nk_end(ctx);
			UnloadPrj();
			state = 0;
			return -1;
		}
	}

	nk_end(ctx);

	return 0;
}

int MGVCompiler()
{
	static char file[MAX_PATH], file2[MAX_PATH];
	char str[512];
	static int state = -1, fps = 30;
	int temp;

	char directory[MAX_PATH];
	char exepath[MAX_PATH];
	char args[MAX_PATH * 3];
	static path2[MAX_PATH];
	MSG msg;

	DWORD error;

	static DWORD exitcode;

	static SHELLEXECUTEINFO info;

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	ofn.lpstrFilter = "Video Files\0*.avi;*.mp4;*.ivf;*.webm;*.mov;*.mpeg;*.wmv;\0";
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFile = file;
	ofn.lpstrTitle = "Select the file";
	//ofn.hInstance = OFN_EXPLORER;
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_EXPLORER;

	if (state == -1)
	{
		CheckFFmpegConversionFolder();
		memset(file, 0, MAX_PATH);

		switch (CheckForFFmpeg())
		{
		case -1:
			msdk.ffmpeg_installed = msdk.ffmpeg_downloaded = 0;
			break;

		case 0:
			msdk.ffmpeg_installed = 0;
			msdk.ffmpeg_downloaded = 1;
			break;

		case 1:
			msdk.ffmpeg_installed = msdk.ffmpeg_downloaded = 1;
			break;
		}

		state = 0;
	}

	if (nk_begin(ctx, "MGV Encoder", nk_rect((st.screenx / 2), (st.screeny / 2), 356, 256), NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE))
	{
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_label(ctx, "Select the video file",NK_TEXT_ALIGN_LEFT);

		nk_layout_row_begin(ctx, NK_DYNAMIC, 25, 2);
		nk_layout_row_push(ctx, 0.9f);
		nk_edit_string_zero_terminated(ctx, NK_EDIT_SIMPLE, file, MAX_PATH, nk_filter_ascii);

		nk_layout_row_push(ctx, 0.1f);
		if (nk_button_label(ctx, "Browse"))
		{
			GetOpenFileName(&ofn);
		}

		nk_layout_row_end(ctx);

		nk_layout_row_dynamic(ctx, 25, 1);
		nk_label(ctx, "MGV file name", NK_TEXT_ALIGN_LEFT);

		nk_edit_string_zero_terminated(ctx, NK_EDIT_SIMPLE, file2, MAX_PATH, nk_filter_ascii);

		fps = nk_propertyi(ctx, "FPS", 10, fps, 60, 1, 1);

		if (state > 0)
		{
			ctx->style.button.normal = ctx->style.button.hover = ctx->style.button.active;
			nk_button_label(ctx, "Converting...");
		}
		else
		{
			if (nk_button_label(ctx, "Convert"))
			{
				if (file2)
				{
					SetCurrentDirectory(st.CurrPath);
					system("md Tools\\FFmpeg\\MGV");

					if (state == 0)
					{
						strcpy(exepath, st.CurrPath);
						strcat(exepath, "\\Tools\\FFmpeg\\");
						strcat(exepath, "ffmpeg.exe");
						strcpy(directory, exepath);
						//strcpy(path2, path);
						PathRemoveFileSpec(directory);

						sprintf(args, "-i \"%s\" -r %d \"Tools\\FFmpeg\\MGV\\frame%%05d.jpg\"", file, fps);

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
							MessageBox(NULL, "Could not execute FFmpeg", "Error", MB_OK);
							state = -1;
							return -1;
						}

						state = 1;
					}
					/*
					sprintf(str, "Tools\\FFmpeg\\ffmpeg.exe -i \"%s\" -r 30 Tools\\FFmpeg\\MGV\\frame%%05d.jpg", file);
					system(str);

					sprintf(str, "Tools\\FFmpeg\\ffmpeg.exe -i \"%s\" Tools\\FFmpeg\\MGV\\MGV.wav", file);
					system(str);

					system("copy Tools\\FFmpeg\\MGV\\frame00001.jpg Tools\\FFmpeg\\MGV\\frame00000.jpg");

					temp = DirFrames("Tools\\FFmpeg\\MGV");

					sprintf(str, "Tools\\mgvcreator.exe -o test.mgv -p Tools\\FFmpeg\\MGV -fps 30 -n %d", temp - 1);

					system(str);

					system("del Tools\\FFmpeg\\MGV\\*.* /q");
					strcpy(str, st.CurrPath);
					strcat(str, "\\Tools\\FFmpeg");
					SetCurrentDirectory(str);

					system("rd MGV /s /q");

					SetCurrentDirectory(st.CurrPath);
					*/
				}
			}
		}
	}

	nk_end(ctx);

	if (state > 0)
	{
		error = WaitForSingleObject(info.hProcess, 0);

		if (error == WAIT_OBJECT_0)
		{
			// Return exit code from process
			GetExitCodeProcess(info.hProcess, &exitcode);

			CloseHandle(info.hProcess);

			if (state == 3)
			{
				system("del Tools\\FFmpeg\\MGV\\*.* /q");
				strcpy(str, st.CurrPath);
				strcat(str, "\\Tools\\FFmpeg");
				SetCurrentDirectory(str);

				system("rd MGV /s /q");

				SetCurrentDirectory(st.CurrPath);

				state = -1;
				return 1;
			}
			else
			if (state == 2)
			{
				system("copy Tools\\FFmpeg\\MGV\\frame00001.jpg Tools\\FFmpeg\\MGV\\frame00000.jpg");

				temp = DirFrames("Tools\\FFmpeg\\MGV");

				strcpy(exepath, st.CurrPath);
				strcat(exepath, "\\Tools\\");
				strcat(exepath, "mgvcreator.exe");
				strcpy(directory, exepath);
				//strcpy(path2, path);
				PathRemoveFileSpec(directory);

				sprintf(args, "-o \"%s\" -p \"Tools\\FFmpeg\\MGV\" -fps %d -n %d", file2, fps, temp - 1);

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
					MessageBox(NULL, "Could not execute MGV Creator", "Error", MB_OK);
					state = 0;
					return -1;
				}

				state = 3;
			}
			else
			if (state == 1)
			{
				strcpy(exepath, st.CurrPath);
				strcat(exepath, "\\Tools\\FFmpeg\\");
				strcat(exepath, "ffmpeg.exe");
				strcpy(directory, exepath);
				//strcpy(path2, path);
				PathRemoveFileSpec(directory);

				sprintf(args, "-i \"%s\" \"Tools\\FFmpeg\\MGV\\MGV.WAV\"", file);

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
					MessageBox(NULL, "Could not execute FFmpeg", "Error", MB_OK);
					state = 0;
					return -1;
				}

				state = 2;
			}
		}
	}

	return 0;
}

void MenuBar()
{
	register int i, a, m;
	char mapname[2048], str[128], filename[2048];
	int id = 0, id2 = 0, check, temp;
	static int state = 0, mggid, path[MAX_PATH];

	//if (nkrendered==0)
	//{
		if (nk_begin(ctx, "Menu", nk_rect(0, 0, st.screenx, 30), NK_WINDOW_NO_SCROLLBAR))
		{
			nk_menubar_begin(ctx);
			nk_layout_row_begin(ctx, NK_STATIC, 25, 3);

			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(120, 270)))
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				if (nk_menu_item_label(ctx, "New project", NK_TEXT_LEFT))
				{
					UnloadPrj();
					state = 1;
				}

				if (nk_menu_item_label(ctx, "Open project", NK_TEXT_LEFT))
				{
					UnloadPrj();

					OPENFILENAME ofn;
					ZeroMemory(&path, sizeof(path));
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
					ofn.lpstrFilter = "SDK project Files\0*.sdkprj\0";
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFile = path;
					ofn.lpstrTitle = "Select the project file";
					//ofn.hInstance = OFN_EXPLORER;
					ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_EXPLORER;

					if (GetOpenFileName(&ofn))
					{
						temp = LoadPrjFile(path);

						if (temp == 0)
							MessageBox(NULL, "Error could not open the file", "Error", MB_OK);

						if (temp == -1)
							MessageBox(NULL, "Error invalid file", "Error", MB_OK);

						if (temp == 1)
						{
							msdk.prj.loaded = 1;
							strcpy(msdk.filepath, path);
						}
					}

					state = 0;
				}

				if (nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT))
				{
					SavePrjFile(msdk.filepath);

					if (msdk.prj.curr_rev > 0)
					{
						//SaveRevLog();
						//ToDoRevListSave();
					}
					else
					{
						//SaveBaseLog();
						//ToDoBaseListSave();
					}
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

				nk_menu_item_label(ctx, "Console", NK_TEXT_LEFT);
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
			temp = NewProject();

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
			temp = ExportProject();

			if (temp == -1 || temp == 1)
				state = 0;
		}
	//}
}

void Pannel()
{
	register int i, j, k;
	static int sl, pannel_state = 0, selected_entry = -1, tdtype = 0, assigned_ids = 0;
	static struct nk_color editcolor = { 255, 255, 255, 255 };
	static char strbuf[32], entry[512];
	static struct nk_rect bounds;
	TEX_DATA data;
	int temp, px, py, sx, sy, tmp;
	struct nk_image texid;

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

	//if (nkrendered == 0)
	//{
		if (nk_begin(ctx, "Left Pannel", nk_rect(0, 30, st.screenx, st.screeny - 30), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER | NK_WINDOW_BACKGROUND))
		{
			nk_layout_row_dynamic(ctx, 25, 10);

			nk_spacing(ctx, 1);
			
			if (nk_button_label(ctx, "mAudio"))
			{
				ZeroMemory(&info, sizeof(info));
				info.cbSize = sizeof(info);
				info.lpVerb = ("open");
				strcpy(exepath, st.CurrPath);
				strcat(exepath, "\\mAudio.exe");
				info.lpFile = exepath;
				//sprintf(args, "-o \"%s\"", path2);
				//info.lpParameters = args;
				info.lpDirectory = st.CurrPath;
				info.nShow = SW_SHOW;

				if (!ShellExecuteEx(&info))
					MessageBox(NULL, "Could not execute mAudio", "Error", MB_OK);
			}
			
			nk_button_label(ctx, "mCode");

			nk_button_label(ctx, "mEngineer");
			
			if (nk_button_label(ctx, "MGGViewer"))
			{
				ZeroMemory(&info, sizeof(info));
				info.cbSize = sizeof(info);
				info.lpVerb = ("open");
				strcpy(exepath, st.CurrPath);
				strcat(exepath, "\\mggviewer.exe");
				info.lpFile = exepath;
				//sprintf(args, "-o \"%s\"", path2);
				//info.lpParameters = args;
				info.lpDirectory = st.CurrPath;
				info.nShow = SW_SHOW;

				if (!ShellExecuteEx(&info))
					MessageBox(NULL, "Could not execute MGGViewer", "Error", MB_OK);
			}

			if (nk_button_label(ctx, "MGVCreator"))
				state = 1;

			nk_button_label(ctx, "mInterface");
			nk_button_label(ctx, "mSprite");

			if (nk_button_label(ctx, "mTex"))
			{
				ZeroMemory(&info, sizeof(info));
				info.cbSize = sizeof(info);
				info.lpVerb = ("open");
				strcpy(exepath, st.CurrPath);
				strcat(exepath, "\\mTex.exe");
				info.lpFile = exepath;
				//sprintf(args, "-o \"%s\"", path2);
				//info.lpParameters = args;
				info.lpDirectory = st.CurrPath;
				info.nShow = SW_SHOW;

				if (!ShellExecuteEx(&info))
					MessageBox(NULL, "Could not execute mTex", "Error", MB_OK);
			}

			nk_spacing(ctx, 1);
			
			nk_layout_row_dynamic(ctx, st.screeny - 100, 3);

			if (nk_group_begin(ctx, "Settings", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
			{
				nk_layout_row_dynamic(ctx, 25, 1);
				nk_button_label(ctx, "Run");
				nk_button_label(ctx, "Debug");

				//nk_layout_row_dynamic(ctx, 25, 2);
				//nk_button_label(ctx, "Commit");
				//nk_button_label(ctx, "Check for update");

				//ctx->style.window.fixed_background = nk_style_item_color(nk_rgb(16, 16, 16));

				nk_layout_row_dynamic(ctx, st.screeny - 240, 1);

				if (nk_group_begin(ctx, "Settings", NK_WINDOW_TITLE | NK_WINDOW_BORDER))
				{
					nk_layout_row_dynamic(ctx, 20, 1);
					nk_label(ctx, "Resolution", NK_TEXT_ALIGN_LEFT);
					//nk_spacing(ctx, 1);
					
					SDL_DisplayMode mode;
					int8 index, num_modes;

					num_modes = SDL_GetNumVideoDisplays();
					
					if (nk_combo_begin_label(ctx, StringFormat("%dx%d", msdk.app[0].w, msdk.app[0].h), nk_vec2(nk_widget_width(ctx),
						35 + (num_modes * 30))))
					{
						for (index = 0; index < num_modes; index++)
						{
							if (SDL_GetVideoDisplay(index, 0, &mode) != 0)
							{
								if (nk_combo_item_label(ctx, StringFormat("%dx%d", mode.w, mode.h), NK_TEXT_ALIGN_LEFT))
								{
									msdk.app[0].w = mode.w;
									msdk.app[0].h = mode.h;
								}
							}
						}
					}

					nk_layout_row_dynamic(ctx, 20, 2);
					nk_label(ctx, "Width", NK_TEXT_ALIGN_LEFT);
					nk_label(ctx, "Height", NK_TEXT_ALIGN_LEFT);

					char buft[4];
					nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, buft, 4, nk_filter_decimal);
					msdk.app[0].w = atoi(buft);

					nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, buft, 4, nk_filter_decimal);
					msdk.app[0].h = atoi(buft);
					
					nk_layout_row_dynamic(ctx, 20, 1);
					//map combo
					nk_layout_row_dynamic(ctx, 20, 2);
					
					
					msdk.app[0].skip_menu = nk_check_label(ctx, "Skip menu", msdk.app[0].skip_menu == 1);
					


					nk_group_end(ctx);
				}

				//SetThemeBack();

				nk_group_end(ctx);
			}

			

			/*
			if (nk_group_begin(ctx, "To do list", NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
			{
				ctx->style.window.fixed_background = nk_style_item_color(nk_rgb(16, 16, 16));
				nk_layout_row_dynamic(ctx, st.screeny - 280, 1);
				if (nk_group_begin(ctx, "List", NULL))
				{
					for (i = 0; i < msdk.prj.TDList_entries; i++)
					{
						if (msdk.prj.TDList[i].creator == msdk.prj.user_id ||
							(msdk.prj.TDList[i].assigned_ids & (int)pow(2, msdk.prj.TDList[i].creator)) == (int)pow(2, msdk.prj.TDList[i].creator))
						{
							switch (msdk.prj.TDList[i].type)
							{
							case 0:
								nk_layout_row_begin(ctx, NK_DYNAMIC, (strlen(msdk.prj.TDList[i].entry) / 64) * 20, 2);
								nk_layout_row_push(ctx, 0.10f);
								msdk.prj.TDList[i].completed = nk_check_label(ctx, " ", msdk.prj.TDList[i].completed == 1);
								nk_layout_row_push(ctx, 0.90f);
								//temp = nk_select_text(ctx, msdk.prj.TDList[i].entry, strlen(msdk.prj.TDList[i].entry), NK_TEXT_ALIGN_LEFT, selected_entry == i);
								nk_text_wrap(ctx, msdk.prj.TDList[i].entry, 512);
								nk_layout_row_end(ctx);

								//if (temp == 1)
									//selected_entry = i;

								nk_layout_row_dynamic(ctx, 15, 1);
								nk_style_set_font(ctx, &fonts[2]->handle);
								nk_label(ctx, StringFormat("Creator: %s", msdk.prj.users[msdk.prj.TDList[i].creator - 1]), NK_TEXT_ALIGN_RIGHT);
								nk_style_set_font(ctx, &fonts[0]->handle);
								break;

							case 1:
								nk_layout_row_dynamic(ctx, strlen(msdk.prj.TDList[i].entry) / 32, 1);

								break;
							}
						}
					}

					nk_group_end(ctx);
				}

				SetThemeBack();

				nk_layout_space_begin(ctx, NK_DYNAMIC, 100, 4);
				nk_layout_space_push(ctx, nk_rect(0.7f, 0.0f, 0.2f, 0.20f));
				tdtype = nk_combo_string(ctx, "Check\0List\0", tdtype, 2, 20, nk_vec2(100, 80));

				nk_layout_space_push(ctx, nk_rect(0.01f, 0.0f, 0.69f, 1.0f));
				nk_edit_string_zero_terminated(ctx, NK_EDIT_BOX, entry, tdtype == 0 ? 512 : 128, nk_filter_default);

				nk_layout_space_push(ctx, nk_rect(0.7f, 0.20f, 0.2f, 0.20f));
				if (nk_combo_begin_label(ctx, "IDs", nk_vec2(200, 35 + (20 * msdk.prj.num_users))))
				{
					nk_layout_row_dynamic(ctx, 20, 1);
					for (i = 0; i < msdk.prj.num_users; i++)
					{
						tmp = (int)pow(2, i);
						temp = nk_check_label(ctx, msdk.prj.users[i], (assigned_ids & tmp) == tmp);

						if (temp == 1)
							assigned_ids |= tmp;
						else
							(assigned_ids & tmp) == tmp ? assigned_ids -= tmp : NULL;
					}

					nk_combo_end(ctx);
				}

				nk_layout_space_push(ctx, nk_rect(0.90f, 0.0f, 0.10f, 0.40f));
				if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT))
				{
					if (msdk.prj.TDList_entries == 0)
						alloc_mem(msdk.prj.TDList, sizeof(ToDo) * 1);

					if (msdk.prj.TDList_entries > 0)
						msdk.prj.TDList = realloc(msdk.prj.TDList, sizeof(ToDo) * (msdk.prj.TDList_entries + 1));

					strcpy(msdk.prj.TDList[msdk.prj.TDList_entries].entry, entry);
					msdk.prj.TDList[msdk.prj.TDList_entries].creator = msdk.prj.user_id + 1;
					msdk.prj.TDList[msdk.prj.TDList_entries].type = tdtype;
					msdk.prj.TDList[msdk.prj.TDList_entries].assigned_ids = assigned_ids;
					msdk.prj.TDList[msdk.prj.TDList_entries].completed = 0;

					msdk.prj.TDList_entries++;
				}

				nk_layout_space_end(ctx);

				nk_group_end(ctx);
			}
			
			*/
			
			

			//nk_layout_row_dynamic(ctx, st.screeny - 240, 1);
			nk_select_label(ctx, "No image available", NK_TEXT_ALIGN_CENTERED, 1);
		}
			
			
			//nk_spacing(ctx, 1);

		//}

		nk_end(ctx);

		if (state == 1)
		{
			if (MGVCompiler())
				state = 0;
		}
	//}
}

int main(int argc, char *argv[])
{
	char str[64];
	int loops;

	struct nk_color background;

	if(LoadCFG()==0)
		if(MessageBox(NULL,"Error while trying to read or write the configuration file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();

	strcpy(st.LogName, "msdk.log");

	Init();

	curl_global_init(CURL_GLOBAL_ALL);

	strcpy(st.WindowTitle,"mSDK");

	//OpenFont("font/Roboto-Regular.ttf","arial",0,128);
	//OpenFont("font/Roboto-Bold.ttf","arial bold",1,128);
	//OpenFont("font//tt0524m_.ttf","geometry",2,128);

	InitMGG();

	CheckFFmpegConversionFolder();
	/*
	if(LoadMGG(&mgg_sys[0],"data/mEngUI.mgg")==NULL)
	{
		LogApp("Could not open UI mgg");
		Quit();
	}
	*/
	UILoadSystem("UI_Sys.cfg");

	st.FPSYes=1;

	st.Developer_Mode=1;

	curr_tic=GetTicks();

	ctx = nk_sdl_init(wn);

	struct nk_font_atlas *atlas;
	nk_sdl_font_stash_begin(&atlas);
	fonts[0] = nk_font_atlas_add_from_file(atlas, "Font\\ProggyClean.ttf", 13, 0);
	fonts[1] = nk_font_atlas_add_from_file(atlas, "Font\\mUI.ttf", 18, 0);
	fonts[2] = nk_font_atlas_add_from_file(atlas, "Font\\ProggyClean.ttf", 11, 0);
	nk_sdl_font_stash_end();
	nk_style_set_font(ctx, &fonts[0]->handle);
	background = nk_rgb(28, 48, 62);

	SETENGINEPATH;

	memset(&msdk, 0, sizeof(mSdk));

	switch (CheckForFFmpeg())
	{
	case -1:
		msdk.ffmpeg_installed = msdk.ffmpeg_downloaded = 0;
		break;

	case 0:
		msdk.ffmpeg_installed = 0;
		msdk.ffmpeg_downloaded = 1;
		break;

	case 1:
		msdk.ffmpeg_installed = msdk.ffmpeg_downloaded = 1;
		break;
	}

	//InitGWEN();
	SetSkin(ctx, msdk.theme);

	register_cipher(&twofish_desc);
	register_cipher(&aes_desc);
	register_prng(&yarrow_desc);

	register_hash(&sha256_desc);
	register_hash(&sha512_desc);

	GetCurrentDirectory(MAX_PATH, msdk.program_path);

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

		if (msdk.prj.loaded == 1)
			Pannel();
		
		//ExportProject();

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