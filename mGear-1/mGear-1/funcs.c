#include "dirent.h"
#include <stdarg.h>
#include <assert.h>
#include "types.h"
#include <Windows.h>
#include "funcs.h"

#ifdef ENG_DIAGNOSTICS

#undef malloc
#undef calloc
#undef free
#undef memcpy
#undef realloc

struct MEM_INFO *mem_inf;

uint32 num_mem = 0, max_mem = 0;

uint8 add_mem_info(const char *func, uint16 line, size_t size, void *addr)
{
	if (max_mem == 0)
	{
		max_mem += 64;
		mem_inf = calloc(max_mem, sizeof(struct MEM_INFO));

		if (mem_inf == NULL)
		{
			LogApp("FATAL ERROR: No more memory available for diagnostics");
			return NULL;
		}
	}

	if (max_mem == num_mem)
	{
		max_mem += 64;
		mem_inf = realloc(mem_inf, max_mem * sizeof(struct MEM_INFO));

		if (mem_inf == NULL)
		{
			LogApp("FATAL ERROR: No more memory available for diagnostics");
			return NULL;
		}
	}

	strcpy(mem_inf[num_mem].func, func);
	mem_inf[num_mem].line = line;
	mem_inf[num_mem].size = size;
	mem_inf[num_mem].addr = addr;
	mem_inf[num_mem].free = 0;

	num_mem++;

	return 1;
}

void *xmalloc(const char *func, int line, size_t size)
{
	void *buf = malloc(size);

	mem_assert(buf);

	mem_assert(func);

	if (add_mem_info(func, line, size, buf) == NULL)
	{
		free(buf);
		return NULL;
	}

	return buf;
}

void *xrealloc(const char *func, int line, void *memory, size_t size)
{
	//mem_assert(memory);

	memory = realloc(memory, size);

	mem_assert(memory);

	if (add_mem_info(func, line, size, memory) == NULL)
	{
		free(memory);
		return NULL;
	}

	mem_inf[num_mem - 1].free = 2; //reallocated

	return memory;
}

void *xcalloc(const char *func, int line, size_t count, size_t size)
{
	void *buf = calloc(count, size);

	mem_assert(buf);

	if (add_mem_info(func, line, size * count, buf) == NULL)
	{
		free(buf);
		return NULL;
	}

	return buf;
}

void xfree(const char *func, int line, void *memory)
{
	if (memory == NULL)
		LogApp("DIAGNOSTICS: tried to free NULL pointer in %s - %d", func, line);
	else
	{
		mem_assert(memory);

		void *addr = memory;

		free(memory);

		for (int i = 0; i < num_mem; i++)
		{
			if (mem_inf[i].addr == addr)
				mem_inf[i].free = 1;
		}

		if (num_mem > 1024)
		{
			struct MEM_INFO *membck = calloc(max_mem, sizeof(struct MEM_INFO));

			int i = 0, k = 0;

			for (; i < num_mem; i++)
			{
				if (mem_inf[i].free == 0)
				{
					memcpy(&membck[k], &mem_inf[i], sizeof(struct MEM_INFO));
					k++;
				}
			}

			memset(mem_inf, 0, num_mem * sizeof(struct MEM_INFO));
			memcpy(mem_inf, membck, num_mem * sizeof(struct MEM_INFO));

			num_mem = k;

			free(membck);
		}
	}
}

void *xmemcpy(const char *func, int line, void *dest, const void* src, size_t size)
{
	mem_assert(dest);
	mem_assert(src);

	//LogApp("DIAGNOSTICS: memcpy in %s: %d - size: - Destination: 0x%0X - Src: 0x%0X");

	return memcpy(dest, src, size);
}

void print_diag()
{
	size_t mem = 0, mem_leak = 0;
	LogApp("DIAGNOSTICS FROM RUNTIME - MEMORY LEAKS");

	for (int i = 0; i < num_mem; i++)
	{
		if (mem_inf[i].free == 0)
		{
			LogApp("%d - Function: %s - %d	Size: %.2fMBs or %uBs		Address: 0x%0X",i, mem_inf[i].func, mem_inf[i].line, (mem_inf[i].size / 1024.0f) / 1024.0f,
				mem_inf[i].size, mem_inf[i].addr);

			mem_leak += mem_inf[i].size;
		}

		if (mem_inf[i].free == 2)
		{
			LogApp("%d - Function: %s - %d	Size: %.2fMBs or %uBs		Address: 0x%0X - REALLOCATED",i ,mem_inf[i].func, mem_inf[i].line, (mem_inf[i].size / 1024.0f) / 1024.0f,
				mem_inf[i].size, mem_inf[i].addr);

			mem_leak += mem_inf[i].size;
		}

		//mem += mem_inf[i].size;
	}

	//LogApp("TOTAL MEMORY CONSUPTION: %.2fMBs or %uBs", (mem / 1024.0f) / 1024.0f, mem);
	LogApp("TOTAL MEMORY LEAKED: %.2fMBs or %uBs", (mem_leak / 1024.0f) / 1024.0f, mem_leak);

	free(mem_inf);
}

#endif

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

int8 IsNumber(const char *str)
{
	register uint8 i;
	register uint16 len;

	mem_assert(str);

	if (str == NULL)
		return 0;

	len = strlen(str);

	if (len > 10)
	{
		LogApp("Too big for a number!");
		return 0;
	}

	for (i = 0; i < len; i++)
	{
		if ((str[i] < 48 || str[i] > 57) && (str[i] != 43 && str[i] != 45 && str[i] != '.'))
			return 0;
	}

	return 1;
}

int8 IsNumberFloat(const char *str)
{
	register uint8 i;
	register uint16 len;

	mem_assert(str);

	if (str == NULL)
		return 0;

	len = strlen(str);

	if (len > 10)
	{
		LogApp("Too big for a number!");
		return 0;
	}

	for (i = 0; i < len; i++)
	{
		if (str[i] == '.')
			return 1;
	}

	return 0;
}