#include "mgl.h" 
#include <stdio.h>

eng_calls e_funcs[] =
{
	{ "log", E_LOG, 2 }, //0
	{ "drawline", E_DRAWLINE, 10 }, //1
	{ "msgbox", E_MSGBOX, 4 } //2
};

const uint16 num_efuncs = 3;

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

MGMC *CompileMGL(FILE *file, uint8 optimization)
{
	MGMC *code;
	char buf[4096], *tok, str1[64], str2[64];
	uint16 func = 0, error = 0, line = 0, curfunc = 0;
	int32 val1, val2, i, cv = 0, cv1 = 0;
	uint32 brackets = 0, expect_bracket = 0;
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

		tok = strtok(buf, " ");

		if (tok == NULL)
			continue;

		//Variable declarations
		if (strcmp(tok, "var") == NULL)
		{
			tok = strtok(NULL, " ");

			if (tok == NULL)
			{
				error++;
				LogApp("Compiler error: missing variable name in declaration - line: %d", line);
				continue;
			}

			strcpy(str1, tok);

			tok = strtok(NULL, " ");

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
				if (code->vars_table == 1)
					code->vars_table = calloc(1, sizeof(code->vars_table));
				else
					code->vars_table = realloc(code->vars_table, code->num_vars * sizeof(code->vars_table));

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
				if (code->function_table[curfunc].vars_table == 1)
					code->function_table[curfunc].vars_table = calloc(1, sizeof(code->function_table[curfunc].vars_table));
				else
					code->function_table[curfunc].vars_table = realloc(code->function_table[curfunc].vars_table,
					code->function_table[curfunc].num_vars * sizeof(code->function_table[curfunc].vars_table));

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
			tok = strtok(NULL, " ");

			if (tok == NULL)
			{
				error++;
				LogApp("Compiler error: missing variable name in declaration - line: %d", line);
				continue;
			}

			strcpy(str1, tok);

			tok = strtok(NULL, " ");

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
				if (code->vars_table == 1)
					code->vars_table = calloc(1, sizeof(code->vars_table));
				else
					code->vars_table = realloc(code->vars_table, code->num_vars * sizeof(code->vars_table));

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
				if (code->function_table[curfunc].vars_table == 1)
					code->function_table[curfunc].vars_table = calloc(1, sizeof(code->function_table[curfunc].vars_table));
				else
					code->function_table[curfunc].vars_table = realloc(code->function_table[curfunc].vars_table,
					code->function_table[curfunc].num_vars * sizeof(code->function_table[curfunc].vars_table));

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
			tok = strtok(NULL, " ");

			if (tok == NULL)
			{
				error++;
				LogApp("Compiler error: missing buffer name in declaration - line: %d", line);
				continue;
			}

			strcpy(str1, tok);

			tok = strtok(NULL, " ");

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
				if (code->vars_table == 1)
					code->vars_table = calloc(1, sizeof(code->vars_table));
				else
					code->vars_table = realloc(code->vars_table, code->num_vars * sizeof(code->vars_table));

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
				if (code->function_table[curfunc].vars_table == 1)
					code->function_table[curfunc].vars_table = calloc(1, sizeof(code->function_table[curfunc].vars_table));
				else
					code->function_table[curfunc].vars_table = realloc(code->function_table[curfunc].vars_table,
					code->function_table[curfunc].num_vars * sizeof(code->function_table[curfunc].vars_table));

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
			tok = strtok(NULL, " ");

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
				if (code->vars_table == 1)
					code->vars_table = calloc(1, sizeof(code->vars_table));
				else
					code->vars_table = realloc(code->vars_table, code->num_vars * sizeof(code->vars_table));

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
				if (code->function_table[curfunc].vars_table == 1)
					code->function_table[curfunc].vars_table = calloc(1, sizeof(code->function_table[curfunc].vars_table));
				else
					code->function_table[curfunc].vars_table = realloc(code->function_table[curfunc].vars_table,
					code->function_table[curfunc].num_vars * sizeof(code->function_table[curfunc].vars_table));

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

			tok = strtok(NULL, " ");

			int g = 0, f = 0, c = 0;

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

							if (code->vars_table[i].type == 3)
								f |= 1;

							val1 = -i;
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

							if (code->function_table[curfunc].vars_table[i].type == 3)
								f |= 1;

							val1 = -i;
							g = 1;
							break;
						}
					}

					if (g == -1)
						continue;

					if (val1 == -1)
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

			tok = strtok(NULL, " ");

			if (tok != NULL)
			{
				if (IsNumber(tok) == 0)
				{
					val2 = -1;

					for (i = 0; i < code->num_vars; i++)
					{
						if (strcmp(code->vars_table[i].name, tok) == NULL)
						{
							//Found global variable in the table
							code->vars_table[i].num_use++;

							if (code->vars_table[i].type == 3)
								f |= 2;

							val2 = -i;
							break;
						}
					}

					for (i = 0; i < code->function_table[curfunc].num_vars; i++)
					{
						if (strcmp(code->function_table[curfunc].vars_table[i].name, tok) == NULL)
						{
							//Found local variable in the table
							if (val2 < -1)
							{
								error++;
								LogApp("Compiler error: function \"%s\" local variable \"%s\" conflicting with global variable with the same name in \"%s\" command - line: %d",
									code->function_table[curfunc].name, tok, str1, line);
								g = -1;
								break;
							}

							code->function_table[curfunc].vars_table[i].num_use++;

							val2 = -i;

							if (code->function_table[curfunc].vars_table[i].type == 3)
								f |= 2;

							g |= 2;
							break;
						}
					}

					if (g == -1)
						continue;

					if (val2 == -1)
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

			if (strcmp(tok, "set") == NULL)
				t = 0;
			else
			if (strcmp(tok, "add") == NULL)
				t = 10;
			else
			if (strcmp(tok, "sub") == NULL)
				t = 20;
			else
			if (strcmp(tok, "mul") == NULL)
				t = 30;
			else
			if (strcmp(tok, "div") == NULL)
				t = 40;
			else
			if (strcmp(tok, "pow") == NULL)
				t = 50;

			if (c == 1 && f == 0)
			{
				val1 *= -1;
				val1 -= 1;

				code->function_code[curfunc][cv] = t + 3;
				code->function_code[curfunc][cv + 1] = g & 1 == 1 ? 0 : 1;
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
			if (c == 1 && f & 2 == 1 && f & 1 == 0)
			{
				val1 *= -1;
				val1 -= 1;

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
				code->function_code[curfunc][cv + 11] = g & 1 == 1 ? 0 : 1;
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
			if (c == 1 && f & 1 == 1 && f & 2 == 0)
			{
				val1 *= -1;
				val1 -= 1;

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
				code->function_code[curfunc][cv + 11] = g & 1 == 1 ? 0 : 1;
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
			if (c == 0 && f & 1 == 1 && f & 2 == 1)
			{
				val1 *= -1;
				val1 -= 1;

				val2 *= -1;
				val2 -= 1;

				code->function_code[curfunc][cv] = 255;
				code->function_code[curfunc][cv + 1] = t + 9;
				code->function_code[curfunc][cv + 2] = g & 1 == 1 ? 0 : 1;
				code->function_code[curfunc][cv + 3] = val1 >> 24;
				code->function_code[curfunc][cv + 4] = (val1 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 5] = (val1 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 6] = val1 & 0xFF;
				code->function_code[curfunc][cv + 7] = g & 2 == 1 ? 0 : 1;
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
			if (c == 0 && f & 1 == 1 && f & 2 == 0)
			{
				val1 *= -1;
				val1 -= 1;

				val2 *= -1;
				val2 -= 1;

				code->function_code[curfunc][cv] = 255;
				code->function_code[curfunc][cv + 1] = t + 8;
				code->function_code[curfunc][cv + 2] = g & 1 == 1 ? 0 : 1;
				code->function_code[curfunc][cv + 3] = val1 >> 24;
				code->function_code[curfunc][cv + 4] = (val1 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 5] = (val1 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 6] = val1 & 0xFF;
				code->function_code[curfunc][cv + 7] = g & 2 == 1 ? 0 : 1;
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
			if (c == 0 && f & 1 == 0 && f & 2 == 1)
			{
				val1 *= -1;
				val1 -= 1;

				val2 *= -1;
				val2 -= 1;

				code->function_code[curfunc][cv] = 255;
				code->function_code[curfunc][cv + 1] = t + 4;
				code->function_code[curfunc][cv + 2] = 0;
				code->function_code[curfunc][cv + 3] = g & 1 == 1 ? 0 : 1;
				code->function_code[curfunc][cv + 4] = val2 >> 24;
				code->function_code[curfunc][cv + 5] = (val2 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 6] = (val2 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 7] = val2 & 0xFF;

				code->function_code[curfunc][cv + 8] = 250;
				code->function_code[curfunc][cv + 9] = 6;
				code->function_code[curfunc][cv + 10] = 0;

				code->function_code[curfunc][cv + 11] = t + 4;
				code->function_code[curfunc][cv + 12] = g & 1 == 1 ? 0 : 1;
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
			if (c == 0 && f & 1 == 0 && f & 2 == 0)
			{
				val1 *= -1;
				val1 -= 1;

				val2 *= -1;
				val2 -= 1;

				code->function_code[curfunc][cv] = t + 5;
				code->function_code[curfunc][cv + 1] = g & 1 == 1 ? 0 : 1;
				code->function_code[curfunc][cv + 2] = val1 >> 24;
				code->function_code[curfunc][cv + 3] = (val1 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 4] = (val1 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 5] = val1 & 0xFF;
				code->function_code[curfunc][cv + 6] = g & 2 == 1 ? 0 : 1;
				code->function_code[curfunc][cv + 7] = val2 >> 24;
				code->function_code[curfunc][cv + 8] = (val2 >> 16) & 0xFF;
				code->function_code[curfunc][cv + 9] = (val2 >> 8) & 0xFF;
				code->function_code[curfunc][cv + 10] = val2 & 0xFF;

				cv += 11;

				code->bt_trl[curfunc][cv1] = t + 5;

				cv1 += 1;
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

			tok = strtok(NULL, " ");

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
				code->function_table = calloc(1, sizeof(code->function_table));
			else
				code->function_table = realloc(code->function_table, code->num_functions * sizeof(code->function_table));

			CHECKMEM(code->function_table);

			strcpy(code->function_table[i].name, tok);
			curfunc = i;
			code->function_table[i].num_vars = 0;
			code->function_table[i].num_use = 0;
			code->function_table[i].size = 0;
			code->function_table[i].address = cv;
			code->function_table[i].num_args = 0;

			while ((tok = strtok(NULL, " ")) != NULL)
			{
				if (strcmp(tok, "var") == NULL || strcmp(tok, "float") == NULL || strcmp(tok, "buffer") == NULL || strcmp(tok, "string") == NULL)
					code->function_table[i].num_args++;
				else
				{
					error++;
					LogApp("Compiler error: undefined keyword \"%s\" in function argument list \"%s\" - line: %d", tok, code->function_table[i].name, line);
					code->num_functions--; //Remove function due to error
					code->function_table = realloc(code->function_table, code->num_functions * sizeof(code->function_table));
					CHECKMEM(code->function_table);
					func = 0;
					break;
				}
			}

			expect_bracket = 1;
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

			tok = strtok(NULL, " ");

			int g = 0, f = 0, c = 0;

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

							if (code->vars_table[i].type == 3)
								f |= 2;

							val1 = -i;
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

							val1 = -i;

							if (code->function_table[curfunc].vars_table[i].type == 3)
								f |= 2;

							g |= 2;
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
				cv++;
				continue;
			}

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
				code->function_code[curfunc][cv + 2] = g == 2 ? 0 : 1;
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
				code->function_code[curfunc][cv + 3] = g == 2 ? 0 : 1;
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

			tok = strtok(NULL, " ");

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
				for (i = 0; i < num_efuncs; i++)
				{
					if (GetEngFunc(tok) != -1)
					{
						//Found engine function
						a = 2;
						break;
					}
				}
			}

			strcpy(str1, tok);

			int c = 0, g = 0, f = 0, j = 0, b = 0;
			val1 = -1;

			while ((tok = strtok(NULL, " ")) != NULL)
			{
				c = 0;
				val1 = -1;
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

							if (code->vars_table[j].type == 2)
								f = 1;

							break;
						}
					}

					if (val1 == -1)
					{
						for (j = 0; j < code->function_table[curfunc].num_vars; j++)
						{
							if (strcmp(tok, code->function_table[curfunc].vars_table[j].name) == NULL)
							{
								val1 = j;

								g = 1;

								if (code->function_table[curfunc].vars_table[j].type == 2)
									f = 1;

								break;
							}
						}

						if (val1 == -1)
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

				if (c == 1 && f == 0)
				{
					code->function_code[curfunc][cv] = 205;
					code->function_code[curfunc][cv + 1] = val1 >> 24;
					code->function_code[curfunc][cv + 2] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 3] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 4] = val1 & 0xFF;

					cv += 5;

					code->bt_trl[curfunc][cv1] = 205;

					cv1++;
				}
				else
				if (c == 1 && f == 1)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = 1;
					code->function_code[curfunc][cv + 2] = 0;
					CopyFloatToInt32(code->function_code[curfunc] + cv + 3, valf1);

					code->function_code[curfunc][cv + 7] = 208;
					code->function_code[curfunc][cv + 8] = 0;

					cv += 9;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = 1;
					code->bt_trl[curfunc][cv1 + 2] = 208;

					cv1 += 3;
				}
				else
				if (c == 0 && f == 0)
				{
					code->function_code[curfunc][cv] = 206;
					code->function_code[curfunc][cv + 1] = g == 1 ? 0 : 1;
					code->function_code[curfunc][cv + 2] = val1 >> 24;
					code->function_code[curfunc][cv + 3] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 4] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 5] = val1 & 0xFF;

					cv += 6;

					code->bt_trl[curfunc][cv1] = 206;

					cv1++;
				}
				else
				if (c == 0 && f == 1)
				{
					code->function_code[curfunc][cv] = 255;
					code->function_code[curfunc][cv + 1] = 4;
					code->function_code[curfunc][cv + 2] = 0;
					code->function_code[curfunc][cv + 3] = g == 1 ? 0 : 1;
					code->function_code[curfunc][cv + 4] = val1 >> 24;
					code->function_code[curfunc][cv + 5] = (val1 >> 16) & 0xFF;
					code->function_code[curfunc][cv + 6] = (val1 >> 8) & 0xFF;
					code->function_code[curfunc][cv + 7] = val1 & 0xFF;

					code->function_code[curfunc][cv + 8] = 208;
					code->function_code[curfunc][cv + 9] = 0;

					cv += 10;

					code->bt_trl[curfunc][cv1] = 255;
					code->bt_trl[curfunc][cv1 + 1] = 4;
					code->bt_trl[curfunc][cv1 + 2] = 208;

					cv1 += 3;
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

				i = code->function_table[i].address;

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

			if (brackets < 0)
				brackets--;
			else
			{
				error++;
				LogApp("Compiler error: found } before { - line: %d", line);
				continue;
			}
		}
	}
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

		c[4] = 30 >> 8;
		c[5] = 30 & 0xFF;

		c[14] = 0;

		//Define two global variables
		c[18] = 254;
		c[19] = 1;
		c[20] = 16384 >> 24;
		c[21] = (16384 >> 16) & 0xFF;
		c[22] = (16384 >> 8) & 0xFF;
		c[23] = 16384 & 0xFF;

		c[24] = 254;
		c[25] = 1;
		c[26] = 32768 >> 24;
		c[27] = (32768 >> 16) & 0xFF;
		c[28] = (32768 >> 8) & 0xFF;
		c[29] = 32768 & 0xFF;

		//Init entry point
		//define local var
		c[30] = 254;
		c[31] = 1;
		c[32] = 65536 >> 24;
		c[33] = (65536 >> 16) & 0xFF;
		c[34] = (65536 >> 8) & 0xFF;
		c[35] = 65536 & 0xFF;

		//set regi mem
		c[36] = 1;
		c[37] = 14;
		c[38] = 0;
		c[39] = 0 >> 24;
		c[40] = (0 >> 16) & 0xFF;
		c[41] = (0 >> 8) & 0xFF;
		c[42] = 0 & 0xFF;

		//set regi regi
		c[43] = 2;
		c[44] = 15;
		c[45] = 14;

		//set mem regi
		c[46] = 4;
		c[47] = 0;
		c[48] = 0 >> 24;
		c[49] = (0 >> 16) & 0xFF;
		c[50] = (0 >> 8) & 0xFF;
		c[51] = 0 & 0xFF;
		c[52] = 16;

		//add mem mem
		c[53] = 15;
		c[54] = 1;
		c[55] = 1 >> 24;
		c[56] = (1 >> 16) & 0xFF;
		c[57] = (1 >> 8) & 0xFF;
		c[58] = 1 & 0xFF;
		c[59] = 0;
		c[60] = 0 >> 24;
		c[61] = (0 >> 16) & 0xFF;
		c[62] = (0 >> 8) & 0xFF;
		c[63] = 0 & 0xFF;
		c[64] = 210;
	}
	else
	{
		openfile_d(f, file, "rb");

		fseek(f, 0, SEEK_END);

		st.mgl.size = ftell(f);
		rewind(f);

		st.mgl.code = malloc(st.mgl.size);
		CHECKMEM(st.mgl.code);

		fclose(f);
	}

	if (st.mgl.code[0] != 'M' || st.mgl.code[1] != 'G' || st.mgl.code[2] != 'L' && st.mgl.code[3] != ' ')
	{
		free(st.mgl.code);
		return NULL;
	}

	if (st.mgl.code[14] == 0)
	{
		st.mgl.memsize = 131072;
		st.mgl.stack_type = 0;
	}
	else
	{
		st.mgl.memsize = (st.mgl.code[15] << 24) | (st.mgl.code[16] << 16) | (st.mgl.code[17] << 8) | st.mgl.code[18];
		st.mgl.stack_type = 1;
	}

	ZeroMem(st.mgl.v, sizeof(8 * sizeof(uint32)));
	st.mgl.bp = st.mgl.sp = 0;

	///Load engine calls

	st.mgl.funcs.log = LogApp;
	st.mgl.funcs.drawline = LineCall;
	st.mgl.funcs.msgbox = MessageBoxRes;

	return 1;
}

int8 ExecuteMGLCode(uint8 location)
{
	register int32 bp, sp, cv;
	int32 v[8];

	static int32 stack1[131072], *stack2, *stack;
	uint8 *vars, state = 0;
	void **heap;

	float f[5];

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
		st.mgl.cv = (st.mgl.code[4] << 8) | st.mgl.code[5]; //Get the entry point Init
		break;

	case 1:
		st.mgl.cv = (st.mgl.code[6] << 8) | st.mgl.code[7]; //Get the entry point PreGame
		break;

	case 2:
		st.mgl.cv = (st.mgl.code[8] << 8) | st.mgl.code[9]; //Get the entry point GameLoop
		break;

	case 3:
		st.mgl.cv = (st.mgl.code[10] << 8) | st.mgl.code[11]; //Get the entry point MainLoop
		break;

	case 4:
		st.mgl.cv = (st.mgl.code[12] << 8) | st.mgl.code[13]; //Get the entry point End
		break;
	}

	cv = st.mgl.cv;
	funcaddr = cv;
	bp = st.mgl.bp;
	sp = st.mgl.sp;

	if (location == 0)
	{
		cv = 18;
		//fetch global variables to the stack
		while (buf[cv] == 254)
		{
			if (buf[cv + 1] < 5)
			{
				CodeToStack(bp, cv + 2);
				bp++;
				cv += 6;
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
		sp = bp + 1;
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
			case 3:
			case 4:
				CodeToStack(sp, cv + 2);
				sp++;
				cv += 6;
				break;
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
					CodeToStack(v[2], cv + 5);
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

				case 210:
					sp = bp;
					PopStack(cv);
					bp = sp - 1;
					bp = stack[bp];
					sp = bp + 1;
					break;

				case 249:
					v[buf[cv + 1]] = f[buf[cv + 2]];
					break;

				case 250:
					f[buf[cv + 1]] = v[buf[cv + 2]];
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