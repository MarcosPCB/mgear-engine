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

//Basic C commands for use with _CMD functions
#define RETURN_CMD(x) { return x; }
#define CONTINUE_CMD { continue; }
#define BREAK_CMD { break; }
#define ABORT_CMD { abort(); }

//Defined expressions
#define CHECKMEM(expr) { if(mchalloc(expr) == CHERROR) return ERROR_RETURN; }
#define check_mem CHECKMEM

#define CHECKMEM_CMD(expr, cmd) { if(mchalloc(expr) == CHERROR) cmd }
#define check_mem_cmd CHECKMEM_CMD

#define ALLOCMEM(data, size) { data = malloc(size); CHECKMEM(data); }
#define alloc_mem ALLOCMEM

#define ALLOCMEM_CMD(data, size, cmd) { data = malloc(size); cmd }
#define alloc_mem_cmd ALLOCMEM_CMD

#define REALLOCMEM(data, size) { data = realloc(data, size); CHECKMEM(data); }
#define realloc_mem REALLOCMEM

#define FREEMEM(data) { if(data != NULL) free(data); }
#define free_mem FREEMEM

#define OPENFILE_D(file, name, mode) { if((file=fopen(name,mode)) == NULL) { MessageBoxRes("Error", MB_OK, "Could not open file: %s", name); return NULL; } }
#define openfile_d OPENFILE_D

#define OPENFILE_STR(file, name, mode, str) { if((file=fopen(name,mode)) == NULL) { MessageBoxRes("Error", MB_OK, str, name); return NULL; } }
#define openfile_str OPENFILE_STR

#define OPENFILE_CMD(file, name, mode, cmd) { if((file=fopen(name,mode)) == NULL) { cmd } }
#define openfile_cmd OPENFILE_CMD

#define GETFILESIZE(file, size) { fseek(file, 0, SEEK_END); size = ftell(file); rewind(file); }
#define getfilesize GETFILESIZE

#define mem_assert(expr) assert(expr && "memory not valid");

cdecl int16 NumDirFile(const char *path, char content[][32]);

cdecl int32 MessageBoxRes(const char *caption, UINT type, const char *string, ...);

cdecl char *StringFormat(char *string, ...);

cdecl uint32 POT(uint32 value);

cdecl int8 mchalloc(void *data); //Checks if the pointer is valid, if not, gives an error message and returns (CHERROR) -1

cdecl int8 mchalloc_ab(void *data); //Checks if the pointer is valid, if not, gives an error message and exits the program

#endif