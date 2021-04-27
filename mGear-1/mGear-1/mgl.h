#ifndef _MGL_H
#define _MGL_H

#include "engine.h"
#include "funcs.h"

#define PushStack(x) { stack[sp] = x; sp++; }
#define PopStack(x) { x = stack[sp - 1]; sp--; }

#define GetVarAddress(x) { v[2] = (buf[x + 1] << 24) | (buf[x + 2] << 16) | (buf[x + 3] << 8) | buf[x + 4]; if (buf[x] == 1) v[2] += bp; }
#define GetValueCV(x, loc) { x = (buf[loc] << 24) | (buf[loc + 1] << 16) | (buf[loc + 2] << 8) | buf[loc + 3]; }
#define GetValueStack(x, loc) { x = stack[loc]; }
#define CodeToStack(loc, x) { stack[loc] = (buf[x] << 24) | (buf[x + 1] << 16) | (buf[x + 2] << 8) | buf[x + 3]; }
#define StackToStack(loc, loc2) stack[loc] = stack[loc2];
#define SetStack(x, loc) stack[loc] = x;

///These are the switch cases for the registers

#define GetRegSwitch(x) v[7] = v[x];
#define SetRegSwitch(x) v[x] = v[7];

#define GetRegfSwitch(x) f[4] = f[x];
#define SetRegfSwitch(x) f[x] = f[4];

//Instructions variant opcodes
#define vopREGCONST 0
#define vopREGMEM 1
#define vopREGREG 2
#define vopMEMCONST 3
#define vopMEMREG 4
#define vopMEMMEM 5

//Instruction opcodes
#define opSET 0
#define opADD 10
#define opSUB 20
#define opMUL 30
#define opDIV 40
#define opPOW 50
#define opLOG 60
#define opSQRT 70
#define opCOS 76
#define opSIN 82
#define opTAN 88
#define opACOS 94
#define opASIN 100
#define opATAN 106
#define opAND 112
#define opOR 122
#define opXOR 132
#define opIFGE 142
#define opIFLE 152
#define opIFG 162
#define opIFL 172
#define opIFE 182
#define opIFNE 192
#define opWHILE 202
#define opLOOP 203
#define opCALL 204
#define opPUSHC 205
#define opPUSHM 206
#define opPUSHR 207
#define opPUSHRF 208
#define opRET 210
#define opSHIFTL 211
#define opSHIFTR 221

#define opFTI 250
#define opITF 251
#define opPOPR 252
#define opPOPRF 253
#define opDATA 254
#define opEXTOP 255

enum enginecalls
{
	E_LOG = 0,
	E_DRAWLINE = 1,
	E_MSGBOX = 2,
	E_PLAYMOV = 3,
	E_PLAYBGMOV = 4,
	C_GSYSSCREEN = 5,
	C_GSYSMOUSEPOS = 6,
	C_GSYSMOUSELMB = 7,
	C_GSYSMOUSERMB = 8,
	C_GSYSMOUSEMMB = 9,
	C_GSYSKEY = 10,
	E_STW = 11,
	E_WTS = 12,
	E_STWCI = 13,
	E_WTSCI = 14,
	E_STWF = 15,
	E_WTSF = 16,
	C_GSYSNUMMGGS = 17,
	C_GSYSMOUSEWHEEL = 18,
	C_GSYSGAMESTATE = 19,
	E_DRAWUI = 20,
	E_DRAWUISTRING = 21,
	E_DRAWUISTRINGL = 22,
	C_GMGGTEX = 23,
	E_OPENFILE = 24,
	E_CLOSEFILE = 25,
	E_GETSTRINGFILE = 26,
	E_CHECKEOFFILE = 27,
	E_WRITEFILE = 28,
	E_READFILE = 29,
	E_REWINDFILE = 30,
	E_SEEKFILE = 31,
	E_CURBYTEFILE = 32,
	E_PRINT = 33,
	E_SCAN = 34,
	E_STRINGSCAN = 35,
	C_GSPRFRAME = 50,
	C_GSPRNUMFRAMES = 51,
	C_GSPRANIM = 52,
	C_GSPRNUMANIMS = 53
};

struct MGLEng_Funcs
{
	char name[32];
	enum enginecalls func;
	uint8 num_args;
	uint8 returnv;
};

typedef struct MGLEng_Funcs eng_calls;

//MGL compiling structs
struct _MGMC
{
	unsigned char **function_code;

	uint32 **bt_trl; //Translation

	uint16 num_functions;
	uint16 num_vars;
	uint32 lines;

	struct FuncTab
	{
		char name[64];
		uint32 size;
		uint16 num_use;
		uint8 num_args;

		int32 address;
		uint32 line;

		uint16 num_vars;

		struct VarTab
		{
			char name[64];
			uint8 type; //0 - constant, 1 - int, 2 - float, 3 - buffer, 4 - string (+10 if its an array)
			uint16 num_use;
			uint8 stat;
			int32 value;
			float valuef;
			char *string;
			size_t len;
			uint32 line;
		} *vars_table;

	} *function_table;

	struct VarTab2
	{
		char name[64];
		uint8 type; //0 - constant, 1 - int, 2 - float, 3 - buffer, 4 - string (+10 if its an array)
		uint16 num_use;
		uint8 stat;
		int32 value;
		float valuef;
		char *string;
		size_t len;
		uint32 line;
	} *vars_table;

	struct LineCode
	{
		int16 func;
		char code[256];
		uint32 byte_start;
		uint32 byte_end;
	} *linecode;

	uint32 stacksize; //0 - 2k, 1 - 4k, 2 - 16k, 3 - 48k, 4 - 64k, 5 - 128k, 6 - 512k, 7 - custom
};

typedef struct _MGMC MGMC;


int8 BuildMGL(const char *filename, const char *finalname);

int8 InitMGLCode(const char *file);
int8 ExecuteMGLCode(uint8 location);
void CleanupHeap(struct MGLHeap *heap, uint16 num_heap);

#endif