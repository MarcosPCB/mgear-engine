#include "types.h"
#include <Windows.h>
#include <stdarg.h>
#include <assert.h>

#ifndef LogApp
	#include <SDL.h>
	#define LogApp SDL_Log
#endif

#ifndef _FUNCS_H
#define _FUNCS_H

extern void Quit();

#define CHERROR -1
#define ERROR_RETURN -10

#define CHECKMEM(expr) { if(mchalloc(expr) == CHERROR) return ERROR_RETURN; }
#define ALLOCMEM(data, size) { data = malloc(size); CHECKMEM(data); }
#define alloc_mem ALLOCMEM

#define OPENFILE_D(file, name, mode) { if((file=fopen(name,mode)) == NULL) { MessageBoxRes("Error", MB_OK, "Could not open file: %s", name); return NULL; } }
#define OPENFILE_STR(file, name, mode, str) { if((file=fopen(name,mode)) == NULL) { MessageBoxRes("Error", MB_OK, str, name); return NULL; } }

#define mem_assert(expr) assert(expr && "memory not valid");

cdecl int16 NumDirFile(const char *path, char content[][32]);

cdecl int32 MessageBoxRes(const char *caption, UINT type, const char *string, ...);

cdecl char *StringFormat(char *string, ...);

cdecl uint32 POT(uint32 value);

cdecl int8 mchalloc(void *data); //Checks if the pointer is valid, if not, gives an error message and returns (CHERROR) -1

cdecl int8 mchalloc_ab(void *data); //Checks if the pointer is valid, if not, gives an error message and exits the program

#endif