#ifndef _MGL_H
#define _MGL_H

#include "engine.h"

//MGHUL = Master Gear Human Understandable Language
//MGMUL = Master Gear Machine Understandable Language

//Float HEX Ex: 0xAB000005
//last 8-bits are the exponent values the others 24-bits are the significand

//Stack address HEX Ex: 0x0D00000A
//first 24-bits are the address
//last 8-bits are the variable type (float, int, constant, array)

//Array HEX Ex: 0x0D02500F
//first 12-bits: start address of the array
//next 12-bits: final address of the array
//las 8-bits: variable type
//this first address is just a reference, so if you have an array of 64 entries, it actually uses 65 entries of the stack

//Stack buffer models

#define MEM_TINY 2048
#define MEM_SMALL 4096
#define MEM_MEDIUM 8192
#define MEM_BIG 16384
#define MEM_HUGE 32768

#define MSP_ADDRESS 0x0000 //Main stack ponter: first 64 bytes are reserved for the main function

//function stack pointer: contains the function return address and subsequent bytes are for the parameters 
//64 bytes reserved
//16 bytes backwards are reserved for the first 3 local variables (32-bit or not)
#define FSP_ADDRESS 0x004C 

#define GV_ADDRESS 0x008C //global variables start address: 128 bytes reserved
#define LV_ADDRESS 0x010C //local variables start address

//In the compiled MGMUL file the first 3 bytes are NULL and the next 5 bytes contain the string MGMUL
//the next 6 bytes represents the version of the coded MGMUL, the number of functions in the code and the number of variables
//jump 4 bytes and then the next 4 bytes represents the total size of the MGMUL code
//jump 4 bytes again and load the code using a signed 32-bit buffer

struct _MGL_SYS
{
	int8 *STACK; //Stack memory

	int8 *bin_file; //compiled MGMUL file
};

typedef struct _MGL_SYS MGL_SYS;

int32 MGL_StackAddress(int8 type, int32 value); //returns an address for the variable or constant

int32 *MGL_Compiler(const char *filename); //compiles the MGHUL file into MGMUL language

void MGL_SaveCompiled(int32 *buffer); //save the MGMUL data

void MGL_LoadCompiled(const char *filename); //load a MGMUL data file

int16 MGL_Interpreter(int16 section, void *data); //read and process the MGMUL data

#endif