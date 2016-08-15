#include "types.h"
#include "engine.h"

#define DEFAULT_GRAVITY 8
#define AIR_RESISTANCE_MAX 10

#define MAX_DYNAMIC_SPRITES 128
#define MAX_ACTIVE_DYNAMIC_SPRITES 64

#define NODES 21

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

	struct Nodes
	{
		Pos min, max;
		uint8 num_sprites;
		int16 sprites[MAX_ACTIVE_DYNAMIC_SPRITES];
		int8 sub_nodes[4];
	} nodes[NODES];

	uint8 num_nodes;

	uint16 sectorarea[MAX_SECTORS];
	int16 sectorarea_ids[MAX_SECTORS][MAX_DYNAMIC_SPRITES];

	int16 spritesectornum[MAX_ACTIVE_DYNAMIC_SPRITES];
	int8 spritesectors[MAX_ACTIVE_DYNAMIC_SPRITES][MAX_SECTORS];

};

typedef struct _Physics Physics;

void InitPhysics(int8 gravity, int32 gravity_vel, int16 gravity_direction, int8 air);
int16 UpdateSector(int16 id);
void ActivateDynamicSprite(int16 id);
void DeactivateDynamicSprite(int16 id);
int8 AddDynamicSprite(int16 id);
void RemoveDynamicSprite(int16 id);
int8 OnTheGround(int16 id, int32 *y);
int8 MainPhysics();