#ifndef _DOS_BUILD
	#include <GLee.h>
	#include <SDL.h>
	#include <SDL_opengl.h>
	#include <SDL_thread.h>
	#include <SDL_mutex.h>
	#include <fmod.h>
	#include <fmod_errors.h>
	#include <SDL_ttf.h>
	#include <SOIL.h>
#else
	#include <dos.h>
	#include <conio.h>
#endif

#ifdef WIN32
	#include <Windows.h>
#endif


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef _MGTYPES_H
	#include "types.h"
#endif

#if !defined (_VAO_RENDER) && !defined (_VBO_RENDER) && !defined (_VA_RENDER) && !defined (_IM_RENDER) && !defined (_DOS_BUILD)
	#error Rendering type not defined
	#error Use _VAO_RENDER or _VBO_RENDER or _VA_RENDER or _IM_RENDER or all together
#endif


#ifndef _ENGINE_H
#define _ENGINE_H

#define MAX_SPRITES 512
#define MAX_GRAPHICS 6656
#define TICSPERSECOND 100
#define MAX_CHANNELS 32
#define MUSIC_CHANNEL MAX_CHANNELS-1
#define MAX_SOUNDS 32
#define MUSIC_SLOT MAX_SOUNDS-1
#define MAX_KEYS 128
#define MAX_OBJS 1024
#define MAX_FONTS 8
#define MAX_SECTORS 512
#define MAX_LIGHTS 16
#define MAX_LIGHTMAPS 128
#define MAX_MAPMGG 32

#define MAX_VERTEX MAX_GRAPHICS*8
#define MAX_COLORS MAX_GRAPHICS*12
#define MAX_INDEX MAX_GRAPHICS*6

#define QLZ_COMPRESSION_LEVEL 3
#define QLZ_STREAMING_BUFFER 0
#define QLZ_MEMORY_SAFE 0

typedef FMOD_SOUND Sound;
typedef FMOD_CHANNEL Channel;

//MGG format
#define MAX_FRAMES 8192
#define MAX_ANIMATIONS 64
#define MAX_MGG 256

#define pi 3.14159265

#define MGG_MAP_START 3 //The first slot to be used for loading map MGGs
#define MGG_SPRITE_START MGG_MAP_START+MAX_MAPMGG

double inline __declspec () __fastcall sqrt14(double n);

#define LogApp SDL_Log
#define LogErr SDL_LogError
#define LogWn SDL_LogWarn
#define CloseFont TTF_CloseFont
#define GetTicks SDL_GetTicks

//#define MAX_MGVFRAMES 65536

enum Enttype
{
	SPRITE,
	HUD,
	TEXTURE,
	VIDEO,
	TEXT,
	TEXT_UI,
	UI,
	LIGHT_MAP,
	LINE,
	ent_none
};

enum Stat
{
	DEAD,
	USED
};

struct VB_DATAT
{
#ifdef _VAO_RENDER 
	GLuint vao_id;
#endif

	GLuint ibo_id;
	GLuint vbo_id;
	float *vertex;
	float *texcoord;
	GLubyte *color;
	GLushort *index;
	GLuint texture;
	GLuint Ntexture; //normal map
	uint8 normal; //boolean for verification
	uint16 num_elements;
	int w, h;
};

struct _TEX_DATA
{
	GLuint data;
	GLuint Ndata; //normal map
	uint8 normal; //boolean for verification
	int16 vb_id;
	uint16 posx, posy; //position in atlas
	uint16 sizex, sizey; //size in atlas
	int w, h, channel; //texture dimensions
};

typedef struct _TEX_DATA TEX_DATA;

struct _ENTITIES //To be rendered
{
	Enttype type;
	Pos pos;
	Pos x1y1;
	Pos x2y2;
	Stat stat;
	TEX_DATA data;
	Pos size;
	int16 ang;
	float vertex[12];
	float texcor[8];
	GLubyte color[16];
	Color Color;
};

enum LIGHT_TYPE
{
	AMBIENT_LIGHT,
	POINT_LIGHT_MEDIUM,
	POINT_LIGHT_STRONG,
	SPOTLIGHT
};

enum LIGHTENG_TYPE
{
	STATIC_LIGHT,
	DYNAMIC_LIGHT
};

struct _LIGHTS
{
	LIGHT_TYPE type;
	PosF pos;
	ColorF color;
	float falloff;
	float radius;
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
	uint16 current_frame;
	uint16 startID;
	uint16 endID;
	int8 speed;
};

//File header
struct _MGGFORMAT
{
	char name[32];
	uint16 num_frames;
	_MGGTYPE type;
	uint8 num_atlas;
	uint16 num_singletex;
	uint16 num_texinatlas;
	uint32 num_animations;
	size_t textures_offset;
	size_t possize_offset;
	size_t framesize_offset;
};

//MGG -> MGear Graphics
struct _MGG
{
	char name[32];
	uint16 num_frames;
	_MGGTYPE type;
	TEX_DATA frames[8192]; //single-texture and atlas objects data
	GLuint *atlas; //texture atlas data
	Pos *size;
	//Pos *sizefix;
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
	uint16 MGG_ID;
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

enum Material
{
	METAL,
	WOOD,
	PLASTIC,
	CONCRETE,
	ORGANIC,
	MATERIAL_END
};

struct _BODY
{
	uint8 physics_on;
	uint16 mass;
	Pos size;
	uint16 max_elasticy;
	Material material;
	uint8 conductor : 2;
	uint8 flamable : 2;
	uint8 explosive : 2;
	Pos position;
	int16 total_vel;
	Pos velxy;
	int16 acceleration;
	int16 energy;
	int16 temperature;
	int16 ang;
};

typedef struct _BODY Body;

enum _OBJTYPE
{
	BACKGROUND1,
	BACKGROUND2,
	BACKGROUND3,
	MIDGROUND,
	FOREGROUND,
	BLANK
};

//Structure for the sprites in the game
//When you create a sprite in the source code
//You must add it to the structure
struct _SPRITES
{
	char name[64];
	int16 MGG_ID;
	int32 frame;
	_OBJTYPE type_s;
	int32 ID;
	int32 GameID;
	int16 tag;
	int16 health;
	_SPRITE_G type;
	int16 current_sector;
	int8 current_layer;
	Body body;
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

struct _SECTOR
{
	Pos position;
	Pos vertex[4];
	uint8 layers;
	struct LAYER
	{
		Pos position; // Y is constant
		Pos size;
		int16 tag;
	} Layer[8];
	Material material;
	int16 tag;
	uint8 destructive : 2;
	int16 id;
};

struct _MGMLIGHT
{
	Pos position;
	Pos size;
	int16 angle;
	Color color;
	uint32 TextureID;
	int16 tag;
};

struct _MGMOBJ
{
	Pos position;
	Pos size;
	Pos texsize;
	Pos texpan;
	int16 angle;
	Color color;
	_TEXTURES tex;
	int16 tag;
	uint8 priority : 2;//0 - Most important, 1 - Medium, 2 - Less important
	_OBJTYPE type;
	_OBJBLOCK block_type;
	uint8 amblight;
};

//Map sprites
struct _MGMSPRITE
{
	//if it's just a texture, you MUST inform the frame ID
	uint16 MGG_ID;

	//Leave it blank if it's not animated
	uint16 animation;

	//Leave it blank if it's not a texture
	int32 frame_ID;
	_SPRITE_T type;

	//If it's an entity, you MUST insert here the game sprite name that has a code
	//or else, just leave it blank
	char game_name[64]; 

	_OBJTYPE type_s;

	uint32 GameID;

	Body body;

	int16 tag;

	int16 health;

	int16 current_sector;
	int8 current_layer;

	Pos position;
	Pos size;

	int16 angle;

	Color color;
};


//MGM -> MGear Map
struct _MGM
{
	char name[32];
	_MGMSPRITE *sprites;
	_MGMOBJ *obj;
	_MGMLIGHT *light;
	_SECTOR *sector;
	uint16 num_sprites;
	uint16 num_obj;
	uint8 num_mgg;
	uint8 num_lights;
	uint16 num_sector;
	char MGG_FILES[32][256];
};

struct _MGMFORMAT
{
	char name[32];
	uint16 num_sprites;
	uint16 num_obj;
	uint8 num_mgg;
	uint8 num_lights;
	uint16 num_sector;
	char MGG_FILES[32][256];
};

struct _CAMERA
{
	Pos position;
	int16 angle;
	PosF dimension;
};

enum GAME_STATE
{
	MAIN_MENU,
	INGAME,
	GAME_MENU,
	STARTUP,
	LOADING
};

struct Control
{
	SDL_GameController *device;

	struct Button
	{
		SDL_GameControllerButton name;
		uint8 state;;
	} button[15];

	struct Axis
	{
		SDL_GameControllerAxis name;
		int16 state;
	} axis[6];

	SDL_Haptic *force;
	SDL_HapticEffect effect;

	SDL_Joystick *joystick;
};
/*
struct Joy
{
	SDL_Joystick *device;

	struct Button
	{
		uint8
		uint8 state;
	} button[16];

	struct Axis
	{
		SDL_GameControllerAxis name;
		uint8 state;
	} axis[6];
}
*/
#ifdef _VBO_RENDER

struct _VBO_PACKET
{
	GLuint VBO, IBO;
	uint16 elements;
};

typedef _VBO_PACKET VBO_PACKET;

#endif

struct Render
{

#ifdef _VAO_RENDER
	uint8 VAO_ON;

	GLuint VAO_1Q;
	GLuint *VAO;

#endif

#ifdef _VBO_RENDER
	uint8 VBO_ON;

	VBO_PACKET VBO_1Q;
	VBO_PACKET *VBO;
#endif

#ifdef _VA_RENDER
	uint8 VA_ON;
#endif


#ifdef _IM_RENDER
	uint8 IM_ON;
#endif

	GLuint VShader[16];
	GLuint FShader[16];
	GLuint GShader[16];
	GLuint Program[16];

	GLuint FBO[4];
	GLuint FBTex[8];
	GLuint RBO[8];

	GLuint Buffers[4];
};

struct TFont
{
	char name[64];
	TTF_Font *font;
};

struct _GAME_LIGHTMAPS
{
	Pos w_pos;

	uint16 W_w;
	uint16 W_h;

	uint16 T_w;
	uint16 T_h;

	uPos16 t_pos[16];

	unsigned char *data;

	uint8 num_lights;

	GLuint tex;
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
	uint8 vsync;

	uint32 num_entities;
	uint32 num_tex;
	uint32 num_hud;
	uint32 num_ui;
	uint8 num_lights;
	uint8 num_lightmap;

	_GAME_LIGHTMAPS game_lightmaps[128];

	struct
	{
		GLuint lightmap;

	} Lightmaps;

	long long unsigned int time;

	_SOUNDSYS sound_sys;
	TFont fonts[MAX_FONTS];

	Pos mouse;
	uint8 mouse1;
	uint8 mouse2;
	uint8 mouse_on : 2;
	int32 mouse_wheel;

	Key keys[MAX_KEYS];

	uint8 quit;
	uint8 PlayingVideo;

	char TextInput[128];
	uint8 Text_Input;

	_MGM Current_Map;

	_SPRITES Game_Sprites[MAX_SPRITES];
	uint16 num_sprites;

	_CAMERA Camera;
	GAME_STATE gt;

	SDL_GLContext glc;

#ifdef ENGINEER
	uint8 Engineer_Mode;
#endif
	
	uint8 Developer_Mode;

	float test;
	float test2;

	uint8 FPSYes;
	uint32 FPSTime;
	float FPS;
	char FPSStr[6];

	uint8 LOWRES;

	uint8 control_num;
	Control controller[4];
	//SDL_Joystick *Joy[4];

	Render renderer;

	//Math content
	float CosTable[3600];
	float SinTable[3600];
	float TanTable[3600];
};

#endif

extern _SETTINGS st;
extern _ENTITIES ent[MAX_GRAPHICS];
extern SDL_Event events;
extern _LIGHTS game_lights[MAX_LIGHTS];
extern _ENTITIES lpm[MAX_LIGHTMAPS]; 

extern _MGG mgg[MAX_MGG];

extern const char WindowTitle[32];

void Init();

uint8 OpenFont(const char *file,const char *name, uint8 index);

void Quit();

//For tests only
void createmgg();
void createmgv();

uint32 CheckMGGFile(const char *name); //Check if its a MGG file
uint32 LoadMGG(_MGG *mgg, const char *name); //Loads a MGG file into a MGG type struct

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

void _fastcall STW(int32 *x, int32 *y);

uint32 POT(uint32 value);

void _fastcall WTS(int32 *x, int32 *y);

uint32 PlayMovie(const char *name);

void ResetVB();

//Faster than math.h functions
float mCos(int16 ang);
float mSin(int16 ang);
float mTan(int16 ang);

int8 DrawGraphic(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, GLuint data, uint8 a, int16 texpanX, int16 texpanY, int16 texsizeX, int16 texsizeY);
int8 DrawSprite(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, TEX_DATA data, uint8 a, int32 z);
int8 DrawLight(int32 x, int32 y, int32 z, int16 ang, uint8 r, uint8 g, uint8 b, LIGHT_TYPE type, uint8 intensity, float falloff, int32 radius);
int8 DrawLightmap(int32 x, int32 y, int32 z, int32 sizex, int32 sizey, GLuint data, LIGHT_TYPE type);
int8 DrawHud(float x, float y, float sizex, float sizey, float ang, uint8 r, uint8 g, uint8 b, float x1, float y1, float x2, float y2, GLuint data, float a);
int8 DrawLine(float x, float y, float x2, float y2, uint8 r, uint8 g, uint8 b, float a, float linewidth);
int8 DrawString(const char *text, float x, float y, float sizex, float sizey, float ang, uint8 r, uint8 g, uint8 b, float a, TTF_Font *f);
int8 DrawString2UI(const char *text, float x, float y, float sizex, float sizey, float ang, uint8 r, uint8 g, uint8 b, float a, TTF_Font *f);
int8 DrawStringUI(const char *text, float x, float y, float sizex, float sizey, float ang, uint8 r, uint8 g, uint8 b, float a, TTF_Font *f);
int8 DrawUI(float x, float y, float sizex, float sizey, float ang, uint8 r, uint8 g, uint8 b, float x1, float y1, float x2, float y2, GLuint data, float a);

int32 MAnim(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, _MGG *mgf, uint16 id, int16 speed, uint8 a);

void Renderer();

void PlaySound(const char *filename, uint8 loop);
void PlayMusic(const char *filename, uint8 loop);
void MainSound();
void StopAllSounds();
void StopMusic();

uint8 CheckColision(float x, float y, float xsize, float ysize, float tx, float ty, float txsize, float tysize, float ang, float angt);
uint8 CheckColisionMouse(float x, float y, float xsize, float ysize, float ang);
uint8 CheckColisionMouseWorld(float x, float y, float xsize, float ysize, float ang);
uint8 CheckCollisionSector(float x, float y, float xsize, float ysize, float ang, Pos vert[4]);