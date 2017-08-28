#ifndef _UI_MGEAR_HEADER
#define _UI_MGEAR_HEADER

#include "engine.h"

#define UI_NULLOP -1
#define UI_OK 0
#define UI_YES 1
#define UI_NO 2
#define UI_DONE 3
#define UI_CANCEL 4
#define UI_CLOSE 5
#define UI_SEL 100

//Basic used colors (mEngineer mainly)

#define UI_COL_SELECTED 0xFF8020
#define UI_COL_NORMAL 0xFFFFFF
#define UI_COL_CLICKED 0xFF2020

#define UI_COL_BLACK 0x000000
#define UI_COL_RED 0xFF0000
#define UI_COL_GREEN 0x00FF00
#define UI_COL_BLUE 0x0000FF

#define MAX_UIWINDOWS 4

#define UI_BASICPANNELFRAME 4 //equivalent to mgg_sys[0].frames[4], used in the Window type 2

enum _UI_POS
{
	CENTER,
	CUSTOM
};

typedef enum _UI_POS UI_POS;

struct _UI_WINDOW
{
	uint8 stat;
	Pos pos;
	Pos size;
	int8 layer;
	uint8 num_options;
	int8 font;
	int16 font_size;
	int8 current;

	int8 window_frame;
};

typedef struct _UI_WINDOW UI_WINDOW;

struct _UI_SYSTEM
{
	int8 mgg_id;

	int8 window2_frame;

	int8 window_frame0;
	int8 window_frame1;
	int8 window_frame2;

	int8 button_frame0;
	int8 button_frame1;
	int8 button_frame2;

	int8 tab_frame;
	int8 close_frame;

	int8 subwindow_frame0;
	int8 subwindow_frame1;
	int8 subwindow_frame2;

	int8 scroll_frame0;
	int8 scroll_frame1;
	int8 scroll_up_frame;
	int8 scroll_down_frame;

	int8 submenu_frame0;
	int8 submenu_frame1;

	int8 save_icon;
	int8 newfile_icon;
	int8 folder_icon;
	int8 delete_icon;
	int8 open_icon;
	int8 close_icon;

	int8 play_icon;
	int8 stop_icon;
	int8 pause_icon;
	int8 foward_icon;
	int8 rewind_icon;
	int8 next_icon;
	int8 back_icon;

	int8 mgg_id2; //for usermade UI frames and icons
	int8 *frames;
	int8 num_usermade;

	int8 resize_cursor;

	uint8 mouse_flag;

	char current_path[2048];

	int32 mouse_scroll;

	int16 current_option;

	char file_name[64];

	int8 sys_freeze;

	int8 textinput;

	char files[512][512];

	int16 filesp[256];
	int16 foldersp[256];
	int16 num_files;
	char extension[32];
	char extension2[16];
};

typedef struct _UI_SYSTEM UI_SYSTEM;

extern UI_SYSTEM UI_Sys;

int16 UIMessageBox(int32 x, int32 y, UI_POS bpos, const char *text, uint8 num_options, uint8 font, size_t font_size, uint32 colorN, uint32 colorS, uint32 colorT);

int16 UIOptionBox(int32 x, int32 y, UI_POS bpos, const char options[8][16], uint8 num_options, uint8 font, size_t font_size, uint32 colorN, uint32 colorS);

int8 UICreateWindow(int32 x, int32 y, int32 xsize, int32 ysize, UI_POS bpos, int8 layer, uint8 window_frame);

int8 UICreateWindow2(int32 x, int32 y, UI_POS bpos, int8 layer, uint8 num_avail_options, int16 font_size, int8 num_charsperoption, int8 font);

void UIDestroyWindow(int8 id);

int8 UIWin2_MarkBox(int8 uiwinid, int8 pos, uint8 marked, char *text, int32 colorN, int32 colorS);

void UIWin2_NumberBoxui8(int8 uiwinid, int8 pos, uint8 *value, char *text, int32 colorN, int32 colorS, int32 colorC);

void UIWin2_NumberBoxi8(int8 uiwinid, int8 pos, int8 *value, char *text, int32 colorN, int32 colorS, int32 colorC);

void UIWin2_NumberBoxui16(int8 uiwinid, int8 pos, uint16 *value, char *text, int32 colorN, int32 colorS, int32 colorC);

void UIWin2_NumberBoxi16(int8 uiwinid, int8 pos, int16 *value, char *text, int32 colorN, int32 colorS, int32 colorC);

void UIWin2_NumberBoxui32(int8 uiwinid, int8 pos, uint32 *value, char *text, int32 colorN, int32 colorS, int32 colorC);

void UIWin2_NumberBoxi32(int8 uiwinid, int8 pos, int32 *value, char *text, int32 colorN, int32 colorS, int32 colorC);

void UIWin2_NumberBoxf(int8 uiwinid, int8 pos, float *value, char *text, int32 colorN, int32 colorS, int32 colorC);

int8 UIWin2_StringButton(int8 uiwinid, int8 pos,char *text, int32 colorN, int32 colorS);

void UIWin2_TextBox(int8 uiwinid, int8 pos, char *text, int32 colorN, int32 colorS, int32 colorC);

void UIMain_DrawSystem();

void UILoadSystem(char *filename);

int8 UIWin_Button(int8 uiwinid, int32 x, int32 y, char *text, uint8 font, uint8 font_size, int32 color, int8 blocked);

int8 UIWin_ButtonIcon(int8 uiwinid, int32 x, int32 y, int32 sizex, int32 sizey, int8 frame, int32 color, int8 blocked);

int8 UIStringButton(int32 x, int32 y,char *text, int8 font, int16 font_size, int8 layer, int32 colorN, int32 colorS);

int8 UIStringButtonWorld(int32 x, int32 y,char *text, int8 font, int16 font_size, int8 layer, int32 colorN, int32 colorS);

int8 Sys_ResizeController(int32 x, int32 y, int32 *sizex, int32 *sizey, uint8 keepaspect, int16 ang, int8 z);

void Sys_ColorPicker(uint8 *r, uint8 *g, uint8 *b);

void UITextBox(int32 x, int32 y, int32 sizex, char *text, int8 font, int16 font_size, int32 colorN, int32 colorS, int32 colorC, int8 layer, int16 option_number);

void SetDirContent(const char *extension);

int8 UISelectFile(char *filename);

int8 UISavePath(char *filename);

int16 UIMakeList(char list[128][128], int16 sizel);

int8 UIButton(int32 x, int32 y, char *text, int8 font, int16 font_size, int8 layer, uint8 select_mode);

#endif