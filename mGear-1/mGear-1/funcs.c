#include "dirent.h"
#include <stdarg.h>
#include <assert.h>
#include "types.h"
#include <Windows.h>
#include "funcs.h"

int16 NumDirFile(const char *path, char content[][32])
{
	DIR *dir;
	dirent *ent;
	uint16 i = 0;
	int16 filenum = 0;

	if ((dir = opendir(path)) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			strcpy(content[i], ent->d_name);
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

int32 MessageBoxRes(const char *caption, UINT type, const char *string, ...)
{
	char str[512];
	va_list args;

	va_start(args, string);

	vsprintf(str, string, args);

	va_end(args);

	LogApp(str);

	return MessageBox(NULL, str, caption, type);
}

char *StringFormat(char *string, ...)
{
	static char buf[1024];

	va_list args;

	strcpy(buf, string);

	va_start(args, string);

	vsprintf(buf, string, args);

	va_end(args);

	return buf;
}

uint32 POT(uint32 value)
{
	if (value != 0)
	{
		value--;
		value |= (value >> 1); //Or first 2 bits
		value |= (value >> 2); //Or next 2 bits
		value |= (value >> 4); //Or next 4 bits
		value |= (value >> 8); //Or next 8 bits
		value |= (value >> 16); //Or next 16 bits
		value++;
	}

	return value;
}

int8 mchalloc(void *data)
{
	if (data == NULL)
	{
		LogApp("Not enough memory to alloc, function terminated");
		MessageBoxRes("Memory error", MB_OK, "Not enough memory to alloc, function terminated");
		
		return CHERROR;
	}
	else
		return NULL;
}

int8 mchalloc_ab(void *data)
{
	if (data == NULL)
	{
		LogApp("Not enough memory to alloc, quitting program");
		MessageBoxRes("Memory error", MB_OK, "Not enough memory to alloc, quitting program");
		
		Quit();
	}
	else
		return NULL;
}

char *GetFileNameOnly(const char *path)
{
	int16 i = 0, j = 0, len;
	char filename[MAX_PATH];

	mem_assert(path);

	if (!path)
		return NULL;

	len = strlen(path);

	for (i = len; i > 0; i--)
	{
		if (path[i] == '/' || path[i] == '\\')
			break;
	}

	strcpy(filename, path + i + 1);

	return filename;
}
