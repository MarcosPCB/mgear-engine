#ifndef _DOS_BUILD
	#include <GLee.h>
	#include <SDL.h>
	#include <SDL_opengl.h>
	#include <SDL_thread.h>
	#include <SDL_mutex.h>
#ifndef MGEAR_CLEAN_VERSION
	#include <fmod.h>
	#include <fmod_errors.h>
#endif
	#include <SDL_ttf.h>
	//#include <SOIL.h>

	//#include "a.h" 
	#include <immintrin.h>
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
#include <stdarg.h>
#include <intrin.h>

#ifndef _MGTYPES_H
	#include "types.h"
#endif

#if !defined (_VAO_RENDER) && !defined (_VBO_RENDER) && !defined (_VA_RENDER) && !defined (_DOS_BUILD)
	#error Rendering type not defined
	#error Use _VAO_RENDER or _VBO_RENDER or _VA_RENDER or all together
#endif

#ifndef _ENGINE_H
#define _ENGINE_H

#ifdef HAS_SPLASHSCREEN
	#ifndef SPLASHSCREEN_FILE
		#define SPLASHSCREEN_FILE "data/splash.bmp"
	#endif
#endif

#define MAX_SPRITES 256
#define MAX_GRAPHICS 2048
#define MAX_DRAWCALLS 1024
#define TICSPERSECOND 125
#define MAX_CHANNELS 128
#define MUSIC_CHANNEL MAX_CHANNELS-1
#define MAX_SOUNDS 2048
#define MAX_MUSICS 2048
#define MUSIC_SLOT MAX_SOUNDS-1
#define MAX_KEYS 128
#define MAX_OBJS 1024
#define MAX_FONTS 16
#define MAX_SECTORS 512
#define MAX_LIGHTS 16
#define MAX_LIGHTMAPS 256
#define MAX_MAPMGG 32
#define MAX_STRINGS 512
#define MAX_UISCREENS 64
#define MAX_LBLOCKS 128

#define MAX_VERTEX MAX_GRAPHICS*8
#define MAX_COLORS MAX_GRAPHICS*12
#define MAX_INDEX MAX_GRAPHICS*6

#define QLZ_COMPRESSION_LEVEL 3
#define QLZ_STREAMING_BUFFER 0
#define QLZ_MEMORY_SAFE 0

#define LINEVB 0

#ifndef MGEAR_CLEAN_VERSION
typedef FMOD_SOUND Sound;
typedef FMOD_CHANNEL Channel;
#endif

#define FONT_SIZE 1024 //Used for overriding the size draw functions
#define TEX_PAN_RANGE 32768 //tex panning ranges from 0 to 32768

//MGG format
#define MAX_FRAMES 8192
#define MAX_ANIMATIONS 64
#define MAX_MAP_MGG 64
#define MAX_GAME_MGG 128

#define pi 3.14159265

#define MGG_START 3 //The first slot to be used for loading sprite MGGs

#define SYS_BOX_TILE 4

#define UI_CALL 0
#define HUD_CALL 1
#define LINE_CALL 2
#define GRAPHICS_CALL 3
#define STRING_CALL 4
#define STRING2_CALL 5
#define STRINGUI_CALL 6
#define STRINGUI2_CALL 7
#define STRINGUIV_CALL 8

#define BCK2_DEFAULT_VEL 0.1
#define BCK1_DEFAULT_VEL 0.5
#define FR_DEFAULT_VEL 1.3

#define GAME_WIDTH 16384 //1024 * 16
#define GAME_HEIGHT 9216 //576 * 16

#define GAME_ASPECT 1.777777777778

#define GAME_UNIT_MAX 2000000000
#define GAME_UNIT_MIN -2000000000

double _inline sqrt14(double n);

#define LogApp SDL_Log
#define LogErr SDL_LogError
#define LogWn SDL_LogWarn
#define CloseFont TTF_CloseFont
#define GetTicks SDL_GetTicks
#define PollEvent SDL_PollEvent
#define SwapBuffer SDL_GL_SwapWindow

//#define MAX_MGVFRAMES 65536

#define CURSOR_NORMAL 0
#define CURSOR_RESIZE_VERTICAL 1
#define CURSOR_RESIZE_HORIZONTAL 2
#define CURSOR_RESIZE_DLEFT 3
#define CURSOR_RESIZE_DRIGHT 4

//OBJ flags

#define OBJF_TEXTURE_MOV 1
#define OBJF_ANIMATED_TEXTURE_MOV_CAM 2

//Layers
#define LIGHT_VIEW 64
#define INGAME_VIEW 32
#define BACKGROUND3_VIEW 16
#define BACKGROUND2_VIEW 8
#define BACKGROUND1_VIEW 4
#define MIDGROUND_VIEW 2
#define FOREGROUND_VIEW 1

//Inter app commands.
#define IA_OPENFILE 0

//#include "mgl.h"

enum _Enttype
{
	SPRITE,
	HUD,
	TEXTURE,
	VIDEO,
	TEXT,
	TEXT_UI,
	UI,
	POINT_LIGHT,
	SPOT_LIGHT,
	LINE,
	ent_none
};

typedef enum _Enttype Enttype;

enum _Stat
{
	DEAD,
	USED
};

typedef enum _Stat Stat;

struct _VB_DATAT
{
	GLuint vao_id;
	GLuint ibo_id;
	GLuint vbo_id;
	float *vertex;
	float *texcoord;
	float *texcoordlight;
	float *texrepeat;
	float *lblocker;
	GLubyte *color;
	GLushort *index;
	GLuint texture;
	GLuint Ntexture; //normal map
	uint8 normal; //boolean for verification
	uint16 num_elements;
	uint16 buffer_elements;
	uint16 num_elements2;
	//uint16 quad_loc[MAX_GRAPHICS];
	GLint imgdata;
	//uint8 imgNdata;
	int w, h;
};

typedef struct _VB_DATAT VB_DATAT;

struct _TEX_DATA
{
	GLuint data;
	GLuint Ndata; //normal map
	GLuint Sdata;
	uint8 normal; //boolean for verification
	int16 vb_id;
	uint16 loc;
	uint16 posx, posy; //position in atlas
	uint16 sizex, sizey; //size in atlas
	int w; 
	int h; 
	int channel; //texture dimensions
	int16 x_offset;
	int16 y_offset;
	
};

typedef struct _TEX_DATA TEX_DATA;

struct _ENTITIES_ //To be rendered
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
	float texcorlight[8];
	float vertex2[12];
	GLubyte color[16];
	ColorF Color;
	int8 scenario;
	int8 circle;
	float l_blocker[4];
};

typedef struct _ENTITIES_ _ENTITIES;

enum _LIGHT_TYPE
{
	POINTLIGHT = 0,
	SPOTLIGHT = 1,
	TGA_FILE = 2
};

typedef enum _LIGHT_TYPE LIGHT_TYPE;

enum _LIGHTENG_TYPE
{
	STATIC_LIGHT,
	DYNAMIC_LIGHT
};

typedef enum _LIGHT_TYPE LIGHT_TYPE;

struct _LIGHTS_
{
	LIGHT_TYPE type;
	PosF pos;
	ColorF color;
	float falloff;
	float radius;
};

typedef struct _LIGHTS_ _LIGHTS;

struct _Key
{
	SDL_Scancode key;
	uint8 state;
};

typedef struct _Key Key;

#ifndef MGEAR_CLEAN_VERSION
//FMOD sound system
struct _SOUNDSYS_
{
	FMOD_SYSTEM *Sound_System;
	Channel *music;
};

typedef struct _SOUNDSYS_ _SOUNDSYS;

#endif

enum _MGGTYPE_
{
	MGG_USED,
	NONE
};

typedef enum _MGGTYPE_ _MGGTYPE;

struct _MGGANIM_
{
	char name[32];
	uint16 num_frames;
	uint16 current_frame;
	uint16 startID;
	uint16 endID;
	int8 speed;
};

typedef struct _MGGANIM_ _MGGANIM;

//File header
struct _MGGFORMAT_
{
	char name[32];
	uint16 num_frames;
	uint8 num_atlas;
	uint16 num_singletex;
	uint16 num_texinatlas;
	uint32 num_animations;
	size_t textures_offset;
	size_t possize_offset;
	size_t framesize_offset;
	size_t framealone_offset;
	size_t frameoffset_offset;
	int8 mipmap;
};

typedef struct _MGGFORMAT_ _MGGFORMAT;

//MGG -> MGear Graphics
struct _MGG_
{
	char name[32];
	uint16 num_frames, num_atlas;
	_MGGTYPE type;
	TEX_DATA *frames; //single-texture and atlas objects data
	GLint *atlas;

	Pos *size;
	//Pos *sizefix;
	uint32 num_anims;
	_MGGANIM *anim;

	char path[MAX_PATH];
};

typedef struct _MGG_ _MGG;

//File header for the video
struct _MGVFORMAT_
{
	uint32 num_frames;
	uint8 fps;
	uint32 sound_buffer_lenght;
	uint32 sound_seeker;
};

typedef struct _MGVFORMAT_ _MGVFORMAT;

struct _MGVTEX_
{
	GLuint ID;
	SDL_Surface *data;
	void *buffer;
	SDL_RWops *rw;
};

typedef struct _MGVTEX_ _MGVTEX;

//MGV -> MGear Video
struct _MGV_
{
	uint32 num_frames;
	_MGVTEX *frames;
	size_t totalsize;
	uint8 fps;
#ifndef MGEAR_CLEAN_VERSION
	FMOD_SOUND *sound;
#endif
	FILE *file;
	uint32 *framesize;
	uint32 *seeker;
};

typedef struct _MGV_ _MGV;

struct PMData
{
	FILE *file;
	uint32 *framesize;
	uint32 num_frames;
	uint32 *loaded_frames_addr;
	uint32 loaded_frames;
	uint32 curr_frame;
	int32 seen;
	uint32 *seeker;
	SDL_sem *sem;
	uint32 w, h, channel;
};

//This is where all map textures information stays
//Each map has it's on set of MGG that can be shared between the other maps
//Some MGGs that contains the menu and some UI textures are basic and contain it's own structure
struct _TEXTURES_
{
	uint32 ID;
	uint16 MGG_ID;
	char name[32];
};

typedef struct _TEXTURES_ _TEXTURES;

enum _SPRITE_G_
{
	GAME_LOGICAL = 0,
	ENEMY = 1,
	FRIEND = 2,
	NORMAL = 3,
	dead = 4
};

typedef enum _SPRITE_G_ _SPRITE_G;

enum _Material
{
	METAL,
	WOOD,
	PLASTIC,
	CONCRETE,
	ORGANIC,
	MATERIAL_END
};

typedef enum _Material Material;

struct _BODY
{
	uint8 physics_on;
	uint16 mass;
	Pos size;
	uint16 max_elasticy;
	Material material;
	uint8 flamable;
	uint8 explosive;
	Pos position;
	int16 total_vel;
	Pos velxy;
	int16 acceleration;
	int16 acc_ang;
	int16 energy;
	int16 temperature;
	int16 ang;
	int16 sector_id;
	int16 damage_owner;
};

typedef struct _BODY Body;

enum _OBJTYPE_
{
	BACKGROUND1 = 0,
	BACKGROUND2 = 1,
	BACKGROUND3 = 2,
	MIDGROUND = 3,
	FOREGROUND = 4,
	BLANK = 5
};

typedef enum _OBJTYPE_ _OBJTYPE;

struct _AISTATE
{
	char name[32];

	uint16 in_transition, out_transition; //transition animations
	uint16 main_anim; //main animation

	uint8 in, out; //enable or disable in/out transitions

	int16 inputs[64], outputs[64]; //input and output nodes

	uint8 used;
	uint8 loop;
	uint8 animation;
};

typedef struct _AISTATE AISTATE;

//Structure for the sprites in the game
//When you create a sprite in the source code
//You must add it to the structure
struct _SPRITES_
{
	char name[64];
	int16 MGG_ID;
	int16 num_frames;
	int16 num_start_frames;
	int16 frame[8];
	int8 num_tags;
	int16 tags[8];
	char tag_names[8][16];
	char tags_str[8][1024];
	char tags_ext[8][6];
	char tag_mod_list[8][32];
	int8 num_ext;
	int8 num_list_tag;
	int16 health;
	_SPRITE_G type;
	int16 flags;
	Pos size_a;
	Pos size_m;
	Body body;
	AISTATE states[64];
	uint8 num_states;
	uint8 shadow;
};

typedef struct _SPRITES_ _SPRITES;



enum _OBJBLOCK_
{
	ALL,
	PLAYER,
	BULLET,
	ENEMYB,
	none
};

typedef enum _OBJBLOCK_ _OBJBLOCK;

enum _SPRITE_T_
{
	ANIMATED,
	TEXTURED,
	ENTITY,
	non
};

typedef enum _SPRITE_T_ _SPRITE_T;

struct _SECTOR_
{
	int8 sloped;
	int32 base_y;
	Pos vertex[2];

	Material material;
	int16 tag;
	uint8 destructive : 2;
	int16 id;
	uint8 num_vertexadded;
	uint8 floor_y_continued;
	int32 floor_y_up, floor_y_down;
	uint8 type; //Floor = 1, Ceiling = 2 - bitwise
};

typedef struct _SECTOR_ _SECTOR;

struct _MGMLIGHT_
{
	Pos w_pos;

	uint32 W_w;
	uint32 W_h;

	uint16 T_w;
	uint16 T_h;

	uPos16 t_pos[16];

	uPos16 t_pos2[16];

	uint8 num_lights;

	LIGHT_TYPE type[16];

	ColorA16 color[16];

	float falloff[16];

	int16 spot_ang[16];

	int16 obj_id;

	int16 ang;

	uint8 alpha;

	Color ambient_color;
};

typedef struct _MGMLIGHT_ _MGMLIGHT;

struct _MGMOBJ_
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
	uint16 flag;
	float amblight;
	//int16 lightmapid;
	int16 current_frame;
};

typedef struct _MGMOBJ_ _MGMOBJ;

//Map sprites
struct _MGMSPRITE_
{
	//if it's just a texture, you MUST inform the frame ID
	uint16 MGG_ID;

	//Leave it blank if it's not a texture
	int16 frame_ID;
	_SPRITE_T type;

	int32 stat;

	//If it's an entity, insert here the game sprite name that has a code
	//or else, just leave it blank
	char game_name[64]; 

	_OBJTYPE type_s;

	uint32 GameID;

	Body body;

	int8 num_tags;

	int16 tags[8];

	int16 current_frame;

	char tags_str[8][1024];

	int16 health;

	int16 current_sector;
	int8 current_layer;

	Pos position;
	Pos size_a;
	Pos size_m;

	int16 angle;

	Color color;

	int16 flags;
};

typedef struct _MGMSPRITE_ _MGMSPRITE;

struct _MGMBLOCK
{
	uint8 type; //0 - Circle, 1 - Quad
	int32 vertex[8];
	uint8 enabled; //0 - Enabled, 1 - Disabled
};

typedef struct _MGMBLOCK MGMBLOCK;

//MGM -> MGear Map
struct _MGM_
{
	char name[32];
	_MGMSPRITE *sprites;
	_MGMOBJ *obj;
	_SECTOR *sector;
	uint16 num_sprites;
	uint16 num_obj;
	uint8 num_mgg;
	uint8 num_lights;
	uint16 num_sector;
	float bck2_v;
	float bck1_v;
	float fr_v;
	uint8 bcktex_mgg;
	int16 bcktex_id;
	char MGG_FILES[32][256];
	Color amb_color;

	struct CamArea
	{
		Pos area_pos;
		PosF area_size;
		int8 horiz_lim;
		int8 vert_lim;
		Pos limit[2];

		PosF max_dim;
	} cam_area;

	Pos bck3_pan;
	Pos bck3_size;

	char activator_table[2048];

	uint8 num_global_states;

	uint16 num_blocks;

	MGMBLOCK *blocks;
};

typedef struct _MGM_ _MGM;

struct _MGMFORMAT_
{
	char name[32];
	uint16 num_sprites;
	uint16 num_obj;
	uint8 num_mgg;
	uint8 num_lights;
	uint16 num_sector;
	float bck2_v;
	float bck1_v;
	float fr_v;
	uint8 bcktex_mgg;
	int16 bcktex_id;
	char MGG_FILES[32][256];
	Color amb_color;

	struct Cam_Area
	{
		Pos area_pos;
		PosF area_size;
		int8 horiz_lim;
		int8 vert_lim;
		Pos limit[2];

		PosF max_dim;
	} cam_area;

	Pos bck3_pan;
	Pos bck3_size;
};

typedef struct _MGMFORMAT_ _MGMFORMAT;

struct _CAMERA_
{
	Pos position;
	int16 angle;
	PosF dimension;
};

typedef struct _CAMERA_ _CAMERA;

enum _GAME_STATE
{
	MAIN_MENU,
	INGAME,
	GAME_MENU,
	STARTUP,
	LOADING
};

typedef enum _GAME_STATE GAME_STATE;

struct _Control
{
	SDL_GameController *device;

	struct Button
	{
		SDL_GameControllerButton name;
		uint8 state;
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

typedef struct _Control Control;

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
/*
struct UNIFORM_SHADER
{
	GLenum type;
	GLint loc;
	GLint vali;
	GLfloat valf;
	GLfloat valv2[2];
	GLfloat valv3[3];
	GLfloat valv4[4];
	GLdouble vald;
};

struct PROGRAM_SHADER
{
	GLuint VShader, FShader, GShader, Program;
	UNIFORM_SHADER *uniforms;

	int8 num_uniforms;
};
*/

#ifndef MGEAR_CLEAN_VERSION
typedef struct
{
	Sound *sound;
	uint8 loop;
	char path[256];
	uint8 type; //0 = ambience sound; 1 = fx; 2 = player; 3 = npc; 4 = talk; 5 = music
	uint8 priority;

} SOUND_LIST;
#endif

typedef struct
{
	uint8 type;

	Pos pos;
	Pos pos2;
	Pos size;
	Pos size2;
	Color color;
	char text[256];
	int32 tex_panx;
	int32 tex_pany;
	int32 tex_sizex;
	int32 tex_sizey;
	int16 ang;
	int8 font;

	TEX_DATA data;

} PIPELINE;

struct _Render
{
	uint8 VAO_ON;

	GLuint VAO_1Q;
	GLuint *VAO;
	uint8 VBO_ON;

	uint8 VA_ON;

	//Basic shaders
	GLuint VShader[16];
	GLuint FShader[16];
	GLuint GShader[16];
	GLuint Program[16];

	GLint unifs[24];

	//Custom shaders
	/*
	PROGRAM_SHADER Custom_Shader[8];
	uint8 use_custom_shader;
	uint8 num_custom_shaders;
	*/

	GLuint FBO[4];
	GLuint FBTex[16];
	GLuint RBO[8];

	GLuint Buffers[16];

	uint16 shader_version;

	PIPELINE ppline[MAX_DRAWCALLS];

	uint16 vpx, vpy;
};

typedef struct _Render Render;

struct _TFont
{
	char name[64];
	TTF_Font *font;
	size_t size_w_px;
	size_t size_h_px;
	size_t size_w_gm;
	size_t size_h_gm;
};

typedef struct _TFont TFont;

struct _GAME_LIGHTMAPS_
{
	Pos w_pos;

	int32 W_w;
	int32 W_h;

	uint16 T_w;
	uint16 T_h;

	uPos16 t_pos[16];

	uPos16 t_pos2[16];

	Pos s_dir;
	int16 spotcos, spotinnercos;

	unsigned char *data;

	uint8 num_lights;

	GLuint tex;

	uint8 stat;

	LIGHT_TYPE type;

	ColorA16 color[16];

	float falloff[16];

	int16 spot_ang[16];

	int16 obj_id;

	int16 ang;

	uint8 alpha;

	Color ambient_color;

	uint8 num_shadows;
};

typedef struct _GAME_LIGHTMAPS_ _GAME_LIGHTMAPS;

struct _STRINGS_E
{
	char string[1024];
	TEX_DATA data;
	int8 stat; //0 - not used, 1 - for rendering, 2 - rendered and waiting for next frame  
};

typedef struct _STRINGS_E StringsE;

//MGL code

struct MGLHeap
{
	int32 stack_pos;
	size_t size;
	size_t cur;
	void *mem;
	char *string;
	uint8 type; //0 - buffer, 1 - string
};

//Flags register
union MGLFlags
{
	struct
	{
		uint8 cm : 1; //conditional result from last comparision - 0: false, 1:true
		uint8 ca : 1; //if last comparision contained an and
		uint8 co : 1; //if last comparision containd an or
		uint8 fc : 1; //float conversion flag
		uint8 ic : 1; //integer conversion flag
		uint8 zf : 1; //zero flag
		uint8 of : 1; //overflow flag
		uint8 r : 1; //reserved
	};

	uint8 flags;
};

struct MGLCode
{
	unsigned char *code;
	size_t size;
	uint16 cv;
	uint32 v[32];
	float  f[24];
	uint32 bp, sp, stack_type, memsize;

	union MGLFlags flags;

	int32 *stack;

	struct MGLHeap *heap;
	uint16 num_heap;

	struct Funcs
	{
		void(*log)(const char *msg, ...);
		int32(*msgbox)(const char *quote, UINT type, const char *msg, ...);
		void(*drawline)(int32 x, int32 y, int32 x2, int32 y2, uint8 r, uint8 g, uint8 b, uint8 a, int16 linewidth, int32 z);
		uint32(*playmovie)(const char *name);
		uint32(*playbgvideo)(const char *name, uint8 play);
		void(*drawui)(int32 x, int32 y, float size, int16 ang, uint8 r, uint8 g, uint8 b, TEX_DATA data, uint8 a, int8 layer);
		void(*drawuistring)(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z);
		void(*drawuistringl)(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z);
	} funcs;
	uint32 ret_addr;
};

//The main structure
//Contais all the information about the game
struct _SETTINGS_
{
	uint32 backtrack;
	char typetext[128];
	char WINDOW_NAME[64];
	char WindowTitle[48];

	uint16 screenx;
	uint16 screeny;
	uint16 gamex, gamey;
	float aspect;
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
	uint8 num_shadows;
	uint16 num_calls;

	uint8 num_uiwindow;

	uint8 num_mgg;
	uint8 num_mgg_basic;

	_GAME_LIGHTMAPS game_lightmaps[MAX_LIGHTMAPS];

#ifndef MGEAR_CLEAN_VERSION
	uint16 num_sounds;
	uint16 num_musics;

	SOUND_LIST sounds[MAX_SOUNDS];
	SOUND_LIST musics[MAX_MUSICS];
#endif

	struct
	{
		GLuint lightmap;

	} Lightmaps;

	long long unsigned int time;
#ifndef MGEAR_CLEAN_VERSION
	_SOUNDSYS sound_sys;
#endif
	TFont fonts[MAX_FONTS];

	Pos mouse;
	uint8 mouse1;
	uint8 mouse2;
	uint8 mouse3;
	uint8 mouse_on : 2;
	int32 mouse_wheel;

	Key keys[MAX_KEYS];

	uint8 quit;
	uint8 PlayingVideo;

	char TextInput[128];
	uint8 Text_Input;

	_MGM Current_Map;

	uint8 sprite_id_list[MAX_SPRITES];
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
	float CosTable[3601];
	float SinTable[3601];
	float TanTable[3601];

	StringsE strings[MAX_STRINGS];

	uint8 cursor_type;

	int8 viewmode;

	char CurrPath[2048];
	char LogName[32];

	TEX_DATA BasicTex;
	TEX_DATA Line;

	HANDLE process;
	HANDLE thread;

	struct MGLCode mgl;
};

typedef struct _SETTINGS_ _SETTINGS;

#endif

//#ifdef MFC_MGEAR
//extern "C"
//{
//#endif

extern _SETTINGS st;
extern _ENTITIES ent[MAX_GRAPHICS];
extern SDL_Event events;
extern _LIGHTS game_lights[MAX_LIGHTS];
extern _ENTITIES lmp[MAX_LIGHTMAPS]; 

extern _MGG mgg_sys[3];
extern _MGG mgg_map[MAX_MAP_MGG];
extern _MGG mgg_game[MAX_GAME_MGG];
extern SDL_Window *wn;

extern const char WindowTitle[32];

void _ProcessError(const char* funcname, int line, int8 silent);

#define GetError _ProcessError(__FUNCTION__, __LINE__, NULL)
#define GetErrorS _ProcessError(__FUNCTION__, __LINE__, 1)
#define ProcessError(silent) _ProcessError(__FUNCTION__, __LINE__, silent)

void PreInit(const char AppName[4], int argc, char *argv[]);

void Init();

uint8 OpenFont(const char *file,const char *name, uint8 index, size_t font_size);

extern void Quit();

//For tests only
//void createmgg();
//void createmgv();

int32 CheckMGGInSystem(const char *name); //Check if the MGG is already loaded into the system
uint32 CheckMGGFile(const char *name); //Check if its a MGG file
uint32 _LoadMGG(_MGG *mgg, const char *name, uint8 shadowtex); //Loads a MGG file into a MGG type struct
#define LoadMGG(mgg, name) _LoadMGG(mgg, name, 0)

void FreeMGG(_MGG *file); //Free a MGG struct
void InitMGG(); //Inits all MGG structs

#ifndef MGEAR_CLEAN_VERSION
#ifdef ENGINEER
	uint32 SaveMap(const char *name);
#endif

#endif

int32 LoadSpriteCFG(char *filename, int id);
int32 LoadSpriteList(char *filename);

#ifndef MGEAR_CLEAN_VERSION

uint8 LoadMGGList(const char *file);

uint32 LoadMap(const char *name);
void FreeMap();

void DrawMap();

#endif

void _fastcall SetTimerM(unsigned long long int x);
unsigned long long _fastcall GetTimerM();

void FPSCounter();

void CreateLog();
void LogIn(void *userdata, int category, SDL_LogPriority, const char *message);

int32 POFCeil(int32 value);

#ifdef HAS_SPLASHSCREEN

int DisplaySplashScreen();

#endif

void InitEngineWindow();

void RestartVideo();

void WindowEvents();

void _inline STW(int32 *x, int32 *y);

void _inline STWci(int32 *x, int32 *y); //No camera position in calculation

void _fastcall STWf(float *x, float *y);

void _inline STWcf(float *x, float *y); //No camera position in calculation

void _fastcall WTS(int32 *x, int32 *y);

void _fastcall WTSci(int32 *x, int32 *y); //No camera position in calculation

void _inline WTSf(float *x, float *y);

void _inline WTScf(float *x, float *y); //No camera position in calculation

void AddCamCalc(Pos *pos, Pos *size); //Add camera calculation

#ifndef MGEAR_CLEAN_VERSION
uint32 PlayMovie(const char *name);
uint32 PlayBGVideo(const char *name, uint8 play);
#endif

void ResetVB();

#ifndef MGEAR_CLEAN_VERSION
//Lightmap functions
unsigned char *GenerateAlphaLight(uint16 w, uint16 h);
uint8 FillAlphaLight(unsigned char *data, uint8 r, uint8 g, uint8 b, uint16 w, uint16 h);
uint32 AddLightToAlphaLight(unsigned char *data, uint16 w, uint16 h, uint8 r, uint8 g, uint8 b, float falloff, uint16 x, uint16 y, uint16 z, uint16 intensity, LIGHT_TYPE type);
uint32 AddSpotlightToAlphaLight(unsigned char *data, uint16 w, uint16 h, uint8 r, uint8 g, uint8 b, float falloff, uint16 x, uint16 y, uint16 z, uint16 intensity, LIGHT_TYPE type, uint16 x2, uint16 y2, uint16 ang);
GLuint GenerateAlphaLightTexture(unsigned char* data, uint16 w, uint16 h);
uint8 AddLightToAlphaTexture(GLuint *tex, unsigned char* data, uint16 w, uint16 h);

unsigned char *GenerateLightmap(uint16 w, uint16 h); //Generate a raw data lightmap
uint32 AddLightToLightmap(unsigned char *data, uint16 w, uint16 h, uint8 r, uint8 g, uint8 b, float falloff, uint16 x, uint16 y, uint16 z, uint16 intensity, LIGHT_TYPE type);
uint32 AddSpotlightToLightmap(unsigned char *data, uint16 w, uint16 h, uint8 r, uint8 g, uint8 b, float falloff, uint16 x, uint16 y, uint16 z, uint16 intensity, LIGHT_TYPE type, uint16 x2, uint16 y2, uint16 ang);
GLuint GenerateLightmapTexture(unsigned char* data, uint16 w, uint16 h);
uint8 AddLightToTexture(GLuint *tex, unsigned char* data, uint16 w, uint16 h);
uint8 FillLightmap(unsigned char *data, uint8 r, uint8 g, uint8 b, uint16 w, uint16 h);

uint32 NAddLightToLightmap(unsigned char *data, uint16 w, uint16 h, uint8 r, uint8 g, uint8 b, float constant, float linear, float quadratic, float intensity);
#endif

//Faster square root

#ifdef _WIN64
	float _cdecl mSqrt64(float);
	#define mSqrt mSqrt64
#elif _WIN32
	float _cdecl mSqrt32(float);
	#define mSqrt mSqrt32

	int32 _cdecl mXOR32(int32, int32);
	#define mXOR mXOR32
#endif

#define P2(expr) expr * expr
#define GetDistance(x, y, x2, y2, dist) { dist = mSqrt(P2((x - x2)) + P2((y - y2))); }

//Faster than math.h functions
float mCos(int16 ang);
float mSin(int16 ang);
float mTan(int16 ang);

//Calculates the Cos, Sin and Tan values for you in signed integers
void CalCos16s(int16 ang, int16 *val);
void CalSin16s(int16 ang, int16 *val);
void CalTan16s(int16 ang, int16 *val);

void CalCos32s(int16 ang, int32 *val);
void CalSin32s(int16 ang, int32 *val);
void CalTan32s(int16 ang, int32 *val);

//Calculates the Cos, Sin and Tan values for you in unsigned integers
void CalCos16u(int16 ang, uint16 *val);
void CalSin16u(int16 ang, uint16 *val);
void CalTan16u(int16 ang, uint16 *val);

void CalCos32u(int16 ang, uint32 *val);
void CalSin32u(int16 ang, uint32 *val);
void CalTan32u(int16 ang, uint32 *val);

//Returns the Z position for your current layer
int32 GetZLayer(int32 curr_z, _OBJTYPE curr_layer, _OBJTYPE layer);

//Checks if the object/sprite/graphic is inside the screen bounds
int8 CheckBounds(int32 x, int32 y, int32 sizex, int32 sizey, int8 z);

//Draws the ambient light
//THIS MUST GO BEFORE ANY DRAWING COMMAND
void BASICBKD(uint8 r, uint8 g, uint8 b);

int8 DrawObj(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, TEX_DATA data, uint8 a, int32 x1, int32 y1, int32 x2, int32 y2, int8 z, int16 lightmap_id);
int8 DrawGraphic(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, TEX_DATA data, uint8 a, int32 x1, int32 y1, int32 x2, int32 y2, int8 z, uint16 flag);
int8 DrawSprite(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, TEX_DATA data, uint8 a, int32 z, int16 flags, int32 sizeax, int32 sizeay, int32 sizemx, int32 sizemy);
int8 DrawLight(int32 x, int32 y, int32 z, int16 ang, uint8 r, uint8 g, uint8 b, LIGHT_TYPE type, uint8 intensity, float falloff, int32 radius);
int8 DrawSpotLight(int32 x, int32 y, int32 z, int32 radius, Color color, float falloff, float intensity, float f3, int32 sx2, int32 sy2, int16 ang, int16 innerang, uint8 light_id);
int8 DrawHud(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, int32 x1, int32 y1, int32 x2, int32 y2, TEX_DATA data, uint8 a, int8 layer);
int8 DrawLine(int32 x, int32 y, int32 x2, int32 y2, uint8 r, uint8 g, uint8 b, uint8 a, int16 linewidth, int32 z);
int8 DrawCircle(int32 x, int32 y, int32 radius, uint8 r, uint8 g, uint8 b, uint8 a, int32 z);
int8 DrawString(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z);
int8 DrawString2(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z); //Light does not affect
int8 DrawString2UI(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z);
int8 DrawStringUI(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z);
int8 DrawUI(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, int32 x1, int32 y1, int32 x2, int32 y2, TEX_DATA data, uint8 a, int8 layer);
int8 DrawPolygon(Pos vertex_s[4], uint8 r, uint8 g, uint8 b, uint8 a, int32 z); //Draw a polygon with no lighting or texture data.
int8 DrawUI2(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, int32 x1, int32 y1, int32 x2, int32 y2, TEX_DATA data, uint8 a, int8 layer); //Camera affected
int8 DrawStringUIv(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z); //Starts at the first vertice

int8 CheckAnim(int16 sprite_id, int16 anim);
void SetAnim(int16 id, int16 sprite_id);
int8 MAnim(int16 id, float speed_mul, int16 sprite_id, int8 loop);
void GetSpriteBodySize(int16 id, int16 gameid);

void Renderer(uint8 type);

#if !defined (MGEAR_CLEAN_VERSION) || defined (ENABLE_SOUND_SYS)
int8 LoadSoundList(char *name);
void PlaySound(int16 id, uint8 loop);
void PlayMusic(int16 id, uint8 loop);
void MainSound();
void StopAllSounds();
void StopMusic();
#endif

#ifndef MGEAR_CLEAN_VERSION
void SpawnSprite(int16 game_id, Pos pos, Pos size, int16 ang);

uint16 CheckCollision(Pos pos, Pos size, int16 ang, Pos pos2, Pos size2, int16 ang2);
#endif
uint8 CheckCollisionMouse(int32 x, int32 y, int32 xsize, int32 ysize, int32 ang);
uint8 CheckCollisionMouseWorld(int32 x, int32 y, int32 xsize, int32 ysize, int32 ang, int8 z);

#ifndef MGEAR_CLEAN_VERSION
int16 CheckCollisionSector(int32 x, int32 y, int32 xsize, int32 ysize, int16 ang, int32 *sety, int16 sectorid);
int16 CheckCollisionSectorWall(int32 x, int32 y, int32 xsize, int32 ysize, int16 ang);
int16 CheckCollisionSectorWallID(int32 x, int32 y, int32 xsize, int32 ysize, int16 ang, int16 id);
uint8 CheckCollisionPossibility(uint16 id, uint16 id2, int32 *dist); //Checks if the two sprites are in the possibility of a collision and returns the distance they are of each other
#endif

void UIData(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, int32 x1, int32 y1, int32 x2, int32 y2, TEX_DATA data, uint8 a, int8 layer);
void UIezData(int32 x, int32 y, float size, int16 ang, uint8 r, uint8 g, uint8 b, TEX_DATA data, uint8 a, int8 layer);
void HudData(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, int32 x1, int32 y1, int32 x2, int32 y2, TEX_DATA data, uint8 a, int8 layer);
void GraphicData(int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, TEX_DATA data, uint8 a, int32 x1, int32 y1, int32 x2, int32 y2, int8 z);
void StringUI2Data(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z);
void StringUIData(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z);
void StringUIvData(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z);
void String2Data(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z);
void StringData(const char *text, int32 x, int32 y, int32 sizex, int32 sizey, int16 ang, uint8 r, uint8 g, uint8 b, uint8 a, uint8 font, int32 override_sizex, int32 override_sizey, int8 z);
void LineData(int32 x, int32 y, int32 x2, int32 y2, uint8 r, uint8 g, uint8 b, uint8 a, int16 linewidth, int32 z);

void Finish();

void DrawSys();

int8 LoadLightmapFromFile(const char *file);

void LockCamera();

int16 LoadTexture(const char *file, uint8 mipmap, Pos *size);

int16 LoadTextureM(void *img_data, size_t data_size, uint8 mipmap, Pos *size);

char *CheckAppComm(int8 *command);
void SendInfo(const char *commfile, const char *data, int8 command);
void ResetAppComm();

//#ifdef MFC_MGEAR
//}
//#endif