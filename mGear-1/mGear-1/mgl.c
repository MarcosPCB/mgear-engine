#include "mgl.h" 
#include "mgl_defs.h"
#include <stdio.h>

MGL_SYS mgl_sys;
struct MGL_FUNCTIONS mglFunctions[MAX_FUNCTIONS*2];

void MGL_FuncDef()
{
	strcpy(mglFunctions[fSTRINGBUTTON].func_name, "StringButton");
	mglFunctions[fSTRINGBUTTON].num_param = 8;

	strcpy(mglFunctions[fMESSAGEBOX].func_name, "MessageBox");
	mglFunctions[fMESSAGEBOX].num_param = 10;
	
}

int32 *MGL_Compiler(const char *filename)
{
	FILE *file;
	char buffer[1024], buff[1024], buf[1024], *compiled, *tok, functions[MAX_FUNCTIONS][32], variables[MAX_INTS][32], floats[MAX_FLOATS][32];
	int value, val, state = 0, line = -1, byte = 15, func = 0, var = 0, float_var = 0, check = 0;

	if ((file = fopen(filename, "r")) == NULL)
	{
		LogApp("Unable to open mghul file: %s", filename);
		return NULL;
	}

	compiled = malloc(MEM_TINY);

	memset(compiled, 0, MEM_TINY);

	compiled[1] = 1;
	compiled[2] = 'm';
	compiled[3] = 'g';
	compiled[4] = 'm';
	compiled[5] = 'u';
	compiled[6] = 'l';

	while (!feof)
	{
		line++;
		check = 0;

		memset(buffer, 0, 1024);
		memset(buff, 0, 1024);
		memset(buf, 0, 1024);

		fgets(buffer, 1024, file);

		if (buffer[0] == '/' && buffer[1] == '/')
			continue;

		tok = strtok(buffer, " ");

		if (state == HEADER)
		{
			if (strcmp(tok, "eventtype") == NULL)
			{
				tok = strtok(NULL, " ");

				if (strcmp(tok, "MAINMENU") == NULL)
				{
					compiled[0] = MAINMENU_EVENT;
				}
				
				if (strcmp(tok, "INGAMEMENU") == NULL)
				{
					compiled[0] = INGAMEMENU_EVENT;
				}

				if (strcmp(tok, "INGAME") == NULL)
				{
					compiled[0] = INGAME_EVENT;
				}

				continue;
			}

			if (strcmp(tok, "overrideevent") == NULL)
			{
				if (compiled[0] == 0)
				{
					LogApp("Error L-%n : no event declared before OVERRIDEEVENT command", line);
					free(compiled);
					return NULL;
				}
				
				if (compiled[0] != MAINMENU_EVENT && compiled[0] != INGAMEMENU_EVENT && compiled[0] != INGAME_EVENT)
				{
					LogApp("Error L-%n : event declared does not exist", line);
					free(compiled);
					return NULL;
				}
				
				compiled[0] += 8;
			}

			if (strcmp(tok, "endh") == NULL)
			{
				state = 0;
				continue;
			}

		}

		if (state == FUNCTION)
		{
			tok = strtok(NULL, " ");

			if (strcmp(tok, "ui") == NULL)
			{
				compiled[byte] = FAST_UI_COND;
				byte++;

				tok = strtok(NULL, " ");

				if (strcmp(tok, "multiple") == NULL)
				{

				}
				else
				{
					if (strcmp(tok, "selected") == NULL)
					{
						compiled[byte] = FUC_SELECTED;
						byte++;

						tok = strtok(NULL, " ");

						if (strcmp(tok, "multiple") == NULL)
						{

						}
					}
				}
			}
		}

		if (strcmp(tok, "header:") == NULL)
		{
			state = HEADER;
			continue;
		}

		if (strcmp(tok, "function") == NULL)
		{
			compiled[byte] = DECL_FUNC;

			byte++;

			tok = strtok(NULL, " ");

			if (strcmp(tok, "noreturn") == NULL)
			{
				compiled[byte] = VOID_FUNC;
				byte++;
				check = 1;
			}

			if (strcmp(tok, "int") == NULL)
			{
				compiled[byte] = INT_FUNC;
				byte++;
				check = 1;
			}

			if (strcmp(tok, "float") == NULL)
			{
				compiled[byte] = FLOAT_FUNC;
				byte++;
				check = 1;
			}

			if (strcmp(tok, "string") == NULL)
			{
				compiled[byte] = STRING_FUNC;
				byte++;
				check = 1;
			}

			if (check == 0)
				LogApp("warning L-%n : function type not declared - using void type instead", line);

			compiled[byte] = func;
			byte++;

			func++;

			tok = strok(NULL, " ");

			strcpy(functions[func], tok);

			state = FUNCTION;

			continue;
		}
	}
}