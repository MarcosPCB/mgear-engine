#include <Windows.h>
#include <commdlg.h>
#include <ShlObj.h>
#include "input.h"
#include "main.h"
#include <stdio.h> 
#include <stdlib.h>
#include <math.h>
//#include <atlstr.h>
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

mTex mtex;

uint16 WriteCFG()
{
	FILE *file;

	if((file=fopen("settings.cfg","w"))==NULL)
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
	if((file=fopen("settings.cfg","r"))==NULL)
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

char *CheckForNormal(char *filename)
{
	register int i, j;
	int len;
	char normal[MAX_PATH], str[8];
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

char *CopyThisFile(char *filepath, char *newpath)
{
	FILE *f, *f2;
	size_t size;
	int len, i;
	char newfile[MAX_PATH], *data;

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

	strcpy(newfile, newpath);

	strcat(newfile, "\\");

	strcat(newfile, filepath + i);

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
	char str[2048];
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

	if ((f = fopen(filepath, "w")) == NULL)
	{
		LogApp("Error: Could not create file %s", filepath);
		return NULL;
	}

	fprintf(f, "MGGNAME %s\n", mtex.mgg.name);
	fprintf(f, "FRAMES %d\n", mtex.mgg.num_frames);
	fprintf(f, "ANIMS %d\n", mtex.mgg.num_anims);
	
	if (mtex.mgg.RLE)
		fprintf(f, "RLE\n");

	if (mtex.mgg.mipmap)
		fprintf(f, "MIPMAP\n");

	if (mtex.mgg.num_c_atlas > 0)
	{
		for (i = 0; i < mtex.mgg.num_c_atlas; i++)
		{
			fprintf(f, "FRAMENAMES_CUSTOM_ATLAS %d ", i);
			for (j = 0; j < mtex.mgg.num_frames; j++)
			{
				if (mtex.mgg.frames_atlas[j] != -1)
				{
					fprintf(f, "\"%s\"", mtex.mgg.texnames[j]);
				}

				if (j == mtex.mgg.num_frames - 1)
					fprintf(f, "\n");
				else
					fprintf(f, ", ");
			}
		}
	}

	if (mtex.mgg.num_c_n_atlas > 0)
	{
		for (i = 0; i < mtex.mgg.num_c_n_atlas; i++)
		{
			fprintf(f, "FRAMENAMES_CUSTOM_ATLAS_NORMALS %d ", i);
			for (j = 0; j < mtex.mgg.num_f_n; j++)
			{
				if (mtex.mgg.frames_atlas[j] != -1)
				{
					fprintf(f, "\"%s\"", mtex.mgg.texnames_n[j]);
				}

				if (j == mtex.mgg.num_f_n - 1)
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
			fprintf(f, "\"%s\"", mtex.mgg.texnames[i]);

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
				fprintf(f, "FRAMEFILE %d %s\n", i, mtex.mgg.texnames[i]);
		}
	}

	if (mtex.mgg.num_c_n_atlas == 0)
	{
		fprintf(f, "FRAMESFILES_NORMALS 0 %d ", mtex.mgg.num_f_n);

		for (i = 0; i < mtex.mgg.num_f_n; i++)
		{
			fprintf(f, "\"%s\"", mtex.mgg.texnames_n[i]);

			if (i == mtex.mgg.num_f_n - 1)
				fprintf(f, "\n");
			else
				fprintf(f, ", ");
		}
	}
	else
	{
		for (i = 0; i < mtex.mgg.num_f_n; i++)
		{
			if (mtex.mgg.frames_atlas[i] == -1)
				fprintf(f, "FRAMEFILE_NORMAL %d %s\n", i, mtex.mgg.texnames_n[i]);
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
			fprintf(f, "FRAMEOFFSET %d %d %d\n", i, mtex.mgg.frameoffset_x[i], mtex.mgg.frameoffset_y[i]);
	}

	fprintf(f, "\0\0");

	fclose(f);

	return 1;
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

int NewMGGBox(const char path[MAX_PATH])
{
	register int i, j, k;
	static char path2[MAX_PATH], files[MAX_PATH + (32 * 512)], tex[512][MAX_PATH], tex_n[512][MAX_PATH], path3[MAX_PATH], str[32], tex_names[512][32], tex_n_names[512][32],
		prj_name[32] = { 0 }, mgg_name[32] = { 0 }, command[MAX_PATH + 32];
	static int len, state = 0, num_files_t = 0, num_files_n = 0, len_prj_n = 0, len_in_n = 0, fls[512], flsn[512];
	int len2;
	char *tok = NULL, strerror[1024];

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
							strcat(tex[num_files_t], "\\");
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

				nk_style_default(ctx);

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

				nk_style_default(ctx);

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
				mtex.textures = malloc(mtex.mgg.num_frames * sizeof(int16));
				mtex.textures_n = malloc(mtex.mgg.num_f_n * sizeof(int16));

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

						if (tok)
						{
							mtex.mgg.fn[j] = j;
							strcpy(mtex.mgg.files[j], tok);
							strcpy(mtex.mgg.texnames[j], tex_names[fls[i]]);

							if ((mtex.textures[j] = LoadTexture(mtex.mgg.files[j], mtex.mgg.mipmap)) == -1)
							{
								sprintf(strerror, "Error: Texture %s could not be loaded", mtex.mgg.files[j]);
								MessageBox(NULL, strerror, "Error", MB_OK);
							}

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

						if (tok)
						{
							mtex.mgg.fnn[j] = j;
							strcpy(mtex.mgg.files_n[j], tok);
							strcpy(mtex.mgg.texnames_n[j], tex_n_names[flsn[i]]);

							if ((mtex.textures_n[j] = LoadTexture(mtex.mgg.files_n[j], mtex.mgg.mipmap)) == -1)
							{
								sprintf(strerror, "Error: Texture %s could not be loaded", mtex.mgg.files_n[j]);
								MessageBox(NULL, strerror, "Error", MB_OK);
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

void MenuBar()
{
	register int i, a, m;
	static char str[128], path[MAX_PATH];
	int id = 0, id2 = 0, check;
	static int state = 0, mggid;

	TCHAR filename[MAX_PATH];

	BROWSEINFO bi;

	ZeroMemory(&bi, sizeof(bi));

	static LPITEMIDLIST pidl;

	if (nkrendered==0)
	{
		if (nk_begin(ctx, "Menu", nk_rect(0, 0, st.screenx, 30), NK_WINDOW_NO_SCROLLBAR))
		{
			nk_menubar_begin(ctx);
			nk_layout_row_begin(ctx, NK_STATIC, 25, 5);

			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(210, 210)))
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				if (nk_menu_item_label(ctx, "New MGG", NK_TEXT_LEFT))
					state = 1;

				if (nk_menu_item_label(ctx, "Open MGG txt file", NK_TEXT_LEFT))
					state = 2;

				nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "Save as...", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "Compile MGG", NK_TEXT_LEFT);
				if (nk_menu_item_label(ctx, "Exit", NK_TEXT_LEFT)) st.quit = 1;
				nk_menu_end(ctx);
			}

			nk_layout_row_push(ctx, 45);
			if (nk_menu_begin_label(ctx, "Edit", NK_TEXT_LEFT, nk_vec2(120, 200)))
			{
				nk_layout_row_dynamic(ctx, 30, 1);
				nk_menu_item_label(ctx, "Preferences", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "MGG properties", NK_TEXT_LEFT);
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
				memset(&tmgg, 0, sizeof(_MGGFORMAT));
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

void LeftPannel()
{
	register int i, j, k;
	static int sl, pannel_state = 0;
	static struct nk_color editcolor = { 255, 255, 255, 255 };
	static char strbuf[32];
	static struct nk_rect bounds;
	TEX_DATA data;
	int temp, px, py, sx, sy;
	struct nk_image texid;

	if (nkrendered==0)
	{
		if (nk_begin(ctx, "Left Pannel", nk_rect(0, 30, st.screenx * 0.10f, st.screeny), NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER))
		{
			nk_layout_row_dynamic(ctx, 30, 1);

			nk_button_label(ctx, "Test");

			//nk_spacing(ctx, 1);

		}

		nk_end(ctx);
	}
}

void Canvas()
{
	register int i, j, k;
	char str[128];
	static int option = 0;
	ctx->style.window.fixed_background = nk_style_item_color(nk_rgb(16, 16, 16));

	if (nk_begin(ctx, mtex.mgg.name, nk_rect(0.10f * st.screenx, 30, 0.70f * st.screenx, st.screeny), NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		nk_layout_row_dynamic(ctx, 25, 1);

		if (option == mtex.mgg.num_c_atlas)
			sprintf(str, "Single textures");
		else
			sprintf(str, "Atlas %d", option + 1);

		if (nk_combo_begin_label(ctx, str, nk_vec2(nk_widget_width(ctx), 30 * mtex.mgg.num_c_atlas + 1)))
		{
			for (i = 0; i < mtex.mgg.num_c_atlas + 1; i++)
			{
				nk_layout_row_dynamic(ctx, 25, 1);
				if (i == mtex.mgg.num_c_atlas)
				{
					if (nk_combo_item_label(ctx, "Single textures", NK_TEXT_ALIGN_LEFT))
						option = i;
				}
				else
				{
					sprintf(str, "Atlas %d", i + 1);
					if (nk_combo_item_label(ctx, str, NK_TEXT_ALIGN_LEFT))
						option = i;
				}
			}
		}

		nk_layout_row_dynamic(ctx, (0.70f * st.screenx) / 8.0f, 8);

		if (option == mtex.mgg.num_c_atlas)
		{
			for (i = 0, j = 0; i < mtex.mgg.num_frames; i++)
			{
				if (mtex.mgg.fn[i] < 1024)
				{
					if (j == 8)
					{
						nk_layout_row_dynamic(ctx, 20, 8);

						for (k = 0; k < 8; k++)
						{
							if (mtex.mgg.fn[i - (8 - k)] < 1024)
							{
								sprintf(str, "%d - %s", i - (8 - k), mtex.mgg.texnames[i - (8 - k)]);
								nk_label(ctx, str, NK_TEXT_ALIGN_CENTERED);
							}
						}

						j = 0;

						nk_layout_row_dynamic(ctx, (0.70f * st.screenx) / 8.0f, 8);
					}

					mtex.selection[i] = nk_select_image_label(ctx, nk_image_id(mtex.textures[i]), " ", NK_TEXT_ALIGN_MIDDLE, mtex.selection[i] == 1);

					if (j < 8)
						j++;
				}
			}

			if (j > 0)
			{
				nk_layout_row_dynamic(ctx, 20, 8);

				for (k = 0; k < j; k++)
				{
					if (mtex.mgg.fn[i - (j - k)] < 1024)
					{
						sprintf(str, "%d - %s", i - (j - k), mtex.mgg.texnames[i - (j - k)]);
						nk_label(ctx, str, NK_TEXT_ALIGN_CENTERED);
					}
				}

				j = 0;

			}
		}
	}

	nk_end(ctx);

	nk_style_default(ctx);
}

void AnimBox()
{
	if (nk_begin(ctx, "Animation Box", nk_rect(0.80f * st.screenx, 30, 0.20f * st.screenx, 0.5f * st.screeny), NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{

	}

	nk_end(ctx);
}

int main(int argc, char *argv[])
{
	char str[64];
	int loops, i;

	struct nk_color background;

	if(LoadCFG()==0)
		if(MessageBox(NULL,L"Error while trying to read or write the configuration file",NULL,MB_OK | MB_ICONERROR)==IDOK) 
			Quit();

	Init();

	strcpy(st.WindowTitle,"Tex ALPHA");

	OpenFont("font/Roboto-Regular.ttf","arial",0,128);
	OpenFont("font/Roboto-Bold.ttf","arial bold",1,128);
	//OpenFont("font//tt0524m_.ttf","geometry",2,128);

	InitMGG();
	
	if(LoadMGG(&mgg_sys[0],"data/mEngUI.mgg")==NULL)
	{
		LogApp("Could not open UI mgg");
		Quit();
	}
	
	UILoadSystem("UI_Sys.cfg");

	st.FPSYes=1;

	st.Developer_Mode=1;

	curr_tic=GetTicks();

	ctx = nk_sdl_init(wn);

	struct nk_font_atlas *atlas;
	nk_sdl_font_stash_begin(&atlas);
	nk_sdl_font_stash_end();
	background = nk_rgb(28, 48, 62);

	SETENGINEPATH;

	memset(&mtex, 0, sizeof(mTex));

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

		MenuBar();
		if (mtex.mgg.num_frames > 0)
		{
			LeftPannel();
			Canvas();
			AnimBox();
		}

		UIMain_DrawSystem();
		//MainSound();
		Renderer(0);

		float bg[4];
		nk_color_fv(bg, background);

		nk_sdl_render(NK_ANTI_ALIASING_OFF, 512 * 1024, 128 * 1024);

		SwapBuffer(wn);

		nkrendered = 0;
	}

	if (mtex.mgg.fn)
		free(mtex.mgg.fn);

	if (mtex.mgg.fnn)
		free(mtex.mgg.fnn);

	if (mtex.mgg.frames_atlas)
		free(mtex.mgg.frames_atlas);

	if (mtex.mgg.mga)
		free(mtex.mgg.mga);

	if (mtex.mgg.num_f_a)
		free(mtex.mgg.num_f_a);

	if (mtex.textures)
	{
		glDeleteTextures(mtex.mgg.num_frames, mtex.textures);
		free(mtex.textures);
	}

	if (mtex.textures_n)
	{
		glDeleteTextures(mtex.mgg.num_f_n, mtex.textures_n);
		free(mtex.textures_n);
	}

	Quit();
	return 1;
}