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
struct nk_font *fonts[2];

int prev_tic, curr_tic, delta;

mSdk msdk;

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

int16 DirFiles(const char *path, char content[512][MAX_PATH])
{
	DIR *dir;
	dirent *ent;
	uint16 i=0;
	int16 filenum=0;

	if((dir=opendir(path))!=NULL)
	{
		while((ent=readdir(dir))!=NULL)
		{
			if (content != NULL)
				strcpy(content[i],ent->d_name);

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

		if (msdk.ffmpeg_downloaded == 1 || state == 1 || msdk.ffmpeg_installed == 1)
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
						MessageBox(NULL, "FFmpeg was installed successfully", NULL, MB_OK);

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

void NewProject()
{
	if (nk_begin(ctx, "New project", nk_rect((st.screenx / 2), (st.screeny / 2), 512, 512), NK_WINDOW_TITLE | NK_WINDOW_BORDER))
	{

	}

	nk_end(ctx);
}

int MGVCompiler()
{
	static char file[MAX_PATH], file2[MAX_PATH];
	char str[MAX_PATH];
	static int state = 0;
	int temp;

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

	if (state == 0)
	{
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

		state = 1;
	}

	if (nk_begin(ctx, "MGV Encoder", nk_rect((st.screenx / 2), (st.screeny / 2), 256, 256), NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE))
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

		if (nk_button_label(ctx, "Encode"))
		{
			if (file2)
			{
				SetCurrentDirectory(st.CurrPath);
				system("md Tools\\FFmpeg\\MGV");
				sprintf(str, "Tools\\FFmpeg\\ffmpeg.exe -i \"%s\" -r 30 Tools\\FFmpeg\\MGV\\frame%%05d.jpg",file);
				system(str);
				
				sprintf(str, "Tools\\FFmpeg\\ffmpeg.exe -i \"%s\" Tools\\FFmpeg\\MGV\\MGV.wav", file);
				system(str);

				system("copy Tools\\FFmpeg\\MGV\\frame00001.jpg Tools\\FFmpeg\\MGV\\frame00000.jpg");

				temp = DirFiles("Tools\\FFmpeg\\MGV", NULL);

				sprintf(str, "Tools\\mgvcreator.exe -o test.mgv -p Tools\\FFmpeg\\MGV -fps 30 -n %d", temp - 4);

				system(str);

				system("del Tools\\FFmpeg\\MGV\\*.* /q");
				strcpy(str, st.CurrPath);
				strcat(str, "\\Tools\\FFmpeg");
				SetCurrentDirectory(str);

				system("rd MGV /s /q");
				
				SetCurrentDirectory(st.CurrPath);
			}
		}
	}

	nk_end(ctx);

	return 0;
}

void MenuBar()
{
	register int i, a, m;
	char mapname[2048], str[128], filename[2048];
	int id = 0, id2 = 0, check;
	static int state = 0, mggid;

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
					state = 1;

				if (nk_menu_item_label(ctx, "Open project", NK_TEXT_LEFT))
				{
					state = 2;
				}

				nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "Save as...", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "Import", NK_TEXT_LEFT);
				nk_menu_item_label(ctx, "Export", NK_TEXT_LEFT);
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

		if (state == 10)
		{
			if (Preferences())
				state = 0;
		}
	//}
}

void Pannel()
{
	register int i, j, k;
	static int sl, pannel_state = 0;
	static struct nk_color editcolor = { 255, 255, 255, 255 };
	static char strbuf[32];
	static struct nk_rect bounds;
	TEX_DATA data;
	int temp, px, py, sx, sy;
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
				info.lpDirectory = msdk.prj.prj_path;
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
				info.lpDirectory = msdk.prj.prj_path;
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
				info.lpDirectory = msdk.prj.prj_path;
				info.nShow = SW_SHOW;

				if (!ShellExecuteEx(&info))
					MessageBox(NULL, "Could not execute mTex", "Error", MB_OK);
			}

			nk_spacing(ctx, 1);
			
			nk_layout_row_dynamic(ctx, st.screeny - 100, 3);

			if (nk_group_begin(ctx, "Settings", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
			{
				nk_layout_row_dynamic(ctx, 25, 1);
				nk_button_label(ctx, "Play");
				nk_button_label(ctx, "Debug (MGL only)");

				nk_layout_row_dynamic(ctx, 25, 2);
				nk_button_label(ctx, "Commit");
				nk_button_label(ctx, "Check for update");

				ctx->style.window.fixed_background = nk_style_item_color(nk_rgb(16, 16, 16));

				nk_layout_row_dynamic(ctx, st.screeny - 240, 1);

				if (nk_group_begin(ctx, "Revisions", NK_WINDOW_TITLE | NK_WINDOW_BORDER))
				{
					if (msdk.prj.log)
						nk_label_wrap(ctx, msdk.prj.log);

					nk_group_end(ctx);
				}

				SetThemeBack();

				nk_group_end(ctx);
			}

			if (nk_group_begin(ctx, "To do list", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
			{
				

				nk_group_end(ctx);
			}

			nk_select_label(ctx, "No image available", NK_TEXT_ALIGN_CENTERED, 1);
			
			
			//nk_spacing(ctx, 1);

		}

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

		//if (msdk.prj.loaded == 1)
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