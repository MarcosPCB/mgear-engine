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

_MGGFORMAT tmgg;

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

int FileBrowser(char *filename)
{
	char path[1024];

	register int i, j;

	static int select = -1, time = 0, len, doubleclick = -1, doubleclick2 = -1, backpath_available = 0, fowardpath_available = 0, lenpath;

	static char backpath[2048], fowardpath[2048], temp[2048];

	size_t lenght;

	FILE *f;
	DIR *d;

	if (nk_begin(ctx, "File Browser", nk_rect(st.screenx/2 - 400, st.screeny/2 - 225, 800, 450), NK_WINDOW_BORDER | NK_WINDOW_CLOSABLE | NK_WINDOW_MOVABLE))
	{
		nk_layout_space_begin(ctx, NK_DYNAMIC, 390, INT_MAX);

		nk_layout_space_push(ctx, nk_rect(0.01, 0.01, 0.08, 0.08));
		if (nk_button_label(ctx, "<"))
		{
			if (backpath_available)
			{
				strcpy(temp, UI_Sys.current_path);

				if ((d = opendir(backpath)) != NULL)
				{
					strcpy(fowardpath, UI_Sys.current_path);
					strcpy(UI_Sys.current_path, backpath);
					backpath_available = 0;
					fowardpath_available = 1;
					strcpy(backpath, temp);
					closedir(d);
					SetDirContent(UI_Sys.extension);
					doubleclick = -1;
					doubleclick2 = -1;
					select = -1;
					time = 0;
				}
			}
		}

		nk_layout_space_push(ctx, nk_rect(0.09, 0.01, 0.08, 0.08));
		if(nk_button_label(ctx, ">"))
		{
			if (fowardpath_available)
			{
				strcpy(temp, UI_Sys.current_path);

				if ((d = opendir(fowardpath)) != NULL)
				{
					strcpy(backpath, UI_Sys.current_path);
					strcpy(UI_Sys.current_path, fowardpath);
					backpath_available = 1;
					fowardpath_available = 0;
					strcpy(fowardpath, temp);
					closedir(d);
					SetDirContent(UI_Sys.extension);
					doubleclick = -1;
					doubleclick2 = -1;
					select = -1;
					time = 0;
				}
			}
		}

		nk_layout_space_push(ctx, nk_rect(0.18, 0.01, 0.08, 0.08));
		if (nk_button_label(ctx, "^"))
		{
			strcpy(temp, UI_Sys.current_path);

			lenght = strlen(temp);

			for (i = lenght; i > 0; i--)
			{
				if (temp[i] != 92 && temp[i] != 47)
					temp[i] = 0;
				
				if (temp[i] == 92 || temp[i] == 47)
				{
					temp[i] = 0;
					break;
				}
			}

			if ((d = opendir(temp)) != NULL)
			{
				strcpy(backpath, UI_Sys.current_path);
				strcpy(UI_Sys.current_path, temp);
				backpath_available = 1;
				fowardpath_available = 0;
				closedir(d);
				SetDirContent(UI_Sys.extension);
				doubleclick = -1;
				doubleclick2 = -1;
				select = -1;
				time = 0;
			}
		}

		nk_layout_space_push(ctx, nk_rect(0.27, 0.01, 0.20, 0.08));
		nk_button_image_label(ctx, nk_image_id(mgg_sys[UI_Sys.mgg_id].frames[UI_Sys.folder_icon].data), "New Folder", NK_TEXT_CENTERED);

		nk_layout_space_push(ctx, nk_rect(0.01, 0.10, 0.99, 0.08));

		lenpath = strlen(UI_Sys.current_path);

		nk_edit_string(ctx, NK_EDIT_FIELD, UI_Sys.current_path, &lenpath, 2048, nk_filter_default);

		nk_layout_space_push(ctx, nk_rect(0.01, 0.20, 0.99, 0.70));

		if (nk_group_begin(ctx, "content", NK_WINDOW_BORDER))
		{
			nk_layout_row_dynamic(ctx, 25, 1);

			if (UI_Sys.num_files > 2)
			{
				for (i = 2, j = 0; i < UI_Sys.num_files; i++)
				{
					if (select == i)
					{
						if (UI_Sys.filesp[i] == UI_Sys.foldersp[i])
							nk_select_image_label(ctx, nk_image_id(mgg_sys[UI_Sys.mgg_id].frames[UI_Sys.folder_icon].data), UI_Sys.files[UI_Sys.foldersp[i]], NK_TEXT_RIGHT, 1);
						/*
						{
						if (doubleclick2 == i && (GetTimerM() - time) > 50)
						{
						strcpy(temp, UI_Sys.current_path);
						strcat(temp, "\\");
						strcat(temp, UI_Sys.files[UI_Sys.foldersp[i]]);

						if ((d = opendir(temp)) != NULL)
						{
						strcpy(backpath, UI_Sys.current_path);
						strcpy(UI_Sys.current_path, temp);
						backpath_available = 1;
						closedir(d);
						SetDirContent(UI_Sys.extension);
						doubleclick = -1;
						doubleclick2 = -1;
						select = -1;
						time = 0;
						break;
						}
						}
						else
						{
						doubleclick2 = i;
						time = GetTimerM();
						}
						}
						*/
						else
							nk_select_label(ctx, UI_Sys.files[UI_Sys.filesp[i]], NK_TEXT_RIGHT, 1);
						/*
						{
							if (doubleclick2 == i && (GetTimerM() - time) > 50)
							{
								strcpy(temp, UI_Sys.current_path);
								strcat(temp, "\\");
								strcat(temp, UI_Sys.files[UI_Sys.filesp[i]]);

								if ((f = fopen(temp, "rb")) == NULL)
								{
									fclose(f);
									return temp;
								}
							}
							else
								doubleclick2 = i;
						}
						*/
					}
					else
					{
						if (UI_Sys.filesp[i] == UI_Sys.foldersp[i])
						{
							if (nk_select_image_label(ctx, nk_image_id(mgg_sys[UI_Sys.mgg_id].frames[UI_Sys.folder_icon].data), UI_Sys.files[UI_Sys.foldersp[i]], NK_TEXT_RIGHT, 0))
							{
								select = i;
								doubleclick = i;
								time = GetTimerM();
							}
						}
						else
						{
							if (nk_select_label(ctx, UI_Sys.files[UI_Sys.filesp[i]], NK_TEXT_RIGHT, 0))
								select = i;
						}
					}
				}

				nk_group_end(ctx);
			}
			else
				nk_spacing(ctx, 1);
		}

		nk_layout_space_push(ctx, nk_rect(0.80, 0.92, 0.10, 0.08));
		if (nk_button_label(ctx, "Open"))
		{
			if (UI_Sys.filesp[select] == UI_Sys.foldersp[select])
			{
				strcpy(temp, UI_Sys.current_path);
				strcat(temp, "\\");
				strcat(temp, UI_Sys.files[UI_Sys.foldersp[select]]);

				if ((d = opendir(temp)) != NULL)
				{
					strcpy(backpath, UI_Sys.current_path);
					strcpy(UI_Sys.current_path, temp);
					backpath_available = 1;
					closedir(d);
					SetDirContent(UI_Sys.extension);
					doubleclick = -1;
					doubleclick2 = -1;
					select = -1;
					time = 0;
				}
			}
			else
			{
				strcpy(temp, UI_Sys.current_path);
				strcat(temp, "\\");
				strcat(temp, UI_Sys.files[UI_Sys.filesp[select]]);

				if ((f = fopen(temp, "rb")) != NULL)
				{
					fclose(f);
					nk_end(ctx);
					strcpy(filename, temp);
					return 1;
				}
			}
		}

		nk_layout_space_push(ctx, nk_rect(0.90, 0.92, 0.10, 0.08));
		if (nk_button_label(ctx, "Cancel"))
		{
			nk_end(ctx);
			return -1;
		}

	}

	nk_end(ctx);

	return 0;
}

void NewMGGBox(const char path[MAX_PATH])
{
	register int i, j, k;
	static char path2[MAX_PATH], files[MAX_PATH * 64] = { 0 }, tex[512][MAX_PATH], tex_n[512][MAX_PATH], path3[MAX_PATH], str[32], tex_names[512][32], tex_n_names[512][32];
	static int len, state = 0, num_files_t = 0, num_files_n = 0;
	int len2;
	char *tok = NULL;

	OPENFILENAME ofn;
	ZeroMemory(&files, sizeof(files));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	ofn.lpstrFilter = "Image Files\0*.png;*.jpg;*.tga;*.bmp\0Any File\0*.*\0";
	ofn.lpstrFile = files;
	ofn.nMaxFile = MAX_PATH*2;
	ofn.lpstrTitle = "Select textures to import";
	//strcpy(ofn.lpstrInitialDir, path);
	ofn.lpstrInitialDir = path;

	BROWSEINFO bi;

	ZeroMemory(&bi, sizeof(bi));

	static LPITEMIDLIST pidl;

	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT;

	if (!state)
	{
		strcpy(path2, path);
		len = strlen(path2);
		
		state = 1;
	}

	if (nk_begin(ctx, "Create new MGG project", nk_rect(st.screenx / 2 - 256, st.screeny / 2 - 256, 512, 512), NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 2);

		nk_layout_row_push(ctx, 0.85f);
		nk_edit_string(ctx, NK_EDIT_SIMPLE, path2, &len, MAX_PATH, nk_filter_default);

		nk_layout_row_push(ctx, 0.15f);
		if (nk_button_label(ctx, "Browse"))
		{
			bi.lpszTitle = ("Select your project folder");

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
		nk_spacing(ctx, 1);

		nk_layout_row_dynamic(ctx, 30, 2);
		if (nk_button_label(ctx, "Import textures"))
		{
			if (GetOpenFileName(&ofn))
			{
				state = 1;
				i = 0;
				while (state)
				{
					if (i == 0)
					{
						tok = strtok(files, " ");
						strcpy(path3, tok);
					}
					else
					{
						tok = strtok(NULL, " ");

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

								num_files_t++;
							}
							state = 0;
							i = 0;
							break;
						}
						else
						{
							strcpy(tex[num_files_t], path3);
							//strcat(tex[num_files_t], "\\");
							strcat(tex[num_files_t], tok);
							strcpy(tex_names[num_files_t], tok);
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
						tok = strtok(files, " ");
						strcpy(path3, tok);
					}
					else
					{
						tok = strtok(NULL, " ");

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

								num_files_n++;
							}
							state = 0;
							i = 0;
							break;
						}
						else
						{
							strcpy(tex_n[num_files_n], path3);
							strcat(tex_n[num_files_n], tok);
							strcpy(tex_n_names[num_files_n], tok);
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

				nk_layout_row_push(ctx, 0.70f);
				nk_label(ctx, tex_names[i], NK_TEXT_ALIGN_LEFT);

				nk_layout_row_push(ctx, 0.1f);
				nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_UP);

				nk_layout_row_push(ctx, 0.1f);
				nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_DOWN);
				nk_layout_row_end(ctx);
			}

			nk_group_end(ctx);
		}

		if (nk_group_begin(ctx, "Selected normal maps", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
		{
			for (i = 0; i < num_files_n; i++)
			{
				nk_layout_row_begin(ctx, NK_DYNAMIC, 15, 4);
				nk_layout_row_push(ctx, 0.1f);
				sprintf(str, "%d", i);
				nk_label(ctx, str, NK_TEXT_ALIGN_LEFT);

				nk_layout_row_push(ctx, 0.70f);
				nk_label(ctx, tex_n_names[i], NK_TEXT_ALIGN_LEFT);

				nk_layout_row_push(ctx, 0.1f);
				nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_UP);

				nk_layout_row_push(ctx, 0.1f);
				nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_DOWN);
				nk_layout_row_end(ctx);
			}

			nk_group_end(ctx);
		}

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "Texture compression", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 15, 2);

		mtex.RLE = nk_option_label(ctx, "None", mtex.RLE == 0) ? 0 : mtex.RLE;
		mtex.RLE = nk_option_label(ctx, "RLE (faster)", mtex.RLE == 1) ? 1 : mtex.RLE;

		nk_layout_row_dynamic(ctx, 15, 1);
		nk_label(ctx, "Texture mipmap (filter)", NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 15, 2);

		tmgg.mipmap = nk_option_label(ctx, "Nearest", tmgg.mipmap == 1) ? 1 : tmgg.mipmap;
		tmgg.mipmap = nk_option_label(ctx, "Linear", tmgg.mipmap == 0) ? 0 : tmgg.mipmap;

		nk_layout_row_dynamic(ctx, 25, 6);
		nk_spacing(ctx, 4);

		nk_button_label(ctx, "Create");
		nk_button_label(ctx, "Cancel");


	}

	nk_end(ctx);
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
				{
					state = 1;
				}

				if (nk_menu_item_label(ctx, "Open MGG txt file", NK_TEXT_LEFT))
				{
					//SetDirContent("mgm");
					state = 2;
				}

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
			bi.lpszTitle = ("Select your project folder");

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
				
				NewMGGBox(path);
			}

			//state = 0;
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
	ctx->style.window.fixed_background = nk_style_item_color(nk_rgb(16, 16, 16));

	if (nk_begin(ctx, "Canvas", nk_rect(0.10f * st.screenx, 30, 0.70f * st.screenx, st.screeny), NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{

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
	int loops;

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
		LeftPannel();
		Canvas();
		AnimBox();

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