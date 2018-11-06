#include "types.h"
#include "engine.h"

#define DEFAULT_GRAVITY 8
#define AIR_RESISTANCE_MAX 10

#define MAX_DYNAMIC_SPRITES 128
#define MAX_ACTIVE_DYNAMIC_SPRITES 64

#define MAX_PARTICLES 128

#define MAX_SPEED 4096

#define MAX_PROJECTILES 128
#define MAX_GAME_PROJECTILES 32

#define BULLET_RADIUS 32 //used for collsion calculation, it calculates it as a small circle

struct _PARTICLES
{
	Pos pos;
	Pos size;
	Pos vel;
	int16 total_vel;
	int16 vel_ang;
	int16 ang;
	
	int16 frame_id;

	uint8 anim_id;
	uint8 mgg_id;

	uint8 animated;

	int16 speed;

	int16 sector;

	float speed_mul;

	int16 id;

	int8 stat;

	void(*func)(PARTICLES);
};

typedef struct _PARTICLES PARTICLES;

struct _GAME_PARTICLES
{
	Pos size;

	int16 frame_id;

	uint8 anim_id;
	uint8 mgg_id;

	uint8 animated;

	float speed_mul;

	int16 speed;

	void(*func)(PARTICLES);
};

typedef struct _GAME_PARTICLES GAME_PARTICLES;

enum PROJ_TYPE
{
	PROJ_BULLET = 1,
	PROJ_REGULAR = 2,
	PROJ_EXPLODES_ON_IMPACT = 4,
	PROJ_TIMED_EXPLOSION = 8,
	PROJ_SPAWNS_SPRITE = 16,
	PROJ_TIMED = 32,
	PROJ_RANGED = 64,
	PROJ_GRAVITY_FALLS = 128,
	PROJ_AIR_RESISTENCE = 256,
	PROJ_TRAIL = 512,
	PROJ_SPAWNS_PARTICLE = 1024,
	PROJ_BULLET_NATO = 2048
};

struct _PROJECTILE
{
	Pos pos;
	Pos size;
	Pos vel;

	int16 ang;

	int16 frame_id;

	uint8 anim_id;
	uint8 mgg_id;

	uint8 bounce : 1,
	      num_bounces : 3,
		  animated : 1,
		  rnd_bounce : 1,
		  vel_bounce : 1;

	int16 mass;

	float speed_mul;

	int16 damage;

	enum PROJ_TYPE type;

	uint32 radius;

	int32 time;

	int8 stat;

	int16 owner;

	int16 trail : 8,
	spawns : 8;

	uint8 num_trail;

	uint8 id;

	int16 health;

	int32 trail_time;
};

typedef struct _PROJECTILE PROJECTILE;

struct _GAME_PROJECTILE
{
	int16 frame_id;

	uint8 anim_id;
	uint8 mgg_id;

	uint8 bounce : 1,
		  num_bounces : 3,
		  animated : 1,
		  rnd_bounce : 1,
		  vel_bounce : 1;

	int16 mass;

	float speed_mul;

	int16 damage;

	enum PROJ_TYPE type;

	uint32 radius;

	uint32 time;

	int32 speed;

	Pos size;

	int16 trail : 8,
	      spawns : 8;

	uint8 num_trail;

	int16 health;
			
};

typedef struct _GAME_PROJECTILE GAME_PROJECTILE;

struct _Physics
{
	uint8 physics_active;
	uint8 gravity;
	int32 gravity_vel;
	int16 gravity_direction;
	uint8 air;
	int16 sprite_list[MAX_DYNAMIC_SPRITES];
	int16 active_sprites[MAX_ACTIVE_DYNAMIC_SPRITES];
	uint16 num_active_sprites;
	uint16 num_sprites;

	uint16 sectorarea[MAX_SECTORS];
	int16 sectorarea_ids[MAX_SECTORS][MAX_DYNAMIC_SPRITES];

	int16 spritesectornum[MAX_ACTIVE_DYNAMIC_SPRITES];
	int8 spritesectors[MAX_ACTIVE_DYNAMIC_SPRITES][MAX_SECTORS];

	uint8 num_particles;

	PARTICLES particles[MAX_PARTICLES];

	GAME_PARTICLES game_part[MAX_PARTICLES];

	int16 part_list[MAX_PARTICLES];

	PROJECTILE proj[MAX_PROJECTILES];

	int16 proj_list[MAX_PROJECTILES];

	uint8 num_proj;

	GAME_PROJECTILE game_proj[MAX_GAME_PROJECTILES];
};

typedef struct _Physics Physics;

void InitPhysics(int8 gravity, int32 gravity_vel, int16 gravity_direction, int8 air);

int16 UpdateSector(Pos pos, Pos size);

int8 AddDynamicSprite(int16 id);
void RemoveDynamicSprite(int16 id);

uint8 DefineProjectileProperties(int8 id, enum PROJ_TYPE type, int16 damage, uint32 radius, uint32 time, int16 trail, uint8 num_trail, int16 spawns);
uint8 DefineProjectilePhysics(int8 id, uint8 bounce, uint8 num_bounces, uint8 rnd_bounce, uint8 vel_bounce, int16 mass, int32 sizex, int32 sizey, int32 speed);
uint8 DefineProjectileVisual(int8 id, int16 frame, uint8 mgg, uint8 anim_id, uint8 animated, float speed_mul);
uint8 ShootProjectile(int8 id, int16 ang, int16 owner, Pos pos);

uint8 DefineParticle(int8 id, int16 frame, uint8 mgg, uint8 anim_id, uint8 animated, float speed_mul, int16 speed, void(*func)(PARTICLES));
uint8 SpawnParticle(int8 id, Pos pos, int16 ang);

int8 OnTheGround(int16 id, int32 *y);
int8 HitSprite(int16 id);
int8 HitscanID(Pos pos, int16 ang, int16 spr_id, Pos *rpos);
int8 Hitscan(Pos pos, int16 ang, int16  *spr_id, int16 *sector_id, Pos *rpos);
int8 HitProj(int16 id, uint8 *proj_id);
int8 HitProjID(int16 id, uint8 proj_id);

int8 MainPhysics();

void DrawMisc();