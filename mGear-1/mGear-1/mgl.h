#ifndef _MGL_H
#define _MGL_H

#include "engine.h"
#include "funcs.h"

#define PushStack(x) { stack[sp] = x; sp++; }
#define PopStack(x) { x = stack[sp]; sp--; }

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

enum enginecalls
{
	E_LOG = 0,
	E_DRAWLINE = 1,
	E_MSGBOX = 2,
};

struct MGLEng_Funcs
{
	char name[32];
	enum enginecalls func;
	uint8 num_args;
};

typedef struct MGLEng_Funcs eng_calls;

//MGL code
struct MGLCode
{
	unsigned char *code;
	size_t size;
	uint16 cv;
	uint32 v[8];
	uint32 bp, sp, stack_type, memsize;
	struct Funcs
	{
		void(*log)(const char *msg, ...);
		int32(*msgbox)(const char *quote, UINT type, const char *msg, ...);
		void(*drawline)(int32 x, int32 y, int32 x2, int32 y2, uint8 r, uint8 g, uint8 b, uint8 a, int16 linewidth, int32 z);
	} funcs;
	uint32 ret_addr;
};

//MGL compiling structs

struct _MGMC
{
	unsigned char **function_code;

	uint32 **bt_trl; //Translation

	uint16 num_functions;
	uint16 num_vars;

	struct FuncTab
	{
		char name[64];
		uint32 size;
		uint16 num_use;
		uint8 num_args;

		int32 address;

		uint16 num_vars;
		struct VarTab
		{
			char name[64];
			uint8 type; //0 - constant, 1 - int, 2 - float, 3 - buffer, 4 - string (+10 if its local)
			uint16 num_use;
			uint8 stat;
			int32 value;
			float valuef;
			char *string;
			size_t len;
		} *vars_table;

	} *function_table;

	struct VarTab
	{
		char name[64];
		uint8 type; //0 - constant, 1 - int, 2 - float, 3 - buffer, 4 - string (+10 if its local)
		uint16 num_use;
		uint8 stat;
		int32 value;
		float valuef;
		char *string;
		size_t len;
	} *vars_table;
};

typedef struct _MGMC MGMC;


#endif