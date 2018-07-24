#include "types.h"
#include <Windows.h>
#include <stdarg.h>
#include <assert.h>

#ifndef _FUNCS_H
#define _FUNCS_H

#define CHERROR -1
#define mem_assert(expr) assert(expr && "memory not valid");

cdecl int16 NumDirFile(const char *path, char content[][32]);

cdecl int32 MessageBoxRes(const char *caption, UINT type, const char *string, ...);

cdecl char *StringFormat(char *string, ...);

cdecl uint32 POT(uint32 value);

cdecl int8 mchalloc(void *data); //Checks if the pointer is valid, if not, gives an error message and returns (CHERROR) -1

cdecl int8 mchalloc_ab(void *data); //Checks if the pointer is valid, if not, gives an error message and exits the program

#endif