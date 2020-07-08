#include "mgl.h" 
#include <stdio.h>

struct MGLHeap
{
	size_t size;
	size_t cur;
	void *mem;
	char *string;
	uint8 type; //0 - buffer, 1 - string
};

eng_calls e_funcs[] =
{
	{ "log", E_LOG, 2, 0 }, //0
	{ "drawline", E_DRAWLINE, 10, 0 }, //1
	{ "msgbox", E_MSGBOX, 4, 0 }, //2
	{ "getsys.Screen", C_GSYSSCREEN, 4, 1} //3
};

const uint16 num_efuncs = 4;

void GetSystemScreen(int32 *w, int32 *h, int32 *bpp, int32 *fullscreen)
{
	if (w != NULL)
		*w = st.screenx;

	if (h != NULL)
		*h = st.screeny;

	if (bpp != NULL)
		*bpp = st.bpp;

	if (fullscreen != NULL)
		*fullscreen = st.fullscreen;
}

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

	uint8 x = mem[0];
	uint8 y = mem[1];

	mem[0] = mem[3];
	mem[1] = mem[2];
	mem[2] = y;
	mem[3] = x;

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

	if (cur > code->function_table[curfunc].size - 20)
	{
		code->function_table[curfunc].size += 64;
		code->function_code[curfunc] = realloc(code->function_code[curfunc], code->function_table[curfunc].size);

		code->bt_trl[curfunc] = realloc(code->bt_trl[curfunc], code->function_table[curfunc].size * sizeof(uint32));

		CHECKMEM(code->function_code[curfunc]);
		CHECKMEM(code->bt_trl[curfunc]);
	}
}

MGMC *CompileMGL(FILE *file, uint8 optimization)
{
	MGMC *code;
	char buf[4096], *tok, str1[64], str2[64];
	uint16 func = 0, error = 0, line = 0, curfunc = 0;
	int32 val1, val2, i, cv = 0, cv1 = 0;
	uint32 brackets = 0, expect_bracket = 0, ret = 0;
	float valf1, valf2;

	code = calloc(1, sizeof(MGMC));
	CHECKMEM(code);

	while (!feof(file))
	{
		line++;
		ZeroMem(buf, 4096);
		fgets(buf, 4096, file);

		if (buf[0] == '\0' || buf[0] == '\n')
			continue;

		tok = strtok(buf, " \n\t");

		if (tok == NULL)
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

			tok = strtok(NULL, " \"");

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
		if (strcmp(tok, "set") == NULL || strcmp(tok, "add") == NULL || strcmp(tok, "sub") == NULL || strcmp(tok, "mul") == NULL || strcmp(tok, "div") == NULL
			|| strcmp(tok, "pow") == NULL)
		{
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
							val2 = tok[3] - 48 + 4;

						if (strlen(tok) == 5 && tok[3] != 'f')
							val2 = atoi(tok + 3) + 4;

						if (strlen(tok) == 5 && tok[3] == 'f')
							val2 = tok[4] - 48 + 4;

						if (strlen(tok) == 6 && tok[3] == 'f')
							val2 = atoi(tok + 4) + 4;

						g1 = 3;
					}
					else
					if (strcmp(tok, "return") == NULL)
					{
						val2 = 0;
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

		}
		else
		if (strcmp(tok, "func") == NULL) //Function entry point
		{
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

			expect_bracket = 1;
			cv = 0;
			cv1 = 0;
		}
		else
		if (strcmp(tok, "return") == NULL)
		{
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

			ret = 1;
		}
		else
		if (strcmp(tok, "call") == NULL)
		{
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

					if (g == 2)
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
				code->function_code[curfunc] = realloc(code->function_code[curfunc], code->function_table[curfunc].size);

				func = 0;
			}
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

	LogApp("Compiled %d functions and %d global variables  - total size: %l", code->num_functions, code->num_vars, tsize);

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

		if (c[cv + 1] != 2)
			Copy32toBuf(c + cv + 2, code->vars_table[j].value);
		else
			CopyFloatToInt32(c + cv + 2, code->vars_table[j].valuef);

		cv += 6;
	}

	for (i = 0; i < code->num_functions; i++)
	{
		if (cv > csize - 32)
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
			if (cv > csize - 10)
			{
				c = realloc(c, csize + 128);
				CHECKMEM(c);
				csize += 128;
			}

			c[cv] = 254;
			c[cv + 1] = code->function_table[i].vars_table[j].type;

			if (c[cv + 1] != 2)
				Copy32toBuf(c + cv + 2, code->function_table[i].vars_table[j].value);
			else
				CopyFloatToInt32(c + cv + 2, code->function_table[i].vars_table[j].valuef);

			cv += 6;
		}

		for (cv1 = 0; cv1 < code->function_table[i].size; )
		{
			if (cv > csize - 16)
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
			if (c[cv] == 255 && c[cv] < 56)
			{
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

	if (file == NULL)
	{
		//TESTING
		st.mgl.size = 1024;
		st.mgl.code = malloc(st.mgl.size);
		CHECKMEM(st.mgl.code);

		unsigned char *c = st.mgl.code;

		c[0] = 'M';
		c[1] = 'G';
		c[2] = 'L';
		c[3] = ' ';

		c[4] = 30 >> 24;
		c[5] = (30 >> 16) & 0xFF;
		c[6] = (30 >> 8) & 0xFF;
		c[7] = 30 & 0xFF;

		c[24] = 0;

		//Define two global variables
		c[29] = 254;
		c[30] = 1;
		c[31] = 16384 >> 24;
		c[32] = (16384 >> 16) & 0xFF;
		c[33] = (16384 >> 8) & 0xFF;
		c[34] = 16384 & 0xFF;

		c[35] = 254;
		c[36] = 1;
		c[37] = 32768 >> 24;
		c[38] = (32768 >> 16) & 0xFF;
		c[39] = (32768 >> 8) & 0xFF;
		c[40] = 32768 & 0xFF;

		//Init entry point
		//define local var
		c[41] = 254;
		c[42] = 1;
		c[43] = 65536 >> 24;
		c[44] = (65536 >> 16) & 0xFF;
		c[45] = (65536 >> 8) & 0xFF;
		c[46] = 65536 & 0xFF;

		//set regi mem
		c[47] = 1;
		c[48] = 14;
		c[49] = 0;
		c[50] = 0 >> 24;
		c[51] = (0 >> 16) & 0xFF;
		c[52] = (0 >> 8) & 0xFF;
		c[53] = 0 & 0xFF;

		//set regi regi
		c[54] = 2;
		c[55] = 15;
		c[56] = 14;

		//set mem regi
		c[57] = 4;
		c[58] = 0;
		c[59] = 0 >> 24;
		c[60] = (0 >> 16) & 0xFF;
		c[61] = (0 >> 8) & 0xFF;
		c[62] = 0 & 0xFF;
		c[63] = 16;

		//add mem mem
		c[64] = 15;
		c[65] = 1;
		c[66] = 1 >> 24;
		c[67] = (1 >> 16) & 0xFF;
		c[68] = (1 >> 8) & 0xFF;
		c[69] = 1 & 0xFF;
		c[70] = 0;
		c[71] = 0 >> 24;
		c[72] = (0 >> 16) & 0xFF;
		c[73] = (0 >> 8) & 0xFF;
		c[74] = 0 & 0xFF;
		c[75] = 210;
	}
	else
	{
		openfile_d(f, file, "rb");

		fseek(f, 0, SEEK_END);

		st.mgl.size = ftell(f);
		rewind(f);

		st.mgl.code = malloc(st.mgl.size);
		CHECKMEM(st.mgl.code);

		fread(st.mgl.code, 1, st.mgl.size, f);

		fclose(f);
	}

	if (st.mgl.code[0] != 'M' || st.mgl.code[1] != 'G' || st.mgl.code[2] != 'L' && st.mgl.code[3] != ' ')
	{
		free(st.mgl.code);
		return NULL;
	}

	if (st.mgl.code[24] == 0)
	{
		st.mgl.memsize = 131072;
		st.mgl.stack_type = 0;
	}
	else
	{
		st.mgl.memsize = (st.mgl.code[25] << 24) | (st.mgl.code[26] << 16) | (st.mgl.code[27] << 8) | st.mgl.code[28];
		st.mgl.stack_type = 1;
	}

	ZeroMem(st.mgl.v, sizeof(8 * sizeof(uint32)));
	st.mgl.bp = st.mgl.sp = 0;

	///Load engine calls

	st.mgl.funcs.log = LogApp;
	st.mgl.funcs.drawline = LineData;
	st.mgl.funcs.msgbox = MessageBoxRes;

	return 1;
}

int8 CallEngFunction(int32 address, int32 *v, int32 *stack, int32 bp, struct MGLHeap *heap)
{
	//mem_assert(stack);
	//mem_assert(heap);

	bp -= 2;

	switch (address)
	{
		case E_LOG:
			st.mgl.funcs.log(heap[v[9]].string);
			break;

		case E_DRAWLINE:
			st.mgl.funcs.drawline(v[9], v[10], v[11], v[12], v[13], v[14], v[15], v[16], v[17], v[18]);
			break;

		case C_GSYSSCREEN:
			st.mgl.funcs.getsystemScreenSize(&v[9], &v[10], &v[11], &v[12]);
			break;
	}
}

int8 ExecuteMGLCode(uint8 location)
{
	register int32 bp, sp, cv;
	int32 v[32];

	static int32 stack1[131072], *stack2, *stack, num_heap = 0;
	uint8 *vars, state = 0;

	static struct MGLHeap *heap;

	float f[24];

	unsigned char *buf = st.mgl.code;

	int error, funcaddr;

	cv = st.mgl.cv;
	memcpy(v, st.mgl.v, 8 * sizeof(uint32));

	if (st.mgl.stack_type == 0)
		stack = stack1;
	else
	{
		stack2 = malloc(st.mgl.memsize * sizeof(uint32));
		CHECKMEM(stack2);
		stack = stack2;
	}

	//location - 0: Init, 1: PreGame, 2: GameLoop (game clock), 3: MainLoop(engine clock), 4: End

	/*
	Type definition:
	0 - constant
	1 - int
	2 - float
	3 - buffer
	4 - string
	7 - st
	8 - map
	9 - mgg_map
	10 - mgg_game
	11 - mgg_sys
	12 - v[1]
	13 - tmp
	14 - v1
	15 - v2
	16 - v3
	17 - v4
	18 - i
	19 - cur
	20 - bp
	21 - sp
	22 - cv
	23 - f1
	24 - f2
	25 - f3
	26 - f4

	Command definition: - after every command, there's a type variable indicator and if its global or local

	Each command has variations depending on the argument type
	0 - com regi const
	1 - com regi mem
	2 - com regi regi
	3 - com mem const
	4 - com mem regi
	5 - com mem mem
	7 - com const const
	8 - com const regi
	9 - com const mem

	If the command accepts only one command
	0 - com const
	1 - com constf
	3 - com regi
	4 - com regf
	5 - com mem

	If the command is a float variation
	0 - com regf const
	1 - com regf constf
	2 - com regf regi
	3 - com regf mem
	4 - com regf regf
	5 - com mem constf
	6 - com mem regf
	7 - com mem mem
	8 - com constf constf
	9 - com constf regf

	//List of commands
	0 - set x y
	10 - add x y
	20 - sub x y
	30 - mul x y
	40 - div x y

	50 - pow x y
	60 - powf x y
	70 - logf x y
	80 - sqrtf x

	86 - cosf x
	92 - sinf x
	98 - tanf x
	104 - acosf x
	110 - asinf x
	116 - atanf x

	126 - and x y
	136 - or x y
	146 - xor x y

	156 - if>= x y z
	166 - if<= x y
	176 - if> x y
	186 - if< x y
	196 - if= x y
	206 - if!= x y

	207 - while iftype
	208 - loop

	209 - call x n ... //Go to the address and take the arguments to the stack - n is the number os arguments
	// if call is calling cv, it means its calling an engine function - x is the engine function address
	210 - ret x

	211 - shiftl x y
	221 - shiftr x y

	231 - setmem buffer regi
	232 - compmem buffer buffer - v[1]
	232 - copymem buffer buffer

	233 - compstring string string - v[1]
	234 - copystring string string
	235 - tokenstring string string - v[1]
	236 - stringstring string string  - v[1]
	237 - charstring string x - v[1]

	238 - openfile buffer string - v[1]
	239 - closefile buffer
	240 - printfile buffer string ...
	241 - scanfile buffer string ...
	242 - writefile buffer buffer x - v[1]
	243 - readfile buffer buffer x - v[1]
	244 - seekfile buffer x y - v[1]
	245 - rewindfile buffer
	246 - sizefile buffer - v[1]

	247 - jump x - jumps to address

	//These should come after an if comparision
	248 - jumpif x - jumps to address if v[1] is 1
	249 - jumpelse x - jumps to address if v[1] is 0

	254 - data //define a variable in the stack
	255 - next byte is the extended command list

	//Extended commands - 01
	0 - setf x y
	10 - addf x y
	20 - subf x y
	30 - mulf x y
	40 - divf x y

	50 - iff>= x y
	60 - iff<= x y
	70 - iff> x y
	80 - iff< x y
	90 - iff= x y
	100 - iff!= x y

	110 - quit
	111 - messagebox string type string ... - v[1]
	112 - log string ...

	//dont look at these yet
	48 - drawpui x y w h frame a z tex_offset_x tex_offset_y tex_offset_w tex_offset_h
	49 - drawphud x y w h frame a z tex_offset_x tex_offset_y tex_offset_w tex_offset_h
	50 - drawpstring x y size string
	51 - drawpstringui x y size string
	52 - drawpstringui2 x y size string
	53 - spawnsprite x y w h sprite - v[1]
	54 - loadmap string - v[1]
	55 - loadmgg string - v[1]
	56 - freemap x
	57 - freemgg x
	58 - checkcolworld x y z w h x2 y2 z2 w2 h2 - v[1]
	59 - checkcol x y z w h x2 y2 z2 w2 h2 - v[1]
	60 - checkcolsprite x y - v[1]
	61 - checkcolspritescene x y - v[1]
	*/

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

	if (location == 0)
	{
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
				MessageBoxRes("MGVM error", MB_OK, "Error: detected invalid operator in the stack definition bytecode:\naddress: %d, byte: %d, stack address: %d",
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

				GetValueCV(v[7], cv + 2);
				heap[num_heap].size = v[7];
				heap[num_heap].type = 1; //String
				heap[num_heap].string = malloc(v[7]);
				CHECKMEM(heap[num_heap].string);
				memcpy(heap[num_heap].string, buf + cv + 6, v[7]);

				cv += 6 + v[7];
				num_heap++;

				sp++;
			}
		}
		else
		{
			if (state == 0)
			{
				switch (buf[cv])
				{
					//set
				case 0:
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
					GetVarAddress(cv + 1);
					GetRegSwitch(buf[cv + 6]);
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
				case 10:
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
					GetValueCV(v[1], cv + 5);
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
				case 20:
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
					GetValueCV(v[1], cv + 5);
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
				case 30:
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
					GetValueCV(v[1], cv + 5);
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
				case 40:
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
					GetValueCV(v[1], cv + 5);
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
				case 50:
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
					GetValueCV(v[1], cv + 5);
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

				case 204:
					if (buf[cv + 1] == 0)
					{
						PushStack(bp);
						PushStack(cv + 6);
						bp = sp;
						sp = bp + 1;
						GetValueCV(cv, cv + 2);
					}
					else
					{
						GetValueCV(v[7], cv + 2);
						CallEngFunction(v[7], v, stack, bp, heap);
						cv += 6;
					}
					break;

				case 210:
					sp = bp;
					PopStack(cv);
					bp = sp - 1;
					bp = stack[bp];
					sp = bp + 1;
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
				}
			}
			else
			if (state == 1)
			{
				switch (buf[cv])
				{
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
				}

				state = 0;
			}
		}
	}

	st.mgl.bp = bp;
	st.mgl.sp = sp;
	st.mgl.cv = cv;
	memcpy(st.mgl.v, v, 8 * sizeof(uint32));

	return 1;
}