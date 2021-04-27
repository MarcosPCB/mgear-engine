#include "mgl.h" 
#include <stdio.h>

eng_calls e_funcs[] =
{
	{ "log", E_LOG, 2, 0 }, //0
	{ "drawline", E_DRAWLINE, 10, 0 }, //1
	{ "msgbox", E_MSGBOX, 4, 0 }, //2
	{ "playmovie", E_PLAYMOV, 1, 1 }, //3
	{ "playbgmovie", E_PLAYBGMOV, 2, 1 }, //4
	{ "getsys.Screen", C_GSYSSCREEN, 4, 1 }, //5
	{ "getsys.mouse.pos", C_GSYSMOUSEPOS, 2, 1 }, //6
	{ "getsys.mouse.lmb", C_GSYSMOUSELMB, 1, 1 }, //7
	{ "getsys.mouse.rmb", C_GSYSMOUSERMB, 1, 1 }, //8
	{ "getsys.mouse.mmb", C_GSYSMOUSEMMB, 1, 1 }, //9
	{ "getsys.key", C_GSYSKEY, 2, 1 }, //10
	{ "screentoworld", E_STW, 2, 1 }, //11
	{ "worldtoscreen", E_WTS, 2, 1 }, //12
	{ "screentoworld_nocamera", E_STWCI, 2, 1 }, //13
	{ "worldtoscreen_nocamera", E_WTSCI, 2, 1 }, //14
	{ "screentoworld_float", E_STWCI, 2, 1 }, //15
	{ "worldtoscreen_float", E_WTSCI, 2, 1 }, //16
	{ "getsys.num_mggs", C_GSYSNUMMGGS, 1, 1 }, //17
	{ "getsys.mouse.wheel", C_GSYSMOUSEWHEEL, 1, 1 }, //18
	{ "getsys.game_state", C_GSYSGAMESTATE, 1, 1 }, //19
	{ "drawui", E_DRAWUI, 10, 0 }, //20
	{ "drawuistring", E_DRAWUISTRING, 14, 0 }, //21
	{ "drawuistring_left", E_DRAWUISTRINGL, 14, 0 }, //22
	{ "getmgg.tex", C_GMGGTEX, 2, 0 }, //23
	{ "open_file", E_OPENFILE, 3, 1 }, //24
	{ "close_file", E_CLOSEFILE, 1, 0 }, //25
	{ "getstring_file", E_GETSTRINGFILE, 3, 1 }, //26
	{ "checkendoffile_file", E_CHECKEOFFILE, 1, 0 }, //27
	{ "write_file", E_WRITEFILE, 2, 0 }, //28
	{ "read_file", E_READFILE, 2, 1 }, //29
	{ "rewind_file", E_REWINDFILE, 1, 0 }, //30
	{ "seek_file", E_SEEKFILE, 3, 0 }, //31
	{ "currentbyte_file", E_CURBYTEFILE, 1, 0 }, //32
	{ "print", E_PRINT, 1, 1 }, //33
	{ "scan", E_SCAN, 1, 1 }, //34
	{ "string_scan", E_STRINGSCAN, 2, 1 }, //35
	{ "getsprite.Frame", C_GSPRFRAME, 1, 1}, //32
	{ "getsprite.NumFrames", C_GSPRNUMFRAMES, 1, 1}, //33
	{ "getsprite.Anim", C_GSPRANIM, 1, 1}, //34
	{ "getsprite.NumAnim", C_GSPRNUMANIMS, 1, 1} //35

};

const uint16 num_efuncs = 128;

int16 GetEngFunc(const char *name)
{
	register int16 i, j;

	mem_assert(name);

	if (name == NULL)
		return -1;

	for (i = 0; i < num_efuncs; i++)
	{
		if (strcmp(name, e_funcs[i].name) == NULL)
		{
			j = 1;
			break;
		}
	}

	if (j == 1)
		return i;
	else
		return -1;
}

//Copy float value to 32-bits memory in unsigned char* format and turn into big endian
int8 CopyFloatToInt32(unsigned char *mem, float valf)
{
	mem_assert(mem);

	if (mem == NULL)
		return NULL;

	memcpy(mem, &valf, sizeof(float));

	/*
	uint8 x = mem[0];
	uint8 y = mem[1];

	mem[0] = mem[3];
	mem[1] = mem[2];
	mem[2] = y;
	mem[3] = x;
	*/

	return 1;
}

int32 GetValueBuf(unsigned char *mem)
{
	mem_assert(mem);

	if (mem == NULL)
		return NULL;

	return (mem[0] << 24) | (mem[1] << 16) | (mem[2] << 8) | mem[3];
}

int8 Copy32toBuf(unsigned char *mem, int32 val)
{
	mem_assert(mem);

	if (mem == NULL)
		return NULL;

	mem[0] = val >> 24;
	mem[1] = (val >> 16) & 0xFF;
	mem[2] = (val >> 8) & 0xFF;
	mem[3] = val & 0xFF;

	return 1;
}

void CheckCodeSize(MGMC *code, uint32 curfunc, uint32 cur)
{
	mem_assert(code);

	if (cur > code->function_table[curfunc].size - 64)
	{
		code->function_table[curfunc].size += 256;
		code->function_code[curfunc] = realloc(code->function_code[curfunc], code->function_table[curfunc].size);

		code->bt_trl[curfunc] = realloc(code->bt_trl[curfunc], code->function_table[curfunc].size * sizeof(uint32));

		CHECKMEM(code->function_code[curfunc]);
		CHECKMEM(code->bt_trl[curfunc]);
	}
}

int16 GetVarOrNumber(char *tok, MGMC *code, int32 curfunc, int32 *var, int32 *number, float *numberf)
{
	int32 val1, valf1, g = 0, f = 0, i = 0;

	if (tok != NULL)
	{
		if (IsNumber(tok) == 0)
		{
			if (tok[0] == 'a' && tok[1] == 'r' && tok[2] == 'g' && ((tok[3] >= '0' && tok[3] <= '9') || (tok[3] == 'f' && (tok[4] >= '0' && tok[4] <= '9'))))
			{
				if (strlen(tok) == 4)
					val1 = tok[3] - 48 + 4;

				if (strlen(tok) == 5 && tok[3] != 'f')
					val1 = atoi(tok + 3) + 4;

				if (strlen(tok) == 5 && tok[3] == 'f')
					val1 = tok[4] - 48 + 4;

				if (strlen(tok) == 6 && tok[3] == 'f')
					val1 = atoi(tok + 4) + 4;

				g = 3;
			}
			else
			if (strcmp(tok, "return") == NULL)
			{
				val1 = 0;
				g = 3;
			}
			else
			{
				//val1 = -1;
				for (i = 0; i < code->num_vars; i++)
				{
					if (strcmp(code->vars_table[i].name, tok) == NULL)
					{
						//Found global variable in the table
						code->vars_table[i].num_use++;

						if (code->vars_table[i].type == 2)
							f |= 1;

						val1 = i;
						g = 2;
						break;
					}
				}

				for (i = 0; i < code->function_table[curfunc].num_vars; i++)
				{
					if (strcmp(code->function_table[curfunc].vars_table[i].name, tok) == NULL)
					{
						//Found local variable in the table
						code->function_table[curfunc].vars_table[i].num_use++;

						if (code->function_table[curfunc].vars_table[i].type == 2)
							f |= 1;

						val1 = i;
						g = 1;
						break;
					}
				}
			}

			if (g == 0)
				return 0;

			*var = val1;

			if (g == 2)
				return 1;

			if (g == 1)
				return 2;

			if (g == 3)
				return 5;
		}
		else
		{
			if (IsNumber(tok))
			{
				*number = atol(tok);
				return 3;
			}
			else
			if (IsNumberFloat(tok))
			{
				*numberf = atof(tok);
				return 4;
			}
		}
	}
	else
		return -2;
}

uint8 DetectArgument(char *string)
{
	static char *buf = NULL;
	static int16 c = 0;
	uint8 fa = 0;

	if (string != NULL)
	{
		mem_assert(string);

		buf = string;
		c = 0;
	}

	mem_assert(buf);

	while (buf[c] != '\0')
	{
		if (fa != 0)
			break;

		if (buf[c] == '%' && buf[c + 1] != '%')
		{
			if (c > 0)
			{
				if (buf[c - 1] == '%')
					continue;
			}

			switch (buf[c + 1])
			{
				case 's':
					fa = 1;
					break;

				case 'd':
					fa = 2;
					break;

				case 'l':
					fa = 3;
					break;

				case 'c':
					fa = 4;
					break;

				case 'f':
					fa = 5;
					break;
			}
		}

		c++;
	}

	return fa;
}

void RemoveSlashes(char *string)
{
	mem_assert(string);

	CHECKMEM(string);

	register int i = 0;

	while (string[i] != '\0')
	{
		if (string[i] == '\\')
		{
			switch (string[i + 1])
			{
				case 'n':
					string[i] = '\n';
					string[i + 1] = '\b';
					break;

				case 't':
					string[i] = '\t';
					string[i + 1] = '\b';
					break;
			}
		}

		i++;
	}
}

MGMC *CompileMGL(FILE *file, uint8 optimization)
{
	MGMC *code;
	char buf[4096], *tok, str1[64], str2[64];
	uint16 func = 0, error = 0, line = 0, curfunc = 0;
	int32 val1, val2, i, cv = 0, cv1 = 0, cur_line = 64;
	uint32 brackets = 0, expect_bracket = 0, ret = 0, ifcmp = 0;
	int32 ifcmpaddr[64];
	float valf1, valf2;

	code = calloc(1, sizeof(MGMC));
	CHECKMEM(code);

	code->linecode = malloc(sizeof(*code->linecode) * 64);

	while (!feof(file))
	{
		if (line > cur_line)
			code->linecode = realloc(code->linecode, sizeof(*code->linecode) * (cur_line + 64));

		line++;

		ZeroMem(code->linecode[line].code, 256);
		code->linecode[line].byte_start = 0;
		code->linecode[line].byte_end = 0;
		code->linecode[line].func = -1;

		ZeroMem(buf, 4096);
		fgets(buf, 4096, file);

		if (buf[0] == '\0' || buf[0] == '\n')
			continue;

		tok = strtok(buf, " \n\t");

		if (tok == NULL)
			continue;

		if (tok[0] == '/' && tok[1] == '/')
			continue;

		if (ret == 1)
			ret++;
		else
		if (ret == 2)
			ret = 0;

		//Variable declarations
		if (strcmp(tok, "MEM") == NULL)
		{
			tok = strtok(NULL, " \n\t");

			if (tok == NULL)
			{
				error++;
				LogApp("Compiler error: missing stack memory type - line: %d", line);
				continue;
			}

			if (IsNumber(tok) == 0)
			{
				error++;
				LogApp("Compiler error: undefined stack memory type - line: %d", line);
				continue;
			}

			val1 = atoi(tok);

			if (val1 < 0)
			{
				error++;
				LogApp("Compiler error: undefined stack memory type - line: %d", line);
				continue;
			}

			switch (val1)
			{
				case 0:
					code->stacksize = 2048;
					break;

				case 1:
					code->stacksize = 4096;
					break;

				case 2:
					code->stacksize = 16384;
					break;

				case 3:
					code->stacksize = 49152;
					break;

				case 4:
					code->stacksize = 65536;
					break;

				case 5:
					code->stacksize = 131072;
					break;

				case 6:
					code->stacksize = 524288;
					break;

				default:
					code->stacksize = val1;
					break;
			}
		}
		else
		if (strcmp(tok, "var") == NULL)
		{
			tok = strtok(NULL, " \n\t");

			if (tok == NULL)
			{
				error++;
				LogApp("Compiler error: missing variable name in declaration - line: %d", line);
				continue;
			}

			int8 arraytype = 0;

			strcpy(str1, tok);

			tok = strtok(NULL, " \n\t");

			if (tok != NULL)
			{
				if (IsNumber(tok) == 0)
				{
					if (strcmp(tok, "array") == 0)
					{
						arraytype = 1;

						tok = strtok(NULL, " \n\t");

						if (tok != NULL)
						{
							if (IsNumber(tok) == 0)
							{
								error++;
								LogApp("Compiler error: invalid array size in declaration - line: %d - size: \"%s\"", line, tok);
								continue;
							}
							else
								val1 = atol(tok);
						}
						else
						{
							error++;
							LogApp("Compiler error: missing array size in declaration - line: %d - size: \"%s\"", line, tok);
							continue;
						}
					}
					else
					{
						error++;
						LogApp("Compiler error: invalid variable value in declaration - line: %d - value: \"%s\"", line, tok);
						continue;
					}
				}
				else
					val1 = atol(tok);
			}
			else
				val1 = 0;

			if (func == 0)
			{
				//Check if there are any global variables with the same name
				for (i = 0; i < code->num_vars; i++)
				{
					if (strcmp(str1, code->vars_table[i].name) == NULL)
					{
						error++;
						LogApp("Compiler error: variable \"%s\" already defined globally - line: %d", str1, line);
						i = -1;
						break;
					}
				}

				if (i == -1)
					continue;

				code->num_vars++;
				if (code->num_vars == 1)
					code->vars_table = calloc(1, sizeof(*code->vars_table));
				else
					code->vars_table = realloc(code->vars_table, code->num_vars * sizeof(*code->vars_table));

				CHECKMEM(code->vars_table);

				i = code->num_vars - 1;

				code->vars_table[i].num_use = 0;
				strcpy(code->vars_table[i].name, str1);

				code->vars_table[i].value = val1;

				code->vars_table[i].type = 1;

				if (arraytype == 1)
					code->vars_table[i].type += 10;

				code->vars_table[i].stat = 0;
			}
			else
			{
				//Check if there are any global variables with the same name
				for (i = 0; i < code->num_vars; i++)
				{
					if (strcmp(str1, code->vars_table[i].name) == NULL)
					{
						error++;
						LogApp("Compiler error: variable \"%s\" already defined globally - line: %d", str1, line);
						i = -1;
						break;
					}
				}

				if (i == -1)
					continue;

				for (i = 0; i < code->function_table[curfunc].num_vars; i++)
				{
					if (strcmp(str1, code->function_table[curfunc].vars_table[i].name) == NULL)
					{
						error++;
						LogApp("Compiler error: variable \"%s\" already defined locally in function \"%s\" - line: %d", str1, code->function_table[curfunc].name, line);
						i = -1;
						break;
					}
				}

				if (i == -1)
					continue;

				code->function_table[curfunc].num_vars++;
				if (code->function_table[curfunc].num_vars == 1)
					code->function_table[curfunc].vars_table = calloc(1, sizeof(*code->function_table[curfunc].vars_table));
				else
					code->function_table[curfunc].vars_table = realloc(code->function_table[curfunc].vars_table,
					code->function_table[curfunc].num_vars * sizeof(*code->function_table[curfunc].vars_table));

				CHECKMEM(code->function_table[curfunc].vars_table);

				i = code->function_table[curfunc].num_vars - 1;

				code->function_table[curfunc].vars_table[i].num_use = 0;
				strcpy(code->function_table[curfunc].vars_table[i].name, str1);

				code->function_table[curfunc].vars_table[i].value = val1;

				code->function_table[curfunc].vars_table[i].type = 1;

				if (arraytype == 1)
					code->function_table[curfunc].vars_table[i].type += 10;

				code->function_table[curfunc].vars_table[i].stat = 0;
			}
		}
		else
		if (strcmp(tok, "float") == NULL)
		{
			tok = strtok(NULL, " \n\t");

			if (tok == NULL)
			{
				error++;
				LogApp("Compiler error: missing variable name in declaration - line: %d", line);
				continue;
			}

			strcpy(str1, tok);

			tok = strtok(NULL, " \n\t");

			if (tok != NULL)
			{
				if (IsNumber(tok) == 0)
				{
					error++;
					LogApp("Compiler error: invalid variable value in declaration - line: %d - value: \"%s\"", line, tok);
					continue;
				}

				valf1 = atof(tok);
			}
			else
				valf1 = 0;

			if (func == 0)
			{
				//Check if there are any global variables with the same name
				for (i = 0; i < code->num_vars; i++)
				{
					if (strcmp(str1, code->vars_table[i].name) == NULL)
					{
						error++;
						LogApp("Compiler error: variable \"%s\" already defined globally - line: %d", str1, line);
						i = -1;
						break;
					}
				}

				if (i == -1)
					continue;

				code->num_vars++;
				if (code->num_vars == 1)
					code->vars_table = calloc(1, sizeof(*code->vars_table));
				else
					code->vars_table = realloc(code->vars_table, code->num_vars * sizeof(*code->vars_table));

				CHECKMEM(code->vars_table);

				i = code->num_vars - 1;

				code->vars_table[i].type = func == 1 ? 1 : 0; //Global or local

				code->vars_table[i].num_use = 0;
				strcpy(code->vars_table[i].name, str1);

				code->vars_table[i].valuef = valf1;

				code->vars_table[i].type = 2;
				code->vars_table[i].stat = 0;
			}
			else
			{
				//Check if there are any global variables with the same name
				for (i = 0; i < code->num_vars; i++)
				{
					if (strcmp(str1, code->vars_table[i].name) == NULL)
					{
						error++;
						LogApp("Compiler error: variable \"%s\" already defined globally - line: %d", str1, line);
						i = -1;
						break;
					}
				}

				if (i == -1)
					continue;

				for (i = 0; i < code->function_table[curfunc].num_vars; i++)
				{
					if (strcmp(str1, code->function_table[curfunc].vars_table[i].name) == NULL)
					{
						error++;
						LogApp("Compiler error: variable \"%s\" already defined locally in function \"%s\" - line: %d", str1, code->function_table[curfunc].name, line);
						i = -1;
						break;
					}
				}

				if (i == -1)
					continue;

				code->function_table[curfunc].num_vars++;
				if (code->function_table[curfunc].num_vars == 1)
					code->function_table[curfunc].vars_table = calloc(1, sizeof(*code->function_table[curfunc].vars_table));
				else
					code->function_table[curfunc].vars_table = realloc(code->function_table[curfunc].vars_table,
					code->function_table[curfunc].num_vars * sizeof(*code->function_table[curfunc].vars_table));

				CHECKMEM(code->function_table[curfunc].vars_table);

				i = code->function_table[curfunc].num_vars - 1;

				code->function_table[curfunc].vars_table[i].num_use = 0;
				strcpy(code->function_table[curfunc].vars_table[i].name, str1);

				code->function_table[curfunc].vars_table[i].value = valf1;

				code->function_table[curfunc].vars_table[i].type = 2;
				code->function_table[curfunc].vars_table[i].stat = 0;
			}
		}
		else
		if (strcmp(tok, "buffer") == NULL)
		{
			tok = strtok(NULL, " \n\t");

			if (tok == NULL)
			{
				error++;
				LogApp("Compiler error: missing buffer name in declaration - line: %d", line);
				continue;
			}

			strcpy(str1, tok);

			tok = strtok(NULL, " \n\t");

			if (tok != NULL)
			{
				if (IsNumber(tok) == 0)
				{
					error++;
					LogApp("Compiler error: invalid buffer size in declaration - line: %d - size: \"%s\"", line, tok);
					continue;
				}

				val1 = atol(tok);
			}
			else
			{
				error++;
				LogApp("Compiler error: missing buffer size in declaration - line: %d", line);
				continue;
			}

			if (func == 0)
			{
				//Check if there are any global variables with the same name
				for (i = 0; i < code->num_vars; i++)
				{
					if (strcmp(str1, code->vars_table[i].name) == NULL)
					{
						error++;
						LogApp("Compiler error: variable \"%s\" already defined globally - line: %d", str1, line);
						i = -1;
						break;
					}
				}

				if (i == -1)
					continue;

				code->num_vars++;
				if (code->num_vars == 1)
					code->vars_table = calloc(1, sizeof(*code->vars_table));
				else
					code->vars_table = realloc(code->vars_table, code->num_vars * sizeof(*code->vars_table));

				CHECKMEM(code->vars_table);

				i = code->num_vars - 1;

				code->vars_table[i].type = func == 1 ? 1 : 0; //Global or local

				code->vars_table[i].num_use = 0;
				strcpy(code->vars_table[i].name, str1);

				code->vars_table[i].value = val1;

				code->vars_table[i].type = 3;
				code->vars_table[i].stat = 0;
			}
			else
			{
				//Check if there are any global variables with the same name
				for (i = 0; i < code->num_vars; i++)
				{
					if (strcmp(str1, code->vars_table[i].name) == NULL)
					{
						error++;
						LogApp("Compiler error: variable \"%s\" already defined globally - line: %d", str1, line);
						i = -1;
						break;
					}
				}

				if (i == -1)
					continue;

				for (i = 0; i < code->function_table[curfunc].num_vars; i++)
				{
					if (strcmp(str1, code->function_table[curfunc].vars_table[i].name) == NULL)
					{
						error++;
						LogApp("Compiler error: variable \"%s\" already defined locally in function \"%s\" - line: %d", str1, code->function_table[curfunc].name, line);
						i = -1;
						break;
					}
				}

				if (i == -1)
					continue;

				code->function_table[curfunc].num_vars++;
				if (code->function_table[curfunc].num_vars == 1)
					code->function_table[curfunc].vars_table = calloc(1, sizeof(*code->function_table[curfunc].vars_table));
				else
					code->function_table[curfunc].vars_table = realloc(code->function_table[curfunc].vars_table,
					code->function_table[curfunc].num_vars * sizeof(*code->function_table[curfunc].vars_table));

				CHECKMEM(code->function_table[curfunc].vars_table);

				i = code->function_table[curfunc].num_vars - 1;

				code->function_table[curfunc].vars_table[i].num_use = 0;
				strcpy(code->function_table[curfunc].vars_table[i].name, str1);

				code->function_table[curfunc].vars_table[i].value = val1;

				code->function_table[curfunc].vars_table[i].type = 3;
				code->function_table[curfunc].vars_table[i].stat = 0;
			}
		}
		else
		if (strcmp(tok, "string") == NULL)
		{
			tok = strtok(NULL, " \n\t");

			if (tok == NULL)
			{
				error++;
				LogApp("Compiler error: missing buffer name in declaration - line: %d", line);
				continue;
			}

			strcpy(str1, tok);

			tok = strtok(NULL, "\"");

			if (func == 0)
			{
				//Check if there are any global variables with the same name
				for (i = 0; i < code->num_vars; i++)
				{
					if (strcmp(str1, code->vars_table[i].name) == NULL)
					{
						error++;
						LogApp("Compiler error: variable \"%s\" already defined globally - line: %d", str1, line);
						i = -1;
						break;
					}
				}

				if (i == -1)
					continue;

				code->num_vars++;
				if (code->num_vars == 1)
					code->vars_table = calloc(1, sizeof(*code->vars_table));
				else
					code->vars_table = realloc(code->vars_table, code->num_vars * sizeof(*code->vars_table));

				CHECKMEM(code->vars_table);

				i = code->num_vars - 1;

				code->vars_table[i].type = func == 1 ? 1 : 0; //Global or local

				code->vars_table[i].num_use = 0;
				strcpy(code->vars_table[i].name, str1);

				if (tok != NULL)
				{
					RemoveSlashes(tok);

					code->vars_table[i].len = strlen(tok);

					code->vars_table[i].string = calloc(code->vars_table[i].len, 1);
					CHECKMEM(code->vars_table[i].string);

					strcpy(code->vars_table[i].string, tok);
				}
				else
				{
					code->vars_table[i].string = NULL;
					code->vars_table[i].len = 0;
				}

				code->vars_table[i].type = 4;
				code->vars_table[i].stat = 0;
			}
			else
			{
				//Check if there are any global variables with the same name
				for (i = 0; i < code->num_vars; i++)
				{
					if (strcmp(str1, code->vars_table[i].name) == NULL)
					{
						error++;
						LogApp("Compiler error: variable \"%s\" already defined globally - line: %d", str1, line);
						i = -1;
						break;
					}
				}

				if (i == -1)
					continue;

				for (i = 0; i < code->function_table[curfunc].num_vars; i++)
				{
					if (strcmp(str1, code->function_table[curfunc].vars_table[i].name) == NULL)
					{
						error++;
						LogApp("Compiler error: variable \"%s\" already defined locally in function \"%s\" - line: %d", str1, code->function_table[curfunc].name, line);
						i = -1;
						break;
					}
				}

				if (i == -1)
					continue;

				code->function_table[curfunc].num_vars++;
				if (code->function_table[curfunc].num_vars == 1)
					code->function_table[curfunc].vars_table = calloc(1, sizeof(*code->function_table[curfunc].vars_table));
				else
					code->function_table[curfunc].vars_table = realloc(code->function_table[curfunc].vars_table,
					code->function_table[curfunc].num_vars * sizeof(*code->function_table[curfunc].vars_table));

				CHECKMEM(code->function_table[curfunc].vars_table);

				i = code->function_table[curfunc].num_vars - 1;

				code->function_table[curfunc].vars_table[i].num_use = 0;
				strcpy(code->function_table[curfunc].vars_table[i].name, str1);

				if (tok != NULL)
				{
					RemoveSlashes(tok);

					code->function_table[curfunc].vars_table[i].len = strlen(tok);

					code->function_table[curfunc].vars_table[i].string = calloc(code->function_table[curfunc].vars_table[i].len, 1);
					CHECKMEM(code->function_table[curfunc].vars_table[i].string);

					strcpy(code->function_table[curfunc].vars_table[i].string, tok);
				}
				else
				{
					code->function_table[curfunc].vars_table[i].string = NULL;
					code->function_table[curfunc].vars_table[i].len = 0;
				}

				code->function_table[curfunc].vars_table[i].type = 4;
				code->function_table[curfunc].vars_table[i].stat = 0;
			}
		}
		else
		if (strcmp(tok, "cat_string") == NULL)
		{
			if (func == 0 || expect_bracket == 1)
			{
				error++;
				LogApp("Compiler error: detected command outside function - line: %d, command: \"%s\"", line, tok);
				continue;
			}

			CheckCodeSize(code, curfunc, cv);

			int32 g = 0, f = 0, g1 = 0, c = 0, s = 0;

			tok = strtok(NULL, " \n\t");

			if (tok != NULL)
			{
				if (IsNumber(tok) == 0)
				{
					for (i = 0; i < code->num_vars; i++)
					{
						if (strcmp(code->vars_table[i].name, tok) == NULL)
						{
							//Found global variable in the table
							code->vars_table[i].num_use++;

							if (code->vars_table[i].type != 4)
							{
								g = 3;
								error++;
								LogApp("Compiler error: detected variable that is not a string in the first argument - line: %d, variable: \"%s\"",
									line, code->vars_table[i].name);
								break;
							}

							val1 = i;
							g = 2;
							break;
						}
					}

					if (g == 3)
						continue;

					for (i = 0; i < code->function_table[curfunc].num_vars; i++)
					{
						if (strcmp(code->function_table[curfunc].vars_table[i].name, tok) == NULL)
						{
							//Found local variable in the table
							if (g == 2)
							{
								error++;
								LogApp("Compiler error: function \"%s\" local variable \"%s\" conflicting with global variable with the same name in \"%s\" command - line: %d",
									code->function_table[curfunc].name, tok, str1, line);
								g = -1;
								break;
							}

							if (code->function_table[curfunc].vars_table[i].type != 4)
							{
								g = 3;
								error++;
								LogApp("Compiler error: detected variable that is not a string in the first argument - line: %d, variable: \"%s\"",
									line, code->function_table[curfunc].vars_table[i].name);
								break;
							}

							code->function_table[curfunc].vars_table[i].num_use++;

							///if (code->function_table[curfunc].vars_table[i].type == 2)
								//f |= 1;

							val1 = i;
							g = 1;
							break;
						}
					}

					if (g == 3)
						continue;
				}
				else
				{
					error++;
					LogApp("Compiler error: detected constant in the first argument - line: %d", line);
					continue;
				}
			}

			tok = strtok(NULL, " \n\t");

			if (tok != NULL)
			{
				if (IsNumber(tok) == 0)
				{
					if (tok[0] == 'a' && tok[1] == 'r' && tok[2] == 'g' && ((tok[3] >= '0' && tok[3] <= '9') || (tok[3] == 'f' && (tok[4] >= '0' && tok[4] <= '9'))))
					{
						if (strlen(tok) == 4)
							val2 = tok[3] - 48 + 9 - 1;

						if (strlen(tok) == 5 && tok[3] != 'f')
							val2 = atoi(tok + 3) + 9 - 1;

						if (strlen(tok) == 5 && tok[3] == 'f')
							val2 = tok[4] - 48 + 5 - 1;

						if (strlen(tok) == 6 && tok[3] == 'f')
							val2 = atoi(tok + 4) + 5 - 1;

						g = 3;
					}
					else
					if (strcmp(tok, "return") == NULL)
					{
						val2 = 2;
						g1 = 3;
					}
					else
					{
						for (i = 0; i < code->num_vars; i++)
						{
							if (strcmp(code->vars_table[i].name, tok) == NULL)
							{
								//Found global variable in the table
								code->vars_table[i].num_use++;

								if (code->vars_table[i].type == 4)
									s = 1;

								val2 = i;
								g1 = 2;
								break;
							}
						}

						for (i = 0; i < code->function_table[curfunc].num_vars; i++)
						{
							if (strcmp(code->function_table[curfunc].vars_table[i].name, tok) == NULL)
							{
								//Found local variable in the table
								if (g1 == 2)
								{
									error++;
									LogApp("Compiler error: function \"%s\" local variable \"%s\" conflicting with global variable with the same name in \"%s\" command - line: %d",
										code->function_table[curfunc].name, tok, str1, line);
									g1 = -1;
									break;
								}

								if (code->function_table[curfunc].vars_table[i].type == 4)
									s = 1;

								code->function_table[curfunc].vars_table[i].num_use++;

								if (code->function_table[curfunc].vars_table[i].type == 2)
									f |= 1;

								val2 = i;
								g1 = 1;
								break;
							}
						}
					}
				}
				else
				{
					c = 1;

					if (IsNumberFloat(tok) == 0)
						val2 = atol(tok);
					else
					{
						f = 1;
						val2 = atof(tok);
					}
				}
			}

			if (g1 == 2)
				g1 = 0;

			if (g == 2)
				g = 0;

			if (s == 1)
			{
				code->function_code[curfunc][cv] = 242;
				code->function_code[curfunc][cv + 1] = g == 1 ? 1 : 0;
				code->function_code[curfunc][cv + 2] = val1 >> 24;
				code->function_code[curfunc][cv + 3] = (val1 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 4] = (val1 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 5] = val1 & 0xFF;
				code->function_code[curfunc][cv + 6] = g1 == 1 ? 1 : 0;
				code->function_code[curfunc][cv + 7] = val2 >> 24;
				code->function_code[curfunc][cv + 8] = (val2 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 9] = (val2 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 10] = val2 & 0xFF;

				cv += 11;

				code->bt_trl[curfunc][cv1] = 242;

				cv1++;
			}
			else
			if (s == 0 && c == 1 && f == 0 && g1 != 3)
			{
				code->function_code[curfunc][cv] = 0;
				code->function_code[curfunc][cv + 1] = 0;
				code->function_code[curfunc][cv + 2] = val2 >> 24;
				code->function_code[curfunc][cv + 3] = (val2 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 4] = (val2 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 5] = val2 & 0xFF;

				code->function_code[curfunc][cv + 6] = 241;
				code->function_code[curfunc][cv + 7] = g == 1 ? 1 : 0;
				code->function_code[curfunc][cv + 8] = val1 >> 24;
				code->function_code[curfunc][cv + 9] = (val1 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 10] = (val1 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 11] = val1 & 0xFF;
				code->function_code[curfunc][cv + 12] = 0;

				cv += 13;

				code->bt_trl[curfunc][cv1] = 0;
				code->bt_trl[curfunc][cv1 + 1] = 241;

				cv1 += 2;
			}
			else
			if (s == 0 && c == 1 && f == 1 && g1 != 3)
			{
				code->function_code[curfunc][cv] = 255;
				code->function_code[curfunc][cv + 1] = 1;
				code->function_code[curfunc][cv + 2] = 0;
				code->function_code[curfunc][cv + 3] = val2 >> 24;
				code->function_code[curfunc][cv + 4] = (val2 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 5] = (val2 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 6] = val2 & 0xFF;

				code->function_code[curfunc][cv + 7] = 245;
				code->function_code[curfunc][cv + 8] = g == 1 ? 1 : 0;
				code->function_code[curfunc][cv + 9] = val1 >> 24;
				code->function_code[curfunc][cv + 10] = (val1 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 11] = (val1 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 12] = val1 & 0xFF;
				code->function_code[curfunc][cv + 13] = 0;

				cv += 14;

				code->bt_trl[curfunc][cv1] = 255;
				code->bt_trl[curfunc][cv1 + 1] = 1;
				code->bt_trl[curfunc][cv1 + 2] = 245;

				cv1 += 3;
			}
			else
			if (s == 0 && c == 0 && f == 0 && g1 != 3)
			{
				code->function_code[curfunc][cv] = 1;
				code->function_code[curfunc][cv + 1] = 0;
				code->function_code[curfunc][cv + 2] = g1 == 1 ? 1 : 0;
				code->function_code[curfunc][cv + 3] = val2 >> 24;
				code->function_code[curfunc][cv + 4] = (val2 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 5] = (val2 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 6] = val2 & 0xFF;

				code->function_code[curfunc][cv + 7] = 242;
				code->function_code[curfunc][cv + 8] = g == 1 ? 1 : 0;
				code->function_code[curfunc][cv + 9] = val1 >> 24;
				code->function_code[curfunc][cv + 10] = (val1 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 11] = (val1 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 12] = val1 & 0xFF;
				code->function_code[curfunc][cv + 13] = 0;

				cv += 14;

				code->bt_trl[curfunc][cv1] = 1;
				code->bt_trl[curfunc][cv1 + 1] = 242;
				
				cv1 += 2;
			}
			else
			if (s == 0 && c == 0 && f == 1 && g1 != 3)
			{
				code->function_code[curfunc][cv] = 255;
				code->function_code[curfunc][cv + 1] = 4;
				code->function_code[curfunc][cv + 2] = 0;
				code->function_code[curfunc][cv + 3] = g1 == 1 ? 1 : 0;
				code->function_code[curfunc][cv + 4] = val2 >> 24;
				code->function_code[curfunc][cv + 5] = (val2 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 6] = (val2 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 7] = val2 & 0xFF;

				code->function_code[curfunc][cv + 8] = 245;
				code->function_code[curfunc][cv + 9] = g == 1 ? 1 : 0;
				code->function_code[curfunc][cv + 10] = val1 >> 24;
				code->function_code[curfunc][cv + 11] = (val1 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 12] = (val1 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 13] = val1 & 0xFF;
				code->function_code[curfunc][cv + 14] = 0;

				cv += 15;

				code->bt_trl[curfunc][cv1] = 255;
				code->bt_trl[curfunc][cv1 + 1] = 4;
				code->bt_trl[curfunc][cv1 + 2] = 245;

				cv1 += 3;
			}
			else
			if (s == 0 && c == 0 && f == 0 && g1 == 3)
			{
				code->function_code[curfunc][cv] = 242;
				code->function_code[curfunc][cv + 1] = g == 1 ? 1 : 0;
				code->function_code[curfunc][cv + 2] = val1 >> 24;
				code->function_code[curfunc][cv + 3] = (val1 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 4] = (val1 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 5] = val1 & 0xFF;
				code->function_code[curfunc][cv + 6] = val2;

				cv += 6;

				code->bt_trl[curfunc][cv1] = 242;

				cv1 ++;
			}
			else
			if (s == 0 && c == 0 && f == 1 && g1 == 3)
			{
				code->function_code[curfunc][cv] = 245;
				code->function_code[curfunc][cv + 1] = g == 1 ? 1 : 0;
				code->function_code[curfunc][cv + 2] = val1 >> 24;
				code->function_code[curfunc][cv + 3] = (val1 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 4] = (val1 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 5] = val1 & 0xFF;
				code->function_code[curfunc][cv + 6] = val2;

				cv += 6;

				code->bt_trl[curfunc][cv1] = 245;

				cv1++;
			}
		}
		else
		if (strcmp(tok, "compare_string") == NULL)
		{
			if (func == 0 || expect_bracket == 1)
			{
				error++;
				LogApp("Compiler error: detected command outside function - line: %d, command: \"%s\"", line, tok);
				continue;
			}

			CheckCodeSize(code, curfunc, cv);

			int32 g = 0, f = 0, g1 = 0;

			tok = strtok(NULL, " \n\t");

			if (tok != NULL)
			{
				if (IsNumber(tok) == 0)
				{
					for (i = 0; i < code->num_vars; i++)
					{
						if (strcmp(code->vars_table[i].name, tok) == NULL)
						{
							//Found global variable in the table
							code->vars_table[i].num_use++;

							if (code->vars_table[i].type != 4)
							{
								g = 3;
								error++;
								LogApp("Compiler error: detected variable that is not a string in the first argument - line: %d, variable: \"%s\"",
									line, code->vars_table[i].name);
								break;
							}

							val1 = i;
							g = 2;
							break;
						}
					}

					if (g == 3)
						continue;

					for (i = 0; i < code->function_table[curfunc].num_vars; i++)
					{
						if (strcmp(code->function_table[curfunc].vars_table[i].name, tok) == NULL)
						{
							//Found local variable in the table
							if (g == 2)
							{
								error++;
								LogApp("Compiler error: function \"%s\" local variable \"%s\" conflicting with global variable with the same name in \"%s\" command - line: %d",
									code->function_table[curfunc].name, tok, str1, line);
								g = -1;
								break;
							}

							if (code->function_table[curfunc].vars_table[i].type != 4)
							{
								g = 3;
								error++;
								LogApp("Compiler error: detected variable that is not a string in the first argument - line: %d, variable: \"%s\"",
									line, code->function_table[curfunc].vars_table[i].name);
								break;
							}

							code->function_table[curfunc].vars_table[i].num_use++;

							///if (code->function_table[curfunc].vars_table[i].type == 2)
							//f |= 1;

							val1 = i;
							g = 1;
							break;
						}
					}

					if (g == 3)
						continue;
				}
				else
				{
					error++;
					LogApp("Compiler error: detected constant in the first argument - line: %d", line);
					continue;
				}
			}

			tok = strtok(NULL, " \n\t");

			if (tok != NULL)
			{
				if (IsNumber(tok) == 0)
				{
					for (i = 0; i < code->num_vars; i++)
					{
						if (strcmp(code->vars_table[i].name, tok) == NULL)
						{
							//Found global variable in the table
							code->vars_table[i].num_use++;

							if (code->vars_table[i].type != 4)
							{
								g = 3;
								error++;
								LogApp("Compiler error: detected variable that is not a string in the first argument - line: %d, variable: \"%s\"",
									line, code->vars_table[i].name);
								break;
							}

							val2 = i;
							g1 = 2;
							break;
						}
					}

					for (i = 0; i < code->function_table[curfunc].num_vars; i++)
					{
						if (strcmp(code->function_table[curfunc].vars_table[i].name, tok) == NULL)
						{
							//Found local variable in the table
							if (g1 == 2)
							{
								error++;
								LogApp("Compiler error: function \"%s\" local variable \"%s\" conflicting with global variable with the same name in \"%s\" command - line: %d",
									code->function_table[curfunc].name, tok, str1, line);
								g1 = -1;
								break;
							}

							if (code->vars_table[i].type != 4)
							{
								g = 3;
								error++;
								LogApp("Compiler error: detected variable that is not a string in the first argument - line: %d, variable: \"%s\"",
									line, code->vars_table[i].name);
								break;
							}

							code->function_table[curfunc].vars_table[i].num_use++;

							val2 = i;
							g1 = 1;
							break;
						}
					}
				}
				else
				{
					error++;
					LogApp("Compiler error: detected constant in the second argument - line: %d", line);
					continue;
				}
			}

			if (g1 == 2)
				g1 = 0;

			if (g == 2)
				g = 0;

			code->function_code[curfunc][cv] = 243;
			code->function_code[curfunc][cv + 1] = g == 1 ? 1 : 0;
			code->function_code[curfunc][cv + 2] = val1 >> 24;
			code->function_code[curfunc][cv + 3] = (val1 >> 16) & 0xFF;
			code->function_code[curfunc][cv + 4] = (val1 >> 8) & 0xFF;
			code->function_code[curfunc][cv + 5] = val1 & 0xFF;
			code->function_code[curfunc][cv + 6] = g1 == 1 ? 1 : 0;
			code->function_code[curfunc][cv + 7] = val2 >> 24;
			code->function_code[curfunc][cv + 8] = (val2 >> 16) & 0xFF;
			code->function_code[curfunc][cv + 9] = (val2 >> 8) & 0xFF;
			code->function_code[curfunc][cv + 10] = val2 & 0xFF;

			cv += 11;

			code->bt_trl[curfunc][cv1] = 243;

			cv1++;
		}
		else
		if (strcmp(tok, "copy_string") == NULL)
		{
			if (func == 0 || expect_bracket == 1)
			{
				error++;
				LogApp("Compiler error: detected command outside function - line: %d, command: \"%s\"", line, tok);
				continue;
			}

			CheckCodeSize(code, curfunc, cv);

			int32 g = 0, f = 0, g1 = 0;

			tok = strtok(NULL, " \n\t");

			if (tok != NULL)
			{
				if (IsNumber(tok) == 0)
				{
					for (i = 0; i < code->num_vars; i++)
					{
						if (strcmp(code->vars_table[i].name, tok) == NULL)
						{
							//Found global variable in the table
							code->vars_table[i].num_use++;

							if (code->vars_table[i].type != 4)
							{
								g = 3;
								error++;
								LogApp("Compiler error: detected variable that is not a string in the first argument - line: %d, variable: \"%s\"",
									line, code->vars_table[i].name);
								break;
							}

							val1 = i;
							g = 2;
							break;
						}
					}

					if (g == 3)
						continue;

					for (i = 0; i < code->function_table[curfunc].num_vars; i++)
					{
						if (strcmp(code->function_table[curfunc].vars_table[i].name, tok) == NULL)
						{
							//Found local variable in the table
							if (g == 2)
							{
								error++;
								LogApp("Compiler error: function \"%s\" local variable \"%s\" conflicting with global variable with the same name in \"%s\" command - line: %d",
									code->function_table[curfunc].name, tok, str1, line);
								g = -1;
								break;
							}

							if (code->function_table[curfunc].vars_table[i].type != 4)
							{
								g = 3;
								error++;
								LogApp("Compiler error: detected variable that is not a string in the first argument - line: %d, variable: \"%s\"",
									line, code->function_table[curfunc].vars_table[i].name);
								break;
							}

							code->function_table[curfunc].vars_table[i].num_use++;

							///if (code->function_table[curfunc].vars_table[i].type == 2)
							//f |= 1;

							val1 = i;
							g = 1;
							break;
						}
					}

					if (g == 3)
						continue;
				}
				else
				{
					error++;
					LogApp("Compiler error: detected constant in the first argument - line: %d", line);
					continue;
				}
			}

			tok = strtok(NULL, " \n\t");

			if (tok != NULL)
			{
				if (IsNumber(tok) == 0)
				{
					for (i = 0; i < code->num_vars; i++)
					{
						if (strcmp(code->vars_table[i].name, tok) == NULL)
						{
							//Found global variable in the table
							code->vars_table[i].num_use++;

							if (code->vars_table[i].type != 4)
							{
								g = 3;
								error++;
								LogApp("Compiler error: detected variable that is not a string in the first argument - line: %d, variable: \"%s\"",
									line, code->vars_table[i].name);
								break;
							}

							val2 = i;
							g1 = 2;
							break;
						}
					}

					for (i = 0; i < code->function_table[curfunc].num_vars; i++)
					{
						if (strcmp(code->function_table[curfunc].vars_table[i].name, tok) == NULL)
						{
							//Found local variable in the table
							if (g1 == 2)
							{
								error++;
								LogApp("Compiler error: function \"%s\" local variable \"%s\" conflicting with global variable with the same name in \"%s\" command - line: %d",
									code->function_table[curfunc].name, tok, str1, line);
								g1 = -1;
								break;
							}

							if (code->vars_table[i].type != 4)
							{
								g = 3;
								error++;
								LogApp("Compiler error: detected variable that is not a string in the first argument - line: %d, variable: \"%s\"",
									line, code->vars_table[i].name);
								break;
							}

							code->function_table[curfunc].vars_table[i].num_use++;

							val2 = i;
							g1 = 1;
							break;
						}
					}
				}
				else
				{
					error++;
					LogApp("Compiler error: detected constant in the second argument - line: %d", line);
					continue;
				}
			}

			if (g1 == 2)
				g1 = 0;

			if (g == 2)
				g = 0;

			code->function_code[curfunc][cv] = 244;
			code->function_code[curfunc][cv + 1] = g == 1 ? 1 : 0;
			code->function_code[curfunc][cv + 2] = val1 >> 24;
			code->function_code[curfunc][cv + 3] = (val1 >> 16) & 0xFF;
			code->function_code[curfunc][cv + 4] = (val1 >> 8) & 0xFF;
			code->function_code[curfunc][cv + 5] = val1 & 0xFF;
			code->function_code[curfunc][cv + 6] = g1 == 1 ? 1 : 0;
			code->function_code[curfunc][cv + 7] = val2 >> 24;
			code->function_code[curfunc][cv + 8] = (val2 >> 16) & 0xFF;
			code->function_code[curfunc][cv + 9] = (val2 >> 8) & 0xFF;
			code->function_code[curfunc][cv + 10] = val2 & 0xFF;

			cv += 11;

			code->bt_trl[curfunc][cv1] = 244;

			cv1++;
		}
		else
		if (strcmp(tok, "set") == NULL || strcmp(tok, "add") == NULL || strcmp(tok, "sub") == NULL || strcmp(tok, "mul") == NULL || strcmp(tok, "div") == NULL
			|| strcmp(tok, "pow") == NULL)
		{
			strcpy(code->linecode[line].code, buf);
			code->linecode[line].func = curfunc;

			if (func == 0 || expect_bracket == 1)
			{
				error++;
				LogApp("Compiler error: detected command outside function - line: %d, command: \"%s\"", line, tok);
				continue;
			}

			strcpy(str1, tok);

			tok = strtok(NULL, " \n\t");

			int g = 0, f = 0, c = 0, g1 = 0;

			if (tok != NULL)
			{
				if (IsNumber(tok) == 0)
				{
					if (tok[0] == 'a' && tok[1] == 'r' && tok[2] == 'g' && ((tok[3] >= '0' && tok[3] <= '9') || (tok[3] == 'f' && (tok[4] >= '0' && tok[4] <= '9'))))
					{
						if (strlen(tok) == 4)
							val1 = tok[3] - 48 + 9 - 1;

						if (strlen(tok) == 5 && tok[3] != 'f')
							val1 = atoi(tok + 3) + 9 - 1;

						if (strlen(tok) == 5 && tok[3] == 'f')
							val1 = tok[4] - 48 + 5 - 1;

						if (strlen(tok) == 6 && tok[3] == 'f')
							val1 = atoi(tok + 4) + 5 - 1;

						g = 3;
					}
					else
					if (strcmp(tok, "return") == NULL)
					{
						val1 = 2;
						g = 3;
					}
					else
					{
						//val1 = -1;
						for (i = 0; i < code->num_vars; i++)
						{
							if (strcmp(code->vars_table[i].name, tok) == NULL)
							{
								//Found global variable in the table
								code->vars_table[i].num_use++;

								if (code->vars_table[i].type % 10 == 2)
									f |= 1;

								val1 = i;
								g = 2;
								break;
							}
						}

						for (i = 0; i < code->function_table[curfunc].num_vars; i++)
						{
							if (strcmp(code->function_table[curfunc].vars_table[i].name, tok) == NULL)
							{
								//Found local variable in the table
								if (g == 2)
								{
									error++;
									LogApp("Compiler error: function \"%s\" local variable \"%s\" conflicting with global variable with the same name in \"%s\" command - line: %d",
										code->function_table[curfunc].name, tok, str1, line);
									g = -1;
									break;
								}

								code->function_table[curfunc].vars_table[i].num_use++;

								if (code->function_table[curfunc].vars_table[i].type == 2)
									f |= 1;

								val1 = i;
								g = 1;
								break;
							}
						}

						if (g == 2)
						{
							if (code->vars_table[val1].type >= 10) //It's an array
							{
								tok = strtok(NULL, " \n\t");

								if (tok != NULL)
								{
									if (tok[0] != '[' || tok[strlen(tok)] != ']')
									{
										error++;
										LogApp("Compiler error: variable \"%s\" array index outside square brackets - line: %d", tok, line);
										continue;
									}

									char str2[64];

									strcpy(str2, tok + 1);
									str2[strlen(str2)] = '\0';

									for (i = 0; i < code->num_vars; i++)
									{
										if (strcmp(code->vars_table[i].name, str2) == NULL)
										{
											//Found global variable in the table
											code->vars_table[i].num_use++;

											if (code->vars_table[i].type % 10 == 2)
											{
												error++;
												LogApp("Compiler error: variable float \"%s\" used as array index - line: %d", str2, line);
												break;
											}

											val1 = i;
											g = 2;
											break;
										}
									}
									
								}
								else
								{
									error++;
									LogApp("Compiler error: missing variable \"%s\" array index - line: %d", tok, line);
									continue;
								}
							}
						}
					}

					if (g == -1)
						continue;

					if (g == 0)
					{
						error++;
						LogApp("Compiler error: undefined variable \"%s\" as first argument in \"%s\" command - line: %d, ", tok, str1, line);
						continue;
					}
				}
				else
				{
					error++;
					LogApp("Compiler error: detected constant value as first argument in \"%s\" command - line: %d, ", str1, line);
					continue;
				}
			}
			else
			{
				error++;
				LogApp("Compiler error: missing first argument in \"%s\" command - line: %d", str1, line);
				continue;
			}

			tok = strtok(NULL, " \n\t");

			if (tok != NULL)
			{
				if (IsNumber(tok) == 0)
				{
					if (tok[0] == 'a' && tok[1] == 'r' && tok[2] == 'g' && ((tok[3] >= '0' && tok[3] <= '9') || (tok[3] == 'f' && (tok[4] >= '0' && tok[4] <= '9'))))
					{
						if (strlen(tok) == 4)
							val2 = tok[3] - 48 + 9 - 1;

						if (strlen(tok) == 5 && tok[3] != 'f')
							val2 = atoi(tok + 3) + 9 - 1;

						if (strlen(tok) == 5 && tok[3] == 'f')
							val2 = tok[4] - 48 + 5 - 1;

						if (strlen(tok) == 6 && tok[3] == 'f')
							val2 = atoi(tok + 4) + 5 - 1;

						g1 = 3;
					}
					else
					if (strcmp(tok, "return") == NULL)
					{
						val2 = 2;
						g1 = 3;
					}
					else
					{
						for (i = 0; i < code->num_vars; i++)
						{
							if (strcmp(code->vars_table[i].name, tok) == NULL)
							{
								//Found global variable in the table
								code->vars_table[i].num_use++;

								if (code->vars_table[i].type == 2)
									f |= 2;

								val2 = i;
								g1 = 2;
								break;
							}
						}

						for (i = 0; i < code->function_table[curfunc].num_vars; i++)
						{
							if (strcmp(code->function_table[curfunc].vars_table[i].name, tok) == NULL)
							{
								//Found local variable in the table
								if (g1 == 2)
								{
									error++;
									LogApp("Compiler error: function \"%s\" local variable \"%s\" conflicting with global variable with the same name in \"%s\" command - line: %d",
										code->function_table[curfunc].name, tok, str1, line);
									g1 = -1;
									break;
								}

								code->function_table[curfunc].vars_table[i].num_use++;

								val2 = i;

								if (code->function_table[curfunc].vars_table[i].type == 2)
									f |= 2;

								g1 = 1;
								break;
							}
						}
					}

					if (g1 == -1)
						continue;

					if (g1 == 0)
					{
						error++;
						LogApp("Compiler error: undefined variable \"%s\" as second argument in \"%s\" command - line: %d, ", tok, str1, line);
						continue;
					}
				}
				else
				{
					c = 1;

					if (IsNumberFloat(tok) == 1)
					{
						valf2 = atof(tok);
						f |= 2;
					}
					else
						val2 = atol(tok);
				}
			}
			else
			{
				error++;
				LogApp("Compiler error: missing second argument in \"%s\" command - line: %d", str1, line);
				continue;
			}

			int t = 0;

			if (g == 2)
				g = 0;

			if (g1 == 2)
				g1 = 0;

			if (strcmp(str1, "set") == NULL)
				t = 0;
			else
			if (strcmp(str1, "add") == NULL)
				t = 10;
			else
			if (strcmp(str1, "sub") == NULL)
				t = 20;
			else
			if (strcmp(str1, "mul") == NULL)
				t = 30;
			else
			if (strcmp(str1, "div") == NULL)
				t = 40;
			else
			if (strcmp(str1, "pow") == NULL)
				t = 50;

			CheckCodeSize(code, curfunc, cv);

			code->linecode[line].byte_start = cv;

			if (g == 3 || g1 == 3)
			{
				if (c == 1 && f == 0 && g == 3)
				{
					code->function_code[curfunc][cv] = t;
					code->function_code[curfunc][cv + 1] = val1;
					code->function_code[curfunc][cv + 2] = val2 >> 24;
					code->function_code[curfunc][cv + 3] = (val2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 4] = (val2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 5] = val2 & 0xFF;

					cv += 6;

					code->bt_trl[curfunc][cv1] = t;

					cv1 += 1;
				}
				else
				if (c == 1 && f == 1 && g == 3)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t;
					code->function_code[curfunc][cv + 2] = val1;
					code->function_code[curfunc][cv + 3] = val2 >> 24;
					code->function_code[curfunc][cv + 4] = (val2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 5] = (val2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 6] = val2 & 0xFF;

					cv += 7;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t;

					cv1 += 2;
				}
				else
				if (c == 1 && f == 2 && g == 3)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t + 1;
					code->function_code[curfunc][cv + 2] = 0;
					memcpy(code->function_code[curfunc] + cv + 3, &valf2, sizeof(float));

					uint8 x = code->function_code[curfunc][cv + 3];
					uint8 y = code->function_code[curfunc][cv + 4];

					code->function_code[curfunc][cv + 3] = code->function_code[curfunc][cv + 6];
					code->function_code[curfunc][cv + 4] = code->function_code[curfunc][cv + 5];
					code->function_code[curfunc][cv + 5] = y;
					code->function_code[curfunc][cv + 6] = x;

					code->function_code[curfunc][cv + 7] = 250;
					code->function_code[curfunc][cv + 8] = val1;
					code->function_code[curfunc][cv + 9] = 0;

					cv += 10;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t + 1;
					code->bt_trl[curfunc][cv1 + 2] = 250;

					cv1 += 3;
				}
				else
				if (c == 1 && f == 3 && g == 3)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t + 1;
					code->function_code[curfunc][cv + 2] = val1;
					memcpy(code->function_code[curfunc] + cv + 3, &valf2, sizeof(float));

					uint8 x = code->function_code[curfunc][cv + 3];
					uint8 y = code->function_code[curfunc][cv + 4];

					code->function_code[curfunc][cv + 3] = code->function_code[curfunc][cv + 6];
					code->function_code[curfunc][cv + 4] = code->function_code[curfunc][cv + 5];
					code->function_code[curfunc][cv + 5] = y;
					code->function_code[curfunc][cv + 6] = x;

					cv += 7;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t + 1;

					cv1 += 2;
				}
				else
				if (c == 0 && f == 0)
				{
					if (g == 3 && g1 != 3)
					{
						code->function_code[curfunc][cv] = t + 1;
						code->function_code[curfunc][cv + 1] = val1;
						code->function_code[curfunc][cv + 2] = g1 == 1 ? 1 : 0;
						code->function_code[curfunc][cv + 3] = val2 >> 24;
						code->function_code[curfunc][cv + 4] = (val2 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 5] = (val2 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 6] = val2 & 0xFF;

						cv += 7;

						code->bt_trl[curfunc][cv1] = t + 1;

						cv1 += 1;
					}
					else
					if (g == 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = t + 2;
						code->function_code[curfunc][cv + 1] = val1;
						code->function_code[curfunc][cv + 2] = val2;

						cv += 3;

						code->bt_trl[curfunc][cv1] = t + 2;

						cv1 += 1;
					}
					if (g != 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = t + 4;
						code->function_code[curfunc][cv + 1] = g == 1 ? 1 : 0;
						code->function_code[curfunc][cv + 2] = val1 >> 24;
						code->function_code[curfunc][cv + 3] = (val1 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 4] = (val1 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 5] = val1 & 0xFF;
						code->function_code[curfunc][cv + 6] = val2;

						cv += 7;

						code->bt_trl[curfunc][cv1] = t + 4;

						cv1 += 1;
					}
				}
				else
				if (c == 0 && f == 1)
				{
					if (g == 3 && g1 != 3)
					{
						code->function_code[curfunc][cv] = 255;
						code->function_code[curfunc][cv + 1] = t + 3;
						code->function_code[curfunc][cv + 2] = val1;
						code->function_code[curfunc][cv + 3] = g1 == 1 ? 1 : 0;
						code->function_code[curfunc][cv + 4] = val2 >> 24;
						code->function_code[curfunc][cv + 5] = (val2 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 6] = (val2 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 7] = val2 & 0xFF;

						cv += 8;

						code->bt_trl[curfunc][cv1] = 255;
						code->bt_trl[curfunc][cv1 + 1] = t + 3;

						cv1 += 2;
					}
					else
					if (g == 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = 255;
						code->function_code[curfunc][cv + 1] = t + 2;
						code->function_code[curfunc][cv + 2] = val1;
						code->function_code[curfunc][cv + 3] = val2;

						cv += 4;

						code->bt_trl[curfunc][cv1] = 255;
						code->bt_trl[curfunc][cv1 + 1] = t + 5;

						cv1 += 2;
					}
					if (g != 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = 251;
						code->function_code[curfunc][cv + 1] = 0;
						code->function_code[curfunc][cv + 2] = val2;
						code->function_code[curfunc][cv + 3] = 255;
						code->function_code[curfunc][cv + 4] = t + 7;
						code->function_code[curfunc][cv + 5] = g == 1 ? 1 : 0;
						code->function_code[curfunc][cv + 6] = val1 >> 24;
						code->function_code[curfunc][cv + 7] = (val1 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 8] = (val1 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 9] = val1 & 0xFF;
						code->function_code[curfunc][cv + 10] = 0;

						cv += 11;

						code->bt_trl[curfunc][cv1] = 251;
						code->bt_trl[curfunc][cv1 + 1] = 255;
						code->bt_trl[curfunc][cv1 + 2] = t + 7;

						cv1 += 3;
					}
				}
				else
				if (c == 0 && f == 2)
				{
					if (g == 3 && g1 != 3)
					{
						code->function_code[curfunc][cv] = 255;
						code->function_code[curfunc][cv + 1] = t + 4;
						code->function_code[curfunc][cv + 2] = 0;
						code->function_code[curfunc][cv + 3] = g1 == 1 ? 1 : 0;
						code->function_code[curfunc][cv + 4] = val2 >> 24;
						code->function_code[curfunc][cv + 5] = (val2 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 6] = (val2 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 7] = val2 & 0xFF;
						code->function_code[curfunc][cv + 8] = 250;
						code->function_code[curfunc][cv + 9] = val1;
						code->function_code[curfunc][cv + 10] = 0;

						cv += 11;

						code->bt_trl[curfunc][cv1] = 255;
						code->bt_trl[curfunc][cv1 + 1] = t + 4;
						code->bt_trl[curfunc][cv1 + 2] = 250;

						cv1 += 3;
					}
					else
					if (g == 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = 255;
						code->function_code[curfunc][cv + 1] = t + 5;
						code->function_code[curfunc][cv + 2] = 0;
						code->function_code[curfunc][cv + 3] = val2;
						code->function_code[curfunc][cv + 4] = 250;
						code->function_code[curfunc][cv + 5] = val1;
						code->function_code[curfunc][cv + 6] = 0;

						cv += 7;

						code->bt_trl[curfunc][cv1] = 255;
						code->bt_trl[curfunc][cv1 + 1] = t + 5;
						code->bt_trl[curfunc][cv1 + 2] = 250;

						cv1 += 3;
					}
					if (g != 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = 250;
						code->function_code[curfunc][cv + 1] = 6;
						code->function_code[curfunc][cv + 2] = val2;
						code->function_code[curfunc][cv + 3] = t + 4;
						code->function_code[curfunc][cv + 4] = g == 1 ? 1 : 0;
						code->function_code[curfunc][cv + 5] = val1 >> 24;
						code->function_code[curfunc][cv + 6] = (val1 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 7] = (val1 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 8] = val1 & 0xFF;
						code->function_code[curfunc][cv + 9] = 6;

						cv += 10;

						code->bt_trl[curfunc][cv1] = 250;
						code->bt_trl[curfunc][cv1 + 1] = t + 4;

						cv1 += 2;
					}
				}
				else
				if (c == 0 && f == 3)
				{
					if (g == 3 && g1 != 3)
					{
						code->function_code[curfunc][cv] = 255;
						code->function_code[curfunc][cv + 1] = t + 4;
						code->function_code[curfunc][cv + 2] = val1;
						code->function_code[curfunc][cv + 3] = g1 == 1 ? 1 : 0;
						code->function_code[curfunc][cv + 4] = val2 >> 24;
						code->function_code[curfunc][cv + 5] = (val2 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 6] = (val2 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 7] = val2 & 0xFF;

						cv += 7;

						code->bt_trl[curfunc][cv1] = 255;
						code->bt_trl[curfunc][cv1 + 1] = t + 4;

						cv1 += 2;
					}
					else
					if (g == 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = 255;
						code->function_code[curfunc][cv + 1] = t + 5;
						code->function_code[curfunc][cv + 2] = val1;
						code->function_code[curfunc][cv + 3] = val2;

						cv += 4;

						code->bt_trl[curfunc][cv1] = 255;
						code->bt_trl[curfunc][cv1 + 1] = t + 5;

						cv1 += 2;
					}
					if (g != 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = 255;
						code->function_code[curfunc][cv + 1] = t + 7;
						code->function_code[curfunc][cv + 2] = g == 1 ? 1 : 0;
						code->function_code[curfunc][cv + 3] = val1 >> 24;
						code->function_code[curfunc][cv + 4] = (val1 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 5] = (val1 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 6] = val1 & 0xFF;
						code->function_code[curfunc][cv + 7] = val2;

						cv += 8;

						code->bt_trl[curfunc][cv1] = 255;
						code->bt_trl[curfunc][cv1 + 1] = t + 7;

						cv1 += 2;
					}
				}
			}
			else
			{
				if (c == 1 && f == 0)
				{
					code->function_code[curfunc][cv] = t + 3;
					code->function_code[curfunc][cv + 1] = g == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 2] = val1 >> 24;
					code->function_code[curfunc][cv + 3] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 4] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 5] = val1 & 0xFF;
					code->function_code[curfunc][cv + 6] = val2 >> 24;
					code->function_code[curfunc][cv + 7] = (val2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 8] = (val2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 9] = val2 & 0xFF;

					cv += 10;

					code->bt_trl[curfunc][cv1] = t + 3;

					cv1 += 1;
				}
				else
				if (c == 1 && f == 1)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t + 1;
					code->function_code[curfunc][cv + 2] = 0;
					memcpy(code->function_code[curfunc] + cv + 3, &valf2, sizeof(float));

					uint8 x = code->function_code[curfunc][cv + 3];
					uint8 y = code->function_code[curfunc][cv + 4];

					code->function_code[curfunc][cv + 3] = code->function_code[curfunc][cv + 6];
					code->function_code[curfunc][cv + 4] = code->function_code[curfunc][cv + 5];
					code->function_code[curfunc][cv + 5] = y;
					code->function_code[curfunc][cv + 6] = x;

					code->function_code[curfunc][cv + 7] = 250;
					code->function_code[curfunc][cv + 8] = 6;
					code->function_code[curfunc][cv + 9] = 0;

					code->function_code[curfunc][cv + 10] = t + 4;
					code->function_code[curfunc][cv + 11] = g == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 12] = val1 >> 24;
					code->function_code[curfunc][cv + 13] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 14] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 15] = val1 & 0xFF;
					code->function_code[curfunc][cv + 16] = 6;

					cv += 17;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t + 1;
					code->bt_trl[curfunc][cv1 + 2] = 250;
					code->bt_trl[curfunc][cv1 + 3] = t + 4;

					cv1 += 4;
				}
				else
				if (c == 1 && f == 2)
				{
					code->function_code[curfunc][cv] = t + 0;
					code->function_code[curfunc][cv + 1] = 6;
					code->function_code[curfunc][cv + 2] = val2 >> 24;
					code->function_code[curfunc][cv + 3] = (val2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 4] = (val2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 5] = val2 & 0xFF;

					code->function_code[curfunc][cv + 6] = 251;
					code->function_code[curfunc][cv + 7] = 0;
					code->function_code[curfunc][cv + 8] = 6;

					code->function_code[curfunc][cv + 9] = 255;
					code->function_code[curfunc][cv + 10] = t + 7;
					code->function_code[curfunc][cv + 11] = g == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 12] = val1 >> 24;
					code->function_code[curfunc][cv + 13] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 14] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 15] = val1 & 0xFF;
					code->function_code[curfunc][cv + 16] = 0;

					cv += 17;

					code->bt_trl[curfunc][cv1] = t + 0;
					code->bt_trl[curfunc][cv1 + 1] = 251;
					code->bt_trl[curfunc][cv1 + 2] = 255;
					code->bt_trl[curfunc][cv1 + 3] = t + 7;

					cv1 += 4;
				}
				else
				if (c == 1 && f == 3)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t + 9;
					code->function_code[curfunc][cv + 2] = g == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 3] = val1 >> 24;
					code->function_code[curfunc][cv + 4] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 5] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 6] = val1 & 0xFF;
					code->function_code[curfunc][cv + 7] = g1 == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 8] = val2 >> 24;
					code->function_code[curfunc][cv + 9] = (val2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 10] = (val2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 11] = val2 & 0xFF;

					cv += 12;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t + 9;

					cv1 += 2;
				}
				else
				if (c == 0 && f == 1)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t + 8;
					code->function_code[curfunc][cv + 2] = g == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 3] = val1 >> 24;
					code->function_code[curfunc][cv + 4] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 5] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 6] = val1 & 0xFF;
					code->function_code[curfunc][cv + 7] = g1 == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 8] = val2 >> 24;
					code->function_code[curfunc][cv + 9] = (val2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 10] = (val2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 11] = val2 & 0xFF;

					cv += 12;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t + 8;

					cv1 += 2;
				}
				else
				if (c == 0 && f == 2)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t + 4;
					code->function_code[curfunc][cv + 2] = 0;
					code->function_code[curfunc][cv + 3] = g1 & 1 == 1 ? 0 : 1;
					code->function_code[curfunc][cv + 4] = val2 >> 24;
					code->function_code[curfunc][cv + 5] = (val2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 6] = (val2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 7] = val2 & 0xFF;

					code->function_code[curfunc][cv + 8] = 250;
					code->function_code[curfunc][cv + 9] = 6;
					code->function_code[curfunc][cv + 10] = 0;

					code->function_code[curfunc][cv + 11] = t + 4;
					code->function_code[curfunc][cv + 12] = g == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 13] = val1 >> 24;
					code->function_code[curfunc][cv + 14] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 15] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 16] = val1 & 0xFF;
					code->function_code[curfunc][cv + 17] = 6;

					cv += 18;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t + 4;
					code->bt_trl[curfunc][cv1 + 2] = 250;
					code->bt_trl[curfunc][cv1 + 3] = t + 4;

					cv1 += 4;
				}
				else
				if (c == 0 && f == 3)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t + 9;
					code->function_code[curfunc][cv + 2] = g == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 3] = val1 >> 24;
					code->function_code[curfunc][cv + 4] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 5] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 6] = val1 & 0xFF;
					code->function_code[curfunc][cv + 7] = g1 == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 8] = val2 >> 24;
					code->function_code[curfunc][cv + 9] = (val2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 10] = (val2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 11] = val2 & 0xFF;

					cv += 12;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t + 9;

					cv1 += 1;
				}
				else
				if (c == 0 && f == 0)
				{
					code->function_code[curfunc][cv] = t + 5;
					code->function_code[curfunc][cv + 1] = g == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 2] = val1 >> 24;
					code->function_code[curfunc][cv + 3] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 4] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 5] = val1 & 0xFF;
					code->function_code[curfunc][cv + 6] = g1 == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 7] = val2 >> 24;
					code->function_code[curfunc][cv + 8] = (val2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 9] = (val2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 10] = val2 & 0xFF;

					cv += 11;

					code->bt_trl[curfunc][cv1] = t + 5;

					cv1 += 1;
				}
			}

			code->linecode[line].byte_end = cv;

		}
		else
		if (strcmp(tok, "func") == NULL) //Function entry point
		{
			strcpy(code->linecode[line].code, buf);
			//code->linecode[line].func = curfunc;

			if (func == 1)
			{
				error++;
				LogApp("Compiler error: trying to declare a function inside a function - line: %d", line);
				continue;
			}

			tok = strtok(NULL, " \n\t");

			if (tok == NULL)
			{
				error++;
				LogApp("Compiler error: missing function name in func command - line: %d", line);
				continue;
			}

			for (i = 0; i < code->num_functions; i++)
			{
				if (strcmp(tok, code->function_table[i].name) == NULL)
				{
					error++;
					LogApp("Compiler error: redefinition of function name in line: %d - function: \"%s\"", line, tok);
					i = -1;
					break;
				}
			}

			if (i == -1)
				continue;

			func = 1;

			code->num_functions++;
			i = code->num_functions - 1;

			if (code->num_functions - 1 == 0)
			{
				code->function_table = calloc(1, sizeof(*code->function_table));
				code->bt_trl = calloc(1, sizeof(uint32*));
				code->bt_trl[0] = calloc(128, sizeof(uint32));
				code->function_code = calloc(1, sizeof(unsigned char*));
				code->function_code[0] = calloc(128, 1);
			}
			else
			{
				code->function_table = realloc(code->function_table, code->num_functions * sizeof(*code->function_table));
				
				code->function_code = realloc(code->function_code, code->num_functions * sizeof(unsigned char*));
				code->function_code[code->num_functions - 1] = calloc(128, sizeof(unsigned char));

				code->bt_trl = realloc(code->bt_trl, code->num_functions * sizeof(uint32*));
				code->bt_trl[code->num_functions - 1] = calloc(128, sizeof(uint32));
			}

			CHECKMEM(code->function_table);
			CHECKMEM(code->function_code);
			CHECKMEM(code->bt_trl);

			strcpy(code->function_table[i].name, tok);
			curfunc = i;
			code->function_table[i].num_vars = 0;
			code->function_table[i].num_use = 0;
			code->function_table[i].address = 0;
			code->function_table[i].num_args = 0;

			code->function_table[i].size = 128;

			while ((tok = strtok(NULL, " \n\t")) != NULL)
			{
				if (strcmp(tok, "var") == NULL || strcmp(tok, "float") == NULL || strcmp(tok, "buffer") == NULL || strcmp(tok, "string") == NULL)
					code->function_table[i].num_args++;
				else
				{
					error++;
					LogApp("Compiler error: undefined keyword \"%s\" in function argument list \"%s\" - line: %d", tok, code->function_table[i].name, line);
					code->num_functions--; //Remove function due to error
					code->function_table = realloc(code->function_table, code->num_functions * sizeof(*code->function_table));
					CHECKMEM(code->function_table);
					func = 0;
					break;
				}
			}

			code->function_table[curfunc].line = line;

			expect_bracket = 1;
			cv = 0;
			cv1 = 0;
		}
		else
		if (strcmp(tok, "return") == NULL)
		{
			strcpy(code->linecode[line].code, buf);
			code->linecode[line].func = curfunc;

			if (func == 0 || expect_bracket == 1)
			{
				error++;
				LogApp("Compiler error: found return outside function - line: %d", line);
				continue;
			}

			tok = strtok(NULL, " \n\t");

			int g = 0, f = 0, c = 0;

			CheckCodeSize(code,curfunc, cv);

			if (tok != NULL)
			{
				if (IsNumber(tok) == 0)
				{
					val1 = -1;

					for (i = 0; i < code->num_vars; i++)
					{
						if (strcmp(code->vars_table[i].name, tok) == NULL)
						{
							//Found global variable in the table
							code->vars_table[i].num_use++;

							if (code->vars_table[i].type == 2)
								f |= 2;

							val1 = i;

							g = 2;

							break;
						}
					}

					for (i = 0; i < code->function_table[curfunc].num_vars; i++)
					{
						if (strcmp(code->function_table[curfunc].vars_table[i].name, tok) == NULL)
						{
							//Found local variable in the table
							if (val1 < -1)
							{
								error++;
								LogApp("Compiler error: function \"%s\" local variable \"%s\" conflicting with global variable with the same name in \"%s\" command - line: %d",
									code->function_table[curfunc].name, tok, str1, line);
								g = -1;
								break;
							}

							code->function_table[curfunc].vars_table[i].num_use++;

							val1 = i;

							if (code->function_table[curfunc].vars_table[i].type == 2)
								f |= 2;

							g = 1;
							break;
						}
					}

					if (g == -1)
						continue;

					if (val1 == -1)
					{
						error++;
						LogApp("Compiler error: undefined variable \"%s\" as argument in return command - line: %d, ", tok, line);
						continue;
					}
				}
				else
				{
					c = 1;

					if (IsNumberFloat(tok) == 1)
					{
						valf1 = atof(tok);
						f |= 2;
					}
					else
						val1 = atol(tok);
				}
			}
			else
			{
				code->function_code[curfunc][cv] = 210;
				ret = 1;
				cv++;
				continue;
			}

			if (g == 2)
				g = 0;

			code->linecode[line].byte_start = cv;

			if (c == 1 && f == 0)
			{
				code->function_code[curfunc][cv] = 0;
				code->function_code[curfunc][cv + 1] = 0;
				code->function_code[curfunc][cv + 2] = val1 >> 24;
				code->function_code[curfunc][cv + 3] = (val1 >> 16) && 0xFF;
				code->function_code[curfunc][cv + 4] = (val1 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 5] = val1 & 0xFF;
				code->function_code[curfunc][cv + 6] = 210;

				cv += 7;

				code->bt_trl[curfunc][cv1] = 0;
				code->bt_trl[curfunc][cv1 + 1] = 210;

				cv1 += 2;
			}
			else
			if (c == 1 && f == 2)
			{
				code->function_code[curfunc][cv] = 255;
				code->function_code[curfunc][cv + 1] = 1;
				code->function_code[curfunc][cv + 2] = 0;

				CopyFloatToInt32(code->function_code[curfunc] + cv + 3, valf1);

				code->function_code[curfunc][cv + 7] = 250;
				code->function_code[curfunc][cv + 8] = 0;
				code->function_code[curfunc][cv + 9] = 0;
				code->function_code[curfunc][cv + 10] = 210;

				cv += 11;

				code->bt_trl[curfunc][cv1] = 255;
				code->bt_trl[curfunc][cv1 + 1] = 1;
				code->bt_trl[curfunc][cv1 + 2] = 250;
				code->bt_trl[curfunc][cv1 + 3] = 210;

				cv1 += 4;
			}
			else
			if (c == 0 && f == 0)
			{
				code->function_code[curfunc][cv] = 1;
				code->function_code[curfunc][cv + 1] = 0;
				code->function_code[curfunc][cv + 2] = g == 1 ? 1 : 0;
				code->function_code[curfunc][cv + 3] = val1 >> 24;
				code->function_code[curfunc][cv + 4] = (val1 >> 16) && 0xFF;
				code->function_code[curfunc][cv + 5] = (val1 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 6] = val1 & 0xFF;
				code->function_code[curfunc][cv + 7] = 210;

				cv += 8;

				code->bt_trl[curfunc][cv1] = 1;
				code->bt_trl[curfunc][cv1 + 1] = 210;

				cv1 += 2;
			}
			else
			if (c == 0 && f == 1)
			{
				code->function_code[curfunc][cv] = 255;
				code->function_code[curfunc][cv + 1] = 4;
				code->function_code[curfunc][cv + 2] = 0;
				code->function_code[curfunc][cv + 3] = g == 1 ? 1 : 0;
				code->function_code[curfunc][cv + 4] = val1 >> 24;
				code->function_code[curfunc][cv + 5] = (val1 >> 16) && 0xFF;
				code->function_code[curfunc][cv + 6] = (val1 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 7] = val1 & 0xFF;

				code->function_code[curfunc][cv + 8] = 250;
				code->function_code[curfunc][cv + 9] = 0;
				code->function_code[curfunc][cv + 10] = 0;

				code->function_code[curfunc][cv + 11] = 210;

				cv += 12;

				code->bt_trl[curfunc][cv1] = 255;
				code->bt_trl[curfunc][cv1 + 1] = 4;
				code->bt_trl[curfunc][cv1 + 2] = 250;
				code->bt_trl[curfunc][cv1 + 3] = 210;

				cv1 += 4;
			}

			code->linecode[line].byte_end = cv;

			ret = 1;
		}
		else
		if (strcmp(tok, "call") == NULL)
		{
			strcpy(code->linecode[line].code, buf);
			code->linecode[line].func = curfunc;

			if (func == 0)
			{
				error++;
				LogApp("Compiler error: call command outside function declaration - line: %d", line);
				continue;
			}

			tok = strtok(NULL, " \n\t");

			if (tok == NULL)
			{
				error++;
				LogApp("Compiler error: missing call function name - line: %d", line);
				continue;
			}

			int a = 0;

			for (i = 0; i < code->num_functions; i++)
			{
				if (strcmp(tok, code->function_table[i].name) == NULL)
				{
					//Found function
					a = 1;
					break;
				}
			}

			if (a == 0)
			{
				if ((i = GetEngFunc(tok)) != -1)
				{
					//Found engine function
					a = 2;
				}
			}

			strcpy(str1, tok);

			int c = 0, g = 0, f = 0, j = 0, b = 0, vars[24], var_c = 0;
			val1 = -1;

			ZeroMem(vars, 24 * sizeof(int));

			code->linecode[line].byte_start = cv;

			while ((tok = strtok(NULL, " \n\t")) != NULL)
			{
				c = 0;
				g = 0;
				f = 0;
				j = 0;

				if (IsNumber(tok) == 0)
				{
					//It's a variable
					for (j = 0; j < code->num_vars; j++)
					{
						if (strcmp(tok, code->vars_table[j].name) == NULL)
						{
							val1 = j;

							g = 2;

							if (code->vars_table[j].type == 2)
								f = 1;

							vars[var_c] = j;
							var_c++;

							break;
						}
					}

					if (g == 0)
					{
						for (j = 0; j < code->function_table[curfunc].num_vars; j++)
						{
							if (strcmp(tok, code->function_table[curfunc].vars_table[j].name) == NULL)
							{
								val1 = j;

								g = 1;

								if (code->function_table[curfunc].vars_table[j].type == 2)
									f = 1;

								vars[var_c] = j * -1;
								var_c++;

								break;
							}
						}

						if (g == 0)
						{
							error++;
							LogApp("Compiler error: undefined keyword \"%s\" at call command - line: %d", tok, line);
							c = -1;
							break;
						}
					}
				}
				else
				{
					c = 1;

					if (IsNumberFloat(tok) == 1)
					{
						valf1 = atof(tok);
						f = 1;
					}
					else
						val1 = atol(tok);
				}

				CheckCodeSize(code, curfunc, cv);

				if (g == 2)
					g = 0;

				if (c == 1 && f == 0)
				{
					code->function_code[curfunc][cv] = 0;
					code->function_code[curfunc][cv + 1] = b + 9;
					code->function_code[curfunc][cv + 2] = val1 >> 24;
					code->function_code[curfunc][cv + 3] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 4] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 5] = val1 & 0xFF;

					cv += 6;

					code->bt_trl[curfunc][cv1] = 0;

					cv1++;
				}
				else
				if (c == 1 && f == 1)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = 1;
					code->function_code[curfunc][cv + 2] = b + 5;
					CopyFloatToInt32(code->function_code[curfunc] + cv + 3, valf1);

					cv += 7;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = 1;

					cv1 += 2;
				}
				else
				if (c == 0 && f == 0)
				{
					code->function_code[curfunc][cv] = 1;
					code->function_code[curfunc][cv + 1] = b + 9;
					code->function_code[curfunc][cv + 2] = g == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 3] = val1 >> 24;
					code->function_code[curfunc][cv + 4] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 5] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 6] = val1 & 0xFF;

					cv += 7;

					code->bt_trl[curfunc][cv1] = 1;

					cv1++;
				}
				else
				if (c == 0 && f == 1)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = 4;
					code->function_code[curfunc][cv + 2] = b + 5;
					code->function_code[curfunc][cv + 3] = g == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 4] = val1 >> 24;
					code->function_code[curfunc][cv + 5] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 6] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 7] = val1 & 0xFF;

					cv += 8;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = 4;

					cv1 += 2;
				}

				b++;
			}

			CheckCodeSize(code, curfunc, cv);

			//If MGL function
			if (a == 1)
			{
				if (b < code->function_table[i].num_args)
				{
					error++;
					LogApp("Compiler error: missing arguments in call to \"%s\" - line: %d", str1, line);
					continue;
				}

				code->function_code[curfunc][cv] = 204;

				//i = code->function_table[i].address;

				code->function_code[curfunc][cv + 1] = 0;
				code->function_code[curfunc][cv + 2] = i >> 24;
				code->function_code[curfunc][cv + 3] = (i >> 16) & 0xFF;
				code->function_code[curfunc][cv + 4] = (i >> 8) & 0xFF;
				code->function_code[curfunc][cv + 5] = i & 0xFF;

				cv += 6;

				code->bt_trl[curfunc][cv1] = 204;

				cv1++;
			}
			else
			{
				//It's an engine call

				if (b < e_funcs[i].num_args)
				{
					error++;
					LogApp("Compiler error: missing arguments in call to \"%s\" - line: %d", str1, line);
					continue;
				}

				code->function_code[curfunc][cv] = 204;
				code->function_code[curfunc][cv + 1] = 1;
				code->function_code[curfunc][cv + 2] = i >> 24;
				code->function_code[curfunc][cv + 3] = (i >> 16) & 0xFF;
				code->function_code[curfunc][cv + 4] = (i >> 8) & 0xFF;
				code->function_code[curfunc][cv + 5] = i & 0xFF;

				cv += 6;

				code->bt_trl[curfunc][cv1] = 204;

				cv1++;

				code->function_table[i].num_use++;

				if (e_funcs[i].returnv == 1)
				{
					int k = 0;
					for (j = 0; j < var_c; j++)
					{
						CheckCodeSize(code, curfunc, cv);

						if (vars[j] < 0) //Local
						{
							k = vars[j] * -1;

							if (code->function_table[curfunc].vars_table[k].type == 2)
							{
								code->function_code[curfunc][cv] = 255;
								code->function_code[curfunc][cv + 1] = 7;
								code->function_code[curfunc][cv + 2] = 1;
								code->function_code[curfunc][cv + 3] = k >> 24;
								code->function_code[curfunc][cv + 4] = (k >> 16) & 0xFF;
								code->function_code[curfunc][cv + 5] = (k >> 8) & 0xFF;
								code->function_code[curfunc][cv + 6] = k & 0xFF;
								code->function_code[curfunc][cv + 7] = j + 5;

								cv += 8;

								code->bt_trl[curfunc][cv1] = 255;
								code->bt_trl[curfunc][cv1 + 1] = 7;

								cv1 += 2;
							}
							else
							{
								code->function_code[curfunc][cv] = 4;
								code->function_code[curfunc][cv + 1] = 1;
								code->function_code[curfunc][cv + 2] = k >> 24;
								code->function_code[curfunc][cv + 3] = (k >> 16) & 0xFF;
								code->function_code[curfunc][cv + 4] = (k >> 8) & 0xFF;
								code->function_code[curfunc][cv + 5] = k & 0xFF;
								code->function_code[curfunc][cv + 6] = j + 9;

								cv += 7;

								code->bt_trl[curfunc][cv1] = 4;

								cv1++;
							}
						}
					}
				}
			}

			code->linecode[line].byte_end = cv;
		}
		else
		if (strcmp(tok, "{") == NULL)
		{
			if (expect_bracket == 1)
				expect_bracket = 0;

			brackets++;
		}
		else
		if (strcmp(tok, "}") == NULL)
		{
			if (expect_bracket == 1)
			{
				error++;
				LogApp("Compiler error: expecting {, found } - line: %d", line);
				continue;
			}

			if (brackets > 0)
				brackets--;
			else
			{
				error++;
				LogApp("Compiler error: found } before { - line: %d", line);
				continue;
			}

			if (ifcmp > 0)
			{
				ifcmp--;
				code->function_code[curfunc][ifcmpaddr[ifcmp]] = cv >> 24;
				code->function_code[curfunc][ifcmpaddr[ifcmp] + 1] = (cv >> 16) & 0xFF;
				code->function_code[curfunc][ifcmpaddr[ifcmp] + 2] = (cv >> 8) & 0xFF;
				code->function_code[curfunc][ifcmpaddr[ifcmp] + 3] = cv & 0xFF;
			}

			if (brackets == 0)
			{
				if (ret == 2)
				{
					CheckCodeSize(code, curfunc, cv);

					code->function_code[curfunc][cv] = 210;
					cv++;

					code->bt_trl[curfunc][cv1] = 210;
					cv1++;
				}

				code->function_table[curfunc].size = cv;

				code->bt_trl[curfunc] = realloc(code->bt_trl[curfunc], code->function_table[curfunc].size * sizeof(uint32));
				code->function_code[curfunc] = realloc(code->function_code[curfunc], code->function_table[curfunc].size + 1);

				func = 0;
			}
		}
		else
		if (strcmp(tok, "if") == NULL)
		{
			strcpy(code->linecode[line].code, buf);
			code->linecode[line].func = curfunc;

			if (func == 0)
			{
				error++;
				LogApp("If comparison outside function at line: %d", line);
				continue;
			}

			CheckCodeSize(code, curfunc, cv);

			int g = 0, f = 0, c = 0, g1 = 0, t;

			int32 var1, var2, v, n;
			float nf, var1f, var2f;
			int16 res = 0;

			if ((tok = strtok(NULL, " \n\t")) != NULL)
			{
				res = GetVarOrNumber(tok, code, curfunc, &v, &n, &nf);

				if (res == 0)
				{
					error++;
					LogApp("Undefined variable \"%s\" at if sentence at line: %d", tok, line);
					continue;
				}
				
				switch (res)
				{
					case 1:
						g = 1;

						if (code->vars_table[v].type == 2)
							f = 1;
						else
							f = 0;

						var1 = v;
						break;

					case 2:
						g = 0;

						if (code->function_table[curfunc].vars_table[v].type == 2)
							f = 1;
						else
							f = 0;

						var1 = v;
						break;

					case 3:
						var1 = n;
						c = 1;
						break;

					case 4:
						var1f = nf;
						f = 1;
						c = 1;
						break;

					case 5:
						var1 = v;
						g = 3;
						break;
				}

				if (c == 1)
				{
					error++;
					LogApp("Found constant value as first argument in if sentence at line: %d", line);
					continue;
				}
			}
			else
			{
				error++;
				LogApp("Missing argument at if sentence at line: %d", line);
				continue;
			}

			if ((tok = strtok(NULL, " \n\t")) != NULL)
			{
				if (strcmp(tok, "==") == NULL)
					t = 182;
				else
				if (strcmp(tok, "!=") == NULL)
					t = 192;
				else
				if (strcmp(tok, ">") == NULL)
					t = 162;
				else
				if (strcmp(tok, "<") == NULL)
					t = 172;
				else
				if (strcmp(tok, ">=") == NULL)
					t = 142;
				else
				if (strcmp(tok, "<=") == NULL)
					t = 152;
				else
				if (strcmp(tok, "=>") == NULL)
					t = 142;
				else
				if (strcmp(tok, "=<") == NULL)
					t = 152;
			}
			else
			{
				error++;
				LogApp("Missing operator at if sentence at line: %d", line);
				continue;
			}

			if ((tok = strtok(NULL, " \n\t")) != NULL)
			{
				res = GetVarOrNumber(tok, code, curfunc, &v, &n, &nf);

				if (res == 0)
				{
					error++;
					LogApp("Undefined variable \"%s\" at if sentence at line: %d", tok, line);
					continue;
				}

				switch (res)
				{
				case 1:
					g1 = 0;

					if (code->vars_table[v].type == 2)
						f += 2;

					var2 = v;
					break;

				case 2:
					g1 = 1;

					if (code->function_table[curfunc].vars_table[v].type == 2)
						f += 2;

					var2 = v;
					break;

				case 3:
					var2 = n;
					c += 1;
					break;

				case 4:
					var2f = nf;
					f += 2;
					c += 1;
					break;

				case 5:
					var2 = v;
					g1 = 3;
					break;
				}
			}
			else
			{
				error++;
				LogApp("Missing second argument at if sentence at line: %d", line);
				continue;
			}

			int8 ifand = -1;

			if ((tok = strtok(NULL, " \n\t")) != NULL)
			{
				if (strcmp(tok, "and") == NULL)
					ifand = 0;
				else
				if (strcmp(tok, "or") == NULL)
					ifand = 1;
				else
				{
					error++;
					LogApp("Undefined keyword \"%s\" at line: %d", tok, line);
					continue;
				}
			}
			else
			{
				expect_bracket = 1;
				ifcmp += 1;
			}

			code->linecode[line].byte_start = cv;

			if (g == 3 || g1 == 3)
			{
				if (c == 1 && f == 0 && g == 3)
				{
					code->function_code[curfunc][cv] = t;
					code->function_code[curfunc][cv + 1] = var1;
					code->function_code[curfunc][cv + 2] = var2 >> 24;
					code->function_code[curfunc][cv + 3] = (var2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 4] = (var2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 5] = var2 & 0xFF;

					cv += 6 + 4;

					if (ifand != -1)
					{
						int32 cv2 = cv - 4;
						code->function_code[curfunc][cv2] = ifand >> 24;
						code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
						code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
						code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
					}
					else
						ifcmpaddr[ifcmp - 1] = cv - 4;

					code->bt_trl[curfunc][cv1] = t;

					cv1 += 1;
				}
				else
				if (c == 1 && f == 1 && g == 3)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t;
					code->function_code[curfunc][cv + 2] = var1;
					code->function_code[curfunc][cv + 3] = var2 >> 24;
					code->function_code[curfunc][cv + 4] = (var2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 5] = (var2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 6] = var2 & 0xFF;

					cv += 7 + 4;

					if (ifand != -1)
					{
						int32 cv2 = cv - 4;
						code->function_code[curfunc][cv2] = ifand >> 24;
						code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
						code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
						code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
					}
					else
						ifcmpaddr[ifcmp - 1] = cv - 4;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t;

					cv1 += 2;
				}
				else
				if (c == 1 && f == 2 && g == 3)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t + 1;
					code->function_code[curfunc][cv + 2] = 0;
					memcpy(code->function_code[curfunc] + cv + 3, &var2f, sizeof(float));

					uint8 x = code->function_code[curfunc][cv + 3];
					uint8 y = code->function_code[curfunc][cv + 4];

					code->function_code[curfunc][cv + 3] = code->function_code[curfunc][cv + 6];
					code->function_code[curfunc][cv + 4] = code->function_code[curfunc][cv + 5];
					code->function_code[curfunc][cv + 5] = y;
					code->function_code[curfunc][cv + 6] = x;

					code->function_code[curfunc][cv + 7] = 250;
					code->function_code[curfunc][cv + 8] = var1;
					code->function_code[curfunc][cv + 9] = 0;

					cv += 10 + 4;

					if (ifand != -1)
					{
						int32 cv2 = cv - 4;
						code->function_code[curfunc][cv2] = ifand >> 24;
						code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
						code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
						code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
					}
					else
						ifcmpaddr[ifcmp - 1] = cv - 4;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t + 1;
					code->bt_trl[curfunc][cv1 + 2] = 250;

					cv1 += 3;
				}
				else
				if (c == 1 && f == 3 && g == 3)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t + 1;
					code->function_code[curfunc][cv + 2] = var1;
					memcpy(code->function_code[curfunc] + cv + 3, &var2f, sizeof(float));

					uint8 x = code->function_code[curfunc][cv + 3];
					uint8 y = code->function_code[curfunc][cv + 4];

					code->function_code[curfunc][cv + 3] = code->function_code[curfunc][cv + 6];
					code->function_code[curfunc][cv + 4] = code->function_code[curfunc][cv + 5];
					code->function_code[curfunc][cv + 5] = y;
					code->function_code[curfunc][cv + 6] = x;

					cv += 7 + 4;

					if (ifand != -1)
					{
						int32 cv2 = cv - 4;
						code->function_code[curfunc][cv2] = ifand >> 24;
						code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
						code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
						code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
					}
					else
						ifcmpaddr[ifcmp - 1] = cv - 4;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t + 1;

					cv1 += 2;
				}
				else
				if (c == 0 && f == 0)
				{
					if (g == 3 && g1 != 3)
					{
						code->function_code[curfunc][cv] = t + 1;
						code->function_code[curfunc][cv + 1] = var1;
						code->function_code[curfunc][cv + 2] = g1 == 1 ? 0 : 1;
						code->function_code[curfunc][cv + 3] = var2 >> 24;
						code->function_code[curfunc][cv + 4] = (var2 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 5] = (var2 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 6] = var2 & 0xFF;

						cv += 7 + 4;

						if (ifand != -1)
						{
							int32 cv2 = cv - 4;
							code->function_code[curfunc][cv2] = ifand >> 24;
							code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
							code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
							code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
						}
						else
							ifcmpaddr[ifcmp - 1] = cv - 4;

						code->bt_trl[curfunc][cv1] = t + 1;

						cv1 += 1;
					}
					else
					if (g == 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = t + 2;
						code->function_code[curfunc][cv + 1] = var1;
						code->function_code[curfunc][cv + 2] = var2;

						cv += 3 + 4;

						if (ifand != -1)
						{
							int32 cv2 = cv - 4;
							code->function_code[curfunc][cv2] = ifand >> 24;
							code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
							code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
							code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
						}
						else
							ifcmpaddr[ifcmp - 1] = cv - 4;

						code->bt_trl[curfunc][cv1] = t + 2;

						cv1 += 1;
					}
					if (g != 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = t + 4;
						code->function_code[curfunc][cv + 1] = g == 1 ? 0 : 1;
						code->function_code[curfunc][cv + 2] = var1 >> 24;
						code->function_code[curfunc][cv + 3] = (var1 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 4] = (var1 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 5] = var1 & 0xFF;
						code->function_code[curfunc][cv + 6] = var2;

						cv += 7 + 4;

						if (ifand != -1)
						{
							int32 cv2 = cv - 4;
							code->function_code[curfunc][cv2] = ifand >> 24;
							code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
							code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
							code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
						}
						else
							ifcmpaddr[ifcmp - 1] = cv - 4;

						code->bt_trl[curfunc][cv1] = t + 4;

						cv1 += 1;
					}
				}
				else
				if (c == 0 && f == 1)
				{
					if (g == 3 && g1 != 3)
					{
						code->function_code[curfunc][cv] = 255;
						code->function_code[curfunc][cv + 1] = t + 3;
						code->function_code[curfunc][cv + 2] = var1;
						code->function_code[curfunc][cv + 3] = g1 == 1 ? 0 : 1;
						code->function_code[curfunc][cv + 4] = var2 >> 24;
						code->function_code[curfunc][cv + 5] = (var2 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 6] = (var2 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 7] = var2 & 0xFF;

						cv += 8 + 4;

						if (ifand != -1)
						{
							int32 cv2 = cv - 4;
							code->function_code[curfunc][cv2] = ifand >> 24;
							code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
							code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
							code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
						}
						else
							ifcmpaddr[ifcmp - 1] = cv - 4;

						code->bt_trl[curfunc][cv1] = 255;
						code->bt_trl[curfunc][cv1 + 1] = t + 3;

						cv1 += 2;
					}
					else
					if (g == 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = 255;
						code->function_code[curfunc][cv + 1] = t + 2;
						code->function_code[curfunc][cv + 2] = var1;
						code->function_code[curfunc][cv + 3] = var2;

						cv += 4 + 4;

						if (ifand != -1)
						{
							int32 cv2 = cv - 4;
							code->function_code[curfunc][cv2] = ifand >> 24;
							code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
							code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
							code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
						}
						else
							ifcmpaddr[ifcmp - 1] = cv - 4;

						code->bt_trl[curfunc][cv1] = 255;
						code->bt_trl[curfunc][cv1 + 1] = t + 5;

						cv1 += 2;
					}
					if (g != 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = 251;
						code->function_code[curfunc][cv + 1] = 0;
						code->function_code[curfunc][cv + 2] = var2;
						code->function_code[curfunc][cv + 3] = 255;
						code->function_code[curfunc][cv + 4] = t + 7;
						code->function_code[curfunc][cv + 5] = g == 1 ? 0 : 1;
						code->function_code[curfunc][cv + 6] = var1 >> 24;
						code->function_code[curfunc][cv + 7] = (var1 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 8] = (var1 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 9] = var1 & 0xFF;
						code->function_code[curfunc][cv + 10] = 0;

						cv += 11 + 4;

						if (ifand != -1)
						{
							int32 cv2 = cv - 4;
							code->function_code[curfunc][cv2] = ifand >> 24;
							code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
							code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
							code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
						}
						else
							ifcmpaddr[ifcmp - 1] = cv - 4;

						code->bt_trl[curfunc][cv1] = 251;
						code->bt_trl[curfunc][cv1 + 1] = 255;
						code->bt_trl[curfunc][cv1 + 2] = t + 7;

						cv1 += 3;
					}
				}
				else
				if (c == 0 && f == 2)
				{
					if (g == 3 && g1 != 3)
					{
						code->function_code[curfunc][cv] = 255;
						code->function_code[curfunc][cv + 1] = t + 4;
						code->function_code[curfunc][cv + 2] = 0;
						code->function_code[curfunc][cv + 3] = g1 == 0 ? 1 : 1;
						code->function_code[curfunc][cv + 4] = var2 >> 24;
						code->function_code[curfunc][cv + 5] = (var2 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 6] = (var2 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 7] = var2 & 0xFF;
						code->function_code[curfunc][cv + 8] = 250;
						code->function_code[curfunc][cv + 9] = var1;
						code->function_code[curfunc][cv + 10] = 0;

						cv += 11 + 4;

						if (ifand != -1)
						{
							int32 cv2 = cv - 4;
							code->function_code[curfunc][cv2] = ifand >> 24;
							code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
							code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
							code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
						}
						else
							ifcmpaddr[ifcmp - 1] = cv - 4;

						code->bt_trl[curfunc][cv1] = 255;
						code->bt_trl[curfunc][cv1 + 1] = t + 4;
						code->bt_trl[curfunc][cv1 + 2] = 250;

						cv1 += 3;
					}
					else
					if (g == 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = 255;
						code->function_code[curfunc][cv + 1] = t + 5;
						code->function_code[curfunc][cv + 2] = 0;
						code->function_code[curfunc][cv + 3] = var2;
						code->function_code[curfunc][cv + 4] = 250;
						code->function_code[curfunc][cv + 5] = var1;
						code->function_code[curfunc][cv + 6] = 0;

						cv += 7 + 4;

						if (ifand != -1)
						{
							int32 cv2 = cv - 4;
							code->function_code[curfunc][cv2] = ifand >> 24;
							code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
							code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
							code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
						}
						else
							ifcmpaddr[ifcmp - 1] = cv - 4;

						code->bt_trl[curfunc][cv1] = 255;
						code->bt_trl[curfunc][cv1 + 1] = t + 5;
						code->bt_trl[curfunc][cv1 + 2] = 250;

						cv1 += 3;
					}
					if (g != 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = 250;
						code->function_code[curfunc][cv + 1] = 6;
						code->function_code[curfunc][cv + 2] = var2;
						code->function_code[curfunc][cv + 3] = t + 4;
						code->function_code[curfunc][cv + 4] = g == 1 ? 0 : 1;
						code->function_code[curfunc][cv + 5] = var1 >> 24;
						code->function_code[curfunc][cv + 6] = (var1 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 7] = (var1 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 8] = var1 & 0xFF;
						code->function_code[curfunc][cv + 9] = 6;

						cv += 10 + 4;

						if (ifand != -1)
						{
							int32 cv2 = cv - 4;
							code->function_code[curfunc][cv2] = ifand >> 24;
							code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
							code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
							code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
						}
						else
							ifcmpaddr[ifcmp - 1] = cv - 4;

						code->bt_trl[curfunc][cv1] = 250;
						code->bt_trl[curfunc][cv1 + 1] = t + 4;

						cv1 += 2;
					}
				}
				else
				if (c == 0 && f == 3)
				{
					if (g == 3 && g1 != 3)
					{
						code->function_code[curfunc][cv] = 255;
						code->function_code[curfunc][cv + 1] = t + 4;
						code->function_code[curfunc][cv + 2] = var1;
						code->function_code[curfunc][cv + 3] = g1 == 1 ? 0 : 1;
						code->function_code[curfunc][cv + 4] = var2 >> 24;
						code->function_code[curfunc][cv + 5] = (var2 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 6] = (var2 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 7] = var2 & 0xFF;

						cv += 7 + 4;

						if (ifand != -1)
						{
							int32 cv2 = cv - 4;
							code->function_code[curfunc][cv2] = ifand >> 24;
							code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
							code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
							code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
						}
						else
							ifcmpaddr[ifcmp - 1] = cv - 4;

						code->bt_trl[curfunc][cv1] = 255;
						code->bt_trl[curfunc][cv1 + 1] = t + 4;

						cv1 += 2;
					}
					else
					if (g == 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = 255;
						code->function_code[curfunc][cv + 1] = t + 5;
						code->function_code[curfunc][cv + 2] = var1;
						code->function_code[curfunc][cv + 3] = var2;

						cv += 4 + 4;

						if (ifand != -1)
						{
							int32 cv2 = cv - 4;
							code->function_code[curfunc][cv2] = ifand >> 24;
							code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
							code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
							code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
						}
						else
							ifcmpaddr[ifcmp - 1] = cv - 4;

						code->bt_trl[curfunc][cv1] = 255;
						code->bt_trl[curfunc][cv1 + 1] = t + 5;

						cv1 += 2;
					}
					if (g != 3 && g1 == 3)
					{
						code->function_code[curfunc][cv] = 255;
						code->function_code[curfunc][cv + 1] = t + 7;
						code->function_code[curfunc][cv + 2] = g == 1 ? 0 : 1;
						code->function_code[curfunc][cv + 3] = var1 >> 24;
						code->function_code[curfunc][cv + 4] = (var1 >> 16) & 0xFF;
						code->function_code[curfunc][cv + 5] = (var1 >> 8) & 0xFF;
						code->function_code[curfunc][cv + 6] = var1 & 0xFF;
						code->function_code[curfunc][cv + 7] = var2;

						cv += 8 + 4;

						if (ifand != -1)
						{
							int32 cv2 = cv - 4;
							code->function_code[curfunc][cv2] = ifand >> 24;
							code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
							code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
							code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
						}
						else
							ifcmpaddr[ifcmp - 1] = cv - 4;

						code->bt_trl[curfunc][cv1] = 255;
						code->bt_trl[curfunc][cv1 + 1] = t + 7;

						cv1 += 2;
					}
				}
			}
			else
			{
				if (c == 1 && f == 0)
				{
					code->function_code[curfunc][cv] = t + 3;
					code->function_code[curfunc][cv + 1] = g == 1 ? 0 : 1;
					code->function_code[curfunc][cv + 2] = var1 >> 24;
					code->function_code[curfunc][cv + 3] = (var1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 4] = (var1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 5] = var1 & 0xFF;
					code->function_code[curfunc][cv + 6] = var2 >> 24;
					code->function_code[curfunc][cv + 7] = (var2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 8] = (var2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 9] = var2 & 0xFF;

					cv += 10 + 4;

					if (ifand != -1)
					{
						int32 cv2 = cv - 4;
						code->function_code[curfunc][cv2] = ifand >> 24;
						code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
						code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
						code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
					}
					else
						ifcmpaddr[ifcmp - 1] = cv - 4;

					code->bt_trl[curfunc][cv1] = t + 3;

					cv1 += 1;
				}
				else
				if (c == 1 && f == 1)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t + 1;
					code->function_code[curfunc][cv + 2] = 0;
					memcpy(code->function_code[curfunc] + cv + 3, &var2f, sizeof(float));

					uint8 x = code->function_code[curfunc][cv + 3];
					uint8 y = code->function_code[curfunc][cv + 4];

					code->function_code[curfunc][cv + 3] = code->function_code[curfunc][cv + 6];
					code->function_code[curfunc][cv + 4] = code->function_code[curfunc][cv + 5];
					code->function_code[curfunc][cv + 5] = y;
					code->function_code[curfunc][cv + 6] = x;

					code->function_code[curfunc][cv + 7] = 250;
					code->function_code[curfunc][cv + 8] = 6;
					code->function_code[curfunc][cv + 9] = 0;

					code->function_code[curfunc][cv + 10] = t + 4;
					code->function_code[curfunc][cv + 11] = g == 1 ? 0 : 1;
					code->function_code[curfunc][cv + 12] = var1 >> 24;
					code->function_code[curfunc][cv + 13] = (var1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 14] = (var1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 15] = var1 & 0xFF;
					code->function_code[curfunc][cv + 16] = 6;

					cv += 17 + 4;

					if (ifand != -1)
					{
						int32 cv2 = cv - 4;
						code->function_code[curfunc][cv2] = ifand >> 24;
						code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
						code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
						code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
					}
					else
						ifcmpaddr[ifcmp - 1] = cv - 4;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t + 1;
					code->bt_trl[curfunc][cv1 + 2] = 250;
					code->bt_trl[curfunc][cv1 + 3] = t + 4;

					cv1 += 4;
				}
				else
				if (c == 1 && f == 2)
				{
					code->function_code[curfunc][cv] = t + 0;
					code->function_code[curfunc][cv + 1] = 6;
					code->function_code[curfunc][cv + 2] = var2 >> 24;
					code->function_code[curfunc][cv + 3] = (var2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 4] = (var2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 5] = var2 & 0xFF;

					code->function_code[curfunc][cv + 6] = 251;
					code->function_code[curfunc][cv + 7] = 0;
					code->function_code[curfunc][cv + 8] = 6;

					code->function_code[curfunc][cv + 9] = 255;
					code->function_code[curfunc][cv + 10] = t + 7;
					code->function_code[curfunc][cv + 11] = g == 1 ? 0 : 1;
					code->function_code[curfunc][cv + 12] = var1 >> 24;
					code->function_code[curfunc][cv + 13] = (var1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 14] = (var1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 15] = var1 & 0xFF;
					code->function_code[curfunc][cv + 16] = 0;

					cv += 17 + 4;

					if (ifand != -1)
					{
						int32 cv2 = cv - 4;
						code->function_code[curfunc][cv2] = ifand >> 24;
						code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
						code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
						code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
					}
					else
						ifcmpaddr[ifcmp - 1] = cv - 4; if (ifand != -1)
					{
						int32 cv2 = cv - 4;
						code->function_code[curfunc][cv2] = ifand >> 24;
						code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
						code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
						code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
					}
						else
							ifcmpaddr[ifcmp - 1] = cv - 4;

					code->bt_trl[curfunc][cv1] = t + 0;
					code->bt_trl[curfunc][cv1 + 1] = 251;
					code->bt_trl[curfunc][cv1 + 2] = 255;
					code->bt_trl[curfunc][cv1 + 3] = t + 7;

					cv1 += 4;
				}
				else
				if (c == 1 && f == 3)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t + 9;
					code->function_code[curfunc][cv + 2] = g == 1 ? 0 : 1;
					code->function_code[curfunc][cv + 3] = var1 >> 24;
					code->function_code[curfunc][cv + 4] = (var1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 5] = (var1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 6] = var1 & 0xFF;
					code->function_code[curfunc][cv + 7] = g1 == 1 ? 0 : 1;
					code->function_code[curfunc][cv + 8] = var2 >> 24;
					code->function_code[curfunc][cv + 9] = (var2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 10] = (var2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 11] = var2 & 0xFF;

					cv += 12 + 4;

					if (ifand != -1)
					{
						int32 cv2 = cv - 4;
						code->function_code[curfunc][cv2] = ifand >> 24;
						code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
						code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
						code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
					}
					else
						ifcmpaddr[ifcmp - 1] = cv - 4;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t + 9;

					cv1 += 2;
				}
				else
				if (c == 0 && f == 1)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t + 8;
					code->function_code[curfunc][cv + 2] = g == 1 ? 0 : 1;
					code->function_code[curfunc][cv + 3] = var1 >> 24;
					code->function_code[curfunc][cv + 4] = (var1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 5] = (var1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 6] = var1 & 0xFF;
					code->function_code[curfunc][cv + 7] = g1 == 1 ? 0 : 1;
					code->function_code[curfunc][cv + 8] = var2 >> 24;
					code->function_code[curfunc][cv + 9] = (var2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 10] = (var2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 11] = var2 & 0xFF;

					cv += 12 + 4;

					if (ifand != -1)
					{
						int32 cv2 = cv - 4;
						code->function_code[curfunc][cv2] = ifand >> 24;
						code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
						code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
						code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
					}
					else
						ifcmpaddr[ifcmp - 1] = cv - 4;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t + 8;

					cv1 += 2;
				}
				else
				if (c == 0 && f == 2)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t + 4;
					code->function_code[curfunc][cv + 2] = 0;
					code->function_code[curfunc][cv + 3] = g1 & 1 == 1 ? 0 : 1;
					code->function_code[curfunc][cv + 4] = var2 >> 24;
					code->function_code[curfunc][cv + 5] = (var2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 6] = (var2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 7] = var2 & 0xFF;

					code->function_code[curfunc][cv + 8] = 250;
					code->function_code[curfunc][cv + 9] = 6;
					code->function_code[curfunc][cv + 10] = 0;

					code->function_code[curfunc][cv + 11] = t + 4;
					code->function_code[curfunc][cv + 12] = g == 1 ? 0 : 1;
					code->function_code[curfunc][cv + 13] = var1 >> 24;
					code->function_code[curfunc][cv + 14] = (var1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 15] = (var1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 16] = var1 & 0xFF;
					code->function_code[curfunc][cv + 17] = 6;

					cv += 18 + 4;
					if (ifand != -1)
					{
						int32 cv2 = cv - 4;
						code->function_code[curfunc][cv2] = ifand >> 24;
						code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
						code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
						code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
					}
					else
						ifcmpaddr[ifcmp - 1] = cv - 4;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t + 4;
					code->bt_trl[curfunc][cv1 + 2] = 250;
					code->bt_trl[curfunc][cv1 + 3] = t + 4;

					cv1 += 4;
				}
				else
				if (c == 0 && f == 3)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = t + 9;
					code->function_code[curfunc][cv + 2] = g == 1 ? 0 : 1;
					code->function_code[curfunc][cv + 3] = var1 >> 24;
					code->function_code[curfunc][cv + 4] = (var1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 5] = (var1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 6] = var1 & 0xFF;
					code->function_code[curfunc][cv + 7] = g1 == 1 ? 1 : 1;
					code->function_code[curfunc][cv + 8] = var2 >> 24;
					code->function_code[curfunc][cv + 9] = (var2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 10] = (var2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 11] = var2 & 0xFF;

					cv += 12 + 4;
					if (ifand != -1)
					{
						int32 cv2 = cv - 4;
						code->function_code[curfunc][cv2] = ifand >> 24;
						code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
						code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
						code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
					}
					else
						ifcmpaddr[ifcmp - 1] = cv - 4;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = t + 9;

					cv1 += 1;
				}
				else
				if (c == 0 && f == 0)
				{
					code->function_code[curfunc][cv] = t + 5;
					code->function_code[curfunc][cv + 1] = g == 1 ? 0 : 1;
					code->function_code[curfunc][cv + 2] = var1 >> 24;
					code->function_code[curfunc][cv + 3] = (var1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 4] = (var1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 5] = var1 & 0xFF;
					code->function_code[curfunc][cv + 6] = g1 == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 7] = var2 >> 24;
					code->function_code[curfunc][cv + 8] = (var2 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 9] = (var2 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 10] = var2 & 0xFF;

					cv += 11 + 4;
					if (ifand != -1)
					{
						int32 cv2 = cv - 4;
						code->function_code[curfunc][cv2] = ifand >> 24;
						code->function_code[curfunc][cv2 + 1] = (ifand >> 16) & 0xFF;
						code->function_code[curfunc][cv2 + 2] = (ifand >> 8) & 0xFF;
						code->function_code[curfunc][cv2 + 3] = ifand & 0xFF;
					}
					else
						ifcmpaddr[ifcmp - 1] = cv - 4;

					code->bt_trl[curfunc][cv1] = t + 5;

					cv1 += 1;
				}
			}

			code->linecode[line].byte_end = cv;
		}
		else //function calling withou call operator
		{
			strcpy(code->linecode[line].code, buf);
			code->linecode[line].func = curfunc;

			int a = 0;

			for (i = 0; i < code->num_functions; i++)
			{
				if (strcmp(tok, code->function_table[i].name) == NULL)
				{
					//Found function
					a = 1;
					break;
				}
			}

			if (a == 0)
			{
				if ((i = GetEngFunc(tok)) != -1)
				{
					//Found engine function
					a = 2;
				}
			}

			if (a == 0)
			{
				error++;
				LogApp("Found undefined token \"%s\" at line: %d", tok, line);
				continue;
			}
			else
			{
				if (func == 0)
				{
					error++;
					LogApp("Compiler error: function call outside function declaration - line: %d", line);
					continue;
				}
			}

			strcpy(str1, tok);

			int c = 0, g = 0, f = 0, j = 0, b = 0, vars[24], var_c = 0;
			val1 = -1;

			ZeroMem(vars, 24 * sizeof(int));

			code->linecode[line].byte_start = cv;

			while ((tok = strtok(NULL, " \n\t")) != NULL)
			{
				c = 0;
				g = 0;
				f = 0;
				if (IsNumber(tok) == 0)
				{
					//It's a variable
					for (j = 0; j < code->num_vars; j++)
					{
						if (strcmp(tok, code->vars_table[j].name) == NULL)
						{
							val1 = j;

							g = 2;

							if (code->vars_table[j].type == 2)
								f = 1;

							vars[var_c] = j;
							var_c++;

							break;
						}
					}

					//if (g == 2)
					//{
						for (j = 0; j < code->function_table[curfunc].num_vars; j++)
						{
							if (strcmp(tok, code->function_table[curfunc].vars_table[j].name) == NULL)
							{
								val1 = j;

								g = 1;

								if (code->function_table[curfunc].vars_table[j].type == 2)
									f = 1;

								vars[var_c] = j * -1;
								var_c++;

								break;
							}
						}

						if (g == 0)
						{
							error++;
							LogApp("Compiler error: undefined keyword \"%s\" at call command - line: %d", tok, line);
							c = -1;
							break;
						}
					//}
				}
				else
				{
					c = 1;

					if (IsNumberFloat(tok) == 1)
					{
						valf1 = atof(tok);
						f = 1;
					}
					else
						val1 = atol(tok);
				}

				CheckCodeSize(code, curfunc, cv);

				if (g == 2)
					g = 0;

				if (c == 1 && f == 0)
				{
					code->function_code[curfunc][cv] = 0;
					code->function_code[curfunc][cv + 1] = b + 9;
					code->function_code[curfunc][cv + 2] = val1 >> 24;
					code->function_code[curfunc][cv + 3] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 4] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 5] = val1 & 0xFF;

					cv += 6;

					code->bt_trl[curfunc][cv1] = 0;

					cv1++;
				}
				else
				if (c == 1 && f == 1)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = 1;
					code->function_code[curfunc][cv + 2] = b + 5;
					CopyFloatToInt32(code->function_code[curfunc] + cv + 3, valf1);

					cv += 7;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = 1;

					cv1 += 2;
				}
				else
				if (c == 0 && f == 0)
				{
					code->function_code[curfunc][cv] = 1;
					code->function_code[curfunc][cv + 1] = b + 9;
					code->function_code[curfunc][cv + 2] = g == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 3] = val1 >> 24;
					code->function_code[curfunc][cv + 4] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 5] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 6] = val1 & 0xFF;

					cv += 7;

					code->bt_trl[curfunc][cv1] = 1;

					cv1++;
				}
				else
				if (c == 0 && f == 1)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = 4;
					code->function_code[curfunc][cv + 2] = b + 5;
					code->function_code[curfunc][cv + 3] = g == 1 ? 1 : 0;
					code->function_code[curfunc][cv + 4] = val1 >> 24;
					code->function_code[curfunc][cv + 5] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 6] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 7] = val1 & 0xFF;

					cv += 8;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = 4;

					cv1 += 2;
				}

				b++;
			}

			CheckCodeSize(code, curfunc, cv);

			//If MGL function
			if (a == 1)
			{
				if (b < code->function_table[i].num_args)
				{
					error++;
					LogApp("Compiler error: missing arguments in call to \"%s\" - line: %d", str1, line);
					continue;
				}

				code->function_code[curfunc][cv] = 204;

				//i = code->function_table[i].address;

				code->function_code[curfunc][cv + 1] = 0;
				code->function_code[curfunc][cv + 2] = i >> 24;
				code->function_code[curfunc][cv + 3] = (i >> 16) & 0xFF;
				code->function_code[curfunc][cv + 4] = (i >> 8) & 0xFF;
				code->function_code[curfunc][cv + 5] = i & 0xFF;

				cv += 6;

				code->bt_trl[curfunc][cv1] = 204;

				cv1++;
			}
			else
			{
				//It's an engine call

				if (b < e_funcs[i].num_args)
				{
					error++;
					LogApp("Compiler error: missing arguments in call to \"%s\" - line: %d", str1, line);
					continue;
				}

				code->function_code[curfunc][cv] = 204;
				code->function_code[curfunc][cv + 1] = 1;
				code->function_code[curfunc][cv + 2] = i >> 24;
				code->function_code[curfunc][cv + 3] = (i >> 16) & 0xFF;
				code->function_code[curfunc][cv + 4] = (i >> 8) & 0xFF;
				code->function_code[curfunc][cv + 5] = i & 0xFF;

				cv += 6;

				code->bt_trl[curfunc][cv1] = 204;

				cv1++;

				code->function_table[curfunc].num_use++;

				if (e_funcs[i].returnv == 1)
				{
					int k = 0;
					for (j = 0; j < var_c; j++)
					{
						CheckCodeSize(code, curfunc, cv);

						if (vars[j] < 0) //Local
						{
							k = vars[j] * -1;

							if (code->function_table[curfunc].vars_table[k].type == 2)
							{
								code->function_code[curfunc][cv] = 255;
								code->function_code[curfunc][cv + 1] = 7;
								code->function_code[curfunc][cv + 2] = 1;
								code->function_code[curfunc][cv + 3] = k >> 24;
								code->function_code[curfunc][cv + 4] = (k >> 16) & 0xFF;
								code->function_code[curfunc][cv + 5] = (k >> 8) & 0xFF;
								code->function_code[curfunc][cv + 6] = k & 0xFF;
								code->function_code[curfunc][cv + 7] = j + 5;

								cv += 8;

								code->bt_trl[curfunc][cv1] = 255;
								code->bt_trl[curfunc][cv1 + 1] = 7;

								cv1 += 2;
							}
							else
							{
								code->function_code[curfunc][cv] = 4;
								code->function_code[curfunc][cv + 1] = 1;
								code->function_code[curfunc][cv + 2] = k >> 24;
								code->function_code[curfunc][cv + 3] = (k >> 16) & 0xFF;
								code->function_code[curfunc][cv + 4] = (k >> 8) & 0xFF;
								code->function_code[curfunc][cv + 5] = k & 0xFF;
								code->function_code[curfunc][cv + 6] = j + 9;

								cv += 7;

								code->bt_trl[curfunc][cv1] = 4;

								cv1++;
							}
						}
					}
				}
			}

			code->linecode[line].byte_end = cv;
		}
	}

	fclose(file);

	if (func == 1)
	{
		error++;
		LogApp("Compiler error: missing } in function \"%s\" - line: %d", code->function_table[curfunc].name, line - 1);
	}

	if (error > 0)
	{
		LogApp("Compiler: found %d errors - compilling process was stopped", error);
		
		for (i = 0; i < code->num_functions; i++)
		{
			free(code->bt_trl[i]);
			free(code->function_code[i]);

			if (code->function_table[i].num_vars > 0)
				free(code->function_table[i].vars_table);
		}

		free(code->bt_trl);
		free(code->function_code);
		free(code->function_table);
		free(code->vars_table);

		return NULL;
	}

	size_t tsize = 0;

	for (i = 0; i < code->num_functions; i++)
		tsize += code->function_table[i].size;

	LogApp("Compiled %d functions and %d global variables  - total size: %d", code->num_functions, code->num_vars, tsize);

	LogApp("Compiling process done - no errors detected");

	return code;
}

unsigned char *LinkMGL(MGMC *code, uint8 optimization)
{
	register int32 i = 0, j = 0, cv = 0, cv1 = 0, cv2 = 0;

	uint32 line = 0, errors = 0, status = 0;

	int32 temp;

	mem_assert(code);

	unsigned char *c = calloc(2048, 1);
	CHECKMEM(c);

	size_t csize = 2048;

	//Header
	c[0] = 'M';
	c[1] = 'G';
	c[2] = 'L';
	c[3] = ' ';

	//Bytes 4 - 23 holds the entry points
	//Byte 24 - holds the stack type - next 4 bytes the size if its a custom type

	if (code->stacksize > 6)
	{
		c[24] = 7;
		c[25] = code->stacksize >> 24;
		c[26] = (code->stacksize >> 16) & 0xFF;
		c[27] = (code->stacksize >> 8) & 0xFF;
		c[28] = code->stacksize & 0xFF;
	}
	else
		c[24] = code->stacksize;

	cv = 37;
	cv1 = 0;
	cv2 = 0;

	for (j = 0; j < code->num_vars; j++)
	{
		c[cv] = 254;
		c[cv + 1] = code->vars_table[j].type;

		if (c[cv + 1] == 1)
			Copy32toBuf(c + cv + 2, code->vars_table[j].value);

		if (c[cv + 1] == 2)
			CopyFloatToInt32(c + cv + 2, code->vars_table[j].valuef);

		if (c[cv + 1] == 4)
		{
			Copy32toBuf(c + cv + 2, code->vars_table[j].len);
			memcpy(c + cv + 6, code->vars_table[j].string, code->vars_table[j].len);
		}

		if (c[cv + 1] != 4)
			cv += 6;
		else
			cv += 6 + code->vars_table[j].len;
	}

	for (i = 0; i < code->num_functions; i++)
	{
		if (cv > csize - 48)
		{
			c = realloc(c, csize + 128);
			CHECKMEM(c);
			csize += 128;
		}

		code->function_table[i].address = cv;

		if (strcmp(code->function_table[i].name, "Init") == NULL)
		{
			c[4] = cv >> 24;
			c[5] = (cv >> 16) & 0xFF;
			c[6] = (cv >> 8) & 0xFF;
			c[7] = cv & 0xFF;

			status |= 2;
		}
		else
		if (strcmp(code->function_table[i].name, "PreGame") == NULL)
		{
			c[8] = cv >> 24;
			c[9] = (cv >> 16) & 0xFF;
			c[10] = (cv >> 8) & 0xFF;
			c[11] = cv & 0xFF;

			status |= 4;
		}
		else
		if (strcmp(code->function_table[i].name, "GameLoop") == NULL)
		{
			c[12] = cv >> 24;
			c[13] = (cv >> 16) & 0xFF;
			c[14] = (cv >> 8) & 0xFF;
			c[15] = cv & 0xFF;

			status |= 8;
		}
		else
		if (strcmp(code->function_table[i].name, "MainLoop") == NULL)
		{
			c[16] = cv >> 24;
			c[17] = (cv >> 16) & 0xFF;
			c[18] = (cv >> 8) & 0xFF;
			c[19] = cv & 0xFF;

			status |= 16;
		}
		else
		if (strcmp(code->function_table[i].name, "End") == NULL)
		{
			c[20] = cv >> 24;
			c[21] = (cv >> 16) & 0xFF;
			c[22] = (cv >> 8) & 0xFF;
			c[23] = cv & 0xFF;

			status |= 32;
		}

		for (j = 0; j < code->function_table[i].num_vars; j++)
		{
			if (cv > csize - 48)
			{
				c = realloc(c, csize + 128);
				CHECKMEM(c);
				csize += 128;
			}

			c[cv] = 254;
			c[cv + 1] = code->function_table[i].vars_table[j].type;

			if (c[cv + 1] == 1)
				Copy32toBuf(c + cv + 2, code->function_table[i].vars_table[j].value);
			
			if (c[cv + 1] == 2)
				CopyFloatToInt32(c + cv + 2, code->function_table[i].vars_table[j].valuef);

			if (c[cv + 1] == 4)
			{
				Copy32toBuf(c + cv + 2, code->function_table[i].vars_table[j].len);
				memcpy(c + cv + 6, code->function_table[i].vars_table[j].string, code->function_table[i].vars_table[j].len);
			}

			if (c[cv + 1] != 4)
				cv += 6;
			else
				cv += 6 + code->function_table[i].vars_table[j].len;
		}

		for (cv1 = 0; cv1 < code->function_table[i].size; )
		{
			if (cv > csize - 48)
			{
				c = realloc(c, csize + 128);
				CHECKMEM(c);
				csize += 128;
			}

			c[cv] = code->function_code[i][cv1];

			if (c[cv] < 56)
			{
				switch (c[cv] % 10)
				{
					case 0: //6 bytes
						c[cv + 1] = code->function_code[i][cv1 + 1];
						c[cv + 2] = code->function_code[i][cv1 + 2];
						c[cv + 3] = code->function_code[i][cv1 + 3];
						c[cv + 4] = code->function_code[i][cv1 + 4];
						c[cv + 5] = code->function_code[i][cv1 + 5];
						cv += 6;
						cv1 += 6;
						break;

					case 1: //7 bytes
					case 4:
						c[cv + 1] = code->function_code[i][cv1 + 1];
						c[cv + 2] = code->function_code[i][cv1 + 2];
						c[cv + 3] = code->function_code[i][cv1 + 3];
						c[cv + 4] = code->function_code[i][cv1 + 4];
						c[cv + 5] = code->function_code[i][cv1 + 5];
						c[cv + 6] = code->function_code[i][cv1 + 6];
						cv += 7;
						cv1 += 7;
						break;

					case 2: //3 bytes
						c[cv + 1] = code->function_code[i][cv1 + 1];
						c[cv + 2] = code->function_code[i][cv1 + 2];
						cv += 3;
						cv1 += 3;
						break;

					case 3: //10 bytes
						c[cv + 1] = code->function_code[i][cv1 + 1];
						c[cv + 2] = code->function_code[i][cv1 + 2];
						c[cv + 3] = code->function_code[i][cv1 + 3];
						c[cv + 4] = code->function_code[i][cv1 + 4];
						c[cv + 5] = code->function_code[i][cv1 + 5];
						c[cv + 6] = code->function_code[i][cv1 + 6];
						c[cv + 7] = code->function_code[i][cv1 + 7];
						c[cv + 8] = code->function_code[i][cv1 + 8];
						c[cv + 9] = code->function_code[i][cv1 + 9];
						cv += 10;
						cv1 += 10;
						break;

					case 5: //11 bytes
						c[cv + 1] = code->function_code[i][cv1 + 1];
						c[cv + 2] = code->function_code[i][cv1 + 2];
						c[cv + 3] = code->function_code[i][cv1 + 3];
						c[cv + 4] = code->function_code[i][cv1 + 4];
						c[cv + 5] = code->function_code[i][cv1 + 5];
						c[cv + 6] = code->function_code[i][cv1 + 6];
						c[cv + 7] = code->function_code[i][cv1 + 7];
						c[cv + 8] = code->function_code[i][cv1 + 8];
						c[cv + 9] = code->function_code[i][cv1 + 9];
						c[cv + 10] = code->function_code[i][cv1 + 10];
						cv += 11;
						cv1 += 11;
						break;
				}

				continue;
			}
			else
			if (c[cv] < 202 && c[cv] > 141)
			{
				switch ((c[cv] % 10) - 2)
				{
				case 0: //6 bytes
					c[cv + 1] = code->function_code[i][cv1 + 1];
					c[cv + 2] = code->function_code[i][cv1 + 2];
					c[cv + 3] = code->function_code[i][cv1 + 3];
					c[cv + 4] = code->function_code[i][cv1 + 4];
					c[cv + 5] = code->function_code[i][cv1 + 5];

					temp = (code->function_code[i][cv1 + 6] << 24) | (code->function_code[i][cv1 + 7] << 16) |
						(code->function_code[i][cv1 + 8] << 8) | code->function_code[i][cv1 + 9];
					temp = temp == 0 || temp  == 1 ? temp : cv + (temp - cv1);

					c[cv + 6] = temp >> 24;
					c[cv + 7] = (temp >> 16) & 0xFF;
					c[cv + 8] = (temp >> 8) & 0xFF;
					c[cv + 9] = temp & 0xFF;

					cv += 6 + 4;
					cv1 += 6 + 4;
					break;

				case 1: //7 bytes
				case 4:
					c[cv + 1] = code->function_code[i][cv1 + 1];
					c[cv + 2] = code->function_code[i][cv1 + 2];
					c[cv + 3] = code->function_code[i][cv1 + 3];
					c[cv + 4] = code->function_code[i][cv1 + 4];
					c[cv + 5] = code->function_code[i][cv1 + 5];
					c[cv + 6] = code->function_code[i][cv1 + 6];

					temp = (code->function_code[i][cv1 + 7] << 24) | (code->function_code[i][cv1 + 8] << 16) |
						(code->function_code[i][cv1 + 9] << 8) | code->function_code[i][cv1 + 10];
					temp = temp == 0 || temp == 1 ? temp : cv + (temp - cv1);

					c[cv + 7] = temp >> 24;
					c[cv + 8] = (temp >> 16) & 0xFF;
					c[cv + 9] = (temp >> 8) & 0xFF;
					c[cv + 10] = temp & 0xFF;

					cv += 7 + 4;
					cv1 += 7 + 4;
					break;

				case 2: //3 bytes
					c[cv + 1] = code->function_code[i][cv1 + 1];
					c[cv + 2] = code->function_code[i][cv1 + 2];

					temp = (code->function_code[i][cv1 + 3] << 24) | (code->function_code[i][cv1 + 4] << 16) |
						(code->function_code[i][cv1 + 5] << 8) | code->function_code[i][cv1 + 6];
					temp = temp == 0 || temp == 1 ? temp : cv + (temp - cv1);

					c[cv + 3] = temp >> 24;
					c[cv + 4] = (temp >> 16) & 0xFF;
					c[cv + 5] = (temp >> 8) & 0xFF;
					c[cv + 6] = temp & 0xFF;

					cv += 3 + 4;
					cv1 += 3 + 4;
					break;

				case 3: //10 bytes
					c[cv + 1] = code->function_code[i][cv1 + 1];
					c[cv + 2] = code->function_code[i][cv1 + 2];
					c[cv + 3] = code->function_code[i][cv1 + 3];
					c[cv + 4] = code->function_code[i][cv1 + 4];
					c[cv + 5] = code->function_code[i][cv1 + 5];
					c[cv + 6] = code->function_code[i][cv1 + 6];
					c[cv + 7] = code->function_code[i][cv1 + 7];
					c[cv + 8] = code->function_code[i][cv1 + 8];
					c[cv + 9] = code->function_code[i][cv1 + 9];

					temp = (code->function_code[i][cv1 + 10] << 24) | (code->function_code[i][cv1 + 11] << 16) |
						(code->function_code[i][cv1 + 12] << 8) | code->function_code[i][cv1 + 13];
					temp = temp == 0 || temp == 1 ? temp : cv + (temp - cv1);

					c[cv + 10] = temp >> 24;
					c[cv + 11] = (temp >> 16) & 0xFF;
					c[cv + 12] = (temp >> 8) & 0xFF;
					c[cv + 13] = temp & 0xFF;

					cv += 14;
					cv1 += 14;
					break;

				case 5: //11 bytes
					c[cv + 1] = code->function_code[i][cv1 + 1];
					c[cv + 2] = code->function_code[i][cv1 + 2];
					c[cv + 3] = code->function_code[i][cv1 + 3];
					c[cv + 4] = code->function_code[i][cv1 + 4];
					c[cv + 5] = code->function_code[i][cv1 + 5];
					c[cv + 6] = code->function_code[i][cv1 + 6];
					c[cv + 7] = code->function_code[i][cv1 + 7];
					c[cv + 8] = code->function_code[i][cv1 + 8];
					c[cv + 9] = code->function_code[i][cv1 + 9];
					c[cv + 10] = code->function_code[i][cv1 + 10];

					temp = (code->function_code[i][cv1 + 11] << 24) | (code->function_code[i][cv1 + 12] << 16) |
						(code->function_code[i][cv1 + 13] << 8) | code->function_code[i][cv1 + 14];
					temp = temp == 0 || temp == 1 ? temp : cv + (temp - cv1);

					c[cv + 11] = temp >> 24;
					c[cv + 12] = (temp >> 16) & 0xFF;
					c[cv + 13] = (temp >> 8) & 0xFF;
					c[cv + 14] = temp & 0xFF;

					cv += 11 + 4;
					cv1 += 11 + 4;
					break;
				}

				continue;
			}
			else
			if (c[cv] > 240 && c[cv] < 246)
			{
				switch (c[cv])
				{
					case 241:
						c[cv + 1] = code->function_code[i][cv1 + 1];
						c[cv + 2] = code->function_code[i][cv1 + 2];
						c[cv + 3] = code->function_code[i][cv1 + 3];
						c[cv + 4] = code->function_code[i][cv1 + 4];
						c[cv + 5] = code->function_code[i][cv1 + 5];
						c[cv + 6] = code->function_code[i][cv1 + 6];

						cv += 7;
						cv1 += 7;
						break;

					case 242:
						c[cv + 1] = code->function_code[i][cv1 + 1];
						c[cv + 2] = code->function_code[i][cv1 + 2];
						c[cv + 3] = code->function_code[i][cv1 + 3];
						c[cv + 4] = code->function_code[i][cv1 + 4];
						c[cv + 5] = code->function_code[i][cv1 + 5];
						c[cv + 6] = code->function_code[i][cv1 + 6];
						c[cv + 7] = code->function_code[i][cv1 + 7];
						c[cv + 8] = code->function_code[i][cv1 + 8];
						c[cv + 9] = code->function_code[i][cv1 + 9];
						c[cv + 10] = code->function_code[i][cv1 + 10];

						cv += 11;
						cv1 += 11;
						break;

					case 243:
						c[cv + 1] = code->function_code[i][cv1 + 1];
						c[cv + 2] = code->function_code[i][cv1 + 2];
						c[cv + 3] = code->function_code[i][cv1 + 3];
						c[cv + 4] = code->function_code[i][cv1 + 4];
						c[cv + 5] = code->function_code[i][cv1 + 5];
						c[cv + 6] = code->function_code[i][cv1 + 6];
						c[cv + 7] = code->function_code[i][cv1 + 7];
						c[cv + 8] = code->function_code[i][cv1 + 8];
						c[cv + 9] = code->function_code[i][cv1 + 9];
						c[cv + 10] = code->function_code[i][cv1 + 10];

						cv += 11;
						cv1 += 11;
						break;

					case 244:
						c[cv + 1] = code->function_code[i][cv1 + 1];
						c[cv + 2] = code->function_code[i][cv1 + 2];
						c[cv + 3] = code->function_code[i][cv1 + 3];
						c[cv + 4] = code->function_code[i][cv1 + 4];
						c[cv + 5] = code->function_code[i][cv1 + 5];
						c[cv + 6] = code->function_code[i][cv1 + 6];
						c[cv + 7] = code->function_code[i][cv1 + 7];
						c[cv + 8] = code->function_code[i][cv1 + 8];
						c[cv + 9] = code->function_code[i][cv1 + 9];
						c[cv + 10] = code->function_code[i][cv1 + 10];

						cv += 11;
						cv1 += 11;
						break;

					case 245:
						c[cv + 1] = code->function_code[i][cv1 + 1];
						c[cv + 2] = code->function_code[i][cv1 + 2];
						c[cv + 3] = code->function_code[i][cv1 + 3];
						c[cv + 4] = code->function_code[i][cv1 + 4];
						c[cv + 5] = code->function_code[i][cv1 + 5];
						c[cv + 6] = code->function_code[i][cv1 + 6];

						cv += 7;
						cv1 += 7;
						break;
				}
			}
			else
			if (c[cv] == 255 && code->function_code[i][cv1 + 1] < 56)
			{
				c[cv + 1] = code->function_code[i][cv1 + 1];

				switch (c[cv + 1] % 10)
				{
				case 0: //6 bytes
				case 1:
					c[cv + 2] = code->function_code[i][cv1 + 2];
					c[cv + 3] = code->function_code[i][cv1 + 3];
					c[cv + 4] = code->function_code[i][cv1 + 4];
					c[cv + 5] = code->function_code[i][cv1 + 5];
					c[cv + 6] = code->function_code[i][cv1 + 6];
					cv += 7;
					cv1 += 7;
					break;

				case 3: //7 bytes
				case 4:
				case 7:
					c[cv + 2] = code->function_code[i][cv1 + 2];
					c[cv + 3] = code->function_code[i][cv1 + 3];
					c[cv + 4] = code->function_code[i][cv1 + 4];
					c[cv + 5] = code->function_code[i][cv1 + 5];
					c[cv + 6] = code->function_code[i][cv1 + 6];
					c[cv + 7] = code->function_code[i][cv1 + 7];
					cv += 8;
					cv1 += 8;
					break;

				case 2: //3 bytes
				case 5:
					c[cv + 2] = code->function_code[i][cv1 + 2];
					c[cv + 3] = code->function_code[i][cv1 + 3];
					cv += 4;
					cv1 += 4;
					break;

				case 6: //10 bytes
					c[cv + 2] = code->function_code[i][cv1 + 2];
					c[cv + 3] = code->function_code[i][cv1 + 3];
					c[cv + 4] = code->function_code[i][cv1 + 4];
					c[cv + 5] = code->function_code[i][cv1 + 5];
					c[cv + 6] = code->function_code[i][cv1 + 6];
					c[cv + 7] = code->function_code[i][cv1 + 7];
					c[cv + 8] = code->function_code[i][cv1 + 8];
					c[cv + 9] = code->function_code[i][cv1 + 9];
					c[cv + 10] = code->function_code[i][cv1 + 10];
					cv += 11;
					cv1 += 11;
					break;

				case 8: //11 bytes
				case 9:
					c[cv + 2] = code->function_code[i][cv1 + 2];
					c[cv + 3] = code->function_code[i][cv1 + 3];
					c[cv + 4] = code->function_code[i][cv1 + 4];
					c[cv + 5] = code->function_code[i][cv1 + 5];
					c[cv + 6] = code->function_code[i][cv1 + 6];
					c[cv + 7] = code->function_code[i][cv1 + 7];
					c[cv + 8] = code->function_code[i][cv1 + 8];
					c[cv + 9] = code->function_code[i][cv1 + 9];
					c[cv + 10] = code->function_code[i][cv1 + 10];
					c[cv + 11] = code->function_code[i][cv1 + 11];
					cv += 12;
					cv1 += 12;
					break;
				}

				continue;
			}
			else
			if (c[cv] == 204 && code->function_code[i][cv1 + 1] == 0)
			{
				temp = (code->function_code[i][cv1 + 2] << 24) | (code->function_code[i][cv1 + 3] << 16) | (code->function_code[i][cv1 + 4] << 8)
					| code->function_code[i][cv1 + 5];

				if (temp > code->num_functions)
				{
					errors++;
					LogApp("Linker error: detected call to undefined function in address: %l of function: %s", cv1, code->function_table[i].name);
					cv += 6;
					cv1 += 6;
					continue;
				}

				if (temp > i)
				{
					errors++;
					LogApp("Linker error: detected call to function: %s - not defined yet", cv1, code->function_table[temp].name);
					cv += 6;
					cv1 += 6;
					continue;
				}

				c[cv + 1] = 0;

				Copy32toBuf(c + cv + 2, code->function_table[temp].address);

				cv += 6;
				cv1 += 6;
				continue;
			}
			else
			if (c[cv] == 204 && code->function_code[i][cv1 + 1] == 1)
			{
				c[cv + 1] = code->function_code[i][cv1 + 1];
				c[cv + 2] = code->function_code[i][cv1 + 2];
				c[cv + 3] = code->function_code[i][cv1 + 3];
				c[cv + 4] = code->function_code[i][cv1 + 4];
				c[cv + 5] = code->function_code[i][cv1 + 5];

				cv += 6;
				cv1 += 6;
				continue;
			}
			else
			{
				cv++;
				cv1++;
			}
		}
	}

	if (!(status && 2))
		LogApp("Linker warning: missing function Init entry point");

	if (!(status && 4))
		LogApp("Linker warning: missing function PreGame entry point");

	if (!(status && 8))
		LogApp("Linker warning: missing function GameLoop entry point");

	if (!(status && 16))
		LogApp("Linker warning: missing function MainLoop entry point");

	if (!(status && 32))
		LogApp("Linker warning: missing function End entry point");

	if (errors > 0)
	{
		LogApp("Errors detected during linking stage - stopping...");
		free(c);
		return NULL;
	}

	Copy32toBuf(c + 29, cv - 37); //Save the code size 
	Copy32toBuf(c + 33, cv); ///Save the resource offset (maybe you can save resources with the code, like art, maps and stuff??)

	c = realloc(c, cv + 1);
	CHECKMEM(c);

	if (status < 2 + 4 + 8 + 16 + 32)
		LogApp("Linking sucessfull with some warnings - code may not work correctly");
	else
		LogApp("Linking sucessfull with no errors or warnings");

	return c;
}

void WriteASM(MGMC *code, uint8 function_or_code)
{
	register  int32 i, j, k, l, cv, lcv;

	uint32 temp = 0;

	FILE *f;

	mem_assert(code);

	system("md asm");

	if (function_or_code == 0) //function
	{
		for (i = 0; i < code->num_functions; i++)
		{
			openfile_d(f, StringFormat("asm/%s.asm", code->function_table[i].name), "w");

			switch (code->stacksize)
			{
				case 0:
					fprintf(f, ".TINY\n");
					break;

				case 1:
					fprintf(f, ".LIGHT\n");
					break;

				case 2:
					fprintf(f, ".SMALL\n");
					break;

				case 3:
					fprintf(f, ".MEDIUM\n");
					break;

				case 4:
					fprintf(f, ".GREAT\n");
					break;

				case 5:
					fprintf(f, ".BIG\n");
					break;

				case 6:
					fprintf(f, ".HUGE\n");
					break;

				default:
					fprintf(f, ".FLAT %d\n", code->stacksize);
					break;
			}

			fprintf(f, "\n");

			fprintf(f, ".global\n");

			for (j = 0; j < code->num_vars; j++)
			{
				if (j < 10)
					fprintf(f, "$G000%d ", j);

				if (j >= 10 && j < 100)
					fprintf(f, "$G00%d ", j);

				if (j >= 100 && j < 1000)
					fprintf(f, "$G0%d ", j);

				if (j >= 1000)
					fprintf(f, "$G%d ", j);

				switch (code->vars_table[j].type)
				{
					case 1:
						fprintf(f, "DW %d     ;%s\n", code->vars_table[j].value, code->vars_table[j].name);
						break;

					case 2:
						fprintf(f, "FP %f     ;%s\n", code->vars_table[j].valuef, code->vars_table[j].name);
						break;

					case 3:
						fprintf(f, "PTR     ;%s\n", code->vars_table[j].name);
						break;

					case 4:
						fprintf(f, "B %d \"%s\"     ;%s\n", code->vars_table[j].len, code->vars_table[j].string, code->vars_table[j].name);
						break;
				}
			}

			fprintf(f, "\n");
			fprintf(f, ".text\n\n");

			//for (k = 0; k < code->num_functions; k++)
			//{
				fprintf(f, "PROC _%s\n\n", code->function_table[i].name);

				k = i;

				for (j = 0; j < code->function_table[k].num_vars; j++)
				{
					fprintf(f, "data ");

					if (j < 10)
						fprintf(f, "$L000%d ", j);

					if (j >= 10 && j < 100)
						fprintf(f, "$L00%d ", j);

					if (j >= 100 && j < 1000)
						fprintf(f, "$L0%d ", j);

					if (j >= 1000)
						fprintf(f, "$L%d ", j);

					switch (code->function_table[k].vars_table[j].type)
					{
					case 1:
						fprintf(f, "DW %d     ;%s\n", code->function_table[k].vars_table[j].value, code->function_table[k].vars_table[j].name);
						break;

					case 2:
						fprintf(f, "FP %f     ;%s\n", code->function_table[k].vars_table[j].valuef, code->function_table[k].vars_table[j].name);
						break;

					case 3:
						fprintf(f, "PTR     ;%s\n", code->function_table[k].vars_table[j].name);
						break;

					case 4:
						fprintf(f, "B %d \"%s\"     ;%s\n", code->function_table[k].vars_table[j].len, code->function_table[k].vars_table[j].string,
							code->function_table[k].vars_table[j].name);
						break;
					}
				}

				unsigned char *buf = code->function_code[k];

				for (cv = 0; cv < code->function_table[k].size; )
				{
					lcv = cv;
					//alloc_mem(buf, code->function_table[k].size);

					if (buf[cv] < 56)
					{
						switch (buf[cv] / 10)
						{
							case 0:
								fprintf(f, "mov");
								break;

							case 1:
								fprintf(f, "add");
								break;

							case 2:
								fprintf(f, "sub");
								break;

							case 3:
								fprintf(f, "mul");
								break;

							case 4:
								fprintf(f, "div");
								break;

							case 5:
								fprintf(f, "pow");
								break;
						}

						switch (buf[cv] % 10)
						{
							case 0:
								fprintf(f, "rc ");

								fprintf(f, "r%d ", buf[cv + 1]);

								fprintf(f, "%d\n", GetValueBuf(buf + cv + 2));

								cv += 6;

								break;

							case 1:
								fprintf(f, "rm ");

								fprintf(f, "r%d ", buf[cv + 1]);

								if (buf[cv + 2] == 0)
									fprintf(f, "$G");
								else
									fprintf(f, "$L");

								temp = GetValueBuf(buf + cv + 3);

								if (temp < 10)
									fprintf(f, "000%d", temp);

								if (temp >= 10 && temp < 100)
									fprintf(f, "00%d", temp);

								if (temp >= 100 && temp < 1000)
									fprintf(f, "0%d", temp);

								if (temp >= 1000)
									fprintf(f, "%d", temp);

								fprintf(f, "\n");

								cv += 7;

								break;

							case 2:
								fprintf(f, "rr ");

								fprintf(f, "r%d ", buf[cv + 1]);

								fprintf(f, "r%d ", buf[cv + 2]);

								cv += 3;

								break;

							case 3:
								fprintf(f, "mc ");

								if (buf[cv + 1] == 0)
									fprintf(f, "$G");
								else
									fprintf(f, "$L");

								temp = GetValueBuf(buf + cv + 2);

								if (temp < 10)
									fprintf(f, "000%d", temp);

								if (temp >= 10 && temp < 100)
									fprintf(f, "00%d", temp);

								if (temp >= 100 && temp < 1000)
									fprintf(f, "0%d", temp);

								if (temp >= 1000)
									fprintf(f, "%d", temp);

								fprintf(f, " %d\n", GetValueBuf(buf + cv + 6));

								cv += 10;

								break;

							case 4:
								fprintf(f, "mr ");

								if (buf[cv + 1] == 0)
									fprintf(f, "$G");
								else
									fprintf(f, "$L");

								temp = GetValueBuf(buf + cv + 2);

								if (temp < 10)
									fprintf(f, "000%d", temp);

								if (temp >= 10 && temp < 100)
									fprintf(f, "00%d", temp);

								if (temp >= 100 && temp < 1000)
									fprintf(f, "0%d", temp);

								if (temp >= 1000)
									fprintf(f, "%d", temp);

								fprintf(f, " r%d\n", buf[cv + 6]);

								cv += 7;

								break;

							case 5:
								fprintf(f, "mm ");

								if (buf[cv + 1] == 0)
									fprintf(f, "$G");
								else
									fprintf(f, "$L");

								temp = GetValueBuf(buf + cv + 2);

								if (temp < 10)
									fprintf(f, "000%d", temp);

								if (temp >= 10 && temp < 100)
									fprintf(f, "00%d", temp);

								if (temp >= 100 && temp < 1000)
									fprintf(f, "0%d", temp);

								if (temp >= 1000)
									fprintf(f, "%d", temp);

								if (buf[cv + 6] == 0)
									fprintf(f, "$G");
								else
									fprintf(f, "$L");

								temp = GetValueBuf(buf + cv + 7);

								if (temp < 10)
									fprintf(f, "000%d", temp);

								if (temp >= 10 && temp < 100)
									fprintf(f, "00%d", temp);

								if (temp >= 100 && temp < 1000)
									fprintf(f, "0%d", temp);

								if (temp >= 1000)
									fprintf(f, "%d", temp);

								fprintf(f, "\n");

								cv += 11;

								break;
						}
					}

					if (buf[cv] == 255 && buf[cv + 1] < 56)
					{
						switch (buf[cv + 1] / 10)
						{
						case 0:
							fprintf(f, "mov");
							break;

						case 1:
							fprintf(f, "add");
							break;

						case 2:
							fprintf(f, "sub");
							break;

						case 3:
							fprintf(f, "mul");
							break;

						case 4:
							fprintf(f, "div");
							break;

						case 5:
							fprintf(f, "pow");
							break;
						}

						switch (buf[cv + 1] % 10)
						{
						case 0:
							fprintf(f, "rfc ");

							fprintf(f, "rf%d ", buf[cv + 1]);

							fprintf(f, "%d\n", GetValueBuf(buf + cv + 2));

							cv += 6;

							break;

						case 1:
							fprintf(f, "rfcf ");

							fprintf(f, "rf%d ", buf[cv + 1]);

							fprintf(f, "%f\n", GetValueBuf(buf + cv + 2));

							cv += 6;

							break;

						case 2:
							fprintf(f, "rfr ");

							fprintf(f, "rf%d ", buf[cv + 1]);

							fprintf(f, "r%d ", buf[cv + 2]);

							cv += 3;

							break;

						case 3:
							fprintf(f, "rfm ");

							fprintf(f, "rf%d ", buf[cv + 1]);

							if (buf[cv + 2] == 0)
								fprintf(f, "$G");
							else
								fprintf(f, "$L");

							temp = GetValueBuf(buf + cv + 3);

							if (temp < 10)
								fprintf(f, "000%d", temp);

							if (temp >= 10 && temp < 100)
								fprintf(f, "00%d", temp);

							if (temp >= 100 && temp < 1000)
								fprintf(f, "0%d", temp);

							if (temp >= 1000)
								fprintf(f, "%d", temp);

							fprintf(f, "\n");

							cv += 7;

							break;

						case 4:
							fprintf(f, "rfmf ");

							fprintf(f, "rf%d ", buf[cv + 1]);

							if (buf[cv + 2] == 0)
								fprintf(f, "$G");
							else
								fprintf(f, "$L");

							temp = GetValueBuf(buf + cv + 3);

							if (temp < 10)
								fprintf(f, "000%d", temp);

							if (temp >= 10 && temp < 100)
								fprintf(f, "00%d", temp);

							if (temp >= 100 && temp < 1000)
								fprintf(f, "0%d", temp);

							if (temp >= 1000)
								fprintf(f, "%d", temp);

							fprintf(f, "\n");

							cv += 7;

							break;

						case 5:
							fprintf(f, "rfrf ");

							fprintf(f, "rf%d ", buf[cv + 1]);

							fprintf(f, "rf%d ", buf[cv + 2]);

							cv += 3;

							break;

						case 6:
							fprintf(f, "mfcf ");

							if (buf[cv + 1] == 0)
								fprintf(f, "$G");
							else
								fprintf(f, "$L");

							temp = GetValueBuf(buf + cv + 2);

							if (temp < 10)
								fprintf(f, "000%d", temp);

							if (temp >= 10 && temp < 100)
								fprintf(f, "00%d", temp);

							if (temp >= 100 && temp < 1000)
								fprintf(f, "0%d", temp);

							if (temp >= 1000)
								fprintf(f, "%d", temp);

							fprintf(f, " %f\n", GetValueBuf(buf + cv + 6));

							cv += 10;

							break;

						case 7:
							fprintf(f, "mfrf ");

							if (buf[cv + 1] == 0)
								fprintf(f, "$G");
							else
								fprintf(f, "$L");

							temp = GetValueBuf(buf + cv + 2);

							if (temp < 10)
								fprintf(f, "000%d", temp);

							if (temp >= 10 && temp < 100)
								fprintf(f, "00%d", temp);

							if (temp >= 100 && temp < 1000)
								fprintf(f, "0%d", temp);

							if (temp >= 1000)
								fprintf(f, "%d", temp);

							fprintf(f, " rf%d\n", buf[cv + 6]);

							cv += 7;

							break;

						case 8:
							fprintf(f, "mfm ");

							if (buf[cv + 1] == 0)
								fprintf(f, "$G");
							else
								fprintf(f, "$L");

							temp = GetValueBuf(buf + cv + 2);

							if (temp < 10)
								fprintf(f, "000%d", temp);

							if (temp >= 10 && temp < 100)
								fprintf(f, "00%d", temp);

							if (temp >= 100 && temp < 1000)
								fprintf(f, "0%d", temp);

							if (temp >= 1000)
								fprintf(f, "%d", temp);

							if (buf[cv + 6] == 0)
								fprintf(f, "$G");
							else
								fprintf(f, "$L");

							temp = GetValueBuf(buf + cv + 7);

							if (temp < 10)
								fprintf(f, "000%d", temp);

							if (temp >= 10 && temp < 100)
								fprintf(f, "00%d", temp);

							if (temp >= 100 && temp < 1000)
								fprintf(f, "0%d", temp);

							if (temp >= 1000)
								fprintf(f, "%d", temp);

							fprintf(f, "\n");

							cv += 11;

							break;

						case 9:
							fprintf(f, "mfmf ");

							if (buf[cv + 1] == 0)
								fprintf(f, "$G");
							else
								fprintf(f, "$L");

							temp = GetValueBuf(buf + cv + 2);

							if (temp < 10)
								fprintf(f, "000%d", temp);

							if (temp >= 10 && temp < 100)
								fprintf(f, "00%d", temp);

							if (temp >= 100 && temp < 1000)
								fprintf(f, "0%d", temp);

							if (temp >= 1000)
								fprintf(f, "%d", temp);

							if (buf[cv + 6] == 0)
								fprintf(f, "$G");
							else
								fprintf(f, "$L");

							temp = GetValueBuf(buf + cv + 7);

							if (temp < 10)
								fprintf(f, "000%d", temp);

							if (temp >= 10 && temp < 100)
								fprintf(f, "00%d", temp);

							if (temp >= 100 && temp < 1000)
								fprintf(f, "0%d", temp);

							if (temp >= 1000)
								fprintf(f, "%d", temp);

							fprintf(f, "\n");

							cv += 11;

							break;
						}

						cv++;
					}

					if (buf[cv] > 141 && buf[cv] < 202)
					{
						switch (buf[cv] / 10)
						{
						case 14:
							fprintf(f, "ifge");
							break;

						case 15:
							fprintf(f, "ifle");
							break;

						case 16:
							fprintf(f, "ifg");
							break;

						case 17:
							fprintf(f, "ifl");
							break;

						case 18:
							fprintf(f, "ife");
							break;

						case 19:
							fprintf(f, "ifne");
							break;
						}

						switch ((buf[cv] % 10) - 2)
						{
						case 0:
							fprintf(f, "rc ");

							fprintf(f, "r%d ", buf[cv + 1]);

							fprintf(f, "%d\n", GetValueBuf(buf + cv + 2));

							cv += 6;

							break;

						case 1:
							fprintf(f, "rm ");

							fprintf(f, "r%d ", buf[cv + 1]);

							if (buf[cv + 2] == 0)
								fprintf(f, "$G");
							else
								fprintf(f, "$L");

							temp = GetValueBuf(buf + cv + 3);

							if (temp < 10)
								fprintf(f, "000%d", temp);

							if (temp >= 10 && temp < 100)
								fprintf(f, "00%d", temp);

							if (temp >= 100 && temp < 1000)
								fprintf(f, "0%d", temp);

							if (temp >= 1000)
								fprintf(f, "%d", temp);

							fprintf(f, "\n");

							cv += 7;

							break;

						case 2:
							fprintf(f, "rr ");

							fprintf(f, "r%d ", buf[cv + 1]);

							fprintf(f, "r%d ", buf[cv + 2]);

							cv += 3;

							break;

						case 3:
							fprintf(f, "mc ");

							if (buf[cv + 1] == 0)
								fprintf(f, "$G");
							else
								fprintf(f, "$L");

							temp = GetValueBuf(buf + cv + 2);

							if (temp < 10)
								fprintf(f, "000%d", temp);

							if (temp >= 10 && temp < 100)
								fprintf(f, "00%d", temp);

							if (temp >= 100 && temp < 1000)
								fprintf(f, "0%d", temp);

							if (temp >= 1000)
								fprintf(f, "%d", temp);

							fprintf(f, " %d\n", GetValueBuf(buf + cv + 6));

							cv += 10;

							break;

						case 4:
							fprintf(f, "mr ");

							if (buf[cv + 1] == 0)
								fprintf(f, "$G");
							else
								fprintf(f, "$L");

							temp = GetValueBuf(buf + cv + 2);

							if (temp < 10)
								fprintf(f, "000%d", temp);

							if (temp >= 10 && temp < 100)
								fprintf(f, "00%d", temp);

							if (temp >= 100 && temp < 1000)
								fprintf(f, "0%d", temp);

							if (temp >= 1000)
								fprintf(f, "%d", temp);

							fprintf(f, " r%d\n", buf[cv + 6]);

							cv += 7;

							break;

						case 5:
							fprintf(f, "mm ");

							if (buf[cv + 1] == 0)
								fprintf(f, "$G");
							else
								fprintf(f, "$L");

							temp = GetValueBuf(buf + cv + 2);

							if (temp < 10)
								fprintf(f, "000%d", temp);

							if (temp >= 10 && temp < 100)
								fprintf(f, "00%d", temp);

							if (temp >= 100 && temp < 1000)
								fprintf(f, "0%d", temp);

							if (temp >= 1000)
								fprintf(f, "%d", temp);

							if (buf[cv + 6] == 0)
								fprintf(f, "$G");
							else
								fprintf(f, "$L");

							temp = GetValueBuf(buf + cv + 7);

							if (temp < 10)
								fprintf(f, "000%d", temp);

							if (temp >= 10 && temp < 100)
								fprintf(f, "00%d", temp);

							if (temp >= 100 && temp < 1000)
								fprintf(f, "0%d", temp);

							if (temp >= 1000)
								fprintf(f, "%d", temp);

							fprintf(f, "\n");

							cv += 11;

							break;
						}

						cv += 4;
					}

					if (buf[cv] == 204)
					{
						fprintf(f, "call ");

						if (buf[cv + 1] == 0)
						{
							temp = GetValueBuf(buf + cv + 2);

							for (l = 0; l < code->num_functions; l++)
							{
								if (temp == code->function_table[l].address)
								{
									fprintf(f, "_%s\n\n", code->function_table[l].name);
									break;
								}
							}
						}
						else
						{
							temp = GetValueBuf(buf + cv + 2);

							fprintf(f, "%s\n\n", e_funcs[temp].name);
						}

						cv += 6;
					}

					if (buf[cv] == 205)
					{
						fprintf(f, "pushc %d\n", GetValueBuf(buf + cv + 1));
						cv += 5;
					}

					if (buf[cv] == 206)
					{
						fprintf(f, "pushm ");

						if (buf[cv + 1] == 0)
							fprintf(f, "$G");
						else
							fprintf(f, "$L");

						temp = GetValueBuf(buf + cv + 2);

						if (temp < 10)
							fprintf(f, "000%d", temp);

						if (temp >= 10 && temp < 100)
							fprintf(f, "00%d", temp);

						if (temp >= 100 && temp < 1000)
							fprintf(f, "0%d", temp);

						if (temp >= 1000)
							fprintf(f, "%d", temp);

						fprintf(f, "\n");

						cv += 6;
					}

					if (buf[cv] == 207)
					{
						fprintf(f, "pushr r%d\n", buf[cv + 1]);
						cv += 2;
					}

					if (buf[cv] == 208)
					{
						fprintf(f, "pushfr f%d\n", buf[cv + 1]);
						cv += 2;
					}

					if (buf[cv] == 210)
					{
						fprintf(f, "ret\n");

						if (buf[lcv] == 210)
							break;

						cv++;
					}

					if (buf[cv] == 250)
					{
						fprintf(f, "fti r%d f%d", buf[cv + 1], buf[cv + 2]);
						cv += 3;
					}

					if (buf[cv] == 251)
					{
						fprintf(f, "itf f%d r%d", buf[cv + 1], buf[cv + 2]);
						cv += 3;
					}

					if (buf[cv] == 252)
					{
						fprintf(f, "popr r%d\n", buf[cv + 1]);
						cv += 2;
					}

					if (buf[cv] == 253)
					{
						fprintf(f, "popfr f%d\n", buf[cv + 1]);
						cv += 2;
					}
				}
			//}

			fclose(f);
		}
	}
}

int8 BuildMGL(const char *filename, const char *finalname)
{
	FILE *f;

	MGMC *code;
	unsigned char *c;

	openfile_d(f, filename, "r");

	code = CompileMGL(f, 0);

 	if (code == NULL)
	{
		LogApp("Compiling stage failed");
		fclose(f);
		return NULL;
	}

	fclose(f);

	//WriteASM(code, 0);

	c = LinkMGL(code, 0);

	if (c == NULL)
	{
		LogApp("Linking stage failed");

		for (int i = 0; i < code->num_functions; i++)
		{
			free(code->bt_trl[i]);
			free(code->function_code[i]);

			if (code->function_table[i].num_vars > 0)
				free(code->function_table[i].vars_table);
		}

		free(code->bt_trl);
		free(code->function_code);
		free(code->function_table);
		free(code->vars_table);

		return NULL;
	}

	openfile_d(f, finalname, "wb");

	size_t size = GetValueBuf(c + 33);

	fwrite(c, 1, size, f);

	fclose(f);

	free(c);

	for (int i = 0; i < code->num_functions; i++)
	{
		free(code->bt_trl[i]);
		free(code->function_code[i]);

		if (code->function_table[i].num_vars > 0)
			free(code->function_table[i].vars_table);
	}

	free(code->bt_trl);
	free(code->function_code);
	free(code->function_table);
	free(code->vars_table);

	LogApp("Build complete");

	return 1;
}

int8 InitMGLCode(const char *file)
{
	//register uint32 val, tmp, v1, v2, v3, v4, i;
	//register uint16 cv;

	///char stack[65536];
	//uint8 *vars;
	//void **heap;

	FILE *f;
	//char *buffer;

	openfile_d(f, file, "rb");

	fseek(f, 0, SEEK_END);

	st.mgl.size = ftell(f);
	rewind(f);

	st.mgl.code = malloc(st.mgl.size);
	CHECKMEM(st.mgl.code);

	fread(st.mgl.code, 1, st.mgl.size, f);

	fclose(f);


	if (st.mgl.code[0] != 'M' || st.mgl.code[1] != 'G' || st.mgl.code[2] != 'L' && st.mgl.code[3] != ' ')
	{
		free(st.mgl.code);
		return NULL;
	}

	switch(st.mgl.code[24])
	{
		case 0:
			st.mgl.memsize = 512;
			st.mgl.stack_type = 0;
			break;

		case 1:
			st.mgl.memsize = 1024;
			st.mgl.stack_type = 0;
			break;

		case 2:
			st.mgl.memsize = 4096;
			st.mgl.stack_type = 0;
			break;

		case 3:
			st.mgl.memsize = 12288;
			st.mgl.stack_type = 0;
			break;

		case 4:
			st.mgl.memsize = 16384;
			st.mgl.stack_type = 0;
			break;

		case 5:
			st.mgl.memsize = 32768;
			st.mgl.stack_type = 0;
			break;

		case 6:
			st.mgl.memsize = 131072;
			st.mgl.stack_type = 0;
			break;

		default:
			st.mgl.memsize = (st.mgl.code[25] << 24) | (st.mgl.code[26] << 16) | (st.mgl.code[27] << 8) | st.mgl.code[28];
			st.mgl.stack_type = 1;
			break;
	}

	ZeroMem(st.mgl.v, sizeof(8 * sizeof(uint32)));
	st.mgl.bp = st.mgl.sp = 0;

	st.mgl.stack = NULL;

	///Load engine calls

	st.mgl.funcs.log = LogApp;
	st.mgl.funcs.drawline = LineData;
	st.mgl.funcs.msgbox = MessageBoxRes;
	st.mgl.funcs.playmovie = PlayMovie;
	st.mgl.funcs.playbgvideo = PlayBGVideo;
	st.mgl.funcs.drawui = UIezData;
	st.mgl.flags.flags = 0;

	return 1;
}

int8 CallEngFunction(int32 address, int32 *v, float *f, int32 *stack, int32 bp, struct MGLHeap *heap)
{
	//mem_assert(stack);
	//mem_assert(heap);
	bp -= 2;

	uint8 args[24];

	ZeroMem(args, 24);

	switch (address)
	{
		case E_LOG:
			v[7] = v[8] = 0;
			while (v[7] == 0)
			{
				if (v[8] == 0)
					args[v[8]] = DetectArgument(heap[v[9]].string);
				else
					args[v[8]] = DetectArgument(NULL);

				if (args[v[8]] == 0)
					v[7] = 1;

				v[8]++;
			}

			st.mgl.funcs.log(heap[v[9]].string, args[0] == 1 ? heap[v[10]].string : v[10], args[1] == 1 ? heap[v[11]].string : v[11], args[2] == 1 ? heap[v[12]].string : v[12],
				args[3] == 1 ? heap[v[13]].string : v[13], args[4] == 1 ? heap[v[14]].string : v[14], args[5] == 1 ? heap[v[15]].string : v[15],
				args[6] == 1 ? heap[v[16]].string : v[16], args[7] == 1 ? heap[v[17]].string : v[17], args[8] == 1 ? heap[v[18]].string : v[18]);
			break;

		case E_DRAWLINE:
			st.mgl.funcs.drawline(v[9], v[10], v[11], v[12], v[13], v[14], v[15], v[16], v[17], v[18]);
			break;

		case E_PLAYMOV:
			v[2] = st.mgl.funcs.playmovie(heap[v[9]].string);
			break;

		case E_PLAYBGMOV:
			v[2] = st.mgl.funcs.playbgvideo(heap[v[9]].string, v[10]);
			break;

		case C_GSYSSCREEN:
			v[9] = st.screenx;
			v[10] = st.screeny;
			v[11] = st.fullscreen;
			v[12] = st.bpp;
			break;

		case C_GSYSMOUSEPOS:
			v[9] = st.mouse.x;
			v[10] = st.mouse.y;
			break;

		case C_GSYSMOUSELMB:
			v[9] = v[2] = st.mouse1;
			break;

		case C_GSYSMOUSERMB:
			v[9] = v[2] = st.mouse2;
			break;

		case C_GSYSKEY:
			v[10] = v[2] = st.keys[v[9]].state;
			break;

		case C_GSPRFRAME:
			v[2] = st.Game_Sprites[v[9]].frame[v[10]];
			break;

		case E_STW:
			STW(&v[9], &v[10]);
			break;

		case E_WTS:
			WTS(&v[9], &v[10]);
			break;

		case E_STWCI:
			STWci(&v[9], &v[10]);
			break;

		case E_WTSCI:
			WTSci(&v[9], &v[10]);
			break;

		case E_STWF:
			STWf(&v[9], &v[10]);
			break;

		case E_WTSF:
			WTSf(&v[9], &v[10]);
			break;

		case C_GSYSNUMMGGS:
			v[9] = st.num_mgg;
			break;

		case C_GSYSMOUSEWHEEL:
			v[9] = st.mouse_wheel;
			break;

		case C_GSYSGAMESTATE:
			v[9] = st.gt;
			break;

		case E_DRAWUI:
			st.mgl.funcs.drawui(v[9], v[10], f[7], v[12], v[13], v[14], v[15], *(TEX_DATA*) v[16], v[17], v[18]);
			break;

		case C_GMGGTEX:
			if (v[9] == 0)
				(uint32*) v[2] = mgg_sys[v[10]].frames + v[11];
			else
			if (v[9] == 1)
				v[2] = &mgg_game[v[10]].frames[v[11]];
			else
			if (v[9] == 2)
				v[2] = &mgg_map[v[10]].frames[v[11]];
			break;

		case E_PRINT:
			v[7] = v[8] = 0;
			while(v[7] == 0)
			{
				if (v[8] == 0)
					args[v[8]] = DetectArgument(heap[v[9]].string);
				else
					args[v[8]] = DetectArgument(NULL);

				if (args[v[8]] == 0)
					v[7] = 1;

				v[8]++;
			}

			printf(heap[v[9]].string, args[0] == 1 ? heap[v[10]].string : v[10], args[1] == 1 ? heap[v[11]].string : v[11], args[2] == 1 ? heap[v[12]].string : v[12],
				args[3] == 1 ? heap[v[13]].string : v[13], args[4] == 1 ? heap[v[14]].string : v[14], args[5] == 1 ? heap[v[15]].string : v[15],
				args[6] == 1 ? heap[v[16]].string : v[16], args[7] == 1 ? heap[v[17]].string : v[17], args[8] == 1 ? heap[v[18]].string : v[18]);
			break;
	}
}

int8 ExecuteMGLCode(uint8 location)
{
	register int32 bp, sp, cv, lcv;
	int32 v[32];
	
	union MGLFlags fl;

	int32 *stack = NULL, num_heap = 0;
	uint8 *vars, state = 0;

	struct MGLHeap *heap;

	float f[24];

	unsigned char *buf = st.mgl.code;

	int error, funcaddr;

	cv = st.mgl.cv;
	memcpy(v, st.mgl.v, 8 * sizeof(uint32));

	//location - 0: Init, 1: PreGame, 2: GameLoop (game clock), 3: MainLoop(engine clock), 4: End

	switch (location)
	{
	case 0:
		st.mgl.cv = GetValueBuf(st.mgl.code + 4);//Get the entry point Init
		break;

	case 1:
		st.mgl.cv = GetValueBuf(st.mgl.code + 8); //Get the entry point PreGame
		break;

	case 2:
		st.mgl.cv = GetValueBuf(st.mgl.code + 12); //Get the entry point GameLoop
		break;

	case 3:
		st.mgl.cv = GetValueBuf(st.mgl.code + 16); //Get the entry point MainLoop
		break;

	case 4:
		st.mgl.cv = GetValueBuf(st.mgl.code + 20); //Get the entry point End
		break;
	}

	cv = st.mgl.cv;
	funcaddr = cv;
	bp = st.mgl.bp;
	sp = st.mgl.sp;
	fl = st.mgl.flags;

	if (st.mgl.stack != NULL)
		stack = st.mgl.stack;

	if (st.mgl.num_heap > 0)
		heap = st.mgl.heap;

	num_heap = st.mgl.num_heap;

	if (location == 0)
	{
		if (stack == NULL)
		{
			stack = malloc(st.mgl.memsize * sizeof(uint32));
			CHECKMEM(stack);
		}

		cv = 37;
		//fetch global variables to the stack
		while (buf[cv] == 254 && cv < GetValueBuf(st.mgl.code + 4))
		{
			if (buf[cv + 1] < 5)
			{
				if (buf[cv + 1] == 3 || buf[cv + 1] == 4)
				{
					if (num_heap == 0)
					{
						heap = calloc(1, sizeof(struct MGLHeap));
						CHECKMEM(heap);
					}
					else
					{
						heap = realloc(heap, (num_heap + 1) * sizeof(struct MGLHeap));
						CHECKMEM(heap);
					}

					stack[bp] = num_heap;

					if (buf[cv + 1] == 4)
					{
						GetValueCV(v[7], cv + 2);
						heap[num_heap].size = v[7];
						heap[num_heap].type = 1; //String
						heap[num_heap].string = malloc(v[7]);
						CHECKMEM(heap[num_heap].string);
						memcpy(heap[num_heap].string, buf + cv + 6, v[7]);

						cv += 6 + v[7];
					}
					else
					{
						heap[num_heap].size = 1;
						heap[num_heap].type = 0;
						heap[num_heap].mem = malloc(1);
						CHECKMEM(heap[num_heap].mem);
						cv += 2;
					}

					num_heap++;
				}
				else
				{
					CodeToStack(bp, cv + 2);
					cv += 6;
				}

				bp++;
			}
			else
			{
				MessageBoxRes("MGVM error", MB_OK, "VM error: detected invalid operator in the stack definition bytecode:\naddress: %d, byte: %d, stack address: %d",
					cv, buf[cv + 1], bp);
				st.quit = 1;
				return NULL;
			}
		}

		stack[bp] = bp;
		st.mgl.ret_addr = bp;
		sp = bp + 1;
		PushStack(0);
		bp += 2;
		sp = bp;
		cv = st.mgl.cv;
	}

	while (bp > st.mgl.ret_addr)
	{
		if (st.quit == 1)
			break;

		//If is data definition
		if (buf[cv] == 254)
		{
			switch (buf[cv + 1])
			{
			case 0:
			case 1:
			case 2:
				CodeToStack(sp, cv + 2);
				sp++;
				cv += 6;
				break;

			case 3:
				if (num_heap == 0)
				{
					heap = calloc(1, sizeof(struct MGLHeap));
					CHECKMEM(heap);
				}
				else
				{
					heap = realloc(heap, (num_heap + 1) * sizeof(struct MGLHeap));
					CHECKMEM(heap);
				}

				stack[sp] = num_heap;

				heap[num_heap].size = 1;
				heap[num_heap].type = 0;
				heap[num_heap].mem = malloc(1);
				CHECKMEM(heap[num_heap].mem);

				num_heap++;

				sp++;
				cv += 2;
				break;

			case 4:
				if (num_heap == 0)
				{
					heap = calloc(1, sizeof(struct MGLHeap));
					CHECKMEM(heap);
				}
				else
				{
					heap = realloc(heap, (num_heap + 1) * sizeof(struct MGLHeap));
					CHECKMEM(heap);
				}

				stack[sp] = num_heap;

				heap[num_heap].stack_pos = sp;

				GetValueCV(v[7], cv + 2);
				heap[num_heap].size = v[7];
				heap[num_heap].type = 1; //String
				heap[num_heap].string = malloc(v[7] + 1);
				CHECKMEM(heap[num_heap].string);
				memcpy(heap[num_heap].string, buf + cv + 6, v[7]);
				heap[num_heap].string[v[7]] = '\0';
				heap[num_heap].stack_pos = sp;
				cv += 6 + v[7];
				num_heap++;

				sp++;
				break;

				default:
					MessageBoxRes("MGVM error", MB_OK, "VM error: Invalid operator in data definition bytecode:\naddress: %d, byte: %d, stack address: %d",
						cv, buf[cv + 1], bp);
					st.quit = 1;

			}
		}
		else
		{
			if (state == 0)
			{
				switch (buf[cv])
				{
					//set
				case opSET:
					GetValueCV(v[7], cv + 2);
					SetRegSwitch(buf[cv + 1]);
					cv += 6;
					break;

				case 1:
					GetVarAddress(cv + 2);
					GetValueStack(v[7], v[2]);
					SetRegSwitch(buf[cv + 1]);
					cv += 7;
					break;

				case 2:
					GetRegSwitch(buf[cv + 2]);
					SetRegSwitch(buf[cv + 1]);
					cv += 3;
					break;

				case 3:
					GetVarAddress(cv + 1);
					CodeToStack(v[2], cv + 6);
					cv += 10;
					break;

				case 4:
					GetRegSwitch(buf[cv + 6]);
					GetVarAddress(cv + 1);
					SetStack(v[7], v[2]);
					cv += 7;
					break;

				case 5:
					GetVarAddress(cv + 1);
					v[7] = v[2];
					GetVarAddress(cv + 6);
					StackToStack(v[7], v[2]);
					cv += 11;
					break;

					//add
				case opADD:
					GetValueCV(v[7], cv + 2);
					v[2] = v[7];

					GetRegSwitch(buf[cv + 1]);
					v[7] += v[2];

					SetRegSwitch(buf[cv + 1]);
					cv += 6;
					break;

				case 11:
					GetVarAddress(cv + 2);
					GetValueStack(v[7], v[2]);
					v[2] = v[7];

					GetRegSwitch(buf[cv + 1]);
					v[7] += v[2];

					SetRegSwitch(buf[cv + 1]);
					cv += 7;
					break;

				case 12:
					GetRegSwitch(buf[cv + 2]);
					v[2] = v[7];

					GetRegSwitch(buf[cv + 1]);
					v[7] += v[2];

					SetRegSwitch(buf[cv + 1]);
					cv += 3;
					break;

				case 13:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetValueCV(v[1], cv + 6);
					v[7] += v[1];
					SetStack(v[7], v[2]);
					cv += 10;
					break;

				case 14:
					GetVarAddress(cv + 1);
					GetRegSwitch(buf[cv + 6]);
					GetValueStack(v[1], v[2]);
					v[7] += v[1];
					SetStack(v[7], v[2]);
					cv += 7;
					break;

				case 15:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					PushStack(v[2]);
					GetVarAddress(cv + 6);
					GetValueStack(v[1], v[2]);
					v[7] += v[1];
					PopStack(v[2]);
					SetStack(v[7], v[2]);
					cv += 11;
					break;

					//sub
				case opSUB:
					GetValueCV(v[7], cv + 2);
					v[2] = v[7];

					GetRegSwitch(buf[cv + 1]);
					v[7] -= v[2];

					SetRegSwitch(buf[cv + 1]);
					cv += 6;
					break;

				case 21:
					GetVarAddress(cv + 2);
					GetValueStack(v[7], v[2]);
					v[2] = v[7];

					GetRegSwitch(buf[cv + 1]);
					v[7] -= v[2];

					SetRegSwitch(buf[cv + 1]);
					cv += 7;
					break;

				case 22:
					GetRegSwitch(buf[cv + 2]);
					v[2] = v[7];

					GetRegSwitch(buf[cv + 1]);
					v[7] -= v[2];

					SetRegSwitch(buf[cv + 1]);
					cv += 3;
					break;

				case 23:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetValueCV(v[1], cv + 6);
					v[7] -= v[1];
					SetStack(v[7], v[2]);
					cv += 10;
					break;

				case 24:
					GetVarAddress(cv + 1);
					GetRegSwitch(buf[cv + 6]);
					GetValueStack(v[1], v[2]);
					v[7] -= v[1];
					SetStack(v[7], v[2]);
					cv += 7;
					break;

				case 25:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					PushStack(v[2]);
					GetVarAddress(cv + 6);
					GetValueStack(v[1], v[2]);
					v[7] -= v[1];
					PopStack(v[2]);
					SetStack(v[7], v[2]);
					cv += 11;
					break;

					//mul
				case opMUL:
					GetValueCV(v[7], cv + 2);
					v[2] = v[7];

					GetRegSwitch(buf[cv + 1]);
					v[7] *= v[2];

					SetRegSwitch(buf[cv + 1]);
					cv += 6;
					break;

				case 31:
					GetVarAddress(cv + 2);
					GetValueStack(v[7], v[2]);
					v[2] = v[7];

					GetRegSwitch(buf[cv + 1]);
					v[7] *= v[2];

					SetRegSwitch(buf[cv + 1]);
					cv += 7;
					break;

				case 32:
					GetRegSwitch(buf[cv + 2]);
					v[2] = v[7];

					GetRegSwitch(buf[cv + 1]);
					v[7] *= v[2];

					SetRegSwitch(buf[cv + 1]);
					cv += 3;
					break;

				case 33:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetValueCV(v[1], cv + 6);
					v[7] *= v[1];
					SetStack(v[7], v[2]);
					cv += 10;
					break;

				case 34:
					GetVarAddress(cv + 1);
					GetRegSwitch(buf[cv + 6]);
					GetValueStack(v[1], v[2]);
					v[7] *= v[1];
					SetStack(v[7], v[2]);
					cv += 7;
					break;

				case 35:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					PushStack(v[2]);
					GetVarAddress(cv + 6);
					GetValueStack(v[1], v[2]);
					v[7] *= v[1];
					PopStack(v[2]);
					SetStack(v[7], v[2]);
					cv += 11;
					break;

					//div
				case opDIV:
					GetValueCV(v[7], cv + 2);
					v[2] = v[7];

					GetRegSwitch(buf[cv + 1]);
					v[7] = v[2] / v[7];

					SetRegSwitch(buf[cv + 1]);
					cv += 6;
					break;

				case 41:
					GetVarAddress(cv + 2);
					GetValueStack(v[7], v[2]);
					v[2] = v[7];

					GetRegSwitch(buf[cv + 1]);
					v[7] = v[2] / v[7];

					SetRegSwitch(buf[cv + 1]);
					cv += 7;
					break;

				case 42:
					GetRegSwitch(buf[cv + 2]);
					v[2] = v[7];

					GetRegSwitch(buf[cv + 1]);
					v[7] = v[2] / v[7];

					SetRegSwitch(buf[cv + 1]);
					cv += 3;
					break;

				case 43:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetValueCV(v[1], cv + 6);
					v[7] /= v[1];
					SetStack(v[7], v[2]);
					cv += 10;
					break;

				case 44:
					GetVarAddress(cv + 1);
					GetRegSwitch(buf[cv + 6]);
					GetValueStack(v[1], v[2]);
					v[7] /= v[1];
					SetStack(v[7], v[2]);
					cv += 7;
					break;

				case 45:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					PushStack(v[2]);
					GetVarAddress(cv + 6);
					GetValueStack(v[1], v[2]);
					v[7] /= v[1];
					PopStack(v[2]);
					SetStack(v[7], v[2]);
					cv += 11;
					break;

					//pow
				case opPOW:
					GetValueCV(v[7], cv + 2);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					v[7] = pow(v[2], v[7]);
					SetRegSwitch(buf[cv + 1]);
					cv += 6;
					break;

				case 51:
					GetVarAddress(cv + 2);
					GetValueStack(v[7], v[2]);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					v[7] = pow(v[2], v[7]);
					SetRegSwitch(buf[cv + 1]);
					cv += 7;
					break;

				case 52:
					GetRegSwitch(buf[cv + 2]);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					v[7] = pow(v[2], v[7]);
					SetRegSwitch(buf[cv + 1]);
					cv += 3;
					break;

				case 53:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetValueCV(v[1], cv + 6);
					v[7] = pow(v[7], v[1]);
					SetStack(v[7], v[2]);
					cv += 10;
					break;

				case 54:
					GetVarAddress(cv + 1);
					GetRegSwitch(buf[cv + 6]);
					GetValueStack(v[1], v[2]);
					v[7] = pow(v[7], v[1]);
					SetStack(v[7], v[2]);
					cv += 7;
					break;

				case 55:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					PushStack(v[2]);
					GetVarAddress(cv + 6);
					GetValueStack(v[1], v[2]);
					v[7] = pow(v[7], v[1]);
					PopStack(v[2]);
					SetStack(v[7], v[2]);
					cv += 11;
					break;

				case opIFGE:
					GetValueCV(v[7], cv + 2);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);

					GetValueCV(v[1], cv + 6);

					fl.cm = v[7] >= v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 6 + 4 : v[1];

					break;

				case 143:
					v[6] = v[2];
					GetVarAddress(cv + 2);
					GetValueStack(v[7], v[2]);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);

					GetValueCV(v[1], cv + 7);

					fl.cm = v[7] >= v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 7 + 4 : v[1];

					break;

				case 144:
					v[6] = v[2];
					GetRegSwitch(buf[cv + 2]);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 3);

					fl.cm = v[7] >= v[2] || (fl.co == 1 && fl.cm == 1);

					cv = fl.cm == 1 ? cv + 3 + 4 : v[1];

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					break;

				case 145:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetValueCV(v[2], cv + 6);
					GetValueCV(v[1], cv + 10);

					fl.cm = v[7] >= v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 10 + 4 : v[1];

					break;

				case 146:
					GetVarAddress(cv + 1);
					GetRegSwitch(buf[cv + 6]);
					GetValueStack(v[2], v[2]);
					GetValueCV(v[1], cv + 7);

					fl.cm = v[7] >= v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 7 + 4 : v[1];

					break;

				case 147:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetVarAddress(cv + 6);
					GetValueStack(v[2], v[2]);
					GetValueCV(v[1], cv + 11);

					fl.cm = v[7] >= v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 11 + 4 : v[1];

					break;

				case opIFLE:
					GetValueCV(v[7], cv + 2);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 6);

					fl.cm = v[7] <= v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 6 + 4 : v[1];

					break;

				case 153:
					GetVarAddress(cv + 2);
					GetValueStack(v[7], v[2]);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 7);

					fl.cm = v[7] <= v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 7 + 4 : v[1];

					break;

				case 154:
					GetRegSwitch(buf[cv + 2]);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 3);

					fl.cm = v[7] <= v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 3 + 4 : v[1];

					break;

				case 155:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetValueCV(v[2], cv + 6);
					GetValueCV(v[1], cv + 10);

					fl.cm = v[7] <= v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 10 + 4 : v[1];

					break;

				case 156:
					GetVarAddress(cv + 1);
					GetRegSwitch(buf[cv + 6]);
					GetValueStack(v[2], v[2]);
					GetValueCV(v[1], cv + 7);

					fl.cm = v[7] <= v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 7 + 4 : v[1];

					break;

				case 157:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetVarAddress(cv + 6);
					GetValueStack(v[2], v[2]);
					GetValueCV(v[1], cv + 11);

					fl.cm = v[7] <= v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 11 + 4 : v[1];

					break;

				case opIFG:
					GetValueCV(v[7], cv + 2);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 6);

					fl.cm = v[7] > v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 6 + 4 : v[1];

					break;

				case 163:
					GetVarAddress(cv + 2);
					GetValueStack(v[7], v[2]);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 7);

					fl.cm = v[7] > v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 7 + 4 : v[1];

					break;

				case 164:
					GetRegSwitch(buf[cv + 2]);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 3);

					fl.cm = v[7] > v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 3 + 4 : v[1];

					break;

				case 165:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetValueCV(v[2], cv + 6);
					GetValueCV(v[1], cv + 10);

					fl.cm = v[7] > v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 10 + 4 : v[1];

					break;

				case 166:
					GetVarAddress(cv + 1);
					GetRegSwitch(buf[cv + 6]);
					GetValueStack(v[2], v[2]);
					GetValueCV(v[1], cv + 7);

					fl.cm = v[7] > v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 7 + 4 : v[1];

					break;

				case 167:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetVarAddress(cv + 6);
					GetValueStack(v[2], v[2]);
					GetValueCV(v[1], cv + 11);

					fl.cm = v[7] > v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 11 + 4 : v[1];

					break;

				case opIFL:
					GetValueCV(v[7], cv + 2);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 6);

					fl.cm = v[7] < v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 6 + 4 : v[1];

					break;

				case 173:
					GetVarAddress(cv + 2);
					GetValueStack(v[7], v[2]);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 7);

					fl.cm = v[7] < v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 7 + 4 : v[1];

					break;

				case 174:
					GetRegSwitch(buf[cv + 2]);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 3);

					fl.cm = v[7] < v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 3 + 4 : v[1];

					break;

				case 175:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetValueCV(v[2], cv + 6);
					GetValueCV(v[1], cv + 10);

					fl.cm = v[7] < v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 10 + 4 : v[1];

					break;

				case 176:
					GetVarAddress(cv + 1);
					GetRegSwitch(buf[cv + 6]);
					GetValueStack(v[2], v[2]);
					GetValueCV(v[1], cv + 7);

					fl.cm = v[7] < v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 7 + 4 : v[1];

					break;

				case 177:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetVarAddress(cv + 6);
					GetValueStack(v[2], v[2]);
					GetValueCV(v[1], cv + 11);

					fl.cm = v[7] < v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 11 + 4 : v[1];

					break;

				case opIFE:
					GetValueCV(v[7], cv + 2);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 6);

					fl.cm = v[7] == v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 6 + 4 : v[1];

					break;

				case 183:
					GetVarAddress(cv + 2);
					GetValueStack(v[7], v[2]);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 7);

					fl.cm = v[7] == v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 7 + 4 : v[1];

					break;

				case 184:
					GetRegSwitch(buf[cv + 2]);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 3);

					fl.cm = v[7] == v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 3 + 4 : v[1];

					break;

				case 185:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetValueCV(v[2], cv + 6);
					GetValueCV(v[1], cv + 10);

					fl.cm = v[7] == v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 10 + 4 : v[1];

					break;

				case 186:
					GetVarAddress(cv + 1);
					GetRegSwitch(buf[cv + 6]);
					GetValueStack(v[2], v[2]);
					GetValueCV(v[1], cv + 7);

					fl.cm = v[7] == v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 7 + 4 : v[1];

					break;

				case 187:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetVarAddress(cv + 6);
					GetValueStack(v[2], v[2]);
					GetValueCV(v[1], cv + 11);

					fl.cm = v[7] == v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 11 + 4 : v[1];

					break;

				case opIFNE:
					GetValueCV(v[7], cv + 2);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 6);

					fl.cm = v[7] != v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 6 + 4 : v[1];

					break;

				case 193:
					GetVarAddress(cv + 2);
					GetValueStack(v[7], v[2]);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 7);

					fl.cm = v[7] != v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 7 + 4 : v[1];

					break;

				case 194:
					GetRegSwitch(buf[cv + 2]);
					v[2] = v[7];
					GetRegSwitch(buf[cv + 1]);
					GetValueCV(v[1], cv + 3);

					fl.cm = v[7] != v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 3 + 4 : v[1];

					break;

				case 195:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetValueCV(v[2], cv + 6);
					GetValueCV(v[1], cv + 10);

					fl.cm = v[7] != v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 10 + 4 : v[1];

					break;

				case 196:
					GetVarAddress(cv + 1);
					GetRegSwitch(buf[cv + 6]);
					GetValueStack(v[2], v[2]);
					GetValueCV(v[1], cv + 7);

					fl.cm = v[7] != v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 7 + 4 : v[1];

					break;

				case 197:
					GetVarAddress(cv + 1);
					GetValueStack(v[7], v[2]);
					GetVarAddress(cv + 6);
					GetValueStack(v[2], v[2]);
					GetValueCV(v[1], cv + 11);

					fl.cm = v[7] != v[2] || (fl.co == 1 && fl.cm == 1);

					fl.ca = v[1] == 0;
					fl.co = v[1] == 1;

					cv = fl.cm == 1 || fl.ca == 1 || fl.co == 1 ? cv + 11 + 4 : v[1];

					break;

				case opCALL:
					if (buf[cv + 1] == 0)
					{
						PushStack(bp);
						PushStack(sp);
						PushStack(cv + 6);
						bp = sp;
						sp = bp + 1;
						GetValueCV(cv, cv + 2);
					}
					else
					{
						GetValueCV(v[7], cv + 2);
						CallEngFunction(v[7], v, f, stack, bp, heap);
						cv += 6;
					}
					break;

				case opRET:
					sp = bp;
					PopStack(cv);
					bp = sp - 1;
					sp = stack[bp];
					sp--;
					bp = stack[bp - 1];
					//sp = bp + 1;

					for (register int m = num_heap - 1; m >= 0; m--)
					{
						if (heap[m].stack_pos > sp)
						{
							free(heap[m].string);

							if (num_heap - 1 == 0)
								free(heap);
							else
								heap = realloc(heap, (num_heap - 1) * sizeof(struct MGLHeap));

							num_heap--;

						}
					}

					break;

				case 233:
					GetValueCV(sp, cv + 1);
					cv++;
					break;

				case 234:
					sp = bp;
					cv++;
					break;

				case 235:
					GetValueCV(v[7], cv + 1);
					sp += v[7];
					cv += 5;
					break;

				case 236:
					sp += bp;
					cv++;
					break;

				case 237:
					GetValueCV(v[7], cv + 1);
					bp -= v[7];
					cv += 5;
					break;

				case 238:
					sp -= bp;
					cv++;
					break;

				case 239:
					v[buf[cv + 1]] = stack[bp - 1 - buf[cv + 2]];
					cv += 3;
					break;

				case 240:
					GetVarAddress(cv + 1);
					stack[v[2]] = stack[bp - 1 - buf[cv + 6]];
					cv += 7;
					break;

				case 241:
					GetVarAddress(cv + 1);

					v[7] = stack[v[2]];

					if (v[7] > num_heap)
					{
						LogApp("Invalid heap memory address access - %0x%X", v[7]);
						LogApp("cv: %d - bp: %d - sp: %d", cv, bp, sp);
						error++;
						break;
					}

					if (heap[v[7]].type != 1)
					{
						LogApp("Invalid string access - %0x%X", v[7]);
						LogApp("cv: %d - bp: %d - sp: %d", cv, bp, sp);
						error++;
						break;
					}

					if (heap[v[7]].cur + 12 > heap[v[7]].size)
					{
						//Allocate more memory
						heap[v[7]].size += 32;
						heap[v[7]].mem = realloc(heap[v[7]].mem, heap[v[7]].size);
						CHECKMEM(heap[v[7]].mem);
					}

					strcat(heap[v[7]].mem, StringFormat("%d", v[buf[cv + 5]]));
					heap[v[7]].cur = strlen(heap[v[7]].mem);
					cv += 7;

					break;

				case 242:
					GetVarAddress(cv + 1);

					v[7] = stack[v[2]];

					GetVarAddress(cv + 6);

					v[6] = stack[v[2]];

					if (v[7] > num_heap)
					{
						LogApp("Invalid heap memory address access - %0x%X", v[7]);
						LogApp("cv: %d - bp: %d - sp: %d", cv, bp, sp);
						error++;
						break;
					}

					if (v[6] > num_heap)
					{
						LogApp("Invalid heap memory address access - %0x%X", v[7]);
						LogApp("cv: %d - bp: %d - sp: %d", cv, bp, sp);
						error++;
						break;
					}

					if (heap[v[7]].type != 1)
					{
						LogApp("Invalid string access - %0x%X", v[7]);
						LogApp("cv: %d - bp: %d - sp: %d", cv, bp, sp);
						error++;
						break;
					}

					if (heap[v[6]].type != 1)
					{
						LogApp("Invalid string access - %0x%X", v[7]);
						LogApp("cv: %d - bp: %d - sp: %d", cv, bp, sp);
						error++;
						break;
					}

					if (heap[v[7]].cur + heap[v[6]].cur > heap[v[7]].size)
					{
						//Allocate more memory
						heap[v[7]].size += heap[v[6]].cur + 32;
						heap[v[7]].mem = realloc(heap[v[7]].mem, heap[v[7]].size);
						CHECKMEM(heap[v[7]].mem);
					}

					strcat(heap[v[7]].mem, heap[v[6]].mem);
					heap[v[7]].cur = strlen(heap[v[7]].mem);
					cv += 11;

					break;

				case 243:
					GetVarAddress(cv + 1);

					v[7] = stack[v[2]];

					GetVarAddress(cv + 6);

					v[6] = stack[v[2]];

					if (v[7] > num_heap)
					{
						LogApp("Invalid heap memory address access - %0x%X", v[7]);
						LogApp("cv: %d - bp: %d - sp: %d", cv, bp, sp);
						error++;
						break;
					}

					if (v[6] > num_heap)
					{
						LogApp("Invalid heap memory address access - %0x%X", v[7]);
						LogApp("cv: %d - bp: %d - sp: %d", cv, bp, sp);
						error++;
						break;
					}

					if (heap[v[7]].type != 1)
					{
						LogApp("Invalid string access - %0x%X", v[7]);
						LogApp("cv: %d - bp: %d - sp: %d", cv, bp, sp);
						error++;
						break;
					}

					if (heap[v[6]].type != 1)
					{
						LogApp("Invalid string access - %0x%X", v[7]);
						LogApp("cv: %d - bp: %d - sp: %d", cv, bp, sp);
						error++;
						break;
					}

					if (heap[v[7]].cur < heap[v[6]].size)
					{
						//Allocate more memory
						heap[v[7]].size = heap[v[6]].size;
						heap[v[7]].mem = realloc(heap[v[7]].mem, heap[v[7]].size);
						CHECKMEM(heap[v[7]].mem);
					}

					strcpy(heap[v[7]].mem, heap[v[6]].mem);
					heap[v[7]].cur = strlen(heap[v[7]].mem);
					cv += 11;

					break;

				case 244:
					GetVarAddress(cv + 1);

					v[7] = stack[v[2]];

					GetVarAddress(cv + 6);

					v[6] = stack[v[2]];

					if (v[7] > num_heap)
					{
						LogApp("Invalid heap memory address access - %0x%X", v[7]);
						LogApp("cv: %d - bp: %d - sp: %d", cv, bp, sp);
						error++;
						break;
					}

					if (v[6] > num_heap)
					{
						LogApp("Invalid heap memory address access - %0x%X", v[7]);
						LogApp("cv: %d - bp: %d - sp: %d", cv, bp, sp);
						error++;
						break;
					}

					if (heap[v[7]].type != 1)
					{
						LogApp("Invalid string access - %0x%X", v[7]);
						LogApp("cv: %d - bp: %d - sp: %d", cv, bp, sp);
						error++;
						break;
					}

					if (heap[v[6]].type != 1)
					{
						LogApp("Invalid string access - %0x%X", v[7]);
						LogApp("cv: %d - bp: %d - sp: %d", cv, bp, sp);
						error++;
						break;
					}

					v[2] = strcmp(heap[v[7]].mem, heap[v[6]].mem);
					heap[v[7]].cur = strlen(heap[v[7]].mem);
					cv += 11;

					break;

				case 250:
					v[buf[cv + 1]] = f[buf[cv + 2]];
					cv += 3;
					break;

				case 251:
					f[buf[cv + 1]] = v[buf[cv + 2]];
					cv += 3;
					break;

				case 255:
					//Extended instruction set
					state = 1;
					cv++;
					break;

				default:
					MessageBoxRes("MGVM error", MB_OK, "VM error: Invalid instruction in bytecode:\naddress: %d, byte: %d, stack address: %d",
						cv, buf[cv + 1], bp);
					st.quit = 1;
				}
			}
			else
			if (state == 1)
			{
				switch (buf[cv])
				{
					//setf
				case 0:
					GetValueCV(v[7], cv + 2);
					GetRegfSwitch(buf[cv + 1]);
					f[4] = v[7];
					SetRegfSwitch(buf[cv + 1]);
					cv += 6;
					break;

				case 1:
					//GetValueCV(f[3], cv + 2);
					memcpy(f + 3, buf + cv + 2, sizeof(float));
					GetRegfSwitch(buf[cv + 1]);
					f[4] = f[3];
					SetRegfSwitch(buf[cv + 1]);
					cv += 6;
					break;

				case 2:
					GetRegfSwitch(buf[cv + 1]);
					GetRegSwitch(buf[cv + 2]);
					f[4] = v[7];
					SetRegfSwitch(buf[cv + 1]);
					cv += 3;
					break;

				case 3:
					GetVarAddress(cv + 2);
					GetValueStack(v[7], v[2]);
					GetRegfSwitch(buf[cv + 1]);
					f[4] = v[7];
					SetRegfSwitch(buf[cv + 1]);
					cv += 7;
					break;

				case 4:
					GetVarAddress(cv + 2);
					GetValueStack(f[3], v[2]);
					GetRegfSwitch(buf[cv + 1]);
					f[4] = f[3];
					SetRegfSwitch(buf[cv + 1]);
					cv += 7;
					break;

				case 5:
					GetRegfSwitch(buf[cv + 2]);
					f[3] = f[4];
					GetRegfSwitch(buf[cv + 1]);
					f[4] = f[3];
					SetRegfSwitch(buf[cv + 1]);
					cv += 3;
					break;

				case 6:
					GetVarAddress(cv + 1);
					GetValueStack(f[4], v[2]);
					GetValueCV(f[3], cv + 5);
					f[4] = f[3];
					memcpy(stack + v[2], &f[4], 4);
					cv += 10;
					break;

				case 7:
					GetVarAddress(cv + 1);
					GetRegfSwitch(buf[cv + 6]);
					GetValueStack(f[3], v[2]);
					f[3] = f[4];
					//SetStack(f[4], v[2]);
					memcpy(stack + v[2], &f[3], 4);
					cv += 7;
					break;

				case 8:
					GetVarAddress(cv + 1);
					GetValueStack(f[4], v[2]);
					PushStack(v[2]);
					GetVarAddress(cv + 6);
					GetValueStack(v[7], v[2]);
					f[4] = v[7];
					PopStack(v[2]);
					cv += 11;
					break;

				case 9:
					GetVarAddress(cv + 1);
					GetValueStack(f[4], v[2]);
					PushStack(v[2]);
					GetVarAddress(cv + 6);
					GetValueStack(f[3], v[2]);
					f[4] = pow(f[4], f[3]);
					PopStack(v[2]);
					cv += 11;
					break;

					//powf
				case 50:
					GetValueCV(v[7], cv + 2);
					GetRegfSwitch(buf[cv + 1]);
					f[4] = pow(f[3], v[7]);
					SetRegfSwitch(buf[cv + 1]);
					cv += 6;
					break;

				case 51:
					GetValueCV(f[3], cv + 2);
					GetRegfSwitch(buf[cv + 1]);
					f[4] = pow(f[3], f[4]);
					SetRegfSwitch(buf[cv + 1]);
					cv += 6;
					break;

				case 52:
					GetRegfSwitch(buf[cv + 2]);
					GetRegSwitch(buf[cv + 1]);
					f[4] = pow(f[4], v[7]);
					SetRegfSwitch(buf[cv + 1]);
					cv += 3;
					break;

				case 53:
					GetVarAddress(cv + 2);
					GetValueStack(v[7], v[2]);
					GetRegfSwitch(buf[cv + 1]);
					f[4] = pow(f[4], v[7]);
					SetRegfSwitch(buf[cv + 1]);
					cv += 7;
					break;

				case 54:
					GetVarAddress(cv + 2);
					GetValueStack(f[3], v[2]);
					GetRegfSwitch(buf[cv + 1]);
					f[4] = pow(f[4], f[3]);
					SetRegfSwitch(buf[cv + 1]);
					cv += 7;
					break;

				case 55:
					GetRegfSwitch(buf[cv + 2]);
					f[3] = f[4];
					GetRegfSwitch(buf[cv + 1]);
					f[4] = pow(f[3], f[4]);
					SetRegfSwitch(buf[cv + 1]);
					cv += 3;
					break;

				case 56:
					GetVarAddress(cv + 1);
					GetValueStack(f[4], v[2]);
					GetValueCV(f[3], cv + 5);
					f[4] = pow(f[4], f[3]);
					memcpy(stack + v[2], &f[4], 4);
					cv += 10;
					break;

				case 57:
					GetVarAddress(cv + 1);
					GetRegfSwitch(buf[cv + 6]);
					GetValueStack(f[3], v[2]);
					f[4] = pow(f[3], f[4]);
					//SetStack(f[4], v[2]);
					memcpy(stack + v[2], &f[4], 4);
					cv += 7;
					break;

				case 58:
					GetVarAddress(cv + 1);
					GetValueStack(f[4], v[2]);
					PushStack(v[2]);
					GetVarAddress(cv + 6);
					GetValueStack(v[7], v[2]);
					f[4] = pow(f[4], v[7]);
					PopStack(v[2]);
					cv += 11;
					break;

				case 59:
					GetVarAddress(cv + 1);
					GetValueStack(f[4], v[2]);
					PushStack(v[2]);
					GetVarAddress(cv + 6);
					GetValueStack(f[3], v[2]);
					f[4] = pow(f[4], f[3]);
					PopStack(v[2]);
					cv += 11;
					break;

				default:
					MessageBoxRes("MGVM error", MB_OK, "VM error: Invalid instruction (set 2#) in bytecode:\naddress: %d, byte: %d, stack address: %d",
						cv, buf[cv + 1], bp);
					st.quit = 1;
				}

				state = 0;
			}
		}
	}

	st.mgl.bp = bp;
	st.mgl.sp = sp;
	st.mgl.cv = cv;
	st.mgl.flags = fl;
	memcpy(st.mgl.v, v, 32 * sizeof(uint32));
	memcpy(st.mgl.f, f, 24 * sizeof(float));

	st.mgl.heap = heap;
	st.mgl.num_heap = num_heap;

	st.mgl.stack = stack;

	return 1;
}

void CleanupHeap(struct MGLHeap *heap, uint16 num_heap)
{
	for (register int m = num_heap - 1; m >= 0; m--)
		free(heap[m].string);

	num_heap = 0;

	free(heap);
}