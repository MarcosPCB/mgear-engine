#pragma once

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_thread.h>
#include <SDL_mutex.h>
#include <fmod.h>
#include <fmod_errors.h>
#include <Windows.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <SOIL.h>
#include <time.h>

#define MAX_SPRITES 512
#define MAX_HUDSPRITES 128
#define MAX_GRAPHICS 4096
#define TICSPERSECOND 100
#define MAX_CHANNELS 32
#define MUSIC_CHANNEL MAX_CHANNELS-1
#define MAX_SOUNDS 32
#define MUSIC_SLOT MAX_SOUNDS-1
#define MAX_KEYS 128
#define MAX_OBJS 1024

#define QLZ_COMPRESSION_LEVEL 3
#define QLZ_STREAMING_BUFFER 0
#define QLZ_MEMORY_SAFE 0

typedef FMOD_SOUND Sound;
typedef FMOD_CHANNEL Channel;

//MGG format
#define MAX_FRAMES 65536
#define MAX_ANIMATIONS 64
#define MAX_MGG 64

#define MGG_MAP_START 3 //The first slot to be used for loading map MGGs

typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int int16;
typedef unsigned short int uint16;
typedef signed long int int32;
typedef unsigned long int uint32;
typedef long long unsigned uint64;

double inline __declspec () __fastcall sqrt14(double n);

#define LogApp SDL_Log
#define LogErr SDL_LogError
#define LogWn SDL_LogWarn

//#define MAX_MGVFRAMES 65536

//Types of font
enum Font
{
	COOPER,
	ARIAL
};

enum Enttype
{
	SPRITE,
	HUD,
	TEXTURE,
	VIDEO,
	TEXT,
	UI
};

enum Stat
{
	USED,
	DEAD
};

typedef struct
{
	double x;
	double y;
} Pos;

typedef struct
{
	float r;
	float g;
	float b;
	float a;
} Color;

typedef struct
{
	uint8 r;
	uint8 g;
	uint8 b;
	float a;
} Colori;

struct _ENTITIES //To be rendered
{
	Enttype type;
	Pos pos;
	Pos x1y1;
	Pos x2y2;
	Stat stat;
	GLuint data;
	Pos size;
	Color color;
	float ang;
};

struct Key
{
	SDL_Scancode key;
	uint8 state;
};

//FMOD sound system
struct _SOUNDSYS
{
	FMOD_SYSTEM *Sound_System;
	Sound *slots[MAX_SOUNDS];
	int16 slot_ID[MAX_SOUNDS];
	Channel *channels[MAX_CHANNELS];
	int16 slotch_ID[MAX_CHANNELS];
};

enum _MGGTYPE
{
	SPRITEM,
	TEXTUREM,
	NONE
};

struct _MGGANIM
{
	char name[32];
	uint16 num_frames;
	float current_frame;
	uint32 startID;
	uint32 endID;
};

//File header
struct _MGGFORMAT
{
	char name[32];
	uint16 num_frames;
	_MGGTYPE type;
	uint32 num_animations;
};

//MGG -> MGear Graphics
struct _MGG
{
	char name[32];
	uint16 num_frames;
	_MGGTYPE type;
	GLuint *frames;
	Pos *size;
	Pos *sizefix;
	uint32 num_anims;
	_MGGANIM *anim;
};

//File header for the video
struct _MGVFORMAT
{
	uint32 num_frames;
	uint8 fps;
	uint32 sound_buffer_lenght;
};

struct _MGVTEX
{
	GLuint ID;
	SDL_Surface *data;
	void *buffer;
	SDL_RWops *rw;
};

//MGV -> MGear Video
struct _MGV
{
	uint32 num_frames;
	_MGVTEX *frames;
	size_t totalsize;
	uint8 fps;
	FMOD_SOUND *sound;
	FILE *file;
	uint32 *framesize;
	uint32 *seeker;
};

//This is where all map textures information stays
//Each map has it's on set of MGG that can be shared between the other maps
//Some MGGs that contains the menu and some UI textures are basic and contain it's own structure
struct _TEXTURES
{
	uint32 ID;
	uint8 MGG_ID : 6;
	char name[32];
};

enum _SPRITE_G
{
	GAME_LOGICAL,
	ENEMY,
	FRIEND,
	NORMAL,
	dead
};

//Structure for te sprites in the game
//When you create a sprite in the source code
//You must add it to the structure
struct _SPRITES
{
	char name[64];
	uint32 ID;
	uint8 MGG_ID : 6;
	int16 health;
	_SPRITE_G type;
};

enum _OBJTYPE
{
	FLOOR,
	CEILING,
	WALL_BACK,
	WALL_FRONT,
	WALL_SIDE,
	BACKGROUND1,
	BACKGROUND2,
	BACKGROUND3,
	BLANK
};

enum _OBJBLOCK
{
	ALL,
	PLAYER,
	BULLET,
	ENEMYB,
	none
};

enum _SPRITE_T
{
	ANIMATED,
	TEXTURED,
	ENTITY,
	non
};

struct _MGMOBJ
{
	Pos position;
	Pos size;
	Pos texsize;
	Pos texpan;
	float angle;
	Colori color;
	uint32 TextureID;
	int16 tag;
	uint8 priority : 2;//0 - Most important, 1 - Medium, 2 - Less important
	_OBJTYPE type;
	_OBJBLOCK block_type;
};

//Map sprites
struct _MGMSPRITE
{
	//if it's just a texture, you MUST inform the frame ID
	uint8 MGG_ID;

	//Leave it blank if it's not animated
	uint16 animation;

	//Leave it blank if it's not a texture
	uint32 frame_ID;
	_SPRITE_T type;

	//If it's an entity, you MUST insert here the game sprite name that has a code
	//or else, just leave it blank
	char game_name[64]; 

	int16 tag;

	int16 health;

	Pos position;
	Pos size;

	float angle;

	Colori color;
};


//MGM -> MGear Map
struct _MGM
{
	char name[32];
	_MGMSPRITE *sprites;
	_MGMOBJ *obj;
	uint16 num_sprites;
	uint16 num_obj;
	uint8 num_mgg;
	char MGG_FILES[32][256];
};

struct _MGMFORMAT
{
	char name[32];
	uint16 num_sprites;
	uint16 num_obj;
	uint8 num_mgg;
	char MGG_FILES[32][256];
};

struct _CAMERA
{
	Pos position;
	float angle;
	Pos dimension;
};

enum GAME_STATE
{
	MAIN_MENU,
	INGAME,
	GAME_MENU,
	STARTUP,
	LOADING
};

//The main structure
//Contais all the information about the game
struct _SETTINGS
{
	uint32 backtrack;
	char typetext[128];
	char WINDOW_NAME[64];
	uint16 screenx;
	uint16 screeny;
	uint8 bpp;
	uint8 fullscreen;
	uint32 audiof;
	uint8 audioc;
	uint32 num_entities;
	long long unsigned int time;
	_SOUNDSYS sound_sys;
	TTF_Font *font[3];
	Pos mouse;
	uint8 mouse1;
	uint8 mouse2;
	uint8 mouse_on : 2;
	Key keys[MAX_KEYS];
	uint8 quit;
	uint8 PlayingVideo;
	_MGM Current_Map;
	_SPRITES Game_Sprites[MAX_SPRITES];
	_TEXTURES UiTex[MAX_GRAPHICS];
	_TEXTURES MapTex[MAX_GRAPHICS];
	_CAMERA Camera;
	GAME_STATE gt;

	SDL_GLContext glc;

#ifdef ENGINEER
	uint8 Engineer_Mode;
#endif

	uint8 FPSYes : 1;
	uint32 FPSTime;
	float FPS;
	char FPSStr[6];
};

extern _SETTINGS st;
extern _ENTITIES ent[MAX_GRAPHICS];
extern SDL_Event events;

extern _MGG mgg[MAX_MGG];

extern const char WindowTitle[32];

void Init();
void Quit();

//For tests only
void createmgg();
void createmgv();

uint32 LoadMGG(_MGG *mgg, const char name[32]); //Loads a MGG file into a MGG type struct

void FreeMGG(_MGG *file); //Free a MGG struct
void InitMGG(); //Inits all MGG structs

#ifdef ENGINEER
	uint32 SaveMap(const char *name);
#endif

uint32 LoadMap(const char *name);
void FreeMap();

void DrawMap();

void Timer();

void FPSCounter();

void CreateLog();
void LogIn(void *userdata, int category, SDL_LogPriority, const char *message);

void RestartVideo();

uint32 POT(uint32 value);

uint32 PlayMovie(const char *name);
void DrawGraphic(double x, double y, double sizex, double sizey, float ang, uint8 r, uint8 g, uint8 b, GLuint data, float a);
void DrawSprite(double x, double y, double sizex, double sizey, float ang, uint8 r, uint8 g, uint8 b, GLuint data, float a);
void DrawHud(double x, double y, double sizex, double sizey, float ang, uint8 r, uint8 g, uint8 b, double x1, double y1, double x2, double y2, GLuint data, float a);
void DrawString(Font type, const char *text, double x, double y, double sizex, double sizey, float ang, uint8 r, uint8 g, uint8 b, float a);

int32 MAnim(double x, double y, double sizex, double sizey, float ang, uint8 r, uint8 g, uint8 b, _MGG *mgf, uint16 id, float speed, float a);

void Renderer();

void PlaySound(const char *filename, uint8 loop);
void PlayMusic(const char *filename, uint8 loop);
void MainSound();
void StopAllSounds();
void StopMusic();