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

	uint8 bounce : 1,
	      num_bounces : 3,
          animated : 1,
		  rnd_bounce : 1,
		  vel_bounce : 1;

	int16 sector;

	float speed_mul;
};

typedef struct _PARTICLES PARTICLES;

enum PROJ_TYPE
{
	PROJ_BULLET = 1,
	PROJ_HITSCAN = 2,
	PROJ_REGULAR = 4,
	PROJ_EXPLODES_ON_IMPACT = 8,
	PROJ_TIMED_EXPLOSION = 16,
	PROJ_SPAWNER = 32,
	PROJ_TIMED = 64,
	PROJ_RANGED = 128
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

	uint32 time;
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

	PROJECTILE proj[MAX_PROJECTILES];

	GAME_PROJECTILE game_proj[MAX_GAME_PROJECTILES];
};

typedef struct _Physics Physics;

void InitPhysics(int8 gravity, int32 gravity_vel, int16 gravity_direction, int8 air);
int16 UpdateSector(int16 id);
void ActivateDynamicSprite(int16 id);
void DeactivateDynamicSprite(int16 id);
int8 AddDynamicSprite(int16 id);
void RemoveDynamicSprite(int16 id);
int8 OnTheGround(int16 id, int32 *y);
int8 HitSprite(int16 id);
int8 MainPhysics();