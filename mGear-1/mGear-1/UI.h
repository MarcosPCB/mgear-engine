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
};

typedef struct _UI_WINDOW UI_WINDOW;

struct _UI_SYSTEM
{
	uint8 mgg_id;

	uint8 window2_frame;

	uint8 window_frame0;
	uint8 window_frame1;
	uint8 window_frame2;
	uint8 button_frame0;
	uint8 button_frame1;
	uint8 button_frame2;
	uint8 tab_frame;
	uint8 close_frame;
	uint8 subwindow_frame0;
	uint8 subwindow_frame1;
	uint8 subwindow_frame2;
};

typedef struct _UI_SYSTEM UI_SYSTEM;

extern UI_SYSTEM UI_Sys;

int16 UIMessageBox(int32 x, int32 y, UI_POS bpos, const char *text, uint8 num_options, uint8 font, size_t font_size, uint32 colorN, uint32 colorS, uint32 colorT);

int16 UIOptionBox(int32 x, int32 y, UI_POS bpos, const char options[8][16], uint8 num_options, uint8 font, size_t font_size, uint32 colorN, uint32 colorS);

int8 UICreateWindow(int32 x, int32 y, int32 xsize, int32 ysize, UI_POS bpos, int8 layer);

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

void UIMain_DrawSystem();

void UILoadSystem(char *filename);

#endif